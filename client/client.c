#include <stdio.h>
#include <stdlib.h>       
#include "client.h"   

char g_fileName[256];     // 保存服务器发送过来的文件名
char* g_fileBuf;          // 接受存储文件内容
char g_recvBuf[1024];     // 接受消息缓冲区
int g_fileSize;           // 文件总大小

int main(void)
{
	initSocket();

	connectToHost();

	closeSocket();

	return 0;
}

// 初始化socket库
bool initSocket()
{
	WSADATA wsadata;

	if (0 != WSAStartup(MAKEWORD(2, 2), &wsadata))        // 启动协议,成功返回0
	{
		printf("WSAStartup faild: %d\n", WSAGetLastError());
		return false;
	}

	return true;
}

// 关闭socket库
bool closeSocket()
{
	if (0 != WSACleanup())
	{
		printf("WSACleanup faild: %d\n", WSAGetLastError());
		return false;
	}

	return true;
}

// 监听客户端连接
void connectToHost()
{
	// 创建server socket套接字 地址、端口号,AF_INET是IPV4
	SOCKET serfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (INVALID_SOCKET == serfd)
	{
		printf("socket faild:%d", WSAGetLastError());
		return;
	}

	// 给socket绑定IP地址和端口号
	struct sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(SPORT);                       // htons把本地字节序转为网络字节序
	serAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1"); // 服务器的IP地址

	// 连接到服务器
	if (0 != connect(serfd, (struct sockaddr*)&serAddr, sizeof(serAddr)))
	{
		printf("connect faild:%d", WSAGetLastError());
		return;
	}

	printf("连接成功！\n");

	downloadFileName(serfd);                              // 输入文件名

	// 开始处理消息,100为发送消息间隔
	while (processMag(serfd))
	{
		//Sleep(100);
	}
}

// 处理消息
bool processMag(SOCKET serfd)
{

	recv(serfd, g_recvBuf, 1024, 0);                     // 收到消息   
	struct MsgHeader* msg = (struct MsgHeader*)g_recvBuf;

	/*
	*MSG_FILENAME       = 1,       // 文件名称                服务器使用
	*MSG_FILESIZE       = 2,       // 文件大小                客户端使用
	*MSG_READY_READ     = 3,       // 准备接受                客户端使用
	*MSG_SENDFILE       = 4,       // 发送                    服务器使用
	*MSG_SUCCESSED      = 5,       // 传输完成                两者都使用
	*MSG_OPENFILE_FAILD = 6        // 告诉客户端文件找不到    客户端使用
	*/

	switch (msg->msgID)
	{
	case MSG_OPENFILE_FAILD:         // 6
		downloadFileName(serfd);
		break;
	case MSG_FILESIZE:               // 2  第一次接收
		readyread(serfd, msg);
		break;
	case MSG_READY_READ:             // 3
		writeFile(serfd, msg);
		break;
	case MSG_SUCCESSED:              // 5
		printf("传输完成！\n");
		closeSocket(serfd);
		return false;
		break;
	}
	return true;
}

void downloadFileName(SOCKET serfd)
{
	char fileName[1024];
	struct MsgHeader file;

	printf("输入下载的文件名：");

	gets_s(fileName, 1023);                                  // 输入文件路径               
	file.msgID = MSG_FILENAME;                               // MSG_FILENAME = 1
	strcpy(file.fileInfo.fileName, fileName);

	send(serfd, (char*)&file, sizeof(struct MsgHeader), 0);  // 发送、IP地址、内容、长度    第一次发送给服务器
}

void readyread(SOCKET serfd, struct MsgHeader* pmsg)
{
	// 准备内存 pmsg->fileInfo.fileSize
	g_fileSize = pmsg->fileInfo.fileSize;
	strcpy(g_fileName, pmsg->fileInfo.fileName);

	g_fileBuf = calloc(g_fileSize + 1, sizeof(char));         // 申请空间

	if (g_fileBuf == NULL)
	{
		printf("申请内存失败\n");
	}
	else
	{
		struct MsgHeader msg;  // MSG_SENDFILE = 4
		msg.msgID = MSG_SENDFILE;

		if (SOCKET_ERROR == send(serfd, (struct MsgHeader*)&msg, sizeof(struct MsgHeader), 0))   // 第二次发送
		{
			printf("客户端 send error: %d\n", WSAGetLastError());
			return;
		}
	}

	printf("size:%d  filename:%s\n", pmsg->fileInfo.fileSize, pmsg->fileInfo.fileName);
}

bool writeFile(SOCKET serfd, struct MsgHeader* pmsg)
{
	if (g_fileBuf == NULL)
	{
		return false;
	}

	int nStart = pmsg->packet.nStart;
	int nsize = pmsg->packet.nsize;

	memcpy(g_fileBuf + nStart, pmsg->packet.buf, nsize);    // strncmpy一样
	printf("packet size:%d %d\n", nStart + nsize, g_fileSize);

	if (nStart + nsize >= g_fileSize)                       // 判断数据是否发完数据
	{
		FILE* pwrite;
		struct MsgHeader msg;

		pwrite = fopen(g_fileName, "wb");
		msg.msgID = MSG_SUCCESSED;

		if (pwrite == NULL)
		{
			printf("write file error...\n");
			return false;
		}

		fwrite(g_fileBuf, sizeof(char), g_fileSize, pwrite);
		fclose(pwrite);

		free(g_fileBuf);
		g_fileBuf = NULL;

		send(serfd, (char*)&msg, sizeof(struct MsgHeader), 0);

		return false;
	}

	return true;
}
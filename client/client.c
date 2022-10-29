#include <stdio.h>
#include <stdlib.h>       
#include "client.h"   

char g_fileName[256];     // 保存服务器发送过来的文件名
char* g_fileBuf;          // 接受存储文件内容
char g_recvBuf[1024];     // 接受消息缓冲区
int g_fileSize;           // 文件总大小
char accountName[25];
char accountPassword[25];
char accountPasswordCheck[25];

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
	serAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1"); // 服务器的IP地址(localhost)

	// 连接到服务器
	if (0 != connect(serfd, (struct sockaddr*)&serAddr, sizeof(serAddr)))
	{
		printf("connect faild:%d", WSAGetLastError());
		return;
	}

	printf("服务器连接成功！欢迎使用本FTP系统！\n");
	if (!signInOrRegister(serfd)) {
		return;
	}
	//while (processMag(serfd));
}

// 处理消息
bool processMag(SOCKET serfd)
{
	recv(serfd, g_recvBuf, 1024, 0);                     // 收到消息   
	struct MsgHeader* msg = (struct MsgHeader*)g_recvBuf;

	switch (msg->msgID)
	{
	case MSG_SIGN_IN:
		return true;
	case MSG_REGISTER:
		return false;
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
		//closeSocket(serfd);
		return false;
	}
	return true;
}

bool signInOrRegister(SOCKET serfd) {
	char flag[10];
	printf("使用本FTP系统前请先进行登录或注册！\n");
	printf("登录请输入s，注册请输入r，退出请输入q：");
	while (1) {
		scanf("%s", flag);
		if (flag[0] == 'r' || flag[0] == 'R') {
			if (!regist(serfd)) {
				return false;
			}
		}
		else if (flag[0] == 's' || flag[0] == 'S') {
			signIn(serfd);
			return true;
		}
		else if (flag[0] == 'q' || flag[0] == 'Q') {
			return false;
		}
		else {
			printf("输入格式不正确！请重新输入\n");
			printf("登录请输入s，注册请输入r，退出请输入q：");
		}
	}
}

void signIn(SOCKET serfd) {
	struct MsgHeader* msg;
	while (1) {
		struct MsgHeader login;
		printf("请输入您的用户名：");
		scanf("%s", &accountName);
		printf("请输入您的密码：");
		scanf("%s", &accountPassword);
		login.msgID = MSG_SIGN_IN;
		strcpy(login.signRegisInfo.accountName, accountName);
		strcpy(login.signRegisInfo.accountPassword, accountPassword);
		send(serfd, (char*)&login, sizeof(struct MsgHeader), 0);
		recv(serfd, g_recvBuf, 1024, 0); 
		msg = (struct MsgHeader*)g_recvBuf;
		if (msg->signRegisInfo.result) {
			printf("登陆成功！\n");
			break;
		}
		else {
			printf("您输入的用户名、密码不正确，请重新输入！\n");
		}
	}
	return;
}

bool regist(SOCKET serfd) {
	struct MsgHeader* msg;
	struct MsgHeader regist;
	int i, j;
	int check = 0;
	while (1) {
		printf("请输入您的用户名，用户名应不超过20个字符：");
		scanf("%s", &accountName);
		if (strlen(accountName) > 20) {
			printf("您输入的用户名不合规！");
		}
		else break;
	}
	while (1) {
		printf("请输入您的密码，密码应不超过20个字符：");
		scanf("%s", &accountPassword);
		if (strlen(accountPassword) > 20) {
			printf("您输入的密码不合规！\n");
		}
		else break;
	}
	while (1) {
		printf("请再次输入您的密码：");
		scanf("%s", &accountPasswordCheck);
		for (i = 0, j = 0;;i++,j++) {
			if (i == strlen(accountPassword) && j == strlen(accountPasswordCheck)) {
				check = 1;
				break;
			}
			else if (accountPassword[i] == accountPasswordCheck[j]) {
				continue;
			}
			else {
				check = 0;
				break;
			}
		}
		if (check==0) {
			printf("您两次输入的密码不相同！\n");
		}
		else break;
	}
	regist.msgID = MSG_REGISTER;
	strcpy(regist.signRegisInfo.accountName, accountName);
	strcpy(regist.signRegisInfo.accountPassword, accountPassword);
	send(serfd, (char*)&regist, sizeof(struct MsgHeader), 0);
	printf("注册申请已发送！请等待客户端回答！\n");
	recv(serfd, g_recvBuf, 1024, 0);
	msg = (struct MsgHeader*)g_recvBuf;
	if (msg->signRegisInfo.result) {
		printf("注册成功！请登录后使用本系统！\n");
		signIn(serfd);
		return true;
	}
	else {
		printf("注册申请未通过！请退出程序！\n");
		system("pause");
		return false;
	}
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
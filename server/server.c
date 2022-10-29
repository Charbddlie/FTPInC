#include <stdio.h>
#include "server.h"

char g_recvBuf[1024] = { 0 };      // 用来接收客户端消息
int g_fileSize;                    // 文件大小
char* g_fileBuf;                   // 储存文件

int main(void)
{
	initSocket();

	listenToClient();

	closeSocket();

	return 0;
}

// 初始化socket库
bool initSocket()
{
	WSADATA wsadata;

	if (0 != WSAStartup(MAKEWORD(2, 2), &wsadata))  // 启动协议,成功返回0
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
void listenToClient()
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
	serAddr.sin_port = htons(SPORT);             // htons把本地字节序转为网络字节序
	serAddr.sin_addr.S_un.S_addr = ADDR_ANY;     // 监听本机所有网卡

	if (0 != bind(serfd, (struct sockaddr*)&serAddr, sizeof(serAddr)))
	{
		printf("bind faild:%d", WSAGetLastError());
		return;
	}

	// 监听客户端连接
	if (0 != listen(serfd, 10))                  // 10为队列最大数
	{
		printf("listen faild:%d", WSAGetLastError());
		return;
	}

	// 有客户端连接，接受连接
	struct sockaddr_in cliAddr;
	int len = sizeof(cliAddr);

	SOCKET clifd = accept(serfd, (struct sockaddr*)&cliAddr, &len);

	if (INVALID_SOCKET == clifd)
	{
		printf("accept faild:%d", WSAGetLastError());
		return;
	}

	printf("接受成功！\n");

	// 开始处理消息
	while (processMag(clifd))
	{
		Sleep(100);
	}

}

// 处理消息
bool processMag(SOCKET clifd)
{
	// 成功接收消息返回收到的字节数，否则返回0
	int nRes = recv(clifd, g_recvBuf, 1024, 0);         // 接收

	if (nRes <= 0)
	{
		printf("客户端下线...%d", WSAGetLastError());
	}

	// 获取接受的的消息
	struct MsgHeader* msg = (struct MsgHeader*)g_recvBuf;
	struct MsgHeader exitmsg;

	/*
	*MSG_FILENAME       = 1,    // 文件名称                服务器使用
	*MSG_FILESIZE       = 2,    // 文件大小                客户端使用
	*MSG_READY_READ     = 3,    // 准备接受                客户端使用
	*MSG_SENDFILE       = 4,    // 发送                    服务器使用
	*MSG_SUCCESSED      = 5,    // 传输完成                两者都使用
	*MSG_OPENFILE_FAILD = 6     // 告诉客户端文件找不到    客户端使用
	*/

	switch (msg->msgID)
	{
	case MSG_SIGN_IN:
		signInCheck(clifd, msg);
		break;
	case MSG_REGISTER:
		registerCheck(clifd, msg);
		return false;
	case MSG_FILENAME:          // 1  第一次接收
		printf("%s\n", msg->fileInfo.fileName);
		readFile(clifd, msg);
		break;
	case MSG_SENDFILE:          // 4
		sendFile(clifd, msg);
		break;
	case MSG_SUCCESSED:         // 5

		exitmsg.msgID = MSG_SUCCESSED;

		if (SOCKET_ERROR == send(clifd, (char*)&exitmsg, sizeof(struct MsgHeader), 0))   //失败发送给客户端
		{
			printf("send faild: %d\n", WSAGetLastError());
		}

		printf("完成！\n");
		closeSocket(clifd);

		return false;
		break;
	}

	printf("%s\n", g_recvBuf);

	return true;
}
void signInCheck(SOCKET clifd, struct MsgHeader* cmsg) {
	FILE* file = fopen("list.csv", "r");
	int i, j;
	bool result;
	if (!file) {
		printf("文件打开失败！");
		exit(1);
	}
	char temp[50];
	char name[20];
	while (1) {
		if (feof(file)) {
			result = false;
			break;
		}
		i = 0;
		j = 0;
		fgets(temp, 49, file);
		while (temp[i] != ',') {
			name[i] = temp[i];
			i++;
		}
		name[i] = '\0';
		if (strcmp(name, cmsg->signRegisInfo.accountName) != 0) {
			continue;
		}
		else {
			char password[20];
			for(i += 1; temp[i] != ','; j++,i++) {
				password[j] = temp[i];
			}
			password[j] = '\0';
			if (strcmp(password, cmsg->signRegisInfo.accountPassword) != 0) {
				result = false;
				break;
			}
			else {
				result = true;
				break;
			}
		}
	}
	struct MsgHeader msg;
	msg.msgID = MSG_SIGN_IN;
	msg.signRegisInfo.result = result;
	send(clifd, (char*)&msg, sizeof(struct MsgHeader), 0);
	fclose(file);
	return;
}	
void registerCheck(SOCKET clifd, struct MsgHeader* cmsg) {
	FILE* file = fopen("list.csv", "r");
	int i;
	bool result;
	if (!file) {
		printf("文件打开失败！");
		exit(1);
	}
	char temp[50];
	char name[20];
	while (1) {
		if (feof(file)) {
			result = true;
			break;
		}
		i = 0;
		fgets(temp, 49, file);
		while (temp[i] != ',') {
			name[i] = temp[i];
			i++;
		}
		name[i] = '\0';
		if (strcmp(name, cmsg->signRegisInfo.accountName) != 0) {
			continue;
		}
		else {
			result = false;
			break;
		}
	}
	fclose(file);
	char getAnswer[5];
	char au[5];
	if (result == true) {
		printf("姓名为%s的用户发来一个注册请求，请问是否同意？\n", cmsg->signRegisInfo.accountName);
		while (1) {
			printf("同意请输入y，不同意请输入n:");
			scanf("%s", getAnswer);
			if (getAnswer[0] == 'y' || getAnswer[0] == 'Y') {
				printf("请为该用户设置权限：1（只能上传文件）、2（可以上传下载）、3（可以上传下载文件、修改目录）\n");
				while (1) {
					printf("请输入对应的编号（1、2、3）：");
					scanf("%s", getAnswer);
					if (getAnswer[0] == '1' || getAnswer[0] == '2' || getAnswer[0] == '3') {
						au[0] = getAnswer[0];
						au[1] = '\0';
						break;
					}
					else {
						printf("输入格式不正确，请重新输入！\n");
					}
				}
				FILE* fileOut = fopen("list.csv", "a");
				if (!fileOut) {
					printf("文件打开失败！\n");
					exit(-1);
				}
				char line[50];
				line[0] = '\0';
				strcat(line, cmsg->signRegisInfo.accountName);
				strcat(line, ",");
				strcat(line, cmsg->signRegisInfo.accountPassword);
				strcat(line, ",");
				strcat(line, au);
				strcat(line, "\n");
				fputs(line, fileOut);
				break;
			}
			if (getAnswer[0] == 'n' || getAnswer[0] == 'N') {
				result == false;
				break;
			}
			else {
				printf("输入格式不正确，请重新输入！\n");
			}
		}
	}
	struct MsgHeader msg;
	msg.msgID = MSG_REGISTER;
	msg.signRegisInfo.result = result;
	send(clifd, (char*)&msg, sizeof(struct MsgHeader), 0);
	return;
}

/*
*1.客户端请求下载文件 —把文件名发送给服务器
*2.服务器接收客户端发送的文件名 —根据文件名找到文件，把文件大小发送给客户端
*3.客户端接收到文件大小—准备开始接受，开辟内存  准备完成要告诉服务器可以发送了
*4.服务器接受的开始发送的指令开始发送
*5.开始接收数据，存起来     接受完成，告诉服务器接收完成
*6.关闭连接
*/

bool readFile(SOCKET clifd, struct MsgHeader* pmsg)
{
	FILE* pread = fopen(pmsg->fileInfo.fileName, "rb");

	if (pread == NULL)
	{
		printf("找不到[%s]文件...\n", pmsg->fileInfo.fileName);

		struct MsgHeader msg;
		msg.msgID = MSG_OPENFILE_FAILD;                                             // MSG_OPENFILE_FAILD = 6

		if (SOCKET_ERROR == send(clifd, (char*)&msg, sizeof(struct MsgHeader), 0))   // 失败发送给客户端
		{
			printf("send faild: %d\n", WSAGetLastError());
		}

		return false;
	}

	// 获取文件大小
	fseek(pread, 0, SEEK_END);
	g_fileSize = ftell(pread);
	fseek(pread, 0, SEEK_SET);

	// 把文件大小发给客户端
	char text[100];
	char tfname[200] = { 0 };
	struct MsgHeader msg;

	msg.msgID = MSG_FILESIZE;                                       // MSG_FILESIZE = 2
	msg.fileInfo.fileSize = g_fileSize;

	_splitpath(pmsg->fileInfo.fileName, NULL, NULL, tfname, text);  //******************************************

	strcat(tfname, text);
	strcpy(msg.fileInfo.fileName, tfname);

	send(clifd, (char*)&msg, sizeof(struct MsgHeader), 0);            // 文件名和后缀、文件大小发回客户端  第一次发送给客户端

	//读写文件内容
	g_fileBuf = calloc(g_fileSize + 1, sizeof(char));

	if (g_fileBuf == NULL)
	{
		printf("内存不足，重试\n");
		return false;
	}

	fread(g_fileBuf, sizeof(char), g_fileSize, pread);
	g_fileBuf[g_fileSize] = '\0';

	fclose(pread);
	return true;
}

bool sendFile(SOCKET clifd, struct MsgHeader* pms)
{
	struct MsgHeader msg;                                                     // 告诉客户端准备接收文件
	msg.msgID = MSG_READY_READ;

	// 如果文件的长度大于每个数据包能传送的大小（1012），那么久分块
	for (size_t i = 0; i < g_fileSize; i += PACKET_SIZE)                       // PACKET_SIZE = 1012
	{
		msg.packet.nStart = i;

		// 包的大小大于总数据的大小
		if (i + PACKET_SIZE + 1 > g_fileSize)
		{
			msg.packet.nsize = g_fileSize - i;
		}
		else
		{
			msg.packet.nsize = PACKET_SIZE;
		}

		memcpy(msg.packet.buf, g_fileBuf + msg.packet.nStart, msg.packet.nsize);

		if (SOCKET_ERROR == send(clifd, (char*)&msg, sizeof(struct MsgHeader), 0))  // 告诉客户端可以发送
		{
			printf("文件发送失败：%d\n", WSAGetLastError());
		}
	}

	return true;
}
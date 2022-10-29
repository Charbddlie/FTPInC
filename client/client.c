#include <stdio.h>
#include <stdlib.h>       
#include "client.h"   

char g_fileName[256];     // ������������͹������ļ���
char* g_fileBuf;          // ���ܴ洢�ļ�����
char g_recvBuf[1024];     // ������Ϣ������
int g_fileSize;           // �ļ��ܴ�С
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

// ��ʼ��socket��
bool initSocket()
{
	WSADATA wsadata;

	if (0 != WSAStartup(MAKEWORD(2, 2), &wsadata))        // ����Э��,�ɹ�����0
	{
		printf("WSAStartup faild: %d\n", WSAGetLastError());
		return false;
	}

	return true;
}

// �ر�socket��
bool closeSocket()
{
	if (0 != WSACleanup())
	{
		printf("WSACleanup faild: %d\n", WSAGetLastError());
		return false;
	}

	return true;
}

// �����ͻ�������
void connectToHost()
{
	// ����server socket�׽��� ��ַ���˿ں�,AF_INET��IPV4
	SOCKET serfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (INVALID_SOCKET == serfd)
	{
		printf("socket faild:%d", WSAGetLastError());
		return;
	}

	// ��socket��IP��ַ�Ͷ˿ں�
	struct sockaddr_in serAddr;
	serAddr.sin_family = AF_INET;
	serAddr.sin_port = htons(SPORT);                       // htons�ѱ����ֽ���תΪ�����ֽ���
	serAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1"); // ��������IP��ַ(localhost)

	// ���ӵ�������
	if (0 != connect(serfd, (struct sockaddr*)&serAddr, sizeof(serAddr)))
	{
		printf("connect faild:%d", WSAGetLastError());
		return;
	}

	printf("���������ӳɹ�����ӭʹ�ñ�FTPϵͳ��\n");
	if (!signInOrRegister(serfd)) {
		return;
	}
	//while (processMag(serfd));
}

// ������Ϣ
bool processMag(SOCKET serfd)
{
	recv(serfd, g_recvBuf, 1024, 0);                     // �յ���Ϣ   
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
	case MSG_FILESIZE:               // 2  ��һ�ν���
		readyread(serfd, msg);
		break;
	case MSG_READY_READ:             // 3
		writeFile(serfd, msg);
		break;
	case MSG_SUCCESSED:              // 5
		printf("������ɣ�\n");
		//closeSocket(serfd);
		return false;
	}
	return true;
}

bool signInOrRegister(SOCKET serfd) {
	char flag[10];
	printf("ʹ�ñ�FTPϵͳǰ���Ƚ��е�¼��ע�ᣡ\n");
	printf("��¼������s��ע��������r���˳�������q��");
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
			printf("�����ʽ����ȷ������������\n");
			printf("��¼������s��ע��������r���˳�������q��");
		}
	}
}

void signIn(SOCKET serfd) {
	struct MsgHeader* msg;
	while (1) {
		struct MsgHeader login;
		printf("�����������û�����");
		scanf("%s", &accountName);
		printf("�������������룺");
		scanf("%s", &accountPassword);
		login.msgID = MSG_SIGN_IN;
		strcpy(login.signRegisInfo.accountName, accountName);
		strcpy(login.signRegisInfo.accountPassword, accountPassword);
		send(serfd, (char*)&login, sizeof(struct MsgHeader), 0);
		recv(serfd, g_recvBuf, 1024, 0); 
		msg = (struct MsgHeader*)g_recvBuf;
		if (msg->signRegisInfo.result) {
			printf("��½�ɹ���\n");
			break;
		}
		else {
			printf("��������û��������벻��ȷ�����������룡\n");
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
		printf("�����������û������û���Ӧ������20���ַ���");
		scanf("%s", &accountName);
		if (strlen(accountName) > 20) {
			printf("��������û������Ϲ棡");
		}
		else break;
	}
	while (1) {
		printf("�������������룬����Ӧ������20���ַ���");
		scanf("%s", &accountPassword);
		if (strlen(accountPassword) > 20) {
			printf("����������벻�Ϲ棡\n");
		}
		else break;
	}
	while (1) {
		printf("���ٴ������������룺");
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
			printf("��������������벻��ͬ��\n");
		}
		else break;
	}
	regist.msgID = MSG_REGISTER;
	strcpy(regist.signRegisInfo.accountName, accountName);
	strcpy(regist.signRegisInfo.accountPassword, accountPassword);
	send(serfd, (char*)&regist, sizeof(struct MsgHeader), 0);
	printf("ע�������ѷ��ͣ���ȴ��ͻ��˻ش�\n");
	recv(serfd, g_recvBuf, 1024, 0);
	msg = (struct MsgHeader*)g_recvBuf;
	if (msg->signRegisInfo.result) {
		printf("ע��ɹ������¼��ʹ�ñ�ϵͳ��\n");
		signIn(serfd);
		return true;
	}
	else {
		printf("ע������δͨ�������˳�����\n");
		system("pause");
		return false;
	}
}

void downloadFileName(SOCKET serfd)
{
	char fileName[1024];
	struct MsgHeader file;

	printf("�������ص��ļ�����");

	gets_s(fileName, 1023);                                  // �����ļ�·��               
	file.msgID = MSG_FILENAME;                               // MSG_FILENAME = 1
	strcpy(file.fileInfo.fileName, fileName);

	send(serfd, (char*)&file, sizeof(struct MsgHeader), 0);  // ���͡�IP��ַ�����ݡ�����    ��һ�η��͸�������
}

void readyread(SOCKET serfd, struct MsgHeader* pmsg)
{
	// ׼���ڴ� pmsg->fileInfo.fileSize
	g_fileSize = pmsg->fileInfo.fileSize;
	strcpy(g_fileName, pmsg->fileInfo.fileName);

	g_fileBuf = calloc(g_fileSize + 1, sizeof(char));         // ����ռ�

	if (g_fileBuf == NULL)
	{
		printf("�����ڴ�ʧ��\n");
	}
	else
	{
		struct MsgHeader msg;  // MSG_SENDFILE = 4
		msg.msgID = MSG_SENDFILE;

		if (SOCKET_ERROR == send(serfd, (struct MsgHeader*)&msg, sizeof(struct MsgHeader), 0))   // �ڶ��η���
		{
			printf("�ͻ��� send error: %d\n", WSAGetLastError());
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

	memcpy(g_fileBuf + nStart, pmsg->packet.buf, nsize);    // strncmpyһ��
	printf("packet size:%d %d\n", nStart + nsize, g_fileSize);

	if (nStart + nsize >= g_fileSize)                       // �ж������Ƿ�������
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
#include <stdio.h>
#include <stdlib.h>       
#include "client.h"   

char g_fileName[256];     // ������������͹������ļ���
char* g_fileBuf;          // ���ܴ洢�ļ�����
char g_recvBuf[1024];     // ������Ϣ������
int g_fileSize;           // �ļ��ܴ�С

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
	serAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1"); // ��������IP��ַ

	// ���ӵ�������
	if (0 != connect(serfd, (struct sockaddr*)&serAddr, sizeof(serAddr)))
	{
		printf("connect faild:%d", WSAGetLastError());
		return;
	}

	printf("���ӳɹ���\n");

	downloadFileName(serfd);                              // �����ļ���

	// ��ʼ������Ϣ,100Ϊ������Ϣ���
	while (processMag(serfd))
	{
		//Sleep(100);
	}
}

// ������Ϣ
bool processMag(SOCKET serfd)
{

	recv(serfd, g_recvBuf, 1024, 0);                     // �յ���Ϣ   
	struct MsgHeader* msg = (struct MsgHeader*)g_recvBuf;

	/*
	*MSG_FILENAME       = 1,       // �ļ�����                ������ʹ��
	*MSG_FILESIZE       = 2,       // �ļ���С                �ͻ���ʹ��
	*MSG_READY_READ     = 3,       // ׼������                �ͻ���ʹ��
	*MSG_SENDFILE       = 4,       // ����                    ������ʹ��
	*MSG_SUCCESSED      = 5,       // �������                ���߶�ʹ��
	*MSG_OPENFILE_FAILD = 6        // ���߿ͻ����ļ��Ҳ���    �ͻ���ʹ��
	*/

	switch (msg->msgID)
	{
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
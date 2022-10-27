#include <stdio.h>
#include "server.h"

char g_recvBuf[1024] = { 0 };      // �������տͻ�����Ϣ
int g_fileSize;                    // �ļ���С
char* g_fileBuf;                   // �����ļ�

int main(void)
{
	initSocket();

	listenToClient();

	closeSocket();

	return 0;
}

// ��ʼ��socket��
bool initSocket()
{
	WSADATA wsadata;

	if (0 != WSAStartup(MAKEWORD(2, 2), &wsadata))  // ����Э��,�ɹ�����0
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
void listenToClient()
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
	serAddr.sin_port = htons(SPORT);             // htons�ѱ����ֽ���תΪ�����ֽ���
	serAddr.sin_addr.S_un.S_addr = ADDR_ANY;     // ����������������

	if (0 != bind(serfd, (struct sockaddr*)&serAddr, sizeof(serAddr)))
	{
		printf("bind faild:%d", WSAGetLastError());
		return;
	}

	// �����ͻ�������
	if (0 != listen(serfd, 10))                  // 10Ϊ���������
	{
		printf("listen faild:%d", WSAGetLastError());
		return;
	}

	// �пͻ������ӣ���������
	struct sockaddr_in cliAddr;
	int len = sizeof(cliAddr);

	SOCKET clifd = accept(serfd, (struct sockaddr*)&cliAddr, &len);

	if (INVALID_SOCKET == clifd)
	{
		printf("accept faild:%d", WSAGetLastError());
		return;
	}

	printf("���ܳɹ���\n");

	// ��ʼ������Ϣ
	while (processMag(clifd))
	{
		Sleep(100);
	}

}

// ������Ϣ
bool processMag(SOCKET clifd)
{
	// �ɹ�������Ϣ�����յ����ֽ��������򷵻�0
	int nRes = recv(clifd, g_recvBuf, 1024, 0);         // ����

	if (nRes <= 0)
	{
		printf("�ͻ�������...%d", WSAGetLastError());
	}

	// ��ȡ���ܵĵ���Ϣ
	struct MsgHeader* msg = (struct MsgHeader*)g_recvBuf;
	struct MsgHeader exitmsg;

	/*
	*MSG_FILENAME       = 1,    // �ļ�����                ������ʹ��
	*MSG_FILESIZE       = 2,    // �ļ���С                �ͻ���ʹ��
	*MSG_READY_READ     = 3,    // ׼������                �ͻ���ʹ��
	*MSG_SENDFILE       = 4,    // ����                    ������ʹ��
	*MSG_SUCCESSED      = 5,    // �������                ���߶�ʹ��
	*MSG_OPENFILE_FAILD = 6     // ���߿ͻ����ļ��Ҳ���    �ͻ���ʹ��
	*/

	switch (msg->msgID)
	{
	case MSG_FILENAME:          // 1  ��һ�ν���
		printf("%s\n", msg->fileInfo.fileName);
		readFile(clifd, msg);
		break;
	case MSG_SENDFILE:          // 4
		sendFile(clifd, msg);
		break;
	case MSG_SUCCESSED:         // 5

		exitmsg.msgID = MSG_SUCCESSED;

		if (SOCKET_ERROR == send(clifd, (char*)&exitmsg, sizeof(struct MsgHeader), 0))   //ʧ�ܷ��͸��ͻ���
		{
			printf("send faild: %d\n", WSAGetLastError());
		}

		printf("��ɣ�\n");
		closeSocket(clifd);

		return false;
		break;
	}

	printf("%s\n", g_recvBuf);

	return true;
}

/*
*1.�ͻ������������ļ� �����ļ������͸�������
*2.���������տͻ��˷��͵��ļ��� �������ļ����ҵ��ļ������ļ���С���͸��ͻ���
*3.�ͻ��˽��յ��ļ���С��׼����ʼ���ܣ������ڴ�  ׼�����Ҫ���߷��������Է�����
*4.���������ܵĿ�ʼ���͵�ָ�ʼ����
*5.��ʼ�������ݣ�������     ������ɣ����߷������������
*6.�ر�����
*/

bool readFile(SOCKET clifd, struct MsgHeader* pmsg)
{
	FILE* pread = fopen(pmsg->fileInfo.fileName, "rb");

	if (pread == NULL)
	{
		printf("�Ҳ���[%s]�ļ�...\n", pmsg->fileInfo.fileName);

		struct MsgHeader msg;
		msg.msgID = MSG_OPENFILE_FAILD;                                             // MSG_OPENFILE_FAILD = 6

		if (SOCKET_ERROR == send(clifd, (char*)&msg, sizeof(struct MsgHeader), 0))   // ʧ�ܷ��͸��ͻ���
		{
			printf("send faild: %d\n", WSAGetLastError());
		}

		return false;
	}

	// ��ȡ�ļ���С
	fseek(pread, 0, SEEK_END);
	g_fileSize = ftell(pread);
	fseek(pread, 0, SEEK_SET);

	// ���ļ���С�����ͻ���
	char text[100];
	char tfname[200] = { 0 };
	struct MsgHeader msg;

	msg.msgID = MSG_FILESIZE;                                       // MSG_FILESIZE = 2
	msg.fileInfo.fileSize = g_fileSize;

	_splitpath(pmsg->fileInfo.fileName, NULL, NULL, tfname, text);  //******************************************

	strcat(tfname, text);
	strcpy(msg.fileInfo.fileName, tfname);

	send(clifd, (char*)&msg, sizeof(struct MsgHeader), 0);            // �ļ����ͺ�׺���ļ���С���ؿͻ���  ��һ�η��͸��ͻ���

	//��д�ļ�����
	g_fileBuf = calloc(g_fileSize + 1, sizeof(char));

	if (g_fileBuf == NULL)
	{
		printf("�ڴ治�㣬����\n");
		return false;
	}

	fread(g_fileBuf, sizeof(char), g_fileSize, pread);
	g_fileBuf[g_fileSize] = '\0';

	fclose(pread);
	return true;
}

bool sendFile(SOCKET clifd, struct MsgHeader* pms)
{
	struct MsgHeader msg;                                                     // ���߿ͻ���׼�������ļ�
	msg.msgID = MSG_READY_READ;

	// ����ļ��ĳ��ȴ���ÿ�����ݰ��ܴ��͵Ĵ�С��1012������ô�÷ֿ�
	for (size_t i = 0; i < g_fileSize; i += PACKET_SIZE)                       // PACKET_SIZE = 1012
	{
		msg.packet.nStart = i;

		// ���Ĵ�С���������ݵĴ�С
		if (i + PACKET_SIZE + 1 > g_fileSize)
		{
			msg.packet.nsize = g_fileSize - i;
		}
		else
		{
			msg.packet.nsize = PACKET_SIZE;
		}

		memcpy(msg.packet.buf, g_fileBuf + msg.packet.nStart, msg.packet.nsize);

		if (SOCKET_ERROR == send(clifd, (char*)&msg, sizeof(struct MsgHeader), 0))  // ���߿ͻ��˿��Է���
		{
			printf("�ļ�����ʧ�ܣ�%d\n", WSAGetLastError());
		}
	}

	return true;
}
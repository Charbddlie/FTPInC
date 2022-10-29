#pragma once
#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")  // ���ؾ�̬��
#include <stdbool.h>

#define SPORT 8888                 // �������˿ں�
#define PACKET_SIZE (1024 - sizeof(int) * 3)



// ������
enum MSGTAG
{
	MSG_FILENAME = 1,        // �ļ�����                ������ʹ��
	MSG_FILESIZE = 2,        // �ļ���С                �ͻ���ʹ��
	MSG_READY_READ = 3,        // ׼������                �ͻ���ʹ��
	MSG_SENDFILE = 4,        // ����                    ������ʹ��
	MSG_SUCCESSED = 5,        // �������                ���߶�ʹ��
	MSG_OPENFILE_FAILD = 6,         // ���߿ͻ����ļ��Ҳ���    �ͻ���ʹ��
	MSG_SIGN_IN = 7,
	MSG_REGISTER = 8,
};

#pragma pack(1)                     // ���ýṹ��1�ֽڶ���**************

struct MsgHeader                    // ��װ��Ϣͷ
{
	enum MSGTAG msgID;              // ��ǰ��Ϣ���   4
	union MyUnion
	{
		struct Mystruct
		{
			int fileSize;           // �ļ���С  4
			char fileName[256];     // �ļ���    256
		}fileInfo;
		struct
		{
			int nStart;             // ���ı��
			int nsize;              // �ð������ݴ�С
			char buf[PACKET_SIZE];
		}packet;
		struct SignInorRegister
		{
			char accountName[20];
			char accountPassword[20];
			bool result;
		}signRegisInfo;
	};

};

#pragma pack()

// ��ʼ��socket��
bool initSocket();

// �ر�socket��
bool closeSocket();

// �����ͻ�������
void connectToHost();

// ������Ϣ
bool processMag(SOCKET serfd);

// ��ȡ�ļ���
void downloadFileName(SOCKET serfd);

// �ļ����ݶ����ڴ�
void readyread(SOCKET, struct MsgHeader*);

// д���ļ�����
bool writeFile(SOCKET, struct MsgHeader*);

bool regist(SOCKET);

void signIn(SOCKET);

bool signInOrRegister(SOCKET);
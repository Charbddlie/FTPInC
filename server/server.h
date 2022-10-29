#pragma once
#include <stdbool.h>
#include <WinSock2.h>
#pragma comment(lib,"ws2_32.lib")  // 加载静态库

#define SPORT 8888                 // 服务器端口号
#define PACKET_SIZE (1024 - sizeof(int) * 3)

// 定义标记
enum MSGTAG
{
	MSG_FILENAME = 1,         // 文件名称              服务器使用
	MSG_FILESIZE = 2,         // 文件大小              客户端使用
	MSG_READY_READ = 3,         // 准备接受              客户端使用
	MSG_SENDFILE = 4,         // 发送                  服务器使用
	MSG_SUCCESSED = 5,         // 传输完成              两者都使用
	MSG_OPENFILE_FAILD = 6,          // 告诉客户端文件找不到  客户端使用
	MSG_SIGN_IN = 7,
	MSG_REGISTER = 8,
};

#pragma pack(1)                     // 设置结构体1字节对齐

struct MsgHeader                    // 封装消息头
{
	enum MSGTAG msgID;              // 当前消息标记   4
	union MyUnion
	{
		struct Mystruct
		{
			int fileSize;           // 文件大小  4
			char fileName[256];     // 文件名    256
		}fileInfo;
		struct
		{
			int nStart;             // 包的编号
			int nsize;              // 该包的数据大小
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

// 初始化socket库
bool initSocket();

// 关闭socket库
bool closeSocket();

// 监听客户端连接
void listenToClient();

// 处理消息
bool processMag(SOCKET clifd);

// 读取文件，获得文件大小
bool readFile(SOCKET, struct MsgHeader*);

// 发送文件
bool sendFile(SOCKET, struct MsgHeader*);

//检查用户名密码是否正确
void signInCheck(SOCKET, struct MsgHeader*);

//注册审批
void registerCheck(SOCKET, struct MsgHeader*);
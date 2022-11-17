#ifndef COMMON_H_
#define COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/mman.h>

//传输数据的端口用
#define DATA_PORT 4500
//处理连接的端口用
#define LISTEN_PORT 4501

#define MAX_SIZE 1024

#define LOGIN_SUCCESS 230
#define LOGIN_FAILED 430
#define LOGIN 100
#define REGIST_SUCCESS 101
#define REGISTER 102
#define REGIST_REFUSED 103
#define REGIST_NAME_REPEAT 104
#define REGIST_NAME_OK 105
#define REGIST_APPLICATION_OK 106

#define CMD_SUCCESS 200
#define SERVER_READY 210
#define FILE_VAIL 222
#define QUIT_SUCESS 221
#define CONN_SUCCESS 220
#define RET_SUCCESS 226
#define PATH_FAIL 510
#define CMD_FAIL 502
#define FILE_UNVAIL 550
#define PATH_OUT 666
#define OUT_OF_AUTHORITY 667
#define IS_DT_DIR 668

int init_server(int port);
int accept_client(int sock_fd);
int connect_server(int port, char *serv_ip);
int send_num(int sock_fd, int ret_code);
void read_input(char *buf, int size);
int recv_data(int sock_fd, char *buf, int buf_size);

//获得相应码
int get_return_code(int sock_fd);

//处理命令参数
void get_cmd_first_arg(char *buf, char *cmd, char *arg);

// 以下函数都当作bool用，出错返回0，直接用if就能判断
//处理文件路径
int file_name_valid(char *arg, int size);
int send_file(int work_fd, int sock_fd, char *file_path, char *file_name);
int get_file(int work_fd, int sock_fd, char *filepath);
#endif

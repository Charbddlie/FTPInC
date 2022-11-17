#ifndef CLIENT_H_
#define CLIENT_H_

#define FILE_DIR "../client_linux/local/"
#include "../common/common.h"
//根据回应码在终端上输出相应的信息
void print_return_code(int rc);
//客户端读取命令
int client_read_command(char *buf, int size, char *arg, char *code);
//向服务端获取指定文件
int client_get(int work_fd, char *arg);
//向服务端发送指定文件
int client_put(int work_fd, char *arg);
//客户端打开连接
int client_open_conn();
//列出当前目录的文件
int client_ls(int work_fd);
//返回当前所在文件夹
int client_pwd(int work_fd);
//新建文件夹
int client_mkdir(int work_fd, char *dir_name);
//变更所在目录
int client_cd(int work_fd, char *path_name);
//删除文件
int client_delete(int work_fd, char *file_name);
//客户端向服务端发送命令
int client_send_cmd(char *arg, char *code);
//客户端登录
void client_login();

void client_register();

#endif

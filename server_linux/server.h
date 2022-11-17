#ifndef SERVER_H_
#define SERVER_H_

#include "../common/common.h"

//服务端检验用户名和密码
int server_check(char *username, char *password);
//接收用户名和密码
int server_login(int sock_fd);
//
void file_write(int sock_fd, char *accountName, char *au);
//
int manager_check(int sock_fd, char *accountName, char *au);
//
int server_register(int sock_fd);
//接收客户端发送的命令
int server_get_request(int sock_fd, char *cmd, char *arg);
//用于数据传输的连接
int server_work_conn(int sock_fd);
//显示当前目录下的文件
int server_cmd_ls(int work_fd, int sock_fd);
//显示当前所在目录
int server_cmd_pwd(int work_fd, int sock_fd);
//新建文件夹
int server_cmd_mkdir(int work_fd, int sock_fd);
//变更所在目录
int server_cmd_cd(int work_fd, int sock_fd);
//删除文件
int server_cmd_delete(int work_fd, int sock_fd);
//下载文件
void server_cmd_get(int sock_fd, int work_fd, char *file_name);
//上传文件
void server_cmd_put(int sock_fd, int work_fd, char *file_name);
//处理请求的进程
void work_process(int sock_fd);

#endif

/*************************************************************************
    > File Name: server.h
    > Author: Ukey
    > Mail: gsl110809@gmail.com
    > Created Time: 2017年05月26日 星期五 10时24分33秒
 ************************************************************************/
#ifndef SERVER_H_
#define SERVER_H_

#include "../common/common.h"

//服务端检验用户名和密码
int server_check(char* username, char* password);
//接收用户名和密码
int server_login(int sock_fd);
//接收客户端发送的命令
int server_get_request(int sock_fd, char *cmd, char *arg);
//用于数据传输的连接
int server_work_conn(int sock_fd);
//显示当前目录下的文件
int server_cmd_list(int work_fd, int sock_fd);
//传输文件
void server_cmd_retr(int sock_fd, int work_fd, char *file_name);
//处理请求的进程
void work_process(int sock_fd);
#endif


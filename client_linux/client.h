/*************************************************************************
    > File Name: client.h
    > Author: Ukey
    > Mail: gsl110809@gmail.com
    > Created Time: 2017年05月26日 星期五 12时14分18秒
 ************************************************************************/

#ifndef CLIENT_H_
#define CLIENT_H_

#include "../common/common.h"
//返回服务端回送的回应码
int read_reply();
//根据回应码在终端上输出相应的信息
void print_reply(int rc);
//客户端读取命令
int client_read_command(char *buf, int size, char *arg, char *code);
//向服务端获取指定文件
int client_get(int work_fd, char *arg);
//客户端打开连接
int client_open_conn(int sock_fd);
//列出当前目录的文件
int client_list(int work_fd, int sock_fd);
//客户端向服务端发送命令
int client_send_cmd(char *arg, char *code);
//客户端登录
void client_login();

#endif

/*************************************************************************
    > File Name: common.h
    > Author: Ukey
    > Mail: gsl110809@gmail.com
    > Created Time: 2017年05月25日 星期四 14时36分26秒
 ************************************************************************/

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
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/mman.h>

//传输数据的端口用20端口
#define WORK_PORT   20
//处理连接的端口用21端口
#define LISTEN_PORT 21
#define MAX_SIZE	1024

int init_server(int port);
int accept_client(int sock_fd);
int connect_server(int port, char *serv_ip);
int send_response(int sock_fd, int ret_code);
void read_input(char *buf, int size);
int recv_data(int sock_fd, char *buf, int buf_size);
#endif

/*************************************************************************
    > File Name: common.c
    > Author: Ukey
    > Mail: gsl110809@gmail.com
    > Created Time: 2017年05月25日 星期四 14时58分33秒
 ************************************************************************/

#include "common.h"

int init_server(int port)
{
	int listen_fd, ret;
	struct sockaddr_in serv_addr;

	//创建套接字
	if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("create socket error:");
		exit(1);
	}

	//设置端口复用
	int flag = 1;
	if((ret = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag))) < 0)
	{
		close(listen_fd);
		perror("set port reuse error:");
		exit(1);
	}

	//关联地址和套接字
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if((ret = bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
	{
		close(listen_fd);
		perror("bind error:");
		exit(1);
	}
	
	//将套接字设为监听状态
	if((ret = listen(listen_fd, 20)) < 0)
	{
		close(listen_fd);
		perror("listen error");
		exit(1);
	}
	return listen_fd;
}

int accept_client(int listen_fd)
{
	int sock_fd;
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);
	sock_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &len);
	if(sock_fd < 0)
	{
		perror("accept error");
		exit(1);
	}
	printf("accept connect from IP:%s PORT:%d,sock_fd=%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), sock_fd);
	return sock_fd;
}

int connect_server(int serv_port, char *serv_ip)
{
	int sock_fd, ret;
	struct sockaddr_in serv_addr;

	if((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket error:");
		exit(1);
	}

	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(serv_port);
	serv_addr.sin_addr.s_addr = inet_addr(serv_ip);

	if((ret = connect(sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0)
	{
		perror("error connect:");
		exit(1);
	}

	return sock_fd;
}

int send_response(int sock_fd, int ret_code)
{
	int ret, conv = htonl(ret_code);
	if((ret = send(sock_fd, &conv, sizeof(conv), 0)) < 0)
	{
		perror("send error:");
		exit(1);
	}
	return 0;
}

void read_input(char* buf, int size)
{
	char *temp = NULL;
	bzero(buf, size);
	if(fgets(buf, size, stdin))
	{
		temp = strchr(buf, '\n');
		if(temp)
		{
			*temp = '\0';
		}
	}
}
int recv_data(int sock_fd, char* buf, int buf_size)
{
	size_t n;
	bzero(buf, sizeof(buf));

	if((n = recv(sock_fd, buf, buf_size, 0)) < 0)
	{
		perror("recv error");
		exit(1);
	}
	return n;
}




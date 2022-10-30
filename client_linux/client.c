/*************************************************************************
  > File Name: client.c
  > Author: Ukey
  > Mail: gsl110809@gmail.com
  > Created Time: 2017年05月25日 星期四 19时49分35秒
 ************************************************************************/

#include "client.h"
int sock_fd;

int read_reply()
{
	int ret_code = 0;
	if (recv(sock_fd, &ret_code, sizeof(ret_code), 0) < 0) 
	{
		perror("client: error reading message from server\n");
		return -1;
	}	
	return ntohl(ret_code);
}
	
void print_reply(int rc) 
{
	switch (rc)
	{
		case 220:
			printf("Welcome.\n");
			break;
		case 221:
			printf("Thanks you!\n");
			break;
		case 226:
			printf("Requested file action successful.\n");
			break;
		case 550:
			printf("File unavailable.\n");
			break;
	}
}
int client_read_command(char *buf, int size, char *arg, char *code)
{
	bzero(arg, sizeof(arg));
	bzero(code, sizeof(code));

	printf("client> ");
	fflush(stdout);
	read_input(buf, size);
	char *temp_arg = NULL;
	temp_arg = strtok(buf, " ");
	temp_arg = strtok(NULL, " ");
	
	if(temp_arg != NULL)
	{
		strncpy(arg, temp_arg, strlen(temp_arg) + 1);
	}
	if(strcmp(buf, "list") == 0)
	{
		strcpy(code, "LIST");
	}
	else if(strcmp(buf, "get") == 0)
	{
		strcpy(code, "RETR");
	}
	else if(strcmp(buf, "quit") == 0)
	{
		strcpy(code, "QUIT");
	}
	else
		return -1;
	bzero(buf, sizeof(buf));
	strcpy(buf, code);

	if(temp_arg != NULL)
	{
		strcat(buf, " ");
		strncat(buf, arg, strlen(arg) + 1);
	}
	
	return 0;
}

int client_get(int work_fd, char *arg)
{
	char data[MAX_SIZE];
	int size;
	printf("filename:%s\n", arg);
	FILE* fd = fopen(arg, "w");

	//将服务器传来的数据写入本地建立的文件
	while((size = recv(work_fd, data, MAX_SIZE, 0)) > 0)
	{
		fwrite(data, 1, size, fd);
	}
	if(size < 0)
	{
		perror("error\n");
	}
	fclose(fd);
	return 0;
}
int client_open_conn(int sock_fd)
{
	int listen_fd = init_server(WORK_PORT);

	int ack = 1;
	if((send(sock_fd, (char *)&ack, sizeof(ack), 0)) < 0)
	{
		printf("client:ack write error:%d\n", errno);
		exit(1);
	}
	int work_fd = accept_client(listen_fd);
	close(listen_fd);
	return work_fd;
}


int client_list(int work_fd, int sock_fd)
{
	size_t n;
	char buf[MAX_SIZE];
	int temp = 0;
	bzero(buf, sizeof(buf));
	//等待服务器启动的信息
	if((recv(sock_fd, &temp, sizeof(temp), 0)) < 0)
	{
		perror("client: error reading message from server\n");
		exit(1);
	}

	//接收服务器传来的信息
	while((n = recv(work_fd, buf, MAX_SIZE, 0)) > 0)
	{
		printf("%s", buf);
		bzero(buf, sizeof(buf));
	}

	if(n < 0)
	{
		perror("error");
	}

	//等待服务器完成的消息
	if(recv(sock_fd, &temp, sizeof(temp), 0) < 0)
	{
		perror("client:error reading message from server\n");
		exit(1);
	}
	return 0;
}



int client_send_cmd(char *arg, char *code)
{
	char buf[MAX_SIZE];
	int rc;

	sprintf(buf, "%s %s", code, arg);

	//发送命令字符串到服务器
	rc = send(sock_fd, buf, sizeof(buf), 0);
	if(rc < 0)
	{
		perror("Error sending command to server");
		exit(1);
	}
	return 0;
}

void client_login()
{
	char arg[100], code[5], user[100];
	bzero(arg, sizeof(arg));
	bzero(code, sizeof(code));
	bzero(user, sizeof(user));

	//获取用户名
	printf("NAME: ");
	fflush(stdout);
	read_input(user, 100);

	//发送用户名到服务器
	strcpy(code, "USER");
	strcpy(arg, user);
	client_send_cmd(arg, code);

	//等待应答码
	int wait;
	recv(sock_fd, &wait, sizeof(wait), 0);

	//获取密码
	fflush(stdout);
	char *pass = getpass("Password:");

	//发送密码到服务器
	strcpy(code, "PASS");
	strcpy(arg, pass);
	client_send_cmd(arg, code);

	//等待响应
	int ret_code = read_reply();
	switch(ret_code)
	{
		case 230:
			printf("login success!\n");
			break;
		case 430:
			printf("Invaild username/password.\n");
			break;
		default:
			{
				perror("error reading message from server");
				exit(1);
				break;
			}
	}

}

int main(int argc, char *argv[])
{
	int work_fd, ret_code, s;
	char buf[MAX_SIZE],  port[10], arg[100], code[5];
	struct addrinfo hints, *result, *rp;
	if(argc != 2)
	{
		printf("please input client hostname\n");
		exit(1);
	}

	char *host = argv[1];

	//获取与主机名匹配的地址
	bzero(&hints, sizeof(struct addrinfo));
	bzero(port, sizeof(port));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	sprintf(port, "%d", LISTEN_PORT);
	if((s = getaddrinfo(host, port, &hints, &result)) != 0)
	{
		printf("getaddrinfo() error %s", gai_strerror(s));
		exit(1);
	}
	//找到符合要求的服务器地址并连接
	for(rp = result; rp != NULL; rp = rp->ai_next)
	{
		sock_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if(sock_fd < 0)
		{
			continue;
		}
		if(connect(sock_fd, result->ai_addr, result->ai_addrlen) == 0)
			break;
		else
		{
			perror("connect error");
			exit(1);
		}
	}
	freeaddrinfo(rp);

	//连接成功
	printf("Connected to %s\n", host);
	print_reply(read_reply());

	//获取用户的名字和密码
	client_login();

	while(1)
	{
		//获取到用户输入的命令
		if(client_read_command(buf, sizeof(buf), arg, code) < 0)
		{
			printf("Invaild command\n");
			continue;
		}

		//发送命令到服务器
		if(send(sock_fd, buf, (int)strlen(buf), 0) < 0)
		{
			close(sock_fd);
			exit(1);
		}
		
		ret_code = read_reply();	//读取服务器响应
		if(ret_code == 221)
		{
			print_reply(221);
			break;
		}
		if(ret_code == 502)
		{
			printf("%d Invaild command.\n", ret_code);
		}
		else //命令是合法的
		{
			//打开数据连接
			if((work_fd = client_open_conn(sock_fd)) < 0)
			{
				perror("Error opening socket for data connection");
				exit(1);
			}

			//执行命令
			if(strcmp(code, "LIST") == 0)
			{
				client_list(work_fd, sock_fd);
			}
			else if(strcmp(code, "RETR") == 0)
			{
				if(read_reply() == 550)
				{
					print_reply(550);
					close(work_fd);
					continue;
				}
				client_get(work_fd, arg);
				print_reply(read_reply());
			}
		}
	}
	close(sock_fd);
	return 0;
}






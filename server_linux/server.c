/*************************************************************************
    > File Name: server.c
    > Author: Ukey
    > Mail: gsl110809@gmail.com
    > Created Time: 2017年05月25日 星期四 14时54分01秒
 ************************************************************************/

#include "server.h"
#define DATA_PORT   20
#define LISTEN_PORT 21

int server_check(char* username, char* password)
{
	char user[MAX_SIZE];
	char passwd[MAX_SIZE];
	char buf[MAX_SIZE];

	size_t n, len = 0;
	char *pch, *line = NULL;
	int flag = 0;
	FILE* fd;
	fd = fopen(".passwd", "r");
	if(fd == NULL)
	{
		perror("file not found");
		exit(1);
	}

	//验证用户名和密码是否合法
	while((n = getline(&line, &len, fd)) != -1)
	{
		bzero(buf, sizeof(MAX_SIZE));
		strcpy(buf, line);

		pch = strtok(buf, " ");
		strcpy(user, pch);

		if(pch != NULL)
		{
			pch = strtok(NULL, " ");
			strcpy(passwd, pch);
		}
		int i, str_len = strlen(passwd);
		for(i = 0; i < str_len; i++)
		{
			if(isspace(passwd[i]) || passwd[i] == '\n')
			{
				passwd[i] = 0;
			}
		}

		if((strcmp(user, username) == 0) && (strcmp(passwd, password) == 0))
		{
			flag = 1;
			break;
		}
	}
	free(line);
	fclose(fd);
	return flag;
}
	
int server_login(int sock_fd)
{
	int ret;
	char buf[MAX_SIZE];
	char user[MAX_SIZE];
	char passwd[MAX_SIZE];
	bzero(buf, sizeof(buf));
	bzero(user, sizeof(user));
	bzero(passwd, sizeof(passwd));

	//获取客户端传来的用户名
	if((ret = recv_data(sock_fd, buf, sizeof(buf))) < 0)
	{
		perror("recv user error:");
		exit(1);
	}
	int n = 0, i = 5;
	while(buf[i])
	{
		user[n++] = buf[i++];
	}
	//通知输入密码
	send_response(sock_fd, 331);

	//获取客户端传来的密码
	bzero(buf, sizeof(buf));
	if((ret = (recv_data(sock_fd, buf, sizeof(buf)))) < 0)
	{
		perror("recv passwd error:");
		exit(1);
	}

	i = 5;
	n = 0;
	while(buf[i])
	{
		passwd[n++] = buf[i++];
	}

	return (server_check(user, passwd));
}

int server_get_request(int sock_fd, char *cmd, char *arg)
{
	int ret_code = 200;
	char buf[MAX_SIZE];

	bzero(buf, sizeof(buf));

	//接收客户端命令
	if((recv_data(sock_fd, buf, sizeof(buf))) == -1)
	{
		perror("recv error");
		exit(1);
	}

	strncpy(cmd, buf, 4);
	char *temp = buf + 5;
	strcpy(arg, temp);

	if(strcmp(cmd, "QUIT") == 0)
	{
		ret_code = 221;
	}
	else if((strcmp(cmd, "USER") == 0) || (strcmp(cmd, "PASS") == 0) || (strcmp(cmd, "LIST") == 0) || (strcmp(cmd, "RETR") == 0))
	{
		ret_code = 200;
	}
	else
	{
		ret_code = 502;
	}

	send_response(sock_fd, ret_code);
	return ret_code;
}

int server_work_conn(int sock_fd)
{
	char buf[MAX_SIZE];
	int wait, work_fd;

	if(recv(sock_fd, &wait, sizeof(wait), 0) < 0)
	{
		perror("error while wait");
		exit(1);
	}

	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);
	getpeername(sock_fd, (struct sockaddr*)&client_addr, &len);
	inet_ntop(AF_INET, &client_addr.sin_addr, buf, sizeof(buf));

	//创建到客户端的数据连接
	if((work_fd = connect_server(WORK_PORT, buf)) < 0)
	{
		exit(1);
	}

	return work_fd;
}

int server_cmd_list(int work_fd, int sock_fd)
{
	//读取当前目录
	send_response(sock_fd, 1);
	char data[MAX_SIZE];
	struct dirent* file;
	DIR* direc = opendir("./");
	if(direc == NULL)
	{
		perror("open dir error");
		exit(1);
	}
	int n = 0;
	bzero(data, sizeof(data));
	while((file = readdir(direc)) != NULL)
	{
		sprintf(data + n, "%s\n", file->d_name);
		n += strlen(file->d_name) + 1;
	}
	data[n] = '\0';
	closedir(direc);
	if(send(work_fd, data, strlen(data), 0) < 0)
	{
		perror("send error");
	}
	send_response(sock_fd, 226);
	return 0;
}

void server_cmd_retr(int sock_fd, int work_fd, char *file_name)
{
	//利用mmap映射,然后传送数据
	int ret, file_size;
	int fd = open(file_name, O_RDONLY);
	if(fd < 0)
	{
		send_response(sock_fd, 550);
		perror("open file error");
		exit(1);
	}
	send_response(sock_fd, 150);
	struct stat s;
	ret = fstat(fd, &s);
	if(ret < 0)
	{
		perror("fstat error");
		exit(1);
	}
	file_size = s.st_size;
	char* src = (char *)mmap(NULL, file_size, PROT_READ, MAP_SHARED, fd, 0);
	if(src == MAP_FAILED)
	{
		perror("mmap src file error");
		exit(1);
	}
	if((ret = send(work_fd, src, file_size, 0)) < 0)
	{
		perror("send file error");
		exit(1);
	}
	send_response(sock_fd, 226);
	if((ret = munmap(src, file_size)) < 0)
	{
		perror("munmap error");
		exit(1);
	}
	close(fd);
}

void work_process(int sock_fd)
{
	int work_fd;
	char cmd[10], arg[MAX_SIZE];

	send_response(sock_fd, 220);

	//认证失败
	if(server_login(sock_fd) != 1)
	{
		send_response(sock_fd, 430);
		exit(1);
	}
	//认证成功
	else
	{
		send_response(sock_fd, 230);
	}
	//处理请求
	while(1)
	{
		//接收客户端的请求并解析
		int ret_code = server_get_request(sock_fd, cmd, arg);
		if((ret_code < 0) || (ret_code == 221))
			break;

		if(ret_code == 200)
		{
			//创建与客户端的数据连接,之前的是监听连接
			if((work_fd = server_work_conn(sock_fd)) < 0)
			{
				close(sock_fd);
				exit(1);
			}
			//创建了数据连接之后,执行对应的命令即可
			if(strcmp(cmd, "LIST") == 0)
			{
				server_cmd_list(work_fd, sock_fd);
			}
			else if(strcmp(cmd, "RETR") == 0)
			{
				server_cmd_retr(sock_fd, work_fd, arg);
			}

			close(work_fd);
		}
	}
}
		


int main()
{
	int listen_fd, sock_fd, pid;

	listen_fd = init_server(LISTEN_PORT);
	
	while(1)
	{
		//接收连接
		sock_fd = accept_client(listen_fd);
		//创建子进程
		if((pid = fork()) < 0)
		{
			perror("fork error");
			exit(1);
		}
		else if(pid == 0)	//child process
		{
			close(listen_fd);
			work_process(sock_fd);
			close(sock_fd);
			exit(0);
		}
		close(sock_fd);
	}
	close(listen_fd);
	return 0;
}


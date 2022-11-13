/*************************************************************************
  > File Name: client.c
  > Author: Ukey
  > Mail: gsl110809@gmail.com
  > Created Time: 2017年05月25日 星期四 19时49分35秒
 ************************************************************************/

#include "client.h"
int sock_fd;

int main(int argc, char *argv[])
{
	int work_fd, ret_code, s;
	char buf[MAX_SIZE], port[10], arg[100], code[5];
	struct addrinfo hints, *result, *rp;
	// debug
	// if (argc != 2)
	// {
	// 	printf("Please input client hostname\n");
	// 	exit(1);
	// }

	// char *host = argv[1];
	char host[] = "localhost";

	//获取与主机名匹配的地址
	bzero(&hints, sizeof(struct addrinfo));
	bzero(port, sizeof(port));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	sprintf(port, "%d", LISTEN_PORT);
	if ((s = getaddrinfo(host, port, &hints, &result)) != 0)
	{
		printf("getaddrinfo() error %s", gai_strerror(s));
		exit(1);
	}
	//找到符合要求的服务器地址并连接
	for (rp = result; rp != NULL; rp = rp->ai_next)
	{
		sock_fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sock_fd < 0)
		{
			continue;
		}
		if (connect(sock_fd, result->ai_addr, result->ai_addrlen) == 0)
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
	print_return_code(get_return_code());
	char choice[5];
	while (1)
	{
		printf("**********************************************************************\n");
		printf("To login please press 'l' or 'L', to register please press 'r' or 'R'!\n");
		printf("YOUR CHOICE: ");
		scanf("%s", choice);
		if (choice[0] == 'r' || choice[0] == 'R')
		{
			send(sock_fd, "REGISTER", (int)strlen("REGISTER"), 0);
			fflush(stdin);
			client_register();
			break;
		}
		if (choice[0] == 'l' || choice[0] == 'L')
		{
			send(sock_fd, "LOGIN", (int)strlen("LOGIN"), 0);
			fflush(stdin);
			client_login();
			break;
		}
		else
		{
			printf("Sorry but you have pressed a wrong letter,please choose again!\n");
		}
	}
	while (1)
	{
		//获取到用户输入的命令
		if (client_read_command(buf, sizeof(buf), arg, code) < 0)
		{
			printf("invalid command\n");
			continue;
		}
		//debug
		// printf("\n\nbuf: %s\n", buf);
		// printf("cmd: %s  arg: %s", code, arg);

		//发送命令到服务器
		if (send(sock_fd, buf, (int)strlen(buf), 0) < 0)
		{
			close(sock_fd);
			exit(1);
		}

		ret_code = get_return_code(); //读取服务器响应
		if (ret_code == QUIT_SUCESS)
		{
			print_return_code(QUIT_SUCESS);
			break;
		}
		if (ret_code == CMD_FAIL)
		{
			printf("%d invalid command.\n", ret_code);
		}
		else //命令是合法的
		{
			//打开数据连接
			work_fd = client_open_conn(sock_fd);
			if (work_fd < 0)
			{
				perror("Error opening socket for data connection");
				exit(1);
			}
			//执行命令
			if (strcmp(code, "LS") == 0)
			{
				client_ls(work_fd, sock_fd);
			}
			else if (strcmp(code, "PWD") == 0)
			{
				client_pwd(work_fd, sock_fd);
			}
			else if (strcmp(code, "MKDIR") == 0)
			{
				client_mkdir(work_fd, sock_fd);
			}
			else if (strcmp(code, "CD") == 0)
			{
				client_cd(work_fd, sock_fd);
			}
			else if (strcmp(code, "DELETE") == 0)
			{
				client_delete(work_fd, sock_fd);
			}
			else if (strcmp(code, "GET") == 0)
			{
				if (get_return_code() == FILE_UNVAIL)
				{
					print_return_code(FILE_UNVAIL);		
					continue;
				}
				client_get(work_fd, arg);
				print_return_code(get_return_code());
			}
			else if (strcmp(code, "PUT") == 0) // todo
			{
				if (get_return_code() == FILE_UNVAIL)
				{
					print_return_code(FILE_UNVAIL);
					continue;
				}
				client_get(work_fd, arg);
				print_return_code(get_return_code());
			}
			close(work_fd);
		}
	}
	close(sock_fd);
	return 0;
}

int get_return_code()
{
	int ret_code = 0;
	if (recv(sock_fd, &ret_code, sizeof(ret_code), 0) < 0) 
	{
		perror("client: error reading message from server\n");
		return -1;
	}	
	return ntohl(ret_code);
}
	
void print_return_code(int rc) 
{
	switch (rc)
	{
		case CONN_SUCCESS:
			printf("\n\nWelcome!!! Please login or register first to use the FTP system.\n");
			break;
		case QUIT_SUCESS:
			printf("Good bye!\n");
			break;
		case RET_SUCCESS:
			printf("Return succeeded.\n");
			break;
		case FILE_UNVAIL:
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
	// char *temp_arg = NULL;
	// temp_arg = strtok(buf, " ");
	// temp_arg = strtok(NULL, " ");
	
	// if(temp_arg != NULL)
	// {
	// 	strncpy(arg, temp_arg, strlen(temp_arg) + 1);
	// }
	get_cmd_first_arg(buf, code, arg);
	
	if(strcmp(code, "ls") == 0 || strcmp(code, "LS") == 0)
	{
		strcpy(code, "LS");
	}
	else if(strcmp(code, "pwd") == 0 || strcmp(code, "PWD") == 0)
	{
		strcpy(code, "PWD");
	}
	else if(strcmp(code, "mkdir") == 0 || strcmp(code, "MKDIR") == 0)
	{
		strcpy(code, "MKDIR");
	}
	else if(strcmp(code, "cd") == 0 || strcmp(code, "CD") == 0)
	{
		strcpy(code, "CD");
	}
	else if(strcmp(code, "delete") == 0 || strcmp(code, "DELETE") == 0)
	{
		strcpy(code, "DELETE");
	}
	else if(strcmp(code, "get") == 0)
	{
		strcpy(code, "GET");
	}
	else if (strcmp(code, "put") == 0)
	{
		strcpy(code, "PUT");
	}
	else if(strcmp(code, "quit") == 0)
	{
		strcpy(code, "QUIT");
	}
	else
		return -1;

	bzero(buf, sizeof(buf));
	strcpy(buf, code);
	strcat(buf, " ");
	strncat(buf, arg, strlen(arg) + 1);
	//debug
	//printf("%s", buf);

	return 0;
}

int client_open_conn(int sock_fd)
{
	int listen_fd = init_server(DATA_PORT);
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

int client_get(int work_fd, char *arg)
{
	char data[MAX_SIZE];
	int size;
	printf("filename:%s\n", arg);
	FILE *file = NULL;

	//将服务器传来的数据写入本地建立的文件
	while ((size = recv(work_fd, data, MAX_SIZE, 0)) > 0)
	{
		if (!file)
			file = fopen(arg, "w");
		fwrite(data, 1, size, file);
	}
	if (size < 0)
	{
		perror("error\n");
	}
	fclose(file);
	return 0;
}

int client_ls(int work_fd, int sock_fd) //以这个函数为例
{
	size_t n;
	char buf[MAX_SIZE];
	int temp = 0;
	bzero(buf, sizeof(buf));
	//等待服务器启动的信息
	if((recv(sock_fd, &temp, sizeof(temp), 0)) < 0)//此处接受到SERVER_READY
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
	if (recv(sock_fd, &temp, sizeof(temp), 0) < 0) //此处接受到RET_SUCCESS
	{
		perror("client:error reading message from server\n");
		exit(1);
	}

	return 0;
}

int client_pwd(int work_fd,int sock_fd)
{
	size_t n;
	char buf[MAX_SIZE];
	int temp = 0;
	bzero(buf, sizeof(buf));
	//等待服务器启动的信息

	if((n = recv(sock_fd, &temp, sizeof(temp), 0)) < 0)
	{
		perror("client: error reading message from server\n");
		exit(1);
	}

	//接收服务器传来的信息
	while((n = recv(work_fd, buf, MAX_SIZE, 0)) > 0)
	{
		printf("%s\n", buf);
		bzero(buf, sizeof(buf));
	}

	if(n < 0)
	{
		perror("error");
	}

	//等待服务器完成的消息
	if(n = recv(sock_fd, &temp, sizeof(temp), 0) < 0)
	{
		perror("client:error reading message from server\n");
		exit(1);
	}

	return 0;
}

int client_mkdir(int work_fd,int sock_fd)
{
	size_t n;
	char buf[MAX_SIZE];
	int temp = 0;
	int success = 0;
	bzero(buf, sizeof(buf));
	//等待服务器启动的信息

	if((n = recv(sock_fd, &temp, sizeof(temp), 0)) < 0)
	{
		perror("client: error reading message from server\n");
		exit(1);
	}

	char dir_name[MAX_SIZE];
	fflush(stdout);
	printf("dir name:");
	scanf("%s", dir_name);
	char *temp_arg = NULL;
	temp_arg = strtok(dir_name, " ");
	temp_arg = strtok(NULL, " ");

	//发送文件夹名
	if(send(work_fd, dir_name, (int)strlen(dir_name), 0) < 0)
	{
		perror("client: error sending dir_name from server\n");
		exit(1);
	}

	//等待服务器完成的消息
	if(recv(sock_fd, &success, sizeof(success), 0) < 0)
	{
		perror("client: error reading message from server\n");
		exit(1);
	}
	if(ntohl(success) == 1)
		printf("%s creation succeed\n", dir_name);
	else
		printf("%s creation failed\n", dir_name);

	return 0;
}

int client_cd(int work_fd,int sock_fd)
{
	size_t n;
	char buf[MAX_SIZE];
	int temp = 0;
	int success = 0;
	bzero(buf, sizeof(buf));
	//等待服务器启动的信息

	if((n = recv(sock_fd, &temp, sizeof(temp), 0)) < 0)
	{
		perror("client: error reading message from server\n");
		exit(1);
	}

	char path_name[MAX_SIZE];
	fflush(stdout);
	printf("path name:");
	scanf("%s", path_name);
	char *temp_arg = NULL;
	temp_arg = strtok(path_name, " ");
	temp_arg = strtok(NULL, " ");

	//发送路径
	if(send(work_fd, path_name, (int)strlen(path_name), 0) < 0)
	{
		perror("client: error sending path_name from server\n");
		exit(1);
	}

	//等待服务器完成的消息
	if(recv(sock_fd, &success, sizeof(success), 0)<0)
	{
		perror("client: error reading message from server\n");
		exit(1);
	}
	if(ntohl(success) == 1)
		printf("change to %s succeed\n", path_name);
	else if(ntohl(success) == 0)
		printf("path:%s not exist\n", path_name);
	else
		printf("change to %s failed\n", path_name);

	return 0;
}

int client_delete(int work_fd,int sock_fd)
{
	size_t n;
	char buf[MAX_SIZE];
	int temp = 0;
	int success = 0;
	bzero(buf, sizeof(buf));
	//等待服务器启动的信息

	if((n = recv(sock_fd, &temp, sizeof(temp), 0)) < 0)
	{
		perror("client: error reading message from server\n");
		exit(1);
	}

	char file_name[MAX_SIZE];
	fflush(stdout);
	printf("file name:");
	scanf("%s", file_name);
	char *temp_arg = NULL;
	temp_arg = strtok(file_name, " ");
	temp_arg = strtok(NULL, " ");

	//发送文件名
	if(send(work_fd, file_name, (int)strlen(file_name), 0) < 0)
	{
		perror("client: error sending path_name from server\n");
		exit(1);
	}

	//等待服务器完成的消息
	if(recv(sock_fd, &success, sizeof(success), 0)<0)
	{
		perror("client: error reading message from server\n");
		exit(1);
	}
	if(ntohl(success) == 1)
		printf("delete %s succeed\n", file_name);
	else if(ntohl(success) == 0)
		printf("file:%s not exist\n", file_name);
	else
		printf("delete %s failed\n", file_name);

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
	char arg[100], code[5], user[100], pass[20];
	int wait,ret_code;
	bzero(arg, sizeof(arg));
	bzero(code, sizeof(code));
	bzero(user, sizeof(user));
	while(1){
		//获取用户名
		printf("NAME: ");
		fflush(stdout);
		fflush(stdin);
		scanf("%s",user);

		//发送用户名到服务器
		strcpy(code, "USER");
		strcpy(arg, user);
		client_send_cmd(arg, code);

		//等待应答码,存放在wait
		recv(sock_fd, &wait, sizeof(wait), 0);

		//获取密码
		printf("PASSWORD: ");
		fflush(stdout);
		fflush(stdin);
		scanf("%s", pass);

		//发送密码到服务器
		strcpy(code, "PASS");
		strcpy(arg, pass);
		client_send_cmd(arg, code);

		//等待响应
		ret_code = get_return_code();
		if(ret_code==LOGIN_SUCCESS){
			printf("Login succeed!\n");
			break;
		}
		else if(ret_code==LOGIN_FAILED){
			printf("invalid username/password. Please try again!\n");
			send(sock_fd, "LOGIN", (int)strlen("LOGIN"), 0);
			continue;
		}
		else{
			perror("Error reading message from server");
			exit(1);
			break;
		}
	}

}

void client_register() {
	int i, j;
	char accountName[20], accountPassword[20], accountPasswordCheck[20], pass[20];
	int ret_code;
	int check = 0;
	while (1) {
		printf("Please enter your name for the system less than 20 bytes:\n");
		printf("NAME: ");
		scanf("%s", accountName);
		if (strlen(accountName) > 20) {
			printf("Illegal input!");
			continue;
		}
		else {
			client_send_cmd(accountName, "USER");
			ret_code=get_return_code();
			if(ret_code==REGIST_NAME_REPEAT){
				printf("Your name has existed! Please change one!\n");
				send(sock_fd, "REGISTER", (int)strlen("REGISTER"), 0);
				continue;
			}
			if(ret_code==REGIST_NAME_OK){
				break;
			}
		}
	}
	printf("Your application has been sent, wait a moment for manager to check！\n");
	ret_code=get_return_code();
	if (ret_code==REGIST_APPLICATION_OK) {
		printf("Your application has passed！ \n");
	}
	else if(ret_code==REGIST_REFUSED) {
		printf("Your application has been refused!Please quit the system!\n");
		exit(1);
		system("pause");
		return;
	}
	else{
		printf("system wrong!");
		exit(1);
		return;
	}
	while (1) {
		accountPassword[0]='\0';
		printf("Please enter your password less than 20 bytes.\n");
		//fflush(stdout);
		printf("Password:");
		scanf("%s", pass);
		strcat(accountPassword,pass);
		if (strlen(accountPassword) > 20) {
			printf("Illegal input！\n");
		}
		else break;
	}
	while (1) {
		accountPasswordCheck[0]='\0';
		//fflush(stdout);
		bzero(pass, sizeof(pass));
		printf("Please re-enter your password:");
		scanf("%s", pass);
		strcat(accountPasswordCheck,pass);
		for (i = 0, j = 0;;i++,j++) {
			if (i == strlen(accountPassword) && j == strlen(accountPasswordCheck)) {
				check = 1;
				break;
			}
			else if (accountPassword[i] == accountPasswordCheck[j]) {
				continue;
			}
			else {
				check = 0;
				break;
			}
		}
		if (check==0) {
			printf("Your password entered is different from the last time！\n");
		}
		else break;
	}
	client_send_cmd(accountPassword, "PASS");
	ret_code=get_return_code();
	if (ret_code==REGIST_SUCCESS) {
		printf("Register successfully！ Please login!\n");
		send(sock_fd, "LOGIN", (int)strlen("LOGIN"), 0);
		client_login();
		return;
	}
	else{
		printf("system wrong!");
		exit(1);
		return;
	}
}

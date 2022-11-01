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
			printf("\n\nWelcome!!! Please login or register first to use the FTP system.\n");
			break;
		case 221:
			printf("Good day!\n");
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
	int wait,ret_code;
	char *pass;
	bzero(arg, sizeof(arg));
	bzero(code, sizeof(code));
	bzero(user, sizeof(user));
	while(1){
		//获取用户名
		printf("NAME: ");
		fflush(stdout);
		scanf("%s",user);

		//发送用户名到服务器
		strcpy(code, "USER");
		strcpy(arg, user);
		client_send_cmd(arg, code);

		//等待应答码
		recv(sock_fd, &wait, sizeof(wait), 0);

		//获取密码
		fflush(stdout);
		pass = getpass("Password:");

		//发送密码到服务器
		strcpy(code, "PASS");
		strcpy(arg, pass);
		client_send_cmd(arg, code);

		//等待响应
		ret_code = read_reply();
		if(ret_code==LOGIN_SUCCESS){
			printf("Login succeed!\n");
			break;
		}
		else if(ret_code==LOGIN_FAILED){
			printf("Invaild username/password. Please try again!\n");
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
	char accountName[20], accountPassword[20], accountPasswordCheck[20];
	int ret_code;
	char *pass;
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
			ret_code=read_reply();
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
	ret_code=read_reply();
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
		pass = getpass("Password:");
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
		pass = getpass("Please re-enter your password:");
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
	ret_code=read_reply();
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

int main(int argc, char *argv[])
{
	int work_fd, ret_code, s;
	char buf[MAX_SIZE],  port[10], arg[100], code[5];
	struct addrinfo hints, *result, *rp;
	if(argc != 2)
	{
		printf("Please input client hostname\n");
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
	char choice[5];
	while(1){
		printf("**********************************************************************\n");
		printf("To login please press 'l' or 'L', to register please press 'r' or 'R'!\n");
		printf("YOUR CHOICE: ");
		scanf("%s",choice);
		if(choice[0]=='r'||choice[0]=='R'){
			send(sock_fd, "REGISTER", (int)strlen("REGISTER"), 0);
			fflush(stdin);
			client_register();
			break;
		}
		if(choice[0]=='l'||choice[0]=='L'){
			send(sock_fd, "LOGIN", (int)strlen("LOGIN"), 0);
			fflush(stdin);
			client_login();
			break;
		}
		else{
			printf("Sorry but you have pressed a wrong letter,please choose again!\n");
		}
	}
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






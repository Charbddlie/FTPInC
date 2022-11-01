/*************************************************************************
    > File Name: server.c
    > Author: Ukey
    > Mail: gsl110809@gmail.com
    > Created Time: 2017年05月25日 星期四 14时54分01秒
 ************************************************************************/

#include "server.h"
#define DATA_PORT   20
#define LISTEN_PORT 21
int server_check(char* username, char* password) {
	FILE* file = fopen("../.passwd", "r");
	int i, j;
	int result;
	if (!file) {
		printf("文件打开失败！");
		exit(1);
	}
	char temp[50];
	char name[20];
	while (1) {
		if (feof(file)) {
			result = 0;
			break;
		}
		i = 0;
		j = 0;
		fgets(temp, 49, file);
		while (temp[i] != ',') {
			name[i] = temp[i];
			i++;
		}
		name[i] = '\0';
		if (strcmp(name, username) != 0) {
			continue;
		}
		else {
			char userpassword[20];
			for(i += 1; temp[i] != ','; j++,i++) {
				userpassword[j] = temp[i];
			}
			userpassword[j] = '\0';
			if (strcmp(userpassword, password) != 0) {
				result = 0;
				break;
			}
			else {
				result = 1;
				break;
			}
		}
	}
	fclose(file);
	return result;
}	

void file_write(int sock_fd,char *accountName ,char* au){
	FILE* file = fopen("../.passwd", "a");
	char buf[MAX_SIZE];
	if (!file) {
		printf("File open failed\n");
		exit(-1);
	}
	char accountPassword[20];
	bzero(buf, sizeof(buf));
	recv_data(sock_fd, buf, sizeof(buf));
	int i=5;
	int j=0;
	while(buf[i]&&buf[i]!='\0'){
		accountPassword[j++]=buf[i++];
	}
	char line[50];
	line[0] = '\0';
	strcat(line, accountName);
	strcat(line, ",");
	strcat(line, accountPassword);
	strcat(line, ",");
	strcat(line, au);
	strcat(line, "\n");
	fputs(line, file);
	fclose(file);
	return;
}

int manager_check(int sock_fd,char* accountName,char *au){
	char getAnswer[5];
	printf("A user named %s just sent a register application!\n", accountName);
	while (1) {
		printf("Agree please press 'y' or 'Y'，or press 'N' or 'n' to refuse:");
		scanf("%s", getAnswer);
		if (getAnswer[0] == 'y' || getAnswer[0] == 'Y') {
			printf("Please set permissions for this user：1（can only upload）、2（upload & download）、3（upload & download & change directory）\n");
			while (1) {
				printf("Please enter number（1、2、3）：");
				scanf("%s", getAnswer);
				if (getAnswer[0] == '1' || getAnswer[0] == '2' || getAnswer[0] == '3') {
					au[0]='\0';
					switch(getAnswer[0]){
						case '3':
							strcat(au,"HIGH");
							break;
						case '2':
							strcat(au,"MID");
							break;
						case '1':
							strcat(au,"LOW");
							break;
					}
					break;
				}
				else {
					printf("Illegal input!\n");
				}
			}
			send_response(sock_fd, REGIST_APPLICATION_OK);
			return 1;
		}
		if (getAnswer[0] == 'n' || getAnswer[0] == 'N') {
			return 0;
		}
		else {
			printf("Illegal input!\n");
		}
	}
}

int server_register(int sock_fd){
	int i,ret,result,flag,j;
	char temp[60];
	char name[20];
	char accountName[20];
	FILE* file = fopen("../.passwd", "r");
	if (!file) {
		printf("文件打开失败！");
		exit(1);
	}
	char buf[MAX_SIZE];
	bzero(buf, sizeof(buf));
	recv(sock_fd, buf, sizeof(buf), 0);
	i=5;
	j=0;
	while(buf[i]){
		accountName[j++]=buf[i++];
	}
	accountName[j]='\0';
	while(1){
		if (feof(file)) {
			result = 1;
			break;
		}
		i = 0;
		fgets(temp, 59, file);
		while (temp[i] != ',') {
			name[i] = temp[i];
			i++;
		}
		name[i] = '\0';
		if (strcmp(name, accountName) != 0) {
			continue;
		}
		else {
			result = 0;
			break;
		}
	}
	fclose(file);
	if(result==0){
		return REGIST_NAME_REPEAT;
	}
	if(result==1){
		send_response(sock_fd, REGIST_NAME_OK);
		char au[5];
		ret=manager_check(sock_fd,accountName,au);
		if(ret==0) return 0;
		else {
			file_write(sock_fd,accountName,au);
			return 1;
		}
	}
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
	DIR* direc = opendir("../user_dir/");
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


	int ret;
	char loginOrRegist[MAX_SIZE];

	while(1){
		bzero(loginOrRegist, sizeof(loginOrRegist));
		//login & register?
		if((ret = recv_data(sock_fd, loginOrRegist, sizeof(loginOrRegist))) < 0){
			perror("recv user error:");
			exit(1);
		}
		if(strcmp(loginOrRegist,"REGISTER")==0){
			ret=server_register(sock_fd);
			if(ret==REGIST_NAME_REPEAT){
				send_response(sock_fd, REGIST_NAME_REPEAT);
				continue;
			}
			if(ret == 0){
				send_response(sock_fd, REGIST_REFUSED);
			}
			else{
				send_response(sock_fd, REGIST_SUCCESS);
			}
			continue;
		}
		if(strcmp(loginOrRegist,"LOGIN")==0){
			ret=server_login(sock_fd);
			//认证失败
			if(ret != 1){
				send_response(sock_fd, LOGIN_FAILED);
				continue;
			}
			else{
				send_response(sock_fd, LOGIN_SUCCESS);
				break;
			}
		}
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


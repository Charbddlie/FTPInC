#include "common.h"

int init_server(int port)
{
	int listen_fd, ret;
	struct sockaddr_in serv_addr;

	//创建套接字
	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("create socket error:");
		exit(1);
	}

	//设置端口复用
	int flag = 1;
	ret = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
	if ((ret) < 0)
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

	if ((ret = bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
	{
		close(listen_fd);
		perror("bind error:");
		exit(1);
	}

	//将套接字设为监听状态
	if ((ret = listen(listen_fd, port)) < 0)
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
	if (sock_fd < 0)
	{
		perror("accept error");
		exit(1);
	}
	return sock_fd;
}

int connect_server(int serv_port, char *serv_ip)
{
	int sock_fd, ret;
	struct sockaddr_in serv_addr;

	if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket error:");
		exit(1);
	}

	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(serv_port);
	serv_addr.sin_addr.s_addr = inet_addr(serv_ip);

	if ((ret = connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0)
	{
		perror("error connect:");
		exit(1);
	}

	return sock_fd;
}

int send_num(int sock_fd, int ret_code)
{
	int conv = htonl(ret_code);
	if (send(sock_fd, &conv, sizeof(conv), 0) < 0)
	{
		perror("send ret_code error!");
		return -1;
	}
	return 0;
}

void read_input(char *buf, int size)
{
	char *temp = NULL;
	bzero(buf, size);
	if (fgets(buf, size, stdin))
	{
		temp = strchr(buf, '\n');
		if (temp)
		{
			*temp = '\0';
		}
	}
}
int recv_data(int sock_fd, char *buf, int buf_size)
{
	size_t n;
	bzero(buf, sizeof(buf));

	if ((n = recv(sock_fd, buf, buf_size, 0)) < 0)
	{
		perror("recv error");
		exit(1);
	}
	return n;
}

void get_cmd_first_arg(char *buf, char *cmd, char *arg)
{
	for (int i = 0; i < MAX_SIZE; i++)
	{
		if (buf[i] == ' ')
		{
			int j = i + 1;
			while (buf[j] == ' ')
				j++;
			strcpy(arg, buf + j);
			strncpy(cmd, buf, i);
			cmd[i] = '\0';
			break;
		}
		else if (buf[i] == '\0')
		{
			strcpy(cmd, buf);
			arg = '\0';
			break;
		}
	}
}

int file_name_valid(char *arg, int size)
{
	int dot = 0;
	for (int i = 0; i < size; i++)
	{
		if (arg[i] == '\\' || arg[i] == '/')
			return 0;
	}
	return 1;
}

int send_file(int work_fd, int sock_fd, char *filepath, char *file_name)
{
	if (!file_name_valid(file_name, sizeof(file_name)))
	{
		printf("File name is not allowed to include '/'\n");
		send_num(sock_fd, PATH_FAIL);
		return 0;
	}
	//获取文件后缀名
	int index = -1;
	for (int i = 0; i < sizeof(file_name); i++)
	{
		if (file_name[i] == '.')
			index = i;
	}
	char extern_name[MAX_SIZE];
	strcpy(extern_name, file_name + index);

	// 两种不同打开方式，对应ascii和二进制两种传输方式
	FILE *file = NULL;
	if (strcmp(extern_name, ".txt") == 0||strcmp(extern_name, ".c") == 0||strcmp(extern_name, ".cpp") == 0)
	{
		file = fopen(filepath, "r");
		printf("File will be transfered in ASCII mode\n");
	}
	else
	{
		file = fopen(filepath, "rb");
		printf("File will be transfered in binary mode\n");
	}

	if (file == NULL)
	{
		send_num(sock_fd, FILE_UNVAIL);
		perror("open file error");
		return 0;
	}
	else
	{
		send_num(sock_fd, FILE_VAIL);
	}

	//返回传输总次数
	struct stat s;
	int fd = open(filepath, O_RDWR);
	if (fstat(fd, &s) < 0)
	{
		perror("fstat error");
		send_num(sock_fd, -1);
		return 0;
	}
	else
	{
		send_num(sock_fd, s.st_size / MAX_SIZE + 1);
	}
	close(fd);
	printf("File's size: %ld\n", s.st_size);

	//正式开始传输
	char buf[MAX_SIZE];
	int send_time = 0;
	int size;
	while (!feof(file))
	{
		bzero(buf, sizeof(buf));
		size = fread(buf, 1, MAX_SIZE, file);
		send(work_fd, buf, size, 0);
	}
	fclose(file);
	printf("File successfully sent!\n");
	return 1;
}

int get_file(int work_fd, int sock_fd, char *filepath)
{
	int code = get_return_code(sock_fd);
	if (code == FILE_UNVAIL)
	{
		printf("file doesn't exist!\n");
		return 0;
	}
	else if (code == PATH_FAIL)
	{
		printf("File name is not allowed to include '/'\n");
		return 0;
	}

	int get_num = get_return_code(sock_fd); //文件传输次数
	if (get_num < 0)
	{
		printf("Failed to receive times needed for file transfer\n");
		return 0;
	}
	FILE *file = fopen(filepath, "w");
	int size;
	char data[MAX_SIZE];
	for (int i = 0; i < get_num; i++)
	{
		bzero(data, sizeof(data));
		size = recv(work_fd, data, sizeof(data), 0);
		if (size < 0)
		{
			perror("reading file data error\n");
			return 0;
		}
		fwrite(data, 1, size, file);
	}
	printf("File successfully received!\n");
	fclose(file);
	return 1;
}

int get_return_code(int sock_fd)
{
	int ret_code = 0;
	if (recv(sock_fd, &ret_code, sizeof(ret_code), 0) < 0)
	{
		perror("client: error reading message from server\n");
		return -1;
	}
	return ntohl(ret_code);
}
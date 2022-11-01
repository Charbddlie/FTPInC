# FTP说明

#### 1、使用方法：

1. 在client_linux目录下执行：`make`命令进行编译、`make clean`命令删除可运行文件、`sudo ./client localhost`命令运行客户端程序。
2. 在server_linux目录下执行：`make`命令进行编译、`make clean`命令删除可运行文件、`sudo ./server`命令运行客户端程序。
3. 登录名和密码在`FTPInC`目录下的隐藏文件`.passwd`中。每一条记录包含用户名、密码、权限。

#### 2、避坑

1. send()函数的**buf**长度需要等于recv()函数的**buf**长度，否则下一次会读取到乱码。

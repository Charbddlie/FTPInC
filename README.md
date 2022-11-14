# FTP说明

#### 1、使用方法：

1. 在client_linux目录下执行：`make`命令进行编译、`make clean`命令删除可运行文件、`sudo ./client localhost`命令运行客户端程序。
2. 在server_linux目录下执行：`make`命令进行编译、`make clean`命令删除可运行文件、`sudo ./server`命令运行客户端程序。
3. 登录名和密码在`FTPInC`目录下的隐藏文件`.passwd`中。每一条记录包含用户名、密码、权限。
4. 目前实现指令delete、ls、cd、mkdir、pwd
5. cd（切换当前目录）操作需输入全路径，如切换到user_dir下的test文件夹，输入路径为"/user_dir/test"，或使用"~"基于当前位置进行切换，如在user_dir文件夹，输入路径为"~/test"。
6. mkdir目前仅可用于在当前位置下新建文件夹。
7. delete目前仅可用于在当前位置下删除文件，目的是防止用户删除user_dir，或是在文件夹内删除该文件夹之类的操作。

#### 2、避坑

1. send()函数的**buf**长度需要等于recv()函数的**buf**长度，否则下一次会读取到乱码。
2. 定义新数组千万记得用bzero(char_name, sizeof(char_name))先置0，否则可能会有奇怪的数据。
3. 检测文件路径是否合法（就是检测输入有没有跨文件夹）只在client中实现了，不安全

#### 3、待后续实现功能

1. 目前已有一个基础的list指令可以使用，显示的是user_dir文件夹下的文件，希望change dir命令只能在user_dir文件夹中切换，不能跳出该文件夹。
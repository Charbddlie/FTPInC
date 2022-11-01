# SimpleFTP
在linux下用c实现的一个简单的ftp服务端和客户端

使用方法：
1. 在client目录下执行`make`命令
2. 在server目录下执行`make`命令
3. 分别运行`./server`和`./client 127.0.0.1`
4. 登录名和密码在`server`目录下的隐藏文件`.passwd`中。用户名`test`,密码`testpass`

注意事项：
目前实现的功能很简单,只有`list`和`get file`以及`quit`这三个命令,只是用于学习用途,并不具有太大的实用价值。后面有时间会实现多线程下载及断点重传等功能。

# 2022-Fall-Computer Network-PJ1-Code Document

## 一、组员及分工

|   姓名   |    学号     |                        分工                         | 贡献度 |
| :------: | :---------: | :-------------------------------------------------: | :----: |
| 买巫予骜 | 20300290032 | 实现FTP系统的注册、登录功能，完成代码文档并录制视频 | 33.3%  |
|  陆一杨  | 20300200013 |           实现ls、cd、delete、mkdir等指令           | 33.3%  |
|  彭一博  | 19307130188 |     实现quit、pwd等指令、实现FTP系统的权限管理      | 33.3%  |
|  徐靖文  | 20302010021 |                  实现put、get指令                   | 33.3%  |





## 二、核心功能说明

#### 1、get

- **格式：**get [remote_filename]
- **功能：**Copy file with the name [remote_filename] from remote directory to local directory. 
- **权限：**



#### 2、put

- **格式：**put [local_filename]
- **功能：**Copy file with the name [local_filename] from local directory to remote directory. 
- **权限：**



#### 3、delete

- **格式：**delete [remote_filename]
- **功能：**Delete the file with the name [remote_filename] from the remote directory. 
- **权限：**



#### 4、ls

- **格式：**ls
- **功能：**List the files and subdirectories in the remote directory. 
- **权限：**



#### 5、cd

- **格式：**cd [remote_direcotry_name] or cd ..
- **功能：**Change to the [remote_direcotry_name] on the remote machine or change to the parent directory of the current directory. 
- **权限：**



#### 6、mkdir

- **格式：**mkdir[remote_direcotry_name]
- **功能：**Create directory named [remote_direcotry_name] as the sub-directory of the current working directory on the remote machine. pwd (pwd) – Print the current working directory on the remote machine. 
- **权限：**



#### 7、quit

- **格式：**quit
- **功能：**End the FTP session
- **权限：**





## 三、额外细节实现

#### 1、登录



#### 2、注册



#### 3、权限管理



#### 4、cd命令格式补充



#### 

## 四、异常处理方案

#### 1、用户输入非法

#### 2、用户操作非法
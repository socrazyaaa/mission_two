# 利用多进程，实现服务器与客户端双工通信
建立socket通信，并采用多进程的方式，实现服务器与客户端双工通信。当socket通信建立以后，利用fork()创建子进程，在子进程中，实现对套接字信息的读取；在父进程中，实现向套接字发送信息。
```c
pid_t child_pid = fork();
if(child_pid == -1){
	perror("创建子进程失败！\n");
	exit(1);
}
if(child_pid == 0){
	//子进程：读取来自客户端的信息
	char buf[MAXLEN]={0};
	while(read(client_socket,&buf,MAXLEN)){
		printf("message from client: %s",buf);
		memset(&buf,0,MAXLEN);
	}
	exit(0);
}else{
	//父进程:向客户端发送信息
	char buf[MAXLEN]={0};
	while(fgets(&buf,sizeof(buf),stdin)){
		write(client_socket,&buf,sizeof(buf));
		memset(&buf,0,MAXLEN);
	}
}
```
## 测试运行
服务器端与用户端相互进行信息发送：  
服务器端:
```shell
$./server
启动监听！
连接成功！
message from client: 你好呀，服务器，我是客户端
message from client: 能收到信息吗？
您好，可以收到
message from client: 好的
```

用户端
```shell
$./client
你好呀，服务器，我是客户端
能收到信息吗？
message from server: 您好，可以收到
好的

```




## 一、多进程
### 1.基本概念
进程就是正在内存中运行中的程序，进程的组成部分：进程控制块PCB，数据段，程序段组成。
- 进程控制块(PCB,Process Control Block):PCB其实在内核空间就是一个task_struct结构体，这个结构体记录了进程的所有的信息，例如进程的标号pid,进程的执行状态，进程的使用的内存空间等等。
- 程序段：程序代码存放的位置
- 数据段：程序运行时使用、产生的运算数据。如全局变量、局部变量、宏定义的常量就存放在数据段内。堆、栈、.data、.bss  

**创建进程的头文件及函数原型：**
```c
//头文件
#include<unistd.h>
#include<sys/types>

/*
功能：创建子进程
参数：
        @无
返回值：
        >0 子进程的pid返回给了父进程
        =0 子进程
        -1 失败置位错误码
*/
pid_t fork(void);
pid_t getpid(void);//功能：获取当前进程的pid
pid_t getppid(void);//功能：获取父进程的pid

```
`pid_t` 是一个宏定义`typedef short pid_t;`，实际表示的是内核中的进程表的索引。  
**进程结束的方式有`return` `exit` `_exit`**
- return :用来退出一个函数执行的栈空间，如果把return放在main函数可以具备退出进程的效果，但是如果把return放在子函数return就不能结束一个进程了。
```c
/*
功能：  退出一个进程，刷新缓冲区
头文件：#include <stdlib.h>
参数：
    @status: 成功返回0 ，失败返回负数 
        EXIT_FAILURE  1
        EXIT_SUCCESS  0
返回值：无
*/
void exit(int status);

/*
功能  : 退出一个进程，刷新缓冲区
头文件:  #include <stdlib.h>
参数：
    @status: 成功返回0 ，失败返回负数 
        EXIT_FAILURE  1
        EXIT_SUCCESS  0
返回值：无
*/
void _exit(int status);
```
### 进程资源回收




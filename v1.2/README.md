# 利用socket实现多对多通信

**在此项目中，每一个节点既拥有客户端的功能（向指定ip发送信息），也用有服务器的功能（接收连接该服务端的客户端的信息，并将其转发给连接到该服务端的其余客户端).**
## 目录结构


## 运行
### 1、一个服务器对多个客户端
首先将源文件进行编译，使用了多线程技术，因此在编译的时候需要添加`-lpthread` 选项。运行时应首先启动一个服务器，然后再启动一个拥有客户端和服务端的程序。后续的可执行程序可连接任意节点。  

在`xxx.xxx.xxx.123`的终端，只开启服务器线程：
```shell
$ ./app
Wed Aug 23 15:35:36 2023
xxx.xxx.xxx.124 加入了连接

Wed Aug 23 15:36:26 2023
xxx.xxx.xxx.125 加入了连接

Wed Aug 23 15:36:36 2023
xxx.xxx.xxx.126 加入了连接

Wed Aug 23 15:40:41 2023
xxx.xxx.xxx.126:nice to meet you!
```
在`xxx.xxx.xxx.126`的终端，开启服务器线程和客户端线程。并向服务器发送信息，服务器接受到信息以后，将其转发给其他的客户端：
```shell
$ ./app xxx.xxx.xxx.123
nice to meet you!
```
在`xxx.xxx.xxx.124`终端，开启服务器线程和客户端线程：
```shell
$ ./app xxx.xxx.xxx.123
Wed Aug 23 15:40:41 2023
xxx.xxx.xxx.126:nice to meet you!
```

在`xxx.xxx.xxx.125`终端，开启服务器线程和客户端线程：
```shell
$ ./app xxx.xxx.xxx.123
Wed Aug 23 15:40:41 2023
xxx.xxx.xxx.126:nice to meet you!
```

### 2、向某一个节点的服务器发起连接
除了第一个开启的程序只有服务器线程以外，其余的程序都拥有客户端线程和服务器线程。运行示意如下：`124` 与 `123`建立连接；而`125`和`126` 与`124`建立连接。    
在`xxx.xxx.xxx.123`的终端，只开启服务器线程：
```shell
$ ./app
Wed Aug 23 15:35:36 2023
xxx.xxx.xxx.124 加入了连接

```

在`xxx.xxx.xxx.124`终端，开启服务器线程和客户端线程：
```shell
$ ./app xxx.xxx.xxx.123
Wed Aug 23 15:36:36 2023
xxx.xxx.xxx.125 加入了连接

Wed Aug 23 15:36:36 2023
xxx.xxx.xxx.126 加入了连接

Wed Aug 23 15:40:41 2023
xxx.xxx.xxx.126:nice to meet you!
```

在`xxx.xxx.xxx.125`终端，开启服务器线程和客户端线程：
```shell
$ ./app xxx.xxx.xxx.124
Wed Aug 23 15:40:41 2023
xxx.xxx.xxx.126:nice to meet you!
```
在`xxx.xxx.xxx.126`的终端，开启服务器线程和客户端线程。并向服务器发送信息，服务器接受到信息以后，将其转发给其他的客户端：
```shell
$ ./app xxx.xxx.xxx.124
nice to meet you!
```



## 代码实现
### 客户端类与服务端类
利用面向对象思想，对服务器和客户端类进行封装，具体的实现如下。在客户端对象中，支持向指定服务器发起连接，并创建线程监听客户端；在服务器对象中，支持保存所有建立连接的套接字，支持向所有的套接字转发信息，支持向所有的套接字发送信息。其定义如下：  
`Client.h`:
```c++
class TcpClient{
	public:
		int m_sockfd;             // 与服务器通信的套接字
		int m_port;			      // 端口
		char* m_ip;			      // 服务器IP地址
	public:
		TcpClient();
		TcpClient(int port);	  //初始化端口
		~TcpClient();

		/*
		 *IPv4_verify-验证IPv4 地址是否合法
		 *@ip：需要验证的IP
		 */
		bool IPv4_verify(char* ip);

		/*
		 *Read-读取来自套接字的消息，并将其打印到终端
		 *@arg：socket套接字
		 */
		static void* Read(void* arg);

		/*
		 *Write-获取终端上的输入，并将其发送给套接字
		 *@buf：需要发送的内容
		 *@buf_size：字符串的长度
		 */
		int Write(char* buf,int buf_size);

		/*
		 *ConnectToServer-向服务器 m_ip:m_port 发起连接
		 */
		bool ConnectToServer();
};
```
`server.h`:
```c++
class TcpServer;

//需要发送的数据结构体包含需要发送的套接字、ip
struct Msg{
	int sockfd;  //套接字
	char *ip;	 // ip地址
	TcpServer* ser;
	Msg(int fd,char* str,TcpServer* ser):sockfd(fd),ip(str),ser(ser){}
};

class TcpServer{
	public:
		pthread_rwlock_t sockfd_rwlock;		//读写锁:用于维护连接到服务器的客户端套接字
		int sock_arr_index;					//统计连接到服务器的客户端个数
		int* sockfd_array;					//用于保存连接的文件描述数组 
		int max_client;						//最大连接数

		int m_sockfd;						//启动时的套接字
		int m_port;							//启动监听的端口
		
		static int buf_size;				//发送字符串的大小
	public:
		//无参构造
		TcpServer();
		/*
		 * TcpServer(int port,int max_client) - 设置监听的端口和最大连接数
		 * @port：监听的端口
		 * @max_client：支持最大连接的客户端数
		 */
		TcpServer(int port,int max_client);
		~TcpServer();

		/*
		 *BlindAndListen - 绑定ip和端口，并进入监听状态：socket()->blind()->listen()
		 */
		void BlindAndListen();

		/*
		 *ConnectToClient - 用于循环监听,返回监听到的套接字: accept() ; 返回监听到的套接字，并使用client_ip记录源IP
		 */
		void ConnectToClient(char* client_ip,int* client_sockfd);

		/*
		 *Broadcast - 向ser的套接字数组sockfd_array中的每一个套接字发送buf信息
		 *@sockfd : 不向此套接字发送信息。用于避免回声的现象，sockfd = 0，表示转发到所有的套接字。
		 *@buf  ：需要发送的字符串
		 *@buf_size：字符串长度
		 *@ser：服务器对象指针
		 */
		static void Broadcast(int sockfd,char* buf, int buf_size,TcpServer* ser);

		/*
		 * ReadAndBroadcast - 读取套接字的消息，输入并将其转发到其余的客户端
		 * @arg：Msg 结构体，保存 源信息的ip 和 套接字sockfd 和 TcpServer*服务器对象指针
		 */
		static void* ReadAndBroadcast(void* argv);

		/*
		 *AddSockfd - 向套接字数组中添加客户端套接字。客户端计数 sock_arr_index 加 1
		 *@sockfd:需要插入的socket套接字
		 */
		void AddSockfd(int sockfd);

		/*
		 *DeleteSockfd - 在套接字数组中删除指定套接字。当客户端断开连接后，及时关闭socket套接字。
		 *@sockfd：需要关闭的套接字
		 */
		static void DeleteSockfd(int sockfd,TcpServer* ser);
};
```
### 工作程序
在**客户线程主要实现的任务是：向指定ip地址的客户端进行通信。通过创建一个子线程，分别向套接字读取信息。主线程决定信息发送给谁**。服务器的功能是接收连接该服务端的客户端的信息，并将其转发给连接到该服务端的其余客户端。实现的原理是：**服务器线程不停的监听，当有一个客户端连接进来以后，便将该套接字插入到套接字数组中。然后创建一个线程，用于实现对该套接字的读取。当有信息传入的时候，便将该消息广播到套接字数组中的套接字去**。
具体实现如下：
```c++
#include "client.h"
#include "server.h"

#define PORT 8080		//定义端口
#define MAXCLIENT 100	//定义最大连接数

/*
 *ClientWork-客户端工作线程：向服务器发起连接，并进行双工通信
 **@argv：服务器的IP，由程序启动时传入
 */
void ClientWork(char* ip,TcpClient* cli){
	//1.判断ip地址是否合法
	if(cli->IPv4_verify(ip)){
		//2.创建客户端tcp套接字 用于连接
		cli->m_ip = ip; 
		if(cli->ConnectToServer()){
			pthread_t tid;
			//创建线程
			pthread_create(&tid,NULL,TcpClient::Read,&cli->m_sockfd);
			//分离线程
			pthread_detach(tid);
		}else{
			printf("connect to %s:%d failed\n",cli->m_ip,cli->m_port);
		}
	}else{
		printf("请输入合法的IP地址: 如 ./app 10.82.242.131 \n");
	}
}

/*
 *ServerWork - 开启服务器，并监听连接。当有客户端时，就创建一个线程，用于接受并转发
 *@argv：TcpServer对象 
 */
void* ServerWork(void* argv){
	//构建服务器对象:设置端口和最大连接数
	TcpServer *ser =(TcpServer*) argv;
	
	//监听任意网口
	ser->BlindAndListen();
	
	//输出时间信息
	time_t cur;
	struct tm *timeinfo;

	//获取客户端的 ip和套接字
	char client_ip[15] = {0};
	int client_sockfd = 0;
	while(ser->ConnectToClient(client_ip,&client_sockfd)){
		//连接达上限
		if(ser->sock_arr_index >= ser->max_client){
			char err_msg = "服务器连接已达上限！\n";
			printf("%s",err_msg);
			//向客户端发送提示信息
			write(client_sockfd,err_msg,sizeof(err_msg));
			close(client_sockfd);
			continue;
		}

		//将新的连接添加进套接字数组中
		ser->AddSockfd(client_sockfd);

		//提示有客户端连接
		time(&cur);
		timeinfo = localtime(&cur);
		printf("%s%s 加入了连接\n",asctime(timeinfo),client_ip);

		//创建工作线程
		pthread_t tid;
		pthread_create(&tid,NULL,TcpServer::ReadAndBroadcast,new Msg(client_sockfd,client_ip,ser));
		pthread_detach(tid);

		//清空ip
		memset(client_ip,0,sizeof(client_ip));
	}
	pthread_exit(NULL);
}

void Work(int argc,char *argv[]){
	//实例化服务器对象和客户端对象
	TcpClient *cli = new TcpClient((int) PORT);
	TcpServer *ser = new TcpServer((int) PORT,(int) MAXCLIENT);	

	//需要创建服务器	
	if(argc <= 2){
		pthread_t tid;
		pthread_create(&tid,NULL,ServerWork,ser);
		pthread_detach(tid);
	}

	//创建客户端
	if(argc >=2 )
		ClientWork(argv[1],cli);

	//读取控制台输入
	char buf[512]={0};
	memset(buf,0,sizeof(buf));
	while(fgets(buf,sizeof(buf),stdin)){
		//向服务器发消息:可自定义什么情况下向服务器发消息
		if(cli->m_sockfd > 0)
			cli->Write(buf,sizeof(buf));

		//向所有客户端发消息:编写发送的内容。可自定义什么情况下向服务器发消息
		if(ser->sock_arr_index > 0){
			time_t cur;
			time(&cur);
			struct tm* timeinfo = localtime(&cur);
			char broadcast_buf[1024];
			//拼接时间信息
			sprintf(broadcast_buf,"%smessage for server:   %s",asctime(timeinfo),buf);
			//进行广播
			TcpServer::Broadcast(0,broadcast_buf,sizeof(broadcast_buf),ser);
			memset(broadcast_buf,0,sizeof(broadcast_buf));
		}
		memset(buf,0,sizeof(buf));
	}
	delete cli;
	delete ser;
}

```


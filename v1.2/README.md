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
利用面向对象思想，对服务器和客户端类进行封装，具体的实现如下：
```c++
class TcpClient{
	public:
		int m_sockfd;
	public:
		TcpClient();
		~TcpClient();
		/*
		 *ConnectToServer-向服务器发起连接
		 *
		 *@serverip:服务器IP地址
		 *@port:端口号
		 */
		bool ConnectToServer(const char* serverip,const int port);
};

class TcpServer{
	public:
		int    m_sockfd;                  //服务器创建的套接字
		int    client_sockfd;             //服务端与客户端进行通信的套接字 
		char*  client_ip;                 //连接服务器的客户端ip     
		struct sockaddr_in m_addr;        //服务器sockaddr_in结构体变量   

	public:
		TcpServer();
		~TcpServer();

        /*
		 *ConnectToServer-绑定所有网卡和端口，并进入监听状态
		 *
		 *@port:端口号
         *@max_client:客户端的最大连接数
		*/
		void BlindAndListen(int port,int max_client);
		
		//用于循环监听,返回监听到的套接字
		int ConnectToClient();

};
```
### 客户端线程
在客户线程主要实现的任务是：向指定ip地址的客户端进行通信。通过创建两个线程，分别向套接字读取信息和发送信息。具体实现如下：
```c++
/*
*Read-读取来自套接字的消息，并将其打印到终端
*
*@arg：socket套接字
*/
void* Read(void* arg){
	int sockfd = *(int*) arg;                
	char buf[MAXLEN] = {0}; 
	while(read(sockfd,&buf,MAXLEN)){       
		printf("%s",buf);
		memset(&buf,0,MAXLEN);
	}
	pthread_exit(NULL);
}

/*
*Write-获取终端上的输入，并将其发送给套接字
*
*@arg：socket套接字
*/
void* Write(void* arg){
	int sockfd = *(int *)arg;
	char buf[MAXLEN] ={0};
	while(fgets(buf,sizeof(buf),stdin)){
		write(sockfd,&buf,sizeof(buf));
		memset(&buf,0,sizeof(buf));
	}
	pthread_exit(NULL);
}

/*
*IPv4_verify-验证IPv4 地址是否合法
*
*@ip：需要验证的IP
*/
bool IPv4_verify(char* ip){
	int a,b,c,d;
	char t;
	if(4 == sscanf(ip,"%d.%d.%d.%d%c",&a,&b,&c,&d,&t)){
		if(0 <= a && a <= 255 \
				&& 0 <= b && b <= 255 \
				&& 0 <= c && c <= 255 \
				&& 0 <= d && d <= 255 )
			return true;
	}
	return false;
}
/*
*ClientWork-客户端工作线程：向服务器发起连接，比进行双工通信
*@argv：服务器的IP，由程序启动时传入
*/
void* ClientWork(void* argv){
	// 获取输入参数 ip   
	char* ip = (char*)argv;
	// 判断是否是合法的ip地址
	if(IPv4_verify(ip)){
		// 创建客户端tcp套接字 用于连接
		TcpClient client;
		if(client.ConnectToServer(ip,PORT)){
			pthread_t tid[2];
			//读写线程
			pthread_create(&tid[0],NULL,Read,&client.m_sockfd);
			pthread_create(&tid[1],NULL,Write,&client.m_sockfd);
			//等待线程结束，回收资源
			pthread_join(tid[0],NULL);
			pthread_join(tid[1],NULL);
		}else{
			printf("connect to %s:%d failed\n",ip,PORT);
		}
	}else{
		printf("请输入合法的IP地址: 如 ./app 10.82.242.131 \n");
	} 
	pthread_exit(0); 
}
```
### 服务端线程
服务器的功能是接收连接该服务端的客户端的信息，并将其转发给连接到该服务端的其余客户端。实现的原理是：服务器线程不停的监听，当有一个客户端连接进来以后，便将该套接字保存到一个数组中。然后创建一个线程，用于实现对该套接字的读取。当有信息传入的时候，便将该消息广播到所有的套接字中去。具体实现如下：
```c++
//创建读写锁，用于维护socket数组
pthread_rwlock_t sockfd_rwlock;
//记录连接到服务器的客户端套接字
int sock_arr_index = 0;			//连接到服务器线程的客户端个数
int sockfd_array[MAXCLIENT]={0};//初始化为0

//需要发送的数据结构体包含需要发送的套接字、当前电脑的ip
struct Msg{
	int sockfd;  //套接字
	char *ip;	 // ip地址
	Msg(int fd,char* str):sockfd(fd),ip(str){}
};

/* 
*Broadcast - 对sockfd_array所维护的套接字数组进行发送信息
*
*@sockfd：socket 套接字，表示不对此套接字进行发送信息
*@buf：需要发送的字符串
*@str_size：字符串的大小
*/
void Broadcast(int sockfd,char* buf, int str_size){
	//请求读锁
	pthread_rwlock_rdlock(&sockfd_rwlock);
	//向除了 sockdf 的以外的服务器进行转发
	for(int i = 0 ; i < sock_arr_index; i++){
		if(*(sockfd_array + i) != sockfd && *(sockfd_array + i)  > 0)
			write(*(sockfd_array + i),buf,str_size);
	}
	//释放读锁
	pthread_rwlock_unlock(&sockfd_rwlock);
}

/*
* DeleteSockfd - 删除sockfd_array数组中的 sockfd 这个套接字。当客户端断开连接后，要及时释放套接字
*
*@sockfd：需要删除的套接字
*/
void DeleteSockfd(int sockfd){
	//请求写锁
	pthread_rwlock_wrlock(&sockfd_rwlock);
	for(int i = 0 ; i < sock_arr_index ; i++){
		if(*(sockfd_array + i)  == sockfd){
			close(*(sockfd_array + i));
			//用最后一个文件描述符覆盖这个文件描述符，并将最后一个文件描述符置0;
			sock_arr_index = sock_arr_index - 1;
			*(sockfd_array + i)  = *(sockfd_array + sock_arr_index);
			*(sockfd_array + sock_arr_index)= 0;
			break;
		}
	}
	//释放写锁
	pthread_rwlock_unlock(&sockfd_rwlock);
}

/*
*AddSockfd - 向套接字数组sockfd_array中 添加套接字
*
*@sockfd：socket 套接字
*/
void AddSockfd(int sockfd){
	//请求写锁
	pthread_rwlock_wrlock(&sockfd_rwlock);
    //插入套接字，计数+1
	*(sockfd_array + sock_arr_index) = sockfd;
	sock_arr_index++;
	//释放写锁
	pthread_rwlock_unlock(&sockfd_rwlock);
}

//读取套接字的信息，并进行广播
void* ReadAndBroadcast(void *arg){
	//结构体中保存的是套接字和客户端ip
	struct Msg* msg = (struct Msg*)arg;
	int sockfd      = msg->sockfd;
	char ip[15]     = {0};
	strcpy(ip,msg->ip);

	//收发字符串
	char read_buf[MAXLEN]  = {0};
	char send_buf[2*MAXLEN]= {0};

	//添加时间信息
	time_t cur;
	struct tm *timeinfo;
	time(&cur);
	timeinfo = localtime(&cur);
	printf("%s%s 加入了连接\n",asctime(timeinfo),ip);

	//读取信息
	while(read(sockfd,&read_buf,MAXLEN)){
		//获取时间信息
		time(&cur);
		timeinfo = localtime(&cur);
		printf("%s%s:%s\n",asctime(timeinfo),ip,read_buf);

		//2.转发读到的信息到所有连接到的客户端
		sprintf(send_buf,"%s%s:%s",asctime(timeinfo),ip,read_buf);
		Broadcast(sockfd,send_buf,sizeof(send_buf));
		memset(&read_buf,0,sizeof(read_buf));
		memset(&send_buf,0,sizeof(send_buf));
	}
	time(&cur);
	timeinfo = localtime(&cur);
	printf("%s %s: 断开了连接\n",asctime(timeinfo),ip);

    //关闭套接字
	DeleteSockfd(sockfd);
	delete msg;
	msg = NULL;
	pthread_exit(NULL);
}


//服务器工作线程
void* ServerWork(void* arg){
	//初始化读写锁
	pthread_rwlock_init(&sockfd_rwlock,NULL);

	//创建服务器对象
	TcpServer server;
	server.BlindAndListen(PORT,MAXCLIENT);

    //通信的套接字
	int client_sockfd;

    //循环监听
	while(client_sockfd = server.AcceptClient()){
		if(sock_arr_index >= (int) MAXCLIENT){
			printf("超过最大连接数！\n");
			break;
		}

		//将新的连接添加进套接字数组中
		AddSockfd(client_sockfd);

		//创建工作线程，传入套接字 和 ip
		pthread_t tid;
		pthread_create(&tid,NULL,ReadAndBroadcast,new Msg(server.client_sockfd,server.client_ip));
		pthread_detach(tid);
	}
	//销毁读写锁
	pthread_rwlock_destroy(&sockfd_rwlock);
	pthread_exit(NULL);
}
```

#include "server.h"
int TcpServer::buf_size = 512;


TcpServer::TcpServer(){
	TcpServer(8000,100);
}

TcpServer::TcpServer(int port,int max_client){
	this->m_port     = port;
	this->max_client = max_client;

	//初始化读写锁
	pthread_rwlock_init(&sockfd_rwlock,NULL);

	//初始化套接字数组
	sockfd_array  = (int*) malloc(max_client * sizeof(int));
	memset(sockfd_array,0,max_client * sizeof(int));

	//初始化一些成员变量
	this->m_sockfd	       = 0 ;
	this->sock_arr_index   = 0 ;
}

TcpServer::~TcpServer(){
	//销毁读写锁
	pthread_rwlock_destroy(&sockfd_rwlock);
	//关闭套接字
	if(this->m_sockfd > 0) 
		close(this->m_sockfd);
	//
	free(this->sockfd_array);
	this->sockfd_array = NULL;
}


/*
 *BlindAndListen - 绑定ip和端口，并进入监听状态：socket()->blind()->listen()
 */
void TcpServer::BlindAndListen(){
	//1.创建tcp套接字
	m_sockfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(m_sockfd < 0 ){
		perror("create socket error!\n");
		exit(1);
	}

	//2. 创建sockaddr_in结构体变量
	struct sockaddr_in addr;
	addr.sin_family      = AF_INET;               //IPv4
	addr.sin_port        = htons(m_port);
	addr.sin_addr.s_addr = INADDR_ANY; 			//监听所有网口

	//3.将套接字和IP、端口绑定
	int ret = bind(m_sockfd, (struct sockaddr*)&addr, sizeof(addr));
	if(ret != 0 ){
		perror("bind port error!\n");
		exit(1);
	}

	//4.启动监听
	ret = listen(m_sockfd,max_client); 
	if(ret != 0 ){
		perror("listen error");
		exit(1);
	} 
}  

/*
 *ConnectToClient - 用于循环监听,返回监听到的套接字: accept() ; 返回监听到的套接字，并使用client_ip记录源IP
 */
bool TcpServer::ConnectToClient(char* client_ip,int* client_sockfd){
	//用于记录连接服务器的客户端信息
	struct sockaddr_in client_addr;         
	memset(&client_addr,0,sizeof(client_addr));
	socklen_t addrlen = sizeof(client_addr);

	//建立连接:返回ip和套接字
	*client_sockfd = accept(m_sockfd,(struct sockaddr*) &client_addr,&addrlen);
	strcpy(client_ip,inet_ntoa(client_addr.sin_addr));
	if(*client_sockfd < 0){
		perror("accept failed!");
		exit(1);
	}
	return true;
} 

/*
 *AddSockfd - 向套接字数组中添加客户端套接字。客户端计数 sock_arr_index 加 1
 *@sockfd:需要插入的socket套接字
 */
void TcpServer::AddSockfd(int sockfd){
	//请求写锁
	pthread_rwlock_wrlock(&sockfd_rwlock);
	sockfd_array[sock_arr_index] = sockfd;
	sock_arr_index++;
	//释放写锁
	pthread_rwlock_unlock(&sockfd_rwlock);
}

/*
 *DeleteSockfd - 在套接字数组中删除指定套接字。当客户端断开连接后，及时关闭socket套接字。
 *@sockfd：需要关闭的套接字
 */
void TcpServer::DeleteSockfd(int sockfd,TcpServer* ser){
	//请求写锁
	pthread_rwlock_wrlock(&ser->sockfd_rwlock);
	for(int i = 0 ; i < ser->sock_arr_index ; i++){
		if(ser->sockfd_array[i] == sockfd){
			close(*(ser->sockfd_array + i));
			//用最后一个文件描述符覆盖这个文件描述符，并将最后一个文件描述符置0;
			ser->sock_arr_index = ser->sock_arr_index - 1;
			*(ser->sockfd_array + i)  = *(ser->sockfd_array + ser->sock_arr_index);
			*(ser->sockfd_array + ser->sock_arr_index)= 0;
			break;
		}
	}
	//释放写锁
	pthread_rwlock_unlock(&ser->sockfd_rwlock);
}

/*
 *Broadcast - 向套接字数组sockfd_array中的每一个套接字发送buf信息
 *@sockfd : 不向此套接字发送信息。用于避免回声的现象，sockfd = 0，表示转发到所有的套接字。
 *@buf  ：需要发送的字符串
 *@buf_size：字符串长度
 */
void TcpServer::Broadcast(int sockfd,char* buf, int buf_size,TcpServer* ser){
	//请求读锁
	pthread_rwlock_rdlock(&ser->sockfd_rwlock);
	//向除了 sockdf 的以外的服务器进行转发
	for(int i = 0 ; i < ser->sock_arr_index; i++){
		if(*(ser->sockfd_array + i) != sockfd && *(ser->sockfd_array + i)  > 0)
			write(*(ser->sockfd_array + i),buf,buf_size);
	}
	//释放读锁
	pthread_rwlock_unlock(&ser->sockfd_rwlock);
}

/*
 * ReadAndBroadcast - 读取套接字的消息，输入并将其转发到其余的客户端
 * @arg：Msg 结构体，保存 源信息的ip 和 套接字sockfd
 */
void* TcpServer::ReadAndBroadcast(void *arg){
	//结构体中保存的是套接字和客户端ip
	struct Msg* msg = (struct Msg*)arg;
	int sockfd      = msg->sockfd;
	char ip[15]     = {0};
	strcpy(ip,msg->ip);

	//收发字符串
	char* read_buf = (char*) malloc(buf_size * sizeof(char));
	char* send_buf = (char*) malloc(2 * buf_size * sizeof(char));
	memset(read_buf,0,buf_size);
	memset(send_buf,0,2 * buf_size);

	//时间
	time_t cur;
	struct tm *timeinfo;
	
	//读取信息
	while(read(sockfd,read_buf,buf_size)){
		//获取时间信息
		time(&cur);
		timeinfo = localtime(&cur);
		printf("%s%s:%s\n",asctime(timeinfo),ip,read_buf);

		//2.转发读到的信息到所有连接到的客户端
		sprintf(send_buf,"%s%s:%s",asctime(timeinfo),ip,read_buf);
		Broadcast(sockfd,send_buf,2 * buf_size,msg->ser);

		//3.清0
		memset(read_buf,0,buf_size);
		memset(send_buf,0,2 * buf_size);
	}

	//客户端断开连接
	time(&cur);
	timeinfo = localtime(&cur);
	printf("%s %s: 断开了连接\n",asctime(timeinfo),ip);

	//关闭套接字
	DeleteSockfd(sockfd,msg->ser);
	free(read_buf);
	free(send_buf);
	read_buf = NULL;
	send_buf = NULL;
	msg = NULL;
	pthread_exit(NULL);
}


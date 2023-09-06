#include "server.h"
#include <sys/stat.h>

//int TcpServer::buf_size = 512;

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
	if(this->m_sockfd > 0) close(this->m_sockfd);
	//释放内存
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
	int opt=1;
	setsockopt(m_sockfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
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
			*(ser->sockfd_array + ser->sock_arr_index) = 0;
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
 *getSaveFileName - 找到一个不存在的文件名字,将文件保存在./download。文件夹下
 *@file_name:文件名字
 */
void TcpServer::GetSaveFileName(char* file_name){
	//1.判断download文件夹是否存在
	const char* download_folder="./download";
	struct stat st ={0};
	//若文件夹不存咋，则创建文件
	if(stat(download_folder,&st)== -1 )
		mkdir(download_folder,0777);

	//2.选择一个合适的保存名字，避免重名造成的影响。
	char new_name[150]={0};
	sprintf(new_name,"%s/%s",download_folder,file_name);
	int version = 0;
	//若文件已经存在，则重新取一个名字
	while(access(new_name, F_OK) == 0)
		sprintf(new_name,"%s/%s(%d)",download_folder,file_name,version++);
	memset(file_name,0,strlen(file_name)+1);
	strcpy(file_name,new_name);
}

/*
 *saveFile - 保存套接字传过来的文件
 *@sockfd:套接字
 *@save_path：保存的路径
 *@file_size：文件的大小
 */
void TcpServer::SaveFile(int sockfd,char* save_path,long int file_size){
	//1.创建文件：以写的方式创建二进制文件
	FILE* save_file_ptr = fopen(save_path,"wb+");
	if(save_file_ptr == NULL){
		perror("save file error!");
		exit(1);
	}
	//2.循环读取文件大小的数据
	long int total_receive = 0;
	char buf[(int)RECEIVE_BUF_SIZE]={0};
	int ret_r,ret_w;
	while(total_receive < file_size){
		ret_r = read(sockfd,buf,(int)RECEIVE_BUF_SIZE);	
		if(ret_r < 0){
			perror("read error!");
			exit(1);
		}
		ret_w = fwrite(buf,ret_r,1,save_file_ptr);
		if(ret_w != 1){
			perror("fwrite error!");
			exit(1);
		}
		total_receive += ret_r;
		printf("write %d bytes,total write %ld/%ld bytes\n",ret_r,total_receive,file_size);
	}
	fclose(save_file_ptr);
}

/*
 *SaveFileformClient - 接受套接字发送的文件
 *@sockfd：套接字信息
 */
void TcpServer::SaveFilefromClient(int sockfd){
	//1.读取客户端发过来的文件信息：名字和大小
	char file_msg[150]={0};
	int ret  = read(sockfd,&file_msg,sizeof(file_msg));
	if(ret < 0){
		perror("read message failed");
		exit(1);
	}
	//获取文件信息及大小:例如客户端发来的消息为  filemsg,util.h,5540
	char *file_info[3];
	char* ptr = strtok(file_msg,",");
	for(int i=0;i<3;i++){
		file_info[i] = ptr;
		ptr = strtok(NULL,",");
	}
	//2.开始写入文件
	if(strcmp(file_info[0],"filemsg") == 0){
		long int file_size = atol(file_info[2]);	//文件大小
		printf("start recevie file\nfile name:%s \tfile size:%ld\n",file_info[1],file_size);
		GetSaveFileName(file_info[1]);				//获取一个保存路径
		SaveFile(sockfd,file_info[1],file_size);	//sockfd从读取file_size的内容，保存到file_info[1]路径下
		printf("file has been saved to %s successfully\n",file_info[1]);
	}
	pthread_exit(NULL);
}

void* TcpServer::Read(void *arg){
	//结构体中保存的是套接字和客户端ip
	struct Msg* msg = (struct Msg*)arg;
	int sockfd      = msg->sockfd;
	char ip[15]     = {0};
	strcpy(ip,msg->ip);

	//收发字符串
	char read_buf[(int) CHAT_BUF_SIZE] = {0};
	char send_buf[2 * (int)CHAT_BUF_SIZE] = {0};

	//时间
	time_t cur;
	struct tm *timeinfo;

	//读取信息
	int ret = 0;
	while((ret = read(sockfd,read_buf,(int) CHAT_BUF_SIZE)) > 0){
		if(strlen(read_buf) == 0)	continue;
		if(ret == -1){
			perror("read failed!\n");
			exit(0);
		}
		if((strlen(read_buf) >= 12) && strncmp(read_buf,"fileTransmit",12) == 0){
			SaveFilefromClient(sockfd);
		}else{
			//获取时间信息
			time(&cur);
			timeinfo = localtime(&cur);
			printf("%s%s:%s",asctime(timeinfo),ip,read_buf);

			//2.转发读到的信息到所有连接到的客户端
			sprintf(send_buf,"%s%s:%s \n",asctime(timeinfo),ip,read_buf);
			//printf("要进行转发的消息\"%s\"\n",send_buf);
			Broadcast(sockfd,send_buf,2 * (int)CHAT_BUF_SIZE,msg->ser);
		}
		//3.清0
		memset(read_buf,0,(int)CHAT_BUF_SIZE);
		memset(send_buf,0,2 * (int)CHAT_BUF_SIZE);
	}

	//客户端断开连接
	time(&cur);
	timeinfo = localtime(&cur);
	printf("%s---------\t%s 断开了连接\t-----------\n",asctime(timeinfo),ip);

	//关闭套接字
	DeleteSockfd(sockfd,msg->ser);
	delete msg;
	msg = NULL;
	pthread_exit(NULL);
}


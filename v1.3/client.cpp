#include "client.h"
#include<sys/stat.h>

TcpClient::TcpClient():m_sockfd(0),m_port(8000){};
TcpClient::TcpClient(int port):m_sockfd(0),m_port(port){};
TcpClient::~TcpClient(){
	if( this->m_sockfd != 0) 
		close(this->m_sockfd);
}

/*
 *IPv4_verify-验证IPv4 地址是否合法
 *@ip：需要验证的IP
 */
bool TcpClient::IPv4_verify(char* ip){
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
 *ConnectToServer-向服务器发起连接
 *@serverip:服务器IP地址
 *@port:端口号
 */
bool TcpClient::ConnectToServer(){
	//1.创建套接字
	this->m_sockfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(this->m_sockfd == -1){
		perror("create socket failed!\n");
		exit(1);
	}

	//设置一些属性
	struct sockaddr_in server_addr;
	memset(&server_addr,0,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port   = htons(m_port);
	server_addr.sin_addr.s_addr = inet_addr(m_ip);

	//3.连接到服务器
	int ret = connect(this->m_sockfd,(struct sockaddr*)&server_addr,sizeof(server_addr));
	if(ret < 0){
		perror("connect to server failed!\n");
		exit(1);
	}
	return true;
}

/*
 *读取来自套接字的消息，并将其打印到终端：
 *@arg：客户端对象
 */
void* TcpClient::Read(void* argv){
	//1.获取客户端对象
	TcpClient* cli = (TcpClient *)argv;

	//2.读取套接字内的内容
	int buf_size = 512;
	char *buf = (char*) malloc(buf_size * sizeof(char));
	memset(buf,0,buf_size);     
	while(true){ 
		int ret = read(cli->m_sockfd,buf,buf_size);
		if(ret == 0 ) break;
		if(ret == -1){
			perror("dadada read failed!\n");
			exit(1);
		}   
		printf("%s",buf);
		memset(buf,0,buf_size);
	}
	//3.服务器断开连接
	printf("服务器已断开!");
	free(buf);
	buf = NULL;
	cli->m_sockfd = 0;
	pthread_exit(NULL);
}

/*
 *Write-获取终端上的输入，并将其发送给套接字
 *@buf：需要发送的内容
 *@buf_size：字符串的长度
 */
int TcpClient::Write(const char* buf,int buf_size){
	return write(m_sockfd,buf,buf_size);
}


/*
 *UploadFileInfo - 告诉服务器需要发送的文件 和 大小
 *
 *@full_file_name：含有路径的文件名
 */
long TcpClient::UploadFileInfo(char* full_file_name){
	//step1:打开文件:以只读的方式打开文件
	FILE* fd;
	if((fd = fopen(full_file_name,"rb")) == NULL){
		printf("file doesn't exist!");
		exit(1);
	}

	//step2:获取文件的大小:通过调用fseek()函数来将文件指针移动到文件尾部，然后使用ftell()函数来获取文件大小。
	fseek(fd, 0, SEEK_END);			//文件结尾
	long file_size = ftell(fd);
	fseek(fd, 0, SEEK_SET);			//文件开头

	//step3:发送文件名字和大小
	char* file_name = basename(full_file_name);//获取文件名字如client.cpp, 头文件include<libgen.h>
	char buf[150]={0};
	sprintf(buf,"filemsg,%s,%ld",file_name,file_size);
	int ret = this->Write(buf,sizeof(buf));
	if(ret < 0){
		perror("send file information failed!");
		exit(1);
	}

	//step4:关闭文件
	fclose(fd);
	printf("send file information successed! start file transmit....\n");
	return file_size;
} 

/*
 *UploadFile()-向服务器 上传文件;先发送 filemsg,<file_name>,<file_size> 再接字
 *
 *@full_file_name:需要上传的文件名字   hom  ./download/app
 */
void TcpClient::UploadFile(char* full_file_name){
	//step0:发送信息：文件名字，大小
	long file_size = UploadFileInfo(full_file_name);

	//step1:打开文件:以只读的方式打开文件
	FILE* read_file_ptr = nullptr;
	if((read_file_ptr = fopen(full_file_name,"rb")) == NULL){
		printf("Fail to open \n");
		exit(1);
	}

	//setp2:读取数据并发送
	char buf[(int)SEND_BUF_SIZE] = {0};
	int read_count;
	int total_write = 0;
	while((read_count = fread(buf,1,(int)SEND_BUF_SIZE,read_file_ptr)) > 0){
		//检查读是否正确
		if((read_count < (int)SEND_BUF_SIZE) && ferror(read_file_ptr)){
			perror("fread error!");
			fclose(read_file_ptr);
			exit(1);
		}
		//检查发送是否正确
		int ret = this->Write(buf,read_count);
		if(ret == -1){
			perror("write error!");              
			fclose(read_file_ptr);
			exit(1);
		}
		total_write += ret;
		printf("send %d bytes,total send %d/%ld bytes\n",ret,total_write,file_size);
		memset(buf,0,(int)SEND_BUF_SIZE);
	}
	//step4:关闭文件
	fclose(read_file_ptr);
}

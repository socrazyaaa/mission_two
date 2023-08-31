#include "client.h"

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
*@arg：套接字
*/
void* TcpClient::Read(void* argv){
	//1.获取套接字
	int sockfd = *(int *)argv;

	//2.读取套接字内的内容
	int buf_size = 512;
	char *buf = (char*) malloc(buf_size * sizeof(char));
	memset(buf,0,buf_size);
	int ret = 0;          
	while(ret = read(sockfd,buf,buf_size)){ 
		if(ret == -1){
			perror("read failed!\n");
			exit(1);
		}   
		printf("%s",buf);
		memset(buf,0,buf_size);
	}
	free(buf);
	buf = NULL;
	pthread_exit(NULL);
}

/*
 *Write-获取终端上的输入，并将其发送给套接字
 *@buf：需要发送的内容
 *@buf_size：字符串的长度
 */
int TcpClient::Write(char* buf,int buf_size){
	return write(m_sockfd,buf,buf_size);
}	


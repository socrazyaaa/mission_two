#include "client.h"
#include "server.h"

#define PORT 8088	//定义端口
#define MAXCLIENT 3	//定义最大连接数
#define FGETS_SIZE 512

/*
 *ClientWork-客户端工作线程：向服务器发起连接，并进行双工通信
 *@ip：服务器的IP，由程序启动时传入
 *@cli：客户端对象
 */
void ClientWork(char* ip,TcpClient* cli){
	//1.判断ip地址是否合法
	if(cli->IPv4_verify(ip)){
		//2.创建客户端tcp套接字 用于连接
		cli->m_ip = ip; 
		if(cli->ConnectToServer()){
			pthread_t tid;
			//创建线程
			pthread_create(&tid,NULL,TcpClient::Read,cli);
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
	//1.构建服务器对象:设置端口和最大连接数
	TcpServer *ser =(TcpServer*) argv;

	//2.监听任意网口
	ser->BlindAndListen();
	int client_sockfd = 0;
	char client_ip[15] = {0};

	//输出时间信息
	time_t cur;
	struct tm *timeinfo;

	//3.获取客户端的 ip和套接字
	while(ser->ConnectToClient(client_ip,&client_sockfd)){
		//判断是否达到连接上限
		if(ser->sock_arr_index >= ser->max_client){
			printf("超过最大连接数！\n");
			//向客户端发送提示信息
			char buf[] ="服务器连接已达上限\n";
			write(client_sockfd,buf,sizeof(buf));
			close(client_sockfd);
			continue;
		}
		//将新的连接添加进套接字数组中
		ser->AddSockfd(client_sockfd);
		//提示有客户端连接
		time(&cur);
		timeinfo = localtime(&cur);
		printf("%s---------\t%s 加入了连接\t-----------\n",asctime(timeinfo),client_ip);
		//创建工作线程
	    pthread_t tid;
		pthread_create(&tid,NULL,TcpServer::Read,new Msg(client_sockfd,client_ip,ser));
		pthread_detach(tid);
		//清空ip
		memset(client_ip,0,sizeof(client_ip));
	}
	pthread_exit(NULL);
}


void Work(int argc,char *argv[]){
	//1.实例化服务器对象和客户端对象
	TcpClient *cli = new TcpClient((int) PORT);
	TcpServer *ser = new TcpServer((int) PORT,(int) MAXCLIENT);

	//2.创建客户端，运行方式： $ ./app 127.xxx.xxx.xxx
	if(argc >= 2)	
    ClientWork(argv[1],cli);

	//3.创建服务器，运行方式：	$ ./app 或者 $ ./app 127.0.0.1
	if(argc <= 2){
		pthread_t tid;
		pthread_create(&tid,NULL,ServerWork,ser);
		pthread_detach(tid);
	}
  printf("请按以下格式输入需要执行的任务\n@server:xxxxx\t@client:xxxxx\t@fileTransfer:xxxxx\n");
	//4.读取控制台输入,并进行相应的操作
	char buf[(int)FGETS_SIZE]={0};//@server:xxxxx   @client:xxxxx    @fileTransfer:xxxxxx
	while(fgets(buf,sizeof(buf),stdin)){
		char* ptr = strtok(buf,":");

		//向服务器上传文件
		if((strlen(ptr) == 13) && (strncmp(ptr,"@fileTransfer",13) == 0)){
			if(cli->m_sockfd <= 0){
				printf("未连接到服务器\n");
				continue;
			}
			ptr = strtok(NULL,":");
            char file_name[100]={0};
            strncpy(file_name,ptr,strlen(ptr)-1);
			cli->Write("fileTransmit",13);
			cli->UploadFile(file_name);
			printf("%s文件传输完成！\n",file_name);
		}

		//向服务器发送消息
		if((strlen(ptr) == 7) && (strncmp(ptr,"@server",7)==0)){
			//执行向服务器发送任务
			if(cli->m_sockfd <= 0){
				printf("未连接到服务器\n");
				continue;
			}
			ptr = strtok(NULL,":");
			cli->Write(ptr,strlen(ptr)+1);
		}

		//向客户端发送信息
		if((strlen(ptr) == 7) && (strncmp(ptr,"@client",7)==0)){
			if(ser->sock_arr_index <= 0){
				printf("没有客户端连接到本服务器\n");
				continue;
			}
			//拼接时间信息
			time_t cur;
			time(&cur);
			struct tm* timeinfo = localtime(&cur);
			char broadcast_buf[512] = {0};
      		ptr = strtok(NULL,":");
			sprintf(broadcast_buf,"%sserver broadcast:%s\n",asctime(timeinfo),ptr);
			//进行广播
			TcpServer::Broadcast(0,broadcast_buf,sizeof(broadcast_buf),ser);
			memset(broadcast_buf,0,sizeof(broadcast_buf));
		}
		memset(buf,0,sizeof(buf));
	}
	delete cli;
	delete ser;
}


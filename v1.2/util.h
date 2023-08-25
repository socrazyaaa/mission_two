#include "client.h"
#include "server.h"

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
	int client_sockfd = 0;
	char client_ip[15] = {0};
	
	//输出时间信息
	time_t cur;
	struct tm *timeinfo;

	//获取客户端的 ip和套接字
	while(ser->ConnectToClient(client_ip,&client_sockfd)){
		if(ser->sock_arr_index >= ser->max_client){
			printf("超过最大连接数！\n");
			break;
		}
		//将新的连接添加进套接字数组中
		ser->AddSockfd(client_sockfd);

		//提示有客户端连接
		time(&cur);
		timeinfo = localtime(&cur);
		printf("%s%s 加入了连接 %d\n",asctime(timeinfo),client_ip,client_sockfd);

		//创建工作线程
		pthread_t tid;
		pthread_create(&tid,NULL,TcpServer::ReadAndBroadcast,new Msg(client_sockfd,client_ip,ser));
		pthread_detach(tid);

		//清空ip
		memset(client_ip,0,sizeof(client_ip));
	}
	exit(0);
}


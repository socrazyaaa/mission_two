#include "util.h"
#include "client.h"
#include "server.h"

#define PORT 8080
#define MAXCLIENT 100

int main(int argc,char* argv[]){
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
		//向服务器发消息
		cli->Write(buf,sizeof(buf));

		//向所有客户端发消息:编写发送的内容
		if(ser->sock_arr_index > 0){
			time_t cur;
			time(&cur);
			struct tm* timeinfo = localtime(&cur);
			char broadcast_buf[1024];

			sprintf(broadcast_buf,"%s message for server:%s",asctime(timeinfo),buf);
			TcpServer::Broadcast(0,broadcast_buf,sizeof(broadcast_buf),ser);
		}
		memset(buf,0,sizeof(buf));
	}
	delete cli;
	delete ser;
	return 0;
}

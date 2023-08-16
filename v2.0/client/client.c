#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

int main(int argc,char **argv){
	//1. 创建socket 套接字
	int client_socket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(client_socket < 0 ){
		printf("error: create client_socket failed !\n");
		return 1;
	}

	//2.设置服务器 IP和端口
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port   = htons(8000);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	//3.连接服务器
	int ret = connect(client_socket,(struct sockaddr*)&addr,sizeof(addr));
	if(ret < 0 ){
		printf("error:connect to serve failed!\n");
		return 0;
	}


	//6.创建子进程，用于与此连接进行双工通信
	pid_t child_pid = fork();
	if(child_pid == -1){
		perror("创建子进程失败！\n");
		exit(1);
	}

	if(child_pid == 0){
		//子进程：读取来自客户端的信息
		char buf[512]={0};
		while(read(client_socket,&buf,512)){
			printf("message from server: %s",buf);
			memset(&buf,0,512);
		}
		exit(0);
	}else{
		//父进程:向客户端发送信息
		char buf[512]={0};
		while(fgets(buf,sizeof(buf),stdin)){
			write(client_socket,&buf,sizeof(buf));
			memset(&buf,0,512);
		}
	}
	//5.关闭套接字
	close(client_socket);
	return 0;
}

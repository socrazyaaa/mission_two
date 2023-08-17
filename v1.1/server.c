#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <fcntl.h>        //close()
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>

#define MAXLEN 512
#define MAXCLIENT 10




int main(int argc,char **argv){
	//1.创建tcp套接字
	int serv_socket = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(serv_socket < 0 ){
		perror("create socket error!\n");
		exit(1);
	}

	//2. 创建sockaddr_in结构体变量
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));                 //全部置0
	addr.sin_family      = AF_INET;               //IPv4
	addr.sin_port        = htons(8000);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //监听网口

	//3.将套接字和IP、端口绑定
	int ret = bind(serv_socket, (struct sockaddr*)&addr, sizeof(addr));
	if(ret != 0 ){
		perror("bind port error!\n");
		exit(1);
	}

	//4.启动监听
	printf("启动监听！\n");
	ret = listen(serv_socket,MAXCLIENT);            //最大支持10个连接
	if(ret != 0 ){
		perror("listen error");
		exit(1);
	}    

	//记录服务器的信息的结构体
	struct sockaddr_in client_addr;         //用于记录连接服务器的客户端信息
	memset(&client_addr,0,sizeof(client_addr));
	socklen_t addrlen = sizeof(client_addr);

	//5.建立连接
	int client_socket = accept(serv_socket,(struct sockaddr*) &client_addr,&addrlen);
	if(client_socket < 0){
		perror("accept failed!");
		exit(1);
	}
	printf("连接成功！\n");

	//6.创建子进程，用于与此连接进行双工通信
	pid_t child_pid = fork();
	if(child_pid == -1){
		perror("创建子进程失败！\n");
		exit(1);
	}
	if(child_pid == 0){
		//子进程：读取来自客户端的信息
		char buf[MAXLEN]={0};
		while(read(client_socket,&buf,MAXLEN)){
			printf("message from client: %s",buf);
			memset(&buf,0,MAXLEN);
		}
		exit(0);
	}else{
		//父进程:向客户端发送信息
		char buf[MAXLEN]={0};
		while(fgets(&buf,sizeof(buf),stdin)){
			write(client_socket,&buf,sizeof(buf));
			memset(&buf,0,MAXLEN);
		}
		printf("父进程退出！\n");
	}

	//7.关闭套接字
	close(client_socket);

	return 0; 
}

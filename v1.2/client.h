#ifndef _TCPCLIENT_H
#define _TCPCLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>  //memset()
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>
//#include <signal.h>

class TcpClient{
	public:
		int m_sockfd;			// 与服务器通信的套接字
		int m_port;			// 端口
		char* m_ip;			// 服务器IP地址
	public:
		TcpClient();
		TcpClient(int port);		//初始化端口
		~TcpClient();

		/*
		 *IPv4_verify-验证IPv4 地址是否合法
		 *
		 *@ip：需要验证的IP
		 */
		bool IPv4_verify(char* ip);

		/*
		 *Read-读取来自套接字的消息，并将其打印到终端
		 *
		 *@arg：socket套接字
		 */
		static void* Read(void* arg);

		/*
		 *Write-获取终端上的输入，并将其发送给套接字
		 *
		 *@buf：需要发送的内容
		 *@buf_size：字符串的长度
		 */
		int Write(char* buf,int buf_size);

		/*
		 *ConnectToServer-向服务器 m_ip:m_port 发起连接
		 *
		 */
		bool ConnectToServer();

};


#endif   //_TCPCLIENT_H

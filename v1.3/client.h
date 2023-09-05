#ifndef _TCPCLIENT_H
#define _TCPCLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>  //memset()
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>//stat()
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>
#include<libgen.h>	//basename()

#define SEND_BUF_SIZE 	4*1024		//传输文件，每次发送4k的数据

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
		 *@arg：客户端对象
		 */
		static void* Read(void* arg);

		/*
		 *Write-获取终端上的输入，并将其发送给套接字
		 *
		 *@buf：需要发送的内容
		 *@buf_size：字符串的长度
		 */
		int Write(const char* buf,int buf_size);

		/*
		 *ConnectToServer-向服务器 m_ip:m_port 发起连接
		 */
		bool ConnectToServer();
		
		/*
		*UploadFileInfo - 告诉服务器 文件名字 及 文件大小
		*
		*@full_file_name：含有路径的文件名 C:\Users\Administrator\Desktop\最终版本\v1.2\client.cpp
		*返回值：文件的大小
		*/
		long UploadFileInfo(char* full_file_name);

		/*
		*UploadFile()-向服务器 上传文件
		*
		*@full_file_name:需要上传的文件名字   
		*/
		void UploadFile(char* full_file_name);

};
#endif   //_TCPCLIENT_H

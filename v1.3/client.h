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
#include<libgen.h>	//basename()

#define FILE_NAME_LEN 100
#define BUF_SIZE 	4*1024		//传输文件，每次发送4k的数据

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
		 */
		bool ConnectToServer();

		/*
		*UploadFile()-向服务器 上传文件
		*
		*@file_name:需要上传的文件名字
		*@file_path:文件的绝对路径
		*/
		void UploadFile(char* file_path,char* file_name);
		
		/*
		*UploadFileInfo - 告诉服务器 文件名字 及 文件大小
		*
		*@full_file_name：含有路径的文件名 C:\Users\Administrator\Desktop\最终版本\v1.2\client.cpp
		*/
		void UploadFileInfo(char* full_file_name);

		/*
		*UploadFile()-向服务器 上传文件
		*
		*@full_file_name:需要上传的文件名字   
		*/
		void UploadFile(char* full_file_name);

};
#endif   //_TCPCLIENT_H


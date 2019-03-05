/*---------------------------------------
 * File Name：WorkSever.h
 * Author: wangbao
 * Mail: 1564676944
 * Create Time: 2018 Mon Nov 26,4:36 PM
-----------------------------------------*/
/*---------------------------------------
 * 1.懒汉式单例模式--double检查。
 * 2.锁机制
 * 3.提供的接口：getWorkServer()
 * 4.成员变量：1连接服务器请求TCPconnect；2发送数据sendToSer；3接受数据recvToSera
 ---------------------------------------*/
#ifndef WORKSER_H
#define WORKSER_H
#include<iostream>
#include<string>
#include<json/json.h>
#include<errno.h>
#include"TCPConnect.h"

//mutex mymutex;
class WorkSever
{
		public:
				static WorkSever* getWorkSever()
				{
					if(_myWorkSever == NULL)
					{
						//	mymutex.lock();
							if(_myWorkSever == NULL)
							{
									_myWorkSever = new WorkSever;
							}
						//	mymutex.unlock();
					}
					return _myWorkSever;

				}
				/*
				//更换服务器
				bool ChangeWorkSer()
				{
						close(_ser->GetserFD());
						_ser->Connect(_balanceIP,_balancePort);

						string str;
						_ser->Recv(str);

						Json::Value val;//发送Json包
						Json::Reader read;//接受Json包

						if(-1 == read.parse(str.c_str(),val))
						{
								cerr<<"WorkSever::ChangeWorkSever()->read.parse()is fail;errno: "<<errno<<endl;
								return;
						}

						string ip = val["IP"].asString();
						unsigned short port = val["port"].asInt();

						close(_ser->GetserFD());
						_ser->Connect(IPnew,portnew);

						return true;
				}
				*/
				bool SendToWorkSer(const string& message)
				{
					return _ser->Send(message);//send函数实际发送的长度
				}

				bool RecvFromWorkSer(string& message)
				{
						if(0 >= _ser->Recv(message))
						{
								return false;
								//重新选择一台服务器，负载均衡。
						}
						return true;
				}


		private:
				WorkSever()//私有的构造函数
				{
						cout<<"输入服务器的IP："<<endl;
						cin>>root_ser_IP;
						cout<<"输入服务器的port:"<<endl;
						cin>>root_ser_port;
						_ser = new TCPConnect();
						_ser->Connect(root_ser_IP,root_ser_port);//建立连接
						//当连接服务器成功后，服务器回返回一个确认信息

						string str;//用于接受这个确认信息？？
						//_ser->Recv(str);
						//这个确认信息也没有输出啊，？？？


						
						/*//负载均衡器发送给客户端重新连接的服务器？
						Json::Value val;//发送Json包
						Json::Reader read;//接受Json包

						if(-1 == read.parse(str.c_str(),val))
						{
								cerr<<"WorkSever::WorkSever()->read.parse()is fail;errno: "<<errno<<endl;
								return;
						}

						string IPnew = val["IP"].asString();
						unsigned short portnew = val["port"].asInt();

						close(_ser->GetserFD());
						_ser->Connect(IPnew,portnew);
						*/
				}
				string root_ser_IP;
				unsigned root_ser_port;
				static WorkSever* _myWorkSever;

				TCPConnect* _ser;//服务器连接的端口

};
WorkSever* WorkSever::_myWorkSever = NULL;


#endif

/*---------------------------------------------
 * File Name   : main_pthread.h
 * Mail        : 1564676944@qq.com
 * Author      : wangbao
 * Create Time : 2018 sat Dec 8, 6:53 PM
 * Function    ：create pthread_pool,start listen.
 * -------------------------------------------*/
#ifndef MAIN_PTHREAD_H
#define MAIN_PTHREAD_H

#include"pthread_pool.h"
#include"TCPaccept.h"
#include<sys/socket.h>
#include<arpa/inet.h>
#include<assert.h>
#include<unistd.h>
#include<string>
#include<string.h>
#include<iostream>
using namespace std;


class Mpthread
{
		public:
				Mpthread(){

						//启动libevent
						_base = event_base_new();
						//启动线程
						cout<<"     how many pthreads do you want to create?"<<endl;
						int num=0;
						cin>>num;
						_mypool = new Pthreadpool(num,_base);
						_ser = new TCPaccept("127.0.0.1",6500);//用来建立服务器的IP和port，先使用默认值

				}
				static void ListenCb(int fd,short event,void *arg)
				{
						TCPaccept* ser = static_cast<TCPaccept*>(arg);
						int clifd = ser->Accept();//客户端套接字
						if(-1 == clifd)
						{
								cerr<<"cli accept fail;errno:"<<errno<<endl;
								return;
						}
						//获取到客户端套接字clifd
						cout<<"     cli connect succesful,clifd = "<<clifd<<endl;

						_mypool->SendToWorkPthread(clifd);//交给线程池
				}
				void Run()
				{
						//添加事件
						struct event *listenEvent = event_new(_base,_ser->GetlistenFD(),EV_READ|EV_PERSIST,Mpthread::ListenCb,_ser);
						if(NULL == listenEvent)
						{
								cerr<<"     Mpthread listenEvent create fail;errno:"<<errno<<endl;
						}
						//事件添加到监听队列中
						event_add(listenEvent,NULL);
						cout<<"     Server Listenevents creatint successfully!"<<endl;
						event_base_dispatch(_base);//循环监听
						cout<<"     Server libevent start!"<<endl;
				}
		private:
				static TCPaccept* _ser;
				static Pthreadpool* _mypool;
				static event_base *_base;
};
event_base* Mpthread::_base = NULL;
Pthreadpool* Mpthread::_mypool = NULL;
TCPaccept* Mpthread::_ser = NULL;
#endif

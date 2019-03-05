/*-------------------------------------------------------
 * File Name: SelectSeverPthread.h
 * Author: wangbao 
 * Mail: 1564676944@qq.com
 * Create Time: 2018 Sat Dec 1, 14:40 PM
 * -----------------------------------------------------*/
#ifndef SELECTSEVERPTHREAD_H
#define SELECTSECERPTHREAD_H

#include<iostream>
#include"TCPSever.h"
#include"WorkSever.h"
#include<pthread.h>
#include<event.h>
#include"public.h"
using namespace std;

class SelectSeverThread
{
		public:
				//监听套界字的回调函数
				static void ListenCb(int fd,short event,void *arg)
				{
						TCPSever * sever = static_cast<TCPSever*>(arg);
						int clifd = sever->Accept();
						if(-1 == clifd)
						{
								cerr<<"selecter cli accept is fail;errno:"<<errno<<endl;
								return;
						}
						struct event* cliEvent = event_new(_base,clifd,EV_READ|EV_PERSIST,SelectSeverThread::CliCb,sever);
						if(NULL == cliEvent)
						{
								cerr<<"select ser threa cli event new fail;errno:"<<errno<<endl;
								return;
						}
						_eventMap.insert(make_pair(clifd,cliEvent));
						event_add(cliEvent,NULL);
				}
				//客户端套接字的回调函数
				static void CliCb(int fd,short event,void* arg)
				{
						
						TCPSever * sever = static_cast<TCPSever*>(arg);

						string id;
						if(0 >= sever->Recv(fd,id))
						{
								cerr<<fd<<" select cli shot down;errno:"<<errno<<endl;
								event_free(_eventMap[fd]);
								_eventMap.erase(fd);
								close(fd);
								return;
						}
						cout<<fd<<" select mechine gets new message is:"<<id<<endl;

						Json::Value val;
						val["type"] = SELECT;
						val["fd"] = fd;
						val["message"] = id.c_str();

						//发送给工作服务器
						WorkSever::getWorkSever()->SendToWorkSer(val.toStyledString());
				}
				//启动线程循环监听
				static void* SelectThreadRun(void* arg)
				{
						cout<<"Select run---"<<endl;
						event_base_dispatch(_base);
				}
				SelectSeverThread()
				{
						//创建分布式信息采集服务器
						unsigned short port;
						cout<<"please input select port:"<<endl;
						cin>>port;

						_selecterSer = new TCPSever("127.0.0.1",port);
						cout<<"message selecter'ser create successfully"<<endl;

						//libevent
						_base = event_base_new();

						//创建监听事件
						struct event *listenEvent = event_new(_base,_selecterSer->GetListenFD(),EV_READ|EV_PERSIST,SelectSeverThread::ListenCb,_selecterSer);
						if(NULL == listenEvent)
						{
								cerr<<"select ser listenevent create fail;errno:"<<errno<<endl;
								return;

						}
						event_add(listenEvent,NULL);
						cout<<"selecter listenevent create successfully"<<endl;

						//启动线程进行循环监听
						pthread_t pid;
						pthread_create(&pid,NULL,SelectSeverThread::SelectThreadRun,_base);
				}

				//发送数据给信息采集机
				void SendSelect(int fd,string &str)
				{
					_selecterSer->Send(fd,str);
				}

		private:
				TCPSever* _selecterSer;//分布式信息采集机的服务器
				static event_base *_base;//libevent
				static map<int,struct event*>_eventMap;
};

event_base * SelectSeverThread::_base = NULL;
map<int,struct event*> SelectSeverThread::_eventMap;




#endif

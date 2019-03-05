/*------------------------------------------
 * File Name: workpthread.h
 * Author : wangbao
 * Mail: 1564676944@qq.com
 * Create Time: 2018 Sun Dec 9, 10:03 AM
 * ------------------------------------------*/
#ifndef WORKPTHREAD
#define WORKPTHREAD
#include<iostream>
using namespace std;
#include<errno.h>
#include<event.h>
#include<map>
#include"Contral.h"
class Workpthread
{
		public:
				Workpthread(int sockpair1)
				{
						_contral = new Contral();
						_base = event_base_new();
						_cliEventmap = new map<int,struct event*>();
						//监听客户端连接，进程业务处理。
						struct event* sockpair1Event = event_new(_base,sockpair1,EV_READ|EV_PERSIST,Workpthread::Sockpair1Cb,this);//直接将this指针传进去

						if(NULL == sockpair1Event)
						{
								cout<<"socket pair 1 event create fail;errno:"<<errno<<endl;
								return;
						}
						event_add(sockpair1Event,NULL);

						//定时器事件
						create_timeout_event(_base,sockpair1);//1端代表 连接线程那端

						Run();

				}
				void create_timeout_event(struct event_base* base,const int sockpair1)
				{
						struct event *ev;
						struct timeval timeout;

						//ev = evtimer_new(base,do_timeout,NULL);
						ev = event_new(base,-1,EV_PERSIST,Workpthread::do_timeout,&sockpair1);
						if(ev)
						{
								timeout.tv_sec = 30;//30秒更新一次
								timeout.tv_usec = 0;//毫秒
								event_add(ev,&timeout);//第二个参数相当与定时事件
						}
				}
				void do_timeout(evutil_socket_t fd,short event,void* arg)
				{
						int* sockpair1 = static_cast<int*>(arg);
						send(*sockpair1,(char*)&_numofcli,4,0);
						//给连接线程那端发送该业务处理线程
				}
				void Run()
				{
						pthread_t pid;
						pthread_create(&pid,NULL,Workpthread::Pthreadrun,_base);
				}
				static void *Pthreadrun(void *arg)
				{
						cout<<"Workpthread "<<pthread_self()<<" run"<<endl;

						struct event_base* base = static_cast<struct event_base*>(arg);
						event_base_dispatch(base);
				}

				static void Sockpair1Cb(int fd,short event,void * arg)
				{
						Workpthread* mypthread = static_cast<Workpthread*>(arg);
						_numofcli+=1;//连接数量
						int clifd;
						recv(fd,(char*)&clifd,4,0);
						cout<<"     recv clifd is"<<clifd<<endl;

						struct event* cliEvent = event_new(mypthread->_base,clifd,EV_READ|EV_PERSIST,cliCb,arg);

						if(NULL == cliEvent)
						{
								cout<<"     cliEvent fail"<<endl;
								return;
						}
						event_add(cliEvent,NULL);

						mypthread->_cliEventmap->insert(make_pair(clifd,cliEvent));//?

				}
				static void cliCb(int fd,short event ,void *arg)
				{

						Workpthread* mypth = static_cast<Workpthread*>(arg);

					    char str[100]={0};
						
								
								int flage = recv(fd,str,100,0);//接受到这个json 包

								if(flage <= 0)
								{
										cerr<<"     cli down clifd:"<<fd<<";"<<endl;
										event_free((*mypth->_cliEventmap)[fd]);
										mypth->_cliEventmap->erase(fd);
										mypth->_numofcli-=1;
										close(fd);
								}
								cout<<str<<endl;
								_contral->process(fd,str);//mypth是workpthread的指针，直接进行的是控制台的处理函数
				
			}
				
		private:
					struct event_base* _base;
					static Contral* _contral;
					std::map<int,struct event*>* _cliEventmap;
					static int _numofcli;//本文件可见

};

Contral* Workpthread::_contral = NULL;
int Workpthread::_numofcli = 0;



#endif

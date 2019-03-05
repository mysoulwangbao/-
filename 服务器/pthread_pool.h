/*-----------------------------------
 * File Name: pthread_pool.h
 * Author : wangbao
 * Mail: 1564676944@qq.com
 * Create Time: 2018 Sun Dec 9, 9:54 AM
 * --------------------------------------------*/
#ifndef PTHREAD_POOL_H
#define PTHREAD_POOL_H
#include<iostream>
#include"workpthread.h"
#include<map>
#include<sys/socket.h>
#include<event.h>
#include<stdlib.h>
#include<pthread.h>
using namespace std;
pthread_mutex_t mymutex;
class Pthreadpool
{
	public:
			Pthreadpool(const int num ,struct event_base *base)
			{
					CreateSomePthread(num,base);
					cout<<"     pthread pool creating ---"<<endl;
					pthread_mutex_init(&mymutex,NULL);//初始化//成功返回0
			}

			void CreateSomePthread(const int num,struct event_base *base)//线程池创建业务处理线程。
			{
					int pipefd[2] = {0};//socketpair进程间交互的工具          
					for(int i = 0;i<num;++i)
					{
							int res = socketpair(AF_UNIX,SOCK_STREAM,0,pipefd);

							if(res == -1)
							{
									cout<<"     pthreadpool CreateThread fail;errno:"<<errno<<endl;
									return;
							}
							//创建事件，有连接事件发生发送给工作线程
							struct event* socketpair0Event = event_new(base,pipefd[0],EV_READ|EV_PERSIST,RecvEventNumFromThread,&_sockpairMap);//
							if(socketpair0Event == NULL)
							{
									cout<<"     socketpair[0] Create fail;errno;"<<errno<<endl;
									exit(0);
							}

							event_add(socketpair0Event,NULL);

							cout<<"     socket pair "<<i<<" Create successfully"<<endl;
							cout<<"     socket pair "<<i<<" 0 event create successfully"<<endl;
							//_sockpairMap.insert(pair<int,int>(pipefd[0],0));
							_sockpairMap.insert(make_pair(pipefd[0],0));//连接数的维护表

							new Workpthread(pipefd[1]);//构造业务处理线程	
					}
			}
			void SendToWorkPthread(int clifd)//分配clifd给线程池中的线程
			{
					map<int,int>::iterator iter = _sockpairMap.begin();
					map<int,int>::iterator iter_tmp = iter ;
					int min = iter->second;
					for(;iter != _sockpairMap.end();iter++)
					{
							if(iter->second < min)
							{
									iter_tmp = iter;
							}
					}
					if(-1 == send(iter_tmp->first,(char*)&clifd,4,0))
					{
							cout<<"send is errno"<<endl;
							return;
					}
					iter_tmp->second++;

					cout<<"     send to pthread falge: "<<iter_tmp->first<<endl;
			}

			static void RecvEventNumFromThread(int fd, short event,void* arg)//这fd就是sockpair[0]端?
			{
					map<int,int>* _mymap = static_cast<map<int,int>*>(arg);

					int num;
					recv(fd,(char*)&num,4,0);//接受到工作线程发送的cli的num

					map<int,int>::iterator it = (*_mymap).begin();
					for(;it!=(*_mymap).end();it++)
					{
							if(it->first == fd)
							{
									pthread_mutex_lock(&mymutex);
									it->second = num;
									pthread_mutex_unlock(&mymutex);
							}
					}

			}
	private:
		
			Workpthread* _workpthread;
			static map<int,int>_sockpairMap;//连接数量的map
			static int _pthreadrunnum;
};
map<int,int>Pthreadpool::_sockpairMap;//sockpair[1],num
int Pthreadpool::_pthreadrunnum = 0;




#endif

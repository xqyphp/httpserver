//
//  event_linux.c
//  klib
//
//  Created by LiKai on 16/1/8.
//  Copyright ? 2016Äê LiKai. All rights reserved.
//
#ifdef linux

#include "event.h"
#include <sys/epoll.h>
#include <assert.h>

#define BUFFER_LEN 1024


#define DO_CALL_BACK(bclose) if(manager->callback != K_NULL)  \
{\
        result = manager->callback(data_ptr);\
        if((result == -1 || bclose) && clientfd !=-1)\
        {\
                close(clientfd);\
                clientfd = -1;\
                data_ptr->clientfd = -1;\
                if(data_ptr->event_type != CLINET_CLOSE)\
                {\
                          socket_event_close(data_ptr);\
                          manager->callback(data_ptr);\
                }\
        }\
}



//todo only support one client case ev is only one
//next put it to an array
static void socket_event_init(socket_event_t* ev)
{
        ev->event_type = EVENT_NONE;
		ev->listenfd = -1;
        ev->clientfd = -1;
        ev->data_len = 0;
        ev->data_ptr = K_NULL;
        ev->user_data = K_NULL;
}

static void socket_event_connect(socket_event_t* ev,int clientfd)
{
        ev->event_type = CLIENT_CONNECT;
        ev->clientfd = clientfd;
        ev->data_len = 0;
        ev->data_ptr = K_NULL;
}

static void socket_event_read(socket_event_t* ev,void* data_ptr,int data_len)
{
        ev->event_type = CLIENT_READ;
        ev->data_len = data_len;
        ev->data_ptr = data_ptr;
}


static void socket_event_close(socket_event_t* ev)
{
        ev->event_type = CLINET_CLOSE;
}

status_t
event_manager_init(event_manager_t* manager,int listenfd,event_callback callback)
{
        manager->epfd = epoll_create(256);
        manager->server_socket = listenfd;
		manager->ev_data.listenfd = listenfd;
        manager->callback = callback;
        struct epoll_event ev;
        ev.data.ptr = &(manager->ev_data);
        ev.events = EPOLLIN|EPOLLET;
        epoll_ctl( manager->epfd,EPOLL_CTL_ADD,listenfd,&ev);
        return K_SUCCESS;
}

status_t event_dispatch(event_manager_t* manager)
{
        int result;
        struct sockaddr serveraddr,clientaddr;
        int clientfd;
        int i,n,clilen;
        struct epoll_event ev;
        struct epoll_event events[20];
        socket_event_t event_data[20];
        socket_event_t* data_ptr = K_NULL;
        char buffer[BUFFER_LEN] = "Hello,Client!";

        for( i = 0; i < 20; i++)
        {
                socket_event_init(&event_data[i]);
        }

        while(K_TRUE)
        {
                int nfds = epoll_wait(manager->epfd,events,20,500);
                for(i = 0; i < nfds; i++){
                        if((&(manager->ev_data)) == events[i].data.ptr)
                        {
                                //printf("event->new client\n");
                                clientfd = accept(manager->server_socket,
                                        &clientaddr,&clilen);
                                assert(clientfd >= 0);

                                data_ptr = &event_data[i];
                                socket_event_connect(data_ptr, clientfd);

                                DO_CALL_BACK(K_FALSE);

                                ev.data.ptr = &event_data[i];
                                ev.events = EPOLLET| EPOLLIN;
                                epoll_ctl(manager->epfd,EPOLL_CTL_ADD, clientfd,&ev);
                        }else if(events[i].events & EPOLLIN)
                        {
                                //printf("event->read\n");
                                data_ptr = (socket_event_t*)events[i].data.ptr;
                                clientfd = data_ptr->clientfd;
                                if(clientfd < 0)
                                        continue;
                                while(K_TRUE)
                                {
                                        if(clientfd < 0)
                                                break;
                                        n = read(clientfd,buffer,BUFFER_LEN);
                                        if(n == 0)
                                        {
                                                socket_event_close(&event_data[i]);
                                                DO_CALL_BACK(K_TRUE);
                                        }else if(n < 0)
                                        {
                                                if(errno == EAGAIN || errno == EWOULDBLOCK)
                                                {
                                                        continue;
                                                }else
                                                {
                                                        break;
                                                }
                                        }else
                                        {
                                                socket_event_read(data_ptr,buffer,n);
                                                DO_CALL_BACK(K_FALSE);
                                        }

                                }

                        }else if (events[i].events & EPOLLOUT)
                        {
                                printf("event->write!\n");
                                socket_event_t* data_ptr = (socket_event_t*)events[i].data.ptr;
                                clientfd = data_ptr->clientfd;
                                write(clientfd,buffer,n);
                        }else{
                                printf("event->unknow!\n");
                        }
                }
        }

}

status_t event_magager_destroy(event_manager_t* manager)
{
        close(manager->epfd);
        close(manager->server_socket);
}

#endif

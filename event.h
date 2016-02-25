//
//  event.h
//  klib
//
//  Created by LiKai on 16/1/8.
//  Copyright ? 2016Äê LiKai. All rights reserved.
//
#ifndef event_h
#define event_h
#include "types.h"
#include "socket.h"


enum event_type_t
{
	EVENT_NONE,
	CLIENT_CONNECT,
	CLIENT_READ,
	CLINET_CLOSE
};

typedef struct socket_event_s
{
	enum event_type_t event_type;
	socket_t clientfd;
	socket_t listenfd;
	size_t data_len;
	void*  data_ptr;
	void*  user_data;

}socket_event_t;

typedef int (*event_callback)(socket_event_t* data);


typedef struct event_manager_s
{
#ifdef linux
	int epfd;
	socket_event_t ev_data;
#else
	HANDLE completionPort;
#endif
	socket_t server_socket;
	event_callback callback;
}event_manager_t;

#ifdef linux
#else
#include <WinSock2.h>
#include <Windows.h>
#pragma comment(lib, "Kernel32.lib")
#endif

socket_t
server_create(const char* hostname, int portnumber);

void
server_listen(socket_t serverfd);

socket_t
client_create(const char* hostname, int portnumber);

status_t
event_manager_init(event_manager_t* event_manager,socket_t serverfd, event_callback callback);

status_t
event_dispatch(event_manager_t* event_manager);

status_t
event_magager_destroy(event_manager_t* manager);

#endif

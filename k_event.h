//
//  event.h
//  klib
//
//  Created by LiKai on 16/1/8.
//  Copyright ? 2016Äê LiKai. All rights reserved.
//
#ifndef event_h
#define event_h
#include "k_types.h"
#include "k_socket.h"


enum event_type_t
{
	EVENT_NONE,
	CLIENT_CONNECT,
	CLIENT_READ,
	CLINET_CLOSE
};

struct socket_event_s
{
	enum event_type_t event_type;
	k_socket_t clientfd;
	k_socket_t listenfd;
	size_t data_len;
	void*  data_ptr;
	void*  user_data;

};

struct event_manager_s
{
#ifdef linux
	int epfd;
	socket_event_t ev_data;
#else
	HANDLE completionPort;
#endif
	k_socket_t server_socket;
	event_callback callback;
};

#ifdef linux
#else
#include <WinSock2.h>
#include <Windows.h>
#pragma comment(lib, "Kernel32.lib")
#endif


k_status_t
event_manager_init(event_manager_t* event_manager, k_socket_t serverfd, event_callback callback);

k_status_t
event_dispatch(event_manager_t* event_manager);

k_status_t
event_magager_destroy(event_manager_t* manager);

#endif

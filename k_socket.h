#ifndef k_socket_h
#define k_socket_h

#include "k_types.h"

#include <errno.h>
#include <stdio.h>

#ifdef linux

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include<unistd.h>

typedef int                  k_socket_t;
typedef socklen_t            k_socklen_t;


#else

#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

typedef SOCKET               k_socket_t;
typedef int                  k_socklen_t;

#endif

typedef struct	sockaddr  k_sockaddr_t;


k_status_t
k_socket_init();

k_status_t
k_socket_destory();

k_errno_t
k_socket_errno();

k_socket_t
k_socket(int domain, int type, int protocol);

k_status_t
k_bind(k_socket_t sock_fd, k_sockaddr_t* my_addr, int addr_len);

k_status_t
k_connect(k_socket_t sock_fd, k_sockaddr_t *serv_addr, int addrlen);

k_status_t
k_listen(k_socket_t sock_fd, int backlog);

k_status_t
k_accept(k_socket_t sock_fd, k_sockaddr_t* addr, k_socklen_t* addrlen);

k_size_t
k_send(k_socket_t fd, const void *buf, k_size_t nbyte, int flags);

k_size_t
k_recv(k_socket_t fd,  void *buf, k_size_t nbytes, int flags);

k_status_t 
k_close(k_socket_t sock_fd);

int
k_setnonblocking(k_socket_t sockfd);

k_socket_t
k_client_create(const char* hostname, int portnumber);


k_socket_t
k_server_create(const char* hostname, int portnumber);

k_status_t
k_server_listen(k_socket_t serverfd);

#endif

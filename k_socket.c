#include "k_socket.h"
#include "k_types.h"
#include <string.h>        // for bzero
#include <assert.h>
#ifndef linux
#include <Ws2tcpip.h>

void* bzero(void*  buf, size_t size)
{
	return memset(buf, 0, size);
}

int inet_aton(const char *string, struct in_addr *addr)
{
	return inet_pton(AF_INET, string, addr);
}
#endif

#define LISTENQ 20

k_status_t
k_socket_init()
{
#ifdef linux
	//noting to do
	return K_SUCCESS;
#else
	WORD wVersionRequested;
	WSADATA wsaData;

	wVersionRequested = MAKEWORD(2, 2);

	// 0 is success
	return WSAStartup(wVersionRequested, &wsaData);
#endif
}

k_status_t
k_socket_destory()
{
#ifdef linux
	//noting to do
#else
	return WSACleanup();
#endif
	return K_SUCCESS;
}

k_errno_t
k_socket_errno()
{
#ifdef linux
	return errno;
#else
	return WSAGetLastError();
#endif
}

k_socket_t
k_socket(int domain, int type, int protocol)
{
	
#ifdef linux
	//noting to do
#else
	
#endif
	return socket(domain, type, protocol);
}

k_status_t
k_bind(k_socket_t sock_fd, k_sockaddr_t* my_addr, int addr_len)
{
	return bind(sock_fd, my_addr, addr_len);
}

k_status_t
k_connect(k_socket_t sock_fd, k_sockaddr_t *serv_addr, int addrlen)
{
	return connect(sock_fd, serv_addr, addrlen);
}

k_status_t
k_listen(k_socket_t sock_fd, int backlog)
{
	return listen(sock_fd, backlog);
}

k_status_t
k_accept(k_socket_t sock_fd, k_sockaddr_t* addr, k_socklen_t* addrlen)
{
	return accept(sock_fd, addr, addrlen);
}

k_size_t
k_send(k_socket_t fd, const void *buf, k_size_t nbytes, int flags)
{
	return send(fd, buf, nbytes, flags);
}

k_size_t
k_recv(k_socket_t fd, void *buf, k_size_t nbytes, int flags)
{
	return recv(fd, buf, nbytes,flags);
}

k_status_t
k_close(k_socket_t sock_fd)
{
#ifdef linux
	return close(sock_fd);
#else
	return closesocket(sock_fd);
#endif
}


int
k_setnonblocking(k_socket_t sockfd)
{
#ifdef linux
	int opts;
	opts = fcntl(sockfd, F_GETFL);
	assert(opts >= 0);

	opts |= O_NONBLOCK;
	opts = fcntl(sockfd, F_SETFL);
	assert(opts >= 0);
#else
	int opt = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
#endif
	return K_SUCCESS;
}


k_socket_t
k_client_create(const char* hostname, int portnumber)
{
	struct sockaddr_in client_addr;
	bzero(&client_addr, sizeof(client_addr));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = htons(INADDR_ANY);
	client_addr.sin_port = htons(0);

	int client_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (client_socket < 0)
	{
		int error = k_socket_errno();
		printf("Create Socket Failed->%d!\n", error);
		return K_ERROR;
	}

	if (bind(client_socket, (struct sockaddr*)&client_addr, sizeof(client_addr)))
	{
		printf("Client Bind Port Failed!\n");
		return K_ERROR;
	}

	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;

	if (inet_aton(hostname, &server_addr.sin_addr) == 0)
	{
		printf("Server IP Address Error!\n");
		return K_ERROR;
	}
	server_addr.sin_port = htons(portnumber);
	socklen_t server_addr_length = sizeof(server_addr);

	if (connect(client_socket, (struct sockaddr*)&server_addr, server_addr_length) < 0)
	{
		printf("Can Not Connect To %d!\n", portnumber);
		return K_ERROR;
	}
	return client_socket;
}


k_socket_t
k_server_create(const char* hostname, int portnumber)
{
	struct sockaddr_in server_addr;
	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htons(INADDR_ANY);
	server_addr.sin_port = htons(portnumber);

	int sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		printf("Create Socket Failed!");
		return K_ERROR;
	}

	if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)))
	{
		printf("Server Bind Port : %d Failed!", portnumber);
		return K_ERROR;
	}
	return sockfd;
}

k_status_t
k_server_listen(k_socket_t serverfd)
{

	if (k_listen(serverfd, LISTENQ))
	{
		printf("Server Listen Failed!");
		return K_ERROR;
	}
	return K_SUCCESS;
}

#ifndef linux
#include "k_event.h"
#include "k_socket.h"
#include <Ws2tcpip.h>

typedef struct
{
	OVERLAPPED overlapped;
	WSABUF databuff;
	char buffer[2014];
	int BufferLen;
	int operationType;
}PER_IO_OPERATEION_DATA, *LPPER_IO_OPERATION_DATA, *LPPER_IO_DATA, PER_IO_DATA;


typedef struct
{
	SOCKADDR_STORAGE ClientAddr;
	event_manager_t*    manager;
	socket_event_t socket_event;
}PER_HANDLE_DATA, *LPPER_HANDLE_DATA;


static DWORD WINAPI ServerWorkThread(LPVOID IpParam);

k_status_t
event_manager_init(event_manager_t* event_pool, k_socket_t server_socket, event_callback callback)
{
	HANDLE completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (NULL == completionPort) {
		return K_ERROR;
	}
	event_pool->completionPort = completionPort;
	event_pool->server_socket = server_socket;
	event_pool->callback = callback;

	return K_SUCCESS;
}

k_status_t
event_magager_destroy(event_manager_t* manager)
{
	manager->callback = K_NULL;
	CloseHandle(manager->completionPort);
	k_close(manager->server_socket);
	return K_SUCCESS;
}


#define BUFFER_LEN 1024


#define DO_CALL_BACK(bclose) if(manager->callback != K_NULL)  \
{\
        result = manager->callback(data_ptr);\
        if((result == -1 || bclose) && data_ptr->clientfd !=-1)\
        {\
                k_close(data_ptr->clientfd);\
                data_ptr->clientfd = -1;\
                if(data_ptr->event_type != CLINET_CLOSE)\
                {\
                          socket_event_close(data_ptr);\
                          manager->callback(data_ptr);\
                }\
				continue;\
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

static void socket_event_connect(socket_event_t* ev, int clientfd)
{
	ev->event_type = CLIENT_CONNECT;
	ev->clientfd = clientfd;
	ev->data_len = 0;
	ev->data_ptr = K_NULL;
}

static void socket_event_read(socket_event_t* ev, void* data_ptr, int data_len)
{
	ev->event_type = CLIENT_READ;
	ev->data_len = data_len;
	ev->data_ptr = data_ptr;
}


static void socket_event_close(socket_event_t* ev)
{
	ev->event_type = CLINET_CLOSE;
}


k_status_t  event_dispatch(event_manager_t* manager)
{
	int result;
	socket_event_t* data_ptr = K_NULL;
	SYSTEM_INFO mySysInfo;
	GetSystemInfo(&mySysInfo);

	for (DWORD i = 0; i < (mySysInfo.dwNumberOfProcessors * 2); ++i) {
		HANDLE ThreadHandle = CreateThread(NULL, 0, ServerWorkThread, manager->completionPort, 0, NULL);
		if (NULL == ThreadHandle) {
			return K_ERROR;
		}
		CloseHandle(ThreadHandle);
	}

	while (K_TRUE) {
		PER_HANDLE_DATA * PerHandleData = NULL;
		SOCKADDR_IN saRemote;
		int RemoteLen;
		SOCKET clientfd;

		RemoteLen = sizeof(saRemote);
		clientfd = accept(manager->server_socket, (SOCKADDR*)&saRemote, &RemoteLen);
		if (SOCKET_ERROR == clientfd) {
			printf("acceptSocket Error->%d\n", GetLastError());
			continue;
		}

		socket_event_t sock_event;
		data_ptr = &sock_event;
		socket_event_init(&sock_event);
		socket_event_connect(&sock_event, clientfd);
		
		DO_CALL_BACK(K_FALSE);

		PerHandleData = (LPPER_HANDLE_DATA)GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA));
		
		PerHandleData->manager = manager;

		memcpy(&PerHandleData->socket_event, &sock_event, sizeof(sock_event));
		data_ptr = &PerHandleData->socket_event;
		memcpy(&PerHandleData->ClientAddr, &saRemote, RemoteLen);

		CreateIoCompletionPort((HANDLE)(PerHandleData->socket_event.clientfd), manager->completionPort, (DWORD)PerHandleData, 0);

		LPPER_IO_OPERATION_DATA PerIoData = NULL;
		PerIoData = (LPPER_IO_OPERATION_DATA)GlobalAlloc(GPTR, sizeof(PER_IO_OPERATEION_DATA));
		ZeroMemory(&(PerIoData->overlapped), sizeof(OVERLAPPED));
		PerIoData->databuff.len = 1024;
		PerIoData->databuff.buf = PerIoData->buffer;
		PerIoData->operationType = 0;    // read

		DWORD RecvBytes;
		DWORD Flags = 0;
		WSARecv(PerHandleData->socket_event.clientfd, &(PerIoData->databuff), 1, &RecvBytes, &Flags, &(PerIoData->overlapped), NULL);
	}
}

static DWORD WINAPI ServerWorkThread(LPVOID IpParam)
{
	int result;
	socket_event_t* data_ptr = K_NULL;
	event_manager_t* manager = K_NULL;
	HANDLE CompletionPort = (HANDLE)IpParam;
	DWORD BytesTransferred;
	LPOVERLAPPED IpOverlapped;
	LPPER_HANDLE_DATA PerHandleData = NULL;
	LPPER_IO_DATA PerIoData = NULL;
	DWORD RecvBytes;
	DWORD Flags = 0;
	BOOL bRet = K_FALSE;

	while (K_TRUE) {

		bRet = GetQueuedCompletionStatus(CompletionPort, &BytesTransferred,
			(PULONG_PTR)&PerHandleData, (LPOVERLAPPED*)&IpOverlapped, INFINITE);

		if (bRet == 0) {
			printf("GetQueuedCompletionStatus Error->%d\n", GetLastError());
			return -1;
		}
		PerIoData = (LPPER_IO_DATA)CONTAINING_RECORD(IpOverlapped, PER_IO_DATA, overlapped);


		if (0 == BytesTransferred) {
			data_ptr = &PerHandleData->socket_event;
			manager = PerHandleData->manager;
			DO_CALL_BACK(K_TRUE);
			GlobalFree(PerHandleData);
			GlobalFree(PerIoData);
			continue;
		}


		log_debug("A Client says->\n%s\n", PerIoData->databuff.buf);

		data_ptr = &PerHandleData->socket_event;
		manager = PerHandleData->manager;
		socket_event_read(data_ptr, PerIoData->databuff.buf, PerIoData->databuff.len);

		DO_CALL_BACK(K_FALSE);

		ZeroMemory(&(PerIoData->overlapped), sizeof(OVERLAPPED));
		PerIoData->databuff.len = 1024;
		PerIoData->databuff.buf = PerIoData->buffer;
		PerIoData->operationType = 0;  // read
		WSARecv(PerHandleData->socket_event.clientfd, &(PerIoData->databuff), 1, &RecvBytes, &Flags, &(PerIoData->overlapped), NULL);
	}

}
#endif

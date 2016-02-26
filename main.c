// server_rasp.cpp : Defines the entry point for the console application.
//

#include "socket.h"
#include "event.h"
#include "http.h"

#include <stdio.h>        // for printf
#include <stdlib.h>        // for exit
#include <string.h>        // for bzero

#define SERVER_PORT 6666

#define MSG_TEST "msg from server!!!"

int my_http_request_process(http_connection_t* connection)
{
	http_request_t* request = &connection->request;
	http_response_t* response = &connection->response;
	http_response_status(response,200);
	http_response_header_add(response, "Content-Language", "en,zh");
	http_response_body_write(response, MSG_TEST, strlen(MSG_TEST));
	return -1;
}

int http_event_callback(socket_event_t* ev)
{
        int result;
        http_connection_t* user_ptr;
        switch(ev->event_type)
        {
        case CLIENT_CONNECT:
        {
                printf("-------------------->new client\n");
                user_ptr = (http_connection_t*)malloc(sizeof(http_connection_t));
                ev->user_data = user_ptr;
                return http_connection_init(user_ptr,ev->clientfd, my_http_request_process);
        }
        case CLIENT_READ:
        {
                //printf("callback->recv->%s\n",ev->data_ptr);
                user_ptr = (http_connection_t*)ev->user_data;
                return http_connection_process(user_ptr,ev->data_ptr,ev->data_len);
        }
        case CLINET_CLOSE:
        {
                printf("-------------------->close client\n");
                user_ptr = (http_connection_t*)ev->user_data;
                result = http_connection_destroy(user_ptr);
                free(ev->user_data);
                ev->user_data = K_NULL;
                return result;
        }
        default:
                return K_SUCCESS;
        }
}

int my_event_callback(socket_event_t* ev)
{
        switch(ev->event_type)
        {
        case CLIENT_CONNECT:
                printf("callback->new client\n");
                break;
        case CLIENT_READ:
                printf("callback->recv->%s\n",ev->data_ptr);
                send(ev->clientfd,MSG_TEST,strlen(MSG_TEST)+1,0);
                return -1;
        case CLINET_CLOSE:
                printf("callback->close client\n");
                break;
        default:
                break;
        }
	return K_SUCCESS;
}

int main(int argc, char *argv[])
{
	socket_init();

	socket_t serverfd = server_create("127.0.0.1", SERVER_PORT);
	setnonblocking(serverfd);

	server_listen(serverfd);

	event_manager_t event_manager;

	event_manager_init(&event_manager, serverfd, http_event_callback);

	event_dispatch(&event_manager);

	event_magager_destroy(&event_manager);

	socket_destory();

    return 0;
}

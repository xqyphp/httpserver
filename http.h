#ifndef http_h
#define http_h

#include "types.h"
#include "utils.h"

#define HTTP_METHOD_GET		0
#define HTTP_METHOD_POST	1
#define HTTP_METHOD_HEAD	2
#define HTTP_METHOD_PUT		3
#define HTTP_METHOD_DELETE	4
#define HTTP_METHOD_OPTIONS 5
#define HTTP_METHOD_TRACE	6
#define HTTP_METHOD_CONNECT 7

typedef void (*http_connection_back)();

typedef struct http_header_s
{
	DEF_LIST_HEADER;
	char key[128];
	char val[128];
}http_header_t;

typedef struct http_request_s
{
        int method;
		char url[512];
		char version[32];
		list_t headers;
		char* body;
}http_request_t;

typedef struct http_response_s
{
        int status;
}http_response_t;

typedef struct http_connection_s
{
        int     connfd;
        char*   data_ptr;
        size_t  data_len;
        http_connection_back fn_back;
        pool_t* pool;
		char*   rdata_ptr;
		size_t  rdata_len;
		size_t  rdata_used;
		// to parse
		http_request_t request;
		size_t  rdata_index;
		int     request_parse_status;
		http_response_t response;
}http_connection_t;


int  http_connection_init(http_connection_t* connection,int connfd,http_connection_back fn_back);
int  http_connection_process(http_connection_t* connection,void* data_ptr,size_t data_len);
int  http_connection_destroy(http_connection_t* connection);

int http_request_parse(http_connection_t* connection);
int http_request_process(http_connection_t* connection);

void http_request_init(http_request_t* request);

int	   http_request_method(http_request_t* request);
char*  http_request_url(http_request_t* request);
char*  http_request_header(http_request_t* request,const char* key);
char*  http_request_body(http_request_t* request);
int  http_request_destroy(http_request_t* request);
/*
void http_response_init(http_response_t* response);
void http_response_status(http_response_t* response,int status);
void http_response_header_add(http_response_t* response,const char* key,const char* val);
void http_response_body_set(http_response_t* response,const char* data_ptr,unsigned int data_len);
void http_response_destroy(http_response_t* response);
*/
#endif

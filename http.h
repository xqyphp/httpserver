#ifndef http_h
#define http_h

#include "k_types.h"
#include "k_list.h"
#include "k_mpool.h"
#include "k_event.h"
#include "k_socket.h"

#define HTTP_METHOD_GET		0
#define HTTP_METHOD_POST	1
#define HTTP_METHOD_HEAD	2
#define HTTP_METHOD_PUT		3
#define HTTP_METHOD_DELETE	4
#define HTTP_METHOD_OPTIONS     5
#define HTTP_METHOD_TRACE	6
#define HTTP_METHOD_CONNECT     7

#define PATH_LEN		260
#define VERSION_LEN		32
#define URL_LEN			512
#define HEADER_LEN      128


typedef struct http_module_s     http_module_t;
typedef struct http_connection_s http_connection_t;
typedef struct http_application_s http_application_t;
typedef struct http_header_s http_header_t;
typedef struct http_request_s http_request_t;
typedef struct http_response_s http_response_t;
typedef struct http_connection_s http_connection_t;

typedef int(*http_request_process_callback)(http_connection_t* connection);


struct http_module_s
{
	DEF_LIST_HEAD(struct http_module_s);
	http_application_t* application;
	http_request_process_callback fn_back;
};


struct http_application_s {
	char version[VERSION_LEN];
	char webroot[PATH_LEN];
	char host[PATH_LEN];
	int  port;
	k_socket_t serverfd;
	event_manager_t event_manager;
	http_request_process_callback fn_back;
	k_list_t modules;
};

struct http_header_s
{
	DEF_LIST_HEAD(struct http_header_s);
	char key[HEADER_LEN];
	char val[HEADER_LEN];
};

struct http_request_s
{
	int method;
	char url[URL_LEN];
	char url_path[PATH_LEN];
	k_list_t params;
	char version[VERSION_LEN];
	k_list_t headers;
	char* body;
};

struct http_response_s
{
	int status;
	char version[VERSION_LEN];
	k_list_t headers;
	char* body_ptr;
	size_t body_used;
	size_t body_len;
	k_mpool_t* pool;
};


struct http_connection_s
{
	int     connfd;
	char*   data_ptr;
	size_t  data_len;
	http_request_process_callback fn_back;
	k_mpool_t* pool;
	char*   rdata_ptr;
	size_t  rdata_len;
	size_t  rdata_used;
	// to parse
	http_request_t request;
	size_t  rdata_index;
	int     request_parse_status;
	http_response_t response;
};

k_status_t	  http_application_init(http_application_t* application, const char* webroot,const char* host,int port);
http_application_t* http_application_instance();
k_status_t    http_application_start(http_application_t* application);
k_status_t    http_application_destory(http_application_t* application);
const char*    http_application_root(http_application_t* application);

k_status_t    http_module_init(http_module_t* module, http_request_process_callback callback);
k_status_t    http_module_register(http_application_t* application, http_module_t* module);

k_status_t    http_connection_init(http_connection_t* connection, int connfd,http_request_process_callback fn_back);
k_status_t    http_connection_process(http_connection_t* connection, void* data_ptr, size_t data_len);
k_status_t    http_connection_destroy(http_connection_t* connection);

k_status_t    http_request_parse(http_connection_t* connection);

k_status_t   http_request_init(http_request_t* request);

k_status_t    http_request_method(http_request_t* request);
char*		  http_request_url(http_request_t* request);
char*         http_request_path(http_request_t* request);
char*         http_request_header(http_request_t* request, const char* key);
char*         http_request_body(http_request_t* request);
k_status_t    http_request_destroy(http_request_t* request);

k_status_t    http_response_init(http_response_t* response, k_mpool_t* pool);
k_status_t	  http_response_status(http_response_t* response, int status);
k_status_t    http_response_header_set(http_response_t* response,const char* key, const char* val);
k_status_t    http_response_body_write(http_response_t* response,const char* data_ptr, unsigned int data_len);
k_status_t    http_response_write(http_connection_t* connection);

k_status_t    http_response_destroy(http_response_t* response);

#endif

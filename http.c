#include "http.h"

#include <string.h>
#include <assert.h>
#include <errno.h>
#define STATUS_INIT         0
#define STATUS_REQUEST_LINE 1
#define STATUS_HEADERS		2
#define STATUS_BLANK        3
#define STATUS_REQUEST_BODY 4
#define STATUS_COMPLETE    -1
#define NEXT_LINE          "\r\n"
#define SPACE_LINE         "\r\n\r\n"
#define COLON_CHAR         ':'
#define COLON_STR          ":"
#define SPACE_STR          " "
#define SPACE_CHAR         ' '

int  http_connection_init(http_connection_t* connection,int connfd,http_connection_back fn_back)
{
        connection->connfd = connfd;
        connection->fn_back = fn_back;
        connection->pool = pool_create("connection_pool",1024,1024);
		connection->rdata_ptr = K_NULL;
		connection->rdata_len = 0;
		connection->rdata_used = 0;
		connection->rdata_index = 0;
		connection->data_ptr = K_NULL;
		connection->data_len = 0;
		connection->request_parse_status = STATUS_INIT;
		http_request_init(&connection->request);
}
/*
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
}http_request_t;*/
void http_request_init(http_request_t* request)
{
	request->method = -1;
	memset(request->url, 0, 512);
	memset(request->version, 0, 32);
	list_init(&request->headers);
	request->body = K_NULL;
}

int  http_connection_process(http_connection_t* connection, void* data_ptr, size_t data_len)
{
        connection->data_ptr = (char*)data_ptr;
        connection->data_len = data_len;
		if (connection->rdata_len - connection->rdata_used < data_len) {
			char* odata = connection->rdata_ptr;
			size_t olen = connection->rdata_len;
			connection->rdata_ptr = pool_malloc(connection->pool, connection->rdata_len + data_len);
			connection->rdata_len = olen + data_len;
			memcpy(connection->rdata_ptr, odata, olen);
		}
		memcpy(connection->rdata_ptr + connection->rdata_used, data_ptr, data_len);
		connection->rdata_used += data_len;
		if (http_request_parse(connection) == -1) {// is parse complete
			return http_request_process(connection);
		}
		return K_SUCCESS;
}

int  http_connection_destroy(http_connection_t* connection)
{
        pool_destory(connection->pool);
}

static int http_parse_request_line(http_connection_t* connection);
static int http_parse_headers(http_connection_t* connection);
static int http_parse_blank(http_connection_t* connection);
static int http_parse_request_body(http_connection_t* connection);
// -1 is complete

static const char* 
g_methods[] = {"GET","POST","HEAD","PUT","DELETE","OPTIONS","TRACE","CONNECT"};

#define BUFFER "Hello,Client!!!\n"

int http_request_parse(http_connection_t* connection)
{
	http_parse_request_line(connection);
	http_parse_headers(connection);
	http_parse_blank(connection);
	return http_parse_request_body(connection);
}
int http_request_process(http_connection_t* connection)
{
	int result = send(connection->connfd, BUFFER, strlen(BUFFER),0);
	printf("send->%s->%d->%d", BUFFER,result,errno);
	return -1;
}


static int http_parse_request_line(http_connection_t* connection)
{
	int i;
	char* next_ptr;
	char* end_ptr;
	http_request_t *request = &connection->request;
	if (connection->request_parse_status != STATUS_INIT) {
		return K_SUCCESS;
	}
	printf("http_parse_request_line---------------------\n");
	assert(connection->rdata_index == 0);
	char* ptr = connection->rdata_ptr + connection->rdata_index;
	
	end_ptr = strstr(ptr, NEXT_LINE);

	if (end_ptr == K_NULL) {
		return K_SUCCESS;
	}

	for (i = 0; i < 8; i++)
	{
		if (strncmp(ptr, g_methods[i], strlen(g_methods[i])) == 0){
			request->method = i;
			printf("method->%d\n", request->method);
			break;
		}
	}
	assert(request->method >=0 && request->method <= 7);
	ptr = ptr + strlen(g_methods[request->method]);
	assert(*ptr = ' ');
	ptr++;

	next_ptr = strchr(ptr, ' ');
	
	assert(end_ptr - next_ptr > 0);
	strncpy(request->url, ptr, next_ptr - ptr);
	printf("url->%s\n", request->url);

	ptr = next_ptr +1;
	strncpy(request->version, ptr, end_ptr - ptr);
	printf("version->%s\n", request->version);

	connection->request_parse_status = STATUS_REQUEST_LINE;
	connection->rdata_index = end_ptr + 2 - connection->rdata_ptr;
	return -1;

}
static int http_parse_headers(http_connection_t* connection)
{
	if (connection->request_parse_status != STATUS_REQUEST_LINE) {
		return K_SUCCESS;
	}
	
	int i;
	char* colon_ptr;
	char* next_ptr;
	char* end_ptr;
	char* ptr;
	http_request_t *request = &connection->request;
	ptr = connection->rdata_ptr + connection->rdata_index;
	printf("http_parse_headers---------------------\n", ptr);
	end_ptr = strstr(ptr, SPACE_LINE);
	if (end_ptr == K_NULL) {
		printf("end_ptr == K_NULL\n", ptr);
		return K_SUCCESS;
	}

	while (K_TRUE) {
		next_ptr = strstr(ptr, NEXT_LINE);
		if (next_ptr > end_ptr) {
			printf("--end header--\n", next_ptr,end_ptr);
			break;
		}
		colon_ptr = strstr(ptr, COLON_STR);
		if (colon_ptr == K_NULL || colon_ptr >= next_ptr) {
			printf("--wrong header--\n");
			ptr = next_ptr + 2;
			continue;
		}
		http_header_t* header = pool_malloc(connection->pool, sizeof(http_header_t));
		strncpy(header->key, ptr, colon_ptr - ptr);
		strncpy(header->val, colon_ptr + 1, next_ptr - (colon_ptr + 1));
		list_insert_before(&request->headers, header);
		printf("%s:%s\n", header->key, header->val);
		ptr = next_ptr + 2;
	}

	connection->request_parse_status = STATUS_HEADERS;
	connection->rdata_index = end_ptr + 2 - connection->rdata_ptr;
	return -1;
}
static int http_parse_blank(http_connection_t* connection) {
	if (connection->request_parse_status != STATUS_HEADERS) {
		return K_SUCCESS;
	}
	printf("http_parse_blank---------------------\n");
	char* ptr;
	ptr = connection->rdata_ptr + connection->rdata_index;
	assert(strncmp(ptr, NEXT_LINE, 2) == 0);
	connection->rdata_index += 2;
	connection->request_parse_status = STATUS_BLANK;
	return -1;
}

static int http_parse_request_body(http_connection_t* connection)
{
	if (connection->request_parse_status != STATUS_BLANK) {
		return K_SUCCESS;
	}
	printf("http_parse_request_body---------------------\n");
	http_request_t *request = &connection->request;
	if (request->method == HTTP_METHOD_GET) {
		printf("http_parse_complete---------------------\n");
		return -1;
	}
	return K_SUCCESS;
}


int	   http_request_method(http_request_t* request)
{
	return request->method;
}
char*  http_request_url(http_request_t* request)
{
	return request->url;
}
char*  http_request_header(http_request_t* request, const char* key)
{
	
	list_t* header_list = &(request->headers);
	http_header_t* header_val = header_list->next;
	while ((http_header_t*)header_val != header_list)
	{
		const char* header_key = header_val->key;
		if (strcmp(key, header_key) == 0)
		{
			return header_val->val;
		}
		header_val = header_val->next;
	}
	return K_NULL;
}
char*  http_request_body(http_request_t* request)
{
	return K_NULL;
}
int  http_request_destroy(http_request_t* request)
{
	return K_SUCCESS;
}
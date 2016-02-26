#include "http.h"

#include <string.h>
#include <assert.h>
#include <errno.h>
#define STATUS_INIT         0
#define STATUS_REQUEST_LINE 1
#define STATUS_HEADERS      2
#define STATUS_BLANK        3
#define STATUS_REQUEST_BODY 4
#define STATUS_COMPLETE    -1
#define NEXT_LINE          "\r\n"
#define SPACE_LINE         "\r\n\r\n"
#define COLON_CHAR         ':'
#define COLON_STR          ":"
#define SPACE_STR          " "
#define SPACE_CHAR         ' '

int
http_connection_init(http_connection_t* connection,int connfd,
                     http_request_process_callback fn_back)
{
        assert(fn_back != K_NULL);
        connection->connfd = connfd;
        connection->fn_back = fn_back;
        connection->pool = pool_create("connection_pool", 1024, 1024);
        connection->rdata_ptr = K_NULL;
        connection->rdata_len = 0;
        connection->rdata_used = 0;
        connection->rdata_index = 0;
        connection->data_ptr = K_NULL;
        connection->data_len = 0;
        connection->request_parse_status = STATUS_INIT;
        http_request_init(&connection->request);
}

void
http_request_init(http_request_t* request)
{
	request->method = -1;
	memset(request->url, 0, 512);
	memset(request->version, 0, 32);
	list_init(&request->headers);
	request->body = K_NULL;
}

static int
http_request_process(http_connection_t* connection)
{
	//todo
	http_response_init(&connection->response,connection->pool);
	int result = connection->fn_back(connection);
	if (result == -1) {
		http_response_write(connection);
	}

	return result;
}

int
http_connection_process(http_connection_t* connection, void* data_ptr, size_t data_len)
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

int
http_connection_destroy(http_connection_t* connection)
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

int
http_request_parse(http_connection_t* connection)
{
	http_parse_request_line(connection);
	http_parse_headers(connection);
	http_parse_blank(connection);
	return http_parse_request_body(connection);
}

static int
http_parse_request_line(http_connection_t* connection)
{
	int i,len;
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
        len = sizeof(g_methods) / sizeof(g_methods[0]);
	for (i = 0; i < len; i++)
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
static int
http_parse_headers(http_connection_t* connection)
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
static int
http_parse_blank(http_connection_t* connection) {
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

static int
http_parse_request_body(http_connection_t* connection)
{
	if (connection->request_parse_status != STATUS_BLANK) {
		return K_SUCCESS;
	}
	printf("http_parse_request_body---------------------\n");
	char* ptr;
	ptr = connection->rdata_ptr + connection->rdata_index;
	http_request_t *request = &connection->request;
	if (request->method == HTTP_METHOD_GET) {
		printf("http_parse_complete1---------------------\n");
		return -1;
	}
	else if (request->method == HTTP_METHOD_POST) {
		int current_len = connection->rdata_len - connection->rdata_index;
		char* str_len = http_request_header(request, "Content-Length");
		if (str_len == K_NULL) {
			printf("http_parse_complete2---------------------\n");
			return -1;
		}
		int req_len = atoi(str_len);
		if (current_len >= req_len) {
			char* body = pool_malloc(connection->pool, req_len + 1);
			memcpy(body, ptr, req_len);
			body[req_len] = '\0';
			request->body = body;
			printf("http_parse_complete3---------------------\n");
			return -1;
		}
	}
	else {
		return K_SUCCESS;
	}

}


int
http_request_method(http_request_t* request)
{
	return request->method;
}
char*
http_request_url(http_request_t* request)
{
	return request->url;
}
char*
http_request_header(http_request_t* request, const char* key)
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
char*
http_request_body(http_request_t* request)
{
	return K_NULL;
}
int
http_request_destroy(http_request_t* request)
{
	return K_SUCCESS;
}


int
http_response_init(http_response_t* response,pool_t* pool)
{
        response->status = -1;
        strcpy(response->version,"HTTP/1.1");
        list_init(&response->headers);

        response->pool = pool;
        response->body_ptr = pool_malloc(pool,1024);
        response->body_len = 1024;
        response->body_used = 0;
        return K_SUCCESS;
}

void
http_response_status(http_response_t* response,int status)
{
        response->status = status;
}

int
http_response_header_add(http_response_t* response,const char* key,const char* val)
{
	http_header_t* header = pool_malloc(response->pool, sizeof(http_header_t));
	strcpy(header->key, key);
	strcpy(header->val, val);
	list_insert_before(&response->headers, header);
        return K_SUCCESS;
}

int
http_response_body_write(http_response_t* response,const char* data_ptr,unsigned int data_len)
{
	if (response->body_len - response->body_used < data_len) {
		char* odata = response->body_ptr;
		size_t olen = response->body_len;
		response->body_ptr = pool_malloc(response->pool, response->body_len + data_len);
		response->body_len = olen + data_len;
		memcpy(response->body_ptr, odata, olen);
	}
	memcpy(response->body_ptr + response->body_used, data_ptr, data_len);
	response->body_used += data_len;
        return K_SUCCESS;
}


static int
my_write(int fd,void *buffer,int length)
{
        int bytes_left;
        int written_bytes;
        char *ptr;

        ptr=buffer;
        bytes_left=length;
        while(bytes_left>0)
        {

                written_bytes=write(fd,ptr,bytes_left);
                if(written_bytes<=0)
                {
                        if(errno==EINTR)
                                written_bytes=0;
                        else
                                return(-1);
                }
                bytes_left-=written_bytes;
                ptr+=written_bytes;
        }
        return(0);
}

#define STATUS_LINE "HTTP/1.1 200 OK\r\n"

int
http_response_write(http_connection_t* connection)
{
	http_response_t* response = &connection->response;

	buffer_t buffer;
	buffer_init(&buffer, connection->pool, 1024);

	buffer_write(&buffer, STATUS_LINE, strlen(STATUS_LINE));

	char str_content_len[32];
	sprintf(str_content_len, "Content-Length:%d", response->body_used);
	buffer_write(&buffer, str_content_len, strlen(str_content_len));
	buffer_write(&buffer, "\r\n", 2);

	const char* header = "Content-Type: text/html; charset=utf-8";
	buffer_write(&buffer, header, strlen(header));
	buffer_write(&buffer, "\r\n", 2);

	buffer_write(&buffer,"\r\n",2);
	buffer_write(&buffer, response->body_ptr, response->body_used);

	printf("----------------send\n");
	printf("%s", buffer.data_ptr);
	printf("\n----------------");
	return my_write(connection->connfd, buffer.data_ptr, buffer.data_used);
}

int
http_response_destroy(http_response_t* response)
{
}

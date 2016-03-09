#include "k_buffer.h"
#include "http.h"

#include <stdlib.h>
#include <stdio.h>
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

static const char* g_status_str[601];
static void init_status_strs()
{
	int i;
	for (i = 0; i < 601; i++) {
		g_status_str[i] = K_NULL;
	}

	g_status_str[100] = "Continue";
	g_status_str[101] = "Switching Protocols";
	g_status_str[102] = "Processing";

	g_status_str[200] = "OK";
	g_status_str[201] = "Created";
	g_status_str[202] = "Accepted";
	g_status_str[203] = "Non - Authoritative Information";
	g_status_str[204] = "No Content";
	g_status_str[205] = "Reset Content";
	g_status_str[206] = "Partial Content";
	g_status_str[207] = "Multi - Status";

	g_status_str[300] = "Multiple Choices";
	g_status_str[301] = "Moved Permanently";
	g_status_str[302] = "Move temporarily";
	g_status_str[303] = "See Other";
	g_status_str[304] = "Not Modified";
	g_status_str[305] = "Use Proxy";
	g_status_str[306] = "Switch Proxy";
	g_status_str[307] = "Temporary Redirect";

	g_status_str[400] = "Bad Request";
	g_status_str[401] = "Unauthorized";
	g_status_str[402] = "Payment Required";
	g_status_str[403] = "Forbidden";
	g_status_str[404] = "Not Found";
	g_status_str[405] = "Method Not Allowed";
	g_status_str[406] = "Not Acceptable";
	g_status_str[407] = "Proxy Authentication Required";
	g_status_str[408] = "Request Timeout";
	g_status_str[409] = "Conflict";
	g_status_str[410] = "Gone";
	g_status_str[411] = "Length Required";
	g_status_str[412] = "Precondition Failed";
	g_status_str[413] = "Request Entity Too Large";
	g_status_str[414] = "Request - URI Too Long";
	g_status_str[415] = "Unsupported Media Type";
	g_status_str[416] = "Requested Range Not Satisfiable";
	g_status_str[417] = "Expectation Failed";
	g_status_str[421] = "here are too many connections from your internet address";
	g_status_str[422] = "Unprocessable Entity";
	g_status_str[423] = "Locked";
	g_status_str[424] = "Failed Dependency";
	g_status_str[425] = "Unordered Collection";
	g_status_str[426] = "Upgrade Required";
	g_status_str[449] = "Retry With";
	g_status_str[500] = "Internal Server Error";
	g_status_str[501] = "Not Implemented";
	g_status_str[502] = "Bad Gateway";
	g_status_str[503] = "Service Unavailable";
	g_status_str[504] = "Gateway Timeout";
	g_status_str[505] = "HTTP Version Not Supported";
	g_status_str[506] = "Variant Also Negotiates";
	g_status_str[507] = "Insufficient Storage";
	g_status_str[509] = "Bandwidth Limit Exceeded";
	g_status_str[510] = "Not Extended";
	g_status_str[600] = "Unparseable Response Headers";
}

static int http_event_callback(socket_event_t* ev)
{
	int result;
	http_connection_t* user_ptr;
	switch (ev->event_type)
	{
	case CLIENT_CONNECT:
	{
		printf("-------------------->new client\n");
		user_ptr = (http_connection_t*)malloc(sizeof(http_connection_t));
		ev->user_data = user_ptr;
		http_application_t* pHttp = http_application_instance();
		assert(pHttp != K_NULL);
		return http_connection_init(user_ptr, ev->clientfd,pHttp->fn_back);
	}
	case CLIENT_READ:
	{
		//printf("callback->recv->%s\n",ev->data_ptr);
		user_ptr = (http_connection_t*)ev->user_data;
		return http_connection_process(user_ptr, ev->data_ptr, ev->data_len);
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

static http_application_t* g_http_application;


static int my_http_request_process(http_connection_t* connection)
{
	http_application_t* pHttp = http_application_instance();
	assert(pHttp != K_NULL);

	k_list_t* modules = &pHttp->modules;

	k_list_t* list_next = (k_list_t*)modules->next;
	while (list_next != (k_list_t*)modules)
	{
		http_module_t* module = (http_module_t*)list_next;
		if (module->fn_back == K_NULL) {
			continue;
		}

		if (module->fn_back(connection) == -1) {
			return -1;
		}

		list_next = list_next->next;
	}

	return -1;
}

k_status_t	   http_application_init(http_application_t* application,
	const char* webroot, const char* host, int port)
{
	g_http_application = application;
	init_status_strs();
	k_socket_init();
	strncpy(application->webroot, webroot, PATH_LEN);
	strncpy(application->host, host, PATH_LEN);
	application->fn_back = my_http_request_process;
	application->port = port;
	k_list_init(&application->modules);
	application->serverfd = k_server_create(application->host, application->port);
	k_setnonblocking(application->serverfd);
	k_server_listen(application->serverfd);
	event_manager_init(&application->event_manager, application->serverfd, http_event_callback);
	return K_SUCCESS;
}

http_application_t* http_application_instance()
{
	return g_http_application;
}

k_status_t    http_application_start(http_application_t* application)
{
	return event_dispatch(&application->event_manager);
}

k_status_t    http_application_destory(http_application_t* application)
{
	g_http_application = K_NULL;
	event_magager_destroy(&application->event_manager);
	k_socket_destory();
	return K_SUCCESS;
}

const char*    http_application_root(http_application_t* application)
{
	return application->webroot;
}


k_status_t    http_module_init(http_module_t* module, http_request_process_callback callback)
{
	k_list_init((k_list_t*)module);
	module->fn_back = callback;
	return K_SUCCESS;
}

k_status_t    http_module_register(http_application_t* application, http_module_t* module)
{
	module->application = application;
	k_list_insert_before(&application->modules, (k_list_t*)module);
	return K_SUCCESS;
}

k_status_t
http_connection_init(http_connection_t* connection, int connfd,
	http_request_process_callback fn_back)
{
	assert(fn_back != K_NULL);
	connection->connfd = connfd;
	connection->fn_back = fn_back;
	connection->pool = k_mpool_create("connection_pool", 1024, 1024);
	connection->rdata_ptr = K_NULL;
	connection->rdata_len = 0;
	connection->rdata_used = 0;
	connection->rdata_index = 0;
	connection->data_ptr = K_NULL;
	connection->data_len = 0;
	connection->request_parse_status = STATUS_INIT;
	http_request_init(&connection->request);
	return K_SUCCESS;
}

k_status_t
http_request_init(http_request_t* request)
{
	request->method = -1;
	memset(request->url, 0, 512);
	memset(request->version, 0, 32);
	k_list_init(&request->headers);
	request->body = K_NULL;
	return K_SUCCESS;
}

static int
http_request_process(http_connection_t* connection)
{
	//todo
	http_response_init(&connection->response, connection->pool);
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
		connection->rdata_ptr = k_mpool_malloc(connection->pool, connection->rdata_len + data_len);
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

k_status_t
http_connection_destroy(http_connection_t* connection)
{
	k_mpool_destory(connection->pool);
	return K_SUCCESS;
}

static int http_parse_request_line(http_connection_t* connection);
static int http_parse_headers(http_connection_t* connection);
static int http_parse_blank(http_connection_t* connection);
static int http_parse_request_body(http_connection_t* connection);
// -1 is complete

static const char*
g_methods[] = { "GET","POST","HEAD","PUT","DELETE","OPTIONS","TRACE","CONNECT" };

k_status_t
http_request_parse(http_connection_t* connection)
{
	http_parse_request_line(connection);
	http_parse_headers(connection);
	http_parse_blank(connection);
	return http_parse_request_body(connection);
}

static int 
http_parse_request_url(http_request_t* request)
{
	const char* url = request->url;
	log_debug(url);
	const char* ptr = strstr(url,"http://");
	if (ptr == K_NULL) {
		strcpy(request->url_path, url);
		log_debug(request->url_path);
		return K_SUCCESS;
	}
	
	ptr = url + strlen("http://");
	ptr = strstr(ptr, "/");
	if (ptr == K_NULL) {
		strcpy(request->url_path, "index.html");
		log_debug(request->url_path);
		return K_SUCCESS;
	}

	log_debug(ptr);
	const char* end_ptr = strstr(ptr, "?");
	
	if (end_ptr = K_NULL) {
		log_debug("end_ptr = K_NULL");
		strcpy(request->url_path, ptr);
	}
	else {
		log_debug("end_ptr != K_NULL->%d", end_ptr - ptr);
		strncpy(request->url_path, ptr, end_ptr - ptr);
	}
	log_debug(request->url_path);
	return K_SUCCESS;
}

static int
http_parse_request_line(http_connection_t* connection)
{
	int i, len;
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
		if (strncmp(ptr, g_methods[i], strlen(g_methods[i])) == 0) {
			request->method = i;
			printf("method->%d\n", request->method);
			break;
		}
	}
	assert(request->method >= 0 && request->method <= 7);
	ptr = ptr + strlen(g_methods[request->method]);
	assert(*ptr = ' ');
	ptr++;

	next_ptr = strchr(ptr, ' ');

	assert(end_ptr - next_ptr > 0);
	strncpy(request->url, ptr, next_ptr - ptr);
	log_debug("url->%s", request->url);
	http_parse_request_url(request);
	log_debug("url_path->%s", request->url_path);
	ptr = next_ptr + 1;
	strncpy(request->version, ptr, end_ptr - ptr);
	log_debug("version->%s", request->version);

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
			printf("--end header--\n", next_ptr, end_ptr);
			break;
		}
		colon_ptr = strstr(ptr, COLON_STR);
		if (colon_ptr == K_NULL || colon_ptr >= next_ptr) {
			printf("--wrong header--\n");
			ptr = next_ptr + 2;
			continue;
		}
		http_header_t* header = k_mpool_malloc(connection->pool, sizeof(http_header_t));
		strncpy(header->key, ptr, colon_ptr - ptr);
		strncpy(header->val, colon_ptr + 1, next_ptr - (colon_ptr + 1));
		k_list_insert_before((k_list_t*)&request->headers, (k_list_t*)header);
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

static k_status_t
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
		return K_ERROR;
	}
	else if (request->method == HTTP_METHOD_POST) {
		int current_len = connection->rdata_len - connection->rdata_index;
		char* str_len = http_request_header(request, "Content-Length");
		if (str_len == K_NULL) {
			printf("http_parse_complete2---------------------\n");
			return K_ERROR;
		}
		int req_len = atoi(str_len);
		if (current_len >= req_len) {
			char* body = k_mpool_malloc(connection->pool, req_len + 1);
			memcpy(body, ptr, req_len);
			body[req_len] = '\0';
			request->body = body;
			printf("http_parse_complete3---------------------\n");
			return K_ERROR;
		}
		return K_SUCCESS;
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
http_request_path(http_request_t* request)
{
	return request->url_path;
}

char*
http_request_header(http_request_t* request, const char* key)
{

	k_list_t* header_list = &(request->headers);
	http_header_t* header_val = (http_header_t*)header_list->next;
	while ((k_list_t*)header_val != header_list)
	{
		const char* header_key = header_val->key;
		if (strcmp(key, header_key) == 0)
		{
			return header_val->val;
		}
		header_val = (http_header_t*)header_val->next;
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
http_response_init(http_response_t* response, k_mpool_t* pool)
{

	response->pool = pool;
	response->body_ptr = k_mpool_malloc(pool, 1024);
	response->body_len = 1024;
	response->body_used = 0;

	response->status = 200;
	strcpy(response->version, "HTTP/1.1");
	k_list_init(&response->headers);
	http_response_header_set(response, "Content-Type", "text/html; charset=utf-8");

	return K_SUCCESS;
}

k_status_t
http_response_status(http_response_t* response, int status)
{
	response->status = status;
	return K_SUCCESS;
}

int
http_response_header_set(http_response_t* response, const char* key, const char* val)
{
	k_list_t* headers = &response->headers;
	http_header_t* header = (http_header_t*)headers->next;

	while ((void*)header != headers) {
		if (strcmp(header->key, key) == 0) {
			strcpy(header->val, val);
			return K_SUCCESS;
		}
		header = (http_header_t*)header->next;
	}

	header = (http_header_t*)k_mpool_malloc(response->pool, sizeof(http_header_t));
	strcpy(header->key, key);
	strcpy(header->val, val);
	k_list_insert_before((k_list_t*)&response->headers, (k_list_t*)header);
	return K_SUCCESS;
}

int
http_response_body_write(http_response_t* response, const char* data_ptr, unsigned int data_len)
{
	if (response->body_len - response->body_used < data_len) {
		char* odata = response->body_ptr;
		size_t olen = response->body_len;
		response->body_ptr = k_mpool_malloc(response->pool, response->body_len + data_len);
		response->body_len = olen + data_len;
		memcpy(response->body_ptr, odata, olen);
	}
	memcpy(response->body_ptr + response->body_used, data_ptr, data_len);
	response->body_used += data_len;
	return K_SUCCESS;
}


static int
my_write(int fd, void *buffer, int length)
{
	int bytes_left;
	int written_bytes;
	char *ptr;

	ptr = buffer;
	bytes_left = length;
	while (bytes_left > 0)
	{

		written_bytes = write(fd, ptr, bytes_left);
		if (written_bytes <= 0)
		{
			if (errno == EINTR)
				written_bytes = 0;
			else
				return(-1);
		}
		bytes_left -= written_bytes;
		ptr += written_bytes;
	}
	return(0);
}

static void write_status_line(http_response_t* response, k_buffer_t* buffer)
{
	char str_buffer[32];

	k_buffer_write(buffer, response->version, strlen(response->version));
	k_buffer_write(buffer, " ", 1);


	sprintf(str_buffer, "%d ", response->status);
	k_buffer_write(buffer, str_buffer, strlen(str_buffer));
	const char* str_status = g_status_str[response->status] != K_NULL ?
		g_status_str[response->status] : "Unknow staus!!!";

	k_buffer_write(buffer, str_status, strlen(str_status));
	k_buffer_write(buffer, "\r\n", 2);
}

static void write_headers(http_response_t* response, k_buffer_t* buffer)
{
	k_list_t* headers = &response->headers;
	http_header_t* header = (http_header_t*)headers->next;
	char str_header[512] = { 0 };
	while ((void*)header != headers) {
		sprintf(str_header, "%s:%s\r\n", header->key, header->val);
		k_buffer_write(buffer, str_header, strlen(str_header));
		header = (http_header_t*)header->next;
	}
}


static void write_blank_line(k_buffer_t* buffer)
{
	k_buffer_write(buffer, "\r\n", 2);
}

int
http_response_write(http_connection_t* connection)
{
	char str_buffer[32];
	http_response_t* response = &connection->response;
	k_buffer_t buffer;
	k_buffer_init(&buffer, connection->pool, 1024);

	sprintf(str_buffer, "%d", response->body_used);
	http_response_header_set(response, "Content-Length",
		str_buffer);

	write_status_line(response, &buffer);

	write_headers(response, &buffer);
	write_blank_line(&buffer);

	//write body
	k_buffer_write(&buffer, response->body_ptr, response->body_used);

	printf("----------------send\n");
	printf("%s", (const char*)buffer.data_ptr);
	printf("\n----------------");
	return my_write(connection->connfd, buffer.data_ptr, buffer.data_used);
}

k_status_t
http_response_destroy(http_response_t* response)
{
	return K_SUCCESS;
}

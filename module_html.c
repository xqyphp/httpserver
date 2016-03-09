#include "module_html.h"
#include "http.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

http_module_t g_http_module_html;

#define MSG_FILE_NOT_FOUND "404!!! file not found!"

int module_html_http_request_process(http_connection_t* connection)
{

	http_application_t* pHttp = http_application_instance();
	assert(pHttp != K_NULL);
	http_request_t* request = &connection->request;
	http_response_t* response = &connection->response;

	const char* root_path = http_application_root(pHttp);
	const char* url_path = http_request_path(request);
	assert(root_path != K_NULL);
	assert(url_path != K_NULL);
	char buffer[512];
	sprintf(buffer,"%s%s", root_path, url_path);

	FILE *fp = K_NULL;
	
	log_info("-------%s------",buffer);
	fp = fopen(buffer, "r");
	if (K_NULL == fp)
	{
		http_response_status(response, 404);
		http_response_header_set(response, "Content-Language", "en,zh");
		http_response_body_write(response, MSG_FILE_NOT_FOUND, strlen(MSG_FILE_NOT_FOUND));
		log_warn(MSG_FILE_NOT_FOUND);
		return -1;
	}

	char tmp[64];
	while (!feof(fp))//判定文件是否结尾
	{

		if (fgets(tmp, 64, fp) != K_NULL) {
			http_response_body_write(response, tmp, strlen(tmp));
		}
	}

	fclose(fp);
	fp = K_NULL;

	return -1;
}


void register_module_html(http_application_t* pHttp)
{
	http_module_init(&g_http_module_html, module_html_http_request_process);
	http_module_register(pHttp, &g_http_module_html);
}


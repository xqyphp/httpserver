#include "module_html.h"
#include "http.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "ksp_lexer.h"
#include "ksp_parser.h"
#include "ksp_runner.h"

http_module_t g_http_module_ksp;
static http_response_t* g_response = K_NULL;
#define MSG_FILE_NOT_FOUND "404!!! file not found!"

void respone_print(const char* msg) {
	http_response_body_write(g_response, msg, strlen(msg));
}

int module_ksp_http_request_process(http_connection_t* connection)
{

	http_application_t* pHttp = http_application_instance();
	assert(pHttp != K_NULL);
	http_request_t* request = &connection->request;
	g_response = &connection->response;
	http_response_t* response = g_response;
	const char* root_path = http_application_root(pHttp);
	const char* url_path = http_request_path(request);
	assert(root_path != K_NULL);
	assert(url_path != K_NULL);
	char buffer[512];
	sprintf(buffer, "%s%s", root_path, url_path);

	FILE *fp = K_NULL;

	log_info("-------%s------", buffer);
	fp = fopen(buffer, "r");
	if (K_NULL == fp)
	{
		http_response_status(g_response, 404);
		http_response_header_set(response, "Content-Language", "en,zh");
		http_response_body_write(response, MSG_FILE_NOT_FOUND, strlen(MSG_FILE_NOT_FOUND));
		log_warn(MSG_FILE_NOT_FOUND);
		return -1;
	}
	fclose(fp);

	ksp_lexer_t lexer;
	
	ksp_lexer_init_doc(&lexer, buffer, respone_print);
	ksp_parser_t parser;
	ksp_parser_init(&parser, &lexer);
	ksp_parser_parse(&parser);

	ksp_runner_t runner;

	ksp_runner_init(&runner, &parser, respone_print);
	ksp_runner_run(&runner);

	ksp_parser_destroy(&parser);

	return -1;
}


void register_module_ksp(http_application_t* pHttp)
{
	http_module_init(&g_http_module_ksp, module_ksp_http_request_process);
	http_module_register(pHttp, &g_http_module_ksp);
}









/*
char ch = look_char(lexer);
char ch2 = look2_char(lexer);
if (lexer->bhtml) {
k_buffer_t html_buffer;
k_buffer_init(&html_buffer, lexer->pool, 1024);
while (!(ch=='<' && ch2=='?'))
{
if (lexer->index >= lexer->text_len-1) {
return ksp_word_get_char(lexer, TAG_UNKNOW, ch);
}
k_buffer_write_ch(&html_buffer, current_char(lexer));
if ('\n' == ch) {
lexer->line++;
}
ch = next_char(lexer);
ch2 = look2_char(lexer);
}

if (lexer->fn_print != K_NULL) {
lexer->fn_print(k_buffer_get_data(&html_buffer));
}

next_char(lexer);
next_char(lexer);

lexer->bhtml = K_FALSE;
}

ch = look_char(lexer);
while (isspace(ch)) {
if ('\n' == ch) {
lexer->line++;
}
ch = next_char(lexer);
}
ch2 = look2_char(lexer);
if (ch == '?' && ch2 == '>') {
next_char(lexer);
next_char(lexer);
lexer->bhtml = K_TRUE;
return ksp_word_read(lexer);
}

ch = look_char(lexer);
ch2 = look2_char(lexer);
if (ch == '/'&&ch2 == '/') {
while (ch != '\n' && ch != (char)-1) {
ch = next_char(lexer);
}
ch = next_char(lexer);
lexer->line++;
}

*/
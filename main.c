#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "k_types.h"
#include "k_mpool.h"
#include "k_buffer.h"
#include "k_hash.h"
#include "http.h"
#include "module_html.h"
#include "ksp_lexer.h"
#include "ksp_parser.h"
#define SERVER_PORT 6666

#define MSG_TEST "msg from server!!!"

int my_http_request_process(http_connection_t* connection)
{
	http_request_t* request = &connection->request;
	http_response_t* response = &connection->response;
	http_response_status(response, 200);
	http_response_header_set(response, "Content-Language", "en,zh");
	http_response_body_write(response, MSG_TEST, strlen(MSG_TEST));
	return -1;
}


int main(int argc, const char * argv[]) {

	ksp_lexer_t lexer;
	const char* file = "C:\\Users\\lk\\Documents\\Visual Studio 2015\\Projects\\Test\\http_server\\Text.txt";
	ksp_lexer_init_doc(&lexer, file);
#if 0
	ksp_word_t* w;
	while (1) {
		w = ksp_word_read(&lexer);
		log_info("%s", w->val);
		if (TAG_UNKNOW == w->tag) {
			break;
		}
	}
	
#endif
	ksp_parser_t parser;
	ksp_parser_init(&parser, &lexer);
	ksp_parser_parse(&parser);
	ksp_parser_destroy(&parser);
	system("pause");
#if 0
	http_application_t http_inst;

	http_application_init(&http_inst, "/usr/local/www/", "localhost", SERVER_PORT);
#if 0
	http_module_t hello_module;
	http_module_init(&hello_module, my_http_request_process);
	http_module_register(&http_inst, &hello_module);
#else
	register_module_html(&http_inst);
#endif
	

	http_application_start(&http_inst);
	http_application_destory(&http_inst);
#endif
	return 0;
}


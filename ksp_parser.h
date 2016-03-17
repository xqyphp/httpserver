#ifndef ksp_parser_h
#define ksp_parser_h
#include "ksp_lexer.h"
#include "k_mpool.h"

typedef enum ksp_type_e ksp_type_t;
typedef struct ksp_tree_s ksp_tree_t;
typedef struct ksp_parser_s ksp_parser_t;

enum ksp_type_e
{
	PROGROM, TEXT/*不执行,直接输出*/, FUNCTION, BLOCK, STATMENTS, STATMENT/*实际是不存在的*/,
	IF, WHILE, EXPR, EXPR1, TERM, FACTOR, BRACKET, NUMBER, STRING, RETURN,
	VAR, ASSIGN, UNKNOW, CALL_ARGS, FUNC_CALL, PARAMS, PARAM,
	ARR_INIT, ARR_AC
};

struct ksp_tree_s {
	ksp_type_t type;
	ksp_tree_t* parent;
	ksp_tree_t* left;
	ksp_tree_t* right;
	ksp_tree_t* ext;
	const char* code;
	int line;
	ksp_word_t* word;
};

struct ksp_parser_s
{
	ksp_lexer_t* lexer;
	ksp_tree_t* root;
	k_mpool_t* pool;
};


k_status_t ksp_parser_init(ksp_parser_t* parser, ksp_lexer_t* lexer);
k_status_t ksp_parser_parse(ksp_parser_t* parser);
k_status_t ksp_parser_destroy(ksp_parser_t* parser);
#endif

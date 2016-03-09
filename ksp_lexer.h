#ifndef ksp_lexer_h
#define ksp_lexer_h
#include "k_types.h"
#include "k_mpool.h"
#include "k_buffer.h"
#include "k_cstd.h"

typedef enum ksp_tag_e ksp_tag_t;
typedef struct ksp_word_s ksp_word_t;
typedef struct ksp_lexer_s ksp_lexer_t;

enum ksp_tag_e
{
	IF = 0, ELSE, FOR, WHILE, RETURN,
	RBL, RBR, SBL, SBR, BBL, BBR,
	GT, LT, GE, LE, EQ, NOT, NE, AND,
	PLUS, MINIS, MUL, DIV,
	ID, AS, SEM, DOT, QUOTE, COMMA,
	NUMBER, STRING, COLON, UNKNOW 
};

struct ksp_word_s
{
	DEF_LIST_HEAD(struct ksp_word_s);
	enum ksp_tag_e tag;
	char* val;
	k_size_t val_len;
};

struct ksp_lexer_s 
{
	const char* file_name;
	char* text;
	k_mpool_t* pool;
	k_size_t text_len;
	int index;
	int line;
	ksp_word_t* _current;
	ksp_word_t* _look;
	k_list_t    _words;
};


k_status_t ksp_lexer_init_doc(ksp_lexer_t* lexer,const char* file);
k_status_t ksp_lexer_init_string(ksp_lexer_t* lexer, const char* str);
k_status_t ksp_lexer_destroy(ksp_lexer_t* lexer);

ksp_word_t* ksp_word_get_char(ksp_lexer_t* lexer, ksp_tag_t tag,char ch);
ksp_word_t* ksp_word_get(ksp_lexer_t* lexer,ksp_tag_t tag);
ksp_word_t* ksp_word_get2(ksp_lexer_t* lexer, ksp_tag_t tag, const char* val);
ksp_word_t* ksp_word_get3(ksp_lexer_t* lexer, ksp_tag_t tag, const char* val,k_size_t val_len);

ksp_word_t* ksp_word_current(ksp_lexer_t* lexer);
ksp_word_t* ksp_word_look(ksp_lexer_t* lexer);
ksp_word_t* ksp_word_next(ksp_lexer_t* lexer);
ksp_word_t* ksp_word_next_tag(ksp_lexer_t* lexer, ksp_tag_t tag);
ksp_word_t* ksp_word_read(ksp_lexer_t* lexer);

#endif


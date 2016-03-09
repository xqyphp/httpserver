#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "k_types.h"
#include "k_mpool.h"
#include "k_buffer.h"
#include "k_hash.h"

typedef enum   parser_state_e {
	INIT,
	WORD,
	NUMBER,
	SYMBOL,
	STRING,
	LINE_COMMENT,
	BLOCK_COMMENT
}parser_state_t;

typedef struct k_int_list_s
{
	DEF_LIST_HEAD(struct k_int_list_s);
	int num;
}k_int_list_t;

typedef struct sql_parser_s {
	char ch;
	char prev_ch;
	char quote_ch;
	parser_state_t state;
	k_list_t boundaries;
	k_bool_t retainComment;
	const char* sql;
	k_size_t sqllen;
	k_buffer_t buf;
	k_bool_t bmysql;
	char mum_mask;
	char str_mask;
	k_mpool_t* pool;
}sql_parser_t;

void sql_parser_init(sql_parser_t* parser, k_mpool_t* pool, k_bool_t is_mysql);
k_status_t sql_parser_parse(sql_parser_t* parser, const char* sql);
k_bool_t   sql_is_cross_boundary(sql_parser_t* parser, const char* input);

struct TestEntry
{
	k_hash_entry_t entry;
	int key;
	char val[32];
};

int my_compare(void* left, void* right)
{
	int* left_ptr = (int*)left;
	int* right_ptr = (int*)right;
	return *left_ptr - *right_ptr;
}
void* my_getkey(void* val)
{
	struct TestEntry* val_ptr = (struct TestEntry*)val;
	return &val_ptr->key;

}

int test(int argc, const char * argv[]) {
	printf("Hello, World!\n");
	k_mpool_t* pool = k_mpool_create("pool", 16, 16);
	sql_parser_t parser;
	sql_parser_init(&parser, pool, 1);
	sql_parser_parse(&parser, "set names 'utf8'");
	k_bool_t bcross = sql_is_cross_boundary(&parser, "26");
	printf("bcross->%d", bcross);
	k_mpool_destory(pool);
	return 0;
}

static void ChangeState(sql_parser_t* parser, k_size_t i);
static k_bool_t IsWhitespace(char c);
static k_bool_t IsAsciiDigit(char c);
static k_bool_t IsAsciiLetter(char c);
static k_bool_t IsSqlWordStart(char c);
static k_bool_t isSqlWordPart(char c);

void sql_parser_init(sql_parser_t* parser, k_mpool_t* pool, k_bool_t is_mysql)
{
	parser->pool = pool;
	parser->bmysql = is_mysql;

	parser->ch = 0;
	parser->prev_ch = 0;
	parser->quote_ch = 0;
	parser->state = INIT;
	parser->mum_mask = '7';
	parser->retainComment = K_FALSE;
	parser->str_mask = 'a';
	k_list_init(&parser->boundaries);
	k_buffer_init(&parser->buf, pool, 1024);

}


static void push_back_boundaries(sql_parser_t* parser, int val)
{
	k_int_list_t* list_val = (k_int_list_t*)k_mpool_malloc(parser->pool, sizeof(k_int_list_t));
	list_val->num = val;
	k_list_insert_before((k_list_t*)&parser->boundaries, (k_list_t*)list_val);
}

k_status_t sql_parser_parse(sql_parser_t* parser, const char* sql)
{
	k_size_t i;

	parser->sql = sql;
	parser->sqllen = strlen(sql);

	for (i = 0; i < parser->sqllen; i++) {
		parser->prev_ch = parser->ch;
		parser->ch = parser->sql[i];
		switch (parser->state)
		{
		case INIT:
			if (!IsWhitespace(parser->ch)) {
				ChangeState(parser, i);
			}
			else {
				k_buffer_write_ch(&parser->buf, parser->ch);
			}
			break;
		case WORD:
			if (isSqlWordPart(parser->ch)) {
				k_buffer_write_ch(&parser->buf, parser->ch);
			}
			else
			{
				ChangeState(parser, i);
			}
			break;
		case NUMBER:
			if (!IsAsciiDigit(parser->ch) && parser->ch != '.')
			{
				ChangeState(parser, i);
			}
			break;
		case SYMBOL:
			if (parser->prev_ch == '-' && parser->ch == '-')
			{
				k_buffer_write_ch(&parser->buf, parser->ch);

				if (k_list_get_valid_size(&parser->boundaries) ||
					((k_int_list_t*)k_list_get_last(&parser->boundaries))->num != i - 1)
				{
					push_back_boundaries(parser, i - 1);
				}
				parser->state = LINE_COMMENT;
			}
			else if (parser->prev_ch == '/' && parser->ch == '*')
			{
				k_buffer_write_ch(&parser->buf, parser->ch);
				if (k_list_get_valid_size(&parser->boundaries) ||
					((k_int_list_t*)k_list_get_last(&parser->boundaries))->num != i - 1)
				{
					push_back_boundaries(parser, i - 1);
				}
				parser->state = BLOCK_COMMENT;
			}
			else
			{
				ChangeState(parser, i);
			}
			break;
		case STRING:
			if (parser->ch == parser->quote_ch)
			{
				if (i + 1 < parser->sqllen)
				{
					int num = sql[i + 1];
					if (num == parser->quote_ch)
					{
						i++;
					}
					else
					{
						parser->state = INIT;
						k_buffer_write_ch(&parser->buf, '\'');
					}

				}
				else
				{
					parser->state = INIT;
					k_buffer_write_ch(&parser->buf, '\'');
				}
			}
			else if (parser->bmysql && parser->ch == '\\' && i + 1 < parser->sqllen)
			{
				i++;
			}
			break;
		case LINE_COMMENT:
			if (parser->ch == '\r' || parser->ch == '\n')
			{
				k_buffer_write_ch(&parser->buf, parser->ch);
				parser->state = INIT;
			}
			else if (parser->retainComment)
			{
				k_buffer_write_ch(&parser->buf, parser->ch);
			}
			break;
		case BLOCK_COMMENT:
			if (parser->prev_ch == '*' && parser->ch == '/')
			{
				if (parser->retainComment)
				{
					k_buffer_write_ch(&parser->buf, parser->ch);
				}
				else
				{
					k_buffer_write(&parser->buf, " */", strlen(" */"));
				}
				parser->state = INIT;
			}
			else if (parser->retainComment)
			{
				k_buffer_write_ch(&parser->buf, parser->ch);
			}
			break;
		}
	}

	return K_SUCCESS;
}

static int GetCeiling(k_list_t* boundaries, int value)
{
	int count = 1;
	k_int_list_t* list_index = (k_int_list_t*)boundaries->next;
	while ((k_list_t*)list_index != boundaries)
	{
		if (list_index->num >= value) {
			return list_index->num;
		}

		list_index = list_index->next;
	}

	return -1;
}

k_bool_t   sql_is_cross_boundary(sql_parser_t* parser, const char* input)
{
	int startIndex = 0;
	const char* startPtr = K_NULL;
	int input_len = strlen(input);
	if (input == K_NULL || input_len <= 1)
	{
		return K_FALSE;
	}
	while (K_TRUE)
	{
		startPtr = strstr(parser->sql + startIndex, input);
		if (startPtr == K_NULL) {
			return K_FALSE;
		}
		int num = startPtr - parser->sql;
		int num2 = num + input_len;
		if (-1 == num)
		{
			return K_FALSE;
		}
		int ceiling = GetCeiling(&parser->boundaries, num + 1);
		if (-1 != ceiling && ceiling < num2)
		{
			break;
		}
		startIndex = num2;
	}
	return K_TRUE;
}


static void ChangeState(sql_parser_t* parser, k_size_t i)
{
	parser_state_t sTATE = parser->state;
	if (IsWhitespace(parser->ch))
	{
		k_buffer_write_ch(&parser->buf, parser->ch);
		parser->state = INIT;
	}
	else if (IsAsciiDigit(parser->ch) ||
		('-' == parser->ch && i + 1 < parser->sqllen && IsAsciiDigit(parser->sql[i + 1])))
	{
		k_buffer_write_ch(&parser->buf, parser->mum_mask);
		parser->state = NUMBER;
	}
	else if (IsSqlWordStart(parser->ch))
	{
		k_buffer_write_ch(&parser->buf, parser->ch);
		parser->state = WORD;
	}
	else if ('\'' == parser->ch || (parser->bmysql && '"' == parser->ch))
	{
		parser->quote_ch = parser->ch;
		k_buffer_write_ch(&parser->buf, '\0');

		if (parser->str_mask != '\0')
		{
			k_buffer_write_ch(&parser->buf, parser->str_mask);
		}
		parser->state = STRING;
		parser->ch = '\0';
	}
	else if (parser->bmysql && '#' == parser->ch)
	{
		k_buffer_write_ch(&parser->buf, parser->ch);
		parser->state = LINE_COMMENT;
	}
	else
	{
		k_buffer_write_ch(&parser->buf, parser->ch);
		parser->state = SYMBOL;
	}
	if ((sTATE != parser->state || parser->state == NUMBER) && parser->state != INIT && i != 0)
	{
		push_back_boundaries(parser, i);
	}
}

static k_bool_t IsWhitespace(char c)
{
	return c == ' ' || c == '\r' || c == '\n' || c == '\t';
}


static k_bool_t IsAsciiDigit(char c)
{
	return c >= '0' && c <= '9';
}

static k_bool_t IsAsciiLetter(char c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static k_bool_t IsSqlWordStart(char c)
{
	return IsAsciiLetter(c) || c == '_' || c == '@' || c == '$' || isalpha(c);
}

static k_bool_t isSqlWordPart(char c)
{
	return IsAsciiLetter(c) || IsAsciiDigit(c) || c == '.' || c == '_' || c == '@' || c == '$' ||
		isalpha(c);
}

#include "k_types.h"
#include "ksp_lexer.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

static const char* g_ksp_words[128] = { K_NULL };

static void ksp_lexer_init_common(ksp_lexer_t* lexer)
{
	g_ksp_words[TAG_IF]= "if";
	g_ksp_words[TAG_ELSE]= "else";
	g_ksp_words[TAG_FOR]= "for";
	g_ksp_words[TAG_WHILE]= "while";
	g_ksp_words[TAG_RETURN]= "return";
	g_ksp_words[TAG_FUNCTION] = "function";
	g_ksp_words[TAG_RBL]= "(";
	g_ksp_words[TAG_RBR]= ")";
	g_ksp_words[TAG_SBR]= "[";
	g_ksp_words[TAG_SBL]= "]";
	g_ksp_words[TAG_BBL]= "{";
	g_ksp_words[TAG_BBR]= "}";
	g_ksp_words[TAG_GT]= ">";
	g_ksp_words[TAG_LT]= "<";
	g_ksp_words[TAG_GE]= ">=";
	g_ksp_words[TAG_LE] = "<=";
	g_ksp_words[TAG_EQ]= "==";
	g_ksp_words[TAG_NOT]= "!";
	g_ksp_words[TAG_NE]= "!=";
	g_ksp_words[TAG_PLUS]= "+";
	g_ksp_words[TAG_MINIS]= "-";
	g_ksp_words[TAG_MUL]= "*";
	g_ksp_words[TAG_DIV]= "/";
	g_ksp_words[TAG_AS]= "=";
	lexer->file_name = K_NULL;
	lexer->text = K_NULL;
	lexer->text_len = 0;
	lexer->index = -1;
	lexer->line = 1;
	lexer->_current = K_NULL;
	lexer->_look = K_NULL;
	lexer->pool = k_mpool_create("lexer pool", 1024, 1024);
	k_list_init(&lexer->_words);
}

k_status_t ksp_lexer_init_doc(ksp_lexer_t* lexer, const char* file)
{
	FILE *fp = K_NULL;
	
	ksp_lexer_init_common(lexer);
	lexer->file_name = file;
	fp = fopen(file, "r");
	if (K_NULL == fp)
	{
		return K_ERROR;
	}

	fseek(fp, 0, SEEK_END); 
	lexer->text_len = ftell(fp); 
	fseek(fp, 0, 0);

	k_buffer_t buffer;
	k_buffer_init(&buffer, lexer->pool, lexer->text_len);
	char tmp[64];

	while (!feof(fp))
	{
 		if (fgets(tmp, 64, fp) != K_NULL) {
			k_buffer_write(&buffer, tmp, strlen(tmp));
		}
	}

	fclose(fp);
	fp = K_NULL;

	lexer->text = k_buffer_get_data(&buffer);
	ksp_word_next(lexer);
	return K_SUCCESS;

}
k_status_t ksp_lexer_init_string(ksp_lexer_t* lexer, const char* str)
{
	ksp_lexer_init_common(lexer);
	lexer->text = str;
	lexer->text_len = strlen(str);
	ksp_word_next(lexer);
	return K_SUCCESS;
}
k_status_t ksp_lexer_destroy(ksp_lexer_t* lexer)
{
	lexer->file_name = K_NULL;
	lexer->text = K_NULL;
	lexer->text_len = 0;
	lexer->index = -1;
	lexer->line = 1;
	k_mpool_destory(lexer->pool);
	lexer->pool = K_NULL;
	return K_SUCCESS;
}

int         ksp_word_ival(ksp_word_t* w)
{
	return atoi(w->val);
}
const char* ksp_word_sval(ksp_word_t* w)
{
	return w->val;
}

ksp_word_t* ksp_word_get_char(ksp_lexer_t* lexer, ksp_tag_t tag, char ch)
{
	return ksp_word_get3(lexer, tag, &ch, 1);
}

ksp_word_t* ksp_word_get(ksp_lexer_t* lexer, ksp_tag_t tag)
{
	return ksp_word_get2(lexer, tag, "");
}
ksp_word_t* ksp_word_get2(ksp_lexer_t* lexer, ksp_tag_t tag, const char* val)
{
	return ksp_word_get3(lexer, tag, val, strlen(val));
}

ksp_word_t* ksp_word_get3(ksp_lexer_t* lexer, ksp_tag_t tag,
	const char* val, k_size_t val_len)
{
	ksp_word_t* word = k_mpool_malloc(lexer->pool, sizeof(ksp_word_t));
	word->tag = tag;
	if (g_ksp_words[tag] != K_NULL) {
		word->val = g_ksp_words[tag];
		return word;
	}
	word->val_len = val_len;
	word->val = k_mpool_malloc(lexer->pool, val_len + 1);
	memcpy(word->val, val, val_len);
	word->val[word->val_len] = '\0';
	return word;
}

static char look_char(ksp_lexer_t* lexer)
{
	if (lexer->index >= lexer->text_len - 1) {
		return (char)-1;
	}
	return lexer->text[lexer->index + 1];
}

static char look2_char(ksp_lexer_t* lexer)
{
	if (lexer->index >= lexer->text_len - 2) {
		return (char)-1;
	}
	return lexer->text[lexer->index + 2];
}

static char next_char(ksp_lexer_t* lexer)
{
	lexer->index++;
	return look_char(lexer);
}

ksp_word_t* ksp_word_current(ksp_lexer_t* lexer)
{
	return lexer->_current;
}
ksp_word_t* ksp_word_look(ksp_lexer_t* lexer)
{
	return lexer->_look;
}
ksp_word_t* ksp_word_next(ksp_lexer_t* lexer)
{
	lexer->_current = lexer->_look;
	lexer->_look = ksp_word_read(lexer);
	return lexer->_current;
}
ksp_word_t* ksp_word_next_tag(ksp_lexer_t* lexer, ksp_tag_t tag)
{
	assert(lexer->_look->tag == tag);
	if (lexer->_look->tag != tag) {
		//TODO
		log_error("tag not equal!!!");
	}
	return ksp_word_next(lexer);
}
//TODO
static k_bool_t is_id_start(char ch)
{
	return isalpha(ch) || ch=='_' || ch=='$';
}

static k_bool_t is_id_part(char ch)
{
	return is_id_start(ch) || isdigit(ch);
}

static k_bool_t str_start_with(const char* str, const char* part)
{
	return strstr(str, part) != K_NULL;
}

ksp_word_t* ksp_word_read(ksp_lexer_t* lexer)
{
	char ch = look_char(lexer);
	while (isspace(ch)) {
		if ('\n' == ch) {
			lexer->line++;
		}
		ch = next_char(lexer);
	}
	char ch2 = look2_char(lexer);
	if (ch == '/'&&ch2 == '/') {
		while (ch != '\n' && ch != (char)-1) {
			ch = next_char(lexer);
		}
		ch = next_char(lexer);
		lexer->line++;
	}


	switch (ch) {
	case '{':
		next_char(lexer);
		return ksp_word_get_char(lexer,TAG_BBL, ch);
	case '}':
		next_char(lexer);
		return ksp_word_get_char(lexer,TAG_BBR, ch);
	case '[':
		next_char(lexer);
		return ksp_word_get_char(lexer,TAG_SBL, ch);
	case ']':
		next_char(lexer);
		return ksp_word_get_char(lexer,TAG_SBR, ch);
	case '(':
		next_char(lexer);
		return ksp_word_get_char(lexer,TAG_RBL, ch);
	case ')':
		next_char(lexer);
		return ksp_word_get_char(lexer,TAG_RBR, ch);
	case '+':
		next_char(lexer);
		return ksp_word_get_char(lexer,TAG_PLUS, ch);
	case '-':
		next_char(lexer);
		return ksp_word_get_char(lexer,TAG_MINIS, ch);
	case '*':
		next_char(lexer);
		return ksp_word_get_char(lexer,TAG_MUL, ch);
	case '/':
		next_char(lexer);
		return ksp_word_get_char(lexer,TAG_DIV, ch);
	case ';':
		next_char(lexer);
		return ksp_word_get_char(lexer,TAG_SEM, ch);
	case ',':
		next_char(lexer);
		return ksp_word_get_char(lexer,TAG_COMMA, ch);
	case '.':
		next_char(lexer);
		return ksp_word_get_char(lexer,TAG_DOT, ch);
	case ':':
		next_char(lexer);
		return ksp_word_get_char(lexer,TAG_COLON, ch);
	case '=':
		if (next_char(lexer) == '=') {
			next_char(lexer);
			return ksp_word_get2(lexer,TAG_EQ, "==");
		}
		else {
			return ksp_word_get2(lexer,TAG_AS, "=");
		}
	case '>':
		if (next_char(lexer) == '=') {
			next_char(lexer);
			return ksp_word_get(lexer,TAG_GE);
		}
		else {
			return ksp_word_get(lexer,TAG_GT);
		}
	case '<':
		if (next_char(lexer) == '=') {
			next_char(lexer);
			return ksp_word_get(lexer,TAG_LE);
		}
		else {
			return ksp_word_get(lexer,TAG_LT);
		}
	case '&':
		next_char(lexer);
		return ksp_word_get(lexer,TAG_AND);
	case '!':
		if (next_char(lexer) == '=') {
			next_char(lexer);
			return ksp_word_get(lexer,TAG_NE);
		}
		else {
			return ksp_word_get(lexer,TAG_NOT);
		}
	default:
		break;
	}

	k_buffer_t sb;
	k_buffer_init(&sb, lexer->pool, 64);

	if (isdigit(ch)) {
		while (isdigit(ch)) {
			k_buffer_write_ch(&sb, ch);
			ch =  next_char(lexer);
		}
		return ksp_word_get3(lexer,TAG_NUMBER,k_buffer_get_data(&sb),k_buffer_get_len(&sb));
	}
	else if (is_id_start(ch)) {
		while (is_id_part(ch)) {
			k_buffer_write_ch(&sb, ch);
			ch =  next_char(lexer);
		}
		const char* str = k_buffer_get_data(&sb);
		if (str_start_with(str,"if")) {
			return ksp_word_get(lexer,TAG_IF);
		}
		else if (str_start_with(str, "while")) {
			return ksp_word_get(lexer,TAG_WHILE);
		}
		else if (str_start_with(str, "else")) {
			return ksp_word_get(lexer,TAG_ELSE);
		}
		else if (str_start_with(str, "for")) {
			return ksp_word_get(lexer,TAG_FOR);
		}
		else if (str_start_with(str, "return")) {
			return ksp_word_get(lexer,TAG_RETURN);
		}
		else if (str_start_with(str, "function")) {
			return ksp_word_get(lexer, TAG_FUNCTION);
		}
		return ksp_word_get3(lexer,TAG_ID, str, k_buffer_get_len(&sb));
	}
	else if (ch == '\"') {
		ch =  next_char(lexer);
		while (ch != '\"') {
			if (ch == '\\') {
				ch =  next_char(lexer);
				k_buffer_write_ch(&sb, ch);
			}
			k_buffer_write_ch(&sb, ch);
			ch =  next_char(lexer);
		}
		 next_char(lexer);
		 return ksp_word_get3(lexer, TAG_STRING, k_buffer_get_data(&sb), k_buffer_get_len(&sb));
	}
	else if (ch == '\'') {
		ch =  next_char(lexer);
		while (ch != '\'') {
			if (ch == '\\') {
				k_buffer_write_ch(&sb, ch);
				ch =  next_char(lexer);
			}
			k_buffer_write_ch(&sb, ch);
			ch =  next_char(lexer);
		}
		 next_char(lexer);
		 return ksp_word_get3(lexer, TAG_STRING, k_buffer_get_data(&sb), k_buffer_get_len(&sb));
	}

	return ksp_word_get_char(lexer, TAG_UNKNOW, ch);
}

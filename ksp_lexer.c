#include "k_types.h"
#include "ksp_lexer.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

static const char* g_ksp_words[128] = { K_NULL };

static void ksp_lexer_init_common(ksp_lexer_t* lexer)
{
	g_ksp_words[IF]= "if";
	g_ksp_words[ELSE]= "else";
	g_ksp_words[FOR]= "for";
	g_ksp_words[WHILE]= "while";
	g_ksp_words[RETURN]= "return";
	g_ksp_words[RBL]= "(";
	g_ksp_words[RBR]= ")";
	g_ksp_words[SBR]= "[";
	g_ksp_words[SBL]= "]";
	g_ksp_words[BBL]= "{";
	g_ksp_words[BBR]= "}";
	g_ksp_words[GT]= ">";
	g_ksp_words[LT]= "<";
	g_ksp_words[GE]= ">=";
	g_ksp_words[LE] = "<=";
	g_ksp_words[EQ]= "==";
	g_ksp_words[NOT]= "!";
	g_ksp_words[NE]= "!=";
	g_ksp_words[PLUS]= "+";
	g_ksp_words[MINIS]= "-";
	g_ksp_words[MUL]= "*";
	g_ksp_words[DIV]= "/";
	g_ksp_words[AS]= "=";
	lexer->file_name = K_NULL;
	lexer->text = K_NULL;
	lexer->text_len = 0;
	lexer->index = -1;
	lexer->line = 1;
	lexer->_current = K_NULL;
	lexer->_look = K_NULL;
	lexer->pool = k_mpool_create("lexer pool", 1024, 1024);
}

k_status_t ksp_lexer_init_doc(ksp_lexer_t* lexer, const char* file)
{
	FILE *fp = K_NULL;

	ksp_lexer_init_common(lexer);

	fp = fopen(file, "r");
	if (K_NULL == fp)
	{
		return K_ERROR;
	}

	fseek(fp, 0, SEEK_END); 
	lexer->text_len = ftell(fp); 
	fseek(fp, 0, 0);

	k_buffer_t buffer;
	k_buffer_init(&buffer, lexer->pool, lexer->text_len + 32);
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
	lexer->text[lexer->text_len] = '\0';

}
k_status_t ksp_lexer_init_string(ksp_lexer_t* lexer, const char* str)
{
	ksp_lexer_init_common(lexer);
	lexer->text = str;
	lexer->text_len = strlen(str);
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
	ksp_word_t* word = k_mpool_malloc(lexer->pool, sizeof(word));
	word->tag = tag;
	if (g_ksp_words[tag] != K_NULL) {
		word->val = g_ksp_words[tag];
		return word;
	}

	word->val = k_mpool_malloc(lexer->pool, val_len + 1);
	strncpy(word->val, val_len, val);
	word->val[word->val_len] = '\0';
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
	if (lexer->_look->tag == tag) {
		//TODO
		log_error("tag not equal!!!");
	}
	return ksp_word_next(lexer);
}
//TODO
static k_bool_t is_id_start(char ch)
{
	return isalpha(ch);
}

static k_bool_t is_id_part(char ch)
{
	return isalpha(ch) || isdigit(ch);
}

static k_bool_t str_start_witch(const char* str, const char* part)
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
		return ksp_word_get_char(lexer,BBL, ch);
	case '}':
		next_char(lexer);
		return ksp_word_get_char(lexer,BBR, ch);
	case '[':
		next_char(lexer);
		return ksp_word_get_char(lexer,SBL, ch);
	case ']':
		next_char(lexer);
		return ksp_word_get_char(lexer,SBR, ch);
	case '(':
		next_char(lexer);
		return ksp_word_get_char(lexer,RBL, ch);
	case ')':
		next_char(lexer);
		return ksp_word_get_char(lexer,RBR, ch);
	case '+':
		next_char(lexer);
		return ksp_word_get_char(lexer,PLUS, ch);
	case '-':
		next_char(lexer);
		return ksp_word_get_char(lexer,MINIS, ch);
	case '*':
		next_char(lexer);
		return ksp_word_get_char(lexer,MUL, ch);
	case '/':
		next_char(lexer);
		return ksp_word_get_char(lexer,DIV, ch);
	case ';':
		next_char(lexer);
		return ksp_word_get_char(lexer,SEM, ch);
	case ',':
		next_char(lexer);
		return ksp_word_get_char(lexer,COMMA, ch);
	case '.':
		next_char(lexer);
		return ksp_word_get_char(lexer,DOT, ch);
	case ':':
		next_char(lexer);
		return ksp_word_get_char(lexer,COLON, ch);
	case '=':
		if (next_char(lexer) == '=') {
			next_char(lexer);
			return ksp_word_get2(lexer,EQ, "==");
		}
		else {
			return ksp_word_get2(lexer,AS, "=");
		}
	case '>':
		if (next_char(lexer) == '=') {
			next_char(lexer);
			return ksp_word_get(lexer,GE);
		}
		else {
			return ksp_word_get(lexer,GT);
		}
	case '<':
		if (next_char(lexer) == '=') {
			next_char(lexer);
			return ksp_word_get(lexer,LE);
		}
		else {
			return ksp_word_get(lexer,LT);
		}
	case '&':
		next_char(lexer);
		return ksp_word_get(lexer,AND, ch);
	case '!':
		if (next_char(lexer) == '=') {
			next_char(lexer);
			return ksp_word_get(lexer,NE);
		}
		else {
			return ksp_word_get(lexer,NOT);
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
		return getWord(Tag.NUMBER, sb.toString());
	}
	else if (is_id_start(ch)) {
		while (is_id_part(ch)) {
			k_buffer_write_ch(&sb, ch);
			ch =  next_char(lexer);
		}
		String str = sb.toString();
		if (str_start_witch(str,"if")) {
			return ksp_word_get(Tag.IF);
		}
		else if (str_start_witch(str, "while")) {
			return ksp_word_get(Tag.WHILE);
		}
		else if (str_start_witch(str, "else")) {
			return ksp_word_get(Tag.ELSE);
		}
		else if (str_start_witch(str, "for")) {
			return ksp_word_get(Tag.FOR);
		}
		else if (str_start_witch(str, "return")) {
			return ksp_word_get(Tag.RETURN);
		}
		return ksp_word_get(Tag.ID, str);
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
		return ksp_word_get(Tag.STRING, sb.toString());
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
		return getWord(Tag.STRING, sb.toString());
	}
	if (Character.isDigit(ch)) {
			while (Character.isDigit(ch)) {
				k_buffer_write_ch(&sb, ch);
				ch =  next_char(lexer);
			}
			return getWord(Tag.NUMBER, sb.toString());
		} else if (Character.isJavaIdentifierStart(ch)) {
			while (Character.isJavaIdentifierPart(ch)) {
				k_buffer_write_ch(&sb, ch);
				ch =  next_char(lexer);
			}
			String str = sb.toString();
			if ("if".equals(str)) {
				return getWord(Tag.IF);
			} else if ("while".equals(str)) {
				return getWord(Tag.WHILE);
			} else if ("else".equals(str)){
				return getWord(Tag.ELSE);
			} else if ("for".equals(str)){
				return getWord(Tag.FOR);
			} else if ("return".equals(str)){
				return getWord(Tag.RETURN);
			}
			return getWord(Tag.ID, str);
		}else if (ch=='\"'){
			ch =  next_char(lexer);
			while(ch != '\"'){
				if(ch == '\\'){
					ch =  next_char(lexer);
					k_buffer_write_ch(&sb, ch);
				}
				k_buffer_write_ch(&sb, ch);
				ch =  next_char(lexer);
			}
			 next_char(lexer);
			return getWord(Tag.STRING,sb.toString());
		}else if (ch=='\''){
			ch =  next_char(lexer);
			while(ch != '\''){
				if(ch == '\\'){
					k_buffer_write_ch(&sb, ch);
					ch =  next_char(lexer);
				}
				k_buffer_write_ch(&sb, ch);
				ch =  next_char(lexer);
			}
			 next_char(lexer);
			return getWord(Tag.STRING,sb.toString());
		}
		return getWord(Tag.UNKNOW,ch); getWord(Tag.UNKNOW, ch);


}

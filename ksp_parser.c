#include "ksp_parser.h"

k_status_t ksp_parser_init(ksp_parser_t* parser, ksp_lexer_t* lexer)
{
	parser->lexer = lexer;
	parser->pool = k_mpool_create("parser pool", 1024, 1024);
	parser->root = K_NULL;
	return K_SUCCESS;
}
/*
ksp_tree_t* tree_new(ksp_parser_t* parser,ksp_type_t type);
ksp_tree_t* tree_new2(ksp_parser_t* parser, ksp_type_t type, ksp_tree_t* parent);
ksp_tree_t* tree_new3(ksp_parser_t* parser, ksp_type_t type,ksp_word_t* word);
ksp_tree_t* tree_set_parent(ksp_parser_t* parser, ksp_tree_t* parent);
ksp_tree_t* tree_remove_child(ksp_parser_t* parser, ksp_tree_t* child);
ksp_tree_t* tree_add_left(ksp_parser_t* parser);
ksp_tree_t* tree_add_right(ksp_parser_t* parser);
ksp_tree_t* tree_add_ext(ksp_parser_t* parser);
ksp_tree_t* tree_set_left(ksp_parser_t* parser, ksp_tree_t* child);
ksp_tree_t* tree_set_right(ksp_parser_t* parser, ksp_tree_t* child);
ksp_tree_t* tree_set_ext(ksp_parser_t* parser, ksp_tree_t* child);
*/

ksp_tree_t* tree_new(ksp_parser_t* parser, ksp_type_t type)
{
	ksp_tree_t* tree = (ksp_tree_t*)k_mpool_malloc(parser->pool, sizeof(ksp_tree_t));
	tree->type = type;
	tree->line = parser->lexer->line;
	tree->parent = K_NULL;
	tree->left = K_NULL;
	tree->right = K_NULL;
	tree->ext = K_NULL;
	return tree;
}

ksp_tree_t* tree_new2(ksp_parser_t* parser, ksp_type_t type, ksp_tree_t* parent)
{
	ksp_tree_t* tree = tree_new(parser, type);
	tree->parent = parent;
	return tree;
}

ksp_tree_t* tree_new3(ksp_parser_t* parser, ksp_type_t type, ksp_word_t* word)
{
	ksp_tree_t* tree = tree_new(parser, type);
	tree->word = word;
	return tree;
}



k_bool_t tree_remove_child(ksp_tree_t*tree, ksp_tree_t* child)
{
	if (tree->left == child) {
		tree->left = K_NULL;
	}
	else if (tree->right == child) {
		tree->right = K_NULL;
	}
	else if (tree->ext == child) {
		tree->ext = K_NULL;
	}
	else {
		return K_FALSE;
	}
	return K_TRUE;
}

ksp_tree_t* tree_set_parent(ksp_tree_t*tree, ksp_tree_t* parent)
{
	ksp_tree_t* old = tree->parent;
	if (old != K_NULL) {
		tree_remove_child(parent, tree);
	}
	tree->parent = parent;
	return old;
}

ksp_tree_t* tree_add_left(ksp_parser_t* parser, ksp_tree_t*tree, ksp_type_t type)
{
	ksp_tree_t* child = tree_new2(parser, type, tree);
	child->parent = tree;
	return tree->left = child;
}
ksp_tree_t* tree_add_right(ksp_parser_t* parser, ksp_tree_t*tree, ksp_type_t type)
{
	ksp_tree_t* child = tree_new2(parser, type, tree);
	child->parent = tree;
	return tree->right = child;
}
ksp_tree_t* tree_add_ext(ksp_parser_t* parser, ksp_tree_t*tree, ksp_type_t type)
{
	ksp_tree_t* child = tree_new2(parser, type, tree);
	child->parent = tree;
	return tree->ext = child;
}
ksp_tree_t* tree_set_left(ksp_tree_t*tree, ksp_tree_t* child)
{
	ksp_tree_t* old = tree->left;
	tree->left = child;
	return old;
}
ksp_tree_t* tree_set_right(ksp_tree_t*tree, ksp_tree_t* child)
{
	ksp_tree_t* old = tree->right;
	tree->right = child;
	return old;
}
ksp_tree_t* tree_set_ext(ksp_tree_t*tree, ksp_tree_t* child)
{
	ksp_tree_t* old = tree->ext;
	tree->ext = child;
	return old;
}

ksp_tree_t* progrom(ksp_parser_t* parser);
ksp_tree_t* func(ksp_parser_t* parser);
ksp_tree_t* args(ksp_parser_t* parser);
ksp_tree_t* args_expr(ksp_parser_t* parser);
ksp_tree_t* block(ksp_parser_t* parser);
ksp_tree_t* statements(ksp_parser_t* parser);
ksp_tree_t* statement(ksp_parser_t* parser);
ksp_tree_t* arr_init(ksp_parser_t* parser);
ksp_tree_t* expr(ksp_parser_t* parser);
ksp_tree_t* expr1(ksp_parser_t* parser);
ksp_tree_t* term(ksp_parser_t* parser);
ksp_tree_t* factor(ksp_parser_t* parser);
ksp_tree_t* call_args(ksp_parser_t* parser);


static ksp_tree_t* progrom(ksp_parser_t* parser)
{
	ksp_tree_t* left = func(parser);
	if (left == K_NULL)
		return left;
	ksp_tree_t* right = K_NULL;
	while ((right = func(parser)) != K_NULL) {
		ksp_tree_t* t = tree_new(parser, PROGROM);
		tree_set_left(t, left);
		tree_set_right(t, right);
		left = t;
	}
	return left;
}

static ksp_tree_t* func(ksp_parser_t* parser) {
	ksp_lexer_t* lexer = parser->lexer;

	ksp_word_t* w = ksp_word_look(lexer);

	if (w->tag == TAG_UNKNOW) {
		return K_NULL;
	}

	else if (w->tag == TAG_FUNCTION) {
		ksp_word_next(lexer);
		w = ksp_word_next_tag(lexer, TAG_ID);
		ksp_tree_t* t = tree_new3(parser, FUNCTION, w);
		tree_set_left(t, args(parser));
		tree_set_right(t, block(parser));
		return t;
	}
	else {
		return statement(parser);
	}

}

static ksp_tree_t* args(ksp_parser_t* parser)
{
	ksp_lexer_t* lexer = parser->lexer;
	ksp_word_next_tag(lexer, TAG_RBL);
	ksp_word_t* w = ksp_word_look(lexer);
	if (w->tag == TAG_RBR) {
		ksp_word_next_tag(lexer, TAG_RBR);
		return K_NULL;
	}
	ksp_tree_t* left = tree_new3(parser, PARAM, w);
	ksp_word_next_tag(lexer, TAG_ID);

	w = ksp_word_look(lexer);

	while (w->tag != TAG_RBR) {
		ksp_word_next_tag(lexer, TAG_COMMA);
		w = ksp_word_next_tag(lexer, TAG_ID);
		ksp_tree_t* right = tree_new3(parser, PARAM, w);
		ksp_tree_t* t = tree_new(parser, PARAMS);
		tree_set_left(t, left);
		tree_set_right(t, right);
		left = t;
		w = ksp_word_look(lexer);
	}

	ksp_word_next_tag(lexer, TAG_RBR);
	return left;
}

static ksp_tree_t* block(ksp_parser_t* parser)
{
	ksp_lexer_t* lexer = parser->lexer;
	ksp_word_next_tag(parser->lexer, TAG_BBL);
	ksp_tree_t* t = tree_new(parser, BLOCK);
	ksp_tree_t* left = statements(parser);
	tree_set_left(t, left);
	ksp_word_next_tag(lexer,TAG_BBR);
	return t;
}

static ksp_tree_t* statements(ksp_parser_t* parser)
{
	ksp_lexer_t* lexer = parser->lexer;
	ksp_tree_t* left = statement(parser);
	if (left == K_NULL)
		return left;
	ksp_tree_t* right = K_NULL;
	while ((right = statement(parser)) != K_NULL) {
		ksp_tree_t* t = tree_new(parser, STATMENTS);
		tree_set_left(t, left);
		tree_set_right(t, right);
		left = t;
	}
	return left;
}

ksp_tree_t* statement(ksp_parser_t* parser)
{
	ksp_lexer_t* lexer = parser->lexer;
	ksp_word_t* w = ksp_word_look(lexer);
	ksp_tree_t* tr = K_NULL;
	switch (w->tag) {
	case TAG_IF:
	{
		ksp_word_next(lexer);
		ksp_word_next_tag(lexer,TAG_RBL);
		ksp_tree_t* left = statement(parser);
		ksp_word_next_tag(lexer,TAG_RBR);
		ksp_tree_t* right = block(parser);
		ksp_tree_t* t = tree_new(parser, IF);
		tree_set_left(t, left);
		tree_set_right(t, right);
		tr = t;
	}
	break;
	case TAG_WHILE:
	{
		ksp_word_next(lexer);
		ksp_word_next_tag(lexer,TAG_RBL);
		ksp_tree_t* left = statement(parser);
		ksp_word_next_tag(lexer,TAG_RBR);
		ksp_tree_t* right = block(parser);
		ksp_tree_t* t = tree_new(parser, WHILE);
		tree_set_left(t, left);
		tree_set_right(t, right);
		tr = t;
	}
	break;
	case TAG_RETURN:
	{
		w = ksp_word_next(lexer);
		ksp_tree_t* t = tree_new3(parser, RETURN, w);
		ksp_tree_t* left = statement(parser);
		tree_set_left(t, left);
		tr = t;
	}
	break;
	case TAG_BBL:
	{
		ksp_tree_t* t = block(parser);
		tr = t;
	}
	break;
	case TAG_SBL:
	{
		ksp_tree_t* t = arr_init(parser);
		tr = t;
	}
	break;
	case TAG_NUMBER:
	case TAG_ID:
	default:
	{
		ksp_tree_t* t = expr(parser);
		w = ksp_word_look(lexer);
		if (w->tag == TAG_AS) {
			tr = tree_new3(parser, ASSIGN, w);
			ksp_word_next(lexer);
			ksp_tree_t* t2 = statement(parser);
			tree_set_left(tr, t);
			tree_set_right(tr, t2);
		}
		else if (w->tag == TAG_SEM) {
			tr = t;
		}
		else {
			tr = t;
		}
	}
	break;
	}
	
	while (ksp_word_look(lexer)->tag == TAG_SEM)
		ksp_word_next(lexer);
	return tr;
}

static ksp_tree_t* arr_init(ksp_parser_t* parser)
{
	ksp_lexer_t* lexer = parser->lexer;
	ksp_word_t* w = ksp_word_next_tag(lexer,TAG_SBL);
	ksp_tree_t* t = tree_new3(parser, ARR_INIT, w);
	ksp_tree_t* left = args_expr(parser);
	tree_set_left(t, left);
	ksp_word_next_tag(lexer,TAG_SBR);
	return t;
}

static ksp_tree_t* args_expr(ksp_parser_t* parser)
{
	ksp_lexer_t* lexer = parser->lexer;
	ksp_tree_t* left = expr(parser);
	if (left == K_NULL)
		return left;
	ksp_tree_t* right = K_NULL;
	while (K_TRUE) {
		ksp_word_t* w = ksp_word_look(lexer);
		ksp_tag_t tag = w->tag;

		if (tag == TAG_COMMA) {
			ksp_tree_t* t = tree_new3(parser, CALL_ARGS, w);
			ksp_word_next(lexer);
			right = expr(parser);
			tree_set_left(t, left);
			tree_set_right(t, right);
			left = t;
		}
		else {
			return left;
		}
	}
}

static ksp_tree_t* expr(ksp_parser_t* parser)
{
	ksp_lexer_t* lexer = parser->lexer;
	ksp_tree_t* left = expr1(parser);
	if (left == K_NULL)
		return left;
	ksp_tree_t* right = K_NULL;

	while (K_TRUE) {
		ksp_word_t* w = ksp_word_look(lexer);
		ksp_tag_t tag = w->tag;

		if (tag == TAG_GE || tag == TAG_GT || tag == TAG_EQ
			|| tag == TAG_LE || tag == TAG_LT || tag == TAG_NE) {
			ksp_tree_t* t = tree_new3(parser, EXPR1, w);
			ksp_word_next(lexer);
			right = expr1(parser);
			tree_set_left(t, left);
			tree_set_right(t, right);
			left = t;
		}
		else {
			return left;
		}
	}
}

static ksp_tree_t* expr1(ksp_parser_t* parser)
{
	ksp_lexer_t* lexer = parser->lexer;
	ksp_tree_t* left = term(parser);
	if (left == K_NULL)
		return left;
	ksp_tree_t* right = K_NULL;

	while (K_TRUE) {
		ksp_word_t* w = ksp_word_look(lexer);
		ksp_tag_t tag = w->tag;

		if (tag == TAG_PLUS || tag == TAG_MINIS) {
			ksp_tree_t* t = tree_new3(parser, EXPR, w);
			ksp_word_next(lexer);
			right = term(parser);
			tree_set_left(t, left);
			tree_set_right(t, right);
			left = t;
		}
		else {
			return left;
		}
	}
}

static ksp_tree_t* term(ksp_parser_t* parser)
{
	ksp_lexer_t* lexer = parser->lexer;

	if (ksp_word_look(lexer)->tag == TAG_STRING) {
		ksp_word_t* w = ksp_word_next(lexer);
		return tree_new3(parser, STRING, w);
	}
	ksp_tree_t* left = factor(parser);
	if (left == K_NULL)
		return left;
	ksp_tree_t* right = K_NULL;

	while (K_TRUE) {
		ksp_word_t* w = ksp_word_look(lexer);
		ksp_tag_t tag = w->tag;

		if (tag == TAG_MUL || tag == TAG_DIV) {
			ksp_tree_t* t = tree_new3(parser, TERM, w);
			ksp_word_next(lexer);
			right = factor(parser);
			tree_set_left(t, left);
			tree_set_right(t, right);
			left = t;
		}
		else {
			return left;
		}
	}
}

static ksp_tree_t* factor(ksp_parser_t* parser)
{
	ksp_lexer_t* lexer = parser->lexer;
	ksp_word_t* w = ksp_word_look(lexer);
	ksp_tag_t tag = w->tag;
	if (tag == TAG_ID) {
		ksp_word_t* wi = ksp_word_next(lexer);
		w = ksp_word_look(lexer);
		if (w->tag == TAG_RBL) {
			ksp_tree_t* t = tree_new3(parser, FUNC_CALL, wi);
			tree_set_left(t, call_args(parser));
			w = ksp_word_look(lexer);
			if (w->tag == TAG_COLON) {
				ksp_word_next(lexer);
			}
			return t;
		}
		else if (w->tag == TAG_SBL) {
			ksp_word_next(lexer);
			ksp_tree_t* t = tree_new3(parser, ARR_AC, wi);
			tree_set_left(t, expr(parser));
			ksp_word_next_tag(lexer,TAG_SBR);
			return t;
		}
		else {
			ksp_tree_t* t = tree_new3(parser, VAR, wi);
			return t;
		}
	}
	else if (tag == TAG_RBL) {
		ksp_word_next(lexer);
		ksp_tree_t* t = tree_new(parser, BRACKET);
		tree_set_left(t,statement(parser));
		ksp_word_next_tag(lexer,TAG_RBR);
		return t;
	}
	else if (tag == TAG_NUMBER) {
		ksp_tree_t* t = tree_new3(parser, NUMBER, w);
		ksp_word_next(lexer);
		return t;
	}
	else {
		return K_NULL;
	}
}

static ksp_tree_t* call_args(ksp_parser_t* parser)
{
	ksp_lexer_t* lexer = parser->lexer;
	ksp_word_next_tag(lexer,TAG_RBL);
	ksp_tree_t* t = args_expr(parser);
	ksp_word_next_tag(lexer,TAG_RBR);
	return t;
}
k_status_t ksp_parser_parse(ksp_parser_t* parser)
{
	parser->root = progrom(parser);
	return K_SUCCESS;
}


k_status_t ksp_parser_destroy(ksp_parser_t* parser)
{
	k_mpool_destory(parser->pool);
	parser->pool = K_NULL;
	return K_SUCCESS;
}
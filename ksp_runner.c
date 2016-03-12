#include "ksp_runner.h"
#include "k_types.h"

#include <assert.h>
#include <string.h>

static void* get_var_key(void* val)
{
	ksp_var_t* var = (ksp_var_t*)val;
	if (var == K_NULL)
		return K_NULL;
	return var->name;
}

k_status_t ksp_val_init(ksp_var_t* var)
{
	ksp_val_t* val = &var->val;
	val->var = var;
	k_hash_entry_init((k_hash_entry_t*)val);
	val->type = VAL_UNKNOW;
	return K_SUCCESS;
}

k_status_t ksp_val_int_set(ksp_val_t* val, int ival)
{
	val->type = VAL_INT;
	val->val.ival = ival;
	return K_SUCCESS;
}
k_status_t ksp_val_string_set(ksp_val_t* val, const char* sval)
{
	val->type = VAL_STRING;
	val->val.sval = k_mpool_malloc(val->var->scope->pool, strlen(sval + 1));
	strcpy(val->val.sval, sval);
	return K_SUCCESS;
}
k_status_t ksp_val_function_set(ksp_val_t* val, ksp_tree_t* func)
{
	val->type = VAL_FUNCTION;
	val->val.fbodyval = func;
	return K_SUCCESS;
}

k_status_t ksp_var_int_set(ksp_var_t* var, int ival)
{
	ksp_val_t* val = &var->val;
	ksp_val_int_set(val, ival);
}
k_status_t ksp_var_string_set(ksp_var_t* var, const char* sval)
{
	ksp_val_t* val = &var->val;
	ksp_val_string_set(val, sval);
}
k_status_t ksp_var_function_set(ksp_var_t* var, ksp_tree_t* func)
{
	ksp_val_t* val = &var->val;
	ksp_val_function_set(val, func);
}

ksp_val_type_t ksp_val_type(ksp_val_t* val)
{
	return val->type;
}

int  ksp_val_int_get(ksp_val_t* val)
{
	assert(val->type == VAL_INT);
	return val->val.ival;
}
const char*  ksp_val_string_get(ksp_val_t* val)
{
	assert(val->type == VAL_STRING);
	return val->val.sval;
}

ksp_tree_t* ksp_val_function_get(ksp_val_t* val)
{
	assert(val->type == VAL_FUNCTION);
	return val->val.fbodyval;
}

k_bool_t    ksp_var_is_true(ksp_var_t* var)
{
	ksp_val_t* val = &var->val;
	return ksp_val_int_get(val) != 0;
}

ksp_var_t* ksp_var_create(ksp_scope_t* scope, const char* name, k_bool_t bTemp, k_bool_t bReturn)
{
	ksp_var_t* var = k_mpool_malloc(scope->pool, sizeof(var));
	k_hash_entry_init((k_hash_entry_t*)var);

	ksp_val_init(var);

	strncpy(var->name, name, NAME_LEN);
	var->bTemp = bTemp;
	var->bReturn = bReturn;
	return var;
}

k_status_t ksp_var_setval(ksp_var_t* var, ksp_val_t* val)
{
	var->val.type = val->type;
	var->val.val = val->val;
	return K_SUCCESS;
}
ksp_val_t* ksp_var_getval(ksp_var_t* var)
{
	return &var->val;
}

k_status_t ksp_scope_enter(ksp_runner_t* runner)
{
	ksp_scope_t* scope = (ksp_scope_t*)k_mpool_malloc(runner->pool,sizeof(ksp_scope_t));
	scope->pool = k_mpool_create("scope pool", 1024, 1024);
	scope->parent = runner->scope;
	k_hash_init_string(&scope->vars, runner->pool, 1024, get_var_key);
	runner->scope = scope;
	return K_SUCCESS;
}
//TODO 内存泄露问题先不管
k_status_t ksp_scope_leave(ksp_runner_t* runner)
{
	assert(runner->scope != K_NULL);
	k_mpool_destory(runner->scope->pool);
	runner->scope = runner->scope->parent;
	return K_SUCCESS;
}

ksp_var_t* ksp_scope_get(ksp_runner_t* runner, const char* name)
{
	return (ksp_var_t*)k_hash_get(&runner->scope->vars, name);
}

k_status_t ksp_scope_add(ksp_runner_t* runner,  ksp_var_t* var)
{
	k_hash_table_t* vars = &runner->scope->vars;
	const char* name = vars->getkey(var);
	ksp_var_t* var1  = (ksp_var_t*)k_hash_get(vars, name);
	if (var1 = K_NULL) {
		k_hash_put(&runner->scope->vars, var);
	}
	else {
		return K_ERROR;
	}
	return K_SUCCESS;
}

k_status_t ksp_scope_update(ksp_runner_t* runner, const char* name, ksp_var_t* var)
{
	return K_SUCCESS;
}

k_status_t ksp_runner_init(ksp_runner_t* runner, ksp_parser_t* parser)
{
	runner->scope = K_NULL;
	runner->parser = parser;
	runner->pool = k_mpool_create("runner pool", 1024, 1024);
	return K_SUCCESS;
}


ksp_var_t* progrom(ksp_runner_t* r,ksp_tree_t* t);
ksp_var_t* function1(ksp_runner_t* r,ksp_tree_t* t);
ksp_var_t* var1(ksp_runner_t* r,ksp_tree_t* t);
ksp_var_t* param(ksp_runner_t* r,ksp_tree_t* t);
ksp_var_t* assign(ksp_runner_t* r,ksp_tree_t* t);
ksp_var_t* number(ksp_runner_t* r,ksp_tree_t* t);
ksp_var_t* expand(k_list_t rs, ksp_runner_t* r, ksp_val_t type);
ksp_var_t* call(ksp_runner_t* r,ksp_tree_t* t);
ksp_var_t* block(ksp_runner_t* r,ksp_tree_t* t);
ksp_var_t* statements(ksp_runner_t* r,ksp_tree_t* t);
ksp_var_t* bracket(ksp_runner_t* r,ksp_tree_t* t);
ksp_var_t* if1(ksp_runner_t* r,ksp_tree_t* t);
ksp_var_t* while1(ksp_runner_t* r,ksp_tree_t* t);
ksp_var_t* opt(ksp_runner_t* r,ksp_tree_t* t);
ksp_var_t* return1(ksp_runner_t* r,ksp_tree_t* t);
ksp_var_t* arr_init(ksp_runner_t* r,ksp_tree_t* t);
ksp_var_t* arr_ac(ksp_runner_t* r,ksp_tree_t* t);
ksp_var_t* string(ksp_runner_t* r,ksp_tree_t* t);
ksp_var_t* process(ksp_runner_t* r,ksp_tree_t* t);

static ksp_var_t* progrom(ksp_runner_t* r,ksp_tree_t* t)
{
	r->current = t;
	process(r,t->left);
	return process(r,t->right);
}

static ksp_var_t* function1(ksp_runner_t* r,ksp_tree_t* t)
{
	r->current = t;
	const char* fname = t->word->val;
	log_info("define function -> %s", t->word->val);
	ksp_var_t* var = ksp_var_create(r->scope->pool, fname, K_FALSE, K_FALSE);
	ksp_var_function_set(var, t);
	ksp_scope_add(r, var);
}

ksp_var_t* var1(ksp_runner_t* r,ksp_tree_t* t)
{
	r->current = t;
	const char* fname = t->word->val;
	ksp_var_t* var = ksp_scope_get(r, fname);
	if (var == K_NULL) {
		log_info("define var -> %s" , fname);
		var = ksp_var_create(r->scope->pool, fname, K_FALSE, K_FALSE);
		ksp_var_int_set(var, 0);
		ksp_scope_add(r, var);
	}
	return var;
}

static ksp_var_t* param(ksp_runner_t* r,ksp_tree_t* t)
{
	r->current = t;
	const char* fname = t->word->val;
	ksp_var_t* var = ksp_var_create(r->scope->pool, fname, K_FALSE, K_FALSE);
	ksp_var_int_set(var, 0);
	ksp_scope_add(r, var);
	return var;
}
static ksp_var_t* assign(ksp_runner_t* r,ksp_tree_t* t)
{
	r->current = t;
	ksp_var_t* left = process(r,t->left);
	ksp_var_t* right = process(r,t->right);
	if (left != K_NULL) {
		ksp_var_setval(left, &right->val);
	}
	return  left;
}
static ksp_var_t* number(ksp_runner_t* r,ksp_tree_t* t)
{
	r->current = t;
	ksp_var_t* var = ksp_var_create(r->scope, "number", K_FALSE, K_FALSE);
	int ival = ksp_word_ival(t->word);
	ksp_var_int_set(var, ival);
	return var;
}

static k_list_t expand(ksp_runner_t* r,)
{
	k_list_t list;
	return list;
}

static ksp_var_t* call(ksp_runner_t* r,ksp_tree_t* t)
{
	const char* fname = t->word->val;
	if ("print".equals(fname)) {
		Tree left = r,t->left;
		ksp_var_t* var = process(left);
		System.out.println(var == K_NULL ? "K_NULL" : var.val);
		return K_NULL;
	}

	ksp_var_t* fvar = scope.getVar(fname);
	Tree ftree = fvar.val.fbody;
	EnterScope();
	List<ksp_var_t*> args = new ArrayList<ksp_var_t*>();
	List<ksp_var_t*> params = new ArrayList<ksp_var_t*>();

	expand(args, r,t->left, Type.CALL_ARGS);
	expand(params, ftree.left, Type.PARAMS);

	Collections.reverse(args);
	Collections.reverse(params);

	for (int i = 0; i < args.size() && i < params.size(); i++) {
		ksp_var_t* param = params.get(i);
		param.updateVar(args.get(i));
		scope.updateVar(param.name, param);
	}
	ksp_var_t* rs = process(ftree.right);
	LeaveScope();
	return rs;
}
static ksp_var_t* block(ksp_runner_t* r,ksp_tree_t* t)
{
	ksp_scope_enter(r);
	ksp_var_t* var = process(r,t->left);
	ksp_scope_leave(r);
	return var;
}
static ksp_var_t* statements(ksp_runner_t* r,ksp_tree_t* t)
{
	ksp_var_t* lvar = process(r,t->left);
	if (lvar->bReturn) {
		return lvar;
	}
	ksp_var_t* rval = process(r,t->right);
	return rval;
}
static ksp_var_t* bracket(ksp_runner_t* r,ksp_tree_t* t)
{
	ksp_var_t* lvar = process(r,t->left);
	return lvar;
}
static ksp_var_t* if1(ksp_runner_t* r,ksp_tree_t* t)
{
	ksp_var_t* lvar = process(r,t->left);
	int ival = ksp_val_int_get(&lvar->val);
	if (ival != 0) {
		ksp_var_t* rval = process(r,t->right);
		return rval;
	}
	return lvar;
}
static ksp_var_t* while1(ksp_runner_t* r,ksp_tree_t* t)
{
	ksp_var_t* lvar = process(r,t->left);
	ksp_var_t* rvar = K_NULL;
	while (1) {

		if (!ksp_var_is_true(lvar))
			break;
		rvar = process(r,t->right);
		if (rvar->bReturn) {
			return rvar;
		}
		lvar = process(r,t->left);
	}
	return rvar;
}

static void ksp_var_eq(ksp_var_t* var,ksp_var_t* lvar, ksp_var_t* rvar)
{
	k_bool_t result = K_FALSE;
	if (rvar == K_NULL) {
		return;
	}
	if (lvar->val.type == rvar->val.type) 
	{	
		result = memcmp(&lvar->val.val, &lvar->val.val, sizeof(lvar->val.val)) == 0;
	}
	ksp_var_int_set(var, result);
}

static void ksp_var_gt(ksp_var_t* var, ksp_var_t* lvar, ksp_var_t* rvar)
{
  k_bool_t result = K_FALSE;
  if(rvar == K_NULL)
    {
      ksp_var_int_set(var, result);
      return;
    }
  if (lvar->val.type == rvar->val.type) 
    {
      	result = memcmp(&lvar->val.val, &lvar->val.val, sizeof(lvar->val.val)) > 0;
    }
  ksp_var_int_set(var, result);
}

static void ksp_var_ge(ksp_var_t* var, ksp_var_t* lvar, ksp_var_t* rvar)
{
  k_bool_t result = K_FALSE;
  if(rvar == K_NULL)
    {
      ksp_var_int_set(var, result);
      return;
    }
  if (lvar->val.type == rvar->val.type) 
    {
      	result = memcmp(&lvar->val.val, &lvar->val.val, sizeof(lvar->val.val)) >= 0;
    }
  ksp_var_int_set(var, result);
}

static void ksp_var_lt(ksp_var_t* var, ksp_var_t* lvar, ksp_var_t* rvar)
{
  k_bool_t result = K_FALSE;
  if(rvar == K_NULL)
    {
      ksp_var_int_set(var, result);
      return;
    }
  if (lvar->val.type == rvar->val.type) 
    {
      	result = memcmp(&lvar->val.val, &lvar->val.val, sizeof(lvar->val.val)) < 0;
    }
  ksp_var_int_set(var, result);
}

static void ksp_var_le(ksp_var_t* var, ksp_var_t* lvar, ksp_var_t* rvar)
{
  k_bool_t result = K_FALSE;
  if(rvar == K_NULL)
    {
      ksp_var_int_set(var, result);
      return;
    }
  if (lvar->val.type == rvar->val.type) 
    {
      	result = memcmp(&lvar->val.val, &lvar->val.val, sizeof(lvar->val.val)) <= 0;
    }
  ksp_var_int_set(var, result);
}

static void ksp_var_ne(ksp_var_t* var, ksp_var_t* lvar, ksp_var_t* rvar)
{
    k_bool_t result = K_FALSE;
  if(rvar == K_NULL)
    {
      ksp_var_int_set(var, result);
      return;
    }
  if (lvar->val.type != rvar->val.type) 
    {
      	result = memcmp(&lvar->val.val, &lvar->val.val, sizeof(lvar->val.val)) != 0;
    }
  ksp_var_int_set(var, result);
}

static void ksp_var_plus(ksp_var_t* var, ksp_var_t* lvar, ksp_var_t* rvar)
{
    k_bool_t result = K_FALSE;
  if(rvar == K_NULL)
    {
      ksp_var_int_set(var, result);
      return;
    }
  if (lvar->val.type == rvar->val.type) 
    {
      if(lvar->val.type == INT){
	result = lvar->val.ival + rvar->val.ival;
      }
      //TODO
    }
  ksp_var_int_set(var, result);
}

static void ksp_var_minus(ksp_var_t* var, ksp_var_t* lvar, ksp_var_t* rvar)
{
    k_bool_t result = K_FALSE;
  if(rvar == K_NULL)
    {
      return;
    }
  if (lvar->val.type == rvar->val.type) 
    {
      result = memcmp(&lvar->val.val, &lvar->val.val, sizeof(lvar->val.val)) > 0;
    }
  ksp_var_int_set(var, result);
}
static void ksp_var_mul(ksp_var_t* var, ksp_var_t* lvar, ksp_var_t* rvar)
{

}
static void ksp_var_div(ksp_var_t* var, ksp_var_t* lvar, ksp_var_t* rvar)
{

}

static ksp_var_t* opt(ksp_runner_t* r,ksp_tree_t* t)
{
	ksp_var_t* lvar = process(r,t->left);
	ksp_var_t* rvar = process(r,t->right);
	
	ksp_var_t* var = ksp_var_create(r->scope,"temp",K_TRUE,K_FALSE);

	switch (t->word.tag) {
	case TAG_EQ:
		ksp_var_eq(var, lvar, rvar);
		break;
	case TAG_GT:
		ksp_var_gt(var, lvar, rvar);
		break;
	case TAG_GE:
		ksp_var_ge(var, lvar, rvar);
		break;
	case TAG_LT:
		ksp_var_lt(var, lvar, rvar);
		break;
	case TAG_LE:
		ksp_var_le(var, lvar, rvar);
		break;
	case TAG_NE:
		ksp_var_ne(var, lvar, rvar);
		break;
	case TAG_PLUS:
		ksp_var_plus(var, lvar, rvar);
		break;
	case TAG_MINIS:
		ksp_var_minis(var, lvar, rvar);
		break;
	case TAG_MUL:
		ksp_var_mul(var, lvar, rvar);
		break;
	case TAG_DIV:
		ksp_var_div(var, lvar, rvar);
		break;
	default:
		break;
	}

	return var;
}
static ksp_var_t* return1(ksp_runner_t* r,ksp_tree_t* t)
{
	ksp_var_t* var = ksp_var_create(r, "return", K_TRUE, K_TRUE);
	ksp_var_t* lvar = process(r,t->left);
	ksp_var_setval(var, &lvar->val);
	return var;
}
static ksp_var_t* arr_init(ksp_runner_t* r,ksp_tree_t* t)
{
	List<ksp_var_t*> arrs = new ArrayList<ksp_var_t*>();
	expand(arrs, r,t->left, Type.CALL_ARGS);
	List<Val> vals = new ArrayList<Val>();
	for (ksp_var_t* var : arrs) {
		vals.add(var.val);
	}
	Collections.reverse(vals);
	ksp_var_t* list = new ksp_var_t*(vals);
	list.bTemp = true;
	return list;
}
static ksp_var_t* arr_ac(ksp_runner_t* r,ksp_tree_t* t)
{
        const char* name = ksp_word_sval(t->word);
        ksp_var_t* var = scope.getVar(name);
        if (var == K_NULL)
	  return ksp_var_create(r,"arr_not_define",K_FALSE,K_FALSE);
	if (var->val.type != ValType.ARR)
		return new ksp_var_t*("not arr");
	ksp_var_t* index = process(r,t->left);
	if (index.val.type == ValType.INT) {
		List<Val> list = var.val.arr;
		if (index.val.ival >= list.size() || index.val.ival < 0) {
			return new ksp_var_t*("index_out_of_bounds");
		}
		return new ksp_var_t*(list.get(index.val.ival));
	}
	return new ksp_var_t*("err unkonw");
}
static ksp_var_t* string(ksp_runner_t* r,ksp_tree_t* t)
{
	ksp_var_t* var = ksp_var_create(r, "str", K_FALSE, K_FALSE);
	const char* sval = ksp_word_sval(t->word);
	ksp_var_string_set(var, sval);
	return var;
}
static ksp_var_t* process(ksp_runner_t* r,ksp_tree_t* t)
{
	if (t == K_NULL)
		return K_NULL;
	switch (r,t->type) {
	case PROGROM:
		return progrom(r,t);
	case FUNCTION:
		return function1(r,t);
	case ASSIGN:
		return assign(r,t);
	case VAR:
		return var(r,t);
	case PARAM:
		return param(r,t);
	case NUMBER:
		return number(r,t);
	case FUNC_CALL:
		return call(r,t);
	case IF:
		return if1(r,t);
	case WHILE:
		return while1(r,t);
	case BLOCK:
		return block(r,t);
	case STATMENTS:
		return statements(r,t);
	case EXPR:
	case EXPR1:
	case TERM:
		return opt(r,t);
	case RETURN:
		return return1(r,t);
	case BRACKET:
		return bracket(r,t);
	case ARR_INIT:
		return arr_init(r,t);
	case ARR_AC:
		return arr_ac(r,t);
	case STRING:
		return string(r,t);
	default:
		return K_NULL;
	}
}

k_status_t ksp_runner_run(ksp_runner_t* runner)
{
	ksp_scope_enter(runner);
	progrom(runner,runner->parser->root);
	ksp_scope_leave(runner);
	return K_SUCCESS;
}

k_status_t ksp_runner_destroy(ksp_runner_t* runner)
{
	return K_SUCCESS;
}

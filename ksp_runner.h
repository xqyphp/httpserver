#ifndef ksp_runner_h
#define ksp_runner_h
#include "ksp_parser.h"
#include "k_rbtree.h"
#include "k_hash.h"

#define NAME_LEN 32

typedef enum ksp_val_type_e ksp_val_type_t;
typedef struct ksp_val_s ksp_val_t;
typedef struct ksp_var_s ksp_var_t;
typedef struct ksp_scope_s ksp_scope_t;
typedef struct ksp_runner_s ksp_runner_t;

enum ksp_val_type_e {
	VAL_FUNCTION,
	VAL_INT,
	VAL_STRING,
	VAL_OBJ,
	VAL_ARR,
	VAL_UNKNOW
};
//内存问题，引用计数实现？
struct ksp_val_s {
	DEF_HASH_ENTRY_HEAD(struct ksp_var_s);
	ksp_val_type_t type;
	ksp_var_t* var;
	union
	{
		int ival;
		const char* sval;
		ksp_tree_t* fbodyval;
	}val;
	k_hash_table_t objval;
	k_list_t arrval;
	//TODO
	int ref_count;
	k_bool_t is_ref;
};

struct ksp_var_s {
	DEF_HASH_ENTRY_HEAD(struct ksp_var_s);
	ksp_val_t val;
	k_bool_t bTemp;
	k_bool_t bReturn;
	char name[NAME_LEN];
	ksp_scope_t* scope;
};

struct ksp_scope_s
{
	ksp_scope_t* parent;
	k_hash_table_t vars;
	k_mpool_t* pool;
};

struct ksp_runner_s {
	ksp_scope_t* scope;
	ksp_parser_t* parser;
	ksp_tree_t* current;
	k_mpool_t* pool;
};

k_status_t ksp_val_int_set(ksp_val_t* val,int ival);
k_status_t ksp_val_string_set(ksp_val_t* val, const char* sval);
k_status_t ksp_val_function_set(ksp_val_t* val, ksp_tree_t* func);

k_status_t ksp_var_int_set(ksp_var_t* var, int ival);
k_status_t ksp_var_string_set(ksp_var_t* var, const char* sval);
k_status_t ksp_var_function_set(ksp_var_t* var, ksp_tree_t* func);

ksp_val_type_t ksp_val_type(ksp_val_type_t type);
int  ksp_val_int_get(ksp_val_t* val);
const char*  ksp_val_string_get(ksp_val_t* val);
ksp_tree_t* ksp_val_function_get(ksp_val_t* val);
k_bool_t    ksp_var_is_true(ksp_var_t* var);

ksp_var_t* ksp_var_create(ksp_scope_t* scope, const char* name, k_bool_t bTemp, k_bool_t bReturn);
k_status_t ksp_var_setval(ksp_var_t* var,ksp_val_t* val);
ksp_val_t* ksp_var_getval(ksp_var_t* var);

k_status_t ksp_scope_enter(ksp_runner_t* runner);
k_status_t ksp_scope_leave(ksp_runner_t* runner);
ksp_var_t* ksp_scope_get(ksp_runner_t* runner, const char* name);
k_status_t ksp_scope_add(ksp_runner_t* runner, const char* name, ksp_val_t* val);
k_status_t ksp_scope_update(ksp_runner_t* runner, const char* name, ksp_val_t* val);

k_status_t ksp_runner_init(ksp_runner_t* runner, ksp_parser_t* parser);
k_status_t ksp_runner_run(ksp_runner_t* runner);
k_status_t ksp_runner_destroy(ksp_runner_t* runner);

#endif

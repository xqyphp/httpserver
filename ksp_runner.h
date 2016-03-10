#ifndef ksp_runner_h
#define ksp_runner_h
#include "ksp_parser.h"
#include "k_rbtree.h"
#include "k_hash.h"

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

struct ksp_val_s {
	ksp_val_type_t type;
	union value
	{
		int ival;
		const char* sval;
	};
	k_hash_table_t obj;
	k_list_t arr;
	ksp_tree_t* fbody;
};

struct ksp_var_s {
	ksp_val_type_t type;
	ksp_val_t* val;
	k_bool_t bTemp;
	k_bool_t bReturn;
	char name[32];
};

struct ksp_scope_s
{
	ksp_scope_t* parent;
	k_hash_table_t vars;
};

struct ksp_runner_s {
	ksp_scope_t* scope;
	ksp_parser_t* parser;
	k_mpool_t* pool;
};



k_status_t ksp_scope_enter(ksp_runner_t* runner);
k_status_t ksp_scope_leave(ksp_runner_t* runner);

k_status_t ksp_runner_init(ksp_runner_t* runner, ksp_parser_t* parser);
k_status_t ksp_runner_run(ksp_runner_t* runner);
k_status_t ksp_runner_destroy(ksp_runner_t* runner);

#endif

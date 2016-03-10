#include "ksp_runner.h"


static void* get_var_key(void* val)
{

}


k_status_t ksp_scope_enter(ksp_runner_t* runner)
{
	ksp_scope_t* scope = (ksp_scope_t*)k_mpool_malloc(runner->pool,sizeof(ksp_scope_t));
	scope->parent = runner->scope;
	k_hash_init_string(&scope->vars, runner->pool, 1024, K_NULL);
	return K_SUCCESS;
}
//TODO ÄÚ´æĞ¹Â¶ÎÊÌâ
k_status_t ksp_scope_leave(ksp_runner_t* runner)
{
	return K_SUCCESS;
}

k_status_t ksp_runner_init(ksp_runner_t* runner, ksp_parser_t* parser)
{
	runner->scope = K_NULL;
	runner->parser = parser;
	runner->pool = k_mpool_create("runner pool", 1024, 1024);
}

k_status_t ksp_runner_run(ksp_runner_t* runner)
{
	ksp_scope_enter(runner);

	ksp_scope_leave(runner);
	return K_SUCCESS;
}

k_status_t ksp_runner_destroy(ksp_runner_t* runner)
{

}

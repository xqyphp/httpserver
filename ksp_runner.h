#ifndef ksp_runner_h
#define ksp_runner_h
#include "ksp_parser.h"

enum ksp_val_type_e {
	VAL_FUNCTION,
	VAL_INT,
	VAL_STRING,
	VAL_OBJ,
	VAL_ARR,
	VAL_UNKNOW
};

k_status_t ksp_parser_run(ksp_parser_t* parser);

#endif

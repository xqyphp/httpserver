#ifndef ktypes_h
#define ktypes_h

#include <stddef.h>
#include <errno.h>

#include "k_cstd.h"

enum k_bool_enum{
	K_FALSE = 0,
	K_TRUE  = 1
};

enum k_status_enum {
	K_SUCCESS = 0,
	K_ERROR = -1
};

#define K_NULL         ((void*)0)


typedef int(*k_compare_t)(void* left, void* right);
typedef void*            (*k_getkey_t)(void* val);
typedef int(*k_gethash_t)(void* hash_key,int max_size);

typedef int					k_errno_t;
typedef unsigned int		k_size_t;
typedef enum k_bool_enum	k_bool_t;
typedef enum k_status_enum	k_status_t;

typedef struct k_list_s		k_list_t;

typedef struct k_buffer_s   k_buffer_t;

typedef struct socket_event_s socket_event_t;
typedef struct event_manager_s event_manager_t;
typedef int(*event_callback)(socket_event_t* data);

typedef struct k_hash_entry_s k_hash_entry_t;
typedef struct k_hash_bucket_s k_hash_bucket_t;
typedef struct k_hash_table_s k_hash_table_t;

typedef enum k_color_type_e k_color_type_t;
typedef struct  k_rbnode_s k_rbnode_t;
typedef struct  k_rbtree_s k_rbtree_t;


#endif /* ktypes_h */

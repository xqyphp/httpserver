

#ifndef k_list_h
#define k_list_h

#include "k_types.h"

//list
#define DEF_LIST_HEAD(list_type) \
    list_type* prev; \
    list_type* next

struct k_list_s {
	DEF_LIST_HEAD(struct k_list_s);
};

void
k_list_init(k_list_t* k_list);

void
k_list_link(k_list_t* prev, k_list_t* next);

//return all the list element, if you want data not include header,
//then you can call k_list_get_valid_size.
k_size_t
k_list_get_size(k_list_t* list_val);

k_size_t
k_list_get_valid_size(k_list_t* list_val);


k_list_t*
k_list_get_first(k_list_t* list_header);

k_list_t*
k_list_get_last(k_list_t* list_header);

k_list_t*
k_list_get_prev(k_list_t* list_val);

k_list_t*
k_list_get_next(k_list_t* list_val);


void
k_list_insert_before(k_list_t* pos,k_list_t* val);

void
k_list_insert_after(k_list_t* pos,k_list_t* val);

k_list_t*
k_list_remove(k_list_t* pos);


#endif /* k_list_h */

#include "k_list.h"
#include <assert.h>
void
k_list_init(k_list_t* k_list)
{
     k_list->prev = k_list;
     k_list->next = k_list;
}

void
k_list_link(k_list_t* prev, k_list_t* next)
{
     prev->next = next;
     next->prev = prev;
}

k_size_t
k_list_get_size(k_list_t* list_val)
{
	int count = 1;
	k_list_t* list_next = list_val;
	while (list_next->next != list_val)
	{
		++count;
		list_next = list_next->next;
	}
	return count;
}

k_size_t
k_list_get_valid_size(k_list_t* list_val)
{
	return k_list_get_size(list_val) - 1;
}

k_list_t*
k_list_get_first(k_list_t* list_header)
{
	return list_header->next;
}

k_list_t*
k_list_get_last(k_list_t* list_header)
{
	return list_header->prev;
}

k_list_t*
k_list_get_prev(k_list_t* list_val) 
{
	return list_val->prev;
}

k_list_t*
k_list_get_next(k_list_t* list_val)
{
	return list_val->next;
}

void
k_list_insert_before(k_list_t* pos,k_list_t* val)
{
     k_list_t* list_temp = pos->prev;
     k_list_link(list_temp,val);
     k_list_link(val,pos);
	 assert(pos->prev == val);
	 assert(val->next == pos);
}

void
k_list_insert_after(k_list_t* pos,k_list_t* val)
{
     k_list_insert_before(pos->next,val);
}

k_list_t*
k_list_remove(k_list_t* pos)
{
     k_list_link(pos->prev,pos->next);
     return pos;
}





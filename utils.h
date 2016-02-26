/***************************************************************************
*            utils.h
*
*  Tue January 05 01:22:53 2016
*  Copyright  2016  lk
*  <user@host>
****************************************************************************/
/*
* utils.h
*
* Copyright (C) 2016 - lk
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include "types.h"
#ifdef __cplusplus
extern "C"
{
#endif
#define DEF_LIST_HEADER \
     struct list_s* prev;			\
     struct list_s* next

#define DEF_LIST_HEADER_TYPE(list_type) \
     struct list_type* prev;			\
     struct list_type* next

	typedef struct list_s
	{
		DEF_LIST_HEADER;
	}list_t;

#define POOL_NAME_LEN 16

	typedef struct block_s
	{
		DEF_LIST_HEADER;
		int   mem_size;
		int   debug_index;
		char* cur_pos;
	}block_t;

	typedef struct pool_s
	{
		char pool_name[POOL_NAME_LEN];
		int init_size;
		int increase_size;
		int cur_size;
		list_t blocks;
		block_t* current_block;
		int block_count;
	}pool_t;

	typedef struct buffer_s {
		void*  data_ptr;
		size_t data_used;
		size_t data_len;
		pool_t* pool;
	}buffer_t;

	void list_init(list_t* list_val);
	void list_link(list_t* list_before, list_t* list_after);
	int  list_get_size(list_t* list_val);
	void list_insert_before(list_t* list_pos, list_t* list_val);
	void list_insert_after(list_t* list_pos, list_t* list_val);

	pool_t* pool_create(const char* name, int init_size, int increase);
	int     pool_destory(pool_t* pool);
	void*   pool_malloc(pool_t* pool, int rq_size);
	void*   pool_malloc_fast(pool_t* pool, int rq_size);

	void buffer_init(buffer_t* buffer, pool_t* pool, size_t init_size);
	void buffer_write(buffer_t* buffer, void* data_ptr, size_t data_len);
	void* buffer_data(buffer_t* buffer);
	void* buffer_len(buffer_t* buffer);
#ifdef __cplusplus
}
#endif
#endif // UTILS_H_INCLUDED

/***************************************************************************
*            k_mpool.h
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
#ifndef k_mpool_h
#define k_mpool_h

#include "k_types.h"
#include "k_list.h"

#define POOL_NAME_LEN 16

#ifdef __cplusplus
extern "C"
{
#endif

	typedef struct block_s
	{
		DEF_LIST_HEAD(struct block_s);
		int   data_size;
		int   debug_index;
		char* cur_pos;
	}block_t;

	typedef struct k_mpool_s
	{
		char pool_name[POOL_NAME_LEN];
		int init_size;
		int increase_size;
		int cur_size;
		k_list_t blocks;
		block_t* current_block;
		int block_count;
	}k_mpool_t;

	k_mpool_t*	k_mpool_create(const char* name, int init_size, int increase);
	k_status_t	k_mpool_destory(k_mpool_t* pool);
	k_status_t  k_mpool_reuse(k_mpool_t* pool);
	void*		k_mpool_malloc(k_mpool_t* pool, int rq_size);
	void*		k_mpool_malloc_fast(k_mpool_t* pool, int rq_size);

#ifdef __cplusplus
}
#endif
#endif // UTILS_H_INCLUDED

/***************************************************************************
*            k_hash.h
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
#ifndef k_hash_h
#define k_hash_h

#include "k_types.h"
#include "k_list.h"
#include "k_mpool.h"

struct k_hash_entry_s 
{
	DEF_LIST_HEAD(struct k_hash_entry_s);
	struct k_hash_bucket_s* parent;
};

struct k_hash_bucket_s
{
	DEF_LIST_HEAD(struct k_hash_bucket_s);
	k_hash_entry_t hash_entries;
	k_size_t hash_code;
};

struct k_hash_table_s
{
	k_hash_bucket_t** buckets;
	k_hash_bucket_t values;
	k_size_t buckets_count;
	k_gethash_t gethash;
	k_getkey_t  getkey;
	k_compare_t compare;
	k_mpool_t* pool;
};

k_size_t str_hash(const char* str_val);
k_size_t int_hash(const int*  int_val);

k_status_t k_hash_init_string(k_hash_table_t* hash_table, k_mpool_t* pool,
	k_size_t hash_arr_size, k_getkey_t fn_getkey);

k_status_t k_hash_init_integer(k_hash_table_t* hash_table, k_mpool_t* pool,
	k_size_t hash_arr_size, k_getkey_t fn_getkey);


k_status_t k_hash_init(k_hash_table_t* hash_table,k_mpool_t* pool,
	k_size_t hash_arr_size,k_getkey_t fn_getkey,
	k_gethash_t fn_gethash, k_compare_t fn_compare);

k_status_t k_hash_destroy(k_hash_table_t* hash_table);

k_status_t k_hash_add(k_hash_table_t* hash_table, k_hash_entry_t* val);

k_hash_entry_t* k_hash_get(k_hash_table_t* hash_table, void* key);

k_hash_entry_t* k_hash_remove(k_hash_table_t* hash_table, void* key);

#endif

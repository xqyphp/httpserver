/***************************************************************************
*            utils.c
*
*  Tue January 05 01:22:53 2016
*  Copyright  2016  lk
*  <user@host>
****************************************************************************/
/*
* utils.c
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
#include "utils.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

void list_init(list_t* list_val)
{
	list_val->prev = list_val;
	list_val->next = list_val;
}

void list_link(list_t* list_before, list_t* list_after)
{
	list_before->next = list_after;
	list_after->prev = list_before;
}

int  list_get_size(list_t* list_val)
{
	int count = 1;
	list_t* list_next = list_val;
	while (list_next->next != list_val)
	{
		++count;
		list_next = list_next->next;
	}
	return count;
}

void list_insert_before(list_t* list_pos, list_t* list_val)
{
	list_insert_after(list_pos->prev, list_val);
}

void list_insert_after(list_t* list_pos, list_t* list_val)
{
	list_t* list_temp_after = list_pos->next;
	list_link(list_pos, list_val);
	list_link(list_val, list_temp_after);
}

int pool_block_get_free(block_t* block)
{
	int used = block->cur_pos - ((char*)block + sizeof(block_t));
	return block->mem_size - used;
}

void pool_add_block(pool_t* pool, block_t* block, int block_size)
{
	LOG("add block->index = %d , size = %d\n", block->debug_index, block_size);
	block->mem_size = block_size;
	block->cur_pos = ((char*)block) + sizeof(block_t);
	list_insert_after((list_t*)&(pool->blocks), (list_t*)block);
}


block_t* pool_find_block(pool_t* pool, int rq_size)
{
	block_t* block = (block_t*)pool->blocks.next;
	while ((void*)block != &(pool->blocks)) {
		block_t* block_next = (block_t*)block->next;
		int mem_free = pool_block_get_free(block);
		if (mem_free >= rq_size) {
			LOG("pool_find_block->index = %d , size = %d,free=%d\n", block->debug_index, block->mem_size, mem_free);
			return block;
		}
		block = block_next;
	}
	LOG("pool_find_block->null.\n");
	return NULL;
}

block_t* pool_new_block(pool_t* pool, int block_size)
{

	block_t* block = (block_t*)malloc(sizeof(block_t) + block_size);
	block->debug_index = pool->block_count++;
	pool_add_block(pool, block, block_size);

	LOG("pool_new_block->index = %d , size = %d.\n", block->debug_index, block->mem_size);

	return block;
}

pool_t* pool_create(const char* name, int init_size, int increase)
{
	LOG("pool_create -> %s,size=%d,increase=%d.\n", name, init_size, increase);
	pool_t* pool = (pool_t*)malloc(sizeof(pool_t));
	assert(strlen(name) < POOL_NAME_LEN);
	strcpy(pool->pool_name, name);
	pool->init_size = init_size;
	pool->cur_size = init_size;
	pool->increase_size = increase;
	pool->block_count = 0;
	pool->current_block = NULL;
	list_init(&(pool->blocks));
	pool_new_block(pool, init_size);
	return pool;
}

void*   pool_malloc(pool_t*pool, int rq_size)
{
	LOG("pool_malloc -> rq_size=%d.\n", rq_size);
	block_t* block = pool_find_block(pool, rq_size);
	if (block == NULL) {
		block = pool_new_block(pool, pool->increase_size > rq_size ? pool->increase_size : rq_size);
		assert(block != NULL);
	}
	pool->current_block = block;
	void* ptr_mem = block->cur_pos;
	block->cur_pos += rq_size;
	LOG("pool_malloc return -> index =%d.\n", block->debug_index);
	return ptr_mem;
}

void*   pool_malloc_fast(pool_t* pool, int rq_size)
{
	LOG("pool_malloc_fast -> rq_size=%d.\n", rq_size);

	block_t* block = pool->current_block;
	if (block == NULL || pool_block_get_free(block) < rq_size) {
		block = pool_new_block(pool, pool->increase_size > rq_size ? pool->increase_size : rq_size);
		assert(block != NULL);
	}
	pool->current_block = block;
	void* ptr_mem = block->cur_pos;
	block->cur_pos += rq_size;
	LOG("pool_malloc return -> index =%d.\n", block->debug_index);
	return ptr_mem;
}

int     pool_destory(pool_t* pool)
{
	LOG("destory pool -> %s\n", pool->pool_name);
	block_t* block = (block_t*)pool->blocks.next;
	while ((void*)block != &(pool->blocks)) {
		block_t* block_next = (block_t*)block->next;
		LOG("free_block -> index =%d.\n", block->debug_index);
		free(block);
		block = block_next;
	}
	LOG("free_pool -> %s.\n", pool->pool_name);
	free(pool);
	return 1;
}

void buffer_init(buffer_t* buffer, pool_t* pool, size_t init_size)
{
	buffer->pool = pool;
	buffer->data_ptr = pool_malloc(pool,init_size);
	memset(buffer->data_ptr, 0, init_size);
	buffer->data_len = init_size;
	buffer->data_used = 0;
}

void buffer_write(buffer_t* buffer, void* data_ptr, size_t data_len)
{
	if (buffer->data_len - buffer->data_used < data_len) {
		char* odata = buffer->data_ptr;
		size_t olen = buffer->data_len;
		buffer->data_ptr = pool_malloc(buffer->pool, buffer->data_len + data_len);
		buffer->data_len = olen + data_len;
		memcpy(buffer->data_ptr, odata, olen);
	}
	memcpy((char*)buffer->data_ptr + buffer->data_used, data_ptr, data_len);
	buffer->data_used += data_len;
	return K_SUCCESS;
}

void* buffer_data(buffer_t* buffer)
{
	return buffer->data_ptr;
}
void* buffer_len(buffer_t* buffer)
{
	return buffer->data_used;
}
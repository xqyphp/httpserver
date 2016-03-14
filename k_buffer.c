#include "k_buffer.h"
#include <string.h>

k_status_t k_buffer_init(k_buffer_t* buffer, k_mpool_t* pool, k_size_t init_size)
{
	buffer->pool = pool;
	buffer->data_ptr = k_mpool_malloc(pool, init_size);
	memset(buffer->data_ptr, 0, init_size);
	buffer->data_len = init_size;
	buffer->data_used = 0;
	return K_SUCCESS;
}

k_status_t k_buffer_write(k_buffer_t* buffer, const void* data_ptr, k_size_t data_len)
{
	if (buffer->data_len - buffer->data_used <= data_len) {
		char* odata = buffer->data_ptr;
		k_size_t olen = buffer->data_len;
		buffer->data_ptr = k_mpool_malloc(buffer->pool, buffer->data_len + data_len *2);
		memset(buffer->data_ptr, 0, buffer->data_len + data_len *2);
		buffer->data_len = olen + data_len;
		memcpy(buffer->data_ptr, odata, olen);
	}
	memcpy((char*)buffer->data_ptr + buffer->data_used, data_ptr, data_len);
	buffer->data_used += data_len;
	return K_SUCCESS;
}

k_status_t  k_buffer_write_ch(k_buffer_t* buffer, char ch)
{
	return k_buffer_write(buffer, &ch, 1);
}

void* k_buffer_get_data(k_buffer_t* buffer)
{
	return buffer->data_ptr;
}
k_size_t k_buffer_get_len(k_buffer_t* buffer)
{
	return buffer->data_used;
}
#include "k_cstd.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

void *k_calloc(int num_elems, int elem_size)
{
	return calloc(num_elems, elem_size);
}

void k_free(void *p)
{
	 free(p);
}

void *k_malloc(unsigned int num_bytes)
{
	return malloc(num_bytes);
}

void *k_realloc(void *mem_address, unsigned int newsize)
{
	return realloc(mem_address, newsize);
}

enum log_level_e g_log_level = LEVEL_DEBUG;

static const char* get_level_string(enum log_level_e l)
{
	switch (l)
	{
	case LEVEL_DEBUG:
		return "debug";
	case LEVEL_INFO:
		return "info";
	case LEVEL_WARN:
		return "warn";
	case LEVEL_ERROR:
		return "error";
	case LEVEL_FATAL:
		return "fatal";
	default:
		break;
	}
	return "unknow";
}

void log_level(enum log_level_e l, const char* logformat, ...) {
	if (l < g_log_level) {
		return;
	}

	int size;
	char buffer[1024];

	va_list args;
	va_start(args, logformat);
	size = vsnprintf(buffer,1204,logformat, args);
	va_end(args);

	printf("[%s]:%s\n",get_level_string(l),buffer);
}
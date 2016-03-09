#ifndef k_cstd_h
#define k_cstd_h

enum log_level_e
{
	LEVEL_DEBUG,
	LEVEL_INFO,
	LEVEL_WARN,
	LEVEL_ERROR,
	LEVEL_FATAL
};

void*	k_calloc(int num_elems, int elem_size);
void	k_free(void *p);
void*	k_malloc(unsigned int num_bytes);
void*	k_realloc(void *mem_address, unsigned int newsize);

void    log_level(enum log_level_e level, const char* fromat, ...);

#ifdef linux

#define log_debug(log_fmt,log_arg...) \
	log_level(LEVEL_DEBUG,log_fmt, ##log_arg)

#define log_info(log_fmt,log_arg...) \
	log_level(LEVEL_INFO,log_fmt, ##log_arg)

#define log_warn(log_fmt,log_arg...) \
	log_level(LEVEL_WARN,log_fmt, ##log_arg)

#define log_error(log_fmt,log_arg...) \
	log_level(LEVEL_ERROR,log_fmt, ##log_arg)

#define log_fatal(log_fmt,log_arg...) \
	log_level(LEVEL_FATAL,log_fmt, ##log_arg)

#else

#define log_debug(log_fmt,...) \
	log_level(LEVEL_DEBUG,log_fmt,__VA_ARGS__)

#define log_info(log_fmt,...) \
	log_level(LEVEL_INFO,log_fmt,__VA_ARGS__)

#define log_warn(log_fmt,...) \
	log_level(LEVEL_WARN,log_fmt,__VA_ARGS__)

#define log_error(log_fmt,...) \
	log_level(LEVEL_ERROR,log_fmt,__VA_ARGS__)

#define log_fatal(log_fmt,...) \
	log_level(LEVEL_FATAL,log_fmt,__VA_ARGS__)

#endif

#endif

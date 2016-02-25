//
//  types.h
//  klib
//
//  Created by LiKai on 16/1/8.
//  Copyright © 2016年 LiKai. All rights reserved.
//

#ifndef types_h
#define types_h

#include <stddef.h>

#define K_SUCCESS      0
#define K_ERROR       -1
#define K_TRUE         1
#define K_FALSE        0
#define K_NULL         ((void*)0)

#define LOG(...) 
//#define LOG(...) printf(__VA_ARGS__)

typedef unsigned char  bool_t;
typedef int            	status_t;
typedef struct 			pool_s pool_t;
typedef struct list_s    list_t;
#ifndef errno_t
typedef int            errno_t;
#endif

#endif /* types_h */

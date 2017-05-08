
#ifndef _OSAPI_INTTYPE_H
#define _OSAPI_INTTYPE_H

#include <stdio.h>

typedef unsigned char OS_UINT8;
typedef unsigned short OS_UINT16;
typedef unsigned int OS_UINT32;

typedef char OS_INT8;
typedef short OS_INT16;
typedef int OS_INT32;

/* 64位整型: 字面常量 */

#ifndef INT64_C
#define INT64_C(c)  c ## LL
#endif

/* 64位整型: 变量定义 */
#ifdef _WIN32
typedef __int64                OS_INT64;
typedef unsigned __int64    OS_UINT64;

// printf和scanf的格式符号
#define PRIi64 "I64d"
#define PRIu64 "I64d"

#else
typedef long long            OS_INT64;
typedef unsigned long long   OS_UINT64;

#define PRIi64 "lld"
#define PRIu64 "llu"
#endif

/**** usage *****
printf("%"PRIu64".\n", x );

*****************/

#endif




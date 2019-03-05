#pragma once

typedef char s8;
typedef short s16;
typedef int s32;
typedef long long s64;
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef float r32;
typedef double r64;
typedef unsigned long umm;
typedef int bool;

#define FLAG(X) (1 << X)
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define OFFSET_OF(T, NAME) (&((T*)0)->NAME)
#define ARRAY_SIZE(N) sizeof(N)
#define ARRAY_LENGTH(N) (sizeof(N) / sizeof(*(N)))

#define true 1
#define false 0

#include <stdlib.h>
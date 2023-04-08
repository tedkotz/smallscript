#ifndef SMALLC_H
#define SMALLC_H
#include<stdio.h>

//#define intptr_t int

#ifdef SMALL_C_COMP

#define intptr_t int
#define SIZEOFINT sizeof(int)
#define PRIdPTR
#define PRIxPTR
#define SCNdPTR

#else

#include<stdlib.h>
#include<inttypes.h>
#include<string.h>
#include<stdbool.h>
#define SIZEOFINT sizeof(intptr_t)

#endif

#endif

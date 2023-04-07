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

#include<inttypes.h>
#define SIZEOFINT sizeof(intptr_t)

#endif

#endif

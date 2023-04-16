#ifndef SMALLC_H
#define SMALLC_H
/**
 * The purpose of this header is to consolidate the code needed to abstract the
 * differences between small-C ( a super minimal pre-ansi 8/16-bit compiler) and
 * gcc (a modern 32/64-bit compiler )
 */

#include<stdio.h>


#ifdef SMALL_C_COMP

#define intptr_t int
#define SIZEOFINT sizeof(int)

#define PRIdPTR "d"
#define PRIxPTR "x"
#define PRIXPTR "X"
#define SCNdPTR "d"

#define unused

/* compiler won't take restrict hint anyway */
#define restrict

/* file number is File Pointer Value */
#define fileno

/*
 * missing functions
 * #define strtoll atoib args don't match
 * snprintf
 */

#else

#include<stdlib.h>
#include<inttypes.h>
#include<string.h>
#include<stdbool.h>

#define SIZEOFINT sizeof(intptr_t)
#define unused __attribute__((__unused__))

/* needed by lex, but not in std C */
extern int fileno(const FILE *stream);

#if (!defined(__STDC__)) || (__STDC_VERSION__ < 199901)
/* #pragma message("Pre-C99 Compiler.") */

#define restrict
/* #define strtoll strtol */
extern int snprintf( char *restrict buffer, size_t bufsz, const char *restrict format, ... );
extern long long strtoll( const char *restrict str, char **restrict str_end, int base );
#endif

#endif

#endif

#ifndef RE_BITS_API_H
#define RE_BITS_API_H

#include "re_config.h"

/* bit: version */
#define RE_VERSION_MAJOR 0
#define RE_VERSION_MINOR 0
#define RE_VERSION_PATCH 1
#define RE_VERSION_STRING "0.0.1"

/* bit: exports */
#if RE_STATIC
    #define RE_API static
#else
    #define RE_API extern
#endif

/* bit: cstd */
/* If __STDC__ is not defined, assume C89. */
#ifndef __STDC__
  #define RE__CSTD 1989
#else
  #if defined(__STDC_VERSION__)
    #if __STDC_VERSION__ >= 201112L
      #define RE__CSTD 2011
    #elif __STDC_VERSION__ >= 199901L
      #define RE__CSTD 1999
    #else
      #define RE__CSTD 1989
    #endif
  #endif
#endif

/* bit: hook_malloc */
#if !RE_USE_CUSTOM_MALLOC
#include <stdlib.h>
#define RE_MALLOC malloc
#define RE_REALLOC realloc
#define RE_FREE free
#else
#if !defined(RE_MALLOC) || !defined(RE_REALLOC) || !defined(RE_FREE)
#error In order to use RE_USE_CUSTOM_MALLOC you must provide defnitions for RE_MALLOC, RE_REALLOC and RE_FREE.
#endif
#endif

/* bit: null */
#define RE_NULL 0

/* bit: sizet */
#if !RE_USE_CUSTOM_SIZE_TYPE
#include <stddef.h>
#define RE_SIZE_TYPE size_t
#else
#if !defined(RE_SIZE_TYPE)
#error In order to use RE_USE_CUSTOM_SIZE_TYPE you must provide a definition for RE_SIZE_TYPE.
#endif
#endif
typedef RE_SIZE_TYPE re_size;

/* bit: hook_assert */
#if !RE_USE_CUSTOM_ASSERT
#include <assert.h>
#define RE_ASSERT assert
#else
#if !defined(RE_ASSERT)
#error In order to use RE_USE_CUSTOM_ASSERT you must provide a definition for RE_ASSERT.
#endif
#endif

/* bit: unused */
#define RE__UNUSED(i) (void)(i)

/* bit: inttypes */
#if RE__CSTD >= 1999
#include <stdint.h>
typedef uint8_t re_uint8;
typedef int8_t re_int8;
typedef uint16_t re_uint16;
typedef int16_t re_int16;
typedef uint32_t re_uint32;
typedef int32_t re_int32;
#else
typedef unsigned char re_uint8;
typedef signed char re_int8;
typedef unsigned short re_uint16;
typedef signed short re_int16;
typedef unsigned int re_uint32;
typedef signed int re_int32;
#endif

#endif

#if !defined(MN__RE_API_H)
#define MN__RE_API_H

#include "../tests/test_config.h"
#include "config.h"

/* bits/types/size */
typedef MN_SIZE_TYPE mn_size;

/* bits/util/cstd */
/* If __STDC__ is not defined, assume C89. */
#ifndef __STDC__
    #define MN__CSTD 1989
#else
    #if defined(__STDC_VERSION__)
        #if __STDC_VERSION__ >= 201112L
            #define MN__CSTD 2011
        #elif __STDC_VERSION__ >= 199901L
            #define MN__CSTD 1999
        #else
            #define MN__CSTD 1989
        #endif
    #endif
#endif

/* bits/util/debug */
#if !MN_DEBUG
#if defined(NDEBUG)
#if NDEBUG == 0
#define MN_DEBUG 1
#else
#define MN_DEBUG 0
#endif
#endif
#endif

/* bits/util/exports */
#if !defined(MN__SPLIT_BUILD)
#if MN_STATIC
#define MN_API static
#else
#define MN_API extern
#endif
#else
#define MN_API extern
#endif

/* bits/util/null */
#define MN_NULL 0

/* bits/types/char */
#if !defined(MN_CHAR_TYPE)
#define MN_CHAR_TYPE char
#endif

typedef MN_CHAR_TYPE mn_char;

/* bits/types/fixed/int32 */
#if !defined(MN_INT32_TYPE)
#if MN__CSTD >= 1999
#include <stdint.h>
#define MN_INT32_TYPE int32_t
#else
#define MN_INT32_TYPE signed int
#endif
#endif

typedef MN_INT32_TYPE mn_int32;

/* bits/types/fixed/uint32 */
#if !defined(MN_UINT32_TYPE)
#if MN__CSTD >= 1999
#include <stdint.h>
#define MN_UINT32_TYPE uint32_t
#else
#define MN_UINT32_TYPE unsigned int
#endif
#endif

typedef MN_UINT32_TYPE mn_uint32;

/* bits/types/fixed/uint8 */
#if !defined(MN_UINT8_TYPE)
#if MN__CSTD >= 1999
#include <stdint.h>
#define MN_UINT8_TYPE uint8_t
#else
#define MN_UINT8_TYPE unsigned char
#endif
#endif

typedef MN_UINT8_TYPE mn_uint8;

#endif /* MN__RE_API_H */

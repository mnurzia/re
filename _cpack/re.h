#if !defined(RE_H)
#define RE_H

/* Set to 1 in order to define malloc(), free(), and realloc() replacements. */
#if !defined(RE_USE_CUSTOM_ALLOCATOR)
#include <stdlib.h>
#define RE_MALLOC malloc
#define RE_REALLOC realloc
#define RE_FREE free
#endif

/* bits/types/size */
/* desc */
/* cppreference */
#if !defined(RE_SIZE_TYPE)
#include <stdlib.h>
#define RE_SIZE_TYPE size_t
#endif

/* bits/util/debug */
/* Set to 1 in order to override the setting of the NDEBUG variable. */
#if !defined(RE_DEBUG)
#define RE_DEBUG 0
#endif

/* bits/hooks/assert */
/* desc */
/* cppreference */
#if !defined(RE_ASSERT)
#include <assert.h>
#define RE_ASSERT assert
#endif

/* bits/util/exports */
/* Set to 1 in order to define all symbols with static linkage (local to the
 * including source file) as opposed to external linkage. */
#if !defined(RE_STATIC)
#define RE_STATIC 0
#endif

/* bits/types/char */
/* desc */
/* cppreference */
#if !defined(RE_CHAR_TYPE)
#define RE_CHAR_TYPE char
#endif

/* bits/types/fixed/int32 */
/* desc */
/* cppreference */
#if !defined(RE_INT32_TYPE)
#endif

/* bits/types/fixed/uint16 */
/* desc */
/* cppreference */
#if !defined(RE_UINT16_TYPE)
#endif

/* bits/types/fixed/uint32 */
/* desc */
/* cppreference */
#if !defined(RE_UINT32_TYPE)
#endif

/* bits/types/fixed/uint8 */
/* desc */
/* cppreference */
#if !defined(RE_UINT8_TYPE)
#endif

/* bits/types/size */
typedef RE_SIZE_TYPE re_size;

/* bits/util/cstd */
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

/* bits/util/debug */
#if !RE_DEBUG
#if defined(NDEBUG)
#if NDEBUG == 0
#define RE_DEBUG 1
#else
#define RE_DEBUG 0
#endif
#endif
#endif

/* bits/hooks/assert */
#ifndef RE__HOOKS_ASSERT_INTERNAL_H
#define RE__HOOKS_ASSERT_INTERNAL_H
#if defined(RE__COVERAGE) || !RE_DEBUG
#undef RE_ASSERT
#define RE_ASSERT(e) ((void)0)
#endif

#endif /* RE__HOOKS_ASSERT_INTERNAL_H */

/* bits/util/exports */
#if !defined(RE__SPLIT_BUILD)
#if RE_STATIC
#define RE_API static
#else
#define RE_API extern
#endif
#else
#define RE_API extern
#endif

/* bits/util/null */
#define RE_NULL 0

/* bits/types/char */
#if !defined(RE_CHAR_TYPE)
#define RE_CHAR_TYPE char
#endif

typedef RE_CHAR_TYPE re_char;

/* bits/types/fixed/int32 */
#if !defined(RE_INT32_TYPE)
#if RE__CSTD >= 1999
#include <stdint.h>
#define RE_INT32_TYPE int32_t
#else
#define RE_INT32_TYPE signed int
#endif
#endif

typedef RE_INT32_TYPE re_int32;

/* bits/types/fixed/uint16 */
#if !defined(RE_UINT16_TYPE)
#if RE__CSTD >= 1999
#include <stdint.h>
#define RE_UINT16_TYPE uint16_t
#else
#define RE_UINT16_TYPE unsigned short
#endif
#endif

typedef RE_UINT16_TYPE re_uint16;

/* bits/types/fixed/uint32 */
#if !defined(RE_UINT32_TYPE)
#if RE__CSTD >= 1999
#include <stdint.h>
#define RE_UINT32_TYPE uint32_t
#else
#define RE_UINT32_TYPE unsigned int
#endif
#endif

typedef RE_UINT32_TYPE re_uint32;

/* bits/types/fixed/uint8 */
#if !defined(RE_UINT8_TYPE)
#if RE__CSTD >= 1999
#include <stdint.h>
#define RE_UINT8_TYPE uint8_t
#else
#define RE_UINT8_TYPE unsigned char
#endif
#endif

typedef RE_UINT8_TYPE re_uint8;

/* re */
#ifndef RE_API_H
#define RE_API_H
typedef int re_error;
typedef re_int32 re_rune;

#define RE_RUNE_MAX 0x10FFFF

#define RE_ERROR_NONE 0
#define RE_ERROR_MATCH 0
#define RE_ERROR_NOMEM -1
#define RE_ERROR_INVALID -2
#define RE_ERROR_PARSE -3
#define RE_ERROR_COMPILE -4
#define RE_ERROR_NOMATCH -5
#define RE_ERROR_INTERNAL -6

#define RE_MATCH 0
#define RE_NOMATCH RE_ERROR_NOMATCH

typedef struct re_data re_data;

typedef struct re {
  re_data* data;
} re;

typedef struct re_span {
  re_size begin;
  re_size end;
} re_span;

typedef enum re_anchor_type {
  RE_ANCHOR_BOTH = 'B',
  RE_ANCHOR_START = 'S',
  RE_ANCHOR_END = 'E',
  RE_UNANCHORED = 'U'
} re_anchor_type;

typedef enum re_syntax_flags {
  RE_SYNTAX_FLAG_ASCII = 1,
  RE_SYNTAX_FLAG_IGNORECASE = 2,
  RE_SYNTAX_FLAG_MULTILINE = 4,
  RE_SYNTAX_FLAG_DOTALL = 8,
  RE_SYNTAX_FLAG_GLOB = 16
} re_syntax_flags;

RE_API re_error re_init(re* reg, const char* regex_nt);
RE_API re_error
re_init_flags(re* reg, const char* regex_nt, re_syntax_flags syntax_flags);
RE_API re_error re_init_sz_flags(
    re* reg, const char* regex, re_size regex_size,
    re_syntax_flags syntax_flags);
RE_API re_error re_init_set(re* reg);

RE_API re_error re_set_add(re* reg, const char* regex_nt);
RE_API re_error
re_set_add_flags(re* reg, const char* regex_nt, re_syntax_flags syntax_flags);
RE_API re_error re_set_add_sz_flags(
    re* reg, const char* regex, re_size regex_size,
    re_syntax_flags syntax_flags);

RE_API re_error re_is_match(
    re* reg, const char* text, re_size text_size, re_anchor_type anchor_type);
RE_API re_error re_match_groups(
    re* reg, const char* text, re_size text_size, re_anchor_type anchor_type,
    re_uint32 max_group, re_span* out_groups);
RE_API re_error re_match_groups_set(
    re* reg, const char* text, re_size text_size, re_anchor_type anchor_type,
    re_uint32 max_group, re_span* out_groups, re_uint32* out_set_index);

RE_API const char* re_get_error(const re* reg, re_size* error_len);
RE_API re_uint32 re_get_max_groups(const re* reg);

RE_API void re_destroy(re* reg);

#endif

#endif /* RE_H */

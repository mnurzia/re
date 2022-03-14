#if !defined(MN__RE_CONFIG_H)
#define MN__RE_CONFIG_H

/* Redefine to your own assert() implementation. */
/* See https://www.cppreference.com/w/c/error/assert for details. */
#if !defined(MN_ASSERT)
#include <assert.h>
#define MN_ASSERT assert
#endif

/* hooks/malloc */
/* Set to 1 in order to define malloc(), free(), and realloc() replacements. */
#if defined(MN_USE_CUSTOM_ALLOCATOR)
#define MN_MALLOC my_malloc
#define MN_REALLOC my_realloc
#define MN_FREE my_free
#else
#include <stdlib.h>
#define MN_MALLOC malloc
#define MN_REALLOC realloc
#define MN_FREE free
#endif

/* types/size */
/* Size type, should be capable of representing any size on the given
 * platform */
/* See https://www.cppreference.com/w/c/types/size_t for details. */
#if !defined(MN_SIZE_TYPE)
#include <stdlib.h>
#define MN_SIZE_TYPE size_t
#endif

/* util/debug */
/* Set to 1 in order to override the setting of the NDEBUG variable. */
#if !defined(MN_DEBUG)
#define MN_DEBUG 0
#endif

/* util/exports */
/* Set to 1 in order to define all symbols with static linkage (local to the
 * including source file) as opposed to external linkage. */
#if !defined(MN_STATIC)
#define MN_STATIC 0
#endif

/* types/char */
/* Character type, should be compatible with char. */
/* See
 * https://www.cppreference.com/w/c/language/arithmetic_types#Character_types
 * for details. */
#if !defined(MN_CHAR_TYPE)
#define MN_CHAR_TYPE char
#endif

/* types/fixed/int32 */
/* 32-bit signed integer type */
/* See https://www.cppreference.com/w/c/types/integer#Types for details. */
#if !defined(MN_INT32_TYPE)
#endif

/* types/fixed/uint32 */
/* 32-bit unsigned integer type */
/* See https://www.cppreference.com/w/c/types/integer#Types for details. */
#if !defined(MN_UINT32_TYPE)
#endif

/* types/fixed/uint8 */
/* 8-bit unsigned integer type */
/* See https://www.cppreference.com/w/c/types/integer#Types for details. */
#if !defined(MN_UINT8_TYPE)
#endif

#endif /* MN__RE_CONFIG_H */

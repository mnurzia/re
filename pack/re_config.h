#ifndef RE_CONFIG_H
#define RE_CONFIG_H

/* config: STATIC */
/* Whether or not API definitions should be defined as static linkage (local to
 * the including source file), as opposed to external linkage. */
#ifndef RE_STATIC
#define RE_STATIC 0
#endif

/* config: USE_CUSTOM_MALLOC */
/* Set RE_USE_CUSTOM_MALLOC to 1 in order to use your own definitions for
 * malloc(), realloc() and free(). If RE_USE_CUSTOM_MALLOC is set, you must
 * also define RE_MALLOC, RE_REALLOC and RE_FREE. Otherwise, <stdlib.h> is
 * included and standard versions of malloc(), realloc() and free() are used. */
#ifndef RE_USE_CUSTOM_MALLOC
#define RE_USE_CUSTOM_MALLOC 0
#endif

/* config: MALLOC */
/* a malloc() function. Performs a memory allocation. */
/* See https://en.cppreference.com/w/c/memory/malloc for more information. */
#if RE_USE_CUSTOM_MALLOC
#ifndef RE_MALLOC
#define RE_MALLOC MY_MALLOC
#endif
#endif

/* config: REALLOC */
/* a realloc() function. Performs a memory reallocation. */
/* See https://en.cppreference.com/w/c/memory/realloc for more information. */
#if RE_USE_CUSTOM_MALLOC
#ifndef RE_REALLOC
#define RE_REALLOC MY_REALLOC
#endif
#endif

/* config: FREE */
/* a free() function. Returns memory back to the operating system. */
/* See https://en.cppreference.com/w/c/memory/free for more information. */
#if RE_USE_CUSTOM_MALLOC
#ifndef RE_FREE
#define RE_FREE MY_FREE
#endif
#endif

/* config: USE_CUSTOM_SIZE_TYPE */
/* Set RE_USE_CUSTOM_SIZE_TYPE to 1 in order to use your own definition for
 * size_t. If RE_USE_CUSTOM_SIZE_TYPE is set, you must also define
 * RE_SIZE_TYPE. Otherwise, <stddef.h> is included and a standard version of
 * size_t is used. */
#ifndef RE_USE_CUSTOM_SIZE_TYPE
#define RE_USE_CUSTOM_SIZE_TYPE 0
#endif

/* config: SIZE_TYPE */
/* a type that can store any size. */
/* See https://en.cppreference.com/w/c/types/size_t for more information. */
#if RE_USE_CUSTOM_SIZE_TYPE
#ifndef RE_SIZE_TYPE
#define RE_SIZE_TYPE my_size_t
#endif
#endif

/* config: DEBUG */
/* Set RE_DEBUG to 1 if you want to enable debug asserts,  bounds checks, and
 * more runtime diagnostics. */
#ifndef RE_DEBUG
#define RE_DEBUG 1
#endif

/* config: USE_CUSTOM_ASSERT */
/* Set RE_USE_CUSTOM_ASSERT to 1 in order to use your own definition for
 * assert(). If RE_USE_CUSTOM_ASSERT is set, you must also define RE_ASSERT.
 * Otherwise, <assert.h> is included and a standard version of assert() is
 * used. */
#ifndef RE_USE_CUSTOM_ASSERT
#define RE_USE_CUSTOM_ASSERT 0
#endif

/* config: ASSERT */
/* an assert() macro. Expects a single argument, which is a expression that
 * evaluates to either 1 or 0. */
/* See https://en.cppreference.com/w/c/error/assert for more information. */
#if RE_USE_CUSTOM_ASSERT
#ifndef RE_ASSERT
#define RE_ASSERT MY_ASSERT
#endif
#endif

#endif

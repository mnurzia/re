#ifndef RE_H
#define RE_H
/*           
 *           
 *  _ __ ___ 
 * | '__/ _ \
 * | | |  __/
 * |_|  \___|
 *           
 *            */


/* Whether or not API definitions should be defined as static linkage (local to
 * the including source file), as opposed to external linkage. */
#ifndef RE_STATIC
#define RE_STATIC 0
#endif

/* Set RE_USE_CUSTOM_MALLOC to 1 in order to use your own definitions for
 * malloc(), realloc() and free(). If RE_USE_CUSTOM_MALLOC is set, you must
 * also define RE_MALLOC, RE_REALLOC and RE_FREE. Otherwise, <stdlib.h> is
 * included and standard versions of malloc(), realloc() and free() are used. */
#ifndef RE_USE_CUSTOM_MALLOC
#define RE_USE_CUSTOM_MALLOC 0
#endif

/* a malloc() function. Performs a memory allocation. */
/* See https://en.cppreference.com/w/c/memory/malloc for more information. */
#if RE_USE_CUSTOM_MALLOC
#ifndef RE_MALLOC
#define RE_MALLOC MY_MALLOC
#endif
#endif

/* a realloc() function. Performs a memory reallocation. */
/* See https://en.cppreference.com/w/c/memory/realloc for more information. */
#if RE_USE_CUSTOM_MALLOC
#ifndef RE_REALLOC
#define RE_REALLOC MY_REALLOC
#endif
#endif

/* a free() function. Returns memory back to the operating system. */
/* See https://en.cppreference.com/w/c/memory/free for more information. */
#if RE_USE_CUSTOM_MALLOC
#ifndef RE_FREE
#define RE_FREE MY_FREE
#endif
#endif

/* Set RE_USE_CUSTOM_SIZE_TYPE to 1 in order to use your own definition for
 * size_t. If RE_USE_CUSTOM_SIZE_TYPE is set, you must also define
 * RE_SIZE_TYPE. Otherwise, <stddef.h> is included and a standard version of
 * size_t is used. */
#ifndef RE_USE_CUSTOM_SIZE_TYPE
#define RE_USE_CUSTOM_SIZE_TYPE 0
#endif

/* a type that can store any size. */
/* See https://en.cppreference.com/w/c/types/size_t for more information. */
#if RE_USE_CUSTOM_SIZE_TYPE
#ifndef RE_SIZE_TYPE
#define RE_SIZE_TYPE my_size_t
#endif
#endif

/* Set RE_DEBUG to 1 if you want to enable debug asserts,  bounds checks, and
 * more runtime diagnostics. */
#ifndef RE_DEBUG
#define RE_DEBUG 1
#endif

/* Set RE_USE_CUSTOM_ASSERT to 1 in order to use your own definition for
 * assert(). If RE_USE_CUSTOM_ASSERT is set, you must also define RE_ASSERT.
 * Otherwise, <assert.h> is included and a standard version of assert() is
 * used. */
#ifndef RE_USE_CUSTOM_ASSERT
#define RE_USE_CUSTOM_ASSERT 0
#endif

/* an assert() macro. Expects a single argument, which is a expression that
 * evaluates to either 1 or 0. */
/* See https://en.cppreference.com/w/c/error/assert for more information. */
#if RE_USE_CUSTOM_ASSERT
#ifndef RE_ASSERT
#define RE_ASSERT MY_ASSERT
#endif
#endif


#define RE_VERSION_MAJOR 0
#define RE_VERSION_MINOR 0
#define RE_VERSION_PATCH 1
#define RE_VERSION_STRING "0.0.1"

#if RE_STATIC
    #define RE_API static
#else
    #define RE_API extern
#endif

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
    #endif /*     #if __STDC_VERSION__ >= 201112L */
  #endif /*   #if defined(__STDC_VERSION__) */
#endif /* #ifndef __STDC__ */

#if !RE_USE_CUSTOM_MALLOC
#include <stdlib.h>
#define RE_MALLOC malloc
#define RE_REALLOC realloc
#define RE_FREE free
#else
#if !defined(RE_MALLOC) || !defined(RE_REALLOC) || !defined(RE_FREE)
#error In order to use RE_USE_CUSTOM_MALLOC you must provide defnitions for RE_MALLOC, RE_REALLOC and RE_FREE.
#endif
#endif /* #if !RE_USE_CUSTOM_MALLOC */

#define RE_NULL 0

#if !RE_USE_CUSTOM_SIZE_TYPE
#include <stddef.h>
#define RE_SIZE_TYPE size_t
#else
#if !defined(RE_SIZE_TYPE)
#error In order to use RE_USE_CUSTOM_SIZE_TYPE you must provide a definition for RE_SIZE_TYPE.
#endif
#endif /* #if !RE_USE_CUSTOM_SIZE_TYPE */
typedef RE_SIZE_TYPE re_size;

#if !RE_USE_CUSTOM_ASSERT
#include <assert.h>
#define RE_ASSERT assert
#else
#if !defined(RE_ASSERT)
#error In order to use RE_USE_CUSTOM_ASSERT you must provide a definition for RE_ASSERT.
#endif
#endif /* #if !RE_USE_CUSTOM_ASSERT */

#define RE__UNUSED(i) (void)(i)

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
#endif /* #if RE__CSTD >= 1999 */

#ifndef RE_API_H
#define RE_API_H


typedef int re_error;
typedef re_int32 re_rune;

#define RE_RUNE_MAX 0x10FFFF

#define RE_ERROR_NONE 0
#define RE_ERROR_NOMEM 1
#define RE_ERROR_INVALID 2
#define RE_ERROR_PARSE 3
#define RE_ERROR_COMPILE 4

typedef struct re_data re_data;

typedef struct re {
    re_data* data;
} re;

re_error re_init(re* re, const char* regex);
re_error re_destroy(re* re);
const char* re_get_error(re* reg, re_size* error_len);

#endif /* #ifndef RE_API_H */

#ifdef RE_IMPLEMENTATION


#define RE_INTERNAL static

#define RE___STRINGIFY_0(s) #s
#define RE__STRINGIFY(s) RE___STRINGIFY_0(s)

#if RE_DEBUG
#include <stdio.h>
#define RE__ASSERT_UNREACHED() RE_ASSERT(0)
#else
#define RE__ASSERT_UNREACHED() (void)(0)
#endif

#define RE_VEC_TYPE(T) \
    T ## _vec

#define RE_VEC_IDENT(T, name) \
    T ## _vec_ ## name

#define RE_VEC_IDENT_INTERNAL(T, name) \
    T ## _vec__ ## name

#if RE_DEBUG

#define RE_VEC_CHECK(vec) \
    do { \
        /* ensure size is not greater than allocation size */ \
        RE_ASSERT(vec->_size <= vec->_alloc); \
        /* ensure that data is not null if size is greater than 0 */ \
        RE_ASSERT(vec->_size ? vec->_data != NULL : 1); \
    } while (0)

#else

#define RE_VEC_CHECK(vec) RE_UNUSED(vec)

#endif /* #if RE_DEBUG */

#define RE_VEC_DECL(T) \
    typedef struct RE_VEC_TYPE(T) { \
        re_size _size; \
        re_size _alloc; \
        T* _data; \
    } RE_VEC_TYPE(T)

#define RE__VEC_DECL_init(T) \
    void RE_VEC_IDENT(T, init)(RE_VEC_TYPE(T)* vec)

#define RE__VEC_IMPL_init(T) \
    void RE_VEC_IDENT(T, init)(RE_VEC_TYPE(T)* vec) { \
        vec->_size = 0; \
        vec->_alloc = 0; \
        vec->_data = RE_NULL; \
    } 

#define RE__VEC_DECL_destroy(T) \
    void RE_VEC_IDENT(T, destroy)(RE_VEC_TYPE(T)* vec)

#define RE__VEC_IMPL_destroy(T) \
    void RE_VEC_IDENT(T, destroy)(RE_VEC_TYPE(T)* vec) { \
        RE_VEC_CHECK(vec); \
        if (vec->_data != RE_NULL) { \
            RE_FREE(vec->_data); \
        } \
    }

#define RE__VEC_GROW_ONE(T, vec) \
    do { \
        vec->_size += 1; \
        if (vec->_size > vec->_alloc) { \
            if (vec->_data == RE_NULL) { \
                vec->_alloc = 1; \
                vec->_data = (T*)RE_MALLOC(sizeof(T) * vec->_alloc); \
                if (vec->_data == RE_NULL) { \
                    return 1; \
                } \
            } else { \
                vec->_alloc *= 2; \
                vec->_data = (T*)RE_REALLOC(vec->_data, sizeof(T) * vec->_alloc); \
                if (vec->_data == RE_NULL) { \
                    return 1; \
                } \
            } \
        } \
    } while (0)

#define RE__VEC_GROW(T, vec, n) \
    do { \
        vec->_size += n; \
        if (vec->_size > vec->_alloc) { \
            vec->_alloc = vec->_size + (vec->_size >> 1); \
            if (vec->_data == RE_NULL) { \
                vec->_data = (T*)RE_MALLOC(sizeof(T) * vec->_alloc); \
            } else { \
                vec->_data = (T*)RE_REALLOC(vec->_data, sizeof(T) * vec->_alloc); \
            } \
            if (vec->_data == RE_NULL) { \
                return 1; \
            } \
        } \
    } while (0)

#define RE__VEC_SETSIZE(T, vec, n) \
    do { \
        if (vec->_alloc < n) { \
            vec->_alloc = n; \
            if (vec->_data == RE_NULL) { \
                vec->_data = (T*)RE_MALLOC(sizeof(T) * vec->_alloc); \
            } else { \
                vec->_data = (T*)RE_REALLOC(vec->_data, sizeof(T) * vec->_alloc); \
            } \
            if (vec->_data == RE_NULL) { \
                return 1; \
            } \
        } \
    } while (0)

#define RE__VEC_DECL_push(T) \
    int RE_VEC_IDENT(T, push)(RE_VEC_TYPE(T)* vec, T elem)

#define RE__VEC_IMPL_push(T) \
    int RE_VEC_IDENT(T, push)(RE_VEC_TYPE(T)* vec, T elem) { \
        RE_VEC_CHECK(vec); \
        RE__VEC_GROW_ONE(T, vec); \
        vec->_data[vec->_size - 1] = elem; \
        RE_VEC_CHECK(vec); \
        return 0; \
    }

#if RE_DEBUG

#define RE_VEC_CHECK_POP(vec) \
    do { \
        /* ensure that there is an element to pop */ \
        RE_ASSERT(vec->_size > 0); \
    } while (0)

#else

#define RE_VEC_CHECK_POP(vec) RE_UNUSED(vec)

#endif /* #if RE_DEBUG */

#define RE__VEC_DECL_pop(T) \
    T RE_VEC_IDENT(T, pop)(RE_VEC_TYPE(T)* vec)

#define RE__VEC_IMPL_pop(T) \
    T RE_VEC_IDENT(T, pop)(RE_VEC_TYPE(T)* vec) { \
        RE_VEC_CHECK(vec); \
        RE_VEC_CHECK_POP(vec); \
        return vec->_data[--vec->_size]; \
    }

#define RE__VEC_DECL_cat(T) \
    T RE_VEC_IDENT(T, cat)(RE_VEC_TYPE(T)* vec, RE_VEC_TYPE(T)* other)

#define RE__VEC_IMPL_cat(T) \
    int RE_VEC_IDENT(T, cat)(RE_VEC_TYPE(T)* vec, RE_VEC_TYPE(T)* other) { \
        re_size i; \
        re_size old_size = vec->_size; \
        RE_VEC_CHECK(vec); \
        RE_VEC_CHECK(other); \
        RE__VEC_GROW(T, vec, other->_size); \
        for (i = 0; i < other->_size; i++) { \
            vec->_data[old_size + i] = other->_data[i]; \
        } \
        RE_VEC_CHECK(vec); \
        return 0; \
    }

#define RE__VEC_DECL_insert(T) \
    int RE_VEC_IDENT(T, insert)(RE_VEC_TYPE(T)* vec, re_size index, T elem)

#define RE__VEC_IMPL_insert(T) \
    int RE_VEC_IDENT(T, insert)(RE_VEC_TYPE(T)* vec, re_size index, T elem) { \
        re_size i; \
        re_size old_size = vec->_size; \
        RE_VEC_CHECK(vec); \
        RE__VEC_GROW_ONE(T, vec); \
        if (old_size != 0) { \
            for (i = old_size; i >= index + 1; i--) { \
                vec->_data[i] = vec->_data[i - 1]; \
            } \
        } \
        vec->_data[index] = elem; \
        return 0; \
    }

#define RE__VEC_DECL_peek(T) \
    T RE_VEC_IDENT(T, peek)(const RE_VEC_TYPE(T)* vec)

#define RE__VEC_IMPL_peek(T) \
    T RE_VEC_IDENT(T, peek)(const RE_VEC_TYPE(T)* vec) { \
        RE_VEC_CHECK(vec); \
        RE_VEC_CHECK_POP(vec); \
        return vec->_data[vec->_size - 1]; \
    }

#define RE__VEC_DECL_clear(T) \
    void RE_VEC_IDENT(T, clear)(RE_VEC_TYPE(T)* vec)

#define RE__VEC_IMPL_clear(T) \
    void RE_VEC_IDENT(T, clear)(RE_VEC_TYPE(T)* vec) { \
        RE_VEC_CHECK(vec); \
        vec->_size = 0; \
    }

#define RE__VEC_DECL_size(T) \
    re_size RE_VEC_IDENT(T, size)(const RE_VEC_TYPE(T)* vec)

#define RE__VEC_IMPL_size(T) \
    re_size RE_VEC_IDENT(T, size)(const RE_VEC_TYPE(T)* vec) { \
        return vec->_size; \
    }

#if RE_DEBUG

#define RE_VEC_CHECK_BOUNDS(vec, idx) \
    do { \
        /* ensure that idx is within bounds */ \
        RE_ASSERT(idx < vec->_size); \
    } while (0)

#else

#define RE_VEC_CHECK_BOUNDS(vec, idx) \
    do { \
        RE_UNUSED(vec); \
        RE_UNUSED(idx); \
    } while (0) 

#endif /* #if RE_DEBUG */

#define RE__VEC_DECL_get(T) \
    T RE_VEC_IDENT(T, get)(const RE_VEC_TYPE(T)* vec, re_size idx)

#define RE__VEC_IMPL_get(T) \
    T RE_VEC_IDENT(T, get)(const RE_VEC_TYPE(T)* vec, re_size idx) { \
        RE_VEC_CHECK(vec); \
        RE_VEC_CHECK_BOUNDS(vec, idx); \
        return vec->_data[idx]; \
    }

#define RE__VEC_DECL_getref(T) \
    T* RE_VEC_IDENT(T, getref)(RE_VEC_TYPE(T)* vec, re_size idx)

#define RE__VEC_IMPL_getref(T) \
    T* RE_VEC_IDENT(T, getref)(RE_VEC_TYPE(T)* vec, re_size idx) { \
        RE_VEC_CHECK(vec); \
        RE_VEC_CHECK_BOUNDS(vec, idx); \
        return &vec->_data[idx]; \
    }

#define RE__VEC_DECL_getcref(T) \
    const T* RE_VEC_IDENT(T, getcref)(const RE_VEC_TYPE(T)* vec, re_size idx)

#define RE__VEC_IMPL_getcref(T) \
    const T* RE_VEC_IDENT(T, getcref)(const RE_VEC_TYPE(T)* vec, re_size idx) { \
        RE_VEC_CHECK(vec); \
        RE_VEC_CHECK_BOUNDS(vec, idx); \
        return &vec->_data[idx]; \
    }

#define RE__VEC_DECL_set(T) \
    void RE_VEC_IDENT(T, set)(RE_VEC_TYPE(T)* vec, re_size idx, T elem)

#define RE__VEC_IMPL_set(T) \
    void RE_VEC_IDENT(T, set)(RE_VEC_TYPE(T)* vec, re_size idx, T elem) { \
        RE_VEC_CHECK(vec); \
        RE_VEC_CHECK_BOUNDS(vec, idx); \
        vec->_data[idx] = elem; \
    }

#define RE__VEC_DECL_capacity(T) \
    re_size RE_VEC_IDENT(T, capacity)(RE_VEC_TYPE(T)* vec)

#define RE__VEC_IMPL_capacity(T) \
    re_size RE_VEC_IDENT(T, capacity)(RE_VEC_TYPE(T)* vec) { \
        return vec->_alloc; \
    }

#define RE__VEC_DECL_get_data(T) \
    const T* RE_VEC_IDENT(T, get_data)(const RE_VEC_TYPE(T)* vec)

#define RE__VEC_IMPL_get_data(T) \
    const T* RE_VEC_IDENT(T, get_data)(const RE_VEC_TYPE(T)* vec) { \
        return vec->_data; \
    }

#define RE__VEC_DECL_move(T) \
    void RE_VEC_IDENT(T, move)(RE_VEC_TYPE(T)* vec, RE_VEC_TYPE(T)* old);

#define RE__VEC_IMPL_move(T) \
    void RE_VEC_IDENT(T, move)(RE_VEC_TYPE(T)* vec, RE_VEC_TYPE(T)* old) { \
        RE_VEC_CHECK(old); \
        *vec = *old; \
        RE_VEC_IDENT(T, init)(old); \
    }

#define RE__VEC_DECL_reserve(T) \
    int RE_VEC_IDENT(T, reserve)(RE_VEC_TYPE(T)* vec, re_size cap);

#define RE__VEC_IMPL_reserve(T) \
    int RE_VEC_IDENT(T, reserve)(RE_VEC_TYPE(T)* vec, re_size cap) { \
        RE_VEC_CHECK(vec); \
        RE__VEC_SETSIZE(T, vec, cap); \
    }

#define RE_VEC_DECL_FUNC(T, func) \
    RE__VEC_DECL_ ## func (T)

#define RE_VEC_IMPL_FUNC(T, func) \
    RE__VEC_IMPL_ ## func (T)

typedef unsigned char re_char;

typedef struct re__str {
    re_size _size_short; /* does not include \0 */
    re_size _alloc; /* does not include \0 */
    re_char* _data;
} re__str;

void re__str_init(re__str* str);
int re__str_init_s(re__str* str, const re_char* s);
int re__str_init_n(re__str* str, re_size n, const re_char* chrs);
int re__str_init_copy(re__str* str, const re__str* in);
void re__str_init_move(re__str* str, re__str* old);
void re__str_destroy(re__str* str);
re_size re__str_size(const re__str* str);
int re__str_cat(re__str* str, const re__str* other);
int re__str_cat_s(re__str* str, const re_char* s);
int re__str_cat_n(re__str* str, re_size n, const re_char* chrs);
int re__str_push(re__str* str, re_char chr);
int re__str_insert(re__str* str, re_size index, re_char chr);
const re_char* re__str_get_data(const re__str* str);
int re__str_cmp(const re__str* str_a, const re__str* str_b);
re_size re__str_slen(const re_char* chars);


typedef struct re__str_view {
    const re_char* _data;
    re_size _size;
} re__str_view;

void re__str_view_init(re__str_view* view, const re__str* other);
void re__str_view_init_s(re__str_view* view, const re_char* chars);
void re__str_view_init_n(re__str_view* view, const re_char* chars, re_size n);
void re__str_view_init_null(re__str_view* view);
re_size re__str_view_size(const re__str_view* view);
const re_char* re__str_view_get_data(const re__str_view* view);
int re__str_view_cmp(const re__str_view* a, const re__str_view* b);

#define RE__MAX(a, b) (((a) < (b)) ? (b) : (a))

RE_INTERNAL re_uint32 re__murmurhash3_32(const re_uint8* data, re_size data_len);

RE_INTERNAL void re__zero_mem(re_size size, void* mem);

#ifndef RE_INTERNAL_H
#define RE_INTERNAL_H


/* POD type */
/* Holds a byte range [min, max] */
typedef struct re__byte_range {
    re_uint8 min;
    re_uint8 max;
} re__byte_range;

int re__byte_range_equals(re__byte_range range, re__byte_range other);
int re__byte_range_adjacent(re__byte_range range, re__byte_range other);
re__byte_range re__byte_range_merge(re__byte_range range, re__byte_range other);

/* POD type */
/* Stores characters in the range [min, max] === [min, max+1) */
typedef struct re__rune_range {
    re_rune min;
    re_rune max;
} re__rune_range;

RE_VEC_DECL(re__rune_range);

int re__rune_range_equals(re__rune_range range, re__rune_range other);
int re__rune_range_intersects(re__rune_range range, re__rune_range clip);
re__rune_range re__rune_range_clamp(re__rune_range range, re__rune_range bounds);

/* Character class. */
typedef struct re__charclass {
    /* Non-overlapping set of ranges. */
    re__rune_range_vec ranges;
} re__charclass;

/* Index of ASCII char class types. */
typedef enum re__charclass_ascii_type {
    RE__CHARCLASS_ASCII_TYPE_ALNUM,
    RE__CHARCLASS_ASCII_TYPE_ALPHA,
    RE__CHARCLASS_ASCII_TYPE_ASCII,
    RE__CHARCLASS_ASCII_TYPE_BLANK,
    RE__CHARCLASS_ASCII_TYPE_CNTRL,
    RE__CHARCLASS_ASCII_TYPE_DIGIT,
    RE__CHARCLASS_ASCII_TYPE_GRAPH,
    RE__CHARCLASS_ASCII_TYPE_LOWER,
    RE__CHARCLASS_ASCII_TYPE_PERL_SPACE,
    RE__CHARCLASS_ASCII_TYPE_PRINT,
    RE__CHARCLASS_ASCII_TYPE_PUNCT,
    RE__CHARCLASS_ASCII_TYPE_SPACE,
    RE__CHARCLASS_ASCII_TYPE_UPPER,
    RE__CHARCLASS_ASCII_TYPE_WORD,
    RE__CHARCLASS_ASCII_TYPE_XDIGIT,
    RE__CHARCLASS_ASCII_TYPE_MAX
} re__charclass_ascii_type;

RE_INTERNAL void re__charclass_init(re__charclass* charclass);
RE_INTERNAL re_error re__charclass_init_from_class(re__charclass* charclass, re__charclass_ascii_type type, int inverted);
RE_INTERNAL re_error re__charclass_init_from_string(re__charclass* charclass, re__str* name, int inverted);
RE_INTERNAL void re__charclass_destroy(re__charclass* charclass);
RE_INTERNAL re_error re__charclass_push(re__charclass* charclass, re__rune_range range);
RE_INTERNAL const re__rune_range* re__charclass_get_ranges(const re__charclass* charclass);
RE_INTERNAL re_size re__charclass_get_num_ranges(const re__charclass* charclass);
RE_INTERNAL int re__charclass_equals(const re__charclass* charclass, const re__charclass* other);

#if RE_DEBUG

RE_INTERNAL void re__charclass_dump(const re__charclass* charclass, re_size lvl);
RE_INTERNAL int re__charclass_verify(const re__charclass* charclass);

#endif

/* Immediate-mode charclass builder. */
typedef struct re__charclass_builder {
    re__rune_range_vec ranges;
    int should_invert;
    re_rune highest;
} re__charclass_builder;

RE_INTERNAL void re__charclass_builder_init(re__charclass_builder* builder);
RE_INTERNAL void re__charclass_builder_destroy(re__charclass_builder* builder);
RE_INTERNAL void re__charclass_builder_begin(re__charclass_builder* builder);
RE_INTERNAL void re__charclass_builder_invert(re__charclass_builder* builder);
RE_INTERNAL re_error re__charclass_builder_insert_range(re__charclass_builder* builder, re__rune_range range);
RE_INTERNAL re_error re__charclass_builder_insert_class(re__charclass_builder* builder, re__charclass* charclass);
RE_INTERNAL re_error re__charclass_builder_finish(re__charclass_builder* builder, re__charclass* charclass);

#if RE_DEBUG

RE_INTERNAL int re__charclass_builder_verify(const re__charclass_builder* builder);

#endif

typedef struct re__ast re__ast; 

/* Enumeration of AST node types. */
typedef enum re__ast_type {
    /* No type. Should never occur. */
    RE__AST_TYPE_NONE,
    /* A single character. */
    RE__AST_TYPE_RUNE,
    /* A character class. */
    RE__AST_TYPE_CLASS,
    /* A concatenation of multiple nodes. */
    RE__AST_TYPE_CONCAT,
    /* An alteration of multiple nodes. */
    RE__AST_TYPE_ALT,
    /* A quantifier (*,+,?) or a range {m,n} */
    RE__AST_TYPE_QUANTIFIER,
    /* Capturing/non-capturing group. */
    RE__AST_TYPE_GROUP,
    /* Text/word asserts */
    RE__AST_TYPE_ASSERT,
    /* Any character (.) */
    RE__AST_TYPE_ANY_CHAR,
    /* Any byte (\C) */
    RE__AST_TYPE_ANY_BYTE,
    RE__AST_TYPE_MAX
} re__ast_type;

RE_VEC_DECL(re__ast);
RE_VEC_DECL_FUNC(re__ast, init);
RE_VEC_DECL_FUNC(re__ast, destroy);
RE_VEC_DECL_FUNC(re__ast, push);
RE_VEC_DECL_FUNC(re__ast, getref);
RE_VEC_DECL_FUNC(re__ast, size);

#define RE__AST_QUANTIFIER_MAX 2000
#define RE__AST_QUANTIFIER_INFINITY RE__AST_QUANTIFIER_MAX+2

/* Quantifier info. */
/* Range: [min, max) */
typedef struct re__ast_quantifier_info {
    /* Minimum amount. */
    re_int32 min;
    /* Maximum amount. -1 for infinity. */
    re_int32 max;
    /* Whether or not to prefer fewer matches. */
    int greediness;
} re__ast_quantifier_info;

/* Assert types, as they are represented in the AST. */
typedef enum re__ast_assert_type {
    RE__AST_ASSERT_TYPE_TEXT_START = 1,
    RE__AST_ASSERT_TYPE_TEXT_END = 2,
    RE__AST_ASSERT_TYPE_TEXT_START_ABSOLUTE = 4,
    RE__AST_ASSERT_TYPE_TEXT_END_ABSOLUTE = 8,
    RE__AST_ASSERT_TYPE_WORD = 16,
    RE__AST_ASSERT_TYPE_WORD_NOT = 32
} re__ast_assert_type;

/* Group flags */
typedef enum re__ast_group_flags {
    RE__AST_GROUP_FLAG_CASE_INSENSITIVE = 1,
    RE__AST_GROUP_FLAG_MULTILINE = 2,
    RE__AST_GROUP_FLAG_DOT_NEWLINE = 4,
    RE__AST_GROUP_FLAG_UNGREEDY = 8,
    RE__AST_GROUP_FLAG_NONMATCHING = 16
} re__ast_group_flags;

/* Group info */
typedef struct re__ast_group_info {
    re__ast_group_flags flags;
    re_uint32 match_number;
} re__ast_group_info;

/* Holds AST node data depending on the node type. */
typedef union re__ast_data {
    /* RE__AST_TYPE_RUNE: holds a single character */
    re_rune rune;
    /* RE__AST_TYPE_CLASS: holds a character class. */
    re__charclass charclass;
    /* RE__AST_TYPE_GROUP: holds the group's index and flags */
    re__ast_group_info group_info;
    /* RE__AST_TYPE_QUANTIFIER: minimum/maximum/greediness */
    re__ast_quantifier_info quantifier_info;
    /* RE__AST_TYPE_ASSERT: type(s) of assert present */
    re__ast_assert_type assert_type;
} re__ast_data;

#define RE__AST_NONE -1

struct re__ast {
    re__ast_type type;
    re_int32 first_child_ref;
    re_int32 prev_sibling_ref;
    re_int32 next_sibling_ref;
    re__ast_data _data;
};

RE_INTERNAL void re__ast_init_rune(re__ast* ast, re_rune rune);
RE_INTERNAL void re__ast_init_class(re__ast* ast, re__charclass charclass);
RE_INTERNAL void re__ast_init_concat(re__ast* ast);
RE_INTERNAL void re__ast_init_alt(re__ast* ast);
RE_INTERNAL void re__ast_init_quantifier(re__ast* ast, re_int32 min, re_int32 max);
RE_INTERNAL void re__ast_init_group(re__ast* ast);
RE_INTERNAL void re__ast_init_assert(re__ast* ast, re__ast_assert_type assert_type);
RE_INTERNAL void re__ast_init_any_char(re__ast* ast);
RE_INTERNAL void re__ast_init_any_byte(re__ast* ast);
RE_INTERNAL void re__ast_destroy(re__ast* ast);
RE_INTERNAL int re__ast_get_quantifier_greediness(re__ast* ast);
RE_INTERNAL void re__ast_set_quantifier_greediness(re__ast* ast, int is_greedy);
RE_INTERNAL re_int32 re__ast_get_quantifier_min(re__ast* ast);
RE_INTERNAL re_int32 re__ast_get_quantifier_max(re__ast* ast);
RE_INTERNAL re_rune re__ast_get_rune(re__ast* ast);
RE_INTERNAL re__ast_group_flags re__ast_get_group_flags(re__ast* ast);
RE_INTERNAL void re__ast_set_group_flags(re__ast* ast, re__ast_group_flags flags);
RE_INTERNAL re__ast_assert_type re__ast_get_assert_type(re__ast* ast);

typedef struct re__ast_root {
    re__ast_vec ast_vec;
    re_int32 last_empty_ref;
} re__ast_root;

RE_INTERNAL void re__ast_root_init(re__ast_root* ast_root);
RE_INTERNAL void re__ast_root_destroy(re__ast_root* ast_root);
RE_INTERNAL re__ast* re__ast_root_get(re__ast_root* ast_root, re_int32 ast_ref);
RE_INTERNAL re_error re__ast_root_add(re__ast_root* ast_root, re__ast ast, re_int32* out_ref);
RE_INTERNAL void re__ast_root_remove(re__ast_root* ast_root, re_int32 ast_ref);
RE_INTERNAL void re__ast_root_link_siblings(re__ast_root* ast_root, re_int32 first_sibling_ref, re_int32 next_sibling_ref);
RE_INTERNAL void re__ast_root_set_child(re__ast_root* ast_root, re_int32 root_ref, re_int32 child_ref);
RE_INTERNAL void re__ast_root_wrap(re__ast_root* ast_root, re_int32 parent_ref, re_int32 inner_ref, re_int32 outer_ref);
RE_INTERNAL re_int32 re__ast_root_size(re__ast_root* ast_root);

#if RE_DEBUG

RE_INTERNAL void re__ast_root_debug_dump(re__ast_root* ast_root, re_int32 root_ref, re_int32 lvl);

#endif

typedef enum re__parse_state {
    RE__PARSE_STATE_GND,
    RE__PARSE_STATE_MAYBE_QUESTION,
    RE__PARSE_STATE_ESCAPE,
    RE__PARSE_STATE_PARENS_INITIAL,
    RE__PARSE_STATE_PARENS_FLAG_INITIAL,
    RE__PARSE_STATE_PARENS_FLAG_NEGATE,
    RE__PARSE_STATE_PARENS_AFTER_COLON,
    RE__PARSE_STATE_PARENS_AFTER_P,
    RE__PARSE_STATE_PARENS_NAME_INITIAL,
    RE__PARSE_STATE_PARENS_NAME,
    RE__PARSE_STATE_OCTAL_SECOND_DIGIT,
    RE__PARSE_STATE_OCTAL_THIRD_DIGIT,
    RE__PARSE_STATE_HEX_INITIAL,
    RE__PARSE_STATE_HEX_SECOND_DIGIT,
    RE__PARSE_STATE_HEX_BRACKETED,
    RE__PARSE_STATE_QUOTE,
    RE__PARSE_STATE_QUOTE_ESCAPE,
    RE__PARSE_STATE_COUNTING_FIRST_NUM_INITIAL,
    RE__PARSE_STATE_COUNTING_FIRST_NUM,
    RE__PARSE_STATE_COUNTING_SECOND_NUM_INITIAL,
    RE__PARSE_STATE_COUNTING_SECOND_NUM,
    RE__PARSE_STATE_CHARCLASS_INITIAL,
    RE__PARSE_STATE_CHARCLASS_AFTER_CARET,
    RE__PARSE_STATE_CHARCLASS_AFTER_BRACKET,
    RE__PARSE_STATE_CHARCLASS_LO,
    RE__PARSE_STATE_CHARCLASS_AFTER_LO,
    RE__PARSE_STATE_CHARCLASS_HI,
    RE__PARSE_STATE_CHARCLASS_NAMED
} re__parse_state;

typedef struct re__parse_frame {
    re_int32 ast_frame_root_ref;
    re_int32 ast_prev_child_ref;
    re__parse_state ret_state;
    re__ast_group_flags group_flags;
    re_int32 depth;
    re_int32 depth_max;
} re__parse_frame;

RE_VEC_DECL(re__parse_frame);

typedef struct re__parse {
    re* re;
    re__parse_frame_vec frames;
    re__ast_root ast_root;
    re_int32 ast_frame_root_ref;
    re_int32 ast_prev_child_ref;
    re__parse_state state;
    re_int32 radix_num;
    int radix_digits;
    re__charclass_builder charclass_builder;
    int defer;
    re__ast_group_flags group_flags_new;
    re__ast_group_flags group_flags;
    const re_char* str_begin;
    const re_char* str_end;
    re_int32 counting_first_num;
    re_rune charclass_lo_rune;
    re_int32 depth;
    re_int32 depth_max;
    re_int32 depth_max_prev;
} re__parse;

RE_INTERNAL void re__parse_init(re__parse* parse, re* re);
RE_INTERNAL void re__parse_destroy(re__parse* parse);
RE_INTERNAL re_error re__parse_regex(re__parse* parse, re_size regex_size, const re_char* regex);

typedef re_uint32 re__prog_loc;

/* Invalid program location (used for debugging) */
#define RE__PROG_LOC_INVALID 0

/* Each instruction has a type, data associated with the specific type, and
 * either one or two pointers to the next instruction(s) to execute. */

/* An instruction is shown like this: */
/* +-------+
 * |  Op   |
 * | Instr |
 * +-------+ */
/* Instructions are shown as assembled into the program like this: */
/* 0       1       2       3
 * +-------+-------+-------+~~~
 * |  Op   |  Op   |  Op   | ..
 * | Instr | Instr | Instr | ..
 * +-------+-------+-------+~~~ */
/* Instructions all have a "primary branch", that is, a pointer to the next
 * instruction that will execute. */
/* +0      +1      +2
 * +-------+-------+-------+~~~
 * |  Op   |  Op   |  Op   | ..
 * | Instr | Instr | Instr | ..
 * +---+---+---+---+---+---+~~~
 *     |     ^ |     ^ |
 *     |     | |     | |
 *     +-----+ +-----+ +--------> ...
 *        ^       ^       ^
 *        |       |       |
 *        +-------+-------+
 *                |
 *             Primary
 *             Branches             */
/* Some instructions have no branches, and some have two. */
/* +0      +1      +2      +3      +4
 * +-------+-------+-------+-------+-------+~~~
 * | Byte  | Byte  | Split | Match | Byte  | ..
 * | Instr | Instr | Instr | Instr | Instr | ..
 * +---+---+---+---+---+-+-+-------+-------+~~~
 *     |     ^ |     ^ | |   ^       ^ |
 *     |     | |     | | |   |       | |
 *     |     | |     | | +-----------+ +--------> ...
 *     |     | |     | |     |   ^
 *     +-----+ +-----+ +-----+   |
 *        ^       ^       ^      |
 *        |       |       |      |
 *        +-------+-------+      |
 *                |              |
 *             Primary       Secondary
 *             Branches       Branch              */

/* Enumeration of instruction types (opcodes) */
typedef enum re__prog_inst_type {
    /* Match a single byte */
    /* +-------+
     * | Byte  |
     * | Instr |
     * +---+---+
     *     |
     *     +-----> */
    RE__PROG_INST_TYPE_BYTE,
    /* Match a range of bytes */
    /* +-------+
     * | ByteR |
     * | Instr |
     * +---+---+
     *     |
     *     +-----> */
    RE__PROG_INST_TYPE_BYTE_RANGE,
    /* Fork execution to two different locations */
    /* +-------+
     * | Split |
     * | Instr |
     * +---+-+-+
     *     | |
     *     +----->  <-- This branch is taken first (primary)
     *       |
     *       +--->  <-- This branch is taken second (secondary) */
    RE__PROG_INST_TYPE_SPLIT,
    /* Finish execution in a match state */
    /* +-------+
     * | Match |
     * | Instr |
     * +-------+ */
    RE__PROG_INST_TYPE_MATCH,
    /* Fail execution */
    /* +-------+
     * | Fail  |
     * | Instr |
     * +-------+ */
    RE__PROG_INST_TYPE_FAIL,
    /* Save current program counter (found a match group) */
    /* +-------+
     * | Save  |
     * | Instr |
     * +---+---+ 
     *     |
     *     +-----> */
    RE__PROG_INST_TYPE_SAVE,
    /* Assert that some context is occurring (begin/end of text/word, etc.) */
    /* +-------+
     * |  Ass  | <- funny
     * | Instr |
     * +---+---+
     *     |
     *     +-----> */
    RE__PROG_INST_TYPE_ASSERT
} re__prog_inst_type;

/* Opcode-specific data */
typedef union re__prog_inst_data {
    /* RE__PROG_INST_TYPE_BYTE: a single byte */
    re_uint8 _byte;
    /* RE__PROG_INST_TYPE_BYTE_RANGE: a range of bytes */
    re__byte_range _range;
    /* RE__PROG_INST_TYPE_SPLIT: secondary branch target */
    re__prog_loc _secondary;
    /* RE__PROG_INST_TYPE_MATCH: match index */
    re_uint32 _match_idx;
    /* RE__PROG_INST_TYPE_ASSERT: assert context set */
    re_uint32 _assert_context;
    /* RE__PROG_INST_TYPE_SAVE: save index */
    re_uint32 _save_idx;
} re__prog_inst_data;

/* Program instruction structure */
typedef struct re__prog_inst {
    /* Opcode */
    re__prog_inst_type _inst_type;
    /* Primary branch target */
    re__prog_loc _primary;
    /* Data about instruction */
    re__prog_inst_data _inst_data;
} re__prog_inst;


RE_VEC_DECL(re__prog_inst);

RE_INTERNAL void re__prog_inst_init_byte(re__prog_inst* inst, re_uint8 byte);
RE_INTERNAL void re__prog_inst_init_byte_range(re__prog_inst* inst, re_uint8 min, re_uint8 max);
RE_INTERNAL void re__prog_inst_init_split(re__prog_inst* inst, re__prog_loc primary, re__prog_loc secondary);
RE_INTERNAL void re__prog_inst_init_match(re__prog_inst* inst, re_uint32 match_idx);
RE_INTERNAL void re__prog_inst_init_fail(re__prog_inst* inst);
RE_INTERNAL void re__prog_inst_init_assert(re__prog_inst* inst, re_uint32 assert_context);
RE_INTERNAL void re__prog_inst_init_save(re__prog_inst* inst, re_uint32 save_idx);
RE_INTERNAL re__prog_loc re__prog_inst_get_primary(re__prog_inst* inst);
RE_INTERNAL void re__prog_inst_set_primary(re__prog_inst* inst, re__prog_loc loc);
RE_INTERNAL re_uint8 re__prog_inst_get_byte(re__prog_inst* inst);
RE_INTERNAL re_uint8 re__prog_inst_get_byte_min(re__prog_inst* inst);
RE_INTERNAL re_uint8 re__prog_inst_get_byte_max(re__prog_inst* inst);
RE_INTERNAL re__prog_loc re__prog_inst_get_split_secondary(re__prog_inst* inst);
RE_INTERNAL void re__prog_inst_set_split_secondary(re__prog_inst* inst, re__prog_loc loc);
RE_INTERNAL re__prog_inst_type re__prog_inst_get_type(re__prog_inst* inst);
RE_INTERNAL re__ast_assert_type re__prog_inst_get_assert_ctx(re__prog_inst* inst);

#define RE__ERROR_PROGMAX (RE_ERROR_COMPILE | (1 << 8))
#define RE__PROG_SIZE_MAX 100000

/* The program itself */
typedef struct re__prog {
    re__prog_inst_vec _instructions;
} re__prog;

RE_INTERNAL re_error re__prog_init(re__prog* prog);
RE_INTERNAL void re__prog_destroy(re__prog* prog);
RE_INTERNAL re__prog_loc re__prog_size(re__prog* prog);
RE_INTERNAL re__prog_inst* re__prog_get(re__prog* prog, re__prog_loc loc);
RE_INTERNAL re_error re__prog_add(re__prog* prog, re__prog_inst inst);

/* A list of program patches -- locations in the program that need to point to
 * later instructions */
/* Like RE2, we store the location of the next patch in each instruction, so it
 * turns out to be a linked-list of sorts. */
/* This representation is useful because we only really need prepend, append,
 * and iterate facilities. So in addition to not having to perform manual memory
 * allocation, we get quick operations "for free". */
typedef struct re__compile_patches {
    re__prog_loc first_inst;
    re__prog_loc last_inst;
} re__compile_patches;

RE_INTERNAL void re__compile_patches_init(re__compile_patches* patches);
RE_INTERNAL void re__compile_patches_append(re__compile_patches* patches, re__prog* prog, re__prog_loc to, int secondary);

#if RE_DEBUG

RE_INTERNAL void re__compile_patches_dump(re__compile_patches* patches, re__prog* prog);

#endif

/* Tree node, used for representing a character class. */
/* The tree representation is convenient because it allows a completely
 * optimized UTF-8 DFA with a somewhat simple construction. */
/* A tree node looks like this:
 *
 * +-------+
 * | AA-BB |----- children...
 * +-------+
 *     |
 *     |
 * siblings...
 *
 * where AA is the minimum byte for a range and BB is the maximum. */
/* Each node is a binary tree node, where the bottom link corresponds to the 
 * first sibling and the right link corresponds to the first child. */
/* Nodes reach each other via indexes in a contiguous chunk of memory. In the
 * code, these are suffixed with "_ref". So, a variable named "child_ref" is
 * just an int that corresponds to another tree at that position within the
 * character class compiler's tree vector. */
/* If a node isn't connected to anything, the reference is set to
 * RE__CHARCLASS_COMPILE_TREE_NONE (think of it as a NULL pointer), and we
 * consider it to be unconnected. */
/* Here is a diagram of some connected tree nodes:
 *
 *                           +-------+
 * root->prev_sibling_ref -> | 66-66 |
 *                           +-------+
 *                               |
 *                               |
 *                           +-------+     +-------+
 *                   root -> | 68-68 |-----| 35-3F | <- root->first_child_ref
 *                           +-------+     +-------+
 *                               |             |
 *                               |             |
 *                           +-------+     +-------+
 * root->next_sibling_ref -> | 70-70 |     | 66-67 |
 *                           +-------+     +-------+
 *                                             |
 *                                             |
 *                                         +-------+
 *                                         | 70-7F | <- root->last_child_ref
 *                                         +-------+ */
/* Remember, each line in the graph is actually an int32 describing the position
 * of the node inside the charclass compiler. */
/* Since it's hard to draw these graphs as they get more complex, from now I'll
 * just show them like this:
 *
 * ---[66-66]
 *       |
 *    [68-68]---[35-3F]
 *       |         |
 *       |      [66-67]
 *       |         |
 *       |      [70-7F]
 *       |
 *    [70-70]
 * 
 * This is equivalent to the above graph. */
#define RE__COMPILE_CHARCLASS_TREE_NONE -1
typedef struct re__compile_charclass_tree {
    /* Range of bytes to match */
    re__byte_range byte_range;
    /* Reference to next sibling */
    re_int32 next_sibling_ref;
    /* Reference to previous sibling (used for reverse-iterating) */
    re_int32 prev_sibling_ref;
    /* Reference to first child */
    re_int32 first_child_ref;
    /* Reference to last child (used for reverse-iterating) */
    re_int32 last_child_ref;
    /* Hash of this tree, used for caching */
    re_uint32 hash;
} re__compile_charclass_tree;

#define RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE -1

/* Entry used to index a tree in the char class compiler's hash table. */
/* The hash table uses a sparse/dense model, where there is a fixed-size sparse
 * array of integers that correspond to indices in the dense vector. */
/* We can quickly test if a tree is in the cache by moduloing its hash by the
 * size of the sparse array, checking if the corresponding index is less than
 * the size of the dense vector, and finally by checking if the corresponding
 * element in the dense vector points *back* to the sparse array. This is very
 * similar to a sparse set. */
/* If more than one element hashes to the same position in the dense vector, we
 * link them together using the 'next' member, forming a mini-linked list. */
typedef struct re__compile_charclass_hash_entry {
    /* Index in sparse array */
    re_int32 sparse_index;
    /* Reference to tree root */
    re_int32 root_ref;
    /* Compiled instruction location in the program */
    re__prog_loc prog_loc;
    /* Next hash_entry that hashes to the same value, 
     * RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE otherwise. */
    re_int32 next;
} re__compile_charclass_hash_entry;

RE_INTERNAL void re__compile_charclass_hash_entry_init(re__compile_charclass_hash_entry* hash_entry, re_int32 sparse_index, re_int32 tree_ref, re__prog_loc prog_loc);

RE_VEC_DECL(re__compile_charclass_hash_entry);
RE_VEC_DECL(re__compile_charclass_tree);

#define RE__COMPILE_CHARCLASS_CACHE_SPARSE_SIZE 1024

/* Character class compiler. */
typedef struct re__compile_charclass {
    /* Vector of tree nodes. Each '_ref' entry in re__compile_charclass_tree
     * represents an index in this vector. */
    re__compile_charclass_tree_vec tree;
    /* Sparse tree cache. Each element in this array points to a corresponding
     * position in 'cache_dense'. Lookup is performed by moduloing a tree's hash
     * with the sparse cache size. Since cache hits are relatively rare, this
     * allows pretty inexpensive lookup. */
    re_int32* cache_sparse;
    /* Dense tree cache. Entries with duplicate hashes are linked using their
     * 'next' member. */
    re__compile_charclass_hash_entry_vec cache_dense;
} re__compile_charclass;

void re__compile_charclass_init(re__compile_charclass* char_comp);
void re__compile_charclass_destroy(re__compile_charclass* char_comp);
re_error re__compile_charclass_gen(re__compile_charclass* char_comp, re__charclass* charclass, re__prog* prog, re__compile_patches* patches_out);

#if RE_DEBUG
void re__compile_charclass_dump(re__compile_charclass* char_comp, re_int32 tree_idx, re_int32 indent);
#endif

typedef struct re__compile_frame {
    re_int32 ast_root_ref;
    re_int32 ast_child_ref;
    re__compile_patches patches;
    re__prog_loc start;
    re__prog_loc end;
    re_int32 rep_idx;
} re__compile_frame;

typedef struct re__compile {
    re* re;
    re__compile_frame* frames;
    re_int32 frames_size;
    re_int32 frame_ptr;
    re_int32 ast_ref;
    re__compile_charclass char_comp;
} re__compile;

RE_INTERNAL void re__compile_init(re__compile* compile, re* re);
RE_INTERNAL void re__compile_destroy(re__compile* compile);
RE_INTERNAL re_error re__compile_regex(re__compile* compile);

typedef re__prog_loc re__exec_thrdmin;

typedef struct re__exec_thrdmin_set {
    re__exec_thrdmin* dense;
    re__exec_thrdmin* sparse;
    re__prog_loc n;
    re__prog_loc size;
} re__exec_thrdmin_set;

RE_VEC_DECL(re__exec_thrdmin);

/* Execution context. */
typedef struct re__exec {
    re* re;
    re__exec_thrdmin_set set_a;
    re__exec_thrdmin_set set_b;
    re__exec_thrdmin_set set_c;
    re__exec_thrdmin_vec thrd_stk;
} re__exec;

/* Internal data structure */
struct re_data {
    re__parse parse;
    re__prog program;
    re__compile compile;
    re__exec exec;
    /* Note: in the case of an OOM situation, we may not be able to allocate
     * memory for the error string. In this case, we set error_string_is_const
     * to 1 and treat error_string specially. See re__set_error. */
    int error_string_is_const;
    re__str error_string;
};

RE_INTERNAL void re__exec_init(re__exec* exec, re* re);
RE_INTERNAL void re__exec_destroy(re__exec* exec);

RE_INTERNAL re_error re__exec_nfa(re__exec* exec, const re_char* str, re_size n);

RE_INTERNAL void re__set_error_str(re* re, const re__str* error_str);
RE_INTERNAL void re__set_error_generic(re* re, re_error err);

/*RE_INTERNAL re_error re__compile(re* re);*/
RE_INTERNAL void re__prog_debug_dump(re__prog* prog);

#endif /* #ifndef RE_INTERNAL_H */


RE_INTERNAL void re__ast_init(re__ast* ast, re__ast_type type) {
    ast->type = type;
    ast->first_child_ref = -1;
    ast->prev_sibling_ref = -1;
    ast->next_sibling_ref = -1;
}

RE_INTERNAL void re__ast_init_rune(re__ast* ast, re_rune rune) {
    re__ast_init(ast, RE__AST_TYPE_RUNE);
    ast->_data.rune = rune;
}

/* Transfers ownership of charclass to the AST node. */
RE_INTERNAL void re__ast_init_class(re__ast* ast, re__charclass charclass) {
    re__ast_init(ast, RE__AST_TYPE_CLASS);
    ast->_data.charclass = charclass;
}

RE_INTERNAL void re__ast_init_concat(re__ast* ast) {
    re__ast_init(ast, RE__AST_TYPE_CONCAT);
}

RE_INTERNAL void re__ast_init_alt(re__ast* ast) {
    re__ast_init(ast, RE__AST_TYPE_ALT);
}

RE_INTERNAL void re__ast_init_quantifier(re__ast* ast, re_int32 min, re_int32 max) {
    re__ast_init(ast, RE__AST_TYPE_QUANTIFIER);
    RE_ASSERT(min != max);
    RE_ASSERT(min < max);
    ast->_data.quantifier_info.min = min;
    ast->_data.quantifier_info.max = max;
    ast->_data.quantifier_info.greediness = 1;
}

RE_INTERNAL void re__ast_init_group(re__ast* ast) {
    re__ast_init(ast, RE__AST_TYPE_GROUP);
    ast->_data.group_info.flags = 0;
    ast->_data.group_info.match_number = 0;
}

RE_INTERNAL void re__ast_init_assert(re__ast* ast, re__ast_assert_type assert_type) {
    re__ast_init(ast, RE__AST_TYPE_ASSERT);
    ast->_data.assert_type = assert_type;
}

RE_INTERNAL void re__ast_init_any_char(re__ast* ast) {
    re__ast_init(ast, RE__AST_TYPE_ANY_CHAR);
}

RE_INTERNAL void re__ast_init_any_byte(re__ast* ast) {
    re__ast_init(ast, RE__AST_TYPE_ANY_BYTE);
}

RE_INTERNAL void re__ast_destroy(re__ast* ast) {
    if (ast->type == RE__AST_TYPE_CLASS) {
        re__charclass_destroy(&ast->_data.charclass);
    }
}

RE_INTERNAL void re__ast_set_quantifier_greediness(re__ast* ast, int is_greedy) {
    RE_ASSERT(ast->type == RE__AST_TYPE_QUANTIFIER);
    ast->_data.quantifier_info.greediness = is_greedy;
}

RE_INTERNAL int re__ast_get_quantifier_greediness(re__ast* ast) {
    RE_ASSERT(ast->type == RE__AST_TYPE_QUANTIFIER);
    return ast->_data.quantifier_info.greediness;
}

RE_INTERNAL re_int32 re__ast_get_quantifier_min(re__ast* ast) {
    RE_ASSERT(ast->type == RE__AST_TYPE_QUANTIFIER);
    return ast->_data.quantifier_info.min;
}

RE_INTERNAL re_int32 re__ast_get_quantifier_max(re__ast* ast) {
    RE_ASSERT(ast->type == RE__AST_TYPE_QUANTIFIER);
    return ast->_data.quantifier_info.max;
}

RE_INTERNAL re_rune re__ast_get_rune(re__ast* ast) {
    RE_ASSERT(ast->type == RE__AST_TYPE_RUNE);
    return ast->_data.rune;
}

RE_INTERNAL void re__ast_set_group_flags(re__ast* ast, re__ast_group_flags flags) {
    RE_ASSERT(ast->type == RE__AST_TYPE_GROUP);
    ast->_data.group_info.flags = flags;
}

RE_INTERNAL re__ast_assert_type re__ast_get_assert_type(re__ast* ast) {
    RE_ASSERT(ast->type == RE__AST_TYPE_ASSERT);
    return ast->_data.assert_type;
}

RE_INTERNAL void re__ast_root_init(re__ast_root* ast_root) {
    re__ast_vec_init(&ast_root->ast_vec);
    ast_root->last_empty_ref = RE__AST_NONE;
}

RE_INTERNAL void re__ast_root_destroy(re__ast_root* ast_root) {
    re__ast_vec_destroy(&ast_root->ast_vec);
}

RE_INTERNAL re__ast* re__ast_root_get(re__ast_root* ast_root, re_int32 ast_ref) {
    RE_ASSERT(ast_ref != RE__AST_NONE);
    RE_ASSERT(ast_ref < (re_int32)re__ast_vec_size(&ast_root->ast_vec));
    return re__ast_vec_getref(&ast_root->ast_vec, (re_size)ast_ref);
}

RE_INTERNAL re_error re__ast_root_add(re__ast_root* ast_root, re__ast ast, re_int32* out_ref) {
    re_error err = RE_ERROR_NONE;
    re_int32 empty_ref = ast_root->last_empty_ref;
    if (empty_ref != RE__AST_NONE) {
        re__ast* empty = re__ast_root_get(ast_root,empty_ref);
        ast_root->last_empty_ref = empty->next_sibling_ref;
        *empty = ast;
        *out_ref = empty_ref;
    } else {
        re_int32 next_ref = (re_int32)re__ast_vec_size(&ast_root->ast_vec);
        if ((err = re__ast_vec_push(&ast_root->ast_vec, ast))) {
            return err;
        }
        *out_ref = next_ref;
    }
    return err;
}

RE_INTERNAL void re__ast_root_remove(re__ast_root* ast_root, re_int32 ast_ref) {
    re__ast* empty = re__ast_root_get(ast_root, ast_ref);
    empty->next_sibling_ref = ast_root->last_empty_ref;
    ast_root->last_empty_ref = ast_ref;
}

RE_INTERNAL void re__ast_root_link_siblings(re__ast_root* ast_root, re_int32 first_sibling_ref, re_int32 next_sibling_ref) {
    re__ast* first_sibling = re__ast_root_get(ast_root, first_sibling_ref);
    re__ast* next_sibling = re__ast_root_get(ast_root, next_sibling_ref);
    first_sibling->next_sibling_ref = next_sibling_ref;
    next_sibling->prev_sibling_ref = first_sibling_ref;
}

RE_INTERNAL void re__ast_root_set_child(re__ast_root* ast_root, re_int32 root_ref, re_int32 child_ref) {
    re__ast* root = re__ast_root_get(ast_root, root_ref);
    root->first_child_ref = child_ref;
}

RE_INTERNAL void re__ast_root_wrap(re__ast_root* ast_root, re_int32 parent_ref, re_int32 inner_ref, re_int32 outer_ref) {
    re__ast* inner = re__ast_root_get(ast_root, inner_ref);
    re__ast* outer = re__ast_root_get(ast_root, outer_ref);
    if (inner->prev_sibling_ref != RE__AST_NONE) {
        re__ast* inner_prev_sibling = re__ast_root_get(ast_root, inner->prev_sibling_ref);
        inner_prev_sibling->next_sibling_ref = outer_ref;
        outer->prev_sibling_ref = inner->prev_sibling_ref;
    } else {
        re__ast* parent = re__ast_root_get(ast_root, parent_ref);
        parent->first_child_ref = outer_ref;
    }
    inner->prev_sibling_ref = RE__AST_NONE;
    outer->first_child_ref = inner_ref;
}

RE_INTERNAL re_int32 re__ast_root_size(re__ast_root* ast_root) {
    return (re_int32)re__ast_vec_size(&ast_root->ast_vec);
}

#if RE_DEBUG

#if 0

RE_INTERNAL void re__ast_root_debug_dump_sexpr_rec(re__ast_root* ast_root, re__debug_sexpr* sexpr, re_int32 ast_root_ref, re_int32 sexpr_root_ref) {
    while (ast_root_ref != RE__AST_NONE) {
        const re__ast* ast = re__ast_root_get(ast_root, ast_root_ref);
        re_int32 ast_sexpr_ref = re__debug_sexpr_new_node(sexpr, sexpr_root_ref);
        switch (ast->type) {
            case RE__AST_TYPE_NONE:
                re__debug_sexpr_new_atom(sexpr, ast_sexpr_ref, "none");
                break;
            case RE__AST_TYPE_RUNE:
                re__debug_sexpr_new_atom(sexpr, ast_sexpr_ref, "char");
                re__debug_sexpr_new_int(sexpr, ast_sexpr_ref, ast->_data.rune);
                break;
            case RE__AST_TYPE_CLASS:
                /*re__charclass_dump(&ast->_data.charclass, (re_size)(lvl+1));*/
                break;
            case RE__AST_TYPE_CONCAT:
                re__debug_sexpr_new_atom(sexpr, ast_sexpr_ref, "concat");
                break;
            case RE__AST_TYPE_ALT:
                re__debug_sexpr_new_atom(sexpr, ast_sexpr_ref, "alt");
                break;
            case RE__AST_TYPE_QUANTIFIER:
                re__debug_sexpr_new_atom(sexpr, ast_sexpr_ref, "quantifier");
                re__debug_sexpr_new_int(sexpr, ast_sexpr_ref, ast->_data.quantifier_info.min);
                re__debug_sexpr_new_int(sexpr, ast_sexpr_ref, ast->_data.quantifier_info.max);
                if (ast->_data.quantifier_info.greediness) {
                    re__debug_sexpr_new_atom(sexpr, ast_sexpr_ref, "greedy");
                } else {
                    re__debug_sexpr_new_atom(sexpr, ast_sexpr_ref, "non-greedy");
                }
                break;
            case RE__AST_TYPE_GROUP: {
                re_int32 flags_node;
                re__debug_sexpr_new_atom(sexpr, ast_sexpr_ref, "group");
                flags_node = re__debug_sexpr_new_node(sexpr, ast_sexpr_ref);
                if (ast->_data.group_info.flags & RE__AST_GROUP_FLAG_CASE_INSENSITIVE) {
                    re__debug_sexpr_new_atom(sexpr, flags_node, "i");
                } else if (ast->_data.group_info.flags & RE__AST_GROUP_FLAG_MULTILINE) {
                    re__debug_sexpr_new_atom(sexpr, flags_node, "m");
                } else if (ast->_data.group_info.flags & RE__AST_GROUP_FLAG_DOT_NEWLINE) {
                    re__debug_sexpr_new_atom(sexpr, flags_node, "s");
                } else if (ast->_data.group_info.flags & RE__AST_GROUP_FLAG_UNGREEDY) {
                    re__debug_sexpr_new_atom(sexpr, flags_node, "U");
                } else if (ast->_data.group_info.flags & RE__AST_GROUP_FLAG_NONMATCHING) {
                    re__debug_sexpr_new_atom(sexpr, flags_node, ":");
                }
                break;
            }
            case RE__AST_TYPE_ASSERT:
                re__debug_sexpr_new_atom(sexpr, ast_sexpr_ref, "assert");
                break;
            case RE__AST_TYPE_ANY_CHAR:
                re__debug_sexpr_new_atom(sexpr, ast_sexpr_ref, "any-char");
                break;
            case RE__AST_TYPE_ANY_BYTE:
                re__debug_sexpr_new_atom(sexpr, ast_sexpr_ref, "any-byte");
                break;
            default:
                RE__ASSERT_UNREACHED();
                break;
        }
        re__ast_root_debug_dump_sexpr_rec(ast_root, sexpr, ast->first_child_ref, ast_sexpr_ref);
        ast_root_ref = ast->next_sibling_ref;
    }
}

RE_INTERNAL void re__ast_root_debug_dump_sexpr(re__ast_root* ast_root, re__debug_sexpr* sexpr, re_int32 sexpr_root_ref) {
    re_int32 root_node;
    root_node = re__debug_sexpr_new_node(sexpr, sexpr_root_ref);
    re__debug_sexpr_new_atom(sexpr, root_node, "ast");
    re__ast_root_debug_dump_sexpr_rec(ast_root, sexpr, 0, root_node);
}

#endif /* #if 0 */

RE_INTERNAL void re__ast_root_debug_dump(re__ast_root* ast_root, re_int32 root_ref, re_int32 lvl) {
    re_int32 i;
    while (root_ref != RE__AST_NONE) {
        const re__ast* ast = re__ast_root_get(ast_root, root_ref);
        for (i = 0; i < lvl; i++) {
            printf("  ");
        }
        printf("%04i | ", root_ref);
        if (ast->prev_sibling_ref == RE__AST_NONE) {
            printf("p=None ");
        } else {
            printf("p=%04i ", ast->next_sibling_ref);
        }
        if (ast->next_sibling_ref == RE__AST_NONE) {
            printf("n=None ");
        } else {
            printf("n=%04i ", ast->next_sibling_ref);
        }
        if (ast->first_child_ref == RE__AST_NONE) {
            printf("c=None ");
        } else {
            printf("c=%04i ", ast->first_child_ref);
        }
        switch (ast->type) {
            case RE__AST_TYPE_NONE:
                printf("NONE");
                break;
            case RE__AST_TYPE_RUNE:
                printf("CHAR: ord=%X ('%c')", ast->_data.rune, ast->_data.rune);
                break;
            case RE__AST_TYPE_CLASS:
                printf("CLASS:\n");
                re__charclass_dump(&ast->_data.charclass, (re_size)(lvl+1));
                break;
            case RE__AST_TYPE_CONCAT:
                printf("CONCAT");
                break;
            case RE__AST_TYPE_ALT:
                printf("ALT");
                break;
            case RE__AST_TYPE_QUANTIFIER:
                printf("QUANTIFIER: %i - %i; %s", 
                    ast->_data.quantifier_info.min,
                    ast->_data.quantifier_info.max,
                    ast->_data.quantifier_info.greediness ? "greedy" : "non-greedy"
                );
                break;
            case RE__AST_TYPE_GROUP:
                printf("GROUP: %c%c%c%c%c",
                    ((ast->_data.group_info.flags & RE__AST_GROUP_FLAG_CASE_INSENSITIVE) ? 'i' : ' '), 
                    ((ast->_data.group_info.flags & RE__AST_GROUP_FLAG_MULTILINE) ? 'm' : ' '), 
                    ((ast->_data.group_info.flags & RE__AST_GROUP_FLAG_DOT_NEWLINE) ? 's' : ' '), 
                    ((ast->_data.group_info.flags & RE__AST_GROUP_FLAG_UNGREEDY) ? 'U' : ' '), 
                    ((ast->_data.group_info.flags & RE__AST_GROUP_FLAG_NONMATCHING) ? ':' : ' ')
                );
                break;
            case RE__AST_TYPE_ASSERT:
                printf("ASSERT: %i", ast->_data.assert_type);
                break;
            case RE__AST_TYPE_ANY_CHAR:
                printf("ANY_CHAR");
                break;
            case RE__AST_TYPE_ANY_BYTE:
                printf("ANY_BYTE");
                break;
            default:
                RE__ASSERT_UNREACHED();
                break;
        }
        printf("\n");
        re__ast_root_debug_dump(ast_root, ast->first_child_ref, lvl + 1);
        root_ref = ast->next_sibling_ref;
    }
}

#endif /* #if RE_DEBUG */


RE_VEC_IMPL_FUNC(re__rune_range, init)
RE_VEC_IMPL_FUNC(re__rune_range, destroy)
RE_VEC_IMPL_FUNC(re__rune_range, push)
RE_VEC_IMPL_FUNC(re__rune_range, get)
RE_VEC_IMPL_FUNC(re__rune_range, insert)
RE_VEC_IMPL_FUNC(re__rune_range, size)
RE_VEC_IMPL_FUNC(re__rune_range, clear)
RE_VEC_IMPL_FUNC(re__rune_range, get_data)

/* Holds a single ASCII character class. */
typedef struct re__charclass_ascii {
    /* Length of name */
    unsigned char name_size;
    const char* name;
    unsigned char num_classes;
    const unsigned char* classes;
} re__charclass_ascii;

/* Table of ASCII character classes */
/* Since ascii characters fit into one byte, we can fit them into uchars. */
static const re__charclass_ascii re__charclass_ascii_defaults[] = {
    {5, "alnum", 3, (unsigned char*)"\x30\x39\x41\x5A\x61\x7A"},
    {5, "alpha", 2, (unsigned char*)"\x41\x5A\x61\x7A"},
    {5, "ascii", 1, (unsigned char*)"\x00\x7F"},
    {5, "blank", 2, (unsigned char*)"\x09\x09\x20\x20"},
    {5, "cntrl", 2, (unsigned char*)"\x00\x1F\x7F\x7F"},
    {5, "digit", 1, (unsigned char*)"\x30\x39"},
    {5, "graph", 1, (unsigned char*)"\x21\x7E"},
    {5, "lower", 1, (unsigned char*)"\x61\x7A"},
    {10, "perl_space", 3, (unsigned char*)"\x09\x0A\x0C\x0D\x20\x20"},
    {5, "print", 1, (unsigned char*)"\x20\x7E"},
    {5, "punct", 4, (unsigned char*)"\x21\x2F\x3A\x40\x5B\x60\x7B\x7E"},
    {5, "space", 2, (unsigned char*)"\x09\x0D\x20\x20"},
    {5, "upper", 1, (unsigned char*)"\x41\x5A"},
    {4, "word", 4, (unsigned char*)"\x30\x39\x41\x5A\x5F\x5F\x61\x7A"},
    {6, "xdigit", 3, (unsigned char*)"\x30\x39\x41\x46\x61\x66"}
};

#define RE__CHARCLASS_ASCII_DEFAULTS_SIZE (sizeof(re__charclass_ascii_defaults) / sizeof(re__charclass_ascii))

RE_INTERNAL void re__charclass_init(re__charclass* charclass) {
    re__rune_range_vec_init(&charclass->ranges);
}

RE_INTERNAL void re__charclass_destroy(re__charclass* charclass) {
    re__rune_range_vec_destroy(&charclass->ranges);
}

RE_INTERNAL re_error re__charclass_push(re__charclass* charclass, re__rune_range range) {
    re__rune_range temp;
    RE_ASSERT(range.min <= range.max);
    re__zero_mem(sizeof(temp), (void*)&temp);
    temp.min = range.min;
    temp.max = range.max;
    return re__rune_range_vec_push(&charclass->ranges, range);
}

RE_INTERNAL const re__rune_range* re__charclass_get_ranges(const re__charclass* charclass) {
    const re__rune_range* out = re__rune_range_vec_get_data(&charclass->ranges);
    RE_ASSERT(out != NULL);
    return out;
}

RE_INTERNAL re_size re__charclass_get_num_ranges(const re__charclass* charclass) {
    return re__rune_range_vec_size(&charclass->ranges);
}

RE_INTERNAL const re__charclass_ascii* re__charclass_ascii_find(re__str* name) {
    re_size i;
    const re__charclass_ascii* found = RE_NULL;
    /* Search table for the matching named character class */
    for (i = 0; i < RE__CHARCLASS_ASCII_DEFAULTS_SIZE; i++) {
        const re__charclass_ascii* cur = &re__charclass_ascii_defaults[i];
        re__str temp;
        /* const string: no need to free */
        re__str_init_const_s(&temp, cur->name_size, (re_char*)cur->name);
        if (re__str_cmp(&temp, name) == 0) {
            found = cur;
            break;
        }
    }
    return found;
}

RE_INTERNAL re_error re__charclass_init_from_ascii(re__charclass* charclass, const re__charclass_ascii* ascii_cc, int inverted) {
    re_error err = RE_ERROR_NONE;
    re_size i;
    re__rune_range temp;
    re_rune last_max = -1;
    /* Dump found ranges into a char class */
    re__charclass_init(charclass);
    for (i = 0; i < ascii_cc->num_classes; i++) {
        if (inverted) {
            if (last_max == -1) {
                temp.min = 0;
                temp.max = ascii_cc->classes[i*2] - 1;
            } else {
                temp.min = last_max;
                temp.max = ascii_cc->classes[i*2] - 1;
            }
            last_max = ascii_cc->classes[i*2 + 1] + 1;
        } else {
            temp.min = ascii_cc->classes[i*2];
            temp.max = ascii_cc->classes[i*2 + 1];
        }
        if (temp.max >= temp.min) {
            if ((err = re__charclass_push(charclass, temp))) {
                re__charclass_destroy(charclass);
                return err;
            }
        }
    }
    if (inverted) {
        if (last_max == -1) {
            temp.min = 0;
            temp.max = RE_RUNE_MAX;
        } else {
            temp.min = last_max;
            temp.max = RE_RUNE_MAX;
        }
        if (temp.min != temp.max) {
            if ((err = re__charclass_push(charclass, temp))) {
                re__charclass_destroy(charclass);
                return err;
            }
        }
    }
    return err;
}

RE_INTERNAL re_error re__charclass_init_from_class(re__charclass* charclass, re__charclass_ascii_type type, int inverted) {
    return re__charclass_init_from_ascii(charclass, &re__charclass_ascii_defaults[type], inverted);
}

/* Returns RE_ERROR_INVALID if not found */
RE_INTERNAL re_error re__charclass_init_from_string(re__charclass* charclass, re__str* name, int inverted) {
    const re__charclass_ascii* found = re__charclass_ascii_find(name);
    /* Found is NULL if we didn't find anything during the loop */
    if (found == RE_NULL) {
        return RE_ERROR_INVALID;
    }
    return re__charclass_init_from_ascii(charclass, found, inverted);
}

RE_INTERNAL void re__charclass_builder_init(re__charclass_builder* builder) {
    re__rune_range_vec_init(&builder->ranges);
    builder->should_invert = 0;
    builder->highest = -1;
}

RE_INTERNAL void re__charclass_builder_begin(re__charclass_builder* builder) {
    re__rune_range_vec_clear(&builder->ranges);
    builder->should_invert = 0;
    builder->highest = -1;
}

RE_INTERNAL void re__charclass_builder_destroy(re__charclass_builder* builder) {
    re__rune_range_vec_destroy(&builder->ranges);
}

RE_INTERNAL void re__charclass_builder_invert(re__charclass_builder* builder) {
    builder->should_invert = 1;
}

/* Insert a range into the builder. */
/* Inserts in sorted order, that is, all ranges are ordered by their low bound. */
RE_INTERNAL re_error re__charclass_builder_insert_range(re__charclass_builder* builder, re__rune_range range) {
    re_error err = RE_ERROR_NONE;
    re_size i;
    re_size max = re__rune_range_vec_size(&builder->ranges);
    /* The range should be normalized. */
    RE_ASSERT(range.min <= range.max);
    /* We don't want to penalize people who sort their character classes already,
     * so we check if the lower bound is higher than the absolute highest bound,
     * and just push it on the end of the list of ranges */
    /* Whether this optimization is actually good is unknown */
    if (range.min > builder->highest) {
        goto just_push;
    }
    for (i = 0; i < max; i++) {
        re__rune_range cmp = re__rune_range_vec_get(&builder->ranges, i);
        if (range.min < cmp.min) {
            /* Keep track of the highest range */
            if (range.max > builder->highest) {
                builder->highest = range.max;
            }
            if ((err = re__rune_range_vec_insert(&builder->ranges, i, range))) {
                return err;
            }
            /* Exit early */
            return err;
        }
    }
just_push:
    /* Range is higher than everything else */
    builder->highest = range.max;
    return re__rune_range_vec_push(&builder->ranges, range);
}

/* Insert a character class into a builder. */
/* Used when putting a named character class inside of an unnamed one. */
RE_INTERNAL re_error re__charclass_builder_insert_class(re__charclass_builder* builder, re__charclass* charclass) {
    re_size i;
    re_size sz = re__rune_range_vec_size(&charclass->ranges);
    re_error err = RE_ERROR_NONE;
    for (i = 0; i < sz; i++) {
        /* Iterate through ranges, adding them all */
        /* Perhaps could be optimized more */
        re__rune_range cur_range = re__rune_range_vec_get(&charclass->ranges, i);
        if ((err = re__charclass_builder_insert_range(builder, cur_range))) {
            return err;
        }
    }
    return err;
}

#if RE_DEBUG

RE_INTERNAL int re__charclass_builder_verify(const re__charclass_builder* builder) {
    re_size i;
    re__rune_range last;
    last.min = -1;
    last.max = -1;
    for (i = 0; i < re__rune_range_vec_size(&builder->ranges); i++) {
        re__rune_range rr = re__rune_range_vec_get_data(&builder->ranges)[i];
        if (rr.min <= last.min) {
            return 0;
        }
        last = rr;
    }
    return 1;
}

#endif /* #if RE_DEBUG */

RE_INTERNAL re_error re__charclass_builder_finish(re__charclass_builder* builder, re__charclass* charclass) {
    re_error err = RE_ERROR_NONE;
    /* Temporary range */
    re_rune temp_min = -1;
    re_rune temp_max = -1;
    re_rune last_temp_max = -1;
    re_size i;
    re_size ranges_size = re__rune_range_vec_size(&builder->ranges);
    re__rune_range new_range;
    re__charclass_init(charclass);
    for (i = 0; i < ranges_size; i++) {
        re__rune_range cur = re__rune_range_vec_get(&builder->ranges, i);
        RE_ASSERT(cur.min <= cur.max);
        if (temp_min == -1) {
            /* First range */
            temp_min = cur.min;
            temp_max = cur.max;
            /* Result */
            /* temp_min      temp_max
             * ***************         */
        } else if (cur.min <= temp_max) {
            if (cur.max > temp_max) {
                /* Current range intersects with temp_min/temp_max range */
                /* temp_min      temp_max
                 * ***************   
                 *             cur.min       cur.max
                 *             ***************           */
                /* Extend range */
                temp_max = cur.max;
                /* Result: */
                /* temp_min                  temp_max
                 * ***************************         */
            } else {
                /* Current range is contained within temp_min/temp_max range */
                /* temp_min      temp_max
                 * ***************   
                 *  cur.min cur.max
                 *  *********         */
                /* Nothing needs to be done */
                /* Result: */
                /* temp_min      temp_max
                 * ***************         */
            }
        } else if (cur.min == temp_max + 1) {
            /* Current range is adjacent to temp_min/temp_max range */
            /* temp_min      temp_max
             * ***************   
             *                cur.min    cur.max
             *                ************       */
            /* Concatenate range */
            temp_max = cur.max;
            /* Result: */
            /* temp_min                  temp_max
             * ***************************         */
        } else if (cur.min > temp_max) {
            /* Current range is outside of temp_min/temp_max range */
            /* temp_min      temp_max
             * ***************   
             *                      cur.min    cur.max
             *                      ************       */
            /* Create a new range, and push the temp_min/temp_max range */
            if (!builder->should_invert) {
                new_range.min = temp_min;
                new_range.max = temp_max;
                if ((err = re__charclass_push(charclass, new_range))) {
                    goto destroy_out;
                }
            } else {
                /* For inverted classes, we need to keep track of the previous
                 * temp_maximum bound so we can "fill in the gaps" */
                if (last_temp_max == -1) {
                    new_range.min = 0;
                    new_range.max = temp_min - 1;
                } else {
                    new_range.min = last_temp_max + 1;
                    new_range.max = temp_min - 1;
                }
                RE_ASSERT(new_range.min <= new_range.max + 1);
                if (new_range.min <= new_range.max) {
                    if ((err = re__charclass_push(charclass, new_range))) {
                        goto destroy_out;
                    }
                    last_temp_max = temp_max;
                }
            }
            temp_min = cur.min;
            temp_max = cur.max;
            /* Result */
            /* old_temp_min  old_temp_max
             * ***************   
             *                      temp_min   temp_max
             *                      ************         */
        }
    }
    /* Finished: add the temporary range */
    if (!builder->should_invert) {
        if (temp_min == -1) {
            /* empty class */
            /* no ranges to add */
        } else {
            new_range.min = temp_min;
            new_range.max = temp_max;
            if ((err = re__charclass_push(charclass, new_range))) {
                goto destroy_out;
            }
        }
    } else {
        /* If we are inverted, add the range *up to* the temporary range */
        if (last_temp_max == -1) {
            /* Initial inversion */
            new_range.min = 0;
            new_range.max = temp_min - 1;
        } else {
            /* Intermediate inversions */
            new_range.min = last_temp_max + 1;
            new_range.max = temp_min - 1;
        }
        RE_ASSERT(new_range.min <= new_range.max + 1);
        if (new_range.min <= new_range.max) {
            /* If the lo/hi are equal, then don't bother */
            if ((err = re__charclass_push(charclass, new_range))) {
                goto destroy_out;
            }
        }
        /* Add the final inverted range, the one that goes from the maximum non-
         * inverted range to infinity (RE_RUNE_MAX) */
        if (new_range.max < RE_RUNE_MAX) {
            if (temp_max == -1) {
                /* empty class */
                new_range.min = 0;
            } else {
                new_range.min = temp_max + 1;
            }
            new_range.max = RE_RUNE_MAX;
            if ((err = re__charclass_push(charclass, new_range))) {
                goto destroy_out;
            }
        }
    }
    return err;
destroy_out:
    re__charclass_destroy(charclass);
    return err;
}

int re__charclass_equals(const re__charclass* charclass, const re__charclass* other) {
    re_size cs = re__charclass_get_num_ranges(charclass);
    re_size os = re__charclass_get_num_ranges(other);
    re_size i;
    if (cs != os) {
        return 0;
    }
    for (i = 0; i < cs; i++) {
        re__rune_range cr = re__charclass_get_ranges(charclass)[i];
        re__rune_range or = re__charclass_get_ranges(charclass)[i];
        if (!re__rune_range_equals(cr, or)) {
            return 0;
        }
    }
    return 1;
}

#if RE_DEBUG

void re__charclass_dump(const re__charclass* charclass, re_size lvl) {
    re_size i;
    re_size sz = re__rune_range_vec_size(&charclass->ranges);
    for (i = 0; i < sz; i++) {
        re__rune_range cur = re__rune_range_vec_get(&charclass->ranges, i);
        re_size j;
        for (j = 0; j < lvl; j++) {
            printf("  ");
        }
        printf("%X - %X\n", (re_rune)cur.min, (re_rune)cur.max);
    }
}

RE_INTERNAL int re__charclass_verify(const re__charclass* charclass) {
    re_size i;
    re__rune_range last;
    last.min = -1;
    last.max = -1;
    for (i = 0; i < re__charclass_get_num_ranges(charclass); i++) {
        re__rune_range rr = re__charclass_get_ranges(charclass)[i];
        if (rr.min <= last.max) {
            return 0;
        }
        last = rr;
    }
    return 1;
}

#endif /* #if RE_DEBUG */


RE_INTERNAL void re__compile_patches_init(re__compile_patches* patches) {
    patches->first_inst = RE__PROG_LOC_INVALID;
    patches->last_inst = RE__PROG_LOC_INVALID;
}

RE_INTERNAL void re__compile_patches_prepend(re__compile_patches* patches, re__prog* prog, re__prog_loc to, int secondary) {
    re__prog_loc out_val = to << 1;
    if (secondary) {
        out_val |= 1;
    }
    if (patches->first_inst == RE__PROG_LOC_INVALID) {
        patches->first_inst = out_val;
        patches->last_inst = out_val;
    } else {
        re__prog_inst* inst = re__prog_get(prog, to >> 1);
        if (!secondary) {
            re__prog_inst_set_primary(inst, patches->first_inst);
        } else {
            re__prog_inst_set_split_secondary(inst, patches->first_inst);
        }
        patches->first_inst = out_val;
    }
}

RE_INTERNAL void re__compile_patches_append(re__compile_patches* patches, re__prog* prog, re__prog_loc to, int secondary) {
    re__prog_loc out_val = to << 1;
    if (secondary) {
        out_val |= 1;
    }
    if (patches->first_inst == RE__PROG_LOC_INVALID) {
        patches->first_inst = out_val;
        patches->last_inst = out_val;
    } else {
        re__prog_inst* inst = re__prog_get(prog, patches->last_inst >> 1);
        if (!(patches->last_inst & 1)) {
            re__prog_inst_set_primary(inst, out_val);
        } else {
            re__prog_inst_set_split_secondary(inst, out_val);
        }
        patches->last_inst = out_val;
    }
}

RE_INTERNAL void re__compile_patches_merge(re__compile_patches* patches, re__prog* prog, re__compile_patches* merge_from) {
    re__prog_loc first_loc;
    re__prog_inst* inst;
    RE_ASSERT(merge_from->first_inst != RE__PROG_LOC_INVALID);
    if (patches->first_inst == RE__PROG_LOC_INVALID) {
        *patches = *merge_from;
        return;
    }
    first_loc = merge_from->first_inst;
    inst = re__prog_get(prog, patches->last_inst >> 1);
    if (!(patches->last_inst & 1)) {
        re__prog_inst_set_primary(inst, first_loc);
    } else {
        re__prog_inst_set_split_secondary(inst, first_loc);
    }
    patches->last_inst = merge_from->last_inst;
}

RE_INTERNAL void re__compile_patches_patch(re__compile_patches* patches, re__prog* prog, re__prog_loc to) {
    re__prog_loc current_loc = patches->first_inst;
    re__prog_inst* inst = re__prog_get(prog, current_loc >> 1);
    while (current_loc != patches->last_inst) {
        re__prog_loc next_loc;
        if (!(current_loc & 1)) {
            next_loc = re__prog_inst_get_primary(inst);
            re__prog_inst_set_primary(inst, to);
        } else {
            next_loc = re__prog_inst_get_split_secondary(inst);
            re__prog_inst_set_split_secondary(inst, to);
        }
        current_loc = next_loc;
        inst = re__prog_get(prog, current_loc >> 1);
    }
    if (!(current_loc & 1)) {
        re__prog_inst_set_primary(inst, to);
    } else {
        re__prog_inst_set_split_secondary(inst, to);
    }
}

#if RE_DEBUG

RE_INTERNAL void re__compile_patches_dump(re__compile_patches* patches, re__prog* prog) {
    re__prog_loc current_loc = patches->first_inst;
    if (current_loc == RE__PROG_LOC_INVALID) {
        printf("<no patches>\n");
        return;
    }
    while (current_loc != patches->last_inst) {
        re__prog_loc next_loc;
        re__prog_inst* inst = re__prog_get(prog, current_loc >> 1);
        if (!(current_loc & 1)) {
            next_loc = re__prog_inst_get_primary(inst);
        } else {
            next_loc = re__prog_inst_get_split_secondary(inst);
        }
        printf("%04X | %s\n", current_loc >> 1, (current_loc & 1) ? "secondary" : "primary");
        current_loc = next_loc;
    }
    printf("%04X | %s\n", current_loc >> 1, (current_loc & 1) ? "secondary" : "primary");
}

#endif /* #if RE_DEBUG */

RE_INTERNAL void re__compile_frame_init(re__compile_frame* frame, re_int32 ast_root_ref, re_int32 ast_child_ref, re__compile_patches patches, re__prog_loc start, re__prog_loc end) {
    frame->ast_root_ref = ast_root_ref;
    frame->ast_child_ref = ast_child_ref;
    frame->patches = patches;
    frame->start = start;
    frame->end = end;
    frame->rep_idx = 0;
}

RE_INTERNAL void re__compile_init(re__compile* compile, re* re) {
    compile->re = re;
    compile->frames = NULL;
    compile->frames_size = 0;
    compile->frame_ptr = 0;
    compile->ast_ref = 0;
    re__compile_charclass_init(&compile->char_comp);
}

RE_INTERNAL void re__compile_destroy(re__compile* compile) {
    re__compile_charclass_destroy(&compile->char_comp);
}

RE_INTERNAL void re__compile_frame_push(re__compile* compile, re__compile_frame frame) {
    RE_ASSERT(compile->frame_ptr != compile->frames_size);
    compile->frames[compile->frame_ptr++] = frame;
}

RE_INTERNAL re__compile_frame re__compile_frame_pop(re__compile* compile) {
    RE_ASSERT(compile->frame_ptr != 0);
    return compile->frames[--compile->frame_ptr];
}

RE_INTERNAL int re__compile_gen_utf8(re_rune codep, re_uint8* out_buf) {
    if (codep <= 0x7F) {
		out_buf[0] = codep & 0x7F;
        return 1;
	} else if (codep <= 0x07FF) {
		out_buf[0] = ((codep >>  6) & 0x1F) | 0xC0;
		out_buf[1] = ((codep >>  0) & 0x3F) | 0x80;
        return 2;
	} else if (codep <= 0xFFFF) {
		out_buf[0] = ((codep >> 12) & 0x0F) | 0xE0;
		out_buf[1] = ((codep >>  6) & 0x3F) | 0x80;
		out_buf[2] = ((codep >>  0) & 0x3F) | 0x80;
        return 3;
	} else if (codep <= 0x10FFFF) {
		out_buf[0] = ((codep >> 18) & 0x07) | 0xF0;
		out_buf[1] = ((codep >> 12) & 0x3F) | 0x80;
		out_buf[2] = ((codep >>  6) & 0x3F) | 0x80;
		out_buf[3] = ((codep >>  0) & 0x3F) | 0x80;
        return 4;
	} else {
        RE__ASSERT_UNREACHED();
        /* fall back to white square */
		return re__compile_gen_utf8(0x25A1, out_buf);
	}
}

RE_INTERNAL re_error re__compile_regex(re__compile* compile) {
    re_error err = RE_ERROR_NONE;
    re__compile_frame initial_frame;
    re__compile_patches initial_patches;
    re__ast_root* ast_root = &compile->re->data->parse.ast_root;
    re__prog* prog = &compile->re->data->program;
    re__compile_frame returned_frame;
    re__prog_inst fail_inst;
    /* Allocate memory for frames */
    /* depth_max + 1 because we include an extra frame for terminals within the
     * deepest multi-child node */
    compile->frames_size = compile->re->data->parse.depth_max + 1;
    compile->frames = (re__compile_frame*)RE_MALLOC((sizeof(re__compile_frame)*((re_size)compile->frames_size)));
    if (compile->frames == RE_NULL) {
        err = RE_ERROR_NOMEM;
        goto error;
    }
    /* Add the FAIL instruction to the program */
    re__prog_inst_init_fail(&fail_inst);
    if ((err = re__prog_add(prog, fail_inst))) {
        goto error;
    }
    re__compile_patches_init(&initial_patches);
    /* Start first frame */
    re__compile_frame_init(&initial_frame, 0, 0, initial_patches, 0, 0);
    /* Push it */
    re__compile_frame_push(compile, initial_frame);
    /* While there are nodes left to compile... */
    while (compile->frame_ptr != 0) {
        re__compile_frame top_frame = re__compile_frame_pop(compile);
        re__ast* top_node = re__ast_root_get(ast_root, top_frame.ast_root_ref);
        re__ast_type top_node_type = top_node->type;
        const re__prog_loc this_start_pc = re__prog_size(prog);
        int push_child = 0;
        int next_child = 0;
        if (top_node_type == RE__AST_TYPE_RUNE) {
            /* Generates a single Byte or series of Byte instructions for a
             * UTF-8 codepoint. */
            /*    +0
             * ~~~+-------+~~~
             * .. | Byte  | ..
             * .. | Instr | ..
             * ~~~+---+---+~~~
             *        |
             *        +-----1--> ... */
            re__prog_inst new_inst;
            re_uint8 utf8_bytes[4];
            int num_bytes = re__compile_gen_utf8(re__ast_get_rune(top_node), utf8_bytes);
            int i;
            for (i = 0; i < num_bytes; i++) {
                if (i == num_bytes - 1) {
                    /* Add an outgoing patch (1) */
                    re__compile_patches_append(&top_frame.patches, prog, re__prog_size(prog), 0);
                }
                re__prog_inst_init_byte(
                    &new_inst, 
                    utf8_bytes[i]
                );
                if ((err = re__prog_add(prog, new_inst))) {
                    goto error;
                }
            }
            next_child = 1;
        } else if (top_node_type == RE__AST_TYPE_CLASS) {
            /* Generates a character class, which is a complex series of Byte
             * and Split instructions. */
            if ((err = re__compile_charclass_gen(
                &compile->char_comp, 
                &top_node->_data.charclass, 
                prog, &top_frame.patches))) {
                goto error;
            }
            next_child = 1;
        } else if (top_node_type == RE__AST_TYPE_CONCAT) {
            /* Generates each child node, and patches them all together,
             * leaving the final child's outgoing branch targets unpatched. */
            /*    +0        +L(C0)          ...
             * ~~~+--.....--+--.....--+.....+--.....--+~~~
             * .. | Child 0 | Child 1 |.....| Child n | ..
             * .. | Instrs  | Instrs  |.....| Instrs  | ..
             * ~~~+--..+..--+--..+..+-+.....+--..+..+-+~~~
             *         |      ^  |  |         ^  |  |
             *         1      |  2  3         |  +--|----4-> ...
             *         |      |  |  |         |     |
             *         |      |  +--|---...---+     +----5-> ...
             *         +------+     |         |
             *                      +---...---+                 */
            RE_ASSERT(top_node->first_child_ref != RE__AST_NONE);
            if (top_frame.ast_child_ref != RE__AST_NONE) {
                re__ast* child = re__ast_root_get(ast_root, top_frame.ast_child_ref);
                if (child->prev_sibling_ref == RE__AST_NONE) {
                    /* Before first child */
                    push_child = 1;
                    compile->ast_ref = child->next_sibling_ref;
                } else {
                    /* Patch outgoing branches (1, 2, 3) */
                    re__compile_patches_patch(&returned_frame.patches, prog, this_start_pc);
                    /* There are children left to check */
                    push_child = 1;
                    compile->ast_ref = child->next_sibling_ref;
                }
            } else {
                /* After last child */
                /* Collect outgoing branches (4, 5) */
                re__compile_patches_merge(&top_frame.patches, prog, &returned_frame.patches);
                next_child = 1;
            }
        } else if (top_node_type == RE__AST_TYPE_ALT) {
            /* For each child node except for the last one, generates a SPLIT 
             * instruction, and then the instructions of the child.
             * Each split instruction's primary branch target is patched to the
             * next child, and the secondary branch targets are patched to the 
             * next split instruction. Leaves each child's outgoing branch
             * targets unpatched. */
            /*    +0      +1        +L(C0)+1                ...
             * ~~~+-------+--.....--+-------+--.....--+.....+--.....--+~~~
             * .. | Split | Child 0 | Split | Child 1 |.....| Child n | ..
             * .. | Instr | Instrs  | Instr | Instrs  |.....| Instrs  | ..
             * ~~~+---+-+-+--..+..+-+---+-+-+--..+..+-+.....+--..+..+-+~~~
             *        | |   ^  |  |   ^ | |   ^  |  |         ^  |  |
             *        | |   |  |  |   | | |   |  |  |         |  |  |
             *        +---1-+  |  |   | +---3-+  |  |         |  |  |
             *          |      |  |   |   |      |  |         |  |  |
             *          +-----------2-+   +-------------...-4-+  |  |
             *                 |  |              |  |            |  |
             *                 +------------------------...--------------5-> ...
             *                    |              |  |            |  |
             *                    +---------------------...--------------6-> ...
             *                                   |  |            |  |
             *                                   +------...--------------7-> ...
             *                                      |            |  |
             *                                      +---...--------------8-> ...
             *                                                   |  |
             *                                                   +-------9-> ...
             *                                                      |
             *                                                      +---10-> ...
             */
            RE_ASSERT(top_node->first_child_ref != RE__AST_NONE);
            if (top_frame.ast_child_ref != RE__AST_NONE) {
                re__ast* child = re__ast_root_get(ast_root, top_frame.ast_child_ref);
                if (child->prev_sibling_ref == RE__AST_NONE) {
                    /* Before first child */
                    /* Initialize split instruction */
                    re__prog_inst new_inst;
                    re__prog_inst_init_split(
                        &new_inst, 
                        this_start_pc + 1, /* Outgoing branch (1) */
                        RE__PROG_LOC_INVALID /* Will become outgoing branch (2) */
                    );
                    /* Add the Split instruction */
                    if ((err = re__prog_add(prog, new_inst))) {
                        goto error;
                    }
                    compile->ast_ref = top_node->next_sibling_ref;
                } else {
                    /* Before intermediate children and last child */
                    /* Patch the secondary branch target of the old SPLIT
                     * instruction. Corresponds to outgoing branch (2). */
                    /* top.seg.end points to the instruction after the old split 
                     * instruction, since we didn't set the endpoint before the 
                     * first child. */
                    re__prog_loc old_split_loc = top_frame.end - 1;
                    re__prog_inst* old_inst = re__prog_get(prog, old_split_loc);
                    /* Patch outgoing branch (2). */
                    re__prog_inst_set_split_secondary(old_inst, this_start_pc);
                    /* Collect outgoing branches (5, 6, 7, 8). */
                    re__compile_patches_merge(&top_frame.patches, prog, &returned_frame.patches);
                    if (child->next_sibling_ref != RE__AST_NONE) {
                        /* Before intermediate children and NOT last child */
                        re__prog_inst new_inst;
                        /* Create the intermediary SPLIT instruction, if there
                         * are more than two child nodes in the alternation. */
                        re__prog_inst_init_split(
                            &new_inst, 
                            this_start_pc + 1, /* Outgoing branch (3) */
                            RE__PROG_LOC_INVALID /* Outgoing branch (4) */
                        );
                        /* Add it to the program. */
                        if ((err = re__prog_add(prog, new_inst))) {
                            goto error;
                        }
                    }
                }
                /* Before every child, including the first and last one */
                push_child = 1;
            } else {
                /* After last child */
                /* Collect outgoing branches (9, 10). */
                re__compile_patches_merge(&top_frame.patches, prog, &returned_frame.patches);
            }
        } else if (top_node_type == RE__AST_TYPE_QUANTIFIER) {
            /*   *   min=0 max=INF */ /* Spl E -> 1 to 2 and out, 2 to 1 */
            /*   +   min=1 max=INF */ /* E Spl -> 1 to 2, 2 to 1 and out */
            /*   ?   min=0 max=1   */ /* Spl E -> 1 to 2 and out, 2 to out */
            /*  {n}  min=n max=n+1 */ /* E repeated -> out */
            /* {n, } min=n max=INF */ /* E repeated, Spl -> spl to last and out */
            /* {n,m} min=n max=m+1 */ /* E repeated, E Spl E Spl E*/
            re_int32 min = re__ast_get_quantifier_min(top_node);
            re_int32 max = re__ast_get_quantifier_max(top_node);
            re_int32 int_idx = top_frame.rep_idx;
            top_frame.rep_idx++;
            if (int_idx < min) {
                /* Generate child min times */
                if (int_idx > 0) {
                    /* Patch previous */
                    re__compile_patches_patch(&returned_frame.patches, prog, this_start_pc);
                }
                push_child = 1;
                compile->ast_ref = top_node->first_child_ref;
            } else { /* int_idx >= min */
                if (max == RE__AST_QUANTIFIER_INFINITY) {
                    re__prog_inst new_spl;
                    if (min == 0) {
                        if (int_idx == 0) {
                            re__prog_inst_init_split(&new_spl, this_start_pc + 1, RE__PROG_LOC_INVALID);
                            if ((err = re__prog_add(prog, new_spl))) {
                                goto error;
                            }
                            re__compile_patches_append(&top_frame.patches, prog, this_start_pc, 1);
                            push_child = 1;
                            compile->ast_ref = top_node->first_child_ref;
                        } else if (int_idx == 1) {
                            re__compile_patches_patch(&returned_frame.patches, prog, top_frame.end - 1);
                        }
                    } else {
                        re__compile_patches_patch(&returned_frame.patches, prog, this_start_pc);
                        re__prog_inst_init_split(&new_spl, returned_frame.start, RE__PROG_LOC_INVALID);
                        if ((err = re__prog_add(prog, new_spl))) {
                            goto error;
                        }
                        re__compile_patches_append(&top_frame.patches, prog, this_start_pc, 1);
                    }
                } else {
                    if (int_idx > 0) {
                        re__compile_patches_patch(&returned_frame.patches, prog, this_start_pc);
                    }
                    if (int_idx <= max - 2) {
                        re__prog_inst new_spl;
                        re__prog_inst_init_split(&new_spl, this_start_pc + 1, RE__PROG_LOC_INVALID);
                        if ((err = re__prog_add(prog, new_spl))) {
                            goto error;
                        }
                        re__compile_patches_append(&top_frame.patches, prog, this_start_pc, 1);
                        push_child = 1;
                        compile->ast_ref = top_node->first_child_ref;
                    } else {
                        re__compile_patches_merge(&top_frame.patches, prog, &returned_frame.patches);
                    }
                }
            }
        } else if (top_node_type == RE__AST_TYPE_GROUP) {
            RE_ASSERT(top_node->first_child_ref != RE__AST_NONE);
            if (top_frame.ast_child_ref != RE__AST_NONE) {
                /* Before child */
                push_child = 1;
                next_child = 1;
                compile->ast_ref = top_node->first_child_ref;
            } else {
                /* After child */
                re__compile_patches_merge(&top_frame.patches, prog, &returned_frame.patches);
            }
        } else if (top_node_type == RE__AST_TYPE_ASSERT) {
            /* Generates a single Assert instruction. */
            /*    +0
             * ~~~+-------+~~~
             * .. |  Ass  | ..
             * .. | Instr | ..
             * ~~~+---+---+~~~
             *        |
             *        +-----1--> ... */
            re__prog_inst new_inst;
            re__prog_inst_init_assert(
                &new_inst,
                (re_uint32)re__ast_get_assert_type(top_node)
            ); /* Creates unpatched branch (1) */
            if ((err = re__prog_add(prog, new_inst))) {
                goto error;
            }
            /* Add an outgoing patch (1) */
            re__compile_patches_append(&top_frame.patches, prog, this_start_pc, 0);
            next_child = 1;
        } else if (top_node_type == RE__AST_TYPE_ANY_CHAR) {
            /* Generates a single Byte Range instruction. */
            /*    +0
             * ~~~+-------+~~~
             * .. | ByteR | ..
             * .. | Instr | ..
             * ~~~+---+---+~~~
             *        |
             *        +-----1--> ... */
            re__prog_inst new_inst;
            re__prog_inst_init_byte_range(
                &new_inst,
                0, 
                255
            ); /* Creates unpatched branch (1) */
            if ((err = re__prog_add(prog, new_inst))) {
                goto error;
            }
            /* Add an outgoing patch (1) */
            re__compile_patches_append(&top_frame.patches, prog, this_start_pc, 0);
            next_child = 1;
        } else {
            RE__ASSERT_UNREACHED();
        }
        /* Set the end of the segment to the next instruction */
        top_frame.end = re__prog_size(prog);
        if (next_child) {
            re__ast* child = re__ast_root_get(ast_root, top_frame.ast_child_ref);
            /* Increment the child index on the top frame before we push
             * it again */
            top_frame.ast_child_ref = child->next_sibling_ref;
        }
        if (push_child) {
            re__compile_frame up_frame;
            re__compile_patches up_patches;
            RE_ASSERT(compile->ast_ref != RE__AST_NONE);
            re__compile_frame_push(compile, top_frame);
            /* Prepare the child's patches */
            re__compile_patches_init(&up_patches);
            /* Prepare the child's stack frame */
            re__compile_frame_init(
                &up_frame,
                compile->ast_ref,
                0, /* Start at first child *of* child */
                up_patches,
                top_frame.end,
                top_frame.end
            );
            re__compile_frame_push(compile, up_frame);
        }
        returned_frame = top_frame;
    }
    /* There should be no more frames. */
    RE_ASSERT(compile->frame_ptr == 0);
    /* Link the returned patches to a final MATCH instruction. */
    re__compile_patches_patch(&returned_frame.patches, prog, re__prog_size(prog));
    {
        re__prog_inst match_inst;
        re__prog_inst_init_match(&match_inst, 0);
        if ((err = re__prog_add(prog, match_inst))) {
            goto error;
        }
    }
    RE_FREE(compile->frames);
    compile->frames = NULL;
    re__prog_debug_dump(&compile->re->data->program);
    return err;
error:
    if (compile->frames != RE_NULL) {
        RE_FREE(compile->frames);
    }
    compile->frames = NULL;
    if (err == RE__ERROR_PROGMAX) {
        re__str err_str;
        RE__STR_INIT_CONST(&err_str, "compiled program length exceeds maximum of " RE__STRINGIFY(RE__PROG_SIZE_MAX) " instructions");
        re__set_error_str(compile->re, &err_str);
        err = RE_ERROR_COMPILE;
    }
    if (err == RE_ERROR_COMPILE) {
        RE_ASSERT(re__str_size(&compile->re->data->error_string));
    } else {
        re__set_error_generic(compile->re, err);
    }
    return err;
}

#if RE_DEBUG

void re__compile_debug_dump(re__compile* compile) {
    re_int32 i;
    printf("%u frames / %u frames:\n", (re_uint32)compile->frame_ptr, (re_uint32)compile->frames_size);
    for (i = 0; i < compile->frames_size; i++) {
        re__compile_frame* cur_frame = &compile->frames[i];
        printf("  Frame %u:\n", i);
        printf("    AST root reference: %i\n", cur_frame->ast_root_ref);
        printf("    AST child reference: %i\n", cur_frame->ast_child_ref);
        printf("    Start loc: %u\n", cur_frame->start);
        printf("    End loc: %u\n", cur_frame->end);
    }
}

#endif /* #if RE_DEBUG */


RE_VEC_IMPL_FUNC(re__compile_charclass_tree, init)
RE_VEC_IMPL_FUNC(re__compile_charclass_tree, destroy)
RE_VEC_IMPL_FUNC(re__compile_charclass_tree, clear)
RE_VEC_IMPL_FUNC(re__compile_charclass_tree, size)
RE_VEC_IMPL_FUNC(re__compile_charclass_tree, getref)
RE_VEC_IMPL_FUNC(re__compile_charclass_tree, push)

/* Get a pointer to a node in the tree, given its index. */
RE_INTERNAL re__compile_charclass_tree* re__compile_charclass_tree_get(re__compile_charclass* char_comp, re_int32 root_ref) {
    RE_ASSERT(root_ref != RE__COMPILE_CHARCLASS_TREE_NONE);
    RE_ASSERT(root_ref < (re_int32)re__compile_charclass_tree_vec_size(&char_comp->tree));
    return re__compile_charclass_tree_vec_getref(&char_comp->tree, (re_size)root_ref);
}

/* Initialize a tree node as a terminal node. */
void re__compile_charclass_tree_init(re__compile_charclass_tree* root, re__byte_range byte_range) {
    root->byte_range = byte_range;
    root->prev_sibling_ref = RE__COMPILE_CHARCLASS_TREE_NONE;
    root->next_sibling_ref = RE__COMPILE_CHARCLASS_TREE_NONE;
    root->first_child_ref = RE__COMPILE_CHARCLASS_TREE_NONE;
    root->last_child_ref = RE__COMPILE_CHARCLASS_TREE_NONE;
    root->hash = 0;
}

/* Create a new child if it doesn't exist, otherwise return the child with the
 * corresponding range. */
re_error re__compile_charclass_touch_child(re__compile_charclass* char_comp, re_int32 root_ref, re__byte_range byte_range, re_int32* out_child_ref) {
    re__compile_charclass_tree* root = re__compile_charclass_tree_get(char_comp, root_ref);
    /* New child node */
    re__compile_charclass_tree new_child;
    /* Reference to the next created node */
    re_int32 next_root_ref = (re_int32)re__compile_charclass_tree_vec_size(&char_comp->tree);
    re_error err = RE_ERROR_NONE;
    if (root->first_child_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
        /* Root has no children. Add this range as a child and return it. */
        /*     creates this node
         *           \/ \/
         * [root]---[AA-BB] */
        re__compile_charclass_tree_init(&new_child, byte_range);
        /* Set root's children to just the new node */
        root->first_child_ref = next_root_ref;
        root->last_child_ref = next_root_ref;
        if ((err = re__compile_charclass_tree_vec_push(&char_comp->tree, new_child))) {
            return err;
        }
        /* tree is invalid */
        *out_child_ref = next_root_ref;
    } else {
        /* Root has children, either return the matching child or create a new
         * one. */
        re_int32 current_child_ref = root->first_child_ref;
        re__compile_charclass_tree* current_child = re__compile_charclass_tree_get(char_comp, current_child_ref);
        while (1) {
            if (re__byte_range_equals(byte_range, current_child->byte_range)) {
                /* Found matching child */
                *out_child_ref = current_child_ref;
                return RE_ERROR_NONE;
            }
            if (current_child->next_sibling_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
                /* Got to end of children without finding a child */
                break;
            } else {
                /* Keep iterating, load next child */
                current_child_ref = current_child->next_sibling_ref;
                current_child = re__compile_charclass_tree_get(char_comp, current_child_ref);
            }
        }
        /* Add the new child to the end of root's children */
        re__compile_charclass_tree_init(&new_child, byte_range);
        /* Link the new child and the last child to each other */
        current_child->next_sibling_ref = next_root_ref;
        new_child.prev_sibling_ref = current_child_ref;
        /* Update root's last child to point to the new child */
        root->last_child_ref = next_root_ref;
        /* Finally, add the new child to the end of the tree vector. */
        if ((err = re__compile_charclass_tree_vec_push(&char_comp->tree, new_child))) {
            return err;
        }
        *out_child_ref = next_root_ref;
    }
    return RE_ERROR_NONE;
}

/* Recursive function to add a rune range to the tree. */
/* This function takes some explaining. It operates on UTF-8 codepoints, which
 * have the following forms:
 * 
 * Codepoint Ranges    Bits                     Encoded Bytes
 * [U+000000-U+00007F] 00000000000000yyyyyyy -> 0yyyyyyy (ASCII)
 * [U+000080-U+0007FF] 0000000000yyyyyxxxxxx -> 110yyyyy 10xxxxxx
 * [U+000800-U+00FFFF] 00000yyyyxxxxxxxxxxxx -> 1110yyyy 10xxxxxx 10xxxxxx
 * [U+010000-U+10FFFF] yyyxxxxxxxxxxxxxxxxxx -> 11110yyy 10xxxxxx 10xxxxxx 10xxxxxx
 * 
 * We notice that each 'class' of codepoints has a unique number of bits labeled
 * 'y' and a unique number of bits labeled 'x':
 * 
 *                     y-bits  x-bits
 * [U+000000-U+00007F]    7       0
 * [U+000080-U+0007FF]    5       6
 * [U+000800-U+00FFFF]    4      12
 * [U+010000-U+10FFFF]    3      18
 * 
 * Furthermore, for each length of continuation bytes, we can classify them as
 * runs of y-bits and x-bits:
 * 
 * Bits                  Encoded Bytes              y-bits  x-bits 
 * 000000000000yyyyyy -> 10yyyyyy                      6       0
 * 000000yyyyyyxxxxxx -> 10yyyyyy 10xxxxxx             6       6
 * yyyyyyxxxxxxxxxxxx -> 10yyyyyy 10xxxxxx 10xxxxxx    6      12
 * 
 * We now have seven 'classes' of byte sequences with unique numbers of 'y'
 * and 'x' bits.
 * However, the neat thing is that each class, when its y-bits are set to 0,
 * corresponds to another, smaller class or nothing if it only represents a
 * 1-byte sequence.
 * 
 *                     y-bits  x-bits   Next Class  Notes
 * [U+000000-U+00007F]    7       0       <none>    [one-byte seq.]
 * [U+000080-U+0007FF]    5       6    1-byte cont. [beginning of two-byte seq.]
 * [U+000800-U+00FFFF]    4      12    2-byte cont. [beginning of three-byte seq.]
 * [U+010000-U+10FFFF]    3      18    3-byte cont. [beginning of four-byte seq.]
 * 1-byte continuation    6       0       <none>    [one continuation byte]
 * 2-byte continuation    6       6    1-byte cont. [two continuation bytes]
 * 3-byte continuation    6      12    2-byte cont. [three continuation bytes]
 * 
 * We can transform this into a kind of state machine where each state
 * corresponds to one of these classes, and the transitions between each state
 * correspond to output bytes.
 * For example, take the character U+546. Since it is in the range [U+80-U+7FF],
 * we start in that state, with y-bits=5 and x-bits=6. Then, we output the first
 * byte in a two-byte unicode sequence (or with 11011111), and switch to the
 * 1-byte class/state.
 * 
 * State         Codepoint Bits        Y Bits X Bits Output Byte
 * 2-byte begin. 000000000010101000110  10101 000110    11010101
 * 1-byte cont.  000000000000000000110 000110    N/A    10000110
 * 
 * The nice thing about y-bits and x-bits is that it's really easy to go from
 * state to state: you're finished if the number of x-bits is 0, and if not, 
 * set the number of y-bits to 6 and subtract 6 from the number of x-bits.
 * 
 * The outputted bytes are 11010101 10000110, the UTF-8 for U+546.
 * 
 * Another example, U+10F6F7:
 * State         Codepoint Bits        Y Bits             X Bits Output Byte
 * 4-byte begin. 100001111011011110111    100 001111011011110111    11110100
 * 3-byte cont.  000001111011011110111 001111       011011110111    10001111
 * 2-byte cont.  000000000011011110111 011011             110111    10011011
 * 1-byte cont.  000000000000000110111 110111                N/A    10110111
 *
 * This generalizes to ranges of bytes, only the output is the tree structure
 * instead of a sequence of bytes.
 * Every time we change state, we "fork" the range (split it into smaller
 * ranges with the same leading byte) and generate trees for each of those
 * ranges. Here's an example with a relatively simple range, [U+80-U+E5].
 * 
 * The encoding sequences (UTF-8 bytes) for this range are, in hex:
 * [C2-C2][80-BF]
 * [C3-C3][80-A5]
 * 
 * We start by looking at the Y-bits of each range bound.
 *      Codepoint Bits Y Bits Encoded First Byte
 * U+80       10000000  00010    11000010 (0xC2)
 * U+E5       11000011  00011    11000011 (0xC3)
 * 
 * Since these differ in terms of their encoded first byte, we will need two
 * tree nodes, one for U+80 and one for U+E5, by encoding them as 0xC2 and 0xC3.
 * The tree of output byte ranges now looks like this:
 * 
 * ---[C2-C2]
 *       |
 *    [C3-C3]
 *
 * Each of these nodes has a sub-node that corresponds to the next encoded byte,
 * a.k.a. the x-bits of 0x80 and 0xE5.
 * We generate these sub-nodes by splitting up [U+80-U+E5] into two ranges per
 * their first encoded byte. In this case, the ranges are [U+80-U+BF] and
 * [U+C0-U+E5].
 * We reduce these sub-ranges by taking off their y-bits and masking them with
 * a Unicode continuation byte mask.
 *         Range, Original                Range, No Y              Range, Masked
 * [U+80-U+BF] 10000000 10111111 -> [00-3F] 000000 111111 -> [80-BF] 10000000 10111111
 * [U+C0-U+E5] 11000000 11100101 -> [80-A5] 000000 100101 -> [80-A5] 10000000 10100101
 * 
 * These final masked ranges are then inserted into the tree:
 * 
 * ---[C2-C2]---[80-BF]
 *       |
 *    [C3-C3]---[80-A5]
 * 
 * This corresponds to our original list of valid UTF-8 sequences for [U+80-U+E5]:
 * [C2-C2][80-BF]
 * [C3-C3][80-A5]
 * 
 * For a more complex example, the entire UTF-8 range [U+0-U+10FFFF], it is
 * necessary to "split" the range into sub-ranges that share common sequence
 * lengths. In particular, we need to turn [U+0-U+10FFFF] into:
 * 
 * [U+00-U+7F]
 * [U+80-U+7FF]
 * [U+800-U+FFFF]
 * [U+10000-U+10FFFF]
 * 
 * which is simple enough, and then these ranges into:
 * 
 * [00-7F]
 * [C2-DF][80-BF]
 * [E0-E0][A0-BF][80-BF]
 * [E1-EF][80-BF][80-BF]
 * [F0-F0][90-BF][80-BF][80-BF]
 * [F1-F3][80-BF][80-BF][80-BF]
 * [F4-F4][80-8F][80-BF][80-BF]
 * 
 * Finally, they are compiled into a tree:
 * 
 * ---[00-7F]
 *       |
 *    [C2-DF]---[80-BF]
 *       |
 *    [E0-E0]---[A0-BF]---[80-BF]
 *       |
 *    [E1-EF]---[80-BF]---[80-BF]
 *       |
 *    [F0-F0]---[90-BF]---[80-BF]---[80-BF]
 *       |
 *    [F1-F3]---[80-BF]---[80-BF]---[80-BF]
 *       |
 *    [F4-F4]---[80-8F]---[80-BF]---[80-BF]
 * 
 * The real magic, however, of the tree representation lies in how it is hashed
 * and linked to turn into a minimal DFA, explained later in the code. 
 */
re_error re__compile_charclass_add_rune_range(re__compile_charclass* char_comp, re_int32 root_ref, re__rune_range rune_range, re_int32 num_x_bits, re_int32 num_y_bits) {
    /* Mask of all ones for x-bits */
    re_rune x_mask = (1 << num_x_bits) - 1;
    /* 0b11111111 */
    re_rune byte_mask = 0xFF;
    /* Unicode top bits -- depending on y-bits, derive a mask to be used in
     * order to encode a byte.
     *
     * ASCII (no mask): u_mask = 0b00000000
     * 2-byte         : u_mask = 0b11000000
     * 3-byte         : u_mask = 0b11100000
     * 4-byte         : u_mask = 0b11110000
     * all cont. bytes: u_mask = 0b10000000 
     * 
     * This is pretty easy: just shift 0xFE by y_bits */
    /* 0xFE == 0b11111110 */
    re_rune u_mask = (0xFE << num_y_bits) & byte_mask;
    /* Reference to last created child */
    re_int32 child_ref = RE__COMPILE_CHARCLASS_TREE_NONE;
    /* Minimum/maximum bound's y-bits */
    re_rune y_min = rune_range.min >> num_x_bits;
    re_rune y_max = rune_range.max >> num_x_bits;
    /* Minimum/maximum upper encoded bytes */
    re_uint8 byte_min = (re_uint8)((y_min & byte_mask) | u_mask);
    re_uint8 byte_max = (re_uint8)((y_max & byte_mask) | u_mask);
    re_error err = RE_ERROR_NONE;
    /* num_y_bits can never be less than 7, as 7 is ASCII */
    RE_ASSERT(num_y_bits <= 7);
    if (num_x_bits == 0) {
        /* if x_bits is 0, then add a terminal node of just the byte range */
        /* This corresponds to the final byte in a UTF-8 sequence */
        /* Output:
         * ---[Ymin-Ymax] */
        re__byte_range br;
        br.min = byte_min;
        br.max = byte_max;
        if ((err = re__compile_charclass_touch_child(char_comp, root_ref, br, &child_ref))) {
            return err;
        }
    } else {
        /* Minimum/maximum bound's x_bits */
        re_rune x_min = rune_range.min & x_mask;
        re_rune x_max = rune_range.max & x_mask;
        /* Generate byte ranges (for this frame) and rune ranges (for child) */
        re__byte_range brs[3];
        re__rune_range rrs[3];
        /* Number of sub_trees (extents of 'brs' and 'rrs') */
        int num_sub_trees = 0;
        /* The next y_bits and x_bits for sub-ranges */
        re_int32 next_num_x_bits = num_x_bits - 6;
        re_int32 next_num_y_bits = 6;
        if (y_min == y_max || (x_min == 0 && x_max == x_mask)) {
            /* Range can be split into either a single byte followed by a range,
             * _or_ one range followed by another maximal range */
            /* Output:
             * ---[Ymin-Ymax]---{tree for [Xmin-Xmax]} */
            brs[0].min = byte_min, brs[0].max = byte_max;
            rrs[0].min = x_min,    rrs[0].max = x_max;
            num_sub_trees = 1;
        } else if (x_min == 0) {
            /* Range begins on zero, but has multiple starting bytes */
            /* Output:
             * ---[Ymin-(Ymax-1)]---{tree for [00-FF]}
             *           |       
             *      [Ymax-Ymax]----{tree for [00-Xmax]} */
            brs[0].min = byte_min, brs[0].max = byte_max - 1;
            rrs[0].min = 0,        rrs[0].max = x_mask;
            brs[1].min = byte_max, brs[1].max = byte_max;
            rrs[1].min = 0,        rrs[1].max = x_max;
            num_sub_trees = 2;
        } else if (x_max == x_mask) {
            /* Range ends on all ones, but has multiple starting bytes */
            /* Output:
             * -----[Ymin-Ymin]----{tree for [Xmin-FF]}
             *           |       
             *    [(Ymin+1)-Ymax]---{tree for [00-FF]} */
            brs[0].min = byte_min,     brs[0].max = byte_min;
            rrs[0].min = x_min,        rrs[0].max = x_mask;
            brs[1].min = byte_min + 1, brs[1].max = byte_max;
            rrs[1].min = 0,            rrs[1].max = x_mask;
            num_sub_trees = 2;
        } else {
            /* Range doesn't begin on all zeroes or all ones, and takes up more
             * than 2 different starting bytes */
            /* Output:
             * -------[Ymin-Ymin]-------{tree for [Xmin-FF]}
             *             |
             *    [(Ymin+1)-(Ymax-1)]----{tree for [00-FF]}
             *             |
             *        [Ymax-Ymax]-------{tree for [00-Xmax]} */
            brs[0].min = byte_min,     brs[0].max = byte_min;
            rrs[0].min = x_min,        rrs[0].max = x_mask;
            brs[1].min = byte_min + 1, brs[1].max = byte_max - 1;
            rrs[1].min = 0,            rrs[1].max = x_mask;
            brs[2].min = byte_max,     brs[2].max = byte_max;
            rrs[2].min = 0,            rrs[2].max = x_max;
            num_sub_trees = 3;
        }
        {
            /* Create all sub trees */
            int i;
            for (i = 0; i < num_sub_trees; i++) {
                if ((err = re__compile_charclass_touch_child(char_comp, root_ref, brs[i], &child_ref))) {
                    return err;
                }
                if ((err = re__compile_charclass_add_rune_range(char_comp, child_ref, rrs[i], next_num_x_bits, next_num_y_bits))) {
                    return err;
                } 
            }
        }
    }
    return err;
}

/* Check if two trees are equal, recursively. */
int re__compile_charclass_tree_equals(
    re__compile_charclass* char_comp,
    re__compile_charclass_tree* a,
    re__compile_charclass_tree* b) {
    re_int32 a_child_ref = a->first_child_ref;
    re_int32 b_child_ref = b->first_child_ref;
    /* While child references aren't none, check their equality. */
    while (a_child_ref != RE__COMPILE_CHARCLASS_TREE_NONE && b_child_ref != RE__COMPILE_CHARCLASS_TREE_NONE) {
        re__compile_charclass_tree* a_child = re__compile_charclass_tree_get(char_comp, a_child_ref);
        re__compile_charclass_tree* b_child = re__compile_charclass_tree_get(char_comp, a_child_ref);
        if (!re__compile_charclass_tree_equals(char_comp, a_child, b_child)) {
            return 0;
        }
        /* Go to next child refs. */
        a_child_ref = a_child->next_sibling_ref;
        b_child_ref = b_child->next_sibling_ref;
    }
    /* Child refs must become NONE at the same time. */
    if (a_child_ref != b_child_ref) { /* (a,b) != RE__COMPILE_CHARCLASS_TREE_NONE */
        return 0;
    }
    return re__byte_range_equals(a->byte_range, b->byte_range);
}

/* Merge two trees. */
void re__compile_charclass_merge_one(re__compile_charclass_tree* root, re__compile_charclass_tree* sibling) {
    root->next_sibling_ref = sibling->next_sibling_ref;
    root->byte_range = re__byte_range_merge(root->byte_range, sibling->byte_range);
}

/* Temporary structure that will be hashed, byte for byte. */
typedef struct re__compile_charclass_hash_temp {
    re__byte_range byte_range;
    re_uint32 down_hash;
    re_uint32 next_hash;
} re__compile_charclass_hash_temp;

/* Hash and merge everything within the tree, in place. */
/* This step is crucial to computing a minimized DFA.
 * In this function, we merge and then compute the hash of each node and its
 * children, in that order. The hashing is useful because the hashes of each
 * tree will be used later in memoizing the program locations for each tree, and
 * in addition adjacent trees will only be compared for merging if their hashes
 * are equal.
 * In a nutshell, the merging takes a tree that looks like this:
 * 
 * [55-66]---[77-88]
 *    |         |
 *    |      [99-AA]---[BB-CC]
 *    |
 * [67-69]---[77-88]
 *              |
 *           [99-AA]---[BB-CC]
 * 
 * And "merges" it into an equivalent tree that looks like this:
 * 
 * [55-69]---[77-88]
 *              |
 *           [99-AA]---[BB-CC]
 * 
 * This step ensures that we produce a minimal instruction encoding. */   
void re__compile_charclass_hash_tree(re__compile_charclass* char_comp, re__compile_charclass_tree* root) {
    /* Iterate through children backwards */
    re_int32 child_ref = root->last_child_ref;
    /* Child currently in process of being hashed */
    re__compile_charclass_tree* child;
    /* 1. Hash all children */
    while (child_ref != RE__COMPILE_CHARCLASS_TREE_NONE) {
        /* While there are children left, hash and merge them all */
        child = re__compile_charclass_tree_get(char_comp, child_ref);
        re__compile_charclass_hash_tree(char_comp, child);
        /* The previous call to hash_tree could have merged the last child into
         * its parent. Update the root's last_child_ref if the child is now the
         * last child. */
        if (child->next_sibling_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
            root->last_child_ref = child_ref;
        } 
        /* Iterate backwards */
        child_ref = child->prev_sibling_ref;
    }
    /* 2. Attempt to merge this root with its sibling */
    if (root->next_sibling_ref != RE__COMPILE_CHARCLASS_TREE_NONE) {
        re__compile_charclass_tree* sibling = re__compile_charclass_tree_get(char_comp, root->next_sibling_ref);
        if (re__byte_range_adjacent(root->byte_range, sibling->byte_range)) {
            /* root and sibling's byte ranges could potentially be merged */
            if (sibling->first_child_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
                if (root->first_child_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
                    /* Siblings are both terminals and can be merged */
                    re__compile_charclass_merge_one(root, sibling);
                }
            } else {
                if (root->first_child_ref != RE__COMPILE_CHARCLASS_TREE_NONE) {
                    re__compile_charclass_tree* root_child = re__compile_charclass_tree_get(char_comp, root->first_child_ref);
                    re__compile_charclass_tree* root_sibling_child = re__compile_charclass_tree_get(char_comp, sibling->first_child_ref);
                    if (root_child->hash == root_sibling_child->hash) {
                        if (re__compile_charclass_tree_equals(char_comp, root_child, root_sibling_child)) {
                            /* Siblings have identical children and can be merged */
                            re__compile_charclass_merge_one(root, sibling);
                        }
                    }
                }
            }
        }
    }
    /* 3. Compute hash of this root. */
    {
        /* The hash of this root is just the hash of the hashes of the first
         * child and the next sibling, as well as the hash of the byte range. */
        /* "Chaining" hashes in this way allows trees to be unique based on
         * their subsequent children and siblings, but also allows the trees of
         * children and siblings to be referenced by a unique hash themselves. */
        re__compile_charclass_hash_temp hash_obj;
        /* C89 does not guarantee struct zero-padding. This will throw off our
         * hash function if uninitialized properly. We explicitly zero out the
         * memory for this reason. */
        re__zero_mem(sizeof(re__compile_charclass_hash_temp), &hash_obj);
        hash_obj.byte_range = root->byte_range;
        /* Register hashes for next sibling and first child. */
        if (root->next_sibling_ref== RE__COMPILE_CHARCLASS_TREE_NONE) {
            /* I know nothing about cryptography. Whether or not this is an
             * actually good value is unknown. */
            hash_obj.next_hash = 0x0F0F0F0F;
        } else {
            re__compile_charclass_tree* next_sibling = re__compile_charclass_tree_get(char_comp, root->next_sibling_ref);
            hash_obj.next_hash = next_sibling->hash;
        }
        if (root->first_child_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
            hash_obj.down_hash = 0x0F0F0F0F;
        } else {
            re__compile_charclass_tree* first_child = re__compile_charclass_tree_get(char_comp, root->first_child_ref);
            hash_obj.down_hash = first_child->hash;
        }
        /* Murmurhash seemed good... */
        root->hash = re__murmurhash3_32((const re_uint8*)&hash_obj, sizeof(hash_obj));
    }
}

/* Split top-level rune ranges into ranges with unique UTF-8 byte lengths. */
/* This function takes a range like this:
 * [U+0-U+10FFFF]
 *
 * and produces trees for these ranges:
 * [U+00-U+7F]        -- ascii, 1 byte,  x_bits =  0, y_bits = 7
 * [U+80-U+7FF]       -- utf-8, 2 bytes, x_bits =  6, y_bits = 5
 * [U+800-U+FFFF]     -- utf-8, 3 bytes, x_bits = 12, y_bits = 4
 * [U+10000-U+10FFFF] -- utf-8, 4 bytes, x_bits = 18, y_bits = 3 */
re_error re__compile_charclass_split_rune_range(re__compile_charclass* char_comp, re__rune_range range) {
    re_error err = RE_ERROR_NONE;
    /* Starting values for y_bits and x_bits per byte length */
    static const re_int32 y_bits[4] = {7, 5, 4, 3};
    static const re_int32 x_bits[4] = {0, 6, 12, 18};
    /* Current byte length being processed */
    re_int32 byte_length;
    /* Minimum value to clamp for this byte length */
    re_rune min_value = 0;
    for (byte_length = 0; byte_length < 4; byte_length++) {
        /* Compute maximum value for this byte length, always equal to the
         * maximum codepoint that can be represented by byte_length bytes */
        re_rune max_value = (1 << (y_bits[byte_length] + x_bits[byte_length])) - 1;
        /* Clamp the current rune range to the given bounds */
        re__rune_range bounds;
        bounds.min = min_value;
        bounds.max = max_value;
        if (re__rune_range_intersects(range, bounds)) {
            /* If the current rune range contains a region within bounds, 
             * clamp it to bounds and then put it into the tree */
            re__rune_range clamped = re__rune_range_clamp(range, bounds);
            if ((err = re__compile_charclass_add_rune_range(char_comp, 0, clamped, x_bits[byte_length], y_bits[byte_length]))) {
                return err;
            }
        }
        /* Next byte length has a minimum value of the previous max + 1 */
        min_value = max_value + 1;
    }
    return err;
}


RE_VEC_IMPL_FUNC(re__compile_charclass_hash_entry, init)
RE_VEC_IMPL_FUNC(re__compile_charclass_hash_entry, destroy)
RE_VEC_IMPL_FUNC(re__compile_charclass_hash_entry, size)
RE_VEC_IMPL_FUNC(re__compile_charclass_hash_entry, getref)
RE_VEC_IMPL_FUNC(re__compile_charclass_hash_entry, push)
RE_VEC_IMPL_FUNC(re__compile_charclass_hash_entry, clear)

void re__compile_charclass_hash_entry_init(re__compile_charclass_hash_entry* hash_entry, re_int32 sparse_index, re_int32 root_ref, re__prog_loc prog_loc) {
    hash_entry->sparse_index = sparse_index;
    hash_entry->root_ref = root_ref;
    hash_entry->prog_loc = prog_loc;
    hash_entry->next = RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE;
}

/* Clear the program location cache. */
void re__compile_charclass_cache_clear(re__compile_charclass* char_comp) {
    /* We don't need to clear cache_sparse, because it can be used while
     * undefined. */
    re__compile_charclass_hash_entry_vec_clear(&char_comp->cache_dense);
}

/* Get a program location from the cache, if its given tree is in the cache. */
/* Returns RE__PROG_LOC_INVALID if not in the cache. */
re__prog_loc re__compile_charclass_cache_get(re__compile_charclass* char_comp, re_int32 root_ref) {
    re__compile_charclass_tree* root = re__compile_charclass_tree_get(char_comp, root_ref);
    /* Index into the sparse array based off of root's hash */
    re_int32 sparse_index = root->hash % RE__COMPILE_CHARCLASS_CACHE_SPARSE_SIZE;
    /* Final index in the dense array, if the tree is found in the cache */
    re_int32 dense_index = RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE;
    if (char_comp->cache_sparse == RE_NULL) {
        /* Cache is empty a.t.m., just return */
        return RE__PROG_LOC_INVALID;
    }
    dense_index = char_comp->cache_sparse[sparse_index];
    /* If dense_index_initial is more than the dense size, sparse_index is
     * nonsensical and the item isn't in the cache. */
    if (dense_index < (re_int32)re__compile_charclass_hash_entry_vec_size(&char_comp->cache_dense)) {
        /* Load the hash entry at dense_index_initial */
        re__compile_charclass_hash_entry* hash_entry_prev = re__compile_charclass_hash_entry_vec_getref(&char_comp->cache_dense, (re_size)dense_index);
        /* If hash_entry_prev->sparse_index doesn't point back to sparse_index,
         * sparse_index is nonsensical. */
        if (hash_entry_prev->sparse_index == sparse_index) {
            while (1) {
                re__compile_charclass_tree* root_cache = re__compile_charclass_tree_vec_getref(&char_comp->tree, (re_size)hash_entry_prev->root_ref);
                if (root_cache->hash == root->hash) {
                    if (re__compile_charclass_tree_equals(char_comp, root_cache, root)) {
                        /* If both hashes and then their trees are equal, we
                         * have already compiled this tree and can return its
                         * program location. */
                        return hash_entry_prev->prog_loc;
                    }
                }
                if (hash_entry_prev->next == RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE) {
                    /* We've exhausted this hash bucket, and didn't find a
                     * matching tree. */
                    break;
                } else {
                    /* There are still more entries in this hash bucket to
                     * check. */
                    dense_index = hash_entry_prev->next;
                    hash_entry_prev = re__compile_charclass_hash_entry_vec_getref(&char_comp->cache_dense, (re_size)dense_index);
                }
            }
            return RE__PROG_LOC_INVALID;
        } else {
            return RE__PROG_LOC_INVALID;
        }
    } else {
        return RE__PROG_LOC_INVALID;
    }
}
/* Add a program location to the cache, after it has been compiled. */
re_error re__compile_charclass_cache_add(re__compile_charclass* char_comp, re_int32 root_ref, re__prog_loc prog_loc) {
    re_error err = RE_ERROR_NONE;
    re__compile_charclass_tree* root = re__compile_charclass_tree_get(char_comp, root_ref);
    /* These variables have the same meaning as they do in cache_get. */
    re_int32 sparse_index = root->hash % RE__COMPILE_CHARCLASS_CACHE_SPARSE_SIZE;
    re_int32 dense_index = RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE;
    re__compile_charclass_hash_entry* hash_entry_prev = RE_NULL;
    int requires_link;
    if (char_comp->cache_sparse == RE_NULL) {
        /* Sparse cache is empty, so let's allocate it on-demand. */
        char_comp->cache_sparse = (re_int32*)RE_MALLOC(sizeof(re_int32) * RE__COMPILE_CHARCLASS_CACHE_SPARSE_SIZE);
        if (char_comp->cache_sparse == RE_NULL) {
            return RE_ERROR_NOMEM;
        }
    }
    dense_index = char_comp->cache_sparse[sparse_index];
    /* Look up the element in the cache, see re__compile_charclass_cache_get */
    if (dense_index < (re_int32)re__compile_charclass_hash_entry_vec_size(&char_comp->cache_dense)) {
        hash_entry_prev = re__compile_charclass_hash_entry_vec_getref(&char_comp->cache_dense, (re_size)dense_index);
        if (hash_entry_prev->sparse_index == sparse_index) {
            while (1) {
                re__compile_charclass_tree* root_cache = re__compile_charclass_tree_get(char_comp, hash_entry_prev->root_ref);
                if (root_cache->hash == root->hash) {
                    if (re__compile_charclass_tree_equals(char_comp, root_cache, root)) {
                        /* We found the item in the cache? This should never
                         * happen. To ensure optimality, we should only ever add
                         * items to the cache once. */
                        requires_link = 0;
                        RE__ASSERT_UNREACHED();
                        break;
                    }
                }
                if (hash_entry_prev->next == RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE) {
                    requires_link = 1;
                    dense_index = RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE;
                    break;
                }
                /* Keep iterating. */
                dense_index = hash_entry_prev->next;
                hash_entry_prev = re__compile_charclass_hash_entry_vec_getref(&char_comp->cache_dense, (re_size)dense_index);
            }
        } else {
            requires_link = 0;
            dense_index = RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE;
        }
    } else {
        requires_link = 0;
        dense_index = RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE;
    }
    if (dense_index == RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE) {
        /* Item isn't in cache. */
        /* The location for the new hash_entry. */
        re_int32 dense_index_final = (re_int32)re__compile_charclass_hash_entry_vec_size(&char_comp->cache_dense);
        re__compile_charclass_hash_entry new_entry;
        re__compile_charclass_hash_entry_init(&new_entry, sparse_index, root_ref, prog_loc);
        if (!requires_link) {
            /* No linking required, insert a new sparse entry */
            char_comp->cache_sparse[sparse_index] = dense_index_final;
            if ((err = re__compile_charclass_hash_entry_vec_push(&char_comp->cache_dense, new_entry))) {
                return err;
            }
            /* hash_entry_prev is invalid */
        } else {
            /* Linking required */
            hash_entry_prev->next = dense_index_final;
            if ((err = re__compile_charclass_hash_entry_vec_push(&char_comp->cache_dense, new_entry))) {
                return err;
            }
            /* hash_entry_prev is invalid */
        }
    } else {
        /* in cache already */
        RE__ASSERT_UNREACHED();
    }
    return err;
}

/* Generate the program for a particular tree. */
/* root should not be in the cache. */
re_error re__compile_charclass_generate_prog(re__compile_charclass* char_comp, re__prog* prog, re__compile_charclass_tree* root, re__prog_loc* out_pc, re__compile_patches* patches) {
    re_int32 child_ref = root->first_child_ref;
    /* Starting program location for this root. */
    re__prog_loc start_pc = re__prog_size(prog);
    /* Keeps track of the previous split location, if there is one. */
    re__prog_loc split_from = RE__PROG_LOC_INVALID;
    re_error err = RE_ERROR_NONE;
    while (child_ref != RE__COMPILE_CHARCLASS_TREE_NONE) {
        re__compile_charclass_tree* child;
        /* Program counter in the cache, if it was found */
        re__prog_loc cache_pc;
        /* Previous compiled instruction, gets linked to the next one */
        re__prog_inst* inst_from;
        /* New instruction template */
        re__prog_inst new_inst;
        /* Location in the program of inst_from */
        re__prog_loc link_from;
        /* Location to link inst_from to */
        re__prog_loc link_to;
        /* Current byte range that is getting compiled */
        re__byte_range byte_range;
        if ((cache_pc = re__compile_charclass_cache_get(char_comp, child_ref)) != RE__PROG_LOC_INVALID) {
            /* Tree is in cache! */
            /* if split_from is invalid, that means that root is in the cache.
             * This shouldn't be possible. */
            RE_ASSERT(split_from != RE__PROG_LOC_INVALID);
            inst_from = re__prog_get(prog, split_from);
            re__prog_inst_set_split_secondary(inst_from, cache_pc);
            break;
        }
        /* Tree is not in cache */
        child = re__compile_charclass_tree_get(char_comp, child_ref);
        if (split_from != RE__PROG_LOC_INVALID) {
            /* If we previously compiled a split instruction, link it to the
             * next compiled instruction. */
            inst_from = re__prog_get(prog, split_from);
            re__prog_inst_set_split_secondary(inst_from, re__prog_size(prog));
        }
        if (child->next_sibling_ref != RE__COMPILE_CHARCLASS_TREE_NONE) {
            /* If there is another sibling that will get compiled, add a SPLIT
             * instruction before the child. This split instruction will get
             * linked to the next child (see above lines) */
            split_from = re__prog_size(prog);
            re__prog_inst_init_split(&new_inst, re__prog_size(prog) + 1, RE__PROG_LOC_INVALID);
            if ((err = re__prog_add(prog, new_inst))) {
                return err;
            }
        }
        /* Location of byte range/byte instruction */
        link_from = re__prog_size(prog);
        byte_range = child->byte_range;
        /* Compile either a byterange or byte instruction depending on range */
        if (byte_range.min == byte_range.max) {
            re__prog_inst_init_byte(&new_inst, byte_range.min);
        } else {
            re__prog_inst_init_byte_range(&new_inst, byte_range.min, byte_range.max);
        }
        if ((err = re__prog_add(prog, new_inst))) {
            return err;
        }
        if (child->first_child_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
            /* Child is a terminal node. Add an outgoing patch. */
            re__compile_patches_append(patches, prog, link_from, 0);
        } else {
            /* Check if child's children are in the cache. */
            /* link_to becomes the cached program location if child's children
             * are in the cache, otherwise it is RE__PROG_LOC_INVALID. */
            if ((link_to = re__compile_charclass_cache_get(char_comp, child->first_child_ref)) == RE__PROG_LOC_INVALID) {
                /* Child's children are not in the cache. Generate them. */
                if ((err = re__compile_charclass_generate_prog(char_comp, prog, child, &link_to, patches))) {
                    return err;
                }
            }
            /* link_to always points to cached pc or next pc */
            inst_from = re__prog_get(prog, link_from);
            re__prog_inst_set_primary(inst_from, link_to);
        }
        /* Register child in the cache. */
        if ((err = re__compile_charclass_cache_add(char_comp, child_ref, link_from))) {
            return err;
        }
        child_ref = child->next_sibling_ref;
    }
    *out_pc = start_pc;
    return RE_ERROR_NONE;
}

/* Compile a single character class. */
re_error re__compile_charclass_gen(re__compile_charclass* char_comp, re__charclass* charclass, re__prog* prog, re__compile_patches* patches_out) {
    re_error err = RE_ERROR_NONE;
    re_size i;
    const re__rune_range* ranges = re__charclass_get_ranges(charclass);
    re__compile_charclass_tree initial_root;
    re__byte_range initial_range;
    initial_range.min = 0;
    initial_range.max = 0xFF;
    /* These are all idempotent. Cool word, right? I just learned it */
    re__compile_charclass_tree_vec_clear(&char_comp->tree);
    re__compile_charclass_cache_clear(char_comp);
    /* Add the initial root (0-FF node) */
    re__compile_charclass_tree_init(&initial_root, initial_range);
    if ((err = re__compile_charclass_tree_vec_push(&char_comp->tree, initial_root))) {
        return err;
    }
    /* Iterate through charclass' ranges and add them all to the tree. */
    for (i = 0; i < re__charclass_get_num_ranges(charclass); i++) {
        re__rune_range r = ranges[i];
        if ((err = re__compile_charclass_split_rune_range(char_comp, r))) {
            return err;
        }
    }
    {
        /* Do the actual compiling. */
        re__prog_loc out_pc;
        re__compile_charclass_tree* initial_tree = re__compile_charclass_tree_vec_getref(&char_comp->tree, 0);
        /* Hash and merge the tree */
        re__compile_charclass_hash_tree(char_comp, initial_tree);
        /* Generate the tree's program */
        if ((err = re__compile_charclass_generate_prog(char_comp, prog, initial_tree, &out_pc, patches_out))) {
            return err;
        }
        /* Done!!! all that effort for just a few instructions. */
    }
    return err;
}

void re__compile_charclass_init(re__compile_charclass* char_comp) {
    re__compile_charclass_tree_vec_init(&char_comp->tree);
    char_comp->cache_sparse = RE_NULL;
    re__compile_charclass_hash_entry_vec_init(&char_comp->cache_dense);
}

void re__compile_charclass_destroy(re__compile_charclass* char_comp) {
    re__compile_charclass_hash_entry_vec_destroy(&char_comp->cache_dense);
    if (char_comp->cache_sparse != RE_NULL) {
        RE_FREE(char_comp->cache_sparse);
    }
    re__compile_charclass_tree_vec_destroy(&char_comp->tree);
}

#if RE_DEBUG

void re__compile_charclass_dump(re__compile_charclass* char_comp, re_int32 tree_idx, re_int32 indent) {
    re_int32 i;
    re_int32 node = tree_idx;
    if (indent == 0) {
        printf("Charclass Compiler %p:\n", (void*)char_comp);
    }
    if (node == RE__COMPILE_CHARCLASS_TREE_NONE) {
        for (i = 0; i < indent + 1; i++) {
            printf("  ");
        }
        printf("<term>\n");
    } else {
        while (node != RE__COMPILE_CHARCLASS_TREE_NONE) {
            re__compile_charclass_tree* tree = re__compile_charclass_tree_vec_getref(&char_comp->tree, (re_size)node);
            for (i = 0; i < indent + 1; i++) {
                printf("  ");
            }
            printf("%04X | [%02X-%02X] hash=%08X\n", node, tree->byte_range.min, tree->byte_range.max, tree->hash);
            re__compile_charclass_dump(char_comp, tree->first_child_ref, indent+1);
            node = tree->next_sibling_ref;
        }
    }
    if (indent == 0) {
        printf("  Cache:\n");
        if (char_comp->cache_sparse == RE_NULL) {
            printf("    Empty cache!\n");
        } else {
            for (i = 0; i < RE__COMPILE_CHARCLASS_CACHE_SPARSE_SIZE; i++) {
                re_int32 j;
                re_int32 dense_loc = char_comp->cache_sparse[i];
                re__compile_charclass_hash_entry* hash_entry;
                if (dense_loc >= (re_int32)re__compile_charclass_hash_entry_vec_size(&char_comp->cache_dense)) {
                    continue;
                }
                hash_entry = re__compile_charclass_hash_entry_vec_getref(&char_comp->cache_dense, (re_size)dense_loc);
                if (hash_entry->sparse_index != i) {
                    continue;
                }
                printf("    Sparse index: %i\n", i);
                j = 0;
                printf("      [%i] root_ref=%04X prog_loc=%04X\n", j, hash_entry->root_ref, hash_entry->prog_loc);
                j++;
                while (hash_entry->next != RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE) {
                    hash_entry = re__compile_charclass_hash_entry_vec_getref(&char_comp->cache_dense, (re_size)hash_entry->next);
                    printf("      [%i] root_ref=%04X prog_loc=%04X\n", j, hash_entry->root_ref, hash_entry->prog_loc);
                    j++;
                }
            }
        }
    }
}

#endif /* #if RE_DEBUG */


RE_INTERNAL void re__exec_thrdmin_set_init(re__exec_thrdmin_set* set) {
    set->size = 0;
    set->n = 0;
    set->dense = RE_NULL;
    set->sparse = RE_NULL;
}

RE_INTERNAL re_error re__exec_thrdmin_set_alloc(re__exec_thrdmin_set* set, re__prog_loc size) {
    re_error err = RE_ERROR_NONE;
    set->size = size;
    set->dense = (re__exec_thrdmin*)RE_MALLOC(sizeof(re__exec_thrdmin) * size);
    if (set->dense == RE_NULL) {
        return RE_ERROR_NOMEM;
    }
    set->sparse = (re__exec_thrdmin*)RE_MALLOC(sizeof(re__exec_thrdmin) * size);
    if (set->sparse == RE_NULL) {
        return RE_ERROR_NOMEM;
    }
    return err;
}

RE_INTERNAL void re__exec_thrdmin_set_free(re__exec_thrdmin_set* set) {
    if (set->dense) {
        RE_FREE(set->dense);
        set->dense = RE_NULL;
    }
    if (set->sparse) {
        RE_FREE(set->sparse);
        set->sparse = RE_NULL;
    }
}

RE_INTERNAL void re__exec_thrdmin_set_destroy(re__exec_thrdmin_set* set) {
    RE__UNUSED(set);
}

RE_INTERNAL void re__exec_thrdmin_set_add(re__exec_thrdmin_set* set, re__exec_thrdmin loc) {
    RE_ASSERT(loc < set->size);
    set->dense[set->n] = loc;
    set->sparse[loc] = set->n;
    set->n++;
}

RE_INTERNAL void re__exec_thrdmin_set_clear(re__exec_thrdmin_set* set) {
    set->n = 0;
}

RE_INTERNAL int re__exec_thrdmin_set_ismemb(re__exec_thrdmin_set* set, re__exec_thrdmin loc) {
    RE_ASSERT(loc < set->size);
    return set->sparse[loc] < set->n && 
           set->dense[set->sparse[loc]] == loc;
}

#if RE_DEBUG

RE_INTERNAL void re__exec_thrdmin_set_dump(re__exec_thrdmin_set* set) {
    printf("n: %u\n", set->n);
    printf("s: %u\n", set->size);
    printf("memb:\n");
    {
        re_uint32 i;
        for (i = 0; i < set->n; i++) {
            printf("  %04X\n", set->dense[i]);
        }
    }
}

#endif /* #if RE_DEBUG */

RE_VEC_IMPL_FUNC(re__exec_thrdmin, init)
RE_VEC_IMPL_FUNC(re__exec_thrdmin, destroy)
RE_VEC_IMPL_FUNC(re__exec_thrdmin, push)
RE_VEC_IMPL_FUNC(re__exec_thrdmin, pop)
RE_VEC_IMPL_FUNC(re__exec_thrdmin, clear)
RE_VEC_IMPL_FUNC(re__exec_thrdmin, size)

RE_INTERNAL void re__exec_init(re__exec* exec, re* re) {
    exec->re = re;
    re__exec_thrdmin_set_init(&exec->set_a);
    re__exec_thrdmin_set_init(&exec->set_b);
    re__exec_thrdmin_set_init(&exec->set_c);
    re__exec_thrdmin_vec_init(&exec->thrd_stk);
}

RE_INTERNAL void re__exec_destroy(re__exec* exec) {
    re__exec_thrdmin_vec_destroy(&exec->thrd_stk);
    re__exec_thrdmin_set_destroy(&exec->set_c);
    re__exec_thrdmin_set_destroy(&exec->set_b);
    re__exec_thrdmin_set_destroy(&exec->set_a);
}

RE_INTERNAL re_error re__exec_nfa_follow_threads(re__exec* exec, re__exec_thrdmin thrd, re__exec_thrdmin_set* target, re__ast_assert_type assert_context) {
    re__prog* prog = &exec->re->data->program;
    re_error err = RE_ERROR_NONE;
    re__exec_thrdmin_vec_clear(&exec->thrd_stk);
    re__exec_thrdmin_set_clear(&exec->set_c);
    if ((err = re__exec_thrdmin_vec_push(&exec->thrd_stk, thrd))) {
        return err;
    }
    while (re__exec_thrdmin_vec_size(&exec->thrd_stk)) {
        re__exec_thrdmin top = re__exec_thrdmin_vec_pop(&exec->thrd_stk);
        re__prog_inst* inst = re__prog_get(prog, top);
        re__prog_inst_type inst_type;
        RE_ASSERT(top != 0);
        if (re__exec_thrdmin_set_ismemb(&exec->set_c, top)) {
            /* target is already a member */
            continue;
        } else {
            re__exec_thrdmin_set_add(&exec->set_c, top);
        }
        inst_type = re__prog_inst_get_type(inst);
        if (inst_type == RE__PROG_INST_TYPE_BYTE) {
            re__exec_thrdmin_set_add(target, top);
        } else if (inst_type == RE__PROG_INST_TYPE_BYTE_RANGE) {
            re__exec_thrdmin_set_add(target, top);
        } else if (inst_type == RE__PROG_INST_TYPE_SPLIT) {
            /* push secondary first to maintain priority */
            re__prog_loc secondary = re__prog_inst_get_split_secondary(inst);
            re__prog_loc primary = re__prog_inst_get_primary(inst);
            if ((err = re__exec_thrdmin_vec_push(&exec->thrd_stk, secondary))) {
                return err;
            }
            if ((err = re__exec_thrdmin_vec_push(&exec->thrd_stk, primary))) {
                return err;
            }
        } else if (inst_type == RE__PROG_INST_TYPE_MATCH) {
            re__exec_thrdmin_set_add(target, top);
        } else if (inst_type == RE__PROG_INST_TYPE_ASSERT) {
            if (assert_context & re__prog_inst_get_assert_ctx(inst)) {
                if ((err = re__exec_thrdmin_vec_push(&exec->thrd_stk, re__prog_inst_get_primary(inst)))) {
                    return err;
                }
            }
        } else {
            RE__ASSERT_UNREACHED();
        }
    }
    return 0;
}

RE_INTERNAL void re__exec_swap(re__exec* exec) {
    re__exec_thrdmin_set temp = exec->set_a;
    exec->set_a = exec->set_b;
    exec->set_b = temp;
}

RE_INTERNAL re_error re__exec_nfa(re__exec* exec, const re_char* str, re_size n) {
    re_error err = RE_ERROR_NONE;
    re__prog* prog = &exec->re->data->program;
    re__prog_loc set_size = re__prog_size(prog);
    re_uint32 i;
    const re_char* str_ptr = str;
    const re_char* str_end = str + n;
    re__ast_assert_type assert_ctx;
    /* Initialize sparse sets. */
    if ((err = re__exec_thrdmin_set_alloc(&exec->set_a, set_size))) {
        goto error;
    }
    if ((err = re__exec_thrdmin_set_alloc(&exec->set_b, set_size))) {
        goto error;
    }
    if ((err = re__exec_thrdmin_set_alloc(&exec->set_c, set_size))) {
        goto error;
    }
    assert_ctx = RE__AST_ASSERT_TYPE_TEXT_START_ABSOLUTE | RE__AST_ASSERT_TYPE_TEXT_START;
    if ((err = re__exec_nfa_follow_threads(exec, 1, &exec->set_a, assert_ctx))) {
        goto error;
    }
    while (str_ptr != str_end) {
        if (exec->set_a.n == 0) {
            RE_ASSERT(0);
        }
        assert_ctx = 0;
        for (i = 0; i < exec->set_a.n; i++) {
            re__prog_inst* inst = re__prog_get(prog, exec->set_a.dense[i]);
            re__prog_inst_type inst_type = re__prog_inst_get_type(inst);
            if (inst_type == RE__PROG_INST_TYPE_BYTE) {
                if (*str_ptr == re__prog_inst_get_byte(inst)) {
                    if ((err = re__exec_nfa_follow_threads(exec, re__prog_inst_get_primary(inst), &exec->set_b, assert_ctx))) {
                        goto error;
                    }
                }
            } else if (inst_type == RE__PROG_INST_TYPE_BYTE_RANGE) {
                if (*str_ptr >= re__prog_inst_get_byte_min(inst) && *str_ptr <= re__prog_inst_get_byte_max(inst)) {
                    if ((err = re__exec_nfa_follow_threads(exec, re__prog_inst_get_primary(inst), &exec->set_b, assert_ctx))) {
                        goto error;
                    }
                }
            } else if (inst_type == RE__PROG_INST_TYPE_MATCH) {
                /* do nothing */
            }
        }
        re__exec_swap(exec);
        re__exec_thrdmin_set_clear(&exec->set_b);
        str_ptr++;
    }
    re__exec_thrdmin_set_dump(&exec->set_a);
    return err;
error:
    re__exec_thrdmin_set_free(&exec->set_c);
    re__exec_thrdmin_set_free(&exec->set_b);
    re__exec_thrdmin_set_free(&exec->set_a);
    re__set_error_generic(exec->re, err);
    return err;
}


RE_VEC_IMPL_FUNC(re__ast, init)
RE_VEC_IMPL_FUNC(re__ast, destroy)
RE_VEC_IMPL_FUNC(re__ast, get)
RE_VEC_IMPL_FUNC(re__ast, get_data)
RE_VEC_IMPL_FUNC(re__ast, getref)
RE_VEC_IMPL_FUNC(re__ast, insert)
RE_VEC_IMPL_FUNC(re__ast, peek)
RE_VEC_IMPL_FUNC(re__ast, pop)
RE_VEC_IMPL_FUNC(re__ast, push)
RE_VEC_IMPL_FUNC(re__ast, size)

RE_VEC_IMPL_FUNC(re__parse_frame, init)
RE_VEC_IMPL_FUNC(re__parse_frame, destroy)
RE_VEC_IMPL_FUNC(re__parse_frame, push)
RE_VEC_IMPL_FUNC(re__parse_frame, pop)
RE_VEC_IMPL_FUNC(re__parse_frame, peek)
RE_VEC_IMPL_FUNC(re__parse_frame, size)

RE_INTERNAL void re__parse_init(re__parse* parse, re* reg) {
    parse->re = reg;
    re__parse_frame_vec_init(&parse->frames);
    re__ast_root_init(&parse->ast_root);
    re__charclass_builder_init(&parse->charclass_builder);
    parse->ast_prev_child_ref = RE__AST_NONE;
    parse->ast_frame_root_ref = RE__AST_NONE;
    parse->state = RE__PARSE_STATE_GND;
    parse->radix_num = 0;
    parse->radix_digits = 0;
    parse->defer = 0;
    parse->group_flags = 0;
    parse->group_flags_new = 0;
    parse->str_begin = NULL;
    parse->str_end = NULL;
    parse->charclass_lo_rune = -1;
    parse->depth_max = 0;
    parse->depth_max_prev = 0;
    parse->depth = 0;
}

RE_INTERNAL void re__parse_destroy(re__parse* parse) {
    re__charclass_builder_destroy(&parse->charclass_builder);
    re__ast_root_destroy(&parse->ast_root);
    re__parse_frame_vec_destroy(&parse->frames);
}

/* Convenience function to set the error to a literal string. */
RE_INTERNAL re_error re__parse_error(re__parse* parse, const char* err_chars) {
    re__str err_str;
    re_error err = re__str_init_s(&err_str, (const re_char*)err_chars);
    if (err) {
        return err;
    }
    re__set_error_str(parse->re, &err_str);
    re__str_destroy(&err_str);
    return RE_ERROR_PARSE;
}

/* Formats a "invalid escape sequence" message */
RE_INTERNAL re_error re__parse_error_invalid_escape(re__parse* parse, re_rune esc) {
    /* Build error message */
    re_error err = RE_ERROR_NONE;
    re__str err_str;
    re_char esc_ch[2];
    esc_ch[0] = (re_char)esc;
    esc_ch[1] = '\'';
    if ((err = re__str_init_s(&err_str, (const re_char*)"invalid escape sequence '\\"))) {
        goto destroy_err_str;
    }
    if ((err = re__str_cat_n(&err_str, 2, esc_ch))) {
        goto destroy_err_str;
    }

    re__set_error_str(parse->re, &err_str);
destroy_err_str:
    re__str_destroy(&err_str);
    return RE_ERROR_PARSE;
}


RE_INTERNAL int re__parse_frame_is_empty(re__parse* parse) {
    return parse->ast_prev_child_ref == RE__AST_NONE;
}

RE_INTERNAL re__ast* re__parse_get_frame(re__parse* parse) {
    return re__ast_root_get(&parse->ast_root, parse->ast_frame_root_ref);
}

RE_INTERNAL re_error re__parse_push_node(re__parse* parse, re__ast ast, re_int32* new_ast_ref) {
    re_error err = RE_ERROR_NONE;
    int was_empty = re__parse_frame_is_empty(parse);
    if ((err = re__ast_root_add(&parse->ast_root, ast, new_ast_ref))) {
        return err;
    }
    if (was_empty) {
        re__ast_root_set_child(&parse->ast_root, parse->ast_frame_root_ref, *new_ast_ref);
    } else {
        re__ast_root_link_siblings(&parse->ast_root, parse->ast_prev_child_ref, *new_ast_ref);
    }
    if (!was_empty) {
        /* Empty frame: increment stk_ptr, leaving prev_child_ptr untouched */
        /* Since we just pushed the first node, prev_child_ptr should now
         * point to it. */
    } else {
        /* Non-empty frame: increment stk_ptr, and set prev_child_ptr to 
         * stk_ptr - 1, so that it points to the just-pushed node. */
        parse->ast_prev_child_ref = *new_ast_ref;
        parse->depth_max_prev = parse->depth;
    }
    return err;
}

/* Insert a node right before the previous child, making the previous child the
 * new node's parent. */
RE_INTERNAL re_error re__parse_link_wrap_node(re__parse* parse, re__ast outer, re_int32* new_outer) {
    re_error err = RE_ERROR_NONE;
    re_int32 new_ref;
    if ((err = re__ast_root_add(&parse->ast_root, outer, &new_ref))) {
        return err;
    }
    re__ast_root_wrap(&parse->ast_root, parse->ast_frame_root_ref, parse->ast_prev_child_ref, new_ref);
    *new_outer = new_ref;
    parse->depth_max_prev += 1;
    parse->depth_max = RE__MAX(parse->depth_max_prev, parse->depth_max);
    parse->ast_prev_child_ref = new_ref;
    return err;
}

RE_INTERNAL re_error re__parse_wrap_node(re__parse* parse, re__ast outer) {
    re_int32 dummy;
    return re__parse_link_wrap_node(parse, outer, &dummy);
}

RE_INTERNAL re_error re__parse_frame_push(re__parse* parse) {
    re__parse_frame op;
    op.ast_frame_root_ref = parse->ast_frame_root_ref;
    op.ast_prev_child_ref = parse->ast_prev_child_ref;
    op.ret_state = parse->state;
    op.group_flags = parse->group_flags;
    op.depth = parse->depth;
    op.depth_max = parse->depth_max;
    return re__parse_frame_vec_push(&parse->frames, op);
}

RE_INTERNAL void re__parse_frame_pop(re__parse* parse) {
    re__parse_frame op;
    RE_ASSERT(re__parse_frame_vec_size(&parse->frames) > 0);
    op = re__parse_frame_vec_pop(&parse->frames);
    parse->ast_frame_root_ref = op.ast_frame_root_ref;
    parse->ast_prev_child_ref = op.ast_prev_child_ref;
    parse->state = op.ret_state;
    parse->group_flags = op.group_flags;
    parse->depth_max_prev = parse->depth_max;
    parse->depth = op.depth;
    parse->depth_max = RE__MAX(op.depth_max, parse->depth_max);
}

/* Add a new node to the end of the stack, while maintaining these invariants:
 * - Group nodes can only hold one immediate node.
 * - Alt nodes can only hold one immediate node per branch.
 * - Concat nodes can hold an infinite number of nodes.
 * 
 * To maintain these, when we have to add a second child to an alt/group node, 
 * we convert it into a concatenation of the first and second children. */
RE_INTERNAL re_error re__parse_link_new_node(re__parse* parse, re__ast new_ast, re_int32* new_ast_ref) {
    re__ast* frame = re__parse_get_frame(parse);
    re__ast_type frame_type = frame->type;
    re_error err = RE_ERROR_NONE;
    /* Weird control flow -- it's the only way I figured out how to do the
     * assertion below. */
    if (frame_type == RE__AST_TYPE_GROUP || frame_type == RE__AST_TYPE_ALT) {
        if (re__parse_frame_is_empty(parse)) {
            /* Push node, fallthrough */
        } else {
            re__ast new_concat;
            re_int32 old_inner;
            re_int32 new_outer;
            /* Push the current frame */
            if ((err = re__parse_frame_push(parse))) {
                return err;
            }
            old_inner = parse->ast_prev_child_ref;
            /* Wrap the last child(ren) in a concatenation */
            re__ast_init_concat(&new_concat);
            if ((err = re__parse_link_wrap_node(parse, new_concat, &new_outer))) {
                return err;
            }
            /* Set frame_ref to the new outer node */
            parse->ast_frame_root_ref = new_outer;
            parse->depth += 1;
            /* Set prev_child_ref to the last child */
            parse->ast_prev_child_ref = old_inner;
            /* frame is now invalid */
            /* new_concat is moved */
        }
    } else if (frame_type == RE__AST_TYPE_CONCAT) {
        /* Push node, fallthrough */
    } else {
        /* Due to operator precedence, we should never arrive here. */
        RE__ASSERT_UNREACHED();
    }
    /* Add the new node to the frame. */
    if ((err = re__parse_push_node(parse, new_ast, new_ast_ref))) {
        return err;
    }
    return err;
}

RE_INTERNAL re_error re__parse_add_new_node(re__parse* parse, re__ast new_ast) {
    re_int32 dummy;
    return re__parse_link_new_node(parse, new_ast, &dummy);
}

RE_INTERNAL re_error re__parse_finish(re__parse* parse) {
    re_error err = RE_ERROR_NONE;
    /* Pop frames until frame_ptr == 0. */
    while (1) {
        re__ast* frame = re__parse_get_frame(parse);
        re__ast_type peek_type = frame->type;
        if (parse->ast_frame_root_ref == 0) {
            /* We have hit the base frame successfully. */
            /* Since the base frame is a group, if we continue the loop we will
             * run into an error. */
            break;
        }
        if (peek_type == RE__AST_TYPE_CONCAT || peek_type == RE__AST_TYPE_ALT) {
            /* These operators are binary and can be popped, but only if they
             * have more than one node. */
            /* Currently, we disallow unary alternations and concatenations. */
            /*if (re__ast_get_children_count(frame) == 1) {
                if (peek_type == RE__AST_TYPE_ALT) {
                    return re__parse_error(parse, "cannot use '|' operator with only one value");
                } else if (peek_type == RE__AST_TYPE_CONCAT) {
                    return re__parse_error(parse, "cannot concatenate only one value");
                }
            }*/
            re__parse_frame_pop(parse);
        } else if (peek_type == RE__AST_TYPE_GROUP) {
            /* If we find a group, that means it has not been closed. */
            return re__parse_error(parse, "unmatched '('");
        }
    }
    return err;
}

/* Begin a new group. Push a group node and set stack/previous/base pointers
 * appropriately. Also push the current frame. */
RE_INTERNAL re_error re__parse_group_begin(re__parse* parse) {
    re__ast new_group;
    re_error err = RE_ERROR_NONE;
    re_int32 new_group_ref;
    re__ast_init_group(&new_group);
    /* Set group's flags */
    re__ast_set_group_flags(&new_group, parse->group_flags_new);
    if ((err = re__parse_link_new_node(parse, new_group, &new_group_ref))) {
        return err;
    }
    /* Also pushes old group flags so they can be restored later */
    if ((err = re__parse_frame_push(parse))) {
        return err;
    }
    /* Increment depth */
    parse->depth += 1;
    /* No previous node to keep track of depth for */
    parse->depth_max_prev = parse->depth;
    /* Maximum depth within this group is equal to base depth since there are
     * no child nodes */
    parse->depth_max = parse->depth;
    /* Set running group flags to the new group flags */
    parse->group_flags = parse->group_flags_new;
    /* The frame is now the newly pushed Group node. */
    parse->ast_frame_root_ref = new_group_ref;
    /* There is no previous child */
    parse->ast_prev_child_ref = RE__AST_NONE;
    return err;
}

/* End a group. Pop operators until we get a group node. */
RE_INTERNAL re_error re__parse_group_end(re__parse* parse) {
    while (1) {
        /* Check the type of the current frame */
        re__ast_type peek_type = re__parse_get_frame(parse)->type;
        /* If we are at the absolute bottom of the stack, there was no opening
         * parentheses to begin with. */
        if (parse->ast_frame_root_ref == 0) {
            return re__parse_error(parse, "unmatched ')'");
        }
        /* Now pop the current frame */
        re__parse_frame_pop(parse);
        /* If we just popped a group, finish */
        if (peek_type == RE__AST_TYPE_GROUP) {
            break;
        }
    }
    return RE_ERROR_NONE;
}

/* Act on a '|' character. If this is the first alteration, we wrap the current
 * group node in an ALT. If not, we simply add to the previous ALT. */
RE_INTERNAL re_error re__parse_alt(re__parse* parse) {
    re__ast_type peek_type;
    re_error err = RE_ERROR_NONE;
    while (1) {
        peek_type = re__parse_get_frame(parse)->type;
        if (peek_type == RE__AST_TYPE_CONCAT) {
            /* Pop all concatenations, alt takes priority */
            re__parse_frame_pop(parse);
        } else if (peek_type == RE__AST_TYPE_GROUP) {
            /* This is the initial alteration: "a|" or "(a|" */
            /* Note: the group in question could be the base frame. */
            /* In any case, we shim an ALT node in before the previous child. */
            /* Since GROUP nodes are defined to have a maximum of one child, we
             * don't need to mess around with the amount of children for either
             * node. */
            re__ast new_alt;
            re_int32 new_alt_ref;
            re__ast_init_alt(&new_alt);
            if ((err = re__parse_link_wrap_node(parse, new_alt, &new_alt_ref))) {
                return err;
            }
            /* frame is now invalid */
            /* Push a new ALT frame */
            if ((err = re__parse_frame_push(parse))) {
                return err;
            }
            /* Set depth correctly, see group_begin() */
            parse->depth += 1;
            parse->depth_max_prev = parse->depth;
            /* Perform a MAX here because we need to keep track of the previous
             * node that started the ALT */
            parse->depth_max = RE__MAX(parse->depth_max, parse->depth);
            /* Set base_ptr to the ALT node */
            parse->ast_frame_root_ref = new_alt_ref;
            /* Indicate that there are no new children. */
            parse->ast_prev_child_ref = RE__AST_NONE;
            return err;
        } else if (peek_type == RE__AST_TYPE_ALT) {
            /* Third+ part of the alteration: "a|b|" or "(a|b|" */
            /* Indicate that there are no new children (this is the beginning
             * of the second+ part of an alteration) */
            parse->ast_prev_child_ref = RE__AST_NONE;
            parse->depth_max = RE__MAX(parse->depth_max, parse->depth_max_prev);
            return err;
        }
    }
}

/* Ingest a single character literal. */
RE_INTERNAL re_error re__parse_char(re__parse* parse, re_char ch) {
    re__ast new_char;
    re__ast_init_rune(&new_char, ch);
    return re__parse_add_new_node(parse, new_char);
}

/* Clear number parsing state. */
RE_INTERNAL void re__parse_radix_clear(re__parse* parse) {
    parse->radix_num = 0;
    parse->radix_digits = 0;
}

/* Maximum octal value that can be held in three digits */
#define RE__PARSE_OCT_NUM_MAX (8 * 8 * 8)
#define RE__PARSE_OCT_DIGITS_MAX 3
#define RE__PARSE_IS_OCT_DIGIT(ch) \
    ((ch) == '0' || (ch) == '1' || (ch) == '2' || (ch) == '3' || (ch) == '4' || (ch) == '5' || (ch) == '6' || (ch) == '7')

/* Parse in a single octal digit. */
RE_INTERNAL re_error re__parse_radix_consume_oct(re__parse* parse, re_rune ch) {
    if (parse->radix_digits == RE__PARSE_OCT_DIGITS_MAX) {
        return re__parse_error(parse, "octal literal exceeds maximum of three digits");
    }
    RE_ASSERT(RE__PARSE_IS_OCT_DIGIT(ch));
    parse->radix_num *= 8;
    parse->radix_num += ((re_uint32)ch) - '0';
    parse->radix_digits++;
    if (parse->radix_num >= RE__PARSE_OCT_NUM_MAX) {
        return re__parse_error(parse, "octal literal exceeds maximum value of \\777");
    }
    return RE_ERROR_NONE;
}

/* Ensure that the radix doesn't exceed octal limits. */
RE_INTERNAL re_error re__parse_radix_check_oct(re__parse* parse) {
    if (parse->radix_num >= RE__PARSE_OCT_NUM_MAX) {
        return re__parse_error(parse, "octal literal exceeds maximum value of \\777");
    }
    if (parse->radix_digits > RE__PARSE_OCT_DIGITS_MAX) {
        return re__parse_error(parse, "octal literal exceeds maximum of three digits");
    }
    return RE_ERROR_NONE;
}

#define RE__PARSE_HEX_SHORT_NUM_MAX (16 * 16)
#define RE__PARSE_HEX_SHORT_DIGITS_MAX  2
#define RE__PARSE_IS_HEX_DIGIT(ch) \
    ((ch) == '0' || (ch) == '1' || (ch) == '2' || (ch) == '3' || (ch) == '4' || (ch) == '5' || (ch) == '6' || (ch) == '7' || \
     (ch) == '8' || (ch) == '9' || (ch) == 'A' || (ch) == 'B' || (ch) == 'C' || (ch) == 'D' || (ch) == 'E' || (ch) == 'F' || \
     (ch) == 'a' || (ch) == 'b' || (ch) == 'c' || (ch) == 'd' || (ch) == 'e' || (ch) == 'f')

RE_INTERNAL re_rune re__parse_radix_hex_digit(re_rune dig) {
    if (dig >= '0' && dig <= '9') {
        return dig - '0';
    } else if (dig >= 'A' && dig <= 'F') {
        return (dig - 'A') + 10;
    } else if (dig >= 'a' && dig <= 'f') {
        return (dig - 'a') + 10;
    } else {
        RE__ASSERT_UNREACHED();
        return 0;
    }
}

/* Parse in a single hex digit in short format (\x..) */
RE_INTERNAL re_error re__parse_radix_consume_hex_short(re__parse* parse, re_rune ch) {
    if (parse->radix_digits == RE__PARSE_HEX_SHORT_DIGITS_MAX) {
        return re__parse_error(parse, "short hex literal exceeds maximum of two digits");
    }
    RE_ASSERT(RE__PARSE_IS_HEX_DIGIT(ch));
    parse->radix_num *= 16;
    parse->radix_num += re__parse_radix_hex_digit(ch);
    parse->radix_digits++;
    if (parse->radix_num >= RE__PARSE_HEX_SHORT_NUM_MAX) {
        return re__parse_error(parse, "short hex literal exceeds maximum value of \\xFF");
    }
    return RE_ERROR_NONE;
}

/* Ensure that the radix doesn't exceed hex limits. */
RE_INTERNAL re_error re__parse_radix_check_hex_short(re__parse* parse) {
    if (parse->radix_num >= RE__PARSE_HEX_SHORT_NUM_MAX) {
        return re__parse_error(parse, "short hex literal exceeds maximum value of \\xFF");
    }
    if (parse->radix_digits > RE__PARSE_HEX_SHORT_DIGITS_MAX) {
        return re__parse_error(parse, "short hex literal exceeds maximum of two digits");
    }
    return RE_ERROR_NONE;
}

#define RE__PARSE_HEX_LONG_NUM_MAX RE_RUNE_MAX+1
#define RE__PARSE_HEX_LONG_DIGITS_MAX 6

/* Parse in a single hex digit in long format (\x{....}) */
RE_INTERNAL re_error re__parse_radix_consume_hex_long(re__parse* parse, re_rune ch) {
    if (parse->radix_digits == RE__PARSE_HEX_LONG_DIGITS_MAX) {
        return re__parse_error(parse, "long hex literal exceeds maximum of six digits");
    }
    RE_ASSERT(RE__PARSE_IS_HEX_DIGIT(ch));
    parse->radix_num *= 16;
    parse->radix_num += re__parse_radix_hex_digit(ch);
    parse->radix_digits++;
    if (parse->radix_num >= RE__PARSE_HEX_LONG_NUM_MAX) {
        return re__parse_error(parse, "long hex literal exceeds maximum value of \\x{10FFFF}");
    }
    return RE_ERROR_NONE;
}

/* Ensure that the radix doesn't exceed hex limits. */
RE_INTERNAL re_error re__parse_radix_check_hex_long(re__parse* parse) {
    if (parse->radix_num > RE__PARSE_HEX_LONG_NUM_MAX) {
        return re__parse_error(parse, "long hex literal exceeds maximum value of \\x{10FFFF}");
    }
    if (parse->radix_digits > RE__PARSE_HEX_LONG_DIGITS_MAX) {
        return re__parse_error(parse, "long hex literal exceeds maximum of six digits");
    }
    return RE_ERROR_NONE;
}

#define RE__PARSE_COUNTING_DIGITS_MAX 6
#define RE__PARSE_IS_DEC_DIGIT(ch) \
    ((ch) == '0' || (ch) == '1' || (ch) == '2' || (ch) == '3' || (ch) == '4' || (ch) == '5' || (ch) == '6' || (ch) == '7' || \
     (ch) == '8' || (ch) == '9')

/* Parse in a single decimal digit */
RE_INTERNAL re_error re__parse_radix_consume_counting(re__parse* parse, re_rune ch) {
    if (parse->radix_digits == RE__PARSE_COUNTING_DIGITS_MAX) {
        return re__parse_error(parse, "counting literal exceeds maximum of four digits");
    }
    RE_ASSERT(RE__PARSE_IS_DEC_DIGIT(ch));
    parse->radix_num *= 10;
    parse->radix_num += ch - '0';
    parse->radix_digits++;
    if (parse->radix_num >= RE__AST_QUANTIFIER_MAX) {
        return re__parse_error(parse, "counting literal exceeds maximum value of " RE__STRINGIFY(RE__AST_QUANTIFIER_MAX) );
    }
    return RE_ERROR_NONE;
}

/* Ensure that the radix doesn't exceed decimal counting limits. */
RE_INTERNAL re_error re__parse_radix_check_counting(re__parse* parse) {
    if (parse->radix_num > RE__AST_QUANTIFIER_MAX) {
        return re__parse_error(parse, "counting literal exceeds maximum value of " RE__STRINGIFY(RE__AST_QUANTIFIER_MAX) );
    }
    if (parse->radix_digits > RE__PARSE_COUNTING_DIGITS_MAX) {
        return re__parse_error(parse, "counting literal exceeds maximum of four digits");
    }
    return RE_ERROR_NONE;
}

/* Disallow an escape character from occurring in a character class. */
/* This function succeeds when the calling state is GND, but does not when the
 * calling state is anything else, in which case it returns an error. */
/* Ensure that this is only called with printable characters. */
RE_INTERNAL re_error re__parse_disallow_escape_in_charclass(re__parse* parse, re_rune esc) {
    re_error err = RE_ERROR_NONE;
    re__parse_frame top;
    re__str err_str;
    RE_ASSERT(re__parse_frame_vec_size(&parse->frames) > 0);
    top = re__parse_frame_vec_peek(&parse->frames);
    if (top.ret_state != RE__PARSE_STATE_GND) {
        /* Build error message */
        re_char esc_ch = (re_char)(esc);
        if ((err = re__str_init_s(&err_str, (const re_char*)"cannot use escape sequence '\\"))) {
            goto destroy_err_str;
        }
        if ((err = re__str_cat_n(&err_str, 1, &esc_ch))) {
            goto destroy_err_str;
        }
        if ((err = re__str_cat_s(&err_str, (const re_char*)"' from within character class (\"[]\")"))) {
            goto destroy_err_str;
        }
        re__set_error_str(parse->re, &err_str);
        return RE_ERROR_PARSE;
    }
    return err;
destroy_err_str:
    re__str_destroy(&err_str);
    return err;
}

/* Create a new assert */
RE_INTERNAL re_error re__parse_create_assert(re__parse* parse, re__ast_assert_type assert_type) {
    re__ast new_node;
    re__ast_init_assert(&new_node, assert_type);
    return re__parse_add_new_node(parse, new_node);
}

/* Create a new "any byte" (\C) */
RE_INTERNAL re_error re__parse_create_any_byte(re__parse* parse) {
    re__ast new_node;
    re__ast_init_any_byte(&new_node);
    return re__parse_add_new_node(parse, new_node);
}

/* Create a new "any char" (.) */
RE_INTERNAL re_error re__parse_create_any_char(re__parse* parse) {
    re__ast new_node;
    re__ast_init_any_char(&new_node);
    return re__parse_add_new_node(parse, new_node);
}

RE_INTERNAL void re__parse_charclass_begin(re__parse* parse) {
    re__charclass_builder_begin(&parse->charclass_builder);
    parse->charclass_lo_rune = -1;
}

RE_INTERNAL void re__parse_charclass_setlo(re__parse* parse, re_rune ch) {
    parse->charclass_lo_rune = ch;
}

RE_INTERNAL re_error re__parse_charclass_addlo(re__parse* parse) {
    re_error err = RE_ERROR_NONE;
    re__rune_range new_range;
    RE_ASSERT(parse->charclass_lo_rune != -1);
    new_range.min = parse->charclass_lo_rune;
    new_range.max = parse->charclass_lo_rune;
    parse->charclass_lo_rune = -1;
    if ((err = re__charclass_builder_insert_range(&parse->charclass_builder, new_range))) {
        return err;
    }
    return err;
}

RE_INTERNAL re_error re__parse_charclass_addhi(re__parse* parse, re_rune ch) {
    re_error err = RE_ERROR_NONE;
    re__rune_range new_range;
    RE_ASSERT(parse->charclass_lo_rune != -1);
    new_range.min = parse->charclass_lo_rune;
    new_range.max = ch;
    parse->charclass_lo_rune = -1;
    if ((err = re__charclass_builder_insert_range(&parse->charclass_builder, new_range))) {
        return err;
    }
    return err;
}


RE_INTERNAL re_error re__parse_charclass_finish(re__parse* parse) {
    re__ast new_node;
    re__charclass new_charclass;
    re_error err = RE_ERROR_NONE;
    if ((err = re__charclass_builder_finish(&parse->charclass_builder, &new_charclass))) {
        return err;
    }
    re__ast_init_class(&new_node, new_charclass);
    if ((err = re__parse_add_new_node(parse, new_node))) {
        return err;
    }
    return err;
}

/* Depending on the current state, push a new character AST node, or add the
 * given character to the current character class. */
RE_INTERNAL re_error re__parse_finish_escape_char(re__parse* parse, re_rune ch) {
    re__parse_state top_state;
    re_error err = RE_ERROR_NONE;
    /* This can only be called on GND or a charclass builder state. */
    RE_ASSERT(re__parse_frame_vec_size(&parse->frames) > 0);
    top_state = re__parse_frame_vec_peek(&parse->frames).ret_state;
    re__parse_frame_pop(parse);
    if (top_state == RE__PARSE_STATE_GND) {
        /* Wrap it in an AST node */
        re__ast new_node;
        re__ast_init_rune(&new_node, ch);
        if ((err = re__parse_add_new_node(parse, new_node))) {
            return err;
        }
    } else if (top_state == RE__PARSE_STATE_CHARCLASS_AFTER_LO) {
        RE_ASSERT(parse->charclass_lo_rune == -1);
        re__parse_charclass_setlo(parse, ch);
    } else if (top_state == RE__PARSE_STATE_CHARCLASS_LO) {
        RE_ASSERT(parse->charclass_lo_rune != -1);
        if ((err = re__parse_charclass_addhi(parse, ch))) {
            return err;
        }
    } else {
        RE__ASSERT_UNREACHED();
    }
    return err;
}

/* Depending on the current state, add a character class *into* a character
 * class, *OR* add a character class AST node. */
RE_INTERNAL re_error re__parse_finish_escape_class(re__parse* parse, re__charclass_ascii_type ascii_cc, int inverted) {
    re__parse_state top_state;
    re__charclass new_class;
    re_error err = RE_ERROR_NONE;
    top_state = re__parse_frame_vec_peek(&parse->frames).ret_state;
    re__parse_frame_pop(parse);
    /* Create the character class */
    if ((err = re__charclass_init_from_class(&new_class, ascii_cc, inverted))) {
        return err;
    }
    if (top_state == RE__PARSE_STATE_GND) {
        /* Wrap it in an AST node */
        re__ast new_node;
        re__ast_init_class(&new_node, new_class);
        /* new_node now owns new_class */
        if ((err = re__parse_add_new_node(parse, new_node))) {
            re__charclass_destroy(&new_class);
            return err;
        }
    } else if (top_state == RE__PARSE_STATE_CHARCLASS_AFTER_LO) {
        RE_ASSERT(parse->charclass_lo_rune == -1);
        /* Add it to the charclass builder */
        if ((err = re__charclass_builder_insert_class(&parse->charclass_builder, &new_class))) {
            /* destroy charclass */
            re__charclass_destroy(&new_class);
            return err;
        }
        re__charclass_destroy(&new_class);
        /* This is possibly the most hacky bit within the entire parser. Since
         * a char class cannot have a "range end", we set state to 
         * RE__PARSE_STATE_CHARCLASS_LO. This is, to my knowledge, the only time
         * we explicity set state outside of the main parse function. */
        parse->state = RE__PARSE_STATE_CHARCLASS_LO;
    } else if (top_state == RE__PARSE_STATE_CHARCLASS_LO) {
        if (parse->charclass_lo_rune != -1) {
            re__charclass_destroy(&new_class);
            return re__parse_error(parse, "cannot have character class as upper bound of character range");
        }
    }
    return err;
}

RE_INTERNAL void re__parse_defer(re__parse* parse) {
    parse->defer = 1;
}

RE_INTERNAL re_error re__parse_create_star(re__parse* parse) {
    re__ast new_star;
    if (re__parse_frame_is_empty(parse)) {
        return re__parse_error(parse, "cannot use '*' operator with nothing");
    }
    re__ast_init_quantifier(&new_star, 0, RE__AST_QUANTIFIER_INFINITY);
    re__ast_set_quantifier_greediness(&new_star, !!!(parse->group_flags & RE__AST_GROUP_FLAG_UNGREEDY));
    return re__parse_wrap_node(parse, new_star);
}

RE_INTERNAL re_error re__parse_create_plus(re__parse* parse) {
    re__ast new_plus;
    if (re__parse_frame_is_empty(parse)) {
        return re__parse_error(parse, "cannot use '+' operator with nothing");
    }
    re__ast_init_quantifier(&new_plus, 1, RE__AST_QUANTIFIER_INFINITY);
    re__ast_set_quantifier_greediness(&new_plus, !!!(parse->group_flags & RE__AST_GROUP_FLAG_UNGREEDY));
    return re__parse_wrap_node(parse, new_plus);
}

RE_INTERNAL re_error re__parse_create_question(re__parse* parse) {
    re__ast new_question;
    if (re__parse_frame_is_empty(parse)) {
        return re__parse_error(parse, "cannot use '?' operator with nothing");
    }
    re__ast_init_quantifier(&new_question, 0, 2);
    re__ast_set_quantifier_greediness(&new_question, !!!(parse->group_flags & RE__AST_GROUP_FLAG_UNGREEDY));
    return re__parse_wrap_node(parse, new_question);
}

RE_INTERNAL re_error re__parse_create_repeat(re__parse* parse, re_int32 min, re_int32 max) {
    re__ast new_question;
    if (re__parse_frame_is_empty(parse)) {
        return re__parse_error(parse, "cannot use '{' operator with nothing");
    }
    if (min == 0 && max == 1) {
        return re__parse_error(parse, "repetition count must not be zero for '{' operator");
    }
    if (max <= min) {
        return re__parse_error(parse, "minimum repetition count must be less than maximum repetition count for '{' operator");
    }
    re__ast_init_quantifier(&new_question, min, max);
    re__ast_set_quantifier_greediness(&new_question, !!!(parse->group_flags & RE__AST_GROUP_FLAG_UNGREEDY));
    return re__parse_wrap_node(parse, new_question);
}

RE_INTERNAL void re__parse_swap_greedy(re__parse* parse) {
    re__ast* quant;
    /* Cannot make nothing ungreedy */
    RE_ASSERT(!re__parse_frame_is_empty(parse));
    quant = re__ast_root_get(&parse->ast_root, parse->ast_prev_child_ref);
    /* Must be a quantifier */
    RE_ASSERT(quant->type == RE__AST_TYPE_QUANTIFIER);
    re__ast_set_quantifier_greediness(quant, !re__ast_get_quantifier_greediness(quant));
}

#define RE__IS_LAST() (ch == -1)
/* This macro is only used within re__parse_regex. */
/* Try-except usually encourages forgetting to clean stuff up, but the 
 * constraints on code within this function allow us to always use this macro 
 * safely. */
/* I think it's a good design choice. */
#define RE__TRY(expr) \
    if ((err = expr)) { \
        goto error; \
    }

RE_INTERNAL re_error re__parse_regex(re__parse* parse, re_size regex_size, const re_char* regex) {
    /*const re_char* start = regex;*/
    const re_char* end = regex + regex_size;
    re_error err = RE_ERROR_NONE;
    re__ast new_group;
    re_int32 new_group_ref;
    /* Start by pushing the initial GROUP node. */
    re__ast_init_group(&new_group);
    if ((err = re__ast_root_add(&parse->ast_root, new_group, &new_group_ref))) {
        return err;
    }
    /* Set stack/previous pointers accordingly. */
    parse->ast_prev_child_ref = RE__AST_NONE;
    /* Set initial depth. */
    parse->depth = 1; /* 1 because of initial group */
    parse->depth_max = 1;
    parse->depth_max_prev = 1; /* same as parse->depth */
    /* Push the base op (group) */
    if ((err = re__parse_frame_push(parse))) {
        return err;
    }
    /* Set the frame pointer to the group node. */
    parse->ast_frame_root_ref = new_group_ref;
    while (regex <= end) {
        /* ch will only be -1 if the if this is the last character, a.k.a.
         * "epsilon" as all the cool kids call it */
        re_rune ch = -1;
        /* Otherwise ch is the character in question */
        if (regex < end) {
            ch = *regex;
        }
        if (parse->state == RE__PARSE_STATE_GND) {
            /* Within the main body of the state machine, everything is kept 
             * purposefully as simple as possible, in order to avoid errors and 
             * keep things easy to follow.
             * Notably:
             *  - No temporaries.
             *  - No control flow.
             *  - Everything is either a function call or an assignment to some
             *    parser variable.
             * Specicfically, disallowing temporaries (i.e., objects that need
             * to be cleaned up) allows much more simplified error handling at
             * the cost of more function calls and functions used during
             * parsing. In this case, it allows the use of RE__TRY macros, which
             * just check the error code returned by each function and goto a
             * single central error handling location. */
            if (RE__IS_LAST()) {
                /* Finish parsing. */
                RE__TRY(re__parse_finish(parse));
            } else if (ch == '$') {
                /* $: Text end assert. */
                RE__TRY(re__parse_create_assert(parse, RE__AST_ASSERT_TYPE_TEXT_END));
            } else if (ch == '(') {
                /* (: Begin a group. */
                parse->group_flags_new = parse->group_flags;
                parse->state = RE__PARSE_STATE_PARENS_INITIAL;
            } else if (ch == ')') {
                /* ): End a group. */
                RE__TRY(re__parse_group_end(parse));
            } else if (ch == '*') {
                /* *: Create a Kleene star. */
                RE__TRY(re__parse_create_star(parse));
                parse->state = RE__PARSE_STATE_MAYBE_QUESTION;
            } else if (ch == '+') {
                /* +: Create a Kleene plus. */
                RE__TRY(re__parse_create_plus(parse));
                parse->state = RE__PARSE_STATE_MAYBE_QUESTION;
            } else if (ch == '.') {
                /* .: Create an "any character." */
                RE__TRY(re__parse_create_any_char(parse));
            } else if (ch == '?') {
                /* ?: Create a question. */
                RE__TRY(re__parse_create_question(parse));
                parse->state = RE__PARSE_STATE_MAYBE_QUESTION;
            } else if (ch == '[') {
                /* [: Start of a character class. */
                re__parse_charclass_begin(parse);
                parse->state = RE__PARSE_STATE_CHARCLASS_INITIAL;
            } else if (ch == '\\') {
                /* Switch to ESCAPE, and push a frame. */
                RE__TRY(re__parse_frame_push(parse));
                parse->state = RE__PARSE_STATE_ESCAPE;  
            } else if (ch == '^') {
                /* ^: Text start assert. */
                RE__TRY(re__parse_create_assert(parse, RE__AST_ASSERT_TYPE_TEXT_START));
            } else if (ch == '{') {
                /* {: Start of counting form. */
                re__parse_radix_clear(parse);
                parse->counting_first_num = 0;
                parse->state = RE__PARSE_STATE_COUNTING_FIRST_NUM_INITIAL;
            } else if (ch == '|') {
                /* Begin or continue an alternation. */
                RE__TRY(re__parse_alt(parse));
            } else {
                /* Push a character. */
                RE__TRY(re__parse_char(parse, (re_char)ch));
            }
        } else if (parse->state == RE__PARSE_STATE_MAYBE_QUESTION) {
            if (RE__IS_LAST()) {
                /* Defer finishing parsing to GND. */
                re__parse_defer(parse);
                parse->state = RE__PARSE_STATE_GND;
            } else if (ch == '?') {
                /* <quant>?: Swap <quant>'s non-greediness. */
                re__parse_swap_greedy(parse);
                parse->state = RE__PARSE_STATE_GND;
            } else {
                re__parse_defer(parse);
                parse->state = RE__PARSE_STATE_GND;
            }
        } else if (parse->state == RE__PARSE_STATE_ESCAPE) {
            /* Escape character: \ */
            if (RE__IS_LAST()) {
                /* \ followed by end of string */
                RE__TRY(re__parse_error(parse, "unfinished escape sequence"));
            } else if (RE__PARSE_IS_OCT_DIGIT(ch)) {
                /* \0 - \7: Octal digits */
                re__parse_radix_clear(parse);
                RE__TRY(re__parse_radix_consume_oct(parse, ch));
                parse->state = RE__PARSE_STATE_OCTAL_SECOND_DIGIT;
            } else if (ch == 'A') {
                /* \A: Absolute text start */
                /* This cannot be used from within a char class */
                RE__TRY(re__parse_disallow_escape_in_charclass(parse, ch));
                /* Return to GND or calling state */
                re__parse_frame_pop(parse);
                RE__TRY(re__parse_create_assert(parse, RE__AST_ASSERT_TYPE_TEXT_START_ABSOLUTE));
            } else if (ch == 'B') {
                /* \B: Not a word boundary */
                RE__TRY(re__parse_disallow_escape_in_charclass(parse, ch));
                re__parse_frame_pop(parse);
                RE__TRY(re__parse_create_assert(parse, RE__AST_ASSERT_TYPE_WORD_NOT));
            } else if (ch == 'C') {
                /* \C: Any *byte* (NOT any char) */
                RE__TRY(re__parse_disallow_escape_in_charclass(parse, ch));
                re__parse_frame_pop(parse);
                RE__TRY(re__parse_create_any_byte(parse));
            } else if (ch == 'D') {
                /* \D: Non-digit characters */
                RE__TRY(re__parse_finish_escape_class(parse, RE__CHARCLASS_ASCII_TYPE_DIGIT, 1));
            } else if (ch == 'E') {
                /* \E: Invalid here */
                RE__TRY(re__parse_error(parse, "\\E can only be used from within \\Q"));
            } else if (ch == 'F' || ch == 'G' || ch == 'H' || ch == 'I' || ch == 'J' || ch == 'K' || ch == 'L' || ch == 'M' || ch == 'N' || ch == 'O') {
                /* \F - \O: Invalid */
                RE__TRY(re__parse_error_invalid_escape(parse, ch));
            } else if (ch == 'P') {
                /* \P: Inverted Unicode character class */
                RE__TRY(re__parse_error(parse, "unimplemented"));
                /*re__parser_unicode_charclass_invert(parser);
                parser.state = RE__PARSE_STATE_UNICODE_CHARCLASS_BEGIN;*/
            } else if (ch == 'Q') {
                /* \Q: Quote begin */
                RE__TRY(re__parse_disallow_escape_in_charclass(parse, ch));
                re__parse_frame_pop(parse);
                parse->state = RE__PARSE_STATE_QUOTE;
            } else if (ch == 'R') {
                /* \R: Invalid */
                RE__TRY(re__parse_error_invalid_escape(parse, ch));
            } else if (ch == 'S') {
                /* \S: Not whitespace (Perl) */
                RE__TRY(re__parse_finish_escape_class(parse, RE__CHARCLASS_ASCII_TYPE_PERL_SPACE, 1));
            } else if (ch == 'T' || ch == 'U' || ch == 'V') {
                /* \T - \V: Invalid */
                RE__TRY(re__parse_error_invalid_escape(parse, ch));
            } else if (ch == 'W') {
                /* \W: Not a word character */
                RE__TRY(re__parse_finish_escape_class(parse, RE__CHARCLASS_ASCII_TYPE_WORD, 1));
            } else if (ch == 'X' || ch == 'Y' || ch == 'Z') {
                /* \X - \Z: Invalid */
                RE__TRY(re__parse_error_invalid_escape(parse, ch));
            } else if (ch == 'a') {
                /* \a: Bell character */
                RE__TRY(re__parse_finish_escape_char(parse, '\a'));
            } else if (ch == 'b') {
                /* \b: Word boundary */
                RE__TRY(re__parse_disallow_escape_in_charclass(parse, ch));
                re__parse_frame_pop(parse);
                RE__TRY(re__parse_create_assert(parse, RE__AST_ASSERT_TYPE_WORD));
            } else if (ch == 'c') {
                /* \c: Invalid */
                RE__TRY(re__parse_error_invalid_escape(parse, ch));
            } else if (ch == 'd') {
                /* \d: Digit characters */
                RE__TRY(re__parse_disallow_escape_in_charclass(parse, ch));
                RE__TRY(re__parse_finish_escape_class(parse, RE__CHARCLASS_ASCII_TYPE_DIGIT, 0));
            } else if (ch == 'e') {
                /* \e: Invalid */
                RE__TRY(re__parse_error_invalid_escape(parse, ch));
            } else if (ch == 'f') {
                /* \f: Form feed character */
                RE__TRY(re__parse_finish_escape_char(parse, '\f'));
            } else if (ch == 'g' || ch == 'h' || ch == 'i' || ch == 'j' || ch == 'k' || ch == 'l' || ch == 'm') {
                /* \g - \m: Invalid */
                RE__TRY(re__parse_error_invalid_escape(parse, ch));
            } else if (ch == 'n') {
                /* \n: Newline */
                RE__TRY(re__parse_finish_escape_char(parse, '\n'));
            } else if (ch == 'o') {
                /* \o: Invalid */
                RE__TRY(re__parse_error_invalid_escape(parse, ch));
            } else if (ch == 'p') {
                /* \p: Unicode character class */
                RE__TRY(re__parse_error(parse, "unimplemented"));
            } else if (ch == 'r') {
                /* \r: Carriage return */
                RE__TRY(re__parse_finish_escape_char(parse, '\r'));
            } else if (ch == 's') {
                /* \s: Whitespace (Perl) */
                RE__TRY(re__parse_finish_escape_class(parse, RE__CHARCLASS_ASCII_TYPE_PERL_SPACE, 0));
            } else if (ch == 't') {
                /* \t: Horizontal tab */
                RE__TRY(re__parse_finish_escape_char(parse, '\t'));
            } else if (ch == 'u') {
                /* \u: Invalid */
                RE__TRY(re__parse_error_invalid_escape(parse, ch));
            } else if (ch == 'v') {
                /* \v: Vertical tab */
                RE__TRY(re__parse_finish_escape_char(parse, '\v'));
            } else if (ch == 'w') {
                /* \w: Word character */
                RE__TRY(re__parse_finish_escape_class(parse, RE__CHARCLASS_ASCII_TYPE_WORD, 0));
            } else if (ch == 'x') {
                /* \x: Two digit hex literal or one to six digit hex literal */
                re__parse_radix_clear(parse);
                parse->state = RE__PARSE_STATE_HEX_INITIAL;
            } else if (ch == 'y') {
                /* \y: Invalid */
                RE__TRY(re__parse_error_invalid_escape(parse, ch));
            } else if (ch == 'z') {
                /* \z: Absolute text end */
                RE__TRY(re__parse_disallow_escape_in_charclass(parse, ch));
                re__parse_frame_pop(parse);
                RE__TRY(re__parse_create_assert(parse, RE__AST_ASSERT_TYPE_TEXT_END_ABSOLUTE));
            } else {
                /* All other characters */
                RE__TRY(re__parse_finish_escape_char(parse, ch));
            }
        } else if (parse->state == RE__PARSE_STATE_PARENS_INITIAL) {
            /* Start of group: ( */
            if (RE__IS_LAST()) {
                re__parse_defer(parse);
                parse->state = RE__PARSE_STATE_GND;
            } else if (ch == '?') {
                /* (?: Start of group flags/name */
                parse->state = RE__PARSE_STATE_PARENS_FLAG_INITIAL;
            } else {
                /* Everything else: begin group */
                re__parse_defer(parse);
                parse->state = RE__PARSE_STATE_GND;
                RE__TRY(re__parse_group_begin(parse));
            }
        } else if (parse->state == RE__PARSE_STATE_PARENS_FLAG_INITIAL) {
            /* Start of group flags: (? */
            if (RE__IS_LAST()) {
                RE__TRY(re__parse_error(parse, "expected one of '-', ':', 'P', 'U', 'i', 'm', 's' for group flags or name"));
            } else if (ch == ')') {
                /* (?): Go back to GND without creating a group, retain flags */
                parse->group_flags = parse->group_flags_new;
                parse->state = RE__PARSE_STATE_GND; 
            } else if (ch == '-') {
                /* (?-: Negate remaining flags */
                parse->state = RE__PARSE_STATE_PARENS_FLAG_NEGATE;
            } else if (ch == ':') {
                /* (?:: Non-matching group, also signals end of flags */
                parse->group_flags_new |= RE__AST_GROUP_FLAG_NONMATCHING;
                parse->state = RE__PARSE_STATE_PARENS_AFTER_COLON;
            } else if (ch == 'P') {
                /* (?P: Start of group name */
                parse->str_begin = regex;
                parse->str_end = regex;
                parse->state = RE__PARSE_STATE_PARENS_NAME_INITIAL;
            } else if (ch == 'U') {
                /* (?U: Ungreedy mode: *+? operators have priority swapped */
                parse->group_flags_new |= RE__AST_GROUP_FLAG_UNGREEDY;
            } else if (ch == 'i') {
                /* (?i: Case insensitive matching */
                parse->group_flags_new |= RE__AST_GROUP_FLAG_CASE_INSENSITIVE;
            } else if (ch == 'm') {
                /* (?m: Multi-line mode: ^$ match line boundaries */
                parse->group_flags_new |= RE__AST_GROUP_FLAG_MULTILINE;
            } else if (ch == 's') {
                /* (?s: Stream (?) mode: . matches \n */
                parse->group_flags_new |= RE__AST_GROUP_FLAG_DOT_NEWLINE;
            } else {
                RE__TRY(re__parse_error(parse, "expected one of '-', ':', 'P', 'U', 'i', 'm', 's' for group flags or name"));
            }
        } else if (parse->state == RE__PARSE_STATE_PARENS_FLAG_NEGATE) {
            /* Start of negated group flags: (?:..- */
            if (RE__IS_LAST()) {
                RE__TRY(re__parse_error(parse, "expected one of ':', 'P', 'U', 'i', 'm', 's' for negated group flags or name"));
            } else if (ch == ')') {
                /* (?): Go back to GND without creating a group, retain flags */
                parse->group_flags = parse->group_flags_new;
                parse->state = RE__PARSE_STATE_GND; 
            } else if (ch == ':') {
                /* (?:: Non-matching group, also signals end of flags */
                parse->group_flags_new &= ~((unsigned int)RE__AST_GROUP_FLAG_NONMATCHING);
                parse->state = RE__PARSE_STATE_PARENS_AFTER_COLON;
            } else if (ch == 'P') {
                /* (?P: Start of group name */
                parse->str_begin = regex;
                parse->str_end = regex;
                parse->state = RE__PARSE_STATE_PARENS_NAME_INITIAL;
            } else if (ch == 'U') {
                /* (?U: Ungreedy mode: *+? operators have priority swapped */
                parse->group_flags_new &= ~((unsigned int)RE__AST_GROUP_FLAG_UNGREEDY);
            } else if (ch == 'i') {
                /* (?i: Case insensitive matching */
                parse->group_flags_new &= ~((unsigned int)RE__AST_GROUP_FLAG_CASE_INSENSITIVE);
            } else if (ch == 'm') {
                /* (?m: Multi-line mode: ^$ match line boundaries */
                parse->group_flags_new &= ~((unsigned int)RE__AST_GROUP_FLAG_MULTILINE);
            } else if (ch == 's') {
                /* (?s: Stream (?) mode: . matches \n */
                parse->group_flags_new &= ~((unsigned int)RE__AST_GROUP_FLAG_DOT_NEWLINE);
            } else {
                RE__TRY(re__parse_error(parse, "expected one of ':', 'P', 'U', 'i', 'm', 's' for negated group flags or name"));
            }
        } else if (parse->state == RE__PARSE_STATE_PARENS_AFTER_COLON) {
            /* Group flags, after colon: (?: */
            if (RE__IS_LAST()) {
                RE__TRY(re__parse_error(parse, "expected expression or ')' to close group"));
            } else if (ch == ')') {
                /* (?:): Group ended. This is a zero-length group. Wacky! */
                parse->state = RE__PARSE_STATE_GND;
                RE__TRY(re__parse_group_begin(parse));
                RE__TRY(re__parse_group_end(parse));
            } else {
                /* (?:...: Group has an expression. */
                re__parse_defer(parse);
                parse->state = RE__PARSE_STATE_GND;
                RE__TRY(re__parse_group_begin(parse));
            }
        } else if (parse->state == RE__PARSE_STATE_PARENS_AFTER_P) {
            /* Before <, after P for named group: (?P */
            if (RE__IS_LAST()) {
                RE__TRY(re__parse_error(parse, "expected '<' to begin group name"));
            } else if (ch == '<') {
                /* (?P<: Begin group name */
                parse->str_begin = regex+1;
                parse->str_end = parse->str_begin;
                parse->state = RE__PARSE_STATE_PARENS_NAME_INITIAL;
            } else {
                RE__TRY(re__parse_error(parse, "expected '<' to begin group name"));
            }
        } else if (parse->state == RE__PARSE_STATE_PARENS_NAME_INITIAL) {
            /* First character between <> in named group: (?P< */
            if (RE__IS_LAST()) {
                RE__TRY(re__parse_error(parse, "expected '>' to close group name"));
            } else if (ch == '>') {
                /* (?P<>: Empty group name, currently not allowed */
                RE__TRY(re__parse_error(parse, "cannot create empty group name"));
            } else {
                /* Otherwise, add characters to the group name */
                parse->str_end++;
                parse->state = RE__PARSE_STATE_PARENS_NAME;
            }
        } else if (parse->state == RE__PARSE_STATE_PARENS_NAME) {
            /* Second+ character between <> in named group: (?P<> */
            if (RE__IS_LAST()) {
                RE__TRY(re__parse_error(parse, "expected '>' to close group name"));
            } else if (ch == '>') {
                /* (?P<...>: End of group name, begin group and defer to GND */
                re__parse_defer(parse);
                parse->state = RE__PARSE_STATE_GND;
                RE__TRY(re__parse_group_begin(parse));
            } else {
                /* (?P<...: Name character, append to name */
                parse->str_end++;
            }
        } else if (parse->state == RE__PARSE_STATE_OCTAL_SECOND_DIGIT) {
            /* Second digit in an octal literal: \0 */
            if (RE__IS_LAST()) {
                /* Last: push character and defer */
                RE__TRY(re__parse_radix_check_oct(parse));
                re__parse_defer(parse);
                RE__TRY(re__parse_finish_escape_char(parse, parse->radix_num));
            } else if (RE__PARSE_IS_OCT_DIGIT(ch)) {
                /* \.x: add to the radix accumulator */
                RE__TRY(re__parse_radix_consume_oct(parse, ch));
                RE__TRY(re__parse_radix_check_oct(parse));
                parse->state = RE__PARSE_STATE_OCTAL_THIRD_DIGIT;
            } else {
                /* Other characters: push character and defer */
                RE__TRY(re__parse_radix_check_oct(parse));
                re__parse_defer(parse);
                RE__TRY(re__parse_finish_escape_char(parse, parse->radix_num));
            }
        } else if (parse->state == RE__PARSE_STATE_OCTAL_THIRD_DIGIT) {
            /* Third digit in an octal literal: \00 */
            if (RE__IS_LAST()) {
                /* Last: push character and defer */
                RE__TRY(re__parse_radix_check_oct(parse));
                re__parse_defer(parse);
                RE__TRY(re__parse_finish_escape_char(parse, parse->radix_num));
            } else if (RE__PARSE_IS_OCT_DIGIT(ch)) {
                /* \.x: add to the radix accumulator and finish */
                RE__TRY(re__parse_radix_consume_oct(parse, ch));
                RE__TRY(re__parse_radix_check_oct(parse));
                RE__TRY(re__parse_finish_escape_char(parse, parse->radix_num));
            } else {
                /* Other characters: push character and defer */
                RE__TRY(re__parse_radix_check_oct(parse));
                re__parse_defer(parse);
                RE__TRY(re__parse_finish_escape_char(parse, parse->radix_num));
            }
        } else if (parse->state == RE__PARSE_STATE_HEX_INITIAL) {
            /* Hex literal: \x */
            if (RE__IS_LAST()) {
                /* Last: error */
                RE__TRY(re__parse_error(parse, "expected two hex characters or a bracketed hex literal for hex escape \"\\x\""));
            } else if (RE__PARSE_IS_HEX_DIGIT(ch)) {
                /* \x.: add to the radix accumulator */
                RE__TRY(re__parse_radix_consume_hex_short(parse, ch));
                RE__TRY(re__parse_radix_check_hex_short(parse));
                parse->state = RE__PARSE_STATE_HEX_SECOND_DIGIT;
            } else if (ch == '{') {
                /* \x{: Begin bracketed hex literal */
                parse->state = RE__PARSE_STATE_HEX_BRACKETED;
            } else {
                /* Invalid character: error */
                RE__TRY(re__parse_error(parse, "expected two hex characters or a bracketed hex literal for hex escape \"\\x\""));
            }
        } else if (parse->state == RE__PARSE_STATE_HEX_SECOND_DIGIT) {
            /* Second digit of hex literal: \x. */
            if (RE__IS_LAST()) {
                /* Last: error */
                RE__TRY(re__parse_error(parse, "expected a second hex character for hex escape \"\\x\""));
            } else if (RE__PARSE_IS_HEX_DIGIT(ch)) {
                /* \x..: add to the radix accumulator and finish */
                RE__TRY(re__parse_radix_consume_hex_short(parse, ch));
                RE__TRY(re__parse_radix_check_hex_short(parse));
                RE__TRY(re__parse_finish_escape_char(parse, parse->radix_num));
            } else {
                /* Invalid character: error */
                RE__TRY(re__parse_error(parse, "expected a second hex character for hex escape \"\\x\""));
            }
        } else if (parse->state == RE__PARSE_STATE_HEX_BRACKETED) {
            /* Beginning of one to six digit hex literal: \x{ */  
            if (RE__IS_LAST()) {
                /* Last: error */
                RE__TRY(re__parse_error(parse, "expected one to six hex characters for bracketed hex escape \"\\x{\""))
            } else if (RE__PARSE_IS_HEX_DIGIT(ch)) {
                /* \x{.: add to the radix accumulator */
                RE__TRY(re__parse_radix_consume_hex_long(parse, ch));
                RE__TRY(re__parse_radix_check_hex_long(parse));
            } else if (ch == '}') {
                /* \x{...}: Finish */
                RE__TRY(re__parse_radix_check_hex_long(parse));
                RE__TRY(re__parse_finish_escape_char(parse, parse->radix_num));
            } else {
                /* Invalid character: error */
                RE__TRY(re__parse_error(parse, "expected one to six hex characters for bracketed hex escape \"\\x{\""))
            }
        } else if (parse->state == RE__PARSE_STATE_QUOTE) {
            /* Quote string: \Q */
            if (RE__IS_LAST()) {
                /* Last: defer to GND */
                re__parse_defer(parse);
                parse->state = RE__PARSE_STATE_GND;
            } else if (ch == '\\') {
                /* \Q..\: check for 'E' to end quote */
                parse->state = RE__PARSE_STATE_QUOTE_ESCAPE;
            } else {
                /* Otherwise, add char */
                re__parse_char(parse, (re_char)ch);
            }
        } else if (parse->state == RE__PARSE_STATE_QUOTE_ESCAPE) {
            /* Quote escape: \Q...\ */
            if (RE__IS_LAST()) {
                /* Last: ambiguous, error */
                RE__TRY(re__parse_error(parse, "expected 'E' or a character after '\\' within \"\\Q\""));
            } else if (ch == 'E') {
                /* \Q...\E: finish quote string */
                parse->state = RE__PARSE_STATE_GND;
            } else {
                /* Otherwise, add escaped char */
                re__parse_char(parse, (re_char)ch);
            }
        } else if (parse->state == RE__PARSE_STATE_COUNTING_FIRST_NUM_INITIAL) {
            /* First number in a counting expression */
            if (RE__IS_LAST()) {
                /* Last: not allowed */
                RE__TRY(re__parse_error(parse, "expected decimal digits for repetition expression '{'"));
            } else if (ch == ',') {
                /* {,: Not allowed yet */
                RE__TRY(re__parse_error(parse, "expected a decimal number before ',' in repetition expression '{'"))
            } else if (RE__PARSE_IS_DEC_DIGIT(ch)) {
                /* {.: Accumulate */
                RE__TRY(re__parse_radix_check_counting(parse));
                RE__TRY(re__parse_radix_consume_counting(parse, ch));
                parse->state = RE__PARSE_STATE_COUNTING_FIRST_NUM;
            } else {
                RE__TRY(re__parse_error(parse, "expected decimal digits for repetition expression '{'"));
            }
        } else if (parse->state == RE__PARSE_STATE_COUNTING_FIRST_NUM) {
            /* First number in a counting expression, after first digit: {. */
            if (RE__IS_LAST()) {
                /* Last: not allowed */
                RE__TRY(re__parse_error(parse, "expected decimal digits for repetition expression '{'"));
            } else if (ch == ',') {
                /* {.,: Move to next number */
                RE__TRY(re__parse_radix_check_counting(parse));
                parse->counting_first_num = parse->radix_num;
                re__parse_radix_clear(parse);
                parse->state = RE__PARSE_STATE_COUNTING_SECOND_NUM_INITIAL;
            } else if (RE__PARSE_IS_DEC_DIGIT(ch)) {
                /* {..: Accumulate */
                RE__TRY(re__parse_radix_check_counting(parse));
                RE__TRY(re__parse_radix_consume_counting(parse, ch));
            } else if (ch == '}') {
                /* {..}: Finish */
                RE__TRY(re__parse_radix_check_counting(parse));
                parse->state = RE__PARSE_STATE_MAYBE_QUESTION;
                RE__TRY(re__parse_create_repeat(parse, parse->radix_num, parse->radix_num+1));
            } else {
                RE__TRY(re__parse_error(parse, "expected decimal digits for repetition expression '{'"));
            }
        } else if (parse->state == RE__PARSE_STATE_COUNTING_SECOND_NUM_INITIAL) {
            /* Second number in a counting expression */
            if (RE__IS_LAST()) {
                /* Last: not allowed */
                RE__TRY(re__parse_error(parse, "expected decimal digits for repetition expression '{'"));
            } else if (RE__PARSE_IS_DEC_DIGIT(ch)) {
                /* {..: Accumulate */
                RE__TRY(re__parse_radix_check_counting(parse));
                RE__TRY(re__parse_radix_consume_counting(parse, ch));
                parse->state = RE__PARSE_STATE_COUNTING_SECOND_NUM;
            } else if (ch == '}') {
                /* {.,}: Finish */
                parse->state = RE__PARSE_STATE_MAYBE_QUESTION;
                RE__TRY(re__parse_create_repeat(parse, parse->counting_first_num, RE__AST_QUANTIFIER_INFINITY));
            } else {
                RE__TRY(re__parse_error(parse, "expected decimal digits for repetition expression '{'"))
            }
        } else if (parse->state == RE__PARSE_STATE_COUNTING_SECOND_NUM) {
            /* Second number in a counting expression, after first char */
            if (RE__IS_LAST()) {
                /* Last: not allowed */
                RE__TRY(re__parse_error(parse, "expected decimal digits for repetition expression '{'"));
            } else if (RE__PARSE_IS_DEC_DIGIT(ch)) {
                /* {..: Accumulate */
                RE__TRY(re__parse_radix_check_counting(parse));
                RE__TRY(re__parse_radix_consume_counting(parse, ch));
            } else if (ch == '}') {
                /* {..}: Finish */
                RE__TRY(re__parse_radix_check_counting(parse));
                parse->state = RE__PARSE_STATE_MAYBE_QUESTION;
                RE__TRY(re__parse_create_repeat(parse, parse->counting_first_num, parse->radix_num+1));
            } else {
                RE__TRY(re__parse_error(parse, "expected decimal digits for repetition expression '{'"));
            }
        } else if (parse->state == RE__PARSE_STATE_CHARCLASS_INITIAL) {
            /* After first charclass bracket: [ */
            if (RE__IS_LAST()) {
                RE__TRY(re__parse_error(parse, "expected '^', characters, character classes, or character ranges for character class expression '['"));
            } else if (ch == '[') {
                /* [[: Literal [ or char class*/
                parse->state = RE__PARSE_STATE_CHARCLASS_AFTER_BRACKET;
            } else if (ch == '\\') {
                /* [\: Escape */
                /* Set return state to AFTERLO */
                parse->state = RE__PARSE_STATE_CHARCLASS_AFTER_LO;
                RE__TRY(re__parse_frame_push(parse));
                parse->state = RE__PARSE_STATE_ESCAPE;
            } else if (ch == '^') {
                /* [^: Invert */
                re__charclass_builder_invert(&parse->charclass_builder);
                parse->state = RE__PARSE_STATE_CHARCLASS_AFTER_CARET;
            } else {
                /* Otherwise, add the character */
                re__parse_charclass_setlo(parse, ch);
                parse->state = RE__PARSE_STATE_CHARCLASS_AFTER_LO;
            }
        } else if (parse->state == RE__PARSE_STATE_CHARCLASS_AFTER_CARET) {
            /* After caret in charclass: [^ */
            if (RE__IS_LAST()) {
                RE__TRY(re__parse_error(parse, "expected characters, character classes, or character ranges for character class expression '['"));
            } else if (ch == '[') {
                /* [^[: Literal [ or charclass */
                parse->state = RE__PARSE_STATE_CHARCLASS_AFTER_BRACKET;
            } else if (ch == '\\') {
                /* [^\: Escape */
                parse->state = RE__PARSE_STATE_CHARCLASS_AFTER_LO;
                RE__TRY(re__parse_frame_push(parse));
                parse->state = RE__PARSE_STATE_ESCAPE;
            } else {
                /* Otherwise, add the character */
                re__parse_charclass_setlo(parse, ch);
                parse->state = RE__PARSE_STATE_CHARCLASS_AFTER_LO;
            }
        } else if (parse->state == RE__PARSE_STATE_CHARCLASS_AFTER_BRACKET) {
            /* After initial bracket in charclass: [[ or [^[ */
            if (RE__IS_LAST()) {
                RE__TRY(re__parse_error(parse, "expected characters, character classes, or character ranges for character class expression '['"));
            } else if (ch == '\\') {
                /* [\: Escape. */
                re__parse_charclass_setlo(parse, '[');
                RE__TRY(re__parse_charclass_addlo(parse));
                parse->state = RE__PARSE_STATE_CHARCLASS_AFTER_LO;
                RE__TRY(re__parse_frame_push(parse));
                parse->state = RE__PARSE_STATE_ESCAPE;
            } else if (ch == ']') {
                /* []: A single [. */
                parse->state = RE__PARSE_STATE_GND;
                re__parse_charclass_setlo(parse, '[');
                RE__TRY(re__parse_charclass_addlo(parse));
                RE__TRY(re__parse_charclass_finish(parse));
            } else if (ch == ':') {
                /* [:: Start of ASCII charclass */
                parse->state = RE__PARSE_STATE_CHARCLASS_NAMED;
            } else {
                /* Otherwise, add the bracket and the character */
                re__parse_charclass_setlo(parse, '[');
                RE__TRY(re__parse_charclass_addlo(parse));
                re__parse_charclass_setlo(parse, ch);
                parse->state = RE__PARSE_STATE_CHARCLASS_AFTER_LO;
            }
        } else if (parse->state == RE__PARSE_STATE_CHARCLASS_LO) {
            /* Before lo character in a charclass range: [.-. */
            if (RE__IS_LAST()) {
                RE__TRY(re__parse_error(parse, "expected characters, character classes, or character ranges for character class expression '['"));
            } else if (ch == '[') {
                /* [.[: Beginning of ASCII charclass or bracket. */
                parse->state = RE__PARSE_STATE_CHARCLASS_AFTER_BRACKET;
            } else if (ch == '\\') {
                /* [.\: Escape. */
                parse->state = RE__PARSE_STATE_CHARCLASS_AFTER_LO;
                RE__TRY(re__parse_frame_push(parse));
                parse->state = RE__PARSE_STATE_ESCAPE;
            } else if (ch == ']') {
                /* [.]: Finish. */
                parse->state = RE__PARSE_STATE_GND;
                RE__TRY(re__parse_charclass_finish(parse));
            } else {
                /* Otherwise set the low bound. */
                re__parse_charclass_setlo(parse, ch);
                parse->state = RE__PARSE_STATE_CHARCLASS_AFTER_LO;
            }
        } else if (parse->state == RE__PARSE_STATE_CHARCLASS_AFTER_LO) {
            /* After lo character in a charclass range: [. */
            if (RE__IS_LAST()) {
                RE__TRY(re__parse_error(parse, "expected characters, character classes, or character ranges for character class expression '['"));
            } else if (ch == '[') {
                /* [.[: Beginning of ASCII charclass or bracket. */
                RE__TRY(re__parse_charclass_addlo(parse));
                parse->state = RE__PARSE_STATE_CHARCLASS_AFTER_BRACKET;
            } else if (ch == '\\') {
                /* [.\: Escape. */
                RE__TRY(re__parse_charclass_addlo(parse));
                parse->state = RE__PARSE_STATE_CHARCLASS_AFTER_LO;
                RE__TRY(re__parse_frame_push(parse));
                parse->state = RE__PARSE_STATE_ESCAPE;
            } else if (ch == ']') {
                /* [.]: Finished. */
                parse->state = RE__PARSE_STATE_GND;
                RE__TRY(re__parse_charclass_addlo(parse));
                RE__TRY(re__parse_charclass_finish(parse));
            } else if (ch == '-') {
                /* [.-: Set to second range. */
                parse->state = RE__PARSE_STATE_CHARCLASS_HI;  
            } else {
                /* Otherwise, stay here and keep adding chars. */
                RE__TRY(re__parse_charclass_addlo(parse));
                re__parse_charclass_setlo(parse, ch);
            }
        } else if (parse->state == RE__PARSE_STATE_CHARCLASS_HI) {
            /* After low value for range and hyphen : [.- */
            if (RE__IS_LAST()) {
                RE__TRY(re__parse_error(parse, "expected characters, character classes, or character ranges for character class expression '['"));
            } else if (ch == '\\') {
                /* [.-\: Escape. */
                parse->state = RE__PARSE_STATE_CHARCLASS_LO;
                RE__TRY(re__parse_frame_push(parse));
                parse->state = RE__PARSE_STATE_ESCAPE;
            } else if (ch == ']') {
                /* [.-]: Finished. Add a literal hyphen. */
                parse->state = RE__PARSE_STATE_GND;
                RE__TRY(re__parse_charclass_addlo(parse));
                re__parse_charclass_setlo(parse, '-');
                RE__TRY(re__parse_charclass_addlo(parse));
                RE__TRY(re__parse_charclass_finish(parse));
            } else {
                /* Otherwise, add a char and go to range start. */
                RE__TRY(re__parse_charclass_addhi(parse, ch));
                parse->state = RE__PARSE_STATE_CHARCLASS_LO;
            }
        } else {
            RE__ASSERT_UNREACHED();
        }
        if (parse->defer) {
            parse->defer -= 1;
        } else {
            regex++;
        }
    }
    RE_ASSERT(re__parse_frame_vec_size(&parse->frames) == 1);
    return RE_ERROR_NONE;
error:
    if (err == RE_ERROR_PARSE) {
        RE_ASSERT(re__str_size(&parse->re->data->error_string));
    } else {
        re__set_error_generic(parse->re, err);
    }
    return err;
}

#undef RE__IS_LAST
#undef RE__TRY


RE_INTERNAL void re__prog_inst_init(re__prog_inst* inst, re__prog_inst_type inst_type) {
    inst->_inst_type = inst_type;
    inst->_primary = RE__PROG_LOC_INVALID;
}

RE_INTERNAL void re__prog_inst_init_byte(re__prog_inst* inst, re_char chr) {
    re__prog_inst_init(inst, RE__PROG_INST_TYPE_BYTE);
    inst->_inst_data._byte = chr;
}

RE_INTERNAL void re__prog_inst_init_byte_range(re__prog_inst* inst, re_char min, re_char max) {
    re__prog_inst_init(inst, RE__PROG_INST_TYPE_BYTE_RANGE);
    inst->_inst_data._range.min = min;
    inst->_inst_data._range.max = max;
}

RE_INTERNAL void re__prog_inst_init_split(re__prog_inst* inst, re__prog_loc primary, re__prog_loc secondary) {
    re__prog_inst_init(inst, RE__PROG_INST_TYPE_SPLIT);
    inst->_primary = primary;
    inst->_inst_data._secondary = secondary;
}

RE_INTERNAL void re__prog_inst_init_match(re__prog_inst* inst, re_uint32 match_idx) {
    re__prog_inst_init(inst, RE__PROG_INST_TYPE_MATCH);
    inst->_inst_data._match_idx = match_idx;
}

RE_INTERNAL void re__prog_inst_init_fail(re__prog_inst* inst) {
    re__prog_inst_init(inst, RE__PROG_INST_TYPE_FAIL);
}

RE_INTERNAL void re__prog_inst_init_assert(re__prog_inst* inst, re_uint32 assert_context) {
    re__prog_inst_init(inst, RE__PROG_INST_TYPE_ASSERT);
    inst->_inst_data._assert_context = assert_context;
}

RE_INTERNAL void re__prog_inst_init_save(re__prog_inst* inst, re_uint32 save_idx) {
    re__prog_inst_init(inst, RE__PROG_INST_TYPE_SAVE);
    inst->_inst_data._save_idx = save_idx;
}

RE_INTERNAL re__prog_loc re__prog_inst_get_primary(re__prog_inst* inst) {
    return inst->_primary;
}

RE_INTERNAL void re__prog_inst_set_primary(re__prog_inst* inst, re__prog_loc loc) {
    inst->_primary = loc;
}

RE_INTERNAL re_char re__prog_inst_get_byte(re__prog_inst* inst) {
    RE_ASSERT(inst->_inst_type == RE__PROG_INST_TYPE_BYTE);
    return inst->_inst_data._byte;
}

RE_INTERNAL re_char re__prog_inst_get_byte_min(re__prog_inst* inst) {
    RE_ASSERT(inst->_inst_type == RE__PROG_INST_TYPE_BYTE_RANGE);
    return inst->_inst_data._range.min;
}

RE_INTERNAL re_char re__prog_inst_get_byte_max(re__prog_inst* inst) {
    RE_ASSERT(inst->_inst_type == RE__PROG_INST_TYPE_BYTE_RANGE);
    return inst->_inst_data._range.max;
}

RE_INTERNAL re__prog_loc re__prog_inst_get_split_secondary(re__prog_inst* inst) {
    RE_ASSERT(inst->_inst_type == RE__PROG_INST_TYPE_SPLIT);
    return inst->_inst_data._secondary;
}

RE_INTERNAL void re__prog_inst_set_split_secondary(re__prog_inst* inst, re__prog_loc loc) {
    RE_ASSERT(inst->_inst_type == RE__PROG_INST_TYPE_SPLIT);
    inst->_inst_data._secondary = loc;
}

RE_INTERNAL re__prog_inst_type re__prog_inst_get_type(re__prog_inst* inst) {
    return inst->_inst_type;
}

RE_INTERNAL re_uint32 re__prog_inst_get_match_idx(re__prog_inst* inst) {
    RE_ASSERT(inst->_inst_type == RE__PROG_INST_TYPE_MATCH);
    return inst->_inst_data._match_idx;
}

RE_INTERNAL re_uint32 re__prog_inst_get_save_idx(re__prog_inst* inst) {
    RE_ASSERT(inst->_inst_type == RE__PROG_INST_TYPE_SAVE);
    return inst->_inst_data._save_idx;
}

RE_INTERNAL re_uint32 re__prog_inst_get_assert_ctx(re__prog_inst* inst) {
    RE_ASSERT(inst->_inst_type == RE__PROG_INST_TYPE_ASSERT);
    return inst->_inst_data._assert_context;
}

RE_VEC_IMPL_FUNC(re__prog_inst, init)
RE_VEC_IMPL_FUNC(re__prog_inst, destroy)
RE_VEC_IMPL_FUNC(re__prog_inst, push)
RE_VEC_IMPL_FUNC(re__prog_inst, get)
RE_VEC_IMPL_FUNC(re__prog_inst, getref)
RE_VEC_IMPL_FUNC(re__prog_inst, set)
RE_VEC_IMPL_FUNC(re__prog_inst, size)

RE_INTERNAL re_error re__prog_init(re__prog* prog) {
    re__prog_inst_vec_init(&prog->_instructions);
    return RE_ERROR_NONE;
}

RE_INTERNAL void re__prog_destroy(re__prog* prog) {
    re__prog_inst_vec_destroy(&prog->_instructions);
}

RE_INTERNAL re__prog_loc re__prog_size(re__prog* prog) {
    return (re__prog_loc)re__prog_inst_vec_size(&prog->_instructions);
}

RE_INTERNAL re__prog_inst* re__prog_get(re__prog* prog, re__prog_loc loc) {
    return re__prog_inst_vec_getref(&prog->_instructions, loc);
}

RE_INTERNAL void re__prog_set(re__prog* prog, re__prog_loc loc, re__prog_inst inst) {
    re__prog_inst_vec_set(&prog->_instructions, loc, inst);
}

RE_INTERNAL re_error re__prog_add(re__prog* prog, re__prog_inst inst) {
    if (re__prog_size(prog) == RE__PROG_SIZE_MAX) {
        return RE__ERROR_PROGMAX;
    } else {
        return re__prog_inst_vec_push(&prog->_instructions, inst);
    }
}

#if RE_DEBUG

RE_INTERNAL void re__prog_debug_dump(re__prog* prog) {
    re__prog_loc i;
    for (i = 0; i < re__prog_size(prog); i++) {
        re__prog_inst* inst = re__prog_get(prog, i);
        printf("%04X | ", i);
        switch (re__prog_inst_get_type(inst)) {
            case RE__PROG_INST_TYPE_BYTE:
                printf("BYTE v=%0X ('%c')", re__prog_inst_get_byte(inst), re__prog_inst_get_byte(inst));
                break;
            case RE__PROG_INST_TYPE_BYTE_RANGE:
                printf("BYTE_RANGE min=%X ('%c') max=%X ('%c')",
                    re__prog_inst_get_byte_min(inst),
                    re__prog_inst_get_byte_min(inst),
                    re__prog_inst_get_byte_max(inst),
                    re__prog_inst_get_byte_max(inst));
                break;
            case RE__PROG_INST_TYPE_SPLIT:
                printf("SPLIT");
                break;
            case RE__PROG_INST_TYPE_MATCH:
                printf("MATCH idx=%u", re__prog_inst_get_match_idx(inst));
                break;
            case RE__PROG_INST_TYPE_FAIL:
                printf("FAIL");
                break;
            case RE__PROG_INST_TYPE_SAVE:
                printf("SAVE idx=%u", re__prog_inst_get_save_idx(inst));
                break;
            case RE__PROG_INST_TYPE_ASSERT:
                printf("ASSERT ctx=%u", re__prog_inst_get_assert_ctx(inst));
                break;
        }
        printf(" -> %04X", re__prog_inst_get_primary(inst));
        if (re__prog_inst_get_type(inst) == RE__PROG_INST_TYPE_SPLIT) {
            printf(", %04X", re__prog_inst_get_split_secondary(inst));
        }
        printf("\n");
    }
}

#endif /* #if RE_DEBUG */


#include <string.h>

RE_INTERNAL void re__error_init(re* reg) {
    reg->data->error_string_is_const = 0;
    re__str_init(&reg->data->error_string);
}

RE_INTERNAL void re__error_destroy(re* reg) {
    if (!reg->data->error_string_is_const) {
        re__str_destroy(&reg->data->error_string);
    }
}

/* Doesn't take ownership of error_str */
/* We don't return an error here because it's expected that you are already in
 * the middle of cleaning up after an error */
/* This is useful because it allows error_string to be a const pointer,
 * allowing error messages to be saved as just const strings */
RE_INTERNAL void re__set_error_str(re* reg, const re__str* error_str) {
    re_error err = RE_ERROR_NONE;
    re__error_destroy(reg);
    reg->data->error_string_is_const = 0;
    if ((err = re__str_init_copy(&reg->data->error_string, error_str))) {
        re__set_error_generic(reg, err);
    }
}

RE_INTERNAL void re__set_error_generic(re* reg, re_error err) {
    if (err == RE_ERROR_NOMEM) {
        /* No memory: not guaranteed if we can allocate a string. */
        reg->data->error_string_is_const = 1;
        RE__STR_INIT_CONST(&reg->data->error_string, "out of memory");
    } else {
        /* This function is only valid for generic errors */
        RE__ASSERT_UNREACHED();
    }
}

re_error re_init(re* reg, const char* regex) {
    re_error err = RE_ERROR_NONE;
    RE__UNUSED(regex);
    reg->data = (re_data*)RE_MALLOC(sizeof(re_data));
    if (!reg->data) {
        return RE_ERROR_NOMEM;
    }
    re__parse_init(&reg->data->parse, reg);
    re__prog_init(&reg->data->program);
    re__compile_init(&reg->data->compile, reg);
    re__exec_init(&reg->data->exec, reg);
    re__error_init(reg);
    /*if ((err = re__parse(re, strlen(regex), (const re_char*)regex))) {
        return err;
    }*/
    return err;
}

re_error re_destroy(re* reg) {
    re_error err = 0;
    re__exec_destroy(&reg->data->exec);
    re__compile_destroy(&reg->data->compile);
    re__prog_destroy(&reg->data->program);
    re__parse_destroy(&reg->data->parse);
    re__error_destroy(reg);
    RE_FREE(reg->data);
    return err;
}

const char* re_get_error(re* reg, re_size* error_len) {
    if (error_len != RE_NULL) {
        *error_len = re__str_size(&reg->data->error_string);
    }
    return (const char*)re__str_get_data(&reg->data->error_string);
}


int re__byte_range_equals(re__byte_range range, re__byte_range other) {
    return range.min == other.min && range.max == other.max;
}

int re__byte_range_adjacent(re__byte_range range, re__byte_range other) {
    return ((re_uint32)other.min) == ((re_uint32)range.max) + 1;
}

re__byte_range re__byte_range_merge(re__byte_range range, re__byte_range other) {
    re__byte_range out;
    RE_ASSERT(re__byte_range_adjacent(range, other));
    out.min = range.min;
    out.max = other.max;
    return out;
}

int re__rune_range_equals(re__rune_range range, re__rune_range other) {
    return range.min == other.min && range.max == other.max;
}

int re__rune_range_intersects(re__rune_range range, re__rune_range clip) {
    return range.min <= clip.max && clip.min <= range.max;
}

re__rune_range re__rune_range_clamp(re__rune_range range, re__rune_range bounds) {
    re__rune_range out = range;
    RE_ASSERT(re__rune_range_intersects(range, bounds));
    if (range.min < bounds.min) {
        out.min = bounds.min;
    }
    if (range.max > bounds.max) {
        out.max = bounds.max;
    }
    return out;
}


#if 0

RE_INTERNAL re_error re__debug_sexpr_tree_init(re__debug_sexpr_tree* sexpr, const re__str* atom, int is_atom) {
    sexpr->first_child_ref = RE__DEBUG_SEXPR_NONE;
    sexpr->next_sibling_ref = RE__DEBUG_SEXPR_NONE;
    sexpr->is_atom = is_atom;
    return re__str_init_copy(&sexpr->atom, atom);
}

RE_INTERNAL void re__debug_sexpr_tree_destroy(re__debug_sexpr_tree* sexpr) {
    re__str_destroy(&sexpr->atom);
}

RE_VEC_IMPL_FUNC(re__debug_sexpr_tree, init)
RE_VEC_IMPL_FUNC(re__debug_sexpr_tree, destroy)
RE_VEC_IMPL_FUNC(re__debug_sexpr_tree, push)
RE_VEC_IMPL_FUNC(re__debug_sexpr_tree, pop)
RE_VEC_IMPL_FUNC(re__debug_sexpr_tree, size)
RE_VEC_IMPL_FUNC(re__debug_sexpr_tree, getref)

RE_INTERNAL void re__debug_sexpr_init(re__debug_sexpr* sexpr) {
    re__debug_sexpr_tree_vec_init(&sexpr->tree);
}

RE_INTERNAL re__debug_sexpr_tree* re__debug_sexpr_get(re__debug_sexpr* sexpr, re_int32 ref) {
    RE_ASSERT(ref != RE__DEBUG_SEXPR_NONE);
    return re__debug_sexpr_tree_vec_getref(&sexpr->tree, (re_size)ref);
}

RE_INTERNAL void re__debug_sexpr_destroy(re__debug_sexpr* sexpr) {
    re_int32 i;
    for (i = 0; i < (re_int32)re__debug_sexpr_tree_vec_size(&sexpr->tree); i++) {
        re__str_destroy(&re__debug_sexpr_get(sexpr, i)->atom);
    }
    re__debug_sexpr_tree_vec_destroy(&sexpr->tree);
}

RE_INTERNAL re_error re__debug_sexpr_new(re__debug_sexpr* sexpr, re__debug_sexpr_tree tree, re_int32* new_ref) {
    re_error err = RE_ERROR_NONE;
    re_int32 next_ref = (re_int32)re__debug_sexpr_tree_vec_size(&sexpr->tree);
    if ((err = re__debug_sexpr_tree_vec_push(&sexpr->tree, tree))) {
        return err;
    }
    *new_ref = next_ref;
    return err;
}

RE_INTERNAL int re__debug_sexpr_isblank(re_char ch) {
    return (ch == '\n') || (ch == '\t') || (ch == '\r') || (ch == ' ');
}

enum re__debug_sexpr_parse_state {
    RE__DEBUG_SEXPR_PARSE_STATE_NODE,
    RE__DEBUG_SEXPR_PARSE_STATE_ATOM
};

typedef struct re__debug_sexpr_parse_frame {
    re_int32 parent_ref;
    re_int32 child_ref;
} re__debug_sexpr_parse_frame;

RE_VEC_DECL(re__debug_sexpr_parse_frame);
RE_VEC_IMPL_FUNC(re__debug_sexpr_parse_frame, init)
RE_VEC_IMPL_FUNC(re__debug_sexpr_parse_frame, destroy)
RE_VEC_IMPL_FUNC(re__debug_sexpr_parse_frame, push)
RE_VEC_IMPL_FUNC(re__debug_sexpr_parse_frame, pop)
RE_VEC_IMPL_FUNC(re__debug_sexpr_parse_frame, getref)

typedef struct re__debug_sexpr_parse {
    re__debug_sexpr_parse_frame_vec sexpr_stack;
    re_size sexpr_stack_ptr;
    re__str atom_str;
} re__debug_sexpr_parse;

RE_INTERNAL void re__debug_sexpr_parse_push_tree(re__debug_sexpr* sexpr, re__debug_sexpr_parse* parse, re_int32 new_ref) {
    re__debug_sexpr_parse_frame* frame;
    re__debug_sexpr_tree* last_child;
    re__debug_sexpr_tree* parent;
    RE_ASSERT(parse->sexpr_stack_ptr);
    frame = re__debug_sexpr_parse_frame_vec_getref(&parse->sexpr_stack, parse->sexpr_stack_ptr - 1);
    if (frame->child_ref == RE__DEBUG_SEXPR_NONE) {
        parent = re__debug_sexpr_get(sexpr, frame->parent_ref);
        parent->first_child_ref = new_ref;
    } else {
        last_child = re__debug_sexpr_get(sexpr, frame->child_ref);
        last_child->next_sibling_ref = new_ref;
    }
    frame->child_ref = new_ref;
}

RE_INTERNAL re_error re__debug_sexpr_parse_new_node(re__debug_sexpr* sexpr, re__debug_sexpr_parse* parse) {
    re_error err = RE_ERROR_NONE;
    re__debug_sexpr_parse_frame new_frame;
    re__debug_sexpr_tree new_tree;
    re_int32 new_ref;
    re__debug_sexpr_tree_init(&new_tree, &parse->atom_str, 0);
    if ((err = re__debug_sexpr_new(sexpr, new_tree, &new_ref))) {
        return err;
    }
    new_frame.child_ref = RE__DEBUG_SEXPR_NONE;
    new_frame.parent_ref = new_ref;
    if (parse->sexpr_stack_ptr != 0) {
        re__debug_sexpr_parse_push_tree(sexpr, parse, new_ref);
    }
    if ((err = re__debug_sexpr_parse_frame_vec_push(&parse->sexpr_stack, new_frame))) {
        return err;
    }
    parse->sexpr_stack_ptr++;
    return err;
}

RE_INTERNAL re_error re__debug_sexpr_parse_new_atom(re__debug_sexpr* sexpr, re__debug_sexpr_parse* parse) {
    re_error err = RE_ERROR_NONE;
    re__debug_sexpr_tree new_tree;
    re_int32 new_ref;
    re__debug_sexpr_tree_init(&new_tree, &parse->atom_str, 1);
    re__str_destroy(&parse->atom_str);
    re__str_init(&parse->atom_str);
    if ((err = re__debug_sexpr_new(sexpr, new_tree, &new_ref))) {
        return err;
    }
    if (parse->sexpr_stack_ptr != 0) {
        re__debug_sexpr_parse_push_tree(sexpr, parse, new_ref);
    }
    return err;
}

RE_INTERNAL re_error re__debug_sexpr_parse_finish_node(re__debug_sexpr* sexpr, re__debug_sexpr_parse* parse) {
    re_error err = RE_ERROR_NONE;
    RE__UNUSED(sexpr);
    RE_ASSERT(parse->sexpr_stack_ptr);
    re__debug_sexpr_parse_frame_vec_pop(&parse->sexpr_stack);
    return err;
}

RE_INTERNAL re_error re__debug_sexpr_do_parse(re__debug_sexpr* sexpr, const re__str* in) {
    re__debug_sexpr_parse parse;
    re_size str_ptr = 0;
    re_error err;
    int state = RE__DEBUG_SEXPR_PARSE_STATE_NODE;
    re__debug_sexpr_parse_frame_vec_init(&parse.sexpr_stack);
    parse.sexpr_stack_ptr = 0;
    re__str_init(&parse.atom_str);
    while (str_ptr != re__str_size(in)) {
        re_char ch = re__str_get_data(in)[str_ptr];
        if (state == RE__DEBUG_SEXPR_PARSE_STATE_NODE) {
            if (re__debug_sexpr_isblank(ch)) {
                state = RE__DEBUG_SEXPR_PARSE_STATE_NODE;
            } else if (ch == '(') {
                if ((err = re__debug_sexpr_parse_new_node(sexpr, &parse))) {
                    goto error;
                }
                state = RE__DEBUG_SEXPR_PARSE_STATE_NODE;
            } else if (ch == ')') {
                if ((err = re__debug_sexpr_parse_finish_node(sexpr, &parse))) {
                    goto error;
                }
                state = RE__DEBUG_SEXPR_PARSE_STATE_NODE;
            } else {
                if ((err = re__str_cat_n(&parse.atom_str, 1, &ch))) {
                    goto error;
                }
                state = RE__DEBUG_SEXPR_PARSE_STATE_ATOM;
            }
        } else if (state == RE__DEBUG_SEXPR_PARSE_STATE_ATOM) {
            if (re__debug_sexpr_isblank(ch)) {
                if ((err = re__debug_sexpr_parse_new_atom(sexpr, &parse))) {
                    goto error;
                }
                state = RE__DEBUG_SEXPR_PARSE_STATE_NODE;
            } else if (ch == '(') {
                if ((err = re__debug_sexpr_parse_new_atom(sexpr, &parse))) {
                    goto error;
                }
                if ((err = re__debug_sexpr_parse_new_node(sexpr, &parse))) {
                    goto error;
                }
                state = RE__DEBUG_SEXPR_PARSE_STATE_NODE;
            } else if (ch == ')') {
                if ((err = re__debug_sexpr_parse_new_atom(sexpr, &parse))) {
                    goto error;
                }
                if ((err = re__debug_sexpr_parse_finish_node(sexpr, &parse))) {
                    goto error;
                }
                state = RE__DEBUG_SEXPR_PARSE_STATE_NODE;
            } else {
                if ((err = re__str_cat_n(&parse.atom_str, 1, &ch))) {
                    goto error;
                }
                state = RE__DEBUG_SEXPR_PARSE_STATE_ATOM;
            }
        }
        str_ptr++;
    }
error:
    re__str_destroy(&parse.atom_str);
    re__debug_sexpr_parse_frame_vec_destroy(&parse.sexpr_stack);
    return err;
}

RE_INTERNAL void re__debug_sexpr_dump(re__debug_sexpr* sexpr, re_int32 parent_ref, re_int32 indent) {
    re__debug_sexpr_tree* tree;
    re_int32 child_ref;
    re_int32 i;
    if (parent_ref == RE__DEBUG_SEXPR_NONE) {
        return;
    }
    tree = re__debug_sexpr_get(sexpr, parent_ref);
    if (tree->first_child_ref == RE__DEBUG_SEXPR_NONE) {
        if (tree->is_atom) {
            printf("%s", re__str_get_data(&tree->atom));
        } else {
            printf("()");
        }
    } else {
        printf("\n");
        for (i = 0; i < indent; i++) {
            printf("  ");
        }
        printf("(");
        child_ref = tree->first_child_ref;
        while (child_ref != RE__DEBUG_SEXPR_NONE) {
            re__debug_sexpr_tree* child = re__debug_sexpr_get(sexpr, child_ref);
            re__debug_sexpr_dump(sexpr, child_ref, indent+1);
            child_ref = child->next_sibling_ref;
            if (child_ref != RE__DEBUG_SEXPR_NONE) {
                printf(" ");
            }
        }
        printf(")");
    }
}

RE_INTERNAL int re__debug_sexpr_equals(re__debug_sexpr* sexpr, re__debug_sexpr* other, re_int32 sexpr_ref, re_int32 other_ref) {
    re__debug_sexpr_tree* parent_tree;
    re__debug_sexpr_tree* other_tree;
    if ((sexpr_ref == other_ref) && sexpr_ref == RE__DEBUG_SEXPR_NONE) {
        return 1;
    } else if (sexpr_ref == RE__DEBUG_SEXPR_NONE || other_ref == RE__DEBUG_SEXPR_NONE) {
        return 0;
    }
    parent_tree = re__debug_sexpr_get(sexpr, sexpr_ref);
    other_tree = re__debug_sexpr_get(other, other_ref);
    if (re__str_cmp(&parent_tree->atom, &other_tree->atom) != 0) {
        return 1;
    }
    {
        re_int32 parent_child_ref = parent_tree->first_child_ref;
        re_int32 other_child_ref = other_tree->first_child_ref;
        while (parent_child_ref != RE__DEBUG_SEXPR_NONE &&
            other_child_ref != RE__DEBUG_SEXPR_NONE) {
            if (!re__debug_sexpr_equals(sexpr, other, parent_child_ref, other_child_ref)) {
                return 0;
            }
        }
        return parent_child_ref != other_child_ref;
    }
}

RE_INTERNAL re_int32 re__debug_sexpr_new_tree(re__debug_sexpr* sexpr, re_int32 parent_ref, re__debug_sexpr_tree new_tree) {
    re_int32 new_ref;
    re__debug_sexpr_new(sexpr, new_tree, &new_ref);
    if (parent_ref == RE__DEBUG_SEXPR_NONE) {
        return new_ref;
    } else {
        re__debug_sexpr_tree* parent = re__debug_sexpr_get(sexpr, parent_ref);
        if (parent->first_child_ref == RE__DEBUG_SEXPR_NONE) {
            parent->first_child_ref = new_ref;
            return new_ref;
        } else {
            re_int32 child_ref = parent->first_child_ref;
            re__debug_sexpr_tree* child_tree;
            do {
                child_tree = re__debug_sexpr_get(sexpr, child_ref);
                child_ref = child_tree->next_sibling_ref;
            } while (child_ref != RE__DEBUG_SEXPR_NONE);
            child_tree->next_sibling_ref = new_ref;
            return new_ref;
        }
    }
}

RE_INTERNAL re_int32 re__debug_sexpr_new_node(re__debug_sexpr* sexpr, re_int32 parent_ref) {
    re__debug_sexpr_tree new_tree;
    re__str mt_str;
    re__str_init(&mt_str);
    re__debug_sexpr_tree_init(&new_tree, &mt_str, 0);
    re__str_destroy(&mt_str);
    return re__debug_sexpr_new_tree(sexpr, parent_ref, new_tree);
}

RE_INTERNAL void re__debug_sexpr_new_atom(re__debug_sexpr* sexpr, re_int32 parent_ref, const char* name) {
    re__debug_sexpr_tree new_tree;
    re__str str;
    re__str_init_s(&str, (const re_char*)name);
    re__debug_sexpr_tree_init(&new_tree, &str, 1);
    re__str_destroy(&str);
    re__debug_sexpr_new_tree(sexpr, parent_ref, new_tree);
}

RE_INTERNAL void re__debug_sexpr_new_int(re__debug_sexpr* sexpr, re_int32 parent_ref, re_int32 num) {
    re__debug_sexpr_tree new_tree;
    re_char sbuf[64];
    re__str str;
    re__str_init_n(&str, (re_size)sprintf((char*)sbuf, "%i", num), sbuf);
    re__debug_sexpr_tree_init(&new_tree, &str, 1);
    re__str_destroy(&str);
    re__debug_sexpr_new_tree(sexpr, parent_ref, new_tree);
}

#endif /* #if 0 */



/* Maximum size, without null terminator */
#define RE__STR_SHORT_SIZE_MAX (((sizeof(re__str) - sizeof(re_size)) / (sizeof(re_char)) - 1))

#define RE__STR_GET_SHORT(str) !((str)->_size_short & 1)
#define RE__STR_SET_SHORT(str, short) \
    do { \
        re_size temp = short; \
        (str)->_size_short &= ~((re_size)1); \
        (str)->_size_short |= !temp; \
    } while (0)
#define RE__STR_GET_SIZE(str) ((str)->_size_short >> 1)
#define RE__STR_SET_SIZE(str, size) \
    do { \
        re_size temp = size; \
        (str)->_size_short &= 1; \
        (str)->_size_short |= temp << 1; \
    } while (0)
#define RE__STR_DATA(str) (RE__STR_GET_SHORT(str) ? ((re_char*)&((str)->_alloc)) : (str)->_data)

/* Round up to multiple of 32 */
#define RE__STR_ROUND_ALLOC(alloc) \
    (((alloc + 1) + 32) & (~((re_size)32)))

#if RE_DEBUG

#define RE__STR_CHECK(str) \
    do { \
        if (RE__STR_GET_SHORT(str)) { \
            /* If string is short, the size must always be less than */ \
            /* RE__STR_SHORT_SIZE_MAX. */ \
            RE_ASSERT(RE__STR_GET_SIZE(str) <= RE__STR_SHORT_SIZE_MAX); \
        } else { \
            /* If string is long, the size can still be less, but the other */ \
            /* fields must be valid. */ \
            /* Ensure there is enough space */ \
            RE_ASSERT((str)->_alloc >= RE__STR_GET_SIZE(str)); \
            /* Ensure that the _data field isn't NULL if the size is 0 */ \
            if (RE__STR_GET_SIZE(str) > 0) { \
                RE_ASSERT((str)->_data != RE_NULL); \
            } \
            /* Ensure that if _alloc is 0 then _data is NULL */ \
            if ((str)->_alloc == 0) { \
                RE_ASSERT((str)->_data == RE_NULL); \
            } \
        } \
        /* Ensure that there is a null-terminator */ \
        RE_ASSERT(RE__STR_DATA(str)[RE__STR_GET_SIZE(str)] == '\0'); \
    } while (0)

#else

#define RE__STR_CHECK(str) RE_UNUSED(str)

#endif /* #if RE_DEBUG */

void re__str_init(re__str* str) {
    str->_size_short = 0;
    RE__STR_DATA(str)[0] = '\0';
}

void re__str_destroy(re__str* str) {
    if (!RE__STR_GET_SHORT(str)) {
        if (str->_data != RE_NULL) {
            RE_FREE(str->_data);
        }
    }
}

re_size re__str_size(const re__str* str) {
    return RE__STR_GET_SIZE(str);
}

RE_INTERNAL int re__str_grow(re__str* str, re_size new_size) {
    re_size old_size = RE__STR_GET_SIZE(str);
    RE__STR_CHECK(str);
    if (RE__STR_GET_SHORT(str)) {
        if (new_size <= RE__STR_SHORT_SIZE_MAX) {
            /* Can still be a short str */
            RE__STR_SET_SIZE(str, new_size);
        } else {
            /* Needs allocation */
            re_size new_alloc = 
                RE__STR_ROUND_ALLOC(new_size + (new_size >> 1));
            re_char* new_data = (re_char*)RE_MALLOC(sizeof(re_char) * (new_alloc + 1));
            re_size i;
            if (new_data == RE_NULL) {
                return 1;
            }
            /* Copy data from old string */
            for (i = 0; i < old_size; i++) {
                new_data[i] = RE__STR_DATA(str)[i];
            }
            /* Fill in the remaining fields */
            RE__STR_SET_SHORT(str, 0);
            RE__STR_SET_SIZE(str, new_size);
            str->_data = new_data;
            str->_alloc = new_alloc;
        }
    } else {
        if (new_size > str->_alloc) {
            /* Needs allocation */
            re_size new_alloc = 
                RE__STR_ROUND_ALLOC(new_size + (new_size >> 1));
            re_char* new_data;
            if (str->_alloc == 0) {
                new_data = \
                    (re_char*)RE_MALLOC(sizeof(re_char) * (new_alloc + 1));
            } else {
                new_data = \
                    (re_char*)RE_REALLOC(
                        str->_data, sizeof(re_char) * (new_alloc + 1));
            }
            if (new_data == RE_NULL) {
                return 1;
            }
            str->_data = new_data;
            str->_alloc = new_alloc;
        }
        RE__STR_SET_SIZE(str, new_size);
    }
    /* Null terminate */
    RE__STR_DATA(str)[RE__STR_GET_SIZE(str)] = '\0';
    RE__STR_CHECK(str);
    return 0;
}

int re__str_push(re__str* str, re_char chr) {
    int err = 0;
    re_size old_size = RE__STR_GET_SIZE(str);
    if ((err = re__str_grow(str, old_size + 1))) {
        return err;
    }
    RE__STR_DATA(str)[old_size] = chr;
    RE__STR_CHECK(str);
    return err;
}

re_size re__str_slen(const re_char* s) {
    re_size out = 0;
    while (*(s++)) {
        out++;
    }
    return out;
}

int re__str_init_s(re__str* str, const re_char* s) {
    int err = 0;
    re_size i;
    re_size sz = re__str_slen(s);
    re__str_init(str);
    if ((err = re__str_grow(str, sz))) {
        return err;
    }
    for (i = 0; i < sz; i++) {
        RE__STR_DATA(str)[i] = s[i];
    }
    return err;
}

int re__str_init_n(re__str* str, re_size n, const re_char* chrs) {
    int err = 0;
    re_size i;
    re__str_init(str);
    if ((err = re__str_grow(str, n))) {
        return err;
    }
    for (i = 0; i < n; i++) {
        RE__STR_DATA(str)[i] = chrs[i];
    }
    return err;
}

void re__str_init_const_s(re__str* str, re_size n, const re_char* s) {
    re__str_init(str);
    RE__STR_SET_SHORT(str, 0);
    RE__STR_SET_SIZE(str, n);
    str->_alloc = 0;
    /* bad!!!!!! */
    str->_data = (re_char*)s;
}

int re__str_cat(re__str* str, const re__str* other) {
    int err = 0;
    re_size i;
    re_size n = RE__STR_GET_SIZE(other);
    re_size old_size = RE__STR_GET_SIZE(str);
    if ((err = re__str_grow(str, old_size + n))) {
        return err;
    }
    /* Copy data */
    for (i = 0; i < n; i++) {
        RE__STR_DATA(str)[old_size + i] = RE__STR_DATA(other)[i];
    }
    RE__STR_CHECK(str);
    return err;
}

int re__str_cat_n(re__str* str, re_size n, const re_char* chrs) {
    int err = 0;
    re_size i;
    re_size old_size = RE__STR_GET_SIZE(str);
    if ((err = re__str_grow(str, old_size + n))) {
        return err;
    }
    /* Copy data */
    for (i = 0; i < n; i++) {
        RE__STR_DATA(str)[old_size + i] = chrs[i];
    }
    RE__STR_CHECK(str);
    return err;
}

int re__str_cat_s(re__str* str, const re_char* chrs) {
    re_size chrs_size = re__str_slen(chrs);
    return re__str_cat_n(str, chrs_size, chrs);
}

int re__str_insert(re__str* str, re_size index, re_char chr) {
    int err = 0;
    re_size i;
    re_size old_size = RE__STR_GET_SIZE(str);
    /* bounds check */
    RE_ASSERT(index <= RE__STR_GET_SIZE(str));
    if ((err = re__str_grow(str, old_size + 1))) {
        return err;
    }
    /* Shift data */
    if (old_size != 0) {
        for (i = old_size; i >= index + 1; i--) {
            RE__STR_DATA(str)[i] = RE__STR_DATA(str)[i - 1];
        }
    }
    RE__STR_DATA(str)[index] = chr;
    RE__STR_CHECK(str);
    return err;
}

const re_char* re__str_get_data(const re__str* str) {
    return RE__STR_DATA(str);
}

int re__str_init_copy(re__str* str, const re__str* in) {
    re_size i;
    int err = 0;
    re__str_init(str);
    if ((err = re__str_grow(str, re__str_size(in)))) {
        return err;
    }
    for (i = 0; i < re__str_size(str); i++) {
        RE__STR_DATA(str)[i] = RE__STR_DATA(in)[i];
    }
    return err;
}

void re__str_init_move(re__str* str, re__str* old) {
    RE__STR_CHECK(old);
    *str = *old;
    re__str_init(old);
}

int re__str_cmp(const re__str* str_a, const re__str* str_b) {
    re_size a_len = re__str_size(str_a);
    re_size b_len = re__str_size(str_b);
    const re_char* a_data = re__str_get_data(str_a);
    const re_char* b_data = re__str_get_data(str_b);
    re_size i;
    if (a_len < b_len) {
        return -1;
    } else if (a_len > b_len) {
        return 1;
    }
    for (i = 0; i < a_len; i++) {
        if (a_data[i] != b_data[i]) {
            if (a_data[i] < b_data[i]) {
                return -1;
            } else {
                return 1;
            }
        }
    }
    return 0;
}


int re__uint8_check[sizeof(re_uint8)==1];
int re__int8_check[sizeof(re_int8)==1];
int re__uint16_check[sizeof(re_uint16)==2];
int re__int16_check[sizeof(re_int16)==2];
int re__uint32_check[sizeof(re_uint32)==4];
int re__int32_check[sizeof(re_int32)==4];

RE_INTERNAL re_uint32 re__murmurhash3_rotl32(re_uint32 x, re_uint8 r) {
    return (x << r) | (x >> (32 - r));
}

RE_INTERNAL re_uint32 re__murmurhash3_fmix32(re_uint32 h) {
    h ^= h >> 16;
    h *= 0x85EBCA6B;
    h ^= h >> 13;
    h *= 0xC2B2AE35;
    h ^= h >> 16;
    return h;
}

/* Note: this behaves differently on machines with different endians. */
RE_INTERNAL re_uint32 re__murmurhash3_32(const re_uint8* data, re_size data_len) {
    const re_size num_blocks = data_len / 4;
    re_uint32 h1 = 0xDEADBEEF;
    const re_uint32 c1 = 0xCC9E2D51;
    const re_uint32 c2 = 0x1B873593;
    re_size i;
    const re_uint32* blocks = (const re_uint32*)data;
    const re_uint8* tail = (const re_uint8*)(data + num_blocks * 4);
    re_uint32 k1;
    for (i = 0; i < num_blocks; i++) {
        k1 = blocks[i];
        k1 *= c1;
        k1 = re__murmurhash3_rotl32(k1, 15);
        k1 *= c2;

        h1 ^= k1;
        h1 = re__murmurhash3_rotl32(h1, 13);
        h1 = h1 * 5 + 0xE6546B64;
    }

    k1 = 0;
    switch (data_len & 3) {
        case 3:
            k1 ^= ((re_uint32)(tail[2])) << 16;
        case 2:
            k1 ^= ((re_uint32)(tail[1])) << 8;
        case 1:
            k1 ^= tail[0];
            k1 *= c1;
            k1 = re__murmurhash3_rotl32(k1, 15);
            k1 *= c2;
            h1 ^= k1;
    }

    h1 ^= (re_uint32)data_len;
    h1 = re__murmurhash3_fmix32(h1);

    return h1;
}

RE_INTERNAL void re__zero_mem(re_size size, void* mem) {
    re_size i;
    for (i = 0; i < size; i++) {
        ((re_uint8*)mem)[i] = 0;
    }
}

#endif /* #ifdef RE_IMPLEMENTATION */

#endif /* #ifndef RE_H */

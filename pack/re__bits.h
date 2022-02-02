#ifndef RE_BITS_H
#define RE_BITS_H

#include "re_config.h"
#include "re__bits_api.h"

/* bit: exports */
#define RE_INTERNAL extern

/* bit: stringify */
#define RE___STRINGIFY_0(s) #s
#define RE__STRINGIFY(s) RE___STRINGIFY_0(s)

/* bit: debug */
#if RE_DEBUG
#include <stdio.h>
#define RE__ASSERT_UNREACHED() RE_ASSERT(0)
#else
#define RE__ASSERT_UNREACHED() (void)(0)
#endif

/* bit: ds_vector */
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

#endif

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

#endif

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

#endif

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
    const T* RE_VEC_IDENT(T, get_data)(RE_VEC_TYPE(T)* vec)

#define RE__VEC_IMPL_get_data(T) \
    const T* RE_VEC_IDENT(T, get_data)(RE_VEC_TYPE(T)* vec) { \
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

/* bit: char */
typedef unsigned char re_char;

/* bit: ds_string */
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
void re__str_init_const_s(re__str*, re_size n, const re_char* s);
void re__str_destroy(re__str* str);
re_size re__str_size(const re__str* str);
int re__str_cat(re__str* str, const re__str* other);
int re__str_cat_s(re__str* str, const re_char* s);
int re__str_cat_n(re__str* str, re_size n, const re_char* chrs);
int re__str_push(re__str* str, re_char chr);
int re__str_insert(re__str* str, re_size index, re_char chr);
const re_char* re__str_get_data(const re__str* str);
int re__str_cmp(const re__str* str_a, const re__str* str_b);

/* Note: trying to change this string will result in an error on debug builds. */
#define RE__STR_INIT_CONST(str, char_literal) \
    re__str_init_const_s(str, (sizeof(char_literal) / sizeof(re_char)), (const re_char*)(char_literal))

/* bit: max */
#define RE__MAX(a, b) (((a) < (b)) ? (b) : (a))

/* bit: hash/murmurhash3 */
RE_INTERNAL re_uint32 re__murmurhash3_32(const re_uint8* data, re_size data_len);

/* bit: zeromem */
RE_INTERNAL void re__zero_mem(re_size size, void* mem);

#endif

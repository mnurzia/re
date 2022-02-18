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

/* bit: tpaste */
#define RE__P2(a, b) a ## b
#define RE__P3(a, b, c) a ## b ## c
#define RE__P4(a, b, c, d) a ## b ## c ## d

/* bit: ds_vector */
#define RE_VEC_TYPE(T) \
    RE__P2(T, _vec)

#define RE_VEC_IDENT(T, name) \
    RE__P3(T, _vec_, name)

#define RE_VEC_IDENT_INTERNAL(T, name) \
    RE__P3(T, _vec__, name)

#define RE_VEC_DECL_FUNC(T, func) \
    RE__P2(RE__VEC_DECL_, func)(T)

#define RE_VEC_IMPL_FUNC(T, func) \
    RE__P2(RE__VEC_IMPL_, func)(T)

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


/* bit: char */
typedef char re_char;

/* bit: ds_string */
typedef struct re__str {
    re_size _size_short; /* does not include \0 */
    re_size _alloc; /* does not include \0 */
    re_char* _data;
} re__str;

void re__str_init(re__str* str);
int re__str_init_s(re__str* str, const re_char* s);
int re__str_init_n(re__str* str, const re_char* chrs, re_size n);
int re__str_init_copy(re__str* str, const re__str* in);
void re__str_init_move(re__str* str, re__str* old);
void re__str_destroy(re__str* str);
re_size re__str_size(const re__str* str);
int re__str_cat(re__str* str, const re__str* other);
int re__str_cat_s(re__str* str, const re_char* s);
int re__str_cat_n(re__str* str, const re_char* chrs, re_size n);
int re__str_push(re__str* str, re_char chr);
int re__str_insert(re__str* str, re_size index, re_char chr);
const re_char* re__str_get_data(const re__str* str);
int re__str_cmp(const re__str* str_a, const re__str* str_b);
re_size re__str_slen(const re_char* chars);


/* bit: ds_string_view */
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

/* bit: ds_refs */
#define RE_REF_NONE -1

#define RE_REFS_TYPE(T) \
    RE__P2(T, _refs)

#define RE__REFS_STORAGE_TYPE(T) \
    RE__P2(T, __refs_storage)

#define RE_REFS_IDENT(T, name) \
    RE__P3(T, _refs_, name)

#define RE_REFS_IDENT_INTERNAL(T, name) \
    RE__P3(T, _refs__, name)

#define RE_REFS_DECL_FUNC(T, func) \
    RE__P2(RE__REFS_DECL_, func)(T)

#define RE_REFS_IMPL_FUNC(T, func) \
    RE__P2(RE__REFS_IMPL_, func)(T)

#define RE_REFS_DECL(T) \
    typedef struct RE__REFS_STORAGE_TYPE(T) { \
        T _elem; \
        re_int32 _next_ref; \
        re_int32 _prev_ref; \
    } RE__REFS_STORAGE_TYPE(T); \
    RE_VEC_DECL(RE__REFS_STORAGE_TYPE(T)); \
    typedef struct RE_REFS_TYPE(T) { \
        RE_VEC_TYPE(RE__REFS_STORAGE_TYPE(T)) _vec; \
        re_int32 _last_empty_ref; \
        re_int32 _first_ref; \
        re_int32 _last_ref; \
    } RE_REFS_TYPE(T)

#define RE__REFS_DECL_init(T) \
    void RE_REFS_IDENT(T, init)(RE_REFS_TYPE(T)* refs);

#define RE__REFS_IMPL_init(T) \
    RE_VEC_IMPL_FUNC(RE__REFS_STORAGE_TYPE(T), init) \
    RE_VEC_IMPL_FUNC(RE__REFS_STORAGE_TYPE(T), destroy) \
    RE_VEC_IMPL_FUNC(RE__REFS_STORAGE_TYPE(T), getref) \
    RE_VEC_IMPL_FUNC(RE__REFS_STORAGE_TYPE(T), getcref) \
    RE_VEC_IMPL_FUNC(RE__REFS_STORAGE_TYPE(T), push) \
    RE_VEC_IMPL_FUNC(RE__REFS_STORAGE_TYPE(T), size) \
    void RE_REFS_IDENT(T, init)(RE_REFS_TYPE(T)* refs) { \
        RE_VEC_IDENT(RE__REFS_STORAGE_TYPE(T), init)(&refs->_vec); \
        refs->_last_empty_ref = RE_REF_NONE; \
        refs->_first_ref = RE_REF_NONE; \
        refs->_last_ref = RE_REF_NONE; \
    }

#define RE__REFS_DECL_destroy(T) \
    void RE_REFS_IDENT(T, destroy)(RE_REFS_TYPE(T)* refs);

#define RE__REFS_IMPL_destroy(T) \
    void RE_REFS_IDENT(T, destroy)(RE_REFS_TYPE(T)* refs) { \
        RE_VEC_IDENT(RE__REFS_STORAGE_TYPE(T), destroy)(&refs->_vec); \
    }

#define RE__REFS_DECL_getref(T) \
    T* RE_REFS_IDENT(T, getref)(RE_REFS_TYPE(T)* refs, re_int32 elem_ref);

#define RE__REFS_IMPL_getref(T) \
    T* RE_REFS_IDENT(T, getref)(RE_REFS_TYPE(T)* refs, re_int32 elem_ref) {\
        RE_ASSERT(elem_ref != RE_REF_NONE); \
        return &RE_VEC_IDENT(RE__REFS_STORAGE_TYPE(T), getref)(&refs->_vec, (re_size)elem_ref)->_elem; \
    }

#define RE__REFS_DECL_getcref(T) \
    const T* RE_REFS_IDENT(T, getcref)(const RE_REFS_TYPE(T)* refs, re_int32 elem_ref);

#define RE__REFS_IMPL_getcref(T) \
    const T* RE_REFS_IDENT(T, getcref)(const RE_REFS_TYPE(T)* refs, re_int32 elem_ref) {\
        RE_ASSERT(elem_ref != RE_REF_NONE); \
        return &RE_VEC_IDENT(RE__REFS_STORAGE_TYPE(T), getcref)(&refs->_vec, (re_size)elem_ref)->_elem; \
    }

#define RE__REFS_DECL_add(T) \
    int RE_REFS_IDENT(T, add)(RE_REFS_TYPE(T)* refs, T elem, re_int32* out_ref)

#define RE__REFS_IMPL_add(T) \
    int RE_REFS_IDENT(T, add)(RE_REFS_TYPE(T)* refs, T elem, re_int32* out_ref) { \
        int err = 0; \
        re_int32 empty_ref = refs->_last_empty_ref; \
        RE__REFS_STORAGE_TYPE(T)* empty; \
        RE__REFS_STORAGE_TYPE(T)* prev_elem_ptr; \
        if (empty_ref != RE_REF_NONE) { \
            empty = RE_VEC_IDENT( \
                RE__REFS_STORAGE_TYPE(T), getref)( \
                    &refs->_vec, (re_size)empty_ref); \
            refs->_last_empty_ref = empty->_next_ref; \
            *out_ref = empty_ref; \
            empty->_elem = elem; \
        } else { \
            RE__REFS_STORAGE_TYPE(T) new_elem; \
            *out_ref = (re_int32)RE_VEC_IDENT( \
                RE__REFS_STORAGE_TYPE(T), size)(&refs->_vec); \
            new_elem._elem = elem; \
            if ((err = RE_VEC_IDENT( \
                RE__REFS_STORAGE_TYPE(T), push)(&refs->_vec, new_elem))) { \
                return err; \
            } \
            empty = RE_VEC_IDENT( \
                RE__REFS_STORAGE_TYPE(T), getref)( \
                    &refs->_vec, (re_size)(*out_ref)); \
        } \
        empty->_next_ref = RE_REF_NONE; \
        empty->_prev_ref = refs->_last_ref; \
        if (refs->_last_ref != RE_REF_NONE) { \
            prev_elem_ptr =  \
                RE_VEC_IDENT( \
                    RE__REFS_STORAGE_TYPE(T), getref)( \
                        &refs->_vec, (re_size)refs->_last_ref); \
            prev_elem_ptr->_next_ref = *out_ref; \
        } \
        if (refs->_first_ref == RE_REF_NONE) { \
            refs->_first_ref = *out_ref; \
        } \
        refs->_last_ref = *out_ref; \
        return err; \
    }

#define RE__REFS_DECL_begin(T) \
    re_int32 RE_REFS_IDENT(T, begin)(RE_REFS_TYPE(T)* refs);

#define RE__REFS_IMPL_begin(T) \
    re_int32 RE_REFS_IDENT(T, begin)(RE_REFS_TYPE(T)* refs) { \
        return refs->_first_ref; \
    }

#define RE__REFS_DECL_next(T) \
    re_int32 RE_REFS_IDENT(T, next)(RE_REFS_TYPE(T)* refs);

#define RE__REFS_IMPL_next(T) \
    re_int32 RE_REFS_IDENT(T, next)(RE_REFS_TYPE(T)* refs, re_int32 prev_ref) { \
        const RE__REFS_STORAGE_TYPE(T)* elem_storage; \
        RE_ASSERT(prev_ref != RE_REF_NONE); \
        elem_storage = RE_VEC_IDENT(RE__REFS_STORAGE_TYPE(T), getcref)(&refs->_vec, (re_size)prev_ref); \
        return elem_storage->_next_ref; \
    }

/* bit: max */
#define RE__MAX(a, b) (((a) < (b)) ? (b) : (a))

/* bit: hash/murmurhash3 */
RE_INTERNAL re_uint32 re__murmurhash3_32(const re_uint8* data, re_size data_len);

/* bit: zeromem */
RE_INTERNAL void re__zero_mem(re_size size, void* mem);

#endif

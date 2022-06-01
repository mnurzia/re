#if !defined(MN__RE_INTERNAL_H)
#define MN__RE_INTERNAL_H

#include "../tests/test_config.h"
#include "api.h"

/* bits/math/implies */
#define MN__IMPLIES(a, b) (!(a) || b)

/* bits/math/max */
#define MN__MAX(a, b) (((a) > (b)) ? (a) : (b))

/* bits/math/min */
#define MN__MIN(a, b) (((a) < (b)) ? (a) : (b))

/* bits/util/exports */
#if !defined(MN__SPLIT_BUILD)
#define MN_INTERNAL static
#else
#define MN_INTERNAL extern
#endif

#define MN_INTERNAL_DATA static

/* bits/util/preproc/stringify */
#define MN__STRINGIFY_0(x) #x
#define MN__STRINGIFY(x) MN__STRINGIFY_0(x)

/* bits/util/preproc/token_paste */
#define MN__PASTE_0(a, b) a ## b
#define MN__PASTE(a, b) MN__PASTE_0(a, b)

/* bits/util/static_assert */
#define MN__STATIC_ASSERT(name, expr) char MN__PASTE(mn__, name)[(expr)==1]

/* bits/container/str */
typedef struct mn__str {
    mn_size _size_short; /* does not include \0 */
    mn_size _alloc; /* does not include \0 */
    mn_char* _data;
} mn__str;

void mn__str_init(mn__str* str);
int mn__str_init_s(mn__str* str, const mn_char* s);
int mn__str_init_n(mn__str* str, const mn_char* chrs, mn_size n);
int mn__str_init_copy(mn__str* str, const mn__str* in);
void mn__str_init_move(mn__str* str, mn__str* old);
void mn__str_destroy(mn__str* str);
mn_size mn__str_size(const mn__str* str);
int mn__str_cat(mn__str* str, const mn__str* other);
int mn__str_cat_s(mn__str* str, const mn_char* s);
int mn__str_cat_n(mn__str* str, const mn_char* chrs, mn_size n);
int mn__str_push(mn__str* str, mn_char chr);
int mn__str_insert(mn__str* str, mn_size index, mn_char chr);
const mn_char* mn__str_get_data(const mn__str* str);
int mn__str_cmp(const mn__str* str_a, const mn__str* str_b);
mn_size mn__str_slen(const mn_char* chars);

/* bits/container/str_view */
typedef struct mn__str_view {
    const mn_char* _data;
    mn_size _size;
} mn__str_view;

void mn__str_view_init(mn__str_view* view, const mn__str* other);
void mn__str_view_init_s(mn__str_view* view, const mn_char* chars);
void mn__str_view_init_n(mn__str_view* view, const mn_char* chars, mn_size n);
void mn__str_view_init_null(mn__str_view* view);
mn_size mn__str_view_size(const mn__str_view* view);
const mn_char* mn__str_view_get_data(const mn__str_view* view);
int mn__str_view_cmp(const mn__str_view* a, const mn__str_view* b);

/* bits/hooks/memset */
void mn__memset(void* ptr, int value, mn_size count);

/* bits/util/swap */
#define MN__SWAP(a, b, T) { T swap_temp_ = b; b = a; a = swap_temp_; }

/* bits/util/unreached */
#define MN__ASSERT_UNREACHED() MN_ASSERT(0)

/* bits/util/unused */
#define MN__UNUSED(x) ((void)(x))

/* bits/algorithm/hash/murmur3 */
mn_uint32 mn__murmurhash3_32(mn_uint32 h1, const mn_uint8* data, mn_size data_len);

/* bits/container/vec */
#define MN__VEC_TYPE(T) \
    MN__PASTE(T, _vec)

#define MN__VEC_IDENT(T, name) \
    MN__PASTE(T, MN__PASTE(_vec_, name))

#define MN__VEC_IDENT_INTERNAL(T, name) \
    MN__PASTE(T, MN__PASTE(_vec__, name))

#define MN__VEC_DECL_FUNC(T, func) \
    MN__PASTE(MN__VEC_DECL_, func)(T)

#define MN__VEC_IMPL_FUNC(T, func) \
    MN__PASTE(MN__VEC_IMPL_, func)(T)

#if MN_DEBUG

#define MN__VEC_CHECK(vec) \
    do { \
        /* ensure size is not greater than allocation size */ \
        MN_ASSERT(vec->_size <= vec->_alloc); \
        /* ensure that data is not null if size is greater than 0 */ \
        MN_ASSERT(vec->_size ? vec->_data != MN_NULL : 1); \
    } while (0)

#else

#define MN__VEC_CHECK(vec) MN__UNUSED(vec)

#endif

#define MN__VEC_DECL(T) \
    typedef struct MN__VEC_TYPE(T) { \
        mn_size _size; \
        mn_size _alloc; \
        T* _data; \
    } MN__VEC_TYPE(T)

#define MN__VEC_DECL_init(T) \
    void MN__VEC_IDENT(T, init)(MN__VEC_TYPE(T)* vec)

#define MN__VEC_IMPL_init(T) \
    void MN__VEC_IDENT(T, init)(MN__VEC_TYPE(T)* vec) { \
        vec->_size = 0; \
        vec->_alloc = 0; \
        vec->_data = MN_NULL; \
    } 

#define MN__VEC_DECL_destroy(T) \
    void MN__VEC_IDENT(T, destroy)(MN__VEC_TYPE(T)* vec)

#define MN__VEC_IMPL_destroy(T) \
    void MN__VEC_IDENT(T, destroy)(MN__VEC_TYPE(T)* vec) { \
        MN__VEC_CHECK(vec); \
        if (vec->_data != MN_NULL) { \
            MN_FREE(vec->_data); \
        } \
    }

#define MN__VEC_GROW_ONE(T, vec) \
    do { \
        vec->_size += 1; \
        if (vec->_size > vec->_alloc) { \
            if (vec->_data == MN_NULL) { \
                vec->_alloc = 1; \
                vec->_data = (T*)MN_MALLOC(sizeof(T) * vec->_alloc); \
                if (vec->_data == MN_NULL) { \
                    return -1; \
                } \
            } else { \
                vec->_alloc *= 2; \
                vec->_data = (T*)MN_REALLOC(vec->_data, sizeof(T) * vec->_alloc); \
                if (vec->_data == MN_NULL) { \
                    return -1; \
                } \
            } \
        } \
    } while (0)

#define MN__VEC_GROW(T, vec, n) \
    do { \
        vec->_size += n; \
        if (vec->_size > vec->_alloc) { \
            vec->_alloc = vec->_size + (vec->_size >> 1); \
            if (vec->_data == MN_NULL) { \
                vec->_data = (T*)MN_MALLOC(sizeof(T) * vec->_alloc); \
            } else { \
                vec->_data = (T*)MN_REALLOC(vec->_data, sizeof(T) * vec->_alloc); \
            } \
            if (vec->_data == MN_NULL) { \
                return -1; \
            } \
        } \
    } while (0)

#define MN__VEC_SETSIZE(T, vec, n) \
    do { \
        if (vec->_alloc < n) { \
            vec->_alloc = n; \
            if (vec->_data == MN_NULL) { \
                vec->_data = (T*)MN_MALLOC(sizeof(T) * vec->_alloc); \
            } else { \
                vec->_data = (T*)MN_REALLOC(vec->_data, sizeof(T) * vec->_alloc); \
            } \
            if (vec->_data == MN_NULL) { \
                return -1; \
            } \
        } \
    } while (0)

#define MN__VEC_DECL_push(T) \
    int MN__VEC_IDENT(T, push)(MN__VEC_TYPE(T)* vec, T elem)

#define MN__VEC_IMPL_push(T) \
    int MN__VEC_IDENT(T, push)(MN__VEC_TYPE(T)* vec, T elem) { \
        MN__VEC_CHECK(vec); \
        MN__VEC_GROW_ONE(T, vec); \
        vec->_data[vec->_size - 1] = elem; \
        MN__VEC_CHECK(vec); \
        return 0; \
    }

#if MN_DEBUG

#define MN__VEC_CHECK_POP(vec) \
    do { \
        /* ensure that there is an element to pop */ \
        MN_ASSERT(vec->_size > 0); \
    } while (0)

#else

#define MN__VEC_CHECK_POP(vec) MN__UNUSED(vec)

#endif

#define MN__VEC_DECL_pop(T) \
    T MN__VEC_IDENT(T, pop)(MN__VEC_TYPE(T)* vec)

#define MN__VEC_IMPL_pop(T) \
    T MN__VEC_IDENT(T, pop)(MN__VEC_TYPE(T)* vec) { \
        MN__VEC_CHECK(vec); \
        MN__VEC_CHECK_POP(vec); \
        return vec->_data[--vec->_size]; \
    }

#define MN__VEC_DECL_cat(T) \
    T MN__VEC_IDENT(T, cat)(MN__VEC_TYPE(T)* vec, MN__VEC_TYPE(T)* other)

#define MN__VEC_IMPL_cat(T) \
    int MN__VEC_IDENT(T, cat)(MN__VEC_TYPE(T)* vec, MN__VEC_TYPE(T)* other) { \
        re_size i; \
        re_size old_size = vec->_size; \
        MN__VEC_CHECK(vec); \
        MN__VEC_CHECK(other); \
        MN__VEC_GROW(T, vec, other->_size); \
        for (i = 0; i < other->_size; i++) { \
            vec->_data[old_size + i] = other->_data[i]; \
        } \
        MN__VEC_CHECK(vec); \
        return 0; \
    }

#define MN__VEC_DECL_insert(T) \
    int MN__VEC_IDENT(T, insert)(MN__VEC_TYPE(T)* vec, mn_size index, T elem)

#define MN__VEC_IMPL_insert(T) \
    int MN__VEC_IDENT(T, insert)(MN__VEC_TYPE(T)* vec, mn_size index, T elem) { \
        mn_size i; \
        mn_size old_size = vec->_size; \
        MN__VEC_CHECK(vec); \
        MN__VEC_GROW_ONE(T, vec); \
        if (old_size != 0) { \
            for (i = old_size; i >= index + 1; i--) { \
                vec->_data[i] = vec->_data[i - 1]; \
            } \
        } \
        vec->_data[index] = elem; \
        return 0; \
    }

#define MN__VEC_DECL_peek(T) \
    T MN__VEC_IDENT(T, peek)(const MN__VEC_TYPE(T)* vec)

#define MN__VEC_IMPL_peek(T) \
    T MN__VEC_IDENT(T, peek)(const MN__VEC_TYPE(T)* vec) { \
        MN__VEC_CHECK(vec); \
        MN__VEC_CHECK_POP(vec); \
        return vec->_data[vec->_size - 1]; \
    }

#define MN__VEC_DECL_clear(T) \
    void MN__VEC_IDENT(T, clear)(MN__VEC_TYPE(T)* vec)

#define MN__VEC_IMPL_clear(T) \
    void MN__VEC_IDENT(T, clear)(MN__VEC_TYPE(T)* vec) { \
        MN__VEC_CHECK(vec); \
        vec->_size = 0; \
    }

#define MN__VEC_DECL_size(T) \
    mn_size MN__VEC_IDENT(T, size)(const MN__VEC_TYPE(T)* vec)

#define MN__VEC_IMPL_size(T) \
    mn_size MN__VEC_IDENT(T, size)(const MN__VEC_TYPE(T)* vec) { \
        return vec->_size; \
    }

#if MN_DEBUG

#define MN__VEC_CHECK_BOUNDS(vec, idx) \
    do { \
        /* ensure that idx is within bounds */ \
        MN_ASSERT(idx < vec->_size); \
    } while (0)

#else

#define MN__VEC_CHECK_BOUNDS(vec, idx) \
    do { \
        MN__UNUSED(vec); \
        MN__UNUSED(idx); \
    } while (0) 

#endif

#define MN__VEC_DECL_get(T) \
    T MN__VEC_IDENT(T, get)(const MN__VEC_TYPE(T)* vec, mn_size idx)

#define MN__VEC_IMPL_get(T) \
    T MN__VEC_IDENT(T, get)(const MN__VEC_TYPE(T)* vec, mn_size idx) { \
        MN__VEC_CHECK(vec); \
        MN__VEC_CHECK_BOUNDS(vec, idx); \
        return vec->_data[idx]; \
    }

#define MN__VEC_DECL_getref(T) \
    T* MN__VEC_IDENT(T, getref)(MN__VEC_TYPE(T)* vec, mn_size idx)

#define MN__VEC_IMPL_getref(T) \
    T* MN__VEC_IDENT(T, getref)(MN__VEC_TYPE(T)* vec, mn_size idx) { \
        MN__VEC_CHECK(vec); \
        MN__VEC_CHECK_BOUNDS(vec, idx); \
        return &vec->_data[idx]; \
    }

#define MN__VEC_DECL_getcref(T) \
    const T* MN__VEC_IDENT(T, getcref)(const MN__VEC_TYPE(T)* vec, mn_size idx)

#define MN__VEC_IMPL_getcref(T) \
    const T* MN__VEC_IDENT(T, getcref)(const MN__VEC_TYPE(T)* vec, mn_size idx) { \
        MN__VEC_CHECK(vec); \
        MN__VEC_CHECK_BOUNDS(vec, idx); \
        return &vec->_data[idx]; \
    }

#define MN__VEC_DECL_set(T) \
    void MN__VEC_IDENT(T, set)(MN__VEC_TYPE(T)* vec, mn_size idx, T elem)

#define MN__VEC_IMPL_set(T) \
    void MN__VEC_IDENT(T, set)(MN__VEC_TYPE(T)* vec, mn_size idx, T elem) { \
        MN__VEC_CHECK(vec); \
        MN__VEC_CHECK_BOUNDS(vec, idx); \
        vec->_data[idx] = elem; \
    }

#define MN__VEC_DECL_capacity(T) \
    mn_size MN__VEC_IDENT(T, capacity)(MN__VEC_TYPE(T)* vec)

#define MN__VEC_IMPL_capacity(T) \
    mn_size MN__VEC_IDENT(T, capacity)(MN__VEC_TYPE(T)* vec) { \
        return vec->_alloc; \
    }

#define MN__VEC_DECL_get_data(T) \
    const T* MN__VEC_IDENT(T, get_data)(const MN__VEC_TYPE(T)* vec)

#define MN__VEC_IMPL_get_data(T) \
    const T* MN__VEC_IDENT(T, get_data)(const MN__VEC_TYPE(T)* vec) { \
        return vec->_data; \
    }

#define MN__VEC_DECL_move(T) \
    void MN__VEC_IDENT(T, move)(MN__VEC_TYPE(T)* vec, MN__VEC_TYPE(T)* old);

#define MN__VEC_IMPL_move(T) \
    void MN__VEC_IDENT(T, move)(MN__VEC_TYPE(T)* vec, MN__VEC_TYPE(T)* old) { \
        MN__VEC_CHECK(old); \
        *vec = *old; \
        MN__VEC_IDENT(T, init)(old); \
    }

#define MN__VEC_DECL_reserve(T) \
    int MN__VEC_IDENT(T, reserve)(MN__VEC_TYPE(T)* vec, mn_size cap);

#define MN__VEC_IMPL_reserve(T) \
    int MN__VEC_IDENT(T, reserve)(MN__VEC_TYPE(T)* vec, mn_size cap) { \
        MN__VEC_CHECK(vec); \
        MN__VEC_SETSIZE(T, vec, cap); \
        return 0; \
    }

/* bits/container/arena */
#define MN__ARENA_REF_NONE -1

#define MN__ARENA_TYPE(T) \
    MN__PASTE(T, _arena)

#define MN__ARENA_STORAGE_TYPE(T) \
    MN__PASTE(T, __arena_storage)

#define MN__ARENA_IDENT(T, name) \
    MN__PASTE(T, MN__PASTE(_arena_, name))

#define MN__ARENA_IDENT_INTERNAL(T, name) \
    MN__PASTE(T, MN__PASTE(__arena__, name))

#define MN__ARENA_DECL_FUNC(T, func) \
    MN__PASTE(MN__ARENA_DECL_, func)(T)

#define MN__ARENA_IMPL_FUNC(T, func) \
    MN__PASTE(MN__ARENA_IMPL_, func)(T)

#define MN__ARENA_DECL(T) \
    typedef struct MN__ARENA_STORAGE_TYPE(T) { \
        T _elem; \
        mn_int32 _next_ref; \
        mn_int32 _prev_ref; \
    } MN__ARENA_STORAGE_TYPE(T); \
    MN__VEC_DECL(MN__ARENA_STORAGE_TYPE(T)); \
    typedef struct MN__ARENA_TYPE(T) { \
        MN__VEC_TYPE(MN__ARENA_STORAGE_TYPE(T)) _vec; \
        mn_int32 _last_empty_ref; \
        mn_int32 _first_ref; \
        mn_int32 _last_ref; \
    } MN__ARENA_TYPE(T)

#define MN__ARENA_DECL_init(T) \
    void MN__ARENA_IDENT(T, init)(MN__ARENA_TYPE(T)* arena);

#define MN__ARENA_IMPL_init(T) \
    MN__VEC_IMPL_FUNC(MN__ARENA_STORAGE_TYPE(T), init) \
    MN__VEC_IMPL_FUNC(MN__ARENA_STORAGE_TYPE(T), destroy) \
    MN__VEC_IMPL_FUNC(MN__ARENA_STORAGE_TYPE(T), getref) \
    MN__VEC_IMPL_FUNC(MN__ARENA_STORAGE_TYPE(T), getcref) \
    MN__VEC_IMPL_FUNC(MN__ARENA_STORAGE_TYPE(T), push) \
    MN__VEC_IMPL_FUNC(MN__ARENA_STORAGE_TYPE(T), size) \
    void MN__ARENA_IDENT(T, init)(MN__ARENA_TYPE(T)* arena) { \
        MN__VEC_IDENT(MN__ARENA_STORAGE_TYPE(T), init)(&arena->_vec); \
        arena->_last_empty_ref = MN__ARENA_REF_NONE; \
        arena->_first_ref = MN__ARENA_REF_NONE; \
        arena->_last_ref = MN__ARENA_REF_NONE; \
    }

#define MN__ARENA_DECL_destroy(T) \
    void MN__ARENA_IDENT(T, destroy)(MN__ARENA_TYPE(T)* arena);

#define MN__ARENA_IMPL_destroy(T) \
    void MN__ARENA_IDENT(T, destroy)(MN__ARENA_TYPE(T)* arena) { \
        MN__VEC_IDENT(MN__ARENA_STORAGE_TYPE(T), destroy)(&arena->_vec); \
    }

#define MN__ARENA_DECL_getref(T) \
    T* MN__ARENA_IDENT(T, getref)(MN__ARENA_TYPE(T)* arena, mn_int32 elem_ref);

#define MN__ARENA_IMPL_getref(T) \
    T* MN__ARENA_IDENT(T, getref)(MN__ARENA_TYPE(T)* arena, mn_int32 elem_ref) {\
        MN_ASSERT(elem_ref != MN__ARENA_REF_NONE); \
        return &MN__VEC_IDENT(MN__ARENA_STORAGE_TYPE(T), getref)(&arena->_vec, (mn_size)elem_ref)->_elem; \
    }

#define MN__ARENA_DECL_getcref(T) \
    const T* MN__ARENA_IDENT(T, getcref)(const MN__ARENA_TYPE(T)* arena, mn_int32 elem_ref);

#define MN__ARENA_IMPL_getcref(T) \
    const T* MN__ARENA_IDENT(T, getcref)(const MN__ARENA_TYPE(T)* arena, mn_int32 elem_ref) {\
        MN_ASSERT(elem_ref != MN__ARENA_REF_NONE); \
        return &MN__VEC_IDENT(MN__ARENA_STORAGE_TYPE(T), getcref)(&arena->_vec, (mn_size)elem_ref)->_elem; \
    }

#define MN__ARENA_DECL_add(T) \
    int MN__ARENA_IDENT(T, add)(MN__ARENA_TYPE(T)* arena, T elem, mn_int32* out_ref)

#define MN__ARENA_IMPL_add(T) \
    int MN__ARENA_IDENT(T, add)(MN__ARENA_TYPE(T)* arena, T elem, mn_int32* out_ref) { \
        int err = 0; \
        mn_int32 empty_ref = arena->_last_empty_ref; \
        MN__ARENA_STORAGE_TYPE(T)* empty; \
        MN__ARENA_STORAGE_TYPE(T)* prev_elem_ptr; \
        if (empty_ref != MN__ARENA_REF_NONE) { \
            empty = MN__VEC_IDENT( \
                MN__ARENA_STORAGE_TYPE(T), getref)( \
                    &arena->_vec, (mn_size)empty_ref); \
            arena->_last_empty_ref = empty->_next_ref; \
            *out_ref = empty_ref; \
            empty->_elem = elem; \
        } else { \
            MN__ARENA_STORAGE_TYPE(T) new_elem; \
            *out_ref = (mn_int32)MN__VEC_IDENT( \
                MN__ARENA_STORAGE_TYPE(T), size)(&arena->_vec); \
            new_elem._elem = elem; \
            if ((err = MN__VEC_IDENT( \
                MN__ARENA_STORAGE_TYPE(T), push)(&arena->_vec, new_elem))) { \
                return err; \
            } \
            empty = MN__VEC_IDENT( \
                MN__ARENA_STORAGE_TYPE(T), getref)( \
                    &arena->_vec, (mn_size)(*out_ref)); \
        } \
        empty->_next_ref = MN__ARENA_REF_NONE; \
        empty->_prev_ref = arena->_last_ref; \
        if (arena->_last_ref != MN__ARENA_REF_NONE) { \
            prev_elem_ptr =  \
                MN__VEC_IDENT( \
                    MN__ARENA_STORAGE_TYPE(T), getref)( \
                        &arena->_vec, (mn_size)arena->_last_ref); \
            prev_elem_ptr->_next_ref = *out_ref; \
        } \
        if (arena->_first_ref == MN__ARENA_REF_NONE) { \
            arena->_first_ref = *out_ref; \
        } \
        arena->_last_ref = *out_ref; \
        return err; \
    }

#define MN__ARENA_DECL_begin(T) \
    mn_int32 MN__ARENA_IDENT(T, begin)(MN__ARENA_TYPE(T)* arena);

#define MN__ARENA_IMPL_begin(T) \
    mn_int32 MN__ARENA_IDENT(T, begin)(MN__ARENA_TYPE(T)* arena) { \
        return arena->_first_ref; \
    }

#define MN__ARENA_DECL_next(T) \
    mn_int32 MN__ARENA_IDENT(T, next)(MN__ARENA_TYPE(T)* arena);

#define MN__ARENA_IMPL_next(T) \
    mn_int32 MN__ARENA_IDENT(T, next)(MN__ARENA_TYPE(T)* arena, mn_int32 prev_ref) { \
        const MN__ARENA_STORAGE_TYPE(T)* elem_storage; \
        MN_ASSERT(prev_ref != MN__ARENA_REF_NONE); \
        elem_storage = MN__VEC_IDENT(MN__ARENA_STORAGE_TYPE(T), getcref)(&arena->_vec, (mn_size)prev_ref); \
        return elem_storage->_next_ref; \
    }

#endif /* MN__RE_INTERNAL_H */

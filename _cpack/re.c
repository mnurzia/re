#include "re.h"
/* bits/math/implies */
#define RE__IMPLIES(a, b) (!(a) || b)

/* bits/math/max */
#define RE__MAX(a, b) (((a) > (b)) ? (a) : (b))

/* bits/math/min */
#define RE__MIN(a, b) (((a) < (b)) ? (a) : (b))

/* bits/util/exports */
#if !defined(RE__SPLIT_BUILD)
#define RE_INTERNAL static
#else
#define RE_INTERNAL extern
#endif

#if !defined(RE__SPLIT_BUILD)
#define RE_INTERNAL_DATA_DECL static
#define RE_INTERNAL_DATA static
#else
#define RE_INTERNAL_DATA_DECL extern
#define RE_INTERNAL_DATA 
#endif

/* bits/util/preproc/stringify */
#define RE__STRINGIFY_0(x) #x
#define RE__STRINGIFY(x) RE__STRINGIFY_0(x)

/* bits/util/preproc/token_paste */
#define RE__PASTE_0(a, b) a##b
#define RE__PASTE(a, b) RE__PASTE_0(a, b)

/* bits/util/static_assert */
#define RE__STATIC_ASSERT(name, expr) char RE__PASTE(re__, name)[(expr) == 1]

/* bits/container/str */
typedef struct re__str {
  re_size _size_short; /* does not include \0 */
  re_size _alloc;      /* does not include \0 */
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
void re__str_clear(re__str* str);

/* bits/container/str_view */
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

/* bits/hooks/memset */
void re__memset(void* ptr, int value, re_size count);

/* bits/util/swap */
#define RE__SWAP(a, b, T)                                                      \
  {                                                                            \
    T swap_temp_ = b;                                                          \
    b = a;                                                                     \
    a = swap_temp_;                                                            \
  }

/* bits/util/unreached */
#if !defined(RE__COVERAGE)

#define RE__ASSERT_UNREACHED() RE_ASSERT(0)

#else

#define RE__ASSERT_UNREACHED()

#endif

/* bits/util/unused */
#define RE__UNUSED(x) ((void)(x))

/* bits/algorithm/hash/murmur3 */
RE_INTERNAL re_uint32
re__murmurhash3_32(re_uint32 h1, const re_uint8* data, re_size data_len);

/* bits/container/vec */
#define RE__VEC_TYPE(T) RE__PASTE(T, _vec)

#define RE__VEC_IDENT(T, name) RE__PASTE(T, RE__PASTE(_vec_, name))

#define RE__VEC_IDENT_INTERNAL(T, name) RE__PASTE(T, RE__PASTE(_vec__, name))

#define RE__VEC_DECL_FUNC(T, func) RE__PASTE(RE__VEC_DECL_, func)(T)

#define RE__VEC_IMPL_FUNC(T, func) RE__PASTE(RE__VEC_IMPL_, func)(T)

#if RE_DEBUG

#define RE__VEC_CHECK(vec)                                                     \
  do {                                                                         \
    /* ensure size is not greater than allocation size */                      \
    RE_ASSERT(vec->_size <= vec->_alloc);                                      \
    /* ensure that data is not null if size is greater than 0 */               \
    RE_ASSERT(vec->_size ? vec->_data != RE_NULL : 1);                         \
  } while (0)

#else

#define RE__VEC_CHECK(vec) RE__UNUSED(vec)

#endif

#define RE__VEC_DECL(T)                                                        \
  typedef struct RE__VEC_TYPE(T) {                                             \
    re_size _size;                                                             \
    re_size _alloc;                                                            \
    T* _data;                                                                  \
  } RE__VEC_TYPE(T)

#define RE__VEC_DECL_init(T) void RE__VEC_IDENT(T, init)(RE__VEC_TYPE(T) * vec)

#define RE__VEC_IMPL_init(T)                                                   \
  void RE__VEC_IDENT(T, init)(RE__VEC_TYPE(T) * vec)                           \
  {                                                                            \
    vec->_size = 0;                                                            \
    vec->_alloc = 0;                                                           \
    vec->_data = RE_NULL;                                                      \
  }

#define RE__VEC_DECL_destroy(T)                                                \
  void RE__VEC_IDENT(T, destroy)(RE__VEC_TYPE(T) * vec)

#define RE__VEC_IMPL_destroy(T)                                                \
  void RE__VEC_IDENT(T, destroy)(RE__VEC_TYPE(T) * vec)                        \
  {                                                                            \
    RE__VEC_CHECK(vec);                                                        \
    if (vec->_data != RE_NULL) {                                               \
      RE_FREE(vec->_data);                                                     \
    }                                                                          \
  }

#define RE__VEC_GROW_ONE(T, vec)                                               \
  do {                                                                         \
    void* new_ptr;                                                             \
    re_size new_alloc;                                                         \
    if (vec->_size + 1 > vec->_alloc) {                                        \
      if (vec->_data == RE_NULL) {                                             \
        new_alloc = 1;                                                         \
        new_ptr = (T*)RE_MALLOC(sizeof(T) * new_alloc);                        \
      } else {                                                                 \
        new_alloc = vec->_alloc * 2;                                           \
        new_ptr = (T*)RE_REALLOC(vec->_data, sizeof(T) * new_alloc);           \
      }                                                                        \
      if (new_ptr == RE_NULL) {                                                \
        return -1;                                                             \
      }                                                                        \
      vec->_alloc = new_alloc;                                                 \
      vec->_data = new_ptr;                                                    \
    }                                                                          \
    vec->_size = vec->_size + 1;                                               \
  } while (0)

#define RE__VEC_GROW(T, vec, n)                                                \
  do {                                                                         \
    void* new_ptr;                                                             \
    re_size new_alloc = vec->_alloc;                                           \
    re_size new_size = vec->_size + n;                                         \
    if (new_size > new_alloc) {                                                \
      if (new_alloc == 0) {                                                    \
        new_alloc = 1;                                                         \
      }                                                                        \
      while (new_alloc < new_size) {                                           \
        new_alloc *= 2;                                                        \
      }                                                                        \
      if (vec->_data == RE_NULL) {                                             \
        new_ptr = (T*)RE_MALLOC(sizeof(T) * new_alloc);                        \
      } else {                                                                 \
        new_ptr = (T*)RE_REALLOC(vec->_data, sizeof(T) * new_alloc);           \
      }                                                                        \
      if (new_ptr == RE_NULL) {                                                \
        return -1;                                                             \
      }                                                                        \
      vec->_alloc = new_alloc;                                                 \
      vec->_data = new_ptr;                                                    \
    }                                                                          \
    vec->_size += n;                                                           \
  } while (0)

#define RE__VEC_SETSIZE(T, vec, n)                                             \
  do {                                                                         \
    void* new_ptr;                                                             \
    if (vec->_alloc < n) {                                                     \
      if (vec->_data == RE_NULL) {                                             \
        new_ptr = (T*)RE_MALLOC(sizeof(T) * n);                                \
      } else {                                                                 \
        new_ptr = (T*)RE_REALLOC(vec->_data, sizeof(T) * n);                   \
      }                                                                        \
      if (new_ptr == RE_NULL) {                                                \
        return -1;                                                             \
      }                                                                        \
      vec->_alloc = n;                                                         \
      vec->_data = new_ptr;                                                    \
    }                                                                          \
  } while (0)

#define RE__VEC_DECL_push(T)                                                   \
  int RE__VEC_IDENT(T, push)(RE__VEC_TYPE(T) * vec, T elem)

#define RE__VEC_IMPL_push(T)                                                   \
  int RE__VEC_IDENT(T, push)(RE__VEC_TYPE(T) * vec, T elem)                    \
  {                                                                            \
    RE__VEC_CHECK(vec);                                                        \
    RE__VEC_GROW_ONE(T, vec);                                                  \
    vec->_data[vec->_size - 1] = elem;                                         \
    RE__VEC_CHECK(vec);                                                        \
    return 0;                                                                  \
  }

#if RE_DEBUG

#define RE__VEC_CHECK_POP(vec)                                                 \
  do {                                                                         \
    /* ensure that there is an element to pop */                               \
    RE_ASSERT(vec->_size > 0);                                                 \
  } while (0)

#else

#define RE__VEC_CHECK_POP(vec) RE__UNUSED(vec)

#endif

#define RE__VEC_DECL_pop(T) T RE__VEC_IDENT(T, pop)(RE__VEC_TYPE(T) * vec)

#define RE__VEC_IMPL_pop(T)                                                    \
  T RE__VEC_IDENT(T, pop)(RE__VEC_TYPE(T) * vec)                               \
  {                                                                            \
    RE__VEC_CHECK(vec);                                                        \
    RE__VEC_CHECK_POP(vec);                                                    \
    return vec->_data[--vec->_size];                                           \
  }

#define RE__VEC_DECL_cat(T)                                                    \
  T RE__VEC_IDENT(T, cat)(RE__VEC_TYPE(T) * vec, RE__VEC_TYPE(T) * other)

#define RE__VEC_IMPL_cat(T)                                                    \
  int RE__VEC_IDENT(T, cat)(RE__VEC_TYPE(T) * vec, RE__VEC_TYPE(T) * other)    \
  {                                                                            \
    re_size i;                                                                 \
    re_size old_size = vec->_size;                                             \
    RE__VEC_CHECK(vec);                                                        \
    RE__VEC_CHECK(other);                                                      \
    RE__VEC_GROW(T, vec, other->_size);                                        \
    for (i = 0; i < other->_size; i++) {                                       \
      vec->_data[old_size + i] = other->_data[i];                              \
    }                                                                          \
    RE__VEC_CHECK(vec);                                                        \
    return 0;                                                                  \
  }

#define RE__VEC_DECL_insert(T)                                                 \
  int RE__VEC_IDENT(T, insert)(RE__VEC_TYPE(T) * vec, re_size index, T elem)

#define RE__VEC_IMPL_insert(T)                                                 \
  int RE__VEC_IDENT(T, insert)(RE__VEC_TYPE(T) * vec, re_size index, T elem)   \
  {                                                                            \
    re_size i;                                                                 \
    re_size old_size = vec->_size;                                             \
    RE__VEC_CHECK(vec);                                                        \
    RE__VEC_GROW_ONE(T, vec);                                                  \
    if (old_size != 0) {                                                       \
      for (i = old_size; i >= index + 1; i--) {                                \
        vec->_data[i] = vec->_data[i - 1];                                     \
      }                                                                        \
    }                                                                          \
    vec->_data[index] = elem;                                                  \
    return 0;                                                                  \
  }

#define RE__VEC_DECL_peek(T)                                                   \
  T RE__VEC_IDENT(T, peek)(const RE__VEC_TYPE(T) * vec)

#define RE__VEC_IMPL_peek(T)                                                   \
  T RE__VEC_IDENT(T, peek)(const RE__VEC_TYPE(T) * vec)                        \
  {                                                                            \
    RE__VEC_CHECK(vec);                                                        \
    RE__VEC_CHECK_POP(vec);                                                    \
    return vec->_data[vec->_size - 1];                                         \
  }

#define RE__VEC_DECL_clear(T)                                                  \
  void RE__VEC_IDENT(T, clear)(RE__VEC_TYPE(T) * vec)

#define RE__VEC_IMPL_clear(T)                                                  \
  void RE__VEC_IDENT(T, clear)(RE__VEC_TYPE(T) * vec)                          \
  {                                                                            \
    RE__VEC_CHECK(vec);                                                        \
    vec->_size = 0;                                                            \
  }

#define RE__VEC_DECL_size(T)                                                   \
  re_size RE__VEC_IDENT(T, size)(const RE__VEC_TYPE(T) * vec)

#define RE__VEC_IMPL_size(T)                                                   \
  re_size RE__VEC_IDENT(T, size)(const RE__VEC_TYPE(T) * vec)                  \
  {                                                                            \
    return vec->_size;                                                         \
  }

#if RE_DEBUG

#define RE__VEC_CHECK_BOUNDS(vec, idx)                                         \
  do {                                                                         \
    /* ensure that idx is within bounds */                                     \
    RE_ASSERT(idx < vec->_size);                                               \
  } while (0)

#else

#define RE__VEC_CHECK_BOUNDS(vec, idx)                                         \
  do {                                                                         \
    RE__UNUSED(vec);                                                           \
    RE__UNUSED(idx);                                                           \
  } while (0)

#endif

#define RE__VEC_DECL_get(T)                                                    \
  T RE__VEC_IDENT(T, get)(const RE__VEC_TYPE(T) * vec, re_size idx)

#define RE__VEC_IMPL_get(T)                                                    \
  T RE__VEC_IDENT(T, get)(const RE__VEC_TYPE(T) * vec, re_size idx)            \
  {                                                                            \
    RE__VEC_CHECK(vec);                                                        \
    RE__VEC_CHECK_BOUNDS(vec, idx);                                            \
    return vec->_data[idx];                                                    \
  }

#define RE__VEC_DECL_getref(T)                                                 \
  T* RE__VEC_IDENT(T, getref)(RE__VEC_TYPE(T) * vec, re_size idx)

#define RE__VEC_IMPL_getref(T)                                                 \
  T* RE__VEC_IDENT(T, getref)(RE__VEC_TYPE(T) * vec, re_size idx)              \
  {                                                                            \
    RE__VEC_CHECK(vec);                                                        \
    RE__VEC_CHECK_BOUNDS(vec, idx);                                            \
    return &vec->_data[idx];                                                   \
  }

#define RE__VEC_DECL_getcref(T)                                                \
  const T* RE__VEC_IDENT(T, getcref)(const RE__VEC_TYPE(T) * vec, re_size idx)

#define RE__VEC_IMPL_getcref(T)                                                \
  const T* RE__VEC_IDENT(T, getcref)(const RE__VEC_TYPE(T) * vec, re_size idx) \
  {                                                                            \
    RE__VEC_CHECK(vec);                                                        \
    RE__VEC_CHECK_BOUNDS(vec, idx);                                            \
    return &vec->_data[idx];                                                   \
  }

#define RE__VEC_DECL_set(T)                                                    \
  void RE__VEC_IDENT(T, set)(RE__VEC_TYPE(T) * vec, re_size idx, T elem)

#define RE__VEC_IMPL_set(T)                                                    \
  void RE__VEC_IDENT(T, set)(RE__VEC_TYPE(T) * vec, re_size idx, T elem)       \
  {                                                                            \
    RE__VEC_CHECK(vec);                                                        \
    RE__VEC_CHECK_BOUNDS(vec, idx);                                            \
    vec->_data[idx] = elem;                                                    \
  }

#define RE__VEC_DECL_capacity(T)                                               \
  re_size RE__VEC_IDENT(T, capacity)(RE__VEC_TYPE(T) * vec)

#define RE__VEC_IMPL_capacity(T)                                               \
  re_size RE__VEC_IDENT(T, capacity)(RE__VEC_TYPE(T) * vec)                    \
  {                                                                            \
    return vec->_alloc;                                                        \
  }

#define RE__VEC_DECL_get_data(T)                                               \
  const T* RE__VEC_IDENT(T, get_data)(const RE__VEC_TYPE(T) * vec)

#define RE__VEC_IMPL_get_data(T)                                               \
  const T* RE__VEC_IDENT(T, get_data)(const RE__VEC_TYPE(T) * vec)             \
  {                                                                            \
    return vec->_data;                                                         \
  }

#define RE__VEC_DECL_move(T)                                                   \
  void RE__VEC_IDENT(T, move)(RE__VEC_TYPE(T) * vec, RE__VEC_TYPE(T) * old);

#define RE__VEC_IMPL_move(T)                                                   \
  void RE__VEC_IDENT(T, move)(RE__VEC_TYPE(T) * vec, RE__VEC_TYPE(T) * old)    \
  {                                                                            \
    RE__VEC_CHECK(old);                                                        \
    *vec = *old;                                                               \
    RE__VEC_IDENT(T, init)(old);                                               \
  }

#define RE__VEC_DECL_reserve(T)                                                \
  int RE__VEC_IDENT(T, reserve)(RE__VEC_TYPE(T) * vec, re_size cap);

#define RE__VEC_IMPL_reserve(T)                                                \
  int RE__VEC_IDENT(T, reserve)(RE__VEC_TYPE(T) * vec, re_size cap)            \
  {                                                                            \
    RE__VEC_CHECK(vec);                                                        \
    RE__VEC_SETSIZE(T, vec, cap);                                              \
    return 0;                                                                  \
  }

/* bits/container/arena */
#define RE__ARENA_REF_NONE -1

#define RE__ARENA_TYPE(T) RE__PASTE(T, _arena)

#define RE__ARENA_STORAGE_TYPE(T) RE__PASTE(T, __arena_storage)

#define RE__ARENA_IDENT(T, name) RE__PASTE(T, RE__PASTE(_arena_, name))

#define RE__ARENA_IDENT_INTERNAL(T, name)                                      \
  RE__PASTE(T, RE__PASTE(__arena__, name))

#define RE__ARENA_DECL_FUNC(T, func) RE__PASTE(RE__ARENA_DECL_, func)(T)

#define RE__ARENA_IMPL_FUNC(T, func) RE__PASTE(RE__ARENA_IMPL_, func)(T)

#define RE__ARENA_DECL(T)                                                      \
  typedef struct RE__ARENA_STORAGE_TYPE(T) {                                   \
    T _elem;                                                                   \
    re_int32 _next_ref;                                                        \
    re_int32 _prev_ref;                                                        \
  } RE__ARENA_STORAGE_TYPE(T);                                                 \
  RE__VEC_DECL(RE__ARENA_STORAGE_TYPE(T));                                     \
  typedef struct RE__ARENA_TYPE(T) {                                           \
    RE__VEC_TYPE(RE__ARENA_STORAGE_TYPE(T)) _vec;                              \
    re_int32 _last_empty_ref;                                                  \
    re_int32 _first_ref;                                                       \
    re_int32 _last_ref;                                                        \
  } RE__ARENA_TYPE(T)

#define RE__ARENA_DECL_init(T)                                                 \
  void RE__ARENA_IDENT(T, init)(RE__ARENA_TYPE(T) * arena);

#define RE__ARENA_IMPL_init(T)                                                 \
  RE__VEC_IMPL_FUNC(RE__ARENA_STORAGE_TYPE(T), init)                           \
  RE__VEC_IMPL_FUNC(RE__ARENA_STORAGE_TYPE(T), destroy)                        \
  RE__VEC_IMPL_FUNC(RE__ARENA_STORAGE_TYPE(T), getref)                         \
  RE__VEC_IMPL_FUNC(RE__ARENA_STORAGE_TYPE(T), getcref)                        \
  RE__VEC_IMPL_FUNC(RE__ARENA_STORAGE_TYPE(T), push)                           \
  RE__VEC_IMPL_FUNC(RE__ARENA_STORAGE_TYPE(T), size)                           \
  void RE__ARENA_IDENT(T, init)(RE__ARENA_TYPE(T) * arena)                     \
  {                                                                            \
    RE__VEC_IDENT(RE__ARENA_STORAGE_TYPE(T), init)(&arena->_vec);              \
    arena->_last_empty_ref = RE__ARENA_REF_NONE;                               \
    arena->_first_ref = RE__ARENA_REF_NONE;                                    \
    arena->_last_ref = RE__ARENA_REF_NONE;                                     \
  }

#define RE__ARENA_DECL_destroy(T)                                              \
  void RE__ARENA_IDENT(T, destroy)(RE__ARENA_TYPE(T) * arena);

#define RE__ARENA_IMPL_destroy(T)                                              \
  void RE__ARENA_IDENT(T, destroy)(RE__ARENA_TYPE(T) * arena)                  \
  {                                                                            \
    RE__VEC_IDENT(RE__ARENA_STORAGE_TYPE(T), destroy)(&arena->_vec);           \
  }

#define RE__ARENA_DECL_getref(T)                                               \
  T* RE__ARENA_IDENT(T, getref)(RE__ARENA_TYPE(T) * arena, re_int32 elem_ref);

#define RE__ARENA_IMPL_getref(T)                                               \
  T* RE__ARENA_IDENT(T, getref)(RE__ARENA_TYPE(T) * arena, re_int32 elem_ref)  \
  {                                                                            \
    RE_ASSERT(elem_ref != RE__ARENA_REF_NONE);                                 \
    return &RE__VEC_IDENT(RE__ARENA_STORAGE_TYPE(T), getref)(                  \
                &arena->_vec, (re_size)elem_ref)                               \
                ->_elem;                                                       \
  }

#define RE__ARENA_DECL_getcref(T)                                              \
  const T* RE__ARENA_IDENT(T, getcref)(                                        \
      const RE__ARENA_TYPE(T) * arena, re_int32 elem_ref);

#define RE__ARENA_IMPL_getcref(T)                                              \
  const T* RE__ARENA_IDENT(T, getcref)(                                        \
      const RE__ARENA_TYPE(T) * arena, re_int32 elem_ref)                      \
  {                                                                            \
    RE_ASSERT(elem_ref != RE__ARENA_REF_NONE);                                 \
    return &RE__VEC_IDENT(RE__ARENA_STORAGE_TYPE(T), getcref)(                 \
                &arena->_vec, (re_size)elem_ref)                               \
                ->_elem;                                                       \
  }

#define RE__ARENA_DECL_add(T)                                                  \
  int RE__ARENA_IDENT(T, add)(                                                 \
      RE__ARENA_TYPE(T) * arena, T elem, re_int32 * out_ref)

#define RE__ARENA_IMPL_add(T)                                                  \
  int RE__ARENA_IDENT(T, add)(                                                 \
      RE__ARENA_TYPE(T) * arena, T elem, re_int32 * out_ref)                   \
  {                                                                            \
    int err = 0;                                                               \
    re_int32 empty_ref = arena->_last_empty_ref;                               \
    RE__ARENA_STORAGE_TYPE(T) * empty;                                         \
    RE__ARENA_STORAGE_TYPE(T) * prev_elem_ptr;                                 \
    if (empty_ref != RE__ARENA_REF_NONE) {                                     \
      empty = RE__VEC_IDENT(RE__ARENA_STORAGE_TYPE(T), getref)(                \
          &arena->_vec, (re_size)empty_ref);                                   \
      arena->_last_empty_ref = empty->_next_ref;                               \
      *out_ref = empty_ref;                                                    \
      empty->_elem = elem;                                                     \
    } else {                                                                   \
      RE__ARENA_STORAGE_TYPE(T) new_elem;                                      \
      *out_ref = (re_int32)RE__VEC_IDENT(RE__ARENA_STORAGE_TYPE(T), size)(     \
          &arena->_vec);                                                       \
      new_elem._elem = elem;                                                   \
      if ((err = RE__VEC_IDENT(RE__ARENA_STORAGE_TYPE(T), push)(               \
               &arena->_vec, new_elem))) {                                     \
        return err;                                                            \
      }                                                                        \
      empty = RE__VEC_IDENT(RE__ARENA_STORAGE_TYPE(T), getref)(                \
          &arena->_vec, (re_size)(*out_ref));                                  \
    }                                                                          \
    empty->_next_ref = RE__ARENA_REF_NONE;                                     \
    empty->_prev_ref = arena->_last_ref;                                       \
    if (arena->_last_ref != RE__ARENA_REF_NONE) {                              \
      prev_elem_ptr = RE__VEC_IDENT(RE__ARENA_STORAGE_TYPE(T), getref)(        \
          &arena->_vec, (re_size)arena->_last_ref);                            \
      prev_elem_ptr->_next_ref = *out_ref;                                     \
    }                                                                          \
    if (arena->_first_ref == RE__ARENA_REF_NONE) {                             \
      arena->_first_ref = *out_ref;                                            \
    }                                                                          \
    arena->_last_ref = *out_ref;                                               \
    return err;                                                                \
  }

#define RE__ARENA_DECL_begin(T)                                                \
  re_int32 RE__ARENA_IDENT(T, begin)(RE__ARENA_TYPE(T) * arena);

#define RE__ARENA_IMPL_begin(T)                                                \
  re_int32 RE__ARENA_IDENT(T, begin)(RE__ARENA_TYPE(T) * arena)                \
  {                                                                            \
    return arena->_first_ref;                                                  \
  }

#define RE__ARENA_DECL_next(T)                                                 \
  re_int32 RE__ARENA_IDENT(T, next)(RE__ARENA_TYPE(T) * arena);

#define RE__ARENA_IMPL_next(T)                                                 \
  re_int32 RE__ARENA_IDENT(T, next)(                                           \
      RE__ARENA_TYPE(T) * arena, re_int32 prev_ref)                            \
  {                                                                            \
    const RE__ARENA_STORAGE_TYPE(T) * elem_storage;                            \
    RE_ASSERT(prev_ref != RE__ARENA_REF_NONE);                                 \
    elem_storage = RE__VEC_IDENT(RE__ARENA_STORAGE_TYPE(T), getcref)(          \
        &arena->_vec, (re_size)prev_ref);                                      \
    return elem_storage->_next_ref;                                            \
  }

/* re */
#ifndef RE_INTERNAL_H
#define RE_INTERNAL_H
#define RE__ERROR_COMPRESSION_FORMAT (RE_ERROR_INTERNAL - 1)
#define RE__ERROR_PROGMAX (RE_ERROR_INTERNAL - 2)

/* Chop the first couple bits off of a hash value. */
/* These are only used for testing in order to generate intentional collisions.
 * In this program collisions are not a security concern, but we'd still like
 * them to be robust. */
#if RE_TEST
#define RE__WEAKEN_HASH(h) (h >> 16)
#else
#define RE__WEAKEN_HASH(h) h
#endif

/* ---------------------------------------------------------------------------
 * Byte ranges (re_range.c)
 * ------------------------------------------------------------------------ */
/* POD type */
/* Holds a byte range [min, max] */
typedef struct re__byte_range {
  re_uint8 min;
  re_uint8 max;
} re__byte_range;

/* Check if two byte ranges equal each other */
int re__byte_range_equals(re__byte_range range, re__byte_range other);

/* Check if two byte ranges are adjacent (other comes directly after range) */
int re__byte_range_adjacent(re__byte_range range, re__byte_range other);

/* Check if two byte ranges intersect */
int re__byte_range_intersects(re__byte_range range, re__byte_range clip);

/* Compute the intersection of two byte ranges (requires predicate
 * re__byte_range_intersects(range, other) == 1) */
re__byte_range
re__byte_range_intersection(re__byte_range range, re__byte_range clip);

/* Compute the union of two adjacent byte ranges (requires predicate
 * re__byte_range_adjacent(range, other) == 1) */
re__byte_range re__byte_range_merge(re__byte_range range, re__byte_range other);

#if RE_DEBUG

void re__byte_debug_dump(re_uint8 byte);
void re__byte_range_debug_dump(re__byte_range br);

#endif

/* ---------------------------------------------------------------------------
 * Rune ranges (re_range.c)
 * ------------------------------------------------------------------------ */
/* POD type */
/* Stores characters in the range [min, max] == [min, max+1) */
typedef struct re__rune_range {
  re_rune min;
  re_rune max;
} re__rune_range;

RE__VEC_DECL(re__rune_range);

/* Check if two rune ranges equal each other */
int re__rune_range_equals(re__rune_range range, re__rune_range other);

/* Check if two rune ranges intersect */
int re__rune_range_intersects(re__rune_range range, re__rune_range clip);

/* Clamp the given range to be within the given bounds (requires predicate
 * re__rune_range_intersects(range, clip) == 1) */
re__rune_range
re__rune_range_clamp(re__rune_range range, re__rune_range bounds);

#if RE_DEBUG

void re__rune_debug_dump(re_rune rune);
void re__rune_range_debug_dump(re__rune_range rr);

#endif

/* ---------------------------------------------------------------------------
 * Character class (re_charclass.c)
 * ------------------------------------------------------------------------ */
typedef struct re__charclass {
  /* Non-overlapping, sorted set of ranges. */
  re__rune_range* ranges;
  re_size ranges_size;
} re__charclass;
/* Implementation detail: since the list of ranges is in this normal form
 * (non-overlapping and sorted) doing certain operations is quick, especially
 * checking equality. Additionally, creating the optimized code for the forward
 * NFA is super fast, see re__compile_charclass.c for more details */

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

/* Destroy the given character class */
RE_INTERNAL void re__charclass_destroy(re__charclass* charclass);

/* Return the array of ranges stored in this character class, pointer is invalid
 * after calls to re__charclass_push() */
RE_INTERNAL const re__rune_range*
re__charclass_get_ranges(const re__charclass* charclass);

/* Get the number of ranges in the character class */
RE_INTERNAL re_size re__charclass_size(const re__charclass* charclass);

/* Check if the given character class is equal to the other */
RE_INTERNAL int re__charclass_equals(
    const re__charclass* charclass, const re__charclass* other);

#if RE_DEBUG

RE_INTERNAL void
re__charclass_dump(const re__charclass* charclass, re_size lvl);
RE_INTERNAL int re__charclass_verify(const re__charclass* charclass);

#endif

/* ---------------------------------------------------------------------------
 * Rune data (re_rune_data.c)
 * ------------------------------------------------------------------------ */
#define RE__RUNE_DATA_MAX_FOLD_CLASS 4

typedef struct re__rune_data_prop re__rune_data_prop;

struct re__rune_data_prop {
  const char* property_name;
  re__rune_range* ranges;
  re_size ranges_size;
  re__rune_data_prop* next;
};

typedef struct re__rune_data {
  /* Holds smaller sets of ranges. This is used by default. */
  re_rune fold_storage[RE__RUNE_DATA_MAX_FOLD_CLASS];
  /* Head of property list. */
  re__rune_data_prop* properties_head;
} re__rune_data;

RE_INTERNAL void re__rune_data_init(re__rune_data* rune_data);
RE_INTERNAL void re__rune_data_destroy(re__rune_data* rune_data);
RE_INTERNAL int
re__rune_data_casefold(re__rune_data* rune_data, re_rune ch, re_rune** runes);
RE_INTERNAL re_error re__rune_data_get_property(
    re__rune_data* rune_data, const char* property_name,
    re_size property_name_size, re__rune_range** ranges_out,
    re_size* ranges_size_out);

/* ---------------------------------------------------------------------------
 * Immediate-mode character class builder (re_charclass.c)
 * ------------------------------------------------------------------------ */
typedef struct re__charclass_builder {
  /* List of pending ranges, is built in a sorted, non-overlapping way */
  re__rune_range_vec ranges;
  /* 1 if the character class will be inverted when
   * re__charclass_builder_finish() is called */
  int should_invert;
  /* 1 if the character class will have its rune ranges case-folded */
  int should_fold;
  /* Optimization: highest rune in this charclass, if the rune-range to be
   * added is greater than this, we can avoid calling insert() on ranges and
   * just use push(), O(1) baby */
  re_rune highest;
  /* Reference to rune data */
  re__rune_data* rune_data;
} re__charclass_builder;

/* Initialize this character class builder */
RE_INTERNAL void re__charclass_builder_init(
    re__charclass_builder* builder, re__rune_data* rune_data);

/* Destroy this character class builder */
RE_INTERNAL void re__charclass_builder_destroy(re__charclass_builder* builder);

/* Begin building a character class */
RE_INTERNAL void re__charclass_builder_begin(re__charclass_builder* builder);

/* Set the invert flag -- when finish() is called the class will be inverted */
RE_INTERNAL void re__charclass_builder_invert(re__charclass_builder* builder);

/* Set the fold flag -- inserted ranges will be case-folded */
RE_INTERNAL void re__charclass_builder_fold(re__charclass_builder* builder);

/* Insert a range of characters into this character class */
RE_INTERNAL re_error re__charclass_builder_insert_range(
    re__charclass_builder* builder, re__rune_range range);

/* Insert a Unicode property into this character class */
RE_INTERNAL re_error re__charclass_builder_insert_property(
    re__charclass_builder* builder, re__str_view str, int inverted);

RE_INTERNAL re_error re__charclass_builder_insert_ascii_class(
    re__charclass_builder* builder, re__charclass_ascii_type type,
    int inverted);

RE_INTERNAL re_error re__charclass_builder_insert_ascii_class_by_str(
    re__charclass_builder* builder, re__str_view name, int inverted);

/* Finish building, and output results to the given character class */
RE_INTERNAL re_error re__charclass_builder_finish(
    re__charclass_builder* builder, re__charclass* charclass);

/* ---------------------------------------------------------------------------
 * AST node (re_ast.c)
 * ------------------------------------------------------------------------ */
typedef struct re__ast re__ast;

/* Enumeration of AST node types. */
typedef enum re__ast_type {
  RE__AST_TYPE_MIN = 0,
  /* No type. Should never occur. */
  RE__AST_TYPE_NONE = 0,
  /* A single character. */
  /* TODO: fold into type_str */
  RE__AST_TYPE_RUNE,
  /* A string of characters. */
  RE__AST_TYPE_STR,
  /* A character class. */
  RE__AST_TYPE_CHARCLASS,
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
  /* Any character including newlines ((?s).) */
  RE__AST_TYPE_ANY_CHAR_NEWLINE,
  /* Any byte (\C) */
  RE__AST_TYPE_ANY_BYTE,
  /* RE__AST_TYPE_BACKREF */ /* <- haha, you wish */
  RE__AST_TYPE_MAX
} re__ast_type;

RE__VEC_DECL(re__ast);

/* Maximum number of repetitions a quantifier can have.
 * One of a few mechanisms in place to prevent diabolical program growth. */
#define RE__AST_QUANTIFIER_MAX 2000

/* Signifies infinite repetitions (*, +, {x,}) */
#define RE__AST_QUANTIFIER_INFINITY RE__AST_QUANTIFIER_MAX + 2

/* Quantifier info. */
/* Range: [min, max) */
typedef struct re__ast_quantifier_info {
  /* Minimum amount. */
  re_int32 min;
  /* Maximum amount. Could be RE__AST_QUANTIFIER_MAX or
   * RE__AST_QUANTIFIER_INFINITY. */
  re_int32 max;
  /* Whether or not to prefer fewer matches. */
  int greediness;
} re__ast_quantifier_info;

/* Assert types, as they are represented in the AST. */
typedef enum re__assert_type {
  /* Minimum value */
  RE__ASSERT_TYPE_MIN = 1,
  /* Text/Line start */
  RE__ASSERT_TYPE_TEXT_START = 1,
  /* Text/Line start */
  RE__ASSERT_TYPE_TEXT_END = 2,
  /* Text start */
  RE__ASSERT_TYPE_TEXT_START_ABSOLUTE = 4,
  /* Text end */
  RE__ASSERT_TYPE_TEXT_END_ABSOLUTE = 8,
  /* Word character (one-character lookaround) */
  RE__ASSERT_TYPE_WORD = 16,
  /* Not a word character */
  RE__ASSERT_TYPE_WORD_NOT = 32,
  /* Maximum value (non-inclusive) */
  RE__ASSERT_TYPE_MAX = 64
} re__assert_type;

/* Group flags */
typedef enum re__ast_group_flags {
  /* Not a capturing group */
  RE__AST_GROUP_FLAG_NONMATCHING = 1,
  /* Group has a name */
  RE__AST_GROUP_FLAG_NAMED = 2,
  /* Maximum value (non-inclusive) */
  RE__AST_GROUP_FLAG_MAX = 4
} re__ast_group_flags;

/* Group info */
typedef struct re__ast_group_info {
  /* Group's flags */
  re__ast_group_flags flags;
  /* Index (ID) if this group */
  re_uint32 group_idx;
} re__ast_group_info;

/* Holds AST node data depending on the node type. */
typedef union re__ast_data {
  /* RE__AST_TYPE_RUNE: holds a single character */
  re_rune rune;
  /* RE__AST_TYPE_STRING: holds a reference to a string. */
  re_int32 str_ref;
  /* RE__AST_TYPE_CLASS: holds a reference to a character class. */
  re_int32 charclass_ref;
  /* RE__AST_TYPE_GROUP: holds the group's index and flags */
  re__ast_group_info group_info;
  /* RE__AST_TYPE_QUANTIFIER: minimum/maximum/greediness */
  re__ast_quantifier_info quantifier_info;
  /* RE__AST_TYPE_ASSERT: type(s) of assert present */
  re__assert_type assert_type;
} re__ast_data;

/* AST node */
struct re__ast {
  /* Reference to next / previous siblings */
  re_int32 next_sibling_ref;
  re_int32 prev_sibling_ref;
  /* Reference to first / last children */
  re_int32 first_child_ref;
  re_int32 last_child_ref;
  /* Type of AST node */
  re__ast_type type;
  /* Data describing this node */
  re__ast_data _data;
};
/* 32 bytes on my M1 Mac */
/* References are in arena format, that is, they are offsets in a contiguous
 * chunk of memory managed by re__ast_root. This design decision was made to
 * simplify destruction / moving of AST nodes, and it speeds up the parser
 * implementation. Null references are RE__AST_NONE. */

/* Initialize the given node as a RE__AST_TYPE_RUNE node */
RE_INTERNAL void re__ast_init_rune(re__ast* ast, re_rune rune);

/* Initialize the given node as a RE__AST_TYPE_STR node, str_ref is the index
 * of the node's string given by the ast_root object in its string arena */
RE_INTERNAL void re__ast_init_str(re__ast* ast, re_int32 str_ref);

/* Initialize the given node as a RE__AST_TYPE_CHARCLASS node, charclass_ref
 * is the index of the node's charclass given by the ast_root object in its
 * charclass arena */
RE_INTERNAL void re__ast_init_charclass(re__ast* ast, re_int32 charclass_ref);

/* Initialize the given node as a RE__AST_TYPE_CONCAT node */
RE_INTERNAL void re__ast_init_concat(re__ast* ast);

/* Initialize the given node as a RE__AST_TYPE_ALT node */
RE_INTERNAL void re__ast_init_alt(re__ast* ast);

/* Initialize the given node as a RE__AST_TYPE_QUANTIFIER node with the
 * specified boundaries min/max */
RE_INTERNAL void
re__ast_init_quantifier(re__ast* ast, re_int32 min, re_int32 max);

/* Initialize the given node as a RE__AST_TYPE_GROUP with the given group index
 * and flags */
RE_INTERNAL void re__ast_init_group(
    re__ast* ast, re_uint32 group_idx, re__ast_group_flags flags);

/* Initialize the given node as a RE__AST_TYPE_ASSERT with the given assert
 * type */
RE_INTERNAL void re__ast_init_assert(re__ast* ast, re__assert_type assert_type);

/* Initialize the given node as a RE__AST_TYPE_ANY_CHAR (.) */
RE_INTERNAL void re__ast_init_any_char(re__ast* ast);

/* Initialize the given node as a RE__AST_TYPE_ANY_CHAR_NEWLINE ((?s).) */
RE_INTERNAL void re__ast_init_any_char_newline(re__ast* ast);

/* Initialize the given node as a RE__AST_TYPE_ANY_BYTE (\C) */
RE_INTERNAL void re__ast_init_any_byte(re__ast* ast);

/* Destroy the given node */
RE_INTERNAL void re__ast_destroy(re__ast* ast);

/* Get the given node's greediness (node must be RE__AST_TYPE_QUANTIFIER) */
RE_INTERNAL int re__ast_get_quantifier_greediness(const re__ast* ast);

/* Set the given node's greediness (node must be RE__AST_TYPE_QUANTIFIER) */
RE_INTERNAL void re__ast_set_quantifier_greediness(re__ast* ast, int is_greedy);

/* Get the given node's minimum repeat (node must be RE__AST_TYPE_QUANTIFIER) */
RE_INTERNAL re_int32 re__ast_get_quantifier_min(const re__ast* ast);

/* Get the given node's maximum repeat (node must be RE__AST_TYPE_QUANTIFIER) */
RE_INTERNAL re_int32 re__ast_get_quantifier_max(const re__ast* ast);

/* Get the given node's rune (node must be RE__AST_TYPE_RUNE) */
RE_INTERNAL re_rune re__ast_get_rune(const re__ast* ast);

/* Get the given node's group flags (node must be RE__AST_TYPE_GROUP) */
RE_INTERNAL re__ast_group_flags re__ast_get_group_flags(const re__ast* ast);

/* Get the given node's group index (node must be RE__AST_TYPE_GROUP) */
RE_INTERNAL re_uint32 re__ast_get_group_idx(const re__ast* ast);

/* Get the given node's assert bits (node must be RE__AST_TYPE_ASSERT) */
RE_INTERNAL re__assert_type re__ast_get_assert_type(const re__ast* ast);

/* Get the reference to the node's string in ast_root (node must be
 * RE__AST_TYPE_STR) */
RE_INTERNAL re_int32 re__ast_get_str_ref(const re__ast* ast);

RE__ARENA_DECL(re__charclass);
RE__ARENA_DECL(re__str);

RE__VEC_DECL(re__str);

/* Sentinel value for re__ast_root object */
#define RE__AST_NONE -1

/* ---------------------------------------------------------------------------
 * AST root manager (re_ast.c)
 * ------------------------------------------------------------------------ */
typedef struct re__ast_root {
  /* Vector of AST nodes, used as an arena */
  re__ast_vec ast_vec;
  /* Last empty location in ast_vec */
  re_int32 last_empty_ref;
  /* Reference to root node */
  re_int32 root_ref;
  /* Reference to last child of root node */
  re_int32 root_last_child_ref;
  /* Character classes in use by RE__AST_TYPE_CHARCLASS nodes */
  re__charclass_arena charclasses;
  /* Strings in use by RE__AST_TYPE_STR nodes */
  re__str_arena strings;
  /* Group names in use by RE__AST_TYPE_GROUP nodes */
  re__str_vec group_names;
} re__ast_root;

/* Initialize this ast root */
RE_INTERNAL void re__ast_root_init(re__ast_root* ast_root);

/* Destroy this ast root */
RE_INTERNAL void re__ast_root_destroy(re__ast_root* ast_root);

/* Given ast_ref, get a pointer to the re__ast within this ast root */
RE_INTERNAL re__ast* re__ast_root_get(re__ast_root* ast_root, re_int32 ast_ref);

/* Given ast_ref, get a const pointer to the re__ast within this ast root */
RE_INTERNAL const re__ast*
re__ast_root_get_const(const re__ast_root* ast_root, re_int32 ast_ref);

/* Remove the given reference from the ast root. Does not clean up references
 * from previous / parent nodes. Only used for testing. */
RE_INTERNAL void re__ast_root_remove(re__ast_root* ast_root, re_int32 ast_ref);

/* Replace the node at the given reference with the given replacement. */
RE_INTERNAL void re__ast_root_replace(
    re__ast_root* ast_root, re_int32 ast_ref, re__ast replacement);

/* Create a new node under the given parent with the given ast, storing its
 * resulting reference in out_ref. */
RE_INTERNAL re_error re__ast_root_add_child(
    re__ast_root* ast_root, re_int32 parent_ref, re__ast ast,
    re_int32* out_ref);

/* Create a new node under the given parent that wraps the given inner reference
 * with the given outer node, storing the resulting reference to ast_outer in
 * out_ref. */
RE_INTERNAL re_error re__ast_root_add_wrap(
    re__ast_root* ast_root, re_int32 parent_ref, re_int32 inner_ref,
    re__ast ast_outer, re_int32* out_ref);

/* Register a new charclass, storing its reference in out_charclass_ref.*/
RE_INTERNAL re_error re__ast_root_add_charclass(
    re__ast_root* ast_root, re__charclass charclass,
    re_int32* out_charclass_ref);

/* Get a const pointer to the charclass referenced by charclass_ref. */
RE_INTERNAL const re__charclass* re__ast_root_get_charclass(
    const re__ast_root* ast_root, re_int32 charclass_ref);

/* Register a new string, storing its reference in out_str_ref. */
RE_INTERNAL re_error re__ast_root_add_str(
    re__ast_root* ast_root, re__str str, re_int32* out_str_ref);

/* Get a pointer to the string referenced by str_ref. */
RE_INTERNAL re__str*
re__ast_root_get_str(re__ast_root* ast_root, re_int32 str_ref);

/* Get the string view referenced by str_ref. */
RE_INTERNAL re__str_view
re__ast_root_get_str_view(const re__ast_root* ast_root, re_int32 str_ref);

/* Register a new group. */
RE_INTERNAL re_error
re__ast_root_add_group(re__ast_root* ast_root, re__str_view group_name);

/* Get a group name referenced by its number. */
RE_INTERNAL re__str_view
re__ast_root_get_group(re__ast_root* ast_root, re_uint32 group_number);

/* Get the number of groups in this ast root. */
RE_INTERNAL re_uint32 re__ast_root_get_num_groups(re__ast_root* ast_root);

/* Get the maximum depth of this ast root. */
RE_INTERNAL re_error
re__ast_root_get_depth(const re__ast_root* ast_root, re_int32* depth);

#if RE_DEBUG

RE_INTERNAL void re__ast_root_debug_dump(
    const re__ast_root* ast_root, re_int32 root_ref, re_int32 lvl);

#endif

/* ---------------------------------------------------------------------------
 * Parser (re_parse.c)
 * ------------------------------------------------------------------------ */
typedef enum re__parse_flags {
  RE__PARSE_FLAG_CASE_INSENSITIVE = 1,
  RE__PARSE_FLAG_MULTILINE = 2,
  RE__PARSE_FLAG_DOT_NEWLINE = 4,
  RE__PARSE_FLAG_UNGREEDY = 8,
  RE__PARSE_FLAG_GLOB = 16
} re__parse_flags;

typedef struct re__parse_frame {
  re_int32 ast_root_ref;
  re_int32 ast_prev_child_ref;
  re__parse_flags flags;
} re__parse_frame;

RE__VEC_DECL(re__parse_frame);

typedef struct re__parse {
  re* reg;
  re__str_view str;
  re_size str_pos;
  re__parse_frame_vec frames;
  re__charclass_builder charclass_builder;
} re__parse;

RE_INTERNAL void re__parse_init(re__parse* parse, re* reg);
RE_INTERNAL void re__parse_destroy(re__parse* parse);
RE_INTERNAL re_error
re__parse_str(re__parse* parse, re__str_view str, re_syntax_flags syntax_flags);

/* ---------------------------------------------------------------------------
 * Instruction format (re_prog.c)
 * ------------------------------------------------------------------------ */
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
  RE__PROG_INST_TYPE_ASSERT,
  /* maximum value of enum */
  RE__PROG_INST_TYPE_MAX
} re__prog_inst_type;

/* Program instruction structure */
/* Layout:
 * 31                              0
 *  PPPPPPPPPPPPPPPPPPPPPPPPPPPPPTTT
 * 31                              0
 *  DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
 * P = primary branch target
 * T = type
 * D = data */
typedef struct re__prog_inst {
  re_uint32 data0;
  re_uint32 data1;
} re__prog_inst;

RE__VEC_DECL(re__prog_inst);

/* Initialize an instruction as a byte instruction, given its ordinal value in
 * the range [0, 255] */
RE_INTERNAL void re__prog_inst_init_byte(re__prog_inst* inst, re_uint8 byte);

/* Initialize an instruction as a byte range instruction, given a
 * re__byte_range */
RE_INTERNAL void
re__prog_inst_init_byte_range(re__prog_inst* inst, re__byte_range br);

/* Initialize an instruction as a split instruction, given its primary and
 * secondary branch targets */
RE_INTERNAL void re__prog_inst_init_split(
    re__prog_inst* inst, re__prog_loc primary, re__prog_loc secondary);

/* Initialize an instruction as a match instruction, given its match index */
RE_INTERNAL void
re__prog_inst_init_match(re__prog_inst* inst, re_uint32 match_idx);

/* Initialize an instruction as a fail instruction */
RE_INTERNAL void re__prog_inst_init_fail(re__prog_inst* inst);

/* Initialize an instruction as an assert instruction, given its assert context
 * flags */
RE_INTERNAL void
re__prog_inst_init_assert(re__prog_inst* inst, re_uint32 assert_context);

/* Initialize an instruction as a save instruction, given its save slot index */
RE_INTERNAL void
re__prog_inst_init_save(re__prog_inst* inst, re_uint32 save_idx);

/* Get the primary branch target of an instruction */
RE_INTERNAL re__prog_loc re__prog_inst_get_primary(const re__prog_inst* inst);

/* Set the primary branch target of an instruction */
RE_INTERNAL void
re__prog_inst_set_primary(re__prog_inst* inst, re__prog_loc loc);

/* Get an instruction's type */
RE_INTERNAL re__prog_inst_type
re__prog_inst_get_type(const re__prog_inst* inst);

/* Get the byte value of an instruction (instruction must be
 * RE__PROG_INST_TYPE_BYTE) */
RE_INTERNAL re_uint8 re__prog_inst_get_byte(const re__prog_inst* inst);

/* Get the minimum byte value of an instruction (instruction must be
 * RE__PROG_INST_TYPE_BYTE_RANGE) */
RE_INTERNAL re_uint8 re__prog_inst_get_byte_min(const re__prog_inst* inst);

/* Get the maximum byte value of an instruction (instruction must be
 * RE__PROG_INST_TYPE_BYTE_RANGE) */
RE_INTERNAL re_uint8 re__prog_inst_get_byte_max(const re__prog_inst* inst);

/* Get the secondary branch target of an instruction (instruction must be
 * RE__PROG_INST_TYPE_SPLIT) */
RE_INTERNAL re__prog_loc
re__prog_inst_get_split_secondary(const re__prog_inst* inst);

/* Set the secondary branch target of an instruction (instruction must be
 * RE__PROG_INST_TYPE_SPLIT) */
RE_INTERNAL void
re__prog_inst_set_split_secondary(re__prog_inst* inst, re__prog_loc loc);

/* Get an instruction's assert context (instruction must be
 * RE__PROG_INST_TYPE_ASSERT) */
RE_INTERNAL re__assert_type
re__prog_inst_get_assert_ctx(const re__prog_inst* inst);

/* Get an instruction's match index (instruction must be
 * RE__PROG_INST_TYPE_MATCH) */
RE_INTERNAL re_uint32 re__prog_inst_get_match_idx(const re__prog_inst* inst);

/* Get an instruction's save index (instruction must be
 * RE__PROG_INST_TYPE_SAVE) */
RE_INTERNAL re_uint32 re__prog_inst_get_save_idx(const re__prog_inst* inst);

/* Check if two instructions are equal */
RE_INTERNAL int
re__prog_inst_equals(const re__prog_inst* a, const re__prog_inst* b);

/* ---------------------------------------------------------------------------
 * Program (re_prog.c)
 * ------------------------------------------------------------------------ */
#define RE__PROG_SIZE_MAX 100000

/* Program entry points. */
typedef enum re__prog_entry {
  /* Default entry point - start of the program as compiled. */
  RE__PROG_ENTRY_DEFAULT,
  /* Dotstar entry - used for left-unanchored matches. Calls into
   * RE__PROG_ENTRY_DEFAULT. */
  RE__PROG_ENTRY_DOTSTAR,
  /* Maximum value */
  RE__PROG_ENTRY_MAX
} re__prog_entry;

/* The program itself */
typedef struct re__prog {
  /* Instruction listing */
  re__prog_inst_vec _instructions;
  /* Entrypoints */
  re__prog_loc _entrypoints[RE__PROG_ENTRY_MAX];
} re__prog;

/* Initialize a program. */
RE_INTERNAL void re__prog_init(re__prog* prog);

/* Destroy a program. */
RE_INTERNAL void re__prog_destroy(re__prog* prog);

/* Get the size of a program. Also tells the location of the next inst. */
RE_INTERNAL re__prog_loc re__prog_size(const re__prog* prog);

/* Get a pointer to the instruction at the given location in the program. */
RE_INTERNAL re__prog_inst* re__prog_get(re__prog* prog, re__prog_loc loc);

/* Get a const pointer to the instruction at the given location in the
 * program. */
RE_INTERNAL const re__prog_inst*
re__prog_get_const(const re__prog* prog, re__prog_loc loc);

/* Add an instruction to the end of the program. */
RE_INTERNAL re_error re__prog_add(re__prog* prog, re__prog_inst inst);

/* Set the given entrypoint of a program to the given location. */
RE_INTERNAL void
re__prog_set_entry(re__prog* prog, re__prog_entry idx, re__prog_loc loc);

/* Get the given entrypoint of a program. */
RE_INTERNAL re__prog_loc
re__prog_get_entry(const re__prog* prog, re__prog_entry idx);

#if RE_DEBUG
RE_INTERNAL void re__prog_debug_dump(const re__prog* prog);
#endif

/* ---------------------------------------------------------------------------
 * Compilation patch list (re_compile.c)
 * ------------------------------------------------------------------------ */
/* A list of program patches -- locations in the program that need to point to
 * later instructions */
/* Like RE2, we store the location of the next patch in each instruction, so it
 * turns out to be a linked-list of sorts. */
/* This representation is useful because we only really need prepend, append,
 * and iterate facilities. So in addition to not having to perform manual memory
 * allocation, we get quick operations "for free". */
typedef struct re__compile_patches {
  /* Head of linked list */
  re__prog_loc first_inst;
  /* Tail of linked list */
  re__prog_loc last_inst;
} re__compile_patches;

/* Initialize a patch list object */
RE_INTERNAL void re__compile_patches_init(re__compile_patches* patches);

/* Append the given location to the patch list object, given a re__prog for
 * linked-list storage, and a flag indicating if the location is to have its
 * secondary branch target patched. */
RE_INTERNAL void re__compile_patches_append(
    re__compile_patches* patches, re__prog* prog, re__prog_loc to,
    int secondary);

RE_INTERNAL void re__compile_patches_patch(
    re__compile_patches* patches, re__prog* prog, re__prog_loc to);

#if RE_DEBUG

RE_INTERNAL
void re__compile_patches_dump(re__compile_patches* patches, re__prog* prog);

#endif

/* ---------------------------------------------------------------------------
 * Pre-compiled programs (re_prog_data.c)
 * ------------------------------------------------------------------------ */
/* Pre-compiled programs, at this point they only exist as optimized UTF-8 .
 * representations */
typedef enum re__prog_data_id {
  /* dot, forward, accept surrogates, accept newlines */
  RE__PROG_DATA_ID_DOT_FWD_ACCSURR_ACCNL,
  /* dot, reverse, accept surrogates, accept newlines */
  RE__PROG_DATA_ID_DOT_REV_ACCSURR_ACCNL,
  /* dot, forward, reject surrogates, accept newlines */
  RE__PROG_DATA_ID_DOT_FWD_REJSURR_ACCNL,
  /* dot, reverse, reject surrogates, accept newlines */
  RE__PROG_DATA_ID_DOT_REV_REJSURR_ACCNL,
  /* dot, forward, accept surrogates, reject newlines */
  RE__PROG_DATA_ID_DOT_FWD_ACCSURR_REJNL,
  /* dot, reverse, accept surrogates, reject newlines */
  RE__PROG_DATA_ID_DOT_REV_ACCSURR_REJNL,
  /* dot, forward, reject surrogates, reject newlines */
  RE__PROG_DATA_ID_DOT_FWD_REJSURR_REJNL,
  /* dot, reverse, reject surrogates, reject newlines */
  RE__PROG_DATA_ID_DOT_REV_REJSURR_REJNL,
  RE__PROG_DATA_ID_MAX
} re__prog_data_id;

/* Pointers to precompiled program data. See re__prog_decompress for the
 * compressed program format. */
RE_INTERNAL_DATA_DECL re_uint8* re__prog_data[RE__PROG_DATA_ID_MAX];

/* Corresponding sizes of precompiled program data. */
RE_INTERNAL_DATA_DECL re_size re__prog_data_size[RE__PROG_DATA_ID_MAX];

/* Decompress a compressed program into an re__prog object, storing outward
 * patches in the given patches object. */
/* The format for compressed data is as follows:
 * <program> ::= <insts>
 * <insts> ::= <inst> | <inst> <insts>
 * <inst> ::= <inst_byte> | <inst_range> | <inst_split>
 * <inst_byte> ::= 0x00 <loc> <byte_value>
 * <inst_range> ::= 0x01 <loc> <byte_value> <byte_value>
 * <inst_split> ::= 0x02 <loc> <loc>
 * <loc> ::= [0x00-0x7F] | [0x80-0xFF] <loc>
 * <byte_value> ::= [0x00-0xFF]
 *
 * Notes:
 *   <loc> is a variable-length integer, big-endian, where the top bit indicates
 *   more data.
 *
 * Returns RE__ERROR_COMPRESSION_FORMAT if the format is wrong.
 */
RE_INTERNAL re_error re__prog_data_decompress(
    re__prog* prog, re_uint8* compressed_data, re_size compressed_size,
    re__compile_patches* patches);

/* ---------------------------------------------------------------------------
 * Character class compiler (re_compile_charclass.c)
 * ------------------------------------------------------------------------ */
typedef struct re__compile_charclass re__compile_charclass;

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
#define RE__COMPILE_CHARCLASS_TREE_NONE 0
typedef struct re__compile_charclass_tree {
  /* Range of bytes to match */
  re__byte_range byte_range;
  /* Reference to next sibling */
  re_uint32 sibling_ref;
  /* Reference to first child */
  re_uint32 child_ref;
  /* Either:
   * - The hash of this tree, used for caching
   * - A reference to this node's complement in the reverse tree
   * If the tree has not had its forward program generated, then aux is a
   * hash, otherwise, the reverse program is being generated, and aux
   * references the complement node. */
  /* This could be better clarified with a union, I plan on doing this
   * sometime soon. */
  re_uint32 aux;
} re__compile_charclass_tree;
/* 16 bytes, nominally, (16 on my M1 Max) */

re_error re__compile_charclass_new_node(
    re__compile_charclass* char_comp, re_uint32 parent_ref,
    re__byte_range byte_range, re_uint32* out_new_node_ref,
    int use_reverse_tree);

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
 * link them together using the 'next' member, forming a mini linked list. */
typedef struct re__compile_charclass_hash_entry {
  /* Index in sparse array */
  re_int32 sparse_index;
  /* Reference to tree root */
  re_uint32 root_ref;
  /* Compiled instruction location in the program */
  re__prog_loc prog_loc;
  /* Next hash_entry that hashes to the same value,
   * RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE otherwise. */
  re_int32 next;
} re__compile_charclass_hash_entry;

RE_INTERNAL void re__compile_charclass_hash_entry_init(
    re__compile_charclass_hash_entry* hash_entry, re_int32 sparse_index,
    re_uint32 tree_ref, re__prog_loc prog_loc);

RE__VEC_DECL(re__compile_charclass_hash_entry);
RE__VEC_DECL(re__compile_charclass_tree);

#define RE__COMPILE_CHARCLASS_CACHE_SPARSE_SIZE 1024

/* Character class compiler. */
struct re__compile_charclass {
  /* Vector of tree nodes. Each '_ref' entry in re__compile_charclass_tree
   * represents an index in this vector. */
  re__compile_charclass_tree_vec tree;
  /* Reference to root node. */
  re_uint32 root_ref;
  /* Reference to last child of root node. */
  re_uint32 root_last_child_ref;
  /* Reference to root node of reverse tree. */
  re_uint32 rev_root_ref;
  /* Reference to last child of reverse root node. */
  re_uint32 rev_root_last_child_ref;
  /* Sparse tree cache. Each element in this array points to a corresponding
   * position in 'cache_dense'. Lookup is performed by moduloing a tree's hash
   * with the sparse cache size. Since cache hits are relatively rare, this
   * allows pretty inexpensive lookup. */
  re_int32* cache_sparse;
  /* Dense tree cache. Entries with duplicate hashes are linked using their
   * 'next' member. */
  re__compile_charclass_hash_entry_vec cache_dense;
};

void re__compile_charclass_init(re__compile_charclass* char_comp);
void re__compile_charclass_destroy(re__compile_charclass* char_comp);
re_error re__compile_charclass_gen(
    re__compile_charclass* char_comp, const re__charclass* charclass,
    re__prog* prog, re__compile_patches* patches_out, int also_make_reverse);
re_error re__compile_charclass_split_rune_range(
    re__compile_charclass* char_comp, re__rune_range range);
RE_INTERNAL re__compile_charclass_tree* re__compile_charclass_tree_get(
    re__compile_charclass* char_comp, re_uint32 tree_ref);

#if RE_DEBUG
void re__compile_charclass_dump(
    re__compile_charclass* char_comp, re_uint32 tree_idx, re_int32 indent);
#endif

/* ---------------------------------------------------------------------------
 * Program compiler (re_compile.c)
 * ------------------------------------------------------------------------ */
/* Stack frame for the compiler. */
typedef struct re__compile_frame {
  /* Base AST node being compiled. Represents the current node that is being
   * examined at any given point. */
  re_int32 ast_base_ref;
  /* Next child node of ast_base_ref that will be compiled. Will be AST_NONE
   * if the last child has already been processed. */
  re_int32 ast_child_ref;
  /* Running set of patches for this AST node. */
  re__compile_patches patches;
  /* Start and end PCs of this frame */
  re__prog_loc start;
  re__prog_loc end;
  /* For repetitions: the number of times the node's child has been generated
   * already */
  /* For alts: if in set mode, the index of the current alternation so that it
   * may be used to initialize the MATCH instruction */
  re_int32 rep_idx;
} re__compile_frame;

/* Compiler internal structure. */
typedef struct re__compile {
  /* Stack frames list. This is not a frame_vec because we already have the
   * maximum stack depth (stored in ast_root). frames is a static worst-case
   * allocation. Generally pretty speedy. I took a cue from that online regex
   * bible for this one. */
  /* "In retrospect, I think the tree form and the Walker might have been a
   *  mistake ... if the RPN form recorded the maximum stack depth used in the
   *  expression, a traversal would allocate a stack of exactly that size and
   *  then zip through the representation in a single linear scan."
   *    -rsc */
  re__compile_frame* frames;
  /* Size of frames (equal to ast_root->max_depth) */
  re_int32 frames_size;
  /* Current location in frames (the stack pointer) */
  re_int32 frame_ptr;
  /* Charclass compiler object to be used to compile all charclasses, it's
   * better to store it here in case there are multiple character classes in
   * the regex */
  re__compile_charclass char_comp;
  /* The ast root object, marked const so we don't modify it (after the parse
   * phase, we should ideally never touch ast_root) */
  const re__ast_root* ast_root;
  /* Flag set if the compiler should push a child frame to the stack for the
   * next iteration. */
  int should_push_child;
  /* Reference to the child that should get pushed. After the child is done
   * processing, the parent's frame->ast_child_ref will point to the next
   * sibling of should_push_child_ref. */
  re_int32 should_push_child_ref;
  /* Returned (popped) frame from child compilation. Contains child boundaries
   * and, more importantly, child patches. */
  re__compile_frame returned_frame;
  /* Whether or not we are compiling in reverse mode. */
  int reversed;
  /* Pointer to the set root, also indicates if we are compiling in set mode
   * (slightly different alt semantics) */
  re_int32 set;
} re__compile;

RE_INTERNAL void re__compile_init(re__compile* compile);
RE_INTERNAL void re__compile_destroy(re__compile* compile);
RE_INTERNAL re_error re__compile_regex(
    re__compile* compile, const re__ast_root* ast_root, re__prog* prog,
    int reversed, re_int32 set_root);
RE_INTERNAL int re__compile_gen_utf8(re_rune codep, re_uint8* out_buf);
RE_INTERNAL re_error
re__compile_dotstar(re__prog* prog, re__prog_data_id data_id);

/* ---------------------------------------------------------------------------
 * NFA execution context (re_exec_nfa.c)
 * ------------------------------------------------------------------------ */
/* Execution thread. */
typedef struct re__exec_thrd {
  /* PC of this thread */
  re__prog_loc loc;
  /* Slot to save match boundaries to. May be -1 if this thread hasn't found
   * anything yet. */
  re_int32 save_slot;
} re__exec_thrd;

RE__VEC_DECL(re__exec_thrd);

/* Sparse set of threads. */
typedef struct re__exec_thrd_set {
  /* Sparse representation: when indexed with a program location, returns the
   * index within 'dense' that holds the thread data */
  re__prog_loc* sparse;
  /* Dense representation: stores each thread based on the order they were
   * added, each thread->prog_loc in this should point back to 'sparse' */
  re__exec_thrd* dense;
  /* Number of elements actually in 'dense' */
  re__prog_loc n;
  /* Allocation size of 'dense' (sparse is a single worst case alloc) */
  re__prog_loc size;
  /* 0 if this does not contain a match instruction, 1+ otherwise */
  re_uint32 match_index;
  /* 0 if the match is the top state, 1+ otherwise */
  re_uint32 match_priority;
  /* Note: (match_priority != 0) implies (match_index != 0) */
} re__exec_thrd_set;

RE__VEC_DECL(re_size);

/* Save state manager for exec. */
typedef struct re__exec_save {
  re_size_vec slots;
  re_int32 last_empty_ref;
  re_uint32 slots_per_thrd;
} re__exec_save;

RE_INTERNAL void re__exec_save_init(re__exec_save* save);
RE_INTERNAL void
re__exec_save_set_slots_per_thrd(re__exec_save* save, re_uint32 slots_per_thrd);
RE_INTERNAL void re__exec_save_destroy(re__exec_save* save);
RE_INTERNAL const re_size*
re__exec_save_get_slots_const(const re__exec_save* save, re_int32 slots_ref);
RE_INTERNAL re_size*
re__exec_save_get_slots(re__exec_save* save, re_int32 slots_ref);
RE_INTERNAL void
re__exec_save_inc_refs(re__exec_save* save, re_int32 slots_ref);
RE_INTERNAL void
re__exec_save_dec_refs(re__exec_save* save, re_int32 slots_ref);
RE_INTERNAL re_size
re__exec_save_get_refs(const re__exec_save* save, re_int32 slots_ref);
RE_INTERNAL re_error
re__exec_save_get_new(re__exec_save* save, re_int32* slots_out_ref);
RE_INTERNAL re_error re__exec_save_do_save(
    re__exec_save* save, re_int32* slots_inout_ref, re_uint32 slot_number,
    re_size data);

typedef unsigned int re__exec_sym;
#define RE__EXEC_SYM_EOT 256
#define RE__EXEC_SYM_MAX 257

/* Execution context. */
typedef struct re__exec_nfa {
  const re__prog* prog;
  re_uint32 num_groups;
  re__exec_thrd_set set_a;
  re__exec_thrd_set set_b;
  re__exec_thrd_set set_c;
  re__exec_thrd_vec thrd_stk;
  re__exec_save save_slots;
} re__exec_nfa;

RE_INTERNAL void re__exec_nfa_init(
    re__exec_nfa* exec, const re__prog* prog, re_uint32 num_groups);
RE_INTERNAL re__prog_loc re__exec_nfa_get_thrds_size(re__exec_nfa* exec);
RE_INTERNAL const re__exec_thrd* re__exec_nfa_get_thrds(re__exec_nfa* exec);
RE_INTERNAL void re__exec_nfa_set_thrds(
    re__exec_nfa* exec, const re__prog_loc* in_thrds,
    re__prog_loc in_thrds_size);
RE_INTERNAL re_uint32 re__exec_nfa_get_match_index(re__exec_nfa* exec);
RE_INTERNAL re_uint32 re__exec_nfa_get_match_priority(re__exec_nfa* exec);
RE_INTERNAL void
re__exec_nfa_set_match_index(re__exec_nfa* exec, re_uint32 match_index);
RE_INTERNAL void
re__exec_nfa_set_match_priority(re__exec_nfa* exec, re_uint32 match_priority);
RE_INTERNAL void re__exec_nfa_destroy(re__exec_nfa* exec);
RE_INTERNAL re_error
re__exec_nfa_start(re__exec_nfa* exec, re__prog_entry entry);
RE_INTERNAL re_error re__exec_nfa_run_byte(
    re__exec_nfa* exec, re__assert_type assert_type, re__exec_sym symbol,
    re_size pos);
RE_INTERNAL re_error
re__exec_nfa_finish(re__exec_nfa* exec, re_span* out, re_size pos);

RE_INTERNAL unsigned int re__is_word_char(re__exec_sym ch);
RE_INTERNAL unsigned int re__is_word_boundary_start(re__exec_sym right);
RE_INTERNAL unsigned int
re__is_word_boundary(int left_is_word, re__exec_sym right);

#if RE_DEBUG

RE_INTERNAL void
re__exec_nfa_debug_dump(const re__exec_nfa* exec, int with_save);

#endif

/* ---------------------------------------------------------------------------
 * DFA execution context (re_exec_dfa.c)
 * ------------------------------------------------------------------------ */
RE__VEC_DECL(re__prog_loc);

#define RE__EXEC_DFA_PAGE_SIZE 4
#define RE__EXEC_DFA_THRD_LOCS_PAGE_SIZE 1024

typedef struct re__exec_dfa_state re__exec_dfa_state;

typedef re__exec_dfa_state* re__exec_dfa_state_ptr;

typedef enum re__exec_dfa_flags {
  RE__EXEC_DFA_FLAG_FROM_WORD = 1,
  RE__EXEC_DFA_FLAG_BEGIN_TEXT = 2,
  RE__EXEC_DFA_FLAG_BEGIN_LINE = 8,
  RE__EXEC_DFA_FLAG_MATCH = 16,
  RE__EXEC_DFA_FLAG_MATCH_PRIORITY = 32
} re__exec_dfa_flags;

/* We can get away with storing information in the lower-order bits of pointers
 * on some (modern) systems. */
/* On a 32-bit system with 4-byte alignment, we get 512 bits of 'free' space,
 * and on a 64-bit system, we get 768 bits. These translate to 64 and 96 bytes
 * to use for whatever we want. Nice! */
/* The main advantage of this is reduced memory consumption. Allocators love it
 * when you allocate a multiple of 1024 / 2048 bytes (the size of 256 pointers
 * on 32/64 bit systems) rather than a multiple of 2096 bytes (the size of this
 * data structure without this trick). Modern arena-based allocators will have
 * no problem binning the states into power-of-2 sized arenas. */
/* The hardest part of this is proving that it's okay to use these bits.
 * It's hard to find an approach that:
 * - Works in C89
 * - Can be done entirely using preprocessor macros
 * - Is reasonably portable across major architectures */
/* Here is how we will use the bits:
 * Idx : Usage
 * [0] : MATCH, MATCH_PRIORITY flags
 * [1] : WORD, BEGIN_TEXT flags
 * [2] : BEGIN_LINE flag
 * [16 - 31] : Match index (32 bits)
 * [32 - 63] : Pointer to threads (32 / 64 bits)
 * [64 - 79] : Threads size (32 bits)
 * [80 - 111] : Pointer to ending transition state */
#define RE__EXEC_DFA_SMALL_STATE 0
#if defined(RE__EXEC_DFA_SMALL_STATE)
#elif (defined(__GNUC__) || defined(__clang__)) &&                             \
    (defined(__arm64__) || defined(__arm__) || defined(__amd64__))
#define RE__EXEC_DFA_SMALL_STATE 1
#elif defined(_MSC_VER) &&                                                     \
    (defined(_M_IX86) || defined(_M_AMD64) || defined(_M_ARM))
#define RE__EXEC_DFA_SMALL_STATE 1
#else
#define RE__EXEC_DFA_SMALL_STATE 0
#endif
struct re__exec_dfa_state {
  re__exec_dfa_state_ptr next[RE__EXEC_SYM_MAX];
#if !RE__EXEC_DFA_SMALL_STATE
  re__exec_dfa_flags flags;
  re_uint32 match_index;
  re_uint32* thrd_locs_begin;
  re_uint32* thrd_locs_end;
#endif
};

typedef struct re__exec_dfa_cache_entry {
  re__exec_dfa_state_ptr state_ptr;
  re_uint32 hash;
} re__exec_dfa_cache_entry;

typedef re_uint32* re_uint32_ptr;

RE__VEC_DECL(re__exec_dfa_state_ptr);
RE__VEC_DECL(re_uint32_ptr);

typedef enum re__exec_dfa_start_state_flags {
  RE__EXEC_DFA_START_STATE_FLAG_AFTER_WORD = 1,
  RE__EXEC_DFA_START_STATE_FLAG_BEGIN_TEXT = 2,
  RE__EXEC_DFA_START_STATE_FLAG_BEGIN_LINE = 4,
  RE__EXEC_DFA_START_STATE_COUNT = 8
} re__exec_dfa_start_state_flags;

typedef struct re__exec_dfa {
  re__exec_dfa_state_ptr current_state;
  re__exec_dfa_state_ptr
      start_states[RE__EXEC_DFA_START_STATE_COUNT * RE__PROG_ENTRY_MAX];
  re__exec_dfa_state_ptr_vec state_pages;
  re_size state_page_idx;
  re_uint32_ptr_vec thrd_loc_pages;
  re_size thrd_loc_page_idx;
  re__exec_nfa nfa;
  /* targets a load factor of 0.75 */
  re__exec_dfa_cache_entry* cache;
  re_size cache_stored;
  re_size cache_alloc;
} re__exec_dfa;

RE_INTERNAL void re__exec_dfa_init(re__exec_dfa* exec, const re__prog* prog);
RE_INTERNAL void re__exec_dfa_destroy(re__exec_dfa* exec);
RE_INTERNAL re_error re__exec_dfa_start(
    re__exec_dfa* exec, re__prog_entry entry,
    re__exec_dfa_start_state_flags start_state_flags);
RE_INTERNAL re_error
re__exec_dfa_run_byte(re__exec_dfa* exec, re_uint8 next_byte);
RE_INTERNAL re_error re__exec_dfa_end(re__exec_dfa* exec);
RE_INTERNAL re_uint32 re__exec_dfa_get_match_index(re__exec_dfa* exec);
RE_INTERNAL re_uint32 re__exec_dfa_get_match_priority(re__exec_dfa* exec);
RE_INTERNAL int re__exec_dfa_get_exhaustion(re__exec_dfa* exec);
RE_INTERNAL void re__exec_dfa_debug_dump(re__exec_dfa* exec);
re_error re__exec_dfa_driver(
    re__exec_dfa* exec, re__prog_entry entry, int boolean_match,
    int boolean_match_exit_early, int reversed, const re_uint8* text,
    re_size text_size, re_size text_start_pos, re_uint32* out_match,
    re_size* out_pos);

/* ---------------------------------------------------------------------------
 * Top-level data (re_api.c)
 * ------------------------------------------------------------------------
 */
/* Internal data structure */
struct re_data {
  re_int32 set;
  re__parse parse;
  re__rune_data rune_data;
  re__ast_root ast_root;
  re__prog program;
  re__prog program_reverse;
  re__compile compile;
  /* Note: error_string_view always points to either a static const char* that
   * is a compile-time constant or a dynamically-allocated const char* inside
   * of error_string. Either way, in OOM situations, we will not allocate more
   * memory to store an error string and default to a constant. */
  re__str error_string;
  re__str_view error_string_view;
};

RE_INTERNAL re_error re__set_error_str(re* re, const re__str* error_str);
RE_INTERNAL void re__set_error_generic(re* re, re_error err);

#endif

/* bits/types/char */
RE__STATIC_ASSERT(re__char_is_one_byte, sizeof(re_char) == 1);

/* bits/container/str */
/* Maximum size, without null terminator */
#define RE__STR_SHORT_SIZE_MAX                                                 \
  (((sizeof(re__str) - sizeof(re_size)) / (sizeof(re_char)) - 1))

#define RE__STR_GET_SHORT(str) !((str)->_size_short & 1)
#define RE__STR_SET_SHORT(str, short)                                          \
  do {                                                                         \
    re_size temp = short;                                                      \
    (str)->_size_short &= ~((re_size)1);                                       \
    (str)->_size_short |= !temp;                                               \
  } while (0)
#define RE__STR_GET_SIZE(str) ((str)->_size_short >> 1)
#define RE__STR_SET_SIZE(str, size)                                            \
  do {                                                                         \
    re_size temp = size;                                                       \
    (str)->_size_short &= 1;                                                   \
    (str)->_size_short |= temp << 1;                                           \
  } while (0)
#define RE__STR_DATA(str)                                                      \
  (RE__STR_GET_SHORT(str) ? ((re_char*)&((str)->_alloc)) : (str)->_data)

/* Round up to multiple of 32 */
#define RE__STR_ROUND_ALLOC(alloc) (((alloc + 1) + 32) & (~((re_size)32)))

#if RE_DEBUG

#define RE__STR_CHECK(str)                                                     \
  do {                                                                         \
    if (RE__STR_GET_SHORT(str)) {                                              \
      /* If string is short, the size must always be less than */              \
      /* RE__STR_SHORT_SIZE_MAX. */                                            \
      RE_ASSERT(RE__STR_GET_SIZE(str) <= RE__STR_SHORT_SIZE_MAX);              \
    } else {                                                                   \
      /* If string is long, the size can still be less, but the other          \
       */                                                                      \
      /* fields must be valid. */                                              \
      /* Ensure there is enough space */                                       \
      RE_ASSERT((str)->_alloc >= RE__STR_GET_SIZE(str));                       \
      /* Ensure that the _data field isn't NULL if the size is 0 */            \
      if (RE__STR_GET_SIZE(str) > 0) {                                         \
        RE_ASSERT((str)->_data != RE_NULL);                                    \
      }                                                                        \
      /* Ensure that if _alloc is 0 then _data is NULL */                      \
      if ((str)->_alloc == 0) {                                                \
        RE_ASSERT((str)->_data == RE_NULL);                                    \
      }                                                                        \
    }                                                                          \
    /* Ensure that there is a null-terminator */                               \
    RE_ASSERT(RE__STR_DATA(str)[RE__STR_GET_SIZE(str)] == '\0');               \
  } while (0)

#else

#define RE__STR_CHECK(str) RE__UNUSED(str)

#endif

void re__str_init(re__str* str)
{
  str->_size_short = 0;
  RE__STR_DATA(str)[0] = '\0';
}

void re__str_destroy(re__str* str)
{
  if (!RE__STR_GET_SHORT(str)) {
    if (str->_data != RE_NULL) {
      RE_FREE(str->_data);
    }
  }
}

re_size re__str_size(const re__str* str) { return RE__STR_GET_SIZE(str); }

RE_INTERNAL int re__str_grow(re__str* str, re_size new_size)
{
  re_size old_size = RE__STR_GET_SIZE(str);
  RE__STR_CHECK(str);
  if (RE__STR_GET_SHORT(str)) {
    if (new_size <= RE__STR_SHORT_SIZE_MAX) {
      /* Can still be a short str */
      RE__STR_SET_SIZE(str, new_size);
    } else {
      /* Needs allocation */
      re_size new_alloc = RE__STR_ROUND_ALLOC(new_size + (new_size >> 1));
      re_char* new_data =
          (re_char*)RE_MALLOC(sizeof(re_char) * (new_alloc + 1));
      re_size i;
      if (new_data == RE_NULL) {
        return -1;
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
      re_size new_alloc = RE__STR_ROUND_ALLOC(new_size + (new_size >> 1));
      re_char* new_data;
      if (str->_alloc == 0) {
        new_data = (re_char*)RE_MALLOC(sizeof(re_char) * (new_alloc + 1));
      } else {
        new_data =
            (re_char*)RE_REALLOC(str->_data, sizeof(re_char) * (new_alloc + 1));
      }
      if (new_data == RE_NULL) {
        return -1;
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

int re__str_push(re__str* str, re_char chr)
{
  int err = 0;
  re_size old_size = RE__STR_GET_SIZE(str);
  if ((err = re__str_grow(str, old_size + 1))) {
    return err;
  }
  RE__STR_DATA(str)[old_size] = chr;
  RE__STR_CHECK(str);
  return err;
}

re_size re__str_slen(const re_char* s)
{
  re_size out = 0;
  while (*(s++)) {
    out++;
  }
  return out;
}

int re__str_init_s(re__str* str, const re_char* s)
{
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

int re__str_init_n(re__str* str, const re_char* chrs, re_size n)
{
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

int re__str_cat(re__str* str, const re__str* other)
{
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

int re__str_cat_n(re__str* str, const re_char* chrs, re_size n)
{
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

int re__str_cat_s(re__str* str, const re_char* chrs)
{
  re_size chrs_size = re__str_slen(chrs);
  return re__str_cat_n(str, chrs, chrs_size);
}

int re__str_insert(re__str* str, re_size index, re_char chr)
{
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

const re_char* re__str_get_data(const re__str* str)
{
  return RE__STR_DATA(str);
}

int re__str_init_copy(re__str* str, const re__str* in)
{
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

void re__str_init_move(re__str* str, re__str* old)
{
  RE__STR_CHECK(old);
  *str = *old;
  re__str_init(old);
}

int re__str_cmp(const re__str* str_a, const re__str* str_b)
{
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

void re__str_clear(re__str* str)
{
  RE__STR_SET_SIZE(str, 0);
  RE__STR_DATA(str)[0] = '\0';
}

/* bits/container/str_view */
void re__str_view_init(re__str_view* view, const re__str* other)
{
  view->_size = re__str_size(other);
  view->_data = re__str_get_data(other);
}

void re__str_view_init_s(re__str_view* view, const re_char* chars)
{
  view->_size = re__str_slen(chars);
  view->_data = chars;
}

void re__str_view_init_n(re__str_view* view, const re_char* chars, re_size n)
{
  view->_size = n;
  view->_data = chars;
}

void re__str_view_init_null(re__str_view* view)
{
  view->_size = 0;
  view->_data = RE_NULL;
}

re_size re__str_view_size(const re__str_view* view) { return view->_size; }

const re_char* re__str_view_get_data(const re__str_view* view)
{
  return view->_data;
}

int re__str_view_cmp(const re__str_view* view_a, const re__str_view* view_b)
{
  re_size a_len = re__str_view_size(view_a);
  re_size b_len = re__str_view_size(view_b);
  const re_char* a_data = re__str_view_get_data(view_a);
  const re_char* b_data = re__str_view_get_data(view_b);
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

/* bits/types/fixed/int32 */
/* If this fails, you need to define RE_INT32_TYPE to a signed integer type
 * that is 32 bits wide. */
RE__STATIC_ASSERT(re__int32_is_4_bytes, sizeof(re_int32) == 4);

/* bits/types/fixed/uint16 */
/* If this fails, you need to define RE_UINT16_TYPE to a unsigned integer type
 * that is 16 bits wide. */
RE__STATIC_ASSERT(re__uint16_is_2_bytes, sizeof(re_uint16) == 2);
/* bits/types/fixed/uint32 */
/* If this fails, you need to define RE_UINT32_TYPE to a unsigned integer type
 * that is 32 bits wide. */
RE__STATIC_ASSERT(re__uint32_is_4_bytes, sizeof(re_uint32) == 4);
/* bits/types/fixed/uint8 */
/* If this fails, you need to define RE_UINT8_TYPE to a unsigned integer type
 * that is 8 bits wide. */
RE__STATIC_ASSERT(re__uint8_is_1_bytes, sizeof(re_uint8) == 1);
/* bits/hooks/memset */
void re__memset(void* ptr, int value, re_size count)
{
  re_size i;
  re_uint8 trunc = (re_uint8)value;
  for (i = 0; i < count; i++) {
    ((re_uint8*)ptr)[i] = trunc;
  }
}

/* bits/algorithm/hash/murmur3 */
RE_INTERNAL re_uint32 re__murmurhash3_rotl32(re_uint32 x, re_uint8 r)
{
  return (x << r) | (x >> (32 - r));
}

RE_INTERNAL re_uint32 re__murmurhash3_fmix32(re_uint32 h)
{
  h ^= h >> 16;
  h *= 0x85EBCA6B;
  h ^= h >> 13;
  h *= 0xC2B2AE35;
  h ^= h >> 16;
  return h;
}

/* Note: this behaves differently on machines with different endians. */
RE_INTERNAL re_uint32
re__murmurhash3_32(re_uint32 h1, const re_uint8* data, re_size data_len)
{
  const re_size num_blocks = data_len / 4;
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
    /* fall through */
  case 2:
    k1 ^= ((re_uint32)(tail[1])) << 8;
    /* fall through */
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

/* re */
RE_INTERNAL void re__ast_init(re__ast* ast, re__ast_type type)
{
  ast->type = type;
  ast->first_child_ref = RE__AST_NONE;
  ast->last_child_ref = RE__AST_NONE;
  ast->prev_sibling_ref = RE__AST_NONE;
  ast->next_sibling_ref = RE__AST_NONE;
}

RE_INTERNAL void re__ast_init_rune(re__ast* ast, re_rune rune)
{
  re__ast_init(ast, RE__AST_TYPE_RUNE);
  ast->_data.rune = rune;
}

RE_INTERNAL void re__ast_init_str(re__ast* ast, re_int32 str_ref)
{
  re__ast_init(ast, RE__AST_TYPE_STR);
  ast->_data.str_ref = str_ref;
}

RE_INTERNAL void re__ast_init_charclass(re__ast* ast, re_int32 charclass_ref)
{
  re__ast_init(ast, RE__AST_TYPE_CHARCLASS);
  ast->_data.charclass_ref = charclass_ref;
}

RE_INTERNAL void re__ast_init_concat(re__ast* ast)
{
  re__ast_init(ast, RE__AST_TYPE_CONCAT);
}

RE_INTERNAL void re__ast_init_alt(re__ast* ast)
{
  re__ast_init(ast, RE__AST_TYPE_ALT);
}

RE_INTERNAL void
re__ast_init_quantifier(re__ast* ast, re_int32 min, re_int32 max)
{
  re__ast_init(ast, RE__AST_TYPE_QUANTIFIER);
  RE_ASSERT(min != max);
  RE_ASSERT(min < max);
  ast->_data.quantifier_info.min = min;
  ast->_data.quantifier_info.max = max;
  ast->_data.quantifier_info.greediness = 1;
}

RE_INTERNAL void re__ast_init_group(
    re__ast* ast, re_uint32 group_idx, re__ast_group_flags group_flags)
{
  re__ast_init(ast, RE__AST_TYPE_GROUP);
  RE_ASSERT(RE__IMPLIES(
      group_flags & RE__AST_GROUP_FLAG_NONMATCHING,
      !(group_flags & RE__AST_GROUP_FLAG_NAMED) && group_idx == 0));
  ast->_data.group_info.flags = group_flags;
  ast->_data.group_info.group_idx = group_idx;
}

RE_INTERNAL void re__ast_init_assert(re__ast* ast, re__assert_type assert_type)
{
  re__ast_init(ast, RE__AST_TYPE_ASSERT);
  ast->_data.assert_type = assert_type;
}

RE_INTERNAL void re__ast_init_any_char(re__ast* ast)
{
  re__ast_init(ast, RE__AST_TYPE_ANY_CHAR);
}

RE_INTERNAL void re__ast_init_any_char_newline(re__ast* ast)
{
  re__ast_init(ast, RE__AST_TYPE_ANY_CHAR_NEWLINE);
}

RE_INTERNAL void re__ast_init_any_byte(re__ast* ast)
{
  re__ast_init(ast, RE__AST_TYPE_ANY_BYTE);
}

RE_INTERNAL void re__ast_destroy(re__ast* ast) { RE__UNUSED(ast); }

RE_INTERNAL void re__ast_set_quantifier_greediness(re__ast* ast, int is_greedy)
{
  RE_ASSERT(ast->type == RE__AST_TYPE_QUANTIFIER);
  ast->_data.quantifier_info.greediness = is_greedy;
}

RE_INTERNAL int re__ast_get_quantifier_greediness(const re__ast* ast)
{
  RE_ASSERT(ast->type == RE__AST_TYPE_QUANTIFIER);
  return ast->_data.quantifier_info.greediness;
}

RE_INTERNAL re_int32 re__ast_get_quantifier_min(const re__ast* ast)
{
  RE_ASSERT(ast->type == RE__AST_TYPE_QUANTIFIER);
  return ast->_data.quantifier_info.min;
}

RE_INTERNAL re_int32 re__ast_get_quantifier_max(const re__ast* ast)
{
  RE_ASSERT(ast->type == RE__AST_TYPE_QUANTIFIER);
  return ast->_data.quantifier_info.max;
}

RE_INTERNAL re_rune re__ast_get_rune(const re__ast* ast)
{
  RE_ASSERT(ast->type == RE__AST_TYPE_RUNE);
  return ast->_data.rune;
}

RE_INTERNAL re__ast_group_flags re__ast_get_group_flags(const re__ast* ast)
{
  RE_ASSERT(ast->type == RE__AST_TYPE_GROUP);
  return ast->_data.group_info.flags;
}

RE_INTERNAL re_uint32 re__ast_get_group_idx(const re__ast* ast)
{
  RE_ASSERT(ast->type == RE__AST_TYPE_GROUP);
  return ast->_data.group_info.group_idx;
}

RE_INTERNAL re__assert_type re__ast_get_assert_type(const re__ast* ast)
{
  RE_ASSERT(ast->type == RE__AST_TYPE_ASSERT);
  return ast->_data.assert_type;
}

RE_INTERNAL re_int32 re__ast_get_str_ref(const re__ast* ast)
{
  RE_ASSERT(ast->type == RE__AST_TYPE_STR);
  return ast->_data.str_ref;
}

RE__ARENA_IMPL_FUNC(re__charclass, init)
RE__ARENA_IMPL_FUNC(re__charclass, destroy)
RE__ARENA_IMPL_FUNC(re__charclass, getref)
RE__ARENA_IMPL_FUNC(re__charclass, getcref)
RE__ARENA_IMPL_FUNC(re__charclass, begin)
RE__ARENA_IMPL_FUNC(re__charclass, next)
RE__ARENA_IMPL_FUNC(re__charclass, add)

RE__ARENA_IMPL_FUNC(re__str, init)
RE__ARENA_IMPL_FUNC(re__str, destroy)
RE__ARENA_IMPL_FUNC(re__str, getref)
RE__ARENA_IMPL_FUNC(re__str, getcref)
RE__ARENA_IMPL_FUNC(re__str, begin)
RE__ARENA_IMPL_FUNC(re__str, next)
RE__ARENA_IMPL_FUNC(re__str, add)

RE__VEC_IMPL_FUNC(re__str, init)
RE__VEC_IMPL_FUNC(re__str, destroy)
RE__VEC_IMPL_FUNC(re__str, push)
RE__VEC_IMPL_FUNC(re__str, getref)
RE__VEC_IMPL_FUNC(re__str, size)

RE__VEC_IMPL_FUNC(re__ast, init)
RE__VEC_IMPL_FUNC(re__ast, destroy)
RE__VEC_IMPL_FUNC(re__ast, get)
RE__VEC_IMPL_FUNC(re__ast, get_data)
RE__VEC_IMPL_FUNC(re__ast, getref)
RE__VEC_IMPL_FUNC(re__ast, getcref)
RE__VEC_IMPL_FUNC(re__ast, insert)
RE__VEC_IMPL_FUNC(re__ast, peek)
RE__VEC_IMPL_FUNC(re__ast, pop)
RE__VEC_IMPL_FUNC(re__ast, push)
RE__VEC_IMPL_FUNC(re__ast, size)

RE_INTERNAL void re__ast_root_init(re__ast_root* ast_root)
{
  re__ast_vec_init(&ast_root->ast_vec);
  ast_root->last_empty_ref = RE__AST_NONE;
  ast_root->root_ref = RE__AST_NONE;
  ast_root->root_last_child_ref = RE__AST_NONE;
  re__charclass_arena_init(&ast_root->charclasses);
  re__str_arena_init(&ast_root->strings);
  re__str_vec_init(&ast_root->group_names);
}

RE_INTERNAL void re__ast_root_destroy(re__ast_root* ast_root)
{
  re_size i;
  re_int32 cur_ref;
  for (i = 0; i < re__str_vec_size(&ast_root->group_names); i++) {
    re__str_destroy(re__str_vec_getref(&ast_root->group_names, i));
  }
  re__str_vec_destroy(&ast_root->group_names);
  cur_ref = re__charclass_arena_begin(&ast_root->charclasses);
  while (cur_ref != RE__ARENA_REF_NONE) {
    re__charclass* cur =
        re__charclass_arena_getref(&ast_root->charclasses, cur_ref);
    re__charclass_destroy(cur);
    cur_ref = re__charclass_arena_next(&ast_root->charclasses, cur_ref);
  }
  re__charclass_arena_destroy(&ast_root->charclasses);
  cur_ref = re__str_arena_begin(&ast_root->strings);
  while (cur_ref != RE__ARENA_REF_NONE) {
    re__str* cur = re__str_arena_getref(&ast_root->strings, cur_ref);
    re__str_destroy(cur);
    cur_ref = re__str_arena_next(&ast_root->strings, cur_ref);
  }
  re__str_arena_destroy(&ast_root->strings);
  for (i = 0; i < re__ast_vec_size(&ast_root->ast_vec); i++) {
    re__ast_destroy(re__ast_vec_getref(&ast_root->ast_vec, i));
  }
  re__ast_vec_destroy(&ast_root->ast_vec);
}

RE_INTERNAL re__ast* re__ast_root_get(re__ast_root* ast_root, re_int32 ast_ref)
{
  RE_ASSERT(ast_ref != RE__AST_NONE);
  RE_ASSERT(ast_ref < (re_int32)re__ast_vec_size(&ast_root->ast_vec));
  return re__ast_vec_getref(&ast_root->ast_vec, (re_size)ast_ref);
}

RE_INTERNAL const re__ast*
re__ast_root_get_const(const re__ast_root* ast_root, re_int32 ast_ref)
{
  RE_ASSERT(ast_ref != RE__AST_NONE);
  RE_ASSERT(ast_ref < (re_int32)re__ast_vec_size(&ast_root->ast_vec));
  return re__ast_vec_getcref(&ast_root->ast_vec, (re_size)ast_ref);
}

RE_INTERNAL void re__ast_root_remove(re__ast_root* ast_root, re_int32 ast_ref)
{
  re__ast* empty = re__ast_root_get(ast_root, ast_ref);
  RE_ASSERT(empty->first_child_ref == RE__AST_NONE);
  RE_ASSERT(empty->last_child_ref == RE__AST_NONE);
  if (ast_root->root_ref == ast_ref) {
    ast_root->root_ref = RE__AST_NONE;
  }
  empty->type = RE__AST_TYPE_NONE;
  empty->next_sibling_ref = ast_root->last_empty_ref;
  ast_root->last_empty_ref = ast_ref;
}

RE_INTERNAL void re__ast_root_replace(
    re__ast_root* ast_root, re_int32 ast_ref, re__ast replacement)
{
  re__ast* loc = re__ast_root_get(ast_root, ast_ref);
  replacement.next_sibling_ref = loc->next_sibling_ref;
  replacement.first_child_ref = loc->first_child_ref;
  replacement.prev_sibling_ref = loc->prev_sibling_ref;
  replacement.last_child_ref = loc->last_child_ref;
  *loc = replacement;
}

RE_INTERNAL re_error
re__ast_root_new(re__ast_root* ast_root, re__ast ast, re_int32* out_ref)
{
  re_error err = RE_ERROR_NONE;
  re_int32 empty_ref = ast_root->last_empty_ref;
  if (empty_ref != RE__AST_NONE) {
    re__ast* empty = re__ast_root_get(ast_root, empty_ref);
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

RE_INTERNAL re_error re__ast_root_add_child(
    re__ast_root* ast_root, re_int32 parent_ref, re__ast ast, re_int32* out_ref)
{
  re_error err = RE_ERROR_NONE;
  re_int32 prev_sibling_ref = RE__AST_NONE;
  re__ast* prev_sibling;
  re__ast* out;
  if ((err = re__ast_root_new(ast_root, ast, out_ref))) {
    return err;
  }
  if (parent_ref == RE__AST_NONE) {
    if (ast_root->root_ref == RE__AST_NONE) {
      /* If this is the first child, set root to it. */
      ast_root->root_ref = *out_ref;
      ast_root->root_last_child_ref = *out_ref;
    } else {
      prev_sibling_ref = ast_root->root_last_child_ref;
      ast_root->root_last_child_ref = *out_ref;
    }
  } else {
    re__ast* parent = re__ast_root_get(ast_root, parent_ref);
    if (parent->first_child_ref == RE__AST_NONE) {
      parent->first_child_ref = *out_ref;
      parent->last_child_ref = *out_ref;
    } else {
      prev_sibling_ref = parent->last_child_ref;
      parent->last_child_ref = *out_ref;
    }
  }
  if (prev_sibling_ref != RE__AST_NONE) {
    prev_sibling = re__ast_root_get(ast_root, prev_sibling_ref);
    prev_sibling->next_sibling_ref = *out_ref;
    out = re__ast_root_get(ast_root, *out_ref);
    out->prev_sibling_ref = prev_sibling_ref;
  }
  return err;
}
/*
RE_INTERNAL re_error re__ast_root_add_sibling(re__ast_root* ast_root, re_int32
prev_sibling_ref, re__ast ast, re_int32* out_ref) { re_error err =
RE_ERROR_NONE; if ((err = re__ast_root_new(ast_root, ast, out_ref))) { return
err;
    }
    RE_ASSERT(prev_sibling_ref != RE__AST_NONE);
    {
        re__ast* prev = re__ast_root_get(ast_root, prev_sibling_ref);
        re__ast* out = re__ast_root_get(ast_root, *out_ref);
        prev->next_sibling_ref = *out_ref;
        out->prev_sibling_ref = prev_sibling_ref;
    }
    return err;
}*/

RE_INTERNAL re_error re__ast_root_add_wrap(
    re__ast_root* ast_root, re_int32 parent_ref, re_int32 inner_ref,
    re__ast ast_outer, re_int32* out_ref)
{
  re_error err = RE_ERROR_NONE;
  re__ast* inner;
  re__ast* outer;
  re__ast* parent;
  if ((err = re__ast_root_new(ast_root, ast_outer, out_ref))) {
    return err;
  }
  inner = re__ast_root_get(ast_root, inner_ref);
  outer = re__ast_root_get(ast_root, *out_ref);
  if (parent_ref != RE__AST_NONE) {
    parent = re__ast_root_get(ast_root, parent_ref);
  }
  if (inner->prev_sibling_ref == RE__AST_NONE) {
    if (parent_ref == RE__AST_NONE) {
      ast_root->root_ref = *out_ref;
    } else {
      parent->first_child_ref = *out_ref;
    }
  } else {
    re__ast* inner_prev_sibling =
        re__ast_root_get(ast_root, inner->prev_sibling_ref);
    inner_prev_sibling->next_sibling_ref = *out_ref;
    outer->prev_sibling_ref = inner->prev_sibling_ref;
  }
  if (inner->next_sibling_ref == RE__AST_NONE) {
    if (parent_ref == RE__AST_NONE) {
      ast_root->root_last_child_ref = *out_ref;
    } else {
      parent->last_child_ref = *out_ref;
    }
  } else {
    re__ast* inner_next_sibling =
        re__ast_root_get(ast_root, inner->next_sibling_ref);
    inner_next_sibling->prev_sibling_ref = *out_ref;
    outer->next_sibling_ref = inner->next_sibling_ref;
  }
  inner->prev_sibling_ref = RE__AST_NONE;
  inner->next_sibling_ref = RE__AST_NONE;
  outer->first_child_ref = inner_ref;
  outer->last_child_ref = inner_ref;
  return err;
}

RE_INTERNAL re_error re__ast_root_add_charclass(
    re__ast_root* ast_root, re__charclass charclass, re_int32* out_ref)
{
  return re__charclass_arena_add(&ast_root->charclasses, charclass, out_ref);
}

RE_INTERNAL const re__charclass*
re__ast_root_get_charclass(const re__ast_root* ast_root, re_int32 charclass_ref)
{
  return re__charclass_arena_getcref(&ast_root->charclasses, charclass_ref);
}

RE_INTERNAL re_error
re__ast_root_add_str(re__ast_root* ast_root, re__str str, re_int32* out_ref)
{
  return re__str_arena_add(&ast_root->strings, str, out_ref);
}

RE_INTERNAL re__str*
re__ast_root_get_str(re__ast_root* ast_root, re_int32 str_ref)
{
  return re__str_arena_getref(&ast_root->strings, str_ref);
}

RE_INTERNAL re__str_view
re__ast_root_get_str_view(const re__ast_root* ast_root, re_int32 str_ref)
{
  re__str_view out;
  const re__str* src = re__str_arena_getcref(&ast_root->strings, str_ref);
  re__str_view_init(&out, src);
  return out;
}

RE_INTERNAL re_error
re__ast_root_add_group(re__ast_root* ast_root, re__str_view group_name)
{
  re__str new_str;
  re_error err = re__str_init_n(
      &new_str, re__str_view_get_data(&group_name),
      re__str_view_size(&group_name));
  if (err) {
    return err;
  }
  return re__str_vec_push(&ast_root->group_names, new_str);
}

RE_INTERNAL re__str_view
re__ast_root_get_group(re__ast_root* ast_root, re_uint32 group_number)
{
  re__str_view view;
  re__str_view_init(
      &view, re__str_vec_getref(&ast_root->group_names, (re_size)group_number));
  return view;
}

RE_INTERNAL re_uint32 re__ast_root_get_num_groups(re__ast_root* ast_root)
{
  return (re_uint32)re__str_vec_size(&ast_root->group_names);
}

RE__VEC_DECL(re_int32);
RE__VEC_IMPL_FUNC(re_int32, init)
RE__VEC_IMPL_FUNC(re_int32, destroy)
RE__VEC_IMPL_FUNC(re_int32, push)
RE__VEC_IMPL_FUNC(re_int32, get)
RE__VEC_IMPL_FUNC(re_int32, size)
RE__VEC_IMPL_FUNC(re_int32, pop)

RE_INTERNAL re_error
re__ast_root_get_depth(const re__ast_root* ast_root, re_int32* depth)
{
  re_error err = RE_ERROR_NONE;
  re_int32_vec stk;
  re_int32 max_depth = 0;
  re_int32_vec_init(&stk);
  if ((err = re_int32_vec_push(&stk, ast_root->root_ref))) {
    goto error;
  }
  while (re_int32_vec_size(&stk)) {
    re_size cur_depth_sz = re_int32_vec_size(&stk);
    re_int32 top_child_ref = re_int32_vec_pop(&stk);
    max_depth = RE__MAX((re_int32)(cur_depth_sz), max_depth);
    if (top_child_ref != RE__AST_NONE) {
      const re__ast* top_child =
          re__ast_root_get_const(ast_root, top_child_ref);
      /* this will never fire an error */
      re_int32_vec_push(&stk, top_child->next_sibling_ref);
      if (top_child->first_child_ref != RE__AST_NONE) {
        /* child has children */
        if ((err = re_int32_vec_push(&stk, top_child->first_child_ref))) {
          goto error;
        }
      }
    }
  }
  *depth = max_depth;
error:
  re_int32_vec_destroy(&stk);
  return err;
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

#endif

#include <stdio.h>

RE_INTERNAL void re__ast_root_debug_dump(
    const re__ast_root* ast_root, re_int32 root_ref, re_int32 lvl)
{
  re_int32 i;
  while (root_ref != RE__AST_NONE) {
    const re__ast* ast = re__ast_root_get_const(ast_root, root_ref);
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
    case RE__AST_TYPE_STR: {
      re__str_view str_view =
          re__ast_root_get_str_view(ast_root, ast->_data.str_ref);
      printf("STR: \"%s\"", re__str_view_get_data(&str_view));
      break;
    }
    case RE__AST_TYPE_CHARCLASS: {
      const re__charclass* charclass;
      printf("CLASS:\n");
      charclass = re__charclass_arena_getcref(
          &ast_root->charclasses, ast->_data.charclass_ref);
      re__charclass_dump(charclass, (re_size)(lvl + 1));
      break;
    }
    case RE__AST_TYPE_CONCAT:
      printf("CONCAT");
      break;
    case RE__AST_TYPE_ALT:
      printf("ALT");
      break;
    case RE__AST_TYPE_QUANTIFIER:
      printf(
          "QUANTIFIER: %i - %i; %s", ast->_data.quantifier_info.min,
          ast->_data.quantifier_info.max,
          ast->_data.quantifier_info.greediness ? "greedy" : "non-greedy");
      break;
    case RE__AST_TYPE_GROUP:
      printf(
          "GROUP: %c%c",
          ((ast->_data.group_info.flags & RE__AST_GROUP_FLAG_NONMATCHING)
               ? ':'
               : ' '),
          ((ast->_data.group_info.flags & RE__AST_GROUP_FLAG_NAMED) ? 'P'
                                                                    : ' '));
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

#if 0
RE_INTERNAL int re__ast_root_verify_depth(const re__ast_root* ast_root, re_int32 start_ref, re_int32 depth) {
    const re__ast* cur_node;
    if (depth == 0) {
        if (start_ref == RE__AST_NONE) {
            return 1;
        } else {
            return 0;
        }
    } else {
        while (start_ref != RE__AST_NONE) {
            cur_node = re__ast_root_get_const(ast_root, start_ref);
            if (!re__ast_root_verify_depth(ast_root, cur_node->first_child_ref, depth - 1)) {
                return 0;
            }
            start_ref = cur_node->next_sibling_ref;
        }
    }
    return 1;
}

RE_INTERNAL int re__ast_root_verify(const re__ast_root* ast_root) {
    re_int32_vec removed_list;
    re_int32_vec_init(&removed_list);
    if (ast_root->last_empty_ref != RE__AST_NONE) {
        re_int32 empty_ref = ast_root->last_empty_ref;
        while (1) {
            if (empty_ref == RE__AST_NONE) {
                break;
            }
            if (empty_ref >= re__ast_root_size(ast_root)) {
                /* empty refs can't exceed size */
                return 0;
            }
            {
                re_size i;
                /* no cycles in empty list */
                for (i = 0; i < re_int32_vec_size(&removed_list); i++) {
                    if (re_int32_vec_get(&removed_list, i) == empty_ref) {
                        return 0;
                    }
                }
            }
            {
                const re__ast* empty = re__ast_root_get_const(ast_root,empty_ref);
                re_int32_vec_push(&removed_list, empty_ref);
                empty_ref = empty->next_sibling_ref;
            }
        }
    }
    {
        re_int32 i = 0;
        for (i = 0; i < re__ast_root_size(ast_root); i++) {
            re_size j;
            /* don't loop over empty nodes */
            for (j = 0; j < re_int32_vec_size(&removed_list); j++) {
                if (re_int32_vec_get(&removed_list, j) == i) {
                    goto cont;
                }
            }
            {
                const re__ast* ast = re__ast_root_get_const(ast_root, i);
                re_int32 to_check[3];
                re_int32 k;
                to_check[0] = ast->prev_sibling_ref;
                to_check[1] = ast->next_sibling_ref;
                to_check[2] = ast->first_child_ref;
                for (k = 0; k < 3; k++) {
                    re_int32 candidate = to_check[k];
                    for (j = 0; j < re_int32_vec_size(&removed_list); j++) {
                        if (re_int32_vec_get(&removed_list, j) == candidate) {
                            /* node points to a removed node */
                            return 0;
                        }
                    }
                }
            }
            if (0) {
cont:
                continue;
            }
        }
    }
    re_int32_vec_destroy(&removed_list);
    return re__ast_root_verify_depth(ast_root, ast_root->root_ref, ast_root->depth_max);
}
#endif

#endif

/* re */
RE_INTERNAL void re__error_init(re* reg)
{
  re__str_init(&reg->data->error_string);
  re__str_view_init_null(&reg->data->error_string_view);
}

RE_INTERNAL void re__error_destroy(re* reg)
{
  re__str_destroy(&reg->data->error_string);
}

/* Doesn't take ownership of error_str */
/* We don't return an error here because it's expected that you are already in
 * the middle of cleaning up after an error */
/* This is useful because it allows error_string to be a const pointer,
 * allowing error messages to be saved as just const strings */
RE_INTERNAL re_error re__set_error_str(re* reg, const re__str* error_str)
{
  re_error err = RE_ERROR_NONE;
  /* Clear the last error */
  re__error_destroy(reg);
  if ((err = re__str_init_copy(&reg->data->error_string, error_str))) {
    re__set_error_generic(reg, err);
  } else {
    re__str_view_init(&reg->data->error_string_view, &reg->data->error_string);
  }
  return err;
}

RE_INTERNAL void re__set_error_generic(re* reg, re_error err)
{
  if (err == RE_ERROR_NOMEM) {
    /* No memory: not guaranteed if we can allocate a string. */
    re__str_view_init_s(&reg->data->error_string_view, "out of memory");
  } else {
    /* This function is only valid for generic errors */
    RE__ASSERT_UNREACHED();
  }
}

re_error re__init(re* reg, int is_set)
{
  re_error err = RE_ERROR_NONE;
  reg->data = (re_data*)RE_MALLOC(sizeof(re_data));
  if (!reg->data) {
    return RE_ERROR_NOMEM;
  }
  reg->data->set = RE__AST_NONE;
  re__error_init(reg);
  re__rune_data_init(&reg->data->rune_data);
  re__parse_init(&reg->data->parse, reg);
  re__ast_root_init(&reg->data->ast_root);
  re__prog_init(&reg->data->program);
  re__prog_init(&reg->data->program_reverse);
  re__compile_init(&reg->data->compile);
  if (is_set) {
    re__ast alt;
    re__ast_init_alt(&alt);
    if ((err = re__ast_root_add_child(
             &reg->data->ast_root, reg->data->ast_root.root_ref, alt,
             &reg->data->set))) {
      return err;
    }
  }
  return err;
}

re_error re_init(re* reg, const char* regex_nt)
{
  return re_init_sz_flags(reg, regex_nt, re__str_slen(regex_nt), 0);
}

re_error
re_init_flags(re* reg, const char* regex_nt, re_syntax_flags syntax_flags)
{
  return re_init_sz_flags(reg, regex_nt, re__str_slen(regex_nt), syntax_flags);
}

re_error re_init_sz_flags(
    re* reg, const char* regex, re_size regex_size,
    re_syntax_flags syntax_flags)
{
  re_error err = RE_ERROR_NONE;
  re__str_view regex_view;
  if ((err = re__init(reg, 0))) {
    return err;
  }
  re__str_view_init_n(&regex_view, regex, regex_size);
  if ((err = re__parse_str(&reg->data->parse, regex_view, syntax_flags))) {
    return err;
  }
  return err;
}

re_error re_init_set(re* reg) { return re__init(reg, 1); }

re_error re_set_add(re* reg, const char* regex_nt)
{
  return re_set_add_flags(reg, regex_nt, 0);
}

re_error
re_set_add_flags(re* reg, const char* regex_nt, re_syntax_flags syntax_flags)
{
  return re_set_add_sz_flags(
      reg, regex_nt, re__str_slen(regex_nt), syntax_flags);
}

re_error re_set_add_sz_flags(
    re* reg, const char* regex, re_size regex_size,
    re_syntax_flags syntax_flags)
{
  re__str_view regex_view;
  RE__UNUSED(syntax_flags);
  re__str_view_init_n(&regex_view, regex, regex_size);
  return re__parse_str(&reg->data->parse, regex_view, syntax_flags);
}

void re_destroy(re* reg)
{
  if (reg->data == RE_NULL) {
    return;
  }
  re__compile_destroy(&reg->data->compile);
  re__prog_destroy(&reg->data->program);
  re__prog_destroy(&reg->data->program_reverse);
  re__ast_root_destroy(&reg->data->ast_root);
  re__rune_data_destroy(&reg->data->rune_data);
  re__parse_destroy(&reg->data->parse);
  re__error_destroy(reg);
  if (reg->data) {
    RE_FREE(reg->data);
  }
}

const char* re_get_error(const re* reg, re_size* error_len)
{
  if (error_len != RE_NULL) {
    *error_len = re__str_view_size(&reg->data->error_string_view);
  }
  return (const char*)re__str_view_get_data(&reg->data->error_string_view);
}

re_uint32 re_get_max_groups(const re* reg)
{
  return re__ast_root_get_num_groups(&reg->data->ast_root);
}

RE_INTERNAL re__assert_type re__match_next_assert_ctx(re_size pos, re_size len)
{
  re__assert_type out = 0;
  if (pos == 0) {
    out |= RE__ASSERT_TYPE_TEXT_START_ABSOLUTE | RE__ASSERT_TYPE_TEXT_START;
  }
  if (pos == len) {
    out |= RE__ASSERT_TYPE_TEXT_END_ABSOLUTE | RE__ASSERT_TYPE_TEXT_END;
  }
  return out;
}

re_error re__match_prepare_progs(
    re* reg, int fwd, int rev, int fwd_dotstar, int rev_dotstar)
{
  re_error err = RE_ERROR_NONE;
  RE_ASSERT(RE__IMPLIES(fwd_dotstar, fwd));
  RE_ASSERT(RE__IMPLIES(rev_dotstar, rev));
  if (fwd) {
    if (!re__prog_size(&reg->data->program)) {
      if ((err = re__compile_regex(
               &reg->data->compile, &reg->data->ast_root, &reg->data->program,
               0, reg->data->set))) {
        return err;
      }
    }
  }
  if (fwd_dotstar) {
    if (re__prog_get_entry(&reg->data->program, RE__PROG_ENTRY_DOTSTAR) ==
        RE__PROG_LOC_INVALID) {
      if ((err = re__compile_dotstar(
               &reg->data->program, RE__PROG_DATA_ID_DOT_FWD_REJSURR_REJNL))) {
        return err;
      }
    }
  }
  if (rev) {
    if (!re__prog_size(&reg->data->program_reverse)) {
      if ((err = re__compile_regex(
               &reg->data->compile, &reg->data->ast_root,
               &reg->data->program_reverse, 1, reg->data->set))) {
        return err;
      }
    }
  }
  if (rev_dotstar) {
    if (re__prog_get_entry(
            &reg->data->program_reverse, RE__PROG_ENTRY_DOTSTAR) ==
        RE__PROG_LOC_INVALID) {
      if ((err = re__compile_dotstar(
               &reg->data->program_reverse,
               RE__PROG_DATA_ID_DOT_REV_REJSURR_REJNL))) {
        return err;
      }
    }
  }
  return err;
}

#if RE_DEBUG

#include <stdio.h>

#endif

#if 0

re_error re__match_dfa_driver(
    re__prog* program, re__prog_entry entry,
    int request, /* 0 for boolean, 1 for match pos + index */
    int bool_bail, int reversed, re_size start_pos, const char* text,
    re_size text_size, re_uint32* out_match, re_size* out_pos)
{
  re_size pos;
  re_error err = RE_ERROR_NONE;
  re__exec_dfa exec_dfa;
  re__exec_dfa_start_state_flags start_state_flags = 0;
  re_size last_found_pos = 0;
  re_uint32 last_found_match = 0;
  re_uint32 match_status = 0;
  if (!reversed) {
    if (start_pos == 0) {
      start_state_flags |= RE__EXEC_DFA_START_STATE_FLAG_BEGIN_TEXT |
                           RE__EXEC_DFA_START_STATE_FLAG_BEGIN_LINE;
    } else {
      start_state_flags |=
          (RE__EXEC_DFA_START_STATE_FLAG_AFTER_WORD *
           re__is_word_char((unsigned char)(text[start_pos - 1])));
      start_state_flags |= RE__EXEC_DFA_START_STATE_FLAG_BEGIN_LINE *
                           (text[start_pos - 1] == '\n');
    }
  } else {
    if (start_pos == text_size) {
      start_state_flags |= RE__EXEC_DFA_START_STATE_FLAG_BEGIN_TEXT |
                           RE__EXEC_DFA_START_STATE_FLAG_BEGIN_LINE;
    } else {
      start_state_flags |=
          (RE__EXEC_DFA_START_STATE_FLAG_AFTER_WORD *
           re__is_word_char((unsigned char)(text[start_pos])));
      start_state_flags |=
          RE__EXEC_DFA_START_STATE_FLAG_BEGIN_LINE * (text[start_pos] == '\n');
    }
  }
  re__exec_dfa_init(&exec_dfa, program);
  if ((err = re__exec_dfa_start(&exec_dfa, entry, start_state_flags))) {
    goto err_destroy_dfa;
  }
  if (!reversed) {
    pos = start_pos;
  } else {
    pos = text_size - start_pos;
  }
  RE_ASSERT(pos <= text_size);
  RE_ASSERT(RE__IMPLIES(request, out_match != RE_NULL));
  RE_ASSERT(RE__IMPLIES(request, out_pos != RE_NULL));
  RE_ASSERT(RE__IMPLIES(bool_bail, !request));
  while (pos < text_size) {
    unsigned char ch = 0;
    if (!reversed) {
      ch = (unsigned char)(text[pos]);
    } else {
      ch = (unsigned char)(text[(text_size - pos) - 1]);
    }
    if ((err = re__exec_dfa_run_byte(&exec_dfa, ch))) {
      goto err_destroy_dfa;
    }
    if (request == 0) {
      if (bool_bail && re__exec_dfa_get_match_index(&exec_dfa)) {
        goto match_early_boolean;
      }
    } else {
      match_status = re__exec_dfa_get_match_index(&exec_dfa);
      if (match_status) {
        last_found_match = match_status;
        last_found_pos = pos;
        if (re__exec_dfa_get_match_priority(&exec_dfa) == 0) {
          goto match_early_priority;
        }
      } else {
        if (re__exec_dfa_get_exhaustion(&exec_dfa)) {
          if (last_found_match) {
            goto match_early_priority;
          }
        }
      }
    }
    pos++;
  }
  if ((err = re__exec_dfa_end(&exec_dfa))) {
    goto err_destroy_dfa;
  }
  if ((last_found_match = re__exec_dfa_get_match_index(&exec_dfa))) {
    last_found_pos = pos;
    if (request) {
      RE_ASSERT(last_found_pos <= text_size);
      if (!reversed) {
        *out_pos = last_found_pos;
      } else {
        *out_pos = text_size - last_found_pos;
      }
    }
  } else {
    err = RE_NOMATCH;
  }
  re__exec_dfa_destroy(&exec_dfa);
  return err;
match_early_boolean:
  re__exec_dfa_destroy(&exec_dfa);
  return RE_MATCH;
match_early_priority:
  re__exec_dfa_destroy(&exec_dfa);
  RE_ASSERT(last_found_pos <= text_size);
  if (!reversed) {
    *out_pos = last_found_pos;
  } else {
    *out_pos = text_size - last_found_pos;
  }
  *out_match = last_found_match;
  return RE_MATCH;
err_destroy_dfa:
  re__exec_dfa_destroy(&exec_dfa);
  return err;
}

#endif

re_error re__match_dfa_driver(
    re__prog* program, re__prog_entry entry,
    int request, /* 0 for boolean, 1 for match pos + index */
    int bool_bail, int reversed, re_size start_pos, const char* text,
    re_size text_size, re_uint32* out_match, re_size* out_pos)
{
  re__exec_dfa exec;
  re_error err = RE_ERROR_NONE;
  re__exec_dfa_init(&exec, program);
  if ((err = re__exec_dfa_driver(
           &exec, entry, !request, bool_bail, reversed, (const re_uint8*)text,
           text_size, start_pos, out_match, out_pos))) {
    goto error;
  }
error:
  re__exec_dfa_destroy(&exec);
  return err;
}

re_error re_is_match(
    re* reg, const char* text, re_size text_size, re_anchor_type anchor_type)
{
  /* no groups -- dfa can be used in all cases */
  re_error err = RE_ERROR_NONE;
  if (anchor_type == RE_ANCHOR_BOTH) {
    if ((err = re__match_prepare_progs(reg, 1, 0, 0, 0))) {
      return err;
    }
    return re__match_dfa_driver(
        &reg->data->program, RE__PROG_ENTRY_DEFAULT, 0, 0, 0, 0, text,
        text_size, RE_NULL, RE_NULL);
  } else if (anchor_type == RE_ANCHOR_START) {
    if ((err = re__match_prepare_progs(reg, 1, 0, 0, 0))) {
      return err;
    }
    return re__match_dfa_driver(
        &reg->data->program, RE__PROG_ENTRY_DEFAULT, 0, 1, 0, 0, text,
        text_size, RE_NULL, RE_NULL);
  } else if (anchor_type == RE_ANCHOR_END) {
    if ((err = re__match_prepare_progs(reg, 0, 1, 0, 0))) {
      return err;
    }
    return re__match_dfa_driver(
        &reg->data->program_reverse, RE__PROG_ENTRY_DEFAULT, 0, 1, 1, text_size,
        text, text_size, RE_NULL, RE_NULL);
  } else if (anchor_type == RE_UNANCHORED) {
    if ((err = re__match_prepare_progs(reg, 1, 0, 1, 0))) {
      return err;
    }
    return re__match_dfa_driver(
        &reg->data->program, RE__PROG_ENTRY_DOTSTAR, 0, 1, 0, 0, text,
        text_size, RE_NULL, RE_NULL);
  } else {
    return RE_ERROR_INVALID;
  }
}

re_error re_match_groups(
    re* reg, const char* text, re_size text_size, re_anchor_type anchor_type,
    re_uint32 max_group, re_span* out_groups)
{
  re_error err = RE_ERROR_NONE;
  if (max_group == 0) {
    return re_is_match(reg, text, text_size, anchor_type);
  } else if (max_group == 1) {
    re_uint32 out_match;
    re_size out_pos;
    re_error match_err;
    if (anchor_type == RE_ANCHOR_BOTH) {
      if ((err = re__match_prepare_progs(reg, 1, 0, 0, 0))) {
        return err;
      }
      match_err = re__match_dfa_driver(
          &reg->data->program, RE__PROG_ENTRY_DEFAULT, 1, 0, 0, 0, text,
          text_size, &out_match, &out_pos);
      if (match_err == RE_MATCH) {
        if (out_pos != text_size) {
          return RE_NOMATCH;
        } else {
          out_groups[0].begin = 0;
          out_groups[0].end = out_pos;
          return RE_MATCH;
        }
      } else {
        return match_err;
      }
    } else if (anchor_type == RE_ANCHOR_START) {
      if ((err = re__match_prepare_progs(reg, 1, 0, 0, 0))) {
        return err;
      }
      match_err = re__match_dfa_driver(
          &reg->data->program, RE__PROG_ENTRY_DEFAULT, 1, 0, 0, 0, text,
          text_size, &out_match, &out_pos);
      if (match_err == RE_MATCH) {
        out_groups[0].begin = 0;
        out_groups[0].end = out_pos;
        return RE_MATCH;
      } else {
        return match_err;
      }
    } else if (anchor_type == RE_ANCHOR_END) {
      if ((err = re__match_prepare_progs(reg, 0, 1, 0, 0))) {
        return err;
      }
      match_err = re__match_dfa_driver(
          &reg->data->program_reverse, RE__PROG_ENTRY_DEFAULT, 1, 0, 1,
          text_size, text, text_size, &out_match, &out_pos);
      if (match_err == RE_MATCH) {
        out_groups[0].begin = out_pos;
        out_groups[0].end = text_size;
        return RE_MATCH;
      } else {
        return match_err;
      }
    } else if (anchor_type == RE_UNANCHORED) {
      if ((err = re__match_prepare_progs(reg, 1, 1, 1, 0))) {
        return err;
      }
      match_err = re__match_dfa_driver(
          &reg->data->program, RE__PROG_ENTRY_DOTSTAR, 1, 0, 0, 0, text,
          text_size, &out_match, &out_pos);
      if (match_err == RE_MATCH) {
        out_groups[0].end = out_pos;
        /* scan back to find the start */
        match_err = re__match_dfa_driver(
            &reg->data->program_reverse, RE__PROG_ENTRY_DEFAULT, 1, 0, 1,
            out_pos, text, text_size, &out_match, &out_pos);
        /* should ALWAYS match. */
        if (match_err != RE_MATCH) {
          return match_err;
        }
        RE_ASSERT(match_err == RE_MATCH);
        out_groups[0].begin = out_pos;
        return RE_MATCH;
      } else {
        return match_err;
      }
    } else {
      return RE_ERROR_INVALID;
    }
  } else {
    re__exec_nfa exec_nfa;
    re__str_view string_view;
    re__exec_nfa_init(&exec_nfa, &reg->data->program, max_group + 1);
    re__str_view_init_n(&string_view, text, text_size);
    {
      re_size pos;
      re__assert_type assert_ctx =
          RE__ASSERT_TYPE_TEXT_START | RE__ASSERT_TYPE_TEXT_START_ABSOLUTE;
      if ((err = re__exec_nfa_start(&exec_nfa, 0))) {
        goto error;
      }
      for (pos = 0; pos < re__str_view_size(&string_view); pos++) {
        re_uint8 ch = (re_uint8)re__str_view_get_data(&string_view)[pos];
        re_uint32 match;
        assert_ctx =
            re__match_next_assert_ctx(pos, re__str_view_size(&string_view));
        if ((err = re__exec_nfa_run_byte(&exec_nfa, assert_ctx, ch, pos))) {
          return err;
        }
        match = re__exec_nfa_get_match_index(&exec_nfa);
        if (match) {
          if (anchor_type == RE_ANCHOR_BOTH || anchor_type == RE_ANCHOR_END) {
            /* can bail! */
            break;
          }
        }
      }
      if ((err = re__exec_nfa_run_byte(
               &exec_nfa, assert_ctx, RE__EXEC_SYM_EOT, pos))) {
        return err;
      }
      err = re__exec_nfa_finish(&exec_nfa, out_groups, pos);
    }
  error:
    re__exec_nfa_destroy(&exec_nfa);
    return err;
  }
}

/* re */
RE__VEC_IMPL_FUNC(re__rune_range, init)
RE__VEC_IMPL_FUNC(re__rune_range, destroy)
RE__VEC_IMPL_FUNC(re__rune_range, push)
RE__VEC_IMPL_FUNC(re__rune_range, get)
RE__VEC_IMPL_FUNC(re__rune_range, set)
RE__VEC_IMPL_FUNC(re__rune_range, insert)
RE__VEC_IMPL_FUNC(re__rune_range, size)
RE__VEC_IMPL_FUNC(re__rune_range, clear)
RE__VEC_IMPL_FUNC(re__rune_range, get_data)

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
    {6, "xdigit", 3, (unsigned char*)"\x30\x39\x41\x46\x61\x66"}};

#define RE__CHARCLASS_ASCII_DEFAULTS_SIZE                                      \
  (sizeof(re__charclass_ascii_defaults) / sizeof(re__charclass_ascii))

RE_INTERNAL void re__charclass_destroy(re__charclass* charclass)
{
  if (charclass->ranges) {
    RE_FREE(charclass->ranges);
  }
}

RE_INTERNAL const re__rune_range*
re__charclass_get_ranges(const re__charclass* charclass)
{
  return charclass->ranges;
}

RE_INTERNAL re_size re__charclass_size(const re__charclass* charclass)
{
  return charclass->ranges_size;
}

RE_INTERNAL const re__charclass_ascii*
re__charclass_ascii_find(re__str_view name_view)
{
  re_size i;
  const re__charclass_ascii* found = RE_NULL;
  /* Search table for the matching named character class */
  for (i = 0; i < RE__CHARCLASS_ASCII_DEFAULTS_SIZE; i++) {
    const re__charclass_ascii* cur = &re__charclass_ascii_defaults[i];
    re__str_view temp_view;
    re__str_view_init_n(
        &temp_view, (re_char*)cur->name, (re_size)cur->name_size);
    if (re__str_view_cmp(&temp_view, &name_view) == 0) {
      found = cur;
      break;
    }
  }
  return found;
}

typedef struct re__charclass_inverter {
  int should_invert;
  re__rune_range temp;
  re_rune last_max;
} re__charclass_inverter;

void re__charclass_inverter_init(
    re__charclass_inverter* inverter, int should_invert)
{
  inverter->should_invert = should_invert;
  inverter->temp.min = 0;
  inverter->temp.max = 0;
  inverter->last_max = -1;
}
void re__charclass_inverter_push(
    re__charclass_inverter* inverter, re__rune_range range)
{
  if (inverter->should_invert) {
    if (inverter->last_max == -1) {
      inverter->temp.min = 0;
    } else {
      inverter->temp.min = inverter->last_max;
    }
    inverter->temp.max = range.min - 1;
    inverter->last_max = range.max + 1;
  } else {
    inverter->temp.min = range.min;
    inverter->temp.max = range.max;
  }
}
void re__charclass_inverter_end(re__charclass_inverter* inverter)
{
  if (inverter->should_invert) {
    if (inverter->last_max == -1) {
      inverter->temp.min = 0;
      inverter->temp.max = RE_RUNE_MAX;
    } else {
      inverter->temp.min = inverter->last_max;
      inverter->temp.max = RE_RUNE_MAX;
    }
  } else {
    inverter->last_max = 0;
  }
}

int re__charclass_inverter_hasnext(re__charclass_inverter* inverter)
{
  if (inverter->should_invert) {
    return inverter->temp.max >= inverter->temp.min;
  } else {
    return inverter->last_max == -1;
  }
}

re__rune_range re__charclass_inverter_next(re__charclass_inverter* inverter)
{
  return inverter->temp;
}

RE_INTERNAL void re__charclass_builder_init(
    re__charclass_builder* builder, re__rune_data* rune_data)
{
  re__rune_range_vec_init(&builder->ranges);
  builder->should_invert = 0;
  builder->should_fold = 0;
  builder->highest = -1;
  builder->rune_data = rune_data;
}

RE_INTERNAL void re__charclass_builder_begin(re__charclass_builder* builder)
{
  re__rune_range_vec_clear(&builder->ranges);
  builder->should_invert = 0;
  builder->should_fold = 0;
  builder->highest = -1;
}

RE_INTERNAL void re__charclass_builder_destroy(re__charclass_builder* builder)
{
  re__rune_range_vec_destroy(&builder->ranges);
}

RE_INTERNAL void re__charclass_builder_invert(re__charclass_builder* builder)
{
  builder->should_invert = 1;
}

RE_INTERNAL void re__charclass_builder_fold(re__charclass_builder* builder)
{
  builder->should_fold = 1;
}

/* Insert a range into the builder. */
/* Inserts in sorted order, that is, all ranges are ordered by their low bound.
 */
RE_INTERNAL re_error re__charclass_builder_insert_range_internal(
    re__charclass_builder* builder, re__rune_range range)
{
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

/* Insert a range into the builder. */
/* Inserts in sorted order, that is, all ranges are ordered by their low bound.
 */
RE_INTERNAL re_error re__charclass_builder_insert_range(
    re__charclass_builder* builder, re__rune_range range)
{
  RE_ASSERT(range.min >= 0);
  RE_ASSERT(range.max <= RE_RUNE_MAX);
  if (builder->should_fold) {
    re_rune i;
    for (i = range.min; i <= range.max; i++) {
      re_rune* folded;
      int folded_size = re__rune_data_casefold(builder->rune_data, i, &folded);
      int j;
      for (j = 0; j < folded_size; j++) {
        re_error err = RE_ERROR_NONE;
        re__rune_range temp;
        temp.min = folded[j];
        temp.max = folded[j];
        if ((err =
                 re__charclass_builder_insert_range_internal(builder, temp))) {
          return err;
        }
      }
    }
    return RE_ERROR_NONE;
  } else {
    return re__charclass_builder_insert_range_internal(builder, range);
  }
}

/* Insert a set of ranges into a builder. */
RE_INTERNAL re_error re__charclass_builder_insert_ranges(
    re__charclass_builder* builder, const re__rune_range* ranges,
    re_size ranges_size, int inverted)
{
  re_size i;
  re_error err = RE_ERROR_NONE;
  re__charclass_inverter inverter;
  re__charclass_inverter_init(&inverter, inverted);
  for (i = 0; i < ranges_size; i++) {
    /* Iterate through ranges, adding them all */
    /* Perhaps could be optimized more */
    re__charclass_inverter_push(&inverter, ranges[i]);
    if (re__charclass_inverter_hasnext(&inverter)) {
      if ((err = re__charclass_builder_insert_range(
               builder, re__charclass_inverter_next(&inverter)))) {
        return err;
      }
    }
  }
  re__charclass_inverter_end(&inverter);
  if (re__charclass_inverter_hasnext(&inverter)) {
    if ((err = re__charclass_builder_insert_range(
             builder, re__charclass_inverter_next(&inverter)))) {
      return err;
    }
  }
  return err;
}

RE_INTERNAL re_error re__charclass_builder_insert_property(
    re__charclass_builder* builder, re__str_view str, int inverted)
{
  re_error err = RE_ERROR_NONE;
  re__rune_range* ranges;
  re_size ranges_size;
  if ((err = re__rune_data_get_property(
           builder->rune_data, re__str_view_get_data(&str),
           re__str_view_size(&str), &ranges, &ranges_size))) {
    return err;
  }
  return re__charclass_builder_insert_ranges(
      builder, ranges, ranges_size, inverted);
}

RE_INTERNAL re_error re__charclass_builder_insert_ascii_internal(
    re__charclass_builder* builder, const re__charclass_ascii* ascii_cc,
    int inverted)
{
  re_error err = RE_ERROR_NONE;
  re_size i;
  re__rune_range temp;
  re__charclass_inverter inverter;
  re__charclass_inverter_init(&inverter, inverted);
  for (i = 0; i < ascii_cc->num_classes; i++) {
    temp.min = ascii_cc->classes[i * 2];
    temp.max = ascii_cc->classes[i * 2 + 1];
    re__charclass_inverter_push(&inverter, temp);
    if (re__charclass_inverter_hasnext(&inverter)) {
      if ((err = re__charclass_builder_insert_range(
               builder, re__charclass_inverter_next(&inverter)))) {
        return err;
      }
    }
  }
  re__charclass_inverter_end(&inverter);
  if (re__charclass_inverter_hasnext(&inverter)) {
    if ((err = re__charclass_builder_insert_range(
             builder, re__charclass_inverter_next(&inverter)))) {
      return err;
    }
  }
  return err;
}

RE_INTERNAL re_error re__charclass_builder_insert_ascii_class(
    re__charclass_builder* builder, re__charclass_ascii_type type, int inverted)
{
  return re__charclass_builder_insert_ascii_internal(
      builder, &re__charclass_ascii_defaults[type], inverted);
}

/* Returns RE_ERROR_INVALID if not found */
RE_INTERNAL re_error re__charclass_builder_insert_ascii_class_by_str(
    re__charclass_builder* builder, re__str_view name, int inverted)
{
  const re__charclass_ascii* found = re__charclass_ascii_find(name);
  /* Found is NULL if we didn't find anything during the loop */
  if (found == RE_NULL) {
    return RE_ERROR_INVALID;
  }
  return re__charclass_builder_insert_ascii_internal(builder, found, inverted);
}

/* I'm proud of this function. */
RE_INTERNAL re_error re__charclass_builder_finish(
    re__charclass_builder* builder, re__charclass* charclass)
{
  re_error err = RE_ERROR_NONE;
  /* Temporary range */
  re__rune_range temp;
  re_size read_ptr;
  re_size write_ptr = 0;
  re_size ranges_size = re__rune_range_vec_size(&builder->ranges);
  re__charclass_inverter inverter;
  temp.min = -1;
  temp.max = -1;
  /* We write back to the range buffer used for building, so we reserve an extra
   * element for the possibility of the ending [x, RUNE_MAX] range */
  if ((err = re__rune_range_vec_push(&builder->ranges, temp))) {
    return err;
  }
  re__charclass_inverter_init(&inverter, builder->should_invert);
  for (read_ptr = 0; read_ptr < ranges_size; read_ptr++) {
    re__rune_range cur = re__rune_range_vec_get(&builder->ranges, read_ptr);
    RE_ASSERT(cur.min <= cur.max);
    if (temp.min == -1) {
      /* First range */
      temp.min = cur.min;
      temp.max = cur.max;
      /* Result */
      /* temp_min      temp_max
       * ***************         */
    } else if (cur.min <= temp.max) {
      /* Some kind of intersection */
      if (cur.max > temp.max) {
        /* Current range intersects with temp.min/temp.max range */
        /* temp.min      temp.max
         * ***************
         *             cur.min       cur.max
         *             ***************           */
        /* Extend range */
        temp.max = cur.max;
        /* Result: */
        /* temp.min                  temp.max
         * ***************************         */
      } else {
        /* Current range is contained within temp.min/temp.max range */
        /* temp.min      temp.max
         * ***************
         *  cur.min cur.max
         *  *********         */
        /* Nothing needs to be done */
        /* Result: */
        /* temp.min      temp.max
         * ***************         */
      }
    } else if (cur.min == temp.max + 1) {
      /* Current range is adjacent to temp.min/temp.max range */
      /* temp.min      temp.max
       * ***************
       *                cur.min    cur.max
       *                ************       */
      /* Concatenate range */
      temp.max = cur.max;
      /* Result: */
      /* temp.min                  temp.max
       * ***************************         */
    } else { /* cur.min > temp.max */
      /* Current range is outside of temp.min/temp.max range */
      /* temp.min      temp.max
       * ***************
       *                      cur.min    cur.max
       *                      ************       */
      /* Create a new range, and push the temp.min/temp.max range */
      re__charclass_inverter_push(&inverter, temp);
      if (re__charclass_inverter_hasnext(&inverter)) {
        re__rune_range_vec_set(
            &builder->ranges, write_ptr++,
            re__charclass_inverter_next(&inverter));
      }
      temp.min = cur.min;
      temp.max = cur.max;
      /* Result */
      /* old_temp.min  old_temp.max
       * ***************
       *                      temp.min   temp.max
       *                      ************         */
    }
  }
  if (temp.min != -1) {
    re__charclass_inverter_push(&inverter, temp);
    if (re__charclass_inverter_hasnext(&inverter)) {
      re__rune_range_vec_set(
          &builder->ranges, write_ptr++,
          re__charclass_inverter_next(&inverter));
    }
  }
  re__charclass_inverter_end(&inverter);
  if (re__charclass_inverter_hasnext(&inverter)) {
    re__rune_range_vec_set(
        &builder->ranges, write_ptr++, re__charclass_inverter_next(&inverter));
  }
  if (write_ptr) {
    charclass->ranges =
        (re__rune_range*)RE_MALLOC(sizeof(re__rune_range) * write_ptr);
    if (charclass->ranges == RE_NULL) {
      return RE_ERROR_NOMEM;
    }
    charclass->ranges_size = write_ptr;
    for (read_ptr = 0; read_ptr < write_ptr; read_ptr++) {
      charclass->ranges[read_ptr] =
          re__rune_range_vec_get_data(&builder->ranges)[read_ptr];
    }
  } else {
    /* don't bother calling malloc(0) */
    charclass->ranges = RE_NULL;
    charclass->ranges_size = 0;
  }
  return err;
}

int re__charclass_equals(
    const re__charclass* charclass, const re__charclass* other)
{
  re_size cs = re__charclass_size(charclass);
  re_size os = re__charclass_size(other);
  re_size i;
  if (cs != os) {
    return 0;
  }
  for (i = 0; i < cs; i++) {
    re__rune_range cr = re__charclass_get_ranges(charclass)[i];
    re__rune_range or = re__charclass_get_ranges(other)[i];
    if (!re__rune_range_equals(cr, or)) {
      return 0;
    }
  }
  return 1;
}

#if RE_DEBUG

#include <stdio.h>

void re__charclass_dump(const re__charclass* charclass, re_size lvl)
{
  re_size i;
  re_size sz = charclass->ranges_size;
  printf("Charclass %p\n", (void*)charclass);
  for (i = 0; i < sz; i++) {
    re__rune_range cur = charclass->ranges[i];
    re_size j;
    for (j = 0; j < lvl; j++) {
      printf("  ");
    }
    printf("%X - %X\n", (re_rune)cur.min, (re_rune)cur.max);
  }
}

RE_INTERNAL int re__charclass_verify(const re__charclass* charclass)
{
  re_size i;
  re__rune_range last;
  last.min = -1;
  last.max = -1;
  for (i = 0; i < re__charclass_size(charclass); i++) {
    re__rune_range rr = re__charclass_get_ranges(charclass)[i];
    if (rr.min <= last.max) {
      return 0;
    }
    last = rr;
  }
  return 1;
}

void re__charclass_builder_dump(const re__charclass_builder* builder)
{
  re_size i;
  printf("Charclass Builder %p:\n", (void*)builder);
  printf("  Highest Rune: %i\n", builder->highest);
  printf("  Should Invert: %i\n", builder->should_invert);
  for (i = 0; i < re__rune_range_vec_size(&builder->ranges); i++) {
    re__rune_range r = re__rune_range_vec_get(&builder->ranges, i);
    printf("  [%u]: ", (unsigned int)i);
    re__rune_range_debug_dump(r);
    printf("\n");
  }
}

#endif

/* re */
#if RE_DEBUG

#include <stdio.h>

void re__compile_charclass_dump(
    re__compile_charclass* char_comp, re_uint32 tree_idx, re_int32 indent);

#endif

RE__VEC_IMPL_FUNC(re__compile_charclass_tree, init)
RE__VEC_IMPL_FUNC(re__compile_charclass_tree, destroy)
RE__VEC_IMPL_FUNC(re__compile_charclass_tree, clear)
RE__VEC_IMPL_FUNC(re__compile_charclass_tree, size)
RE__VEC_IMPL_FUNC(re__compile_charclass_tree, getref)
RE__VEC_IMPL_FUNC(re__compile_charclass_tree, push)
RE__VEC_IMPL_FUNC(re__compile_charclass_tree, reserve)

RE__VEC_IMPL_FUNC(re__compile_charclass_hash_entry, init)
RE__VEC_IMPL_FUNC(re__compile_charclass_hash_entry, destroy)
RE__VEC_IMPL_FUNC(re__compile_charclass_hash_entry, size)
RE__VEC_IMPL_FUNC(re__compile_charclass_hash_entry, getref)
RE__VEC_IMPL_FUNC(re__compile_charclass_hash_entry, push)
RE__VEC_IMPL_FUNC(re__compile_charclass_hash_entry, clear)

void re__compile_charclass_tree_init(
    re__compile_charclass_tree* tree, re__byte_range byte_range)
{
  tree->byte_range = byte_range;
  tree->sibling_ref = RE__COMPILE_CHARCLASS_TREE_NONE;
  tree->child_ref = RE__COMPILE_CHARCLASS_TREE_NONE;
  tree->aux = 0;
}

/* Get a pointer to a node in the tree, given its index. */
RE_INTERNAL re__compile_charclass_tree* re__compile_charclass_tree_get(
    re__compile_charclass* char_comp, re_uint32 tree_ref)
{
  RE_ASSERT(tree_ref != RE__COMPILE_CHARCLASS_TREE_NONE);
  RE_ASSERT(
      tree_ref <
      (re_uint32)re__compile_charclass_tree_vec_size(&char_comp->tree));
  return re__compile_charclass_tree_vec_getref(
      &char_comp->tree, (re_size)tree_ref);
}

/* Create a new node in the charclass compiler's internal tree structure. */
re_error re__compile_charclass_new_node(
    re__compile_charclass* char_comp, re_uint32 parent_ref,
    re__byte_range byte_range, re_uint32* out_new_node_ref,
    int use_reverse_tree)
{
  re_error err = RE_ERROR_NONE;
  re__compile_charclass_tree new_node;
  re_uint32 prev_sibling_ref;
  /* out_new_node_ref contains the next available position in the vector. */
  *out_new_node_ref =
      (re_uint32)re__compile_charclass_tree_vec_size(&char_comp->tree);
  if (parent_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
    /* adding to root */
    re_uint32* root_ref;
    re_uint32* root_last_child_ref;
    if (!use_reverse_tree) {
      root_ref = &char_comp->root_ref;
      root_last_child_ref = &char_comp->root_last_child_ref;
    } else {
      root_ref = &char_comp->rev_root_ref;
      root_last_child_ref = &char_comp->rev_root_last_child_ref;
    }
    if (*root_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
      /* root hasn't been found yet */
      /* Insert a dummy node as node 0 */
      re__byte_range zero_range;
      zero_range.min = 0;
      zero_range.max = 0;
      /* account for the dummy node */
      *out_new_node_ref += 1;
      *root_ref = *out_new_node_ref;
      *root_last_child_ref = *out_new_node_ref;
      /* create and push later, good practice for avoiding use-after-free */
      re__compile_charclass_tree_init(&new_node, zero_range);
      if ((err = re__compile_charclass_tree_vec_push(
               &char_comp->tree, new_node))) {
        return err;
      }
      prev_sibling_ref = RE__COMPILE_CHARCLASS_TREE_NONE;
    } else {
      /* root has been found, append to last child */
      prev_sibling_ref = *root_last_child_ref;
      *root_last_child_ref = *out_new_node_ref;
    }
  } else {
    re__compile_charclass_tree* parent;
    do {
      parent = re__compile_charclass_tree_get(char_comp, parent_ref);
      prev_sibling_ref = parent->child_ref;
      parent->child_ref = *out_new_node_ref;
      parent_ref = parent->sibling_ref;
      if (1) {
        break;
      }
    } while (parent_ref != RE__COMPILE_CHARCLASS_TREE_NONE);
  }
  re__compile_charclass_tree_init(&new_node, byte_range);
  new_node.sibling_ref = prev_sibling_ref;
  if ((err = re__compile_charclass_tree_vec_push(&char_comp->tree, new_node))) {
    return err;
  }
  return err;
}

re_error re__compile_charclass_add_rune_range(
    re__compile_charclass* char_comp, re_uint32 parent_ref,
    re__rune_range rune_range, re_uint32 num_x_bits, re_uint32 num_y_bits)
{
  re_rune x_mask = (1 << num_x_bits) - 1;
  re_rune byte_mask = 0xFF;
  re_rune u_mask = (0xFE << num_y_bits) & byte_mask;
  re_uint32 child_ref = RE__COMPILE_CHARCLASS_TREE_NONE;
  re_rune y_min = rune_range.min >> num_x_bits;
  re_rune y_max = rune_range.max >> num_x_bits;
  re_uint8 byte_min = (re_uint8)((y_min & byte_mask) | u_mask);
  re_uint8 byte_max = (re_uint8)((y_max & byte_mask) | u_mask);
  re_error err = RE_ERROR_NONE;
  RE_ASSERT(num_y_bits <= 7);
  if (num_x_bits == 0) {
    /* terminal nodes can never intersect when the input to this algorithm is a
     * set of sorted disjoint ranges, so we're covered here */
    re__byte_range br;
    br.min = byte_min;
    br.max = byte_max;
    if ((err = re__compile_charclass_new_node(
             char_comp, parent_ref, br, &child_ref, 0))) {
      return err;
    }
  } else {
    re_rune x_min = rune_range.min & x_mask;
    re_rune x_max = rune_range.max & x_mask;
    /* Generate byte ranges (for this frame) and rune ranges (for child) */
    re__byte_range brs[3];
    re__rune_range rrs[3];
    /* Number of sub_trees (extents of 'brs' and 'rrs') */
    int num_sub_trees = 0;
    /* The next y_bits and x_bits for sub-ranges */
    re_uint32 next_num_x_bits = num_x_bits - 6;
    re_uint32 next_num_y_bits = 6;
    if (y_min == y_max || (x_min == 0 && x_max == x_mask)) {
      /* Range can be split into either a single byte followed by a range,
       * _or_ one range followed by another maximal range */
      /* Output:
       * ---[Ymin-Ymax]---{tree for [Xmin-Xmax]} */
      brs[0].min = byte_min, brs[0].max = byte_max;
      rrs[0].min = x_min, rrs[0].max = x_max;
      num_sub_trees = 1;
    } else if (x_min == 0) {
      /* Range begins on zero, but has multiple starting bytes */
      /* Output:
       * ---[Ymin-(Ymax-1)]---{tree for [00-FF]}
       *           |
       *      [Ymax-Ymax]----{tree for [00-Xmax]} */
      brs[0].min = byte_min, brs[0].max = byte_max - 1;
      rrs[0].min = 0, rrs[0].max = x_mask;
      brs[1].min = byte_max, brs[1].max = byte_max;
      rrs[1].min = 0, rrs[1].max = x_max;
      num_sub_trees = 2;
    } else if (x_max == x_mask) {
      /* Range ends on all ones, but has multiple starting bytes */
      /* Output:
       * -----[Ymin-Ymin]----{tree for [Xmin-FF]}
       *           |
       *    [(Ymin+1)-Ymax]---{tree for [00-FF]} */
      brs[0].min = byte_min, brs[0].max = byte_min;
      rrs[0].min = x_min, rrs[0].max = x_mask;
      brs[1].min = byte_min + 1, brs[1].max = byte_max;
      rrs[1].min = 0, rrs[1].max = x_mask;
      num_sub_trees = 2;
    } else if (y_min == y_max - 1) {
      /* Range occupies exactly two starting bytes */
      /* Output:
       * -----[Ymin-Ymin]----{tree for [Xmin-FF]}
       *           |
       *      [Ymax-Ymax]----{tree for [00-Xmax]} */
      brs[0].min = byte_min, brs[0].max = byte_min;
      rrs[0].min = x_min, rrs[0].max = x_mask;
      brs[1].min = byte_min + 1, brs[1].max = byte_max;
      rrs[1].min = 0, rrs[1].max = x_max;
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
      brs[0].min = byte_min, brs[0].max = byte_min;
      rrs[0].min = x_min, rrs[0].max = x_mask;
      brs[1].min = byte_min + 1, brs[1].max = byte_max - 1;
      rrs[1].min = 0, rrs[1].max = x_mask;
      brs[2].min = byte_max, brs[2].max = byte_max;
      rrs[2].min = 0, rrs[2].max = x_max;
      num_sub_trees = 3;
    }
    {
      /* Create all sub trees */
      int i;
      for (i = 0; i < num_sub_trees; i++) {
        /* First check if the last child intersects and compute the
         * intersection. */
        re_uint32 prev_sibling_ref;
        if (parent_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
          prev_sibling_ref = char_comp->root_last_child_ref;
        } else {
          re__compile_charclass_tree* parent =
              re__compile_charclass_tree_get(char_comp, parent_ref);
          prev_sibling_ref = parent->child_ref;
        }
        if ((prev_sibling_ref != RE__COMPILE_CHARCLASS_TREE_NONE) &&
            re__byte_range_intersects(
                re__compile_charclass_tree_get(char_comp, prev_sibling_ref)
                    ->byte_range,
                brs[i])) {
          /* Edge case, this range intersects with the previous range in terms
           * of its x-bytes, so don't create a new node */
          child_ref = prev_sibling_ref;
        } else {
          /* Create a new node */
          if ((err = re__compile_charclass_new_node(
                   char_comp, parent_ref, brs[i], &child_ref, 0))) {
            return err;
          }
        }
        /* Add the range to the tree */
        if ((err = re__compile_charclass_add_rune_range(
                 char_comp, child_ref, rrs[i], next_num_x_bits,
                 next_num_y_bits))) {
          return err;
        }
      }
    }
  }
  return err;
}

/* Check if two trees are equal, recursively. */
int re__compile_charclass_tree_equals(
    re__compile_charclass* char_comp, re__compile_charclass_tree* a,
    re__compile_charclass_tree* b)
{
  re_uint32 a_child_ref = a->child_ref;
  re_uint32 b_child_ref = b->child_ref;
  /* While child references aren't none, check their equality. */
  while (a_child_ref != RE__COMPILE_CHARCLASS_TREE_NONE &&
         b_child_ref != RE__COMPILE_CHARCLASS_TREE_NONE) {
    re__compile_charclass_tree* a_child =
        re__compile_charclass_tree_get(char_comp, a_child_ref);
    re__compile_charclass_tree* b_child =
        re__compile_charclass_tree_get(char_comp, b_child_ref);
    if (!re__compile_charclass_tree_equals(char_comp, a_child, b_child)) {
      return 0;
    }
    /* Go to next child refs. */
    a_child_ref = a_child->sibling_ref;
    b_child_ref = b_child->sibling_ref;
  }
  /* Child refs must become NONE at the same time. */
  if (a_child_ref !=
      b_child_ref) { /* (a,b) != RE__COMPILE_CHARCLASS_TREE_NONE */
    return 0;
  }
  return re__byte_range_equals(a->byte_range, b->byte_range);
}

/* Temporary structure that will be hashed, byte for byte. */
typedef struct re__compile_charclass_hash_temp {
  re__byte_range byte_range;
  re_uint32 down_hash;
  re_uint32 next_hash;
} re__compile_charclass_hash_temp;

void re__compile_charclass_merge_one(
    re__compile_charclass_tree* child, re__compile_charclass_tree* sibling)
{
  child->sibling_ref = sibling->sibling_ref;
  child->byte_range =
      re__byte_range_merge(child->byte_range, sibling->byte_range);
}

void re__compile_charclass_hash_tree(
    re__compile_charclass* char_comp, re_uint32 parent_ref)
{
  re__compile_charclass_tree* child;
  re__compile_charclass_tree* sibling;
  re__compile_charclass_tree* parent;
  re_uint32 child_ref, sibling_ref, next_child_ref;
  if (parent_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
    /* this is root */
    child_ref = char_comp->root_last_child_ref;
  } else {
    parent = re__compile_charclass_tree_get(char_comp, parent_ref);
    child_ref = parent->child_ref;
  }
  sibling_ref = RE__COMPILE_CHARCLASS_TREE_NONE;
  while (child_ref != RE__COMPILE_CHARCLASS_TREE_NONE) {
    child = re__compile_charclass_tree_get(char_comp, child_ref);
    next_child_ref = child->sibling_ref;
    child->sibling_ref = sibling_ref;
    re__compile_charclass_hash_tree(char_comp, child_ref);
    if (sibling_ref != RE__COMPILE_CHARCLASS_TREE_NONE) {
      sibling = re__compile_charclass_tree_get(char_comp, sibling_ref);
      if (re__byte_range_adjacent(child->byte_range, sibling->byte_range)) {
        if (sibling->child_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
          if (child->child_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
            re__compile_charclass_merge_one(child, sibling);
          }
        } else {
          if (child->child_ref != RE__COMPILE_CHARCLASS_TREE_NONE) {
            re__compile_charclass_tree* child_child =
                re__compile_charclass_tree_get(char_comp, child->child_ref);
            re__compile_charclass_tree* sibling_child =
                re__compile_charclass_tree_get(char_comp, sibling->child_ref);
            if (RE__WEAKEN_HASH(child_child->aux) ==
                RE__WEAKEN_HASH(sibling_child->aux)) {
              if (re__compile_charclass_tree_equals(
                      char_comp, child_child, sibling_child)) {
                /* Siblings have identical children and can be merged */
                re__compile_charclass_merge_one(child, sibling);
              }
            }
          }
        }
      }
    }
    {
      re__compile_charclass_hash_temp hash_obj;
      /* C89 does not guarantee struct zero-padding. This will throw off our
       * hash function if uninitialized properly. We explicitly zero out the
       * memory for this reason. */
      re__memset((void*)&hash_obj, 0, sizeof(re__compile_charclass_hash_temp));
      hash_obj.byte_range = child->byte_range;
      /* Register hashes for next sibling and first child. */
      if (child->sibling_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
        /* I know nothing about cryptography. Whether or not this is an
         * actually good value is unknown. */
        hash_obj.next_hash = 0xF0F0F0F0;
      } else {
        sibling = re__compile_charclass_tree_get(char_comp, sibling_ref);
        hash_obj.next_hash = sibling->aux;
      }
      if (child->child_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
        hash_obj.down_hash = 0x0F0F0F0F;
      } else {
        re__compile_charclass_tree* child_child =
            re__compile_charclass_tree_get(char_comp, child->child_ref);
        hash_obj.down_hash = child_child->aux;
      }
      /* Murmurhash seemed good... */
      child->aux =
          re__murmurhash3_32(0, (const re_uint8*)&hash_obj, sizeof(hash_obj));
    }
    sibling_ref = child_ref;
    sibling = child;
    child_ref = next_child_ref;
  }
  if (parent_ref != RE__COMPILE_CHARCLASS_TREE_NONE) {
    parent->child_ref = sibling_ref;
  }
}

re_error re__compile_charclass_split_rune_range(
    re__compile_charclass* char_comp, re__rune_range range)
{
  re_error err = RE_ERROR_NONE;
  static const re_uint32 y_bits[4] = {7, 5, 4, 3};
  static const re_uint32 x_bits[4] = {0, 6, 12, 18};
  re_uint32 byte_length;
  re_rune min_value = 0;
  for (byte_length = 0; byte_length < 4; byte_length++) {
    re_rune max_value = (1 << (y_bits[byte_length] + x_bits[byte_length])) - 1;
    re__rune_range bounds;
    bounds.min = min_value;
    bounds.max = max_value;
    if (re__rune_range_intersects(range, bounds)) {
      re__rune_range clamped = re__rune_range_clamp(range, bounds);
      if ((err = re__compile_charclass_add_rune_range(
               char_comp, RE__COMPILE_CHARCLASS_TREE_NONE, clamped,
               x_bits[byte_length], y_bits[byte_length]))) {
        return err;
      }
    }
    min_value = max_value + 1;
  }
  return err;
}

void re__compile_charclass_hash_entry_init(
    re__compile_charclass_hash_entry* hash_entry, re_int32 sparse_index,
    re_uint32 root_ref, re__prog_loc prog_loc)
{
  hash_entry->sparse_index = sparse_index;
  hash_entry->root_ref = root_ref;
  hash_entry->prog_loc = prog_loc;
  hash_entry->next = RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE;
}

/* Clear the program location cache. */
void re__compile_charclass_cache_clear(re__compile_charclass* char_comp)
{
  /* We don't need to clear cache_sparse, because it can be used while
   * undefined. */
  re__compile_charclass_hash_entry_vec_clear(&char_comp->cache_dense);
}

/* Get a program location from the cache, if its given tree is in the cache. */
/* Returns RE__PROG_LOC_INVALID if not in the cache. */
re__prog_loc re__compile_charclass_cache_get(
    re__compile_charclass* char_comp, re_uint32 root_ref)
{
  re__compile_charclass_tree* root;
  /* Index into the sparse array based off of root's hash */
  re_int32 sparse_index;
  /* Final index in the dense array, if the tree is found in the cache */
  re_int32 dense_index;
  RE_ASSERT(root_ref != RE__COMPILE_CHARCLASS_TREE_NONE);
  root = re__compile_charclass_tree_get(char_comp, root_ref);
  sparse_index = root->aux % RE__COMPILE_CHARCLASS_CACHE_SPARSE_SIZE;
  dense_index = RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE;
  if (char_comp->cache_sparse == RE_NULL) {
    /* Cache is empty a.t.m., just return */
    return RE__PROG_LOC_INVALID;
  }
  dense_index = char_comp->cache_sparse[sparse_index];
  /* If dense_index_initial is more than the dense size, sparse_index is
   * nonsensical and the item isn't in the cache. */
  if (dense_index < (re_int32)re__compile_charclass_hash_entry_vec_size(
                        &char_comp->cache_dense) &&
      dense_index >= 0) {
    /* Load the hash entry at dense_index_initial */
    re__compile_charclass_hash_entry* hash_entry_prev =
        re__compile_charclass_hash_entry_vec_getref(
            &char_comp->cache_dense, (re_size)dense_index);
    /* If hash_entry_prev->sparse_index doesn't point back to sparse_index,
     * sparse_index is nonsensical. */
    if (hash_entry_prev->sparse_index == sparse_index) {
      while (1) {
        re__compile_charclass_tree* root_cache =
            re__compile_charclass_tree_vec_getref(
                &char_comp->tree, (re_size)hash_entry_prev->root_ref);
        if (RE__WEAKEN_HASH(root_cache->aux) == RE__WEAKEN_HASH(root->aux)) {
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
          hash_entry_prev = re__compile_charclass_hash_entry_vec_getref(
              &char_comp->cache_dense, (re_size)dense_index);
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

#if RE_DEBUG || RE__COVERAGE

void re__compile_charclass_randomize_sparse(re__compile_charclass* char_comp)
{
  /* Debug: initialize sparse with "undefined" (but deterministic) values */
  re_int32 i;
  if (char_comp->cache_sparse == RE_NULL) {
    return;
  }
  for (i = 0; i < RE__COMPILE_CHARCLASS_CACHE_SPARSE_SIZE; i++) {
    char_comp->cache_sparse[i] = i;
    if (i % 2 == 0) {
      /* Make some values negative */
      char_comp->cache_sparse[i] -=
          (RE__COMPILE_CHARCLASS_CACHE_SPARSE_SIZE / 2);
    }
  }
}

#endif

/* Add a program location to the cache, after it has been compiled. */
re_error re__compile_charclass_cache_add(
    re__compile_charclass* char_comp, re_uint32 root_ref, re__prog_loc prog_loc)
{
  re_error err = RE_ERROR_NONE;
  re__compile_charclass_tree* root =
      re__compile_charclass_tree_get(char_comp, root_ref);
  /* These variables have the same meaning as they do in cache_get. */
  re_int32 sparse_index = root->aux % RE__COMPILE_CHARCLASS_CACHE_SPARSE_SIZE;
  re_int32 dense_index = RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE;
  re__compile_charclass_hash_entry* hash_entry_prev = RE_NULL;
  int requires_link;
  if (char_comp->cache_sparse == RE_NULL) {
    /* Sparse cache is empty, so let's allocate it on-demand. */
    char_comp->cache_sparse = (re_int32*)RE_MALLOC(
        sizeof(re_int32) * RE__COMPILE_CHARCLASS_CACHE_SPARSE_SIZE);
#if RE_DEBUG || RE__COVERAGE
    re__compile_charclass_randomize_sparse(char_comp);
#endif
    if (char_comp->cache_sparse == RE_NULL) {
      return RE_ERROR_NOMEM;
    }
  }
  dense_index = char_comp->cache_sparse[sparse_index];
  /* Look up the element in the cache, see re__compile_charclass_cache_get */
  if (dense_index < (re_int32)re__compile_charclass_hash_entry_vec_size(
                        &char_comp->cache_dense) &&
      dense_index >= 0) {
    hash_entry_prev = re__compile_charclass_hash_entry_vec_getref(
        &char_comp->cache_dense, (re_size)dense_index);
    if (hash_entry_prev->sparse_index == sparse_index) {
      while (1) {
        /* We found the item in the cache? This should never
         * happen. To ensure optimality, we should only ever add
         * items to the cache once. */
        RE_ASSERT(!(
            re__compile_charclass_tree_get(char_comp, hash_entry_prev->root_ref)
                    ->aux == root->aux &&
            re__compile_charclass_tree_equals(
                char_comp,
                re__compile_charclass_tree_get(
                    char_comp, hash_entry_prev->root_ref),
                root)));
        if (hash_entry_prev->next == RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE) {
          requires_link = 1;
          dense_index = RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE;
          break;
        }
        /* Keep iterating. */
        dense_index = hash_entry_prev->next;
        hash_entry_prev = re__compile_charclass_hash_entry_vec_getref(
            &char_comp->cache_dense, (re_size)dense_index);
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
    re_int32 dense_index_final =
        (re_int32)re__compile_charclass_hash_entry_vec_size(
            &char_comp->cache_dense);
    re__compile_charclass_hash_entry new_entry;
    re__compile_charclass_hash_entry_init(
        &new_entry, sparse_index, root_ref, prog_loc);
    if (!requires_link) {
      /* No linking required, insert a new sparse entry */
      char_comp->cache_sparse[sparse_index] = dense_index_final;
      if ((err = re__compile_charclass_hash_entry_vec_push(
               &char_comp->cache_dense, new_entry))) {
        return err;
      }
      /* hash_entry_prev is invalid */
    } else {
      /* Linking required */
      hash_entry_prev->next = dense_index_final;
      if ((err = re__compile_charclass_hash_entry_vec_push(
               &char_comp->cache_dense, new_entry))) {
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
re_error re__compile_charclass_generate_prog(
    re__compile_charclass* char_comp, re__prog* prog, re_uint32 node_ref,
    re__prog_loc* out_pc, re__compile_patches* patches)
{
  /* Starting program location for this root. */
  re__prog_loc start_pc = re__prog_size(prog);
  /* Keeps track of the previous split location, if there is one. */
  re__prog_loc split_from = RE__PROG_LOC_INVALID;
  re_error err = RE_ERROR_NONE;
  while (node_ref != RE__COMPILE_CHARCLASS_TREE_NONE) {
    re__compile_charclass_tree* node;
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
    if ((cache_pc = re__compile_charclass_cache_get(char_comp, node_ref)) !=
        RE__PROG_LOC_INVALID) {
      /* Tree is in cache! */
      /* if split_from is invalid, that means that root is in the cache.
       * This shouldn't be possible. */
      RE_ASSERT(split_from != RE__PROG_LOC_INVALID);
      inst_from = re__prog_get(prog, split_from);
      re__prog_inst_set_split_secondary(inst_from, cache_pc);
      break;
    }
    /* Tree is not in cache */
    node = re__compile_charclass_tree_get(char_comp, node_ref);
    if (split_from != RE__PROG_LOC_INVALID) {
      /* If we previously compiled a split instruction, link it to the
       * next compiled instruction. */
      inst_from = re__prog_get(prog, split_from);
      re__prog_inst_set_split_secondary(inst_from, re__prog_size(prog));
    }
    if (node->sibling_ref != RE__COMPILE_CHARCLASS_TREE_NONE) {
      /* If there is another sibling that will get compiled, add a SPLIT
       * instruction before the child. This split instruction will get
       * linked to the next child (see above lines) */
      split_from = re__prog_size(prog);
      re__prog_inst_init_split(
          &new_inst, re__prog_size(prog) + 1, RE__PROG_LOC_INVALID);
      if ((err = re__prog_add(prog, new_inst))) {
        return err;
      }
    }
    /* Location of byte range/byte instruction */
    link_from = re__prog_size(prog);
    byte_range = node->byte_range;
    /* Compile either a byterange or byte instruction depending on range */
    if (byte_range.min == byte_range.max) {
      re__prog_inst_init_byte(&new_inst, byte_range.min);
    } else {
      re__prog_inst_init_byte_range(&new_inst, byte_range);
    }
    if ((err = re__prog_add(prog, new_inst))) {
      return err;
    }
    if (node->child_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
      /* Child is a terminal node. Add an outgoing patch. */
      re__compile_patches_append(patches, prog, link_from, 0);
    } else {
      /* Check if child's children are in the cache. */
      /* link_to becomes the cached program location if child's children
       * are in the cache, otherwise it is RE__PROG_LOC_INVALID. */
      if ((link_to = re__compile_charclass_cache_get(
               char_comp, node->child_ref)) == RE__PROG_LOC_INVALID) {
        /* Child's children are not in the cache. Generate them. */
        if ((err = re__compile_charclass_generate_prog(
                 char_comp, prog, node->child_ref, &link_to, patches))) {
          return err;
        }
      }
      /* link_to always points to cached pc or next pc */
      inst_from = re__prog_get(prog, link_from);
      re__prog_inst_set_primary(inst_from, link_to);
    }
    /* Register child in the cache. */
    if ((err =
             re__compile_charclass_cache_add(char_comp, node_ref, link_from))) {
      return err;
    }
    node_ref = node->sibling_ref;
  }
  *out_pc = start_pc;
  return RE_ERROR_NONE;
}

/* Clear the aux field on all nodes in the tree. */
void re__compile_charclass_clear_aux(
    re__compile_charclass* char_comp, re_uint32 parent_ref)
{
  re_uint32 child_ref;
  if (parent_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
    child_ref = char_comp->root_ref;
  } else {
    re__compile_charclass_tree* parent =
        re__compile_charclass_tree_get(char_comp, parent_ref);
    child_ref = parent->child_ref;
  }
  while (child_ref != RE__COMPILE_CHARCLASS_TREE_NONE) {
    re__compile_charclass_tree* child =
        re__compile_charclass_tree_get(char_comp, child_ref);
    child->aux = RE__COMPILE_CHARCLASS_TREE_NONE;
    re__compile_charclass_clear_aux(char_comp, child_ref);
    child_ref = child->sibling_ref;
  }
}

re_error
re__compile_charclass_transpose_pass_1(re__compile_charclass* char_comp)
{
  re_error err = RE_ERROR_NONE;
  re_uint32 rev_start_idx =
      (re_uint32)re__compile_charclass_tree_vec_size(&char_comp->tree);
  re_uint32 i;
  if ((err = re__compile_charclass_tree_vec_reserve(
           &char_comp->tree, rev_start_idx * 2))) {
    return err;
  }
  for (i = 1; i < rev_start_idx + 1; i++) {
    re__compile_charclass_tree node;
    re__compile_charclass_tree* orig =
        re__compile_charclass_tree_get(char_comp, i);
    orig->aux = i + rev_start_idx - 1;
    re__compile_charclass_tree_init(&node, orig->byte_range);
    re__compile_charclass_tree_vec_push(&char_comp->tree, node);
  }
  return err;
}

void re__compile_charclass_transpose_pass_2(
    re__compile_charclass* char_comp, re_uint32 parent_ref)
{
  re_uint32 child_ref;
  if (parent_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
    /* root */
    child_ref = char_comp->root_ref;
  } else {
    child_ref =
        re__compile_charclass_tree_get(char_comp, parent_ref)->child_ref;
  }
  while (child_ref != RE__COMPILE_CHARCLASS_TREE_NONE) {
    re__compile_charclass_tree* child =
        re__compile_charclass_tree_get(char_comp, child_ref);
    re__compile_charclass_tree* child_rev =
        re__compile_charclass_tree_get(char_comp, child->aux);
    if (child->child_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
      /* Terminal, add to root */
      if (char_comp->rev_root_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
        char_comp->rev_root_ref = child->aux;
        char_comp->rev_root_last_child_ref = child->aux;
      } else {
        child_rev->sibling_ref = char_comp->rev_root_ref;
        char_comp->rev_root_ref = child->aux;
      }
    } else {
      /* Transpose children */
      re__compile_charclass_transpose_pass_2(char_comp, child_ref);
      {
        /* For each child, link back */
        re_uint32 child_child_ref = child->child_ref;
        while (child_child_ref != RE__COMPILE_CHARCLASS_TREE_NONE) {
          re__compile_charclass_tree* child_child =
              re__compile_charclass_tree_get(char_comp, child_child_ref);
          re__compile_charclass_tree* child_child_rev =
              re__compile_charclass_tree_get(char_comp, child_child->aux);
          child_child_rev->child_ref = child->aux;
          child_child_ref = child_child->sibling_ref;
        }
      }
    }
    child_ref = child->sibling_ref;
  }
}

re_error re__compile_charclass_transpose(
    re__compile_charclass* char_comp, re_uint32 parent_ref,
    re_uint32* aux_prev_in)
{
  re_uint32 child_ref;
  re_error err = RE_ERROR_NONE;
  re_uint32 aux_head = RE__COMPILE_CHARCLASS_TREE_NONE;
  re_uint32 aux_prev;
  if (parent_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
    child_ref = char_comp->root_ref;
  } else {
    re__compile_charclass_tree* parent =
        re__compile_charclass_tree_get(char_comp, parent_ref);
    child_ref = parent->child_ref;
  }
  while (child_ref != RE__COMPILE_CHARCLASS_TREE_NONE) {
    re__compile_charclass_tree* child =
        re__compile_charclass_tree_get(char_comp, child_ref);
    re_uint32 new_ref;
    if (child->child_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
      /* terminal, add to reverse root */
      if ((err = re__compile_charclass_new_node(
               char_comp, RE__COMPILE_CHARCLASS_TREE_NONE, child->byte_range,
               &new_ref, 1))) {
        return err;
      }
      /* Prevent UAF, child may have changed */
      child = re__compile_charclass_tree_get(char_comp, child_ref);
    } else {
      if ((err = re__compile_charclass_transpose(
               char_comp, child_ref, &aux_prev))) {
        return err;
      }
      child = re__compile_charclass_tree_get(char_comp, child_ref);
      child->aux = aux_prev;
      /* ALERT!!! child may have changed!!! */
      child = re__compile_charclass_tree_get(char_comp, child_ref);
      {
        /* Back-link new node to previously created ones */
        re_uint32 aux_ptr = child->aux;
        re__byte_range br = child->byte_range;
        while (aux_ptr != RE__COMPILE_CHARCLASS_TREE_NONE) {
          re__compile_charclass_tree* aux_tree =
              re__compile_charclass_tree_get(char_comp, aux_ptr);
          re_uint32 next_aux = aux_tree->aux;
          if ((err = re__compile_charclass_new_node(
                   char_comp, aux_ptr, br, &new_ref, 1))) {
            return err;
          }
          aux_ptr = next_aux;
        }
      }
      child = re__compile_charclass_tree_get(char_comp, child_ref);
    }
    {
      re__compile_charclass_tree* new_rev =
          re__compile_charclass_tree_get(char_comp, new_ref);
      new_rev->aux = aux_head;
      aux_head = new_ref;
    }
    child_ref = child->sibling_ref;
  }
  if (aux_prev_in != RE_NULL) {
    *aux_prev_in = aux_head;
  }
  return err;
}

void re__compile_charclass_reset(re__compile_charclass* char_comp)
{
  /* These are all idempotent. Cool word, right? I just learned it */
  re__compile_charclass_tree_vec_clear(&char_comp->tree);
  re__compile_charclass_cache_clear(char_comp);
#if RE_DEBUG || RE__COVERAGE
  re__compile_charclass_randomize_sparse(char_comp);
#endif
  char_comp->root_ref = RE__COMPILE_CHARCLASS_TREE_NONE;
  char_comp->root_last_child_ref = RE__COMPILE_CHARCLASS_TREE_NONE;
  char_comp->rev_root_ref = RE__COMPILE_CHARCLASS_TREE_NONE;
  char_comp->rev_root_last_child_ref = RE__COMPILE_CHARCLASS_TREE_NONE;
}

/* Compile a single character class. */
re_error re__compile_charclass_gen(
    re__compile_charclass* char_comp, const re__charclass* charclass,
    re__prog* prog, re__compile_patches* patches_out, int reversed)
{
  re_error err = RE_ERROR_NONE;
  re_size i;
  const re__rune_range* ranges = re__charclass_get_ranges(charclass);
  re__compile_charclass_reset(char_comp);
  /* Iterate through charclass' ranges and add them all to the tree. */
  for (i = 0; i < re__charclass_size(charclass); i++) {
    re__rune_range r = ranges[i];
    if ((err = re__compile_charclass_split_rune_range(char_comp, r))) {
      return err;
    }
  }
  {
    /* Do the actual compiling. */
    re__prog_loc out_pc;
    /* Hash and merge the tree */
    re__compile_charclass_hash_tree(char_comp, RE__COMPILE_CHARCLASS_TREE_NONE);
    /* Generate the tree's program */
    if (!reversed) {
      if ((err = re__compile_charclass_generate_prog(
               char_comp, prog, char_comp->root_ref, &out_pc, patches_out))) {
        return err;
      }
    } else {
      re__compile_charclass_clear_aux(
          char_comp, RE__COMPILE_CHARCLASS_TREE_NONE);
      if ((err = re__compile_charclass_transpose_pass_1(char_comp))) {
        return err;
      }
      re__compile_charclass_transpose_pass_2(
          char_comp, RE__COMPILE_CHARCLASS_TREE_NONE);
      if ((err = re__compile_charclass_generate_prog(
               char_comp, prog, char_comp->rev_root_last_child_ref, &out_pc,
               patches_out))) {
        return err;
      }
    }
    /* Done!!! all that effort for just a few instructions. */
  }

  return err;
}

void re__compile_charclass_init(re__compile_charclass* char_comp)
{
  re__compile_charclass_tree_vec_init(&char_comp->tree);
  re__compile_charclass_hash_entry_vec_init(&char_comp->cache_dense);
  char_comp->cache_sparse = RE_NULL;
  re__compile_charclass_reset(char_comp);
}

void re__compile_charclass_destroy(re__compile_charclass* char_comp)
{
  re__compile_charclass_hash_entry_vec_destroy(&char_comp->cache_dense);
  if (char_comp->cache_sparse) {
    RE_FREE(char_comp->cache_sparse);
  }
  re__compile_charclass_tree_vec_destroy(&char_comp->tree);
}

#if RE_DEBUG

#include <stdio.h>

void re__compile_charclass_dump(
    re__compile_charclass* char_comp, re_uint32 tree_idx, re_int32 indent)
{
  re_int32 i;
  re_uint32 node = tree_idx;
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
      re__compile_charclass_tree* tree = re__compile_charclass_tree_vec_getref(
          &char_comp->tree, (re_size)node);
      for (i = 0; i < indent + 1; i++) {
        printf("  ");
      }
      printf(
          "%04X | [%02X-%02X] hash=%08X\n", node, tree->byte_range.min,
          tree->byte_range.max, tree->aux);
      re__compile_charclass_dump(char_comp, tree->child_ref, indent + 1);
      node = tree->sibling_ref;
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
        if (dense_loc >= (re_int32)re__compile_charclass_hash_entry_vec_size(
                             &char_comp->cache_dense) ||
            dense_loc < 0) {
          continue;
        }
        hash_entry = re__compile_charclass_hash_entry_vec_getref(
            &char_comp->cache_dense, (re_size)dense_loc);
        if (hash_entry->sparse_index != i) {
          continue;
        }
        printf("    Sparse index: %i\n", i);
        j = 0;
        printf(
            "      [%i] root_ref=%04X prog_loc=%04X\n", j, hash_entry->root_ref,
            hash_entry->prog_loc);
        j++;
        while (hash_entry->next != RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE) {
          hash_entry = re__compile_charclass_hash_entry_vec_getref(
              &char_comp->cache_dense, (re_size)hash_entry->next);
          printf(
              "      [%i] root_ref=%04X prog_loc=%04X\n", j,
              hash_entry->root_ref, hash_entry->prog_loc);
          j++;
        }
      }
    }
  }
}

#endif

/* re */
RE_INTERNAL void re__compile_patches_init(re__compile_patches* patches)
{
  patches->first_inst = RE__PROG_LOC_INVALID;
  patches->last_inst = RE__PROG_LOC_INVALID;
}

#if 0
/* I might want to use this later! */
RE_INTERNAL void re__compile_patches_prepend(
    re__compile_patches* patches, re__prog* prog, re__prog_loc to,
    int secondary)
{
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
#endif

RE_INTERNAL void re__compile_patches_append(
    re__compile_patches* patches, re__prog* prog, re__prog_loc to,
    int secondary)
{
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

RE_INTERNAL void re__compile_patches_merge(
    re__compile_patches* patches, re__prog* prog,
    re__compile_patches* merge_from)
{
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

RE_INTERNAL void re__compile_patches_patch(
    re__compile_patches* patches, re__prog* prog, re__prog_loc to)
{
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

RE_VEC_IMPL_FUNC(re__prog_loc, init)
RE_VEC_IMPL_FUNC(re__prog_loc, destroy)
RE_VEC_IMPL_FUNC(re__prog_loc, push)
RE_VEC_IMPL_FUNC(re__prog_loc, size)
RE_VEC_IMPL_FUNC(re__prog_loc, get)

/* check for cycles in compile_patches */
RE_INTERNAL int
re__compile_patches_verify(re__compile_patches* patches, re__prog* prog)
{
  re__prog_loc current_loc = patches->first_inst;
  re__prog_loc_vec found_list;
  if (patches->first_inst == RE__PROG_LOC_INVALID) {
    return patches->last_inst == RE__PROG_LOC_INVALID;
  }
  re__prog_loc_vec_init(&found_list);
  /* O(n^2) so use with care! */
  while (current_loc != patches->last_inst) {
    re_size i;
    re__prog_inst* inst = re__prog_get(prog, current_loc >> 1);
    for (i = 0; i < re__prog_loc_vec_size(&found_list); i++) {
      re__prog_loc found = re__prog_loc_vec_get(&found_list, i);
      if (found == current_loc) {
        return 0; /* cycle detected */
      }
    }
    re__prog_loc_vec_push(&found_list, current_loc);
    if (!(current_loc & 1)) {
      current_loc = re__prog_inst_get_primary(inst);
    } else {
      current_loc = re__prog_inst_get_split_secondary(inst);
    }
  }
  re__prog_loc_vec_destroy(&found_list);
  return 1;
}

RE_INTERNAL void
re__compile_patches_dump(re__compile_patches* patches, re__prog* prog)
{
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
    printf(
        "%04X | %s\n", current_loc >> 1,
        (current_loc & 1) ? "secondary" : "primary");
    current_loc = next_loc;
  }
  printf(
      "%04X | %s\n", current_loc >> 1,
      (current_loc & 1) ? "secondary" : "primary");
}

#endif

RE_INTERNAL void re__compile_frame_init(
    re__compile_frame* frame, re_int32 ast_base_ref, re_int32 ast_child_ref,
    re__compile_patches patches, re__prog_loc start, re__prog_loc end)
{
  frame->ast_base_ref = ast_base_ref;
  frame->ast_child_ref = ast_child_ref;
  frame->patches = patches;
  frame->start = start;
  frame->end = end;
  frame->rep_idx = 0;
}

RE_INTERNAL void re__compile_init(re__compile* compile)
{
  compile->frames = NULL;
  compile->frames_size = 0;
  compile->frame_ptr = 0;
  re__compile_charclass_init(&compile->char_comp);
  /* set to NULL, re__compile_regex() sets ast_root */
  compile->ast_root = NULL;
  compile->should_push_child = 0;
  compile->should_push_child_ref = RE__AST_NONE;
  /* purposefully don't initialize returned_frame */
  /* compile->returned_frame; */
  compile->reversed = 0;
  compile->set = RE__AST_NONE;
}

RE_INTERNAL void re__compile_destroy(re__compile* compile)
{
  re__compile_charclass_destroy(&compile->char_comp);
}

RE_INTERNAL void
re__compile_frame_push(re__compile* compile, re__compile_frame frame)
{
  RE_ASSERT(compile->frame_ptr != compile->frames_size);
  compile->frames[compile->frame_ptr++] = frame;
}

RE_INTERNAL re__compile_frame re__compile_frame_pop(re__compile* compile)
{
  RE_ASSERT(compile->frame_ptr != 0);
  return compile->frames[--compile->frame_ptr];
}

RE_INTERNAL int re__compile_gen_utf8(re_rune codep, re_uint8* out_buf)
{
  if (codep <= 0x7F) {
    out_buf[0] = codep & 0x7F;
    return 1;
  } else if (codep <= 0x07FF) {
    out_buf[0] = (re_uint8)(((codep >> 6) & 0x1F) | 0xC0);
    out_buf[1] = (re_uint8)(((codep >> 0) & 0x3F) | 0x80);
    return 2;
  } else if (codep <= 0xFFFF) {
    out_buf[0] = (re_uint8)(((codep >> 12) & 0x0F) | 0xE0);
    out_buf[1] = (re_uint8)(((codep >> 6) & 0x3F) | 0x80);
    out_buf[2] = (re_uint8)(((codep >> 0) & 0x3F) | 0x80);
    return 3;
  } else if (codep <= 0x10FFFF) {
    out_buf[0] = (re_uint8)(((codep >> 18) & 0x07) | 0xF0);
    out_buf[1] = (re_uint8)(((codep >> 12) & 0x3F) | 0x80);
    out_buf[2] = (re_uint8)(((codep >> 6) & 0x3F) | 0x80);
    out_buf[3] = (re_uint8)(((codep >> 0) & 0x3F) | 0x80);
    return 4;
  } else {
    return 0;
  }
}

RE_INTERNAL re_error re__compile_do_rune(
    re__compile* compile, re__compile_frame* frame, const re__ast* ast,
    re__prog* prog)
{
  /* Generates a single Byte or series of Byte instructions for a
   * UTF-8 codepoint. */
  /*    +0
   * ~~~+-------+~~~
   * .. | Byte  | ..
   * .. | Instr | ..
   * ~~~+---+---+~~~
   *        |
   *        +-----1--> ... */
  re_error err = RE_ERROR_NONE;
  re__prog_inst new_inst;
  re_uint8 utf8_bytes[4];
  int num_bytes = re__compile_gen_utf8(re__ast_get_rune(ast), utf8_bytes);
  int i;
  RE__UNUSED(compile);
  for (i = 0; i < num_bytes; i++) {
    if (!compile->reversed) {
      re__prog_inst_init_byte(&new_inst, utf8_bytes[i]);
    } else {
      re__prog_inst_init_byte(&new_inst, utf8_bytes[(num_bytes - 1) - i]);
    }
    if (i == num_bytes - 1) {
      /* Add an outgoing patch (1) */
      re__compile_patches_append(&frame->patches, prog, re__prog_size(prog), 0);
    } else {
      re__prog_inst_set_primary(&new_inst, re__prog_size(prog) + 1);
    }
    if ((err = re__prog_add(prog, new_inst))) {
      return err;
    }
  }
  return err;
}

RE_INTERNAL re_error re__compile_do_str(
    re__compile* compile, re__compile_frame* frame, const re__ast* ast,
    re__prog* prog)
{
  /* Generates a single Byte or series of Byte instructions for a string. */
  /*    +0
   * ~~~+-------+~~~
   * .. | Byte  | ..
   * .. | Instr | ..
   * ~~~+---+---+~~~
   *        |
   *        +-----1--> ... */
  re_error err = RE_ERROR_NONE;
  re__prog_inst new_inst;
  re__str_view str_view =
      re__ast_root_get_str_view(compile->ast_root, re__ast_get_str_ref(ast));
  re_size i;
  re_size sz = re__str_view_size(&str_view);
  for (i = 0; i < sz; i++) {
    if (!compile->reversed) {
      re__prog_inst_init_byte(
          &new_inst, (re_uint8)re__str_view_get_data(&str_view)[i]);
    } else {
      re__prog_inst_init_byte(
          &new_inst, (re_uint8)re__str_view_get_data(&str_view)[(sz - 1) - i]);
    }
    if (i == sz - 1) {
      /* Add an outgoing patch (1) */
      re__compile_patches_append(&frame->patches, prog, re__prog_size(prog), 0);
    } else {
      re__prog_inst_set_primary(&new_inst, re__prog_size(prog) + 1);
    }
    if ((err = re__prog_add(prog, new_inst))) {
      return err;
    }
  }
  return err;
}

RE_INTERNAL re_error re__compile_do_charclass(
    re__compile* compile, re__compile_frame* frame, const re__ast* ast,
    re__prog* prog)
{
  /* Generates a character class, which is a complex series of Byte and Split
   * instructions. */
  const re__charclass* charclass =
      re__ast_root_get_charclass(compile->ast_root, ast->_data.charclass_ref);
  return re__compile_charclass_gen(
      &compile->char_comp, charclass, prog, &frame->patches, compile->reversed);
}

RE_INTERNAL re_error re__compile_do_concat(
    re__compile* compile, re__compile_frame* frame, const re__ast* ast,
    re__prog* prog)
{
  /* Generates each child node, and patches them all together, leaving the
   * final child's outgoing branch targets unpatched. */
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
  /* Assert that there are children to compile.
   * Currently, we disallow concats with zero children. */
  RE_ASSERT(ast->first_child_ref != RE__AST_NONE);
  if (!compile->reversed) {
    if (frame->ast_child_ref == RE__AST_NONE) {
      /* Before first child */
      compile->should_push_child = 1;
      compile->should_push_child_ref = ast->first_child_ref;
      frame->ast_child_ref = ast->first_child_ref;
    } else {
      const re__ast* prev_child =
          re__ast_root_get_const(compile->ast_root, frame->ast_child_ref);
      if (prev_child->next_sibling_ref != RE__AST_NONE) {
        /* Patch outgoing branches (1, 2, 3) */
        re__compile_patches_patch(
            &compile->returned_frame.patches, prog, re__prog_size(prog));
        compile->should_push_child = 1;
        compile->should_push_child_ref = prev_child->next_sibling_ref;
        frame->ast_child_ref = prev_child->next_sibling_ref;
      } else {
        /* We are done */
        re__compile_patches_merge(
            &frame->patches, prog, &compile->returned_frame.patches);
      }
    }
  } else {
    if (frame->ast_child_ref == RE__AST_NONE) {
      /* Before *last* child */
      compile->should_push_child = 1;
      compile->should_push_child_ref = ast->last_child_ref;
      frame->ast_child_ref = ast->last_child_ref;
    } else {
      const re__ast* next_child =
          re__ast_root_get_const(compile->ast_root, frame->ast_child_ref);
      if (next_child->prev_sibling_ref != RE__AST_NONE) {
        re__compile_patches_patch(
            &compile->returned_frame.patches, prog, re__prog_size(prog));
        compile->should_push_child = 1;
        compile->should_push_child_ref = next_child->prev_sibling_ref;
        frame->ast_child_ref = next_child->prev_sibling_ref;
      } else {
        re__compile_patches_merge(
            &frame->patches, prog, &compile->returned_frame.patches);
      }
    }
  }
  return RE_ERROR_NONE; /* <- cool! */
}

RE_INTERNAL re_error re__compile_do_alt(
    re__compile* compile, re__compile_frame* frame, const re__ast* ast,
    re__prog* prog)
{
  /* For each child node except for the last one, generates a SPLIT
   * instruction, and then the instructions of the child. Each split
   * instruction's primary branch target is patched to the next child, and the
   * secondary branch targets are patched to the next split instruction.
   * Leaves each child's outgoing branch targets unpatched. */
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
  re_error err = RE_ERROR_NONE;
  RE_ASSERT(ast->first_child_ref != RE__AST_NONE);
  /* Unlike concats, we still go through alterations in the correct order. */
  /* if (!compile->reversed || compile->reversed) { */
  if (frame->ast_child_ref == RE__AST_NONE) {
    /* Before *first* child */
    if (re__ast_root_get_const(compile->ast_root, ast->first_child_ref)
            ->next_sibling_ref != RE__AST_NONE) {
      /* Alt has more than one child */
      /* Initialize split instruction */
      re__prog_inst new_inst;
      re__prog_inst_init_split(
          &new_inst, re__prog_size(prog) + 1, /* Outgoing branch (1) */
          RE__PROG_LOC_INVALID /* Will become outgoing branch (2) */
      );
      /* Add the Split instruction */
      if ((err = re__prog_add(prog, new_inst))) {
        return err;
      }
    }
    compile->should_push_child = 1;
    compile->should_push_child_ref = ast->first_child_ref;
    frame->ast_child_ref = ast->first_child_ref;
  } else {
    const re__ast* prev_child =
        re__ast_root_get_const(compile->ast_root, frame->ast_child_ref);
    if (compile->set != frame->ast_base_ref) {
      /* Collect outgoing branches (5, 6, 7, 8, 9, 10). */
      re__compile_patches_merge(
          &frame->patches, prog, &compile->returned_frame.patches);
    } else {
      /* This alt node is the set alt, so generate a match instruction after
       * every completed child */
      re__prog_inst new_inst;
      /* Add a match instruction, and link all returned patches to that */
      /* Increment rep_idx because set match offsets start at 1, not 0 */
      re__prog_inst_init_match(&new_inst, (re_uint32)++frame->rep_idx);
      if ((err = re__prog_add(prog, new_inst))) {
        return err;
      }
      re__compile_patches_patch(
          &compile->returned_frame.patches, prog, re__prog_size(prog) - 1);
    }
    if (prev_child->next_sibling_ref != RE__AST_NONE) {
      /* Before intermediate children */
      /* Patch the secondary branch target of the old SPLIT
       * instruction. Corresponds to outgoing branch (2). */
      /* top.seg.end points to the instruction after the old split
       * instruction, since we didn't set the endpoint before the
       * first child. */
      re__prog_loc old_split_loc = frame->end - 1;
      re__prog_inst* old_inst = re__prog_get(prog, old_split_loc);
      const re__ast* child;
      /* Patch outgoing branch (2). */
      re__prog_inst_set_split_secondary(old_inst, re__prog_size(prog));
      child = re__ast_root_get_const(
          compile->ast_root, prev_child->next_sibling_ref);
      if (child->next_sibling_ref != RE__AST_NONE) {
        /* Before intermediate children and NOT last child */
        re__prog_inst new_inst;
        /* Create the intermediary SPLIT instruction, if there
         * are more than two child nodes in the alternation. */
        re__prog_inst_init_split(
            &new_inst, re__prog_size(prog) + 1, /* Outgoing branch (3) */
            RE__PROG_LOC_INVALID                /* Outgoing branch (4) */
        );
        /* Add it to the program. */
        if ((err = re__prog_add(prog, new_inst))) {
          return err;
        }
      }
      compile->should_push_child = 1;
      compile->should_push_child_ref = prev_child->next_sibling_ref;
      frame->ast_child_ref = prev_child->next_sibling_ref;
    }
  }
  return err;
}

RE_INTERNAL re_error re__compile_do_quantifier(
    re__compile* compile, re__compile_frame* frame, const re__ast* ast,
    re__prog* prog)
{
  /*   *   min=0 max=INF */ /* Spl E -> 1 to 2 and out, 2 to 1 */
  /*   +   min=1 max=INF */ /* E Spl -> 1 to 2, 2 to 1 and out */
  /*   ?   min=0 max=1   */ /* Spl E -> 1 to 2 and out, 2 to out */
  /*  {n}  min=n max=n+1 */ /* E repeated -> out */
  /* {n, } min=n max=INF */ /* E repeated, Spl -> spl to last and out */
  /* {n,m} min=n max=m+1 */ /* E repeated, E Spl E Spl E*/
  /*   *   min=0 max=INF */
  /*   +   min=1 max=INF */ /* E Spl -> 1 to 2, 2 to 1 and out */
  /*   ?   min=0 max=1   */ /* Spl E -> 1 to 2 and out, 2 to out */
  /*  {n}  min=n max=n+1 */ /* E repeated -> out */
  /* {n, } min=n max=INF */ /* E repeated, Spl -> spl to last and out */
  /* {n,m} min=n max=m+1 */ /* E repeated, E Spl E Spl E*/
  re_error err = RE_ERROR_NONE;
  re_int32 min = re__ast_get_quantifier_min(ast);
  re_int32 max = re__ast_get_quantifier_max(ast);
  re_int32 int_idx = frame->rep_idx;
  re__prog_loc this_start_pc = re__prog_size(prog);
  frame->rep_idx++;
  RE_ASSERT(ast->first_child_ref != RE__AST_NONE);
  RE_ASSERT(ast->first_child_ref == ast->last_child_ref);
  /* Quantifiers are the same when reversed */
  if (int_idx < min) {
    /* Generate child min times */
    if (int_idx > 0) {
      /* Patch previous */
      re__compile_patches_patch(
          &compile->returned_frame.patches, prog, this_start_pc);
    }
    compile->should_push_child = 1;
    compile->should_push_child_ref = ast->first_child_ref;
    frame->ast_child_ref = ast->first_child_ref;
  } else { /* int_idx >= min */
    if (max == RE__AST_QUANTIFIER_INFINITY) {
      re__prog_inst new_spl;
      if (min == 0) {
        RE_ASSERT(int_idx == 0 || int_idx == 1);
        if (int_idx == 0) {
          if (re__ast_get_quantifier_greediness(ast)) {
            re__prog_inst_init_split(
                &new_spl, this_start_pc + 1, RE__PROG_LOC_INVALID);
            if ((err = re__prog_add(prog, new_spl))) {
              return err;
            }
            re__compile_patches_append(&frame->patches, prog, this_start_pc, 1);
          } else {
            re__prog_inst_init_split(
                &new_spl, RE__PROG_LOC_INVALID, this_start_pc + 1);
            if ((err = re__prog_add(prog, new_spl))) {
              return err;
            }
            re__compile_patches_append(&frame->patches, prog, this_start_pc, 0);
          }
          compile->should_push_child = 1;
          compile->should_push_child_ref = ast->first_child_ref;
          frame->ast_child_ref = ast->first_child_ref;
        } else { /* int_idx == 1 */
          re__compile_patches_patch(
              &compile->returned_frame.patches, prog, frame->end - 1);
        }
      } else {
        re__compile_patches_patch(
            &compile->returned_frame.patches, prog, this_start_pc);
        if (re__ast_get_quantifier_greediness(ast)) {
          re__prog_inst_init_split(
              &new_spl, compile->returned_frame.start, RE__PROG_LOC_INVALID);
          if ((err = re__prog_add(prog, new_spl))) {
            return err;
          }
          re__compile_patches_append(&frame->patches, prog, this_start_pc, 1);
        } else {
          re__prog_inst_init_split(
              &new_spl, RE__PROG_LOC_INVALID, compile->returned_frame.start);
          if ((err = re__prog_add(prog, new_spl))) {
            return err;
          }
          re__compile_patches_append(&frame->patches, prog, this_start_pc, 0);
        }
      }
    } else {
      if (int_idx < max - 1) {
        re__prog_inst new_spl;
        if (int_idx > 0) {
          re__compile_patches_patch(
              &compile->returned_frame.patches, prog, this_start_pc);
        }
        if (re__ast_get_quantifier_greediness(ast)) {
          re__prog_inst_init_split(
              &new_spl, this_start_pc + 1, RE__PROG_LOC_INVALID);
          if ((err = re__prog_add(prog, new_spl))) {
            return err;
          }
          re__compile_patches_append(&frame->patches, prog, this_start_pc, 1);
        } else {
          re__prog_inst_init_split(
              &new_spl, RE__PROG_LOC_INVALID, this_start_pc + 1);
          if ((err = re__prog_add(prog, new_spl))) {
            return err;
          }
          re__compile_patches_append(&frame->patches, prog, this_start_pc, 0);
        }
        compile->should_push_child = 1;
        compile->should_push_child_ref = ast->first_child_ref;
        frame->ast_child_ref = ast->first_child_ref;
      } else {
        re__compile_patches_merge(
            &frame->patches, prog, &compile->returned_frame.patches);
      }
    }
  }
  return err;
}

RE_INTERNAL re_error re__compile_do_group(
    re__compile* compile, re__compile_frame* frame, const re__ast* ast,
    re__prog* prog)
{
  re_error err = RE_ERROR_NONE;
  re__prog_inst new_inst;
  re_uint32 group_idx;
  re__ast_group_flags group_flags = re__ast_get_group_flags(ast);
  RE_ASSERT(ast->first_child_ref != RE__AST_NONE);
  RE_ASSERT(ast->first_child_ref == ast->last_child_ref);
  if (frame->ast_child_ref == RE__AST_NONE) {
    /* Before child */
    if (!(group_flags & RE__AST_GROUP_FLAG_NONMATCHING)) {
      group_idx = re__ast_get_group_idx(ast);
      if (!compile->reversed) {
        re__prog_inst_init_save(&new_inst, group_idx * 2);
      } else {
        re__prog_inst_init_save(&new_inst, group_idx * 2 + 1);
      }
      re__prog_inst_set_primary(&new_inst, re__prog_size(prog) + 1);
      if ((err = re__prog_add(prog, new_inst))) {
        return err;
      }
    }
    compile->should_push_child = 1;
    compile->should_push_child_ref = ast->first_child_ref;
    frame->ast_child_ref = ast->first_child_ref;
  } else {
    /* After child */
    if (!(group_flags & RE__AST_GROUP_FLAG_NONMATCHING)) {
      re__prog_loc save_pc = re__prog_size(prog);
      group_idx = re__ast_get_group_idx(ast);
      if (!compile->reversed) {
        re__prog_inst_init_save(&new_inst, group_idx * 2 + 1);
      } else {
        re__prog_inst_init_save(&new_inst, group_idx * 2);
      }
      if ((err = re__prog_add(prog, new_inst))) {
        return err;
      }
      re__compile_patches_patch(
          &compile->returned_frame.patches, prog, save_pc);
      re__compile_patches_append(&frame->patches, prog, save_pc, 0);
    } else {
      re__compile_patches_merge(
          &frame->patches, prog, &compile->returned_frame.patches);
    }
  }
  return err;
}

RE_INTERNAL re_error re__compile_do_assert(
    re__compile* compile, re__compile_frame* frame, const re__ast* ast,
    re__prog* prog)
{
  /* Generates a single Assert instruction. */
  /*    +0
   * ~~~+-------+~~~
   * .. |  Ass  | ..
   * .. | Instr | ..
   * ~~~+---+---+~~~
   *        |
   *        +-----1--> ... */
  re_error err = RE_ERROR_NONE;
  re__prog_inst new_inst;
  re__prog_loc assert_pc = re__prog_size(prog);
  re__assert_type assert_type = re__ast_get_assert_type(ast);
  RE__UNUSED(compile);
  if (compile->reversed) {
    re__assert_type new_assert_type = 0;
    if (assert_type & RE__ASSERT_TYPE_TEXT_START) {
      new_assert_type |= RE__ASSERT_TYPE_TEXT_END;
    }
    if (assert_type & RE__ASSERT_TYPE_TEXT_END) {
      new_assert_type |= RE__ASSERT_TYPE_TEXT_START;
    }
    if (assert_type & RE__ASSERT_TYPE_TEXT_START_ABSOLUTE) {
      new_assert_type |= RE__ASSERT_TYPE_TEXT_END_ABSOLUTE;
    }
    if (assert_type & RE__ASSERT_TYPE_TEXT_END_ABSOLUTE) {
      new_assert_type |= RE__ASSERT_TYPE_TEXT_START_ABSOLUTE;
    }
    /* word boundaries are symmetric */
    if (assert_type & RE__ASSERT_TYPE_WORD) {
      new_assert_type |= RE__ASSERT_TYPE_WORD;
    }
    if (assert_type & RE__ASSERT_TYPE_WORD_NOT) {
      new_assert_type |= RE__ASSERT_TYPE_WORD_NOT;
    }
    assert_type = new_assert_type;
  }
  re__prog_inst_init_assert(
      &new_inst, (re_uint32)assert_type); /* Creates unpatched branch (1) */
  if ((err = re__prog_add(prog, new_inst))) {
    return err;
  }
  /* Add an outgoing patch (1) */
  re__compile_patches_append(&frame->patches, prog, assert_pc, 0);
  return err;
}

RE_INTERNAL re_error re__compile_do_any_char(
    re__compile* compile, re__compile_frame* frame, const re__ast* ast,
    re__prog* prog)
{
  /* Generates a sequence of instructions corresponding to a single
   * UTF-8 codepoint. */
  re__prog_data_id id = RE__PROG_DATA_ID_DOT_FWD_REJSURR_REJNL;
  RE__UNUSED(ast);
  if (compile->reversed) {
    id = RE__PROG_DATA_ID_DOT_REV_REJSURR_REJNL;
  }
  return re__prog_data_decompress(
      prog, re__prog_data[id], re__prog_data_size[id], &frame->patches);
}

RE_INTERNAL re_error re__compile_do_any_char_newline(
    re__compile* compile, re__compile_frame* frame, const re__ast* ast,
    re__prog* prog)
{
  re__prog_data_id id = RE__PROG_DATA_ID_DOT_FWD_REJSURR_ACCNL;
  RE__UNUSED(ast);
  if (compile->reversed) {
    id = RE__PROG_DATA_ID_DOT_REV_REJSURR_ACCNL;
  }
  return re__prog_data_decompress(
      prog, re__prog_data[id], re__prog_data_size[id], &frame->patches);
}

RE_INTERNAL re_error re__compile_do_any_byte(
    re__compile* compile, re__compile_frame* frame, const re__ast* ast,
    re__prog* prog)
{
  /* Generates a single Byte Range instruction. */
  /*    +0
   * ~~~+-------+~~~
   * .. | ByteR | ..
   * .. | Instr | ..
   * ~~~+---+---+~~~
   *        |
   *        +-----1--> ... */
  re_error err = RE_ERROR_NONE;
  re__prog_inst new_inst;
  re__prog_loc byte_range_pc = re__prog_size(prog);
  re__byte_range br;
  RE__UNUSED(compile);
  RE__UNUSED(ast);
  br.min = 0;
  br.max = 255;
  re__prog_inst_init_byte_range(
      &new_inst, br); /* Creates unpatched branch (1) */
  if ((err = re__prog_add(prog, new_inst))) {
    return err;
  }
  /* Add an outgoing patch (1) */
  re__compile_patches_append(&frame->patches, prog, byte_range_pc, 0);
  return err;
}

RE_INTERNAL re_error re__compile_regex(
    re__compile* compile, const re__ast_root* ast_root, re__prog* prog,
    int reversed, re_int32 set_root)
{
  re_error err = RE_ERROR_NONE;
  re__compile_frame initial_frame;
  re__compile_patches initial_patches;
  re__prog_inst fail_inst;
  re__prog_loc entry;
  /* Set ast_root */
  compile->ast_root = ast_root;
  /* Set reversed flag, more economical to set here */
  compile->reversed = reversed;
  /* Set set root */
  compile->set = set_root;
  /* Allocate memory for frames */
  /* depth_max + 1 because we include an extra frame for terminals within the
   * deepest multi-child node */
  if ((err = re__ast_root_get_depth(ast_root, &compile->frames_size))) {
    return err;
  }
  compile->frames = (re__compile_frame*)RE_MALLOC(
      (sizeof(re__compile_frame) * ((re_size)compile->frames_size)));
  if (compile->frames == RE_NULL) {
    err = RE_ERROR_NOMEM;
    goto error;
  }
  /* Can't run compiler twice */
  RE_ASSERT(
      re__prog_get_entry(prog, RE__PROG_ENTRY_DEFAULT) == RE__PROG_LOC_INVALID);
  /* Add the FAIL instruction to the program */
  re__prog_inst_init_fail(&fail_inst);
  if ((err = re__prog_add(prog, fail_inst))) {
    goto error;
  }
  entry = re__prog_size(prog);
  re__compile_patches_init(&initial_patches);
  /* Start first frame */
  re__compile_frame_init(
      &initial_frame, ast_root->root_ref, RE__AST_NONE, initial_patches, 0, 0);
  /* Push it */
  re__compile_frame_push(compile, initial_frame);
  /* While there are nodes left to compile... */
  while (compile->frame_ptr != 0) {
    re__compile_frame top_frame = re__compile_frame_pop(compile);
    const re__ast* top_node;
    re__ast_type top_node_type;
    if (top_frame.ast_base_ref == RE__AST_NONE) {
      /* empty regex */
      top_node = RE_NULL;
      top_node_type = RE__AST_TYPE_NONE;
    } else {
      top_node = re__ast_root_get_const(ast_root, top_frame.ast_base_ref);
      top_node_type = top_node->type;
    }
    compile->should_push_child = 0;
    compile->should_push_child_ref = top_frame.ast_child_ref;
    RE_ASSERT(top_node_type < RE__AST_TYPE_MAX);
    switch (top_node_type) {
    case RE__AST_TYPE_RUNE:
      err = re__compile_do_rune(compile, &top_frame, top_node, prog);
      break;
    case RE__AST_TYPE_STR:
      err = re__compile_do_str(compile, &top_frame, top_node, prog);
      break;
    case RE__AST_TYPE_CHARCLASS:
      err = re__compile_do_charclass(compile, &top_frame, top_node, prog);
      break;
    case RE__AST_TYPE_CONCAT:
      err = re__compile_do_concat(compile, &top_frame, top_node, prog);
      break;
    case RE__AST_TYPE_ALT:
      err = re__compile_do_alt(compile, &top_frame, top_node, prog);
      break;
    case RE__AST_TYPE_QUANTIFIER:
      err = re__compile_do_quantifier(compile, &top_frame, top_node, prog);
      break;
    case RE__AST_TYPE_GROUP:
      err = re__compile_do_group(compile, &top_frame, top_node, prog);
      break;
    case RE__AST_TYPE_ASSERT:
      err = re__compile_do_assert(compile, &top_frame, top_node, prog);
      break;
    case RE__AST_TYPE_ANY_CHAR:
      err = re__compile_do_any_char(compile, &top_frame, top_node, prog);
      break;
    case RE__AST_TYPE_ANY_CHAR_NEWLINE:
      err =
          re__compile_do_any_char_newline(compile, &top_frame, top_node, prog);
      break;
    case RE__AST_TYPE_ANY_BYTE:
      err = re__compile_do_any_byte(compile, &top_frame, top_node, prog);
      break;
    default: /* RE__AST_TYPE_NONE */
      break;
    }
    if (err) {
      goto error;
    }
    /* Set the end of the segment to the next instruction */
    top_frame.end = re__prog_size(prog);
    if (compile->should_push_child) {
      re__compile_frame up_frame;
      re__compile_patches up_patches;

      RE_ASSERT(compile->should_push_child_ref != RE__AST_NONE);
      re__compile_frame_push(compile, top_frame);
      /* Prepare the child's patches */
      re__compile_patches_init(&up_patches);
      /* Prepare the child's stack frame */
      re__compile_frame_init(
          &up_frame, compile->should_push_child_ref, RE__AST_NONE, up_patches,
          top_frame.end, top_frame.end);
      re__compile_frame_push(compile, up_frame);
    }
    compile->returned_frame = top_frame;
  }
  /* There should be no more frames. */
  RE_ASSERT(compile->frame_ptr == 0);
  /* If in set mode, returned patches should not have anything. */
  RE_ASSERT(RE__IMPLIES(
      compile->set != RE__AST_NONE,
      compile->returned_frame.patches.first_inst == RE__PROG_LOC_INVALID));
  if (compile->set == RE__AST_NONE) {
    re__prog_inst match_inst;
    /* Link the returned patches to a final MATCH instruction. */
    re__compile_patches_patch(
        &compile->returned_frame.patches, prog, re__prog_size(prog));
    re__prog_inst_init_match(&match_inst, 1);
    if ((err = re__prog_add(prog, match_inst))) {
      goto error;
    }
  }
  RE_FREE(compile->frames);
  compile->frames = RE_NULL;
  re__prog_set_entry(prog, RE__PROG_ENTRY_DEFAULT, entry);
  return err;
error:
  if (compile->frames != RE_NULL) {
    RE_FREE(compile->frames);
  }
  compile->frames = RE_NULL;
  return err;
}

RE_INTERNAL re_error
re__compile_dotstar(re__prog* prog, re__prog_data_id data_id)
{
  re__compile_patches patches;
  re__prog_loc entry = re__prog_size(prog);
  re__prog_inst inst;
  re_error err = RE_ERROR_NONE;
  RE_ASSERT(
      re__prog_get_entry(prog, RE__PROG_ENTRY_DOTSTAR) == RE__PROG_LOC_INVALID);
  RE_ASSERT(
      re__prog_get_entry(prog, RE__PROG_ENTRY_DEFAULT) != RE__PROG_LOC_INVALID);
  re__prog_inst_init_split(
      &inst, re__prog_get_entry(prog, RE__PROG_ENTRY_DEFAULT), entry + 1);
  if ((err = re__prog_add(prog, inst))) {
    return err;
  }
  re__compile_patches_init(&patches);
  if ((err = re__prog_data_decompress(
           prog, re__prog_data[data_id], re__prog_data_size[data_id],
           &patches))) {
    return err;
  }
  re__compile_patches_patch(&patches, prog, entry);
  re__prog_set_entry(prog, RE__PROG_ENTRY_DOTSTAR, entry);
  return err;
}

#if RE_DEBUG

void re__compile_debug_dump(re__compile* compile)
{
  re_int32 i;
  printf(
      "%u frames / %u frames:\n", (re_uint32)compile->frame_ptr,
      (re_uint32)compile->frames_size);
  for (i = 0; i < compile->frames_size; i++) {
    re__compile_frame* cur_frame = &compile->frames[i];
    printf("  Frame %u:\n", i);
    printf("    AST root reference: %i\n", cur_frame->ast_base_ref);
    printf("    AST child reference: %i\n", cur_frame->ast_child_ref);
    printf("    Start loc: %u\n", cur_frame->start);
    printf("    End loc: %u\n", cur_frame->end);
  }
}

#endif

/* re */
RE__VEC_IMPL_FUNC(re__exec_dfa_state_ptr, init)
RE__VEC_IMPL_FUNC(re__exec_dfa_state_ptr, destroy)
RE__VEC_IMPL_FUNC(re__exec_dfa_state_ptr, push)
RE__VEC_IMPL_FUNC(re__exec_dfa_state_ptr, size)
RE__VEC_IMPL_FUNC(re__exec_dfa_state_ptr, get)

RE__VEC_IMPL_FUNC(re_uint32_ptr, init)
RE__VEC_IMPL_FUNC(re_uint32_ptr, destroy)
RE__VEC_IMPL_FUNC(re_uint32_ptr, push)
RE__VEC_IMPL_FUNC(re_uint32_ptr, size)
RE__VEC_IMPL_FUNC(re_uint32_ptr, get)

void re__exec_dfa_state_init(
    re__exec_dfa_state* state, re_uint32* thrd_locs_begin,
    re_uint32* thrd_locs_end)
{
  re_uint32 i;
  for (i = 0; i < RE__EXEC_SYM_MAX; i++) {
    state->next[i] = RE_NULL;
  }
  state->flags = 0;
  state->thrd_locs_begin = thrd_locs_begin;
  state->thrd_locs_end = thrd_locs_end;
  state->match_index = 0;
}

#define RE__EXEC_DFA_PTR_MASK (~0x3)

RE_INTERNAL re__exec_dfa_state*
re__exec_dfa_state_get_next(re__exec_dfa_state* state, re_uint8 sym)
{
#if !RE__EXEC_DFA_SMALL_STATE
  return state->next[sym];
#else
  return state->next[sym] & RE__EXEC_DFA_PTR_MASK;
#endif
}

RE_INTERNAL int re__exec_dfa_state_is_match(re__exec_dfa_state* state)
{
  return state->flags & RE__EXEC_DFA_FLAG_MATCH;
}

RE_INTERNAL int re__exec_dfa_state_is_priority(re__exec_dfa_state* state)
{
  return !(state->flags & RE__EXEC_DFA_FLAG_MATCH_PRIORITY);
}

RE_INTERNAL int re__exec_dfa_state_is_empty(re__exec_dfa_state* state)
{
  return (state->thrd_locs_end - state->thrd_locs_begin) == 0;
}

RE_INTERNAL re_uint32
re__exec_dfa_state_get_match_index(re__exec_dfa_state* state)
{
  return state->match_index;
}

void re__exec_dfa_init(re__exec_dfa* exec, const re__prog* prog)
{
  exec->current_state = RE_NULL;
  {
    int i;
    for (i = 0; i < RE__EXEC_DFA_START_STATE_COUNT * RE__PROG_ENTRY_MAX; i++) {
      exec->start_states[i] = RE_NULL;
    }
  }
  re__exec_dfa_state_ptr_vec_init(&exec->state_pages);
  exec->state_page_idx = 0;
  re_uint32_ptr_vec_init(&exec->thrd_loc_pages);
  exec->thrd_loc_page_idx = 0;
  re__exec_nfa_init(&exec->nfa, prog, 0);
  exec->cache = RE_NULL;
  exec->cache_stored = 0;
  exec->cache_alloc = 0;
}

void re__exec_dfa_destroy(re__exec_dfa* exec)
{
  if (exec->cache) {
    RE_FREE(exec->cache);
  }
  re__exec_nfa_destroy(&exec->nfa);
  {
    re_size i;
    for (i = 0; i < re_uint32_ptr_vec_size(&exec->thrd_loc_pages); i++) {
      RE_FREE(re_uint32_ptr_vec_get(&exec->thrd_loc_pages, i));
    }
  }
  re_uint32_ptr_vec_destroy(&exec->thrd_loc_pages);
  {
    re_size i;
    for (i = 0; i < re__exec_dfa_state_ptr_vec_size(&exec->state_pages); i++) {
      RE_FREE(re__exec_dfa_state_ptr_vec_get(&exec->state_pages, i));
    }
  }
  re__exec_dfa_state_ptr_vec_destroy(&exec->state_pages);
}

re_error re__exec_dfa_stash_loc_set(
    re__exec_dfa* exec, const re__exec_thrd* thrds, re_size thrds_size,
    re_uint32** out_locs_begin, re_uint32** out_locs_end)
{
  re_error err = RE_ERROR_NONE;
  re_uint32* thrds_page;
  if (thrds_size == 0) {
    /* set to NULL, it's fine because these will never be dereferenced */
    *out_locs_begin = RE_NULL;
    *out_locs_end = RE_NULL;
  }
  if (exec->thrd_loc_page_idx != 0) {
    re_uint32 logical_page_idx =
        exec->thrd_loc_page_idx % RE__EXEC_DFA_THRD_LOCS_PAGE_SIZE;
    re_uint32 space_left = RE__EXEC_DFA_THRD_LOCS_PAGE_SIZE - logical_page_idx;
    RE_ASSERT(re_uint32_ptr_vec_size(&exec->thrd_loc_pages));
    /* space left in previous page */
    if (space_left >= thrds_size) {
      re_uint32* last_page = re_uint32_ptr_vec_get(
          &exec->thrd_loc_pages,
          re_uint32_ptr_vec_size(&exec->thrd_loc_pages) - 1);
      thrds_page = last_page + exec->thrd_loc_page_idx;
    } else {
      /* need a new page */
      /* fallthrough to next if block */
      exec->thrd_loc_page_idx = 0;
    }
  }
  if (exec->thrd_loc_page_idx == 0) {
    /* need to allocate a page */
    /* round down to figure out how many pages to allocate */
    re_size pages_needed = (thrds_size + RE__EXEC_DFA_THRD_LOCS_PAGE_SIZE) /
                           RE__EXEC_DFA_THRD_LOCS_PAGE_SIZE;
    thrds_page = RE_MALLOC(
        sizeof(re_size) * RE__EXEC_DFA_THRD_LOCS_PAGE_SIZE * pages_needed);
    if (thrds_page == RE_NULL) {
      return RE_ERROR_NOMEM;
    }
    if ((err = re_uint32_ptr_vec_push(&exec->thrd_loc_pages, thrds_page))) {
      RE_FREE(thrds_page);
      return err;
    }
  }
  {
    /* assert that there is enough space left */
    RE_ASSERT(
        RE__EXEC_DFA_THRD_LOCS_PAGE_SIZE -
            (exec->thrd_loc_page_idx % RE__EXEC_DFA_THRD_LOCS_PAGE_SIZE) >=
        thrds_size);
  }
  {
    re_size i;
    *out_locs_begin = thrds_page;
    for (i = 0; i < thrds_size; i++) {
      re__prog_loc loc = thrds[i].loc;
      thrds_page[i] = loc;
    }
    *out_locs_end = thrds_page + i;
    exec->thrd_loc_page_idx += thrds_size;
    if (exec->thrd_loc_page_idx % RE__EXEC_DFA_THRD_LOCS_PAGE_SIZE == 0) {
      /* exhausted this page, indicate a reallocation for the next time */
      exec->thrd_loc_page_idx = 0;
    }
  }
  return err;
}

re_error re__exec_dfa_get_new_state(
    re__exec_dfa* exec, const re__exec_thrd* thrds, re_size thrds_size,
    re__exec_dfa_state** out_state)
{
  re_error err = RE_ERROR_NONE;
  re__exec_dfa_state* page;
  re_uint32* state_thrds_begin;
  re_uint32* state_thrds_end;
  if (exec->state_page_idx == 0) {
    /* need to allocate a new page */
    page = RE_MALLOC(sizeof(re__exec_dfa_state) * RE__EXEC_DFA_PAGE_SIZE);
    if (page == RE_NULL) {
      return RE_ERROR_NOMEM;
    }
    if ((err = re__exec_dfa_state_ptr_vec_push(&exec->state_pages, page))) {
      RE_FREE(page);
      return err;
    }
  } else {
    page = re__exec_dfa_state_ptr_vec_get(
        &exec->state_pages,
        re__exec_dfa_state_ptr_vec_size(&exec->state_pages) - 1);
  }
  *out_state = page + exec->state_page_idx;
  if ((err = re__exec_dfa_stash_loc_set(
           exec, thrds, thrds_size, &state_thrds_begin, &state_thrds_end))) {
    return err;
  }
  re__exec_dfa_state_init(*out_state, state_thrds_begin, state_thrds_end);
  if (exec->state_page_idx == RE__EXEC_DFA_PAGE_SIZE) {
    exec->state_page_idx = 0;
  }
  return err;
}

#define RE__EXEC_DFA_CACHE_INITIAL_SIZE 16

int re__exec_dfa_state_equal(
    const re__exec_thrd* thrds, re__prog_loc thrds_size,
    re__exec_dfa_flags flags, re__exec_dfa_state* state)
{
  re_size i;
  if (state->flags != flags) {
    return 0;
  }
  if ((state->thrd_locs_end - state->thrd_locs_begin) != thrds_size) {
    return 0;
  }
  for (i = 0; i < thrds_size; i++) {
    if (state->thrd_locs_begin[i] != thrds[i].loc) {
      return 0;
      break;
    }
  }
  return 1;
}

re_error re__exec_dfa_get_state(
    re__exec_dfa* exec, re__exec_dfa_flags flags,
    re__exec_dfa_state** out_state)
{
  re_error err = RE_ERROR_NONE;
  re__prog_loc thrds_size = re__exec_nfa_get_thrds_size(&exec->nfa);
  const re__exec_thrd* thrds = re__exec_nfa_get_thrds(&exec->nfa);
  re_uint32 match_index = re__exec_nfa_get_match_index(&exec->nfa);
  re_uint32 match_priority = re__exec_nfa_get_match_priority(&exec->nfa);
  re_uint32 hash = 0;
  if (match_index) {
    flags |= RE__EXEC_DFA_FLAG_MATCH;
  }
  if (match_priority) {
    flags |= RE__EXEC_DFA_FLAG_MATCH_PRIORITY;
  }
  /* ok to cast to uint8, no ambiguous padding inside of flags */
  hash = re__murmurhash3_32(hash, (const re_uint8*)&flags, sizeof(flags));
  {
    re_size i;
    for (i = 0; i < thrds_size; i++) {
      /* ok to cast to uint8, also no ambiguous padding */
      hash = re__murmurhash3_32(
          hash, (const re_uint8*)&thrds[i].loc, sizeof(re__prog_loc));
    }
  }
  /* lookup in cache */
  if (!exec->cache) {
    /* cache has not been initialized */
    exec->cache_alloc = RE__EXEC_DFA_CACHE_INITIAL_SIZE;
    exec->cache =
        RE_MALLOC(sizeof(re__exec_dfa_cache_entry) * exec->cache_alloc);
    if (exec->cache == RE_NULL) {
      return RE_ERROR_NOMEM;
    }
    re__memset(
        exec->cache, 0, sizeof(re__exec_dfa_cache_entry) * exec->cache_alloc);
  }
  {
    /* lookup in cache */
    re__exec_dfa_cache_entry* probe = exec->cache + (hash % exec->cache_alloc);
    re_size q = 1;
    while (1) {
      if (probe->state_ptr == RE_NULL) {
        /* not found, but slot is empty so we can use that */
        break;
      } else {
        if (probe->hash == hash) {
          /* collision or found */
          re__exec_dfa_state* state = probe->state_ptr;
          /* check if state is equal */
          if (re__exec_dfa_state_equal(thrds, thrds_size, flags, state)) {
            *out_state = state;
            return RE_ERROR_NONE;
          }
        }
      }
      /* otherwise, find a new slot */
      probe = exec->cache + ((q + hash) % exec->cache_alloc);
      q++;
    }
    /* if not found in cache, we are here */
    /* store in cache */
    {
      re__exec_dfa_state* new_state;
      if ((err = re__exec_dfa_get_new_state(
               exec, thrds, thrds_size, &new_state))) {
        return err;
      }
      /* Set input flags (start state, etc) */
      new_state->flags |= flags;
      new_state->match_index = match_index;
      probe->hash = hash;
      probe->state_ptr = new_state;
      exec->cache_stored++;
      *out_state = new_state;
    }
    /* check load factor and resize if necessary (0.75 in this case) */
    /* calculation: x - 0.25 * x == 0.75 * x    |    (0.25*x == x >> 2) */
    if (exec->cache_stored == (exec->cache_alloc - (exec->cache_alloc >> 2))) {
      /* need to resize */
      re_size old_alloc = exec->cache_alloc;
      re_size i;
      re__exec_dfa_cache_entry* old_cache = exec->cache;
      exec->cache_alloc *= 2;
      exec->cache =
          RE_MALLOC(sizeof(re__exec_dfa_cache_entry) * exec->cache_alloc);
      if (exec->cache == RE_NULL) {
        return RE_ERROR_NOMEM;
      }
      re__memset(
          exec->cache, 0, sizeof(re__exec_dfa_cache_entry) * exec->cache_alloc);
      /* rehash */
      for (i = 0; i < old_alloc; i++) {
        re__exec_dfa_cache_entry* old_entry = old_cache + i;
        probe = exec->cache + (old_entry->hash % exec->cache_alloc);
        q = 1;
        while (probe->state_ptr != RE_NULL) {
          probe = exec->cache + ((q + old_entry->hash) % exec->cache_alloc);
          q++;
        }
        *probe = *old_entry;
      }
      RE_FREE(old_cache);
    }
  }
  return err;
}
/*
re__ast_assert_type re__exec_dfa_get_assert_ctx(re_uint32 left_char, re_uint32
right_char) { re__ast_assert_type out = 0; if (left_char == RE__EXEC_SYM_SOT) {
        out |= RE__AST_ASSERT_TYPE_TEXT_START_ABSOLUTE;
        out |= RE__AST_ASSERT_TYPE_TEXT_START;
    }
    if (left_char == '\n') {
        out |= RE__AST_ASSERT_TYPE_TEXT_START;
    }
    if (right_char == '\n') {
        out |= RE__AST_ASSERT_TYPE_TEXT_END;
    }
    if (right_char == RE__EXEC_SYM_EOT) {
        out |= RE__AST_ASSERT_TYPE_TEXT_END_ABSOLUTE;
        out |= RE__AST_ASSERT_TYPE_TEXT_END;
    }
}
*/

re_error re__exec_dfa_start(
    re__exec_dfa* exec, re__prog_entry entry,
    re__exec_dfa_start_state_flags start_state_flags)
{
  re_error err = RE_ERROR_NONE;
  unsigned int start_state_idx =
      start_state_flags + (entry * RE__EXEC_DFA_START_STATE_COUNT);
  re__exec_dfa_state** start_state = &exec->start_states[start_state_idx];
  RE_ASSERT(entry < RE__PROG_ENTRY_MAX);
  if (*start_state == RE_NULL) {
    re__exec_dfa_flags dfa_flags = 0;
    if (start_state_flags & RE__EXEC_DFA_START_STATE_FLAG_AFTER_WORD) {
      dfa_flags |= RE__EXEC_DFA_FLAG_FROM_WORD;
    }
    if (start_state_flags & RE__EXEC_DFA_START_STATE_FLAG_BEGIN_LINE) {
      dfa_flags |= RE__EXEC_DFA_FLAG_BEGIN_LINE;
    }
    if (start_state_flags & RE__EXEC_DFA_START_STATE_FLAG_BEGIN_TEXT) {
      dfa_flags |= RE__EXEC_DFA_FLAG_BEGIN_TEXT;
    }
    if ((err = re__exec_nfa_start(&exec->nfa, entry))) {
      return err;
    }
    if ((err = re__exec_dfa_get_state(exec, dfa_flags, start_state))) {
      return err;
    }
  }
  exec->current_state = exec->start_states[start_state_idx];
  return err;
}

re_error re__exec_dfa_construct(re__exec_dfa* exec, re__exec_sym next_sym)
{
  re_error err = RE_ERROR_NONE;
  re__assert_type assert_ctx = 0;
  unsigned int is_word_boundary;
  re__exec_dfa_flags new_flags = 0;
  re__exec_dfa_state_ptr current_state = exec->current_state;
  re__exec_dfa_state_ptr* next_state = &current_state->next[next_sym];
  is_word_boundary = re__is_word_boundary(
      !!(current_state->flags & RE__EXEC_DFA_FLAG_FROM_WORD), next_sym);
  if (is_word_boundary) {
    assert_ctx |= RE__ASSERT_TYPE_WORD;
  } else {
    assert_ctx |= RE__ASSERT_TYPE_WORD_NOT;
  }
  if (next_sym == RE__EXEC_SYM_EOT) {
    assert_ctx |= RE__ASSERT_TYPE_TEXT_END_ABSOLUTE;
    assert_ctx |= RE__ASSERT_TYPE_TEXT_END;
  }
  if (current_state->flags & RE__EXEC_DFA_FLAG_BEGIN_LINE) {
    assert_ctx |= RE__ASSERT_TYPE_TEXT_START;
  }
  if (current_state->flags & RE__EXEC_DFA_FLAG_BEGIN_TEXT) {
    assert_ctx |= RE__ASSERT_TYPE_TEXT_START_ABSOLUTE;
  }
  re__exec_nfa_set_thrds(
      &exec->nfa, current_state->thrd_locs_begin,
      (re__prog_loc)(current_state->thrd_locs_end - current_state->thrd_locs_begin));
  re__exec_nfa_set_match_index(&exec->nfa, current_state->match_index);
  re__exec_nfa_set_match_priority(
      &exec->nfa, current_state->flags & RE__EXEC_DFA_FLAG_MATCH_PRIORITY);
  if ((err = re__exec_nfa_run_byte(&exec->nfa, assert_ctx, next_sym, 0))) {
    return err;
  }
  if (re__is_word_char(next_sym)) {
    new_flags |= RE__EXEC_DFA_FLAG_FROM_WORD;
  }
  if (next_sym == '\n') {
    assert_ctx |= RE__ASSERT_TYPE_TEXT_END;
    new_flags |= RE__EXEC_DFA_FLAG_BEGIN_LINE;
  }
  if ((err = re__exec_dfa_get_state(exec, new_flags, next_state))) {
    return err;
  }
  exec->current_state = *next_state;
  return err;
}

re_error re__exec_dfa_run_byte(re__exec_dfa* exec, re_uint8 next_byte)
{
  re__exec_dfa_state* current_state = exec->current_state;
  re__exec_dfa_state* next_state;
  re_error err = RE_ERROR_NONE;
  RE_ASSERT(current_state != RE_NULL);
  next_state = current_state->next[next_byte];
  if (next_state == RE_NULL) {
    if ((err = re__exec_dfa_construct(exec, next_byte))) {
      return err;
    }
  } else {
    exec->current_state = next_state;
  }
  return 0;
}

re_error re__exec_dfa_end(re__exec_dfa* exec)
{
  re__exec_dfa_state* current_state = exec->current_state;
  re__exec_dfa_state* next_state;
  re_error err = RE_ERROR_NONE;
  /* for now, ensure it's not null */
  RE_ASSERT(current_state != RE_NULL);
  next_state = current_state->next[RE__EXEC_SYM_EOT];
  if (next_state == RE_NULL) {
    if ((err = re__exec_dfa_construct(exec, RE__EXEC_SYM_EOT))) {
      return err;
    }
  } else {
    exec->current_state = next_state;
  }
  return 0;
}

#define RE__EXEC_DFA_LOOP_DEF(name, body)                                      \
  re_error re__exec_dfa_search_##name(                                         \
      re__exec_dfa* exec, const re_uint8* start, const re_uint8* end,          \
      const re_uint8** out_pos, re_uint32* out_match_index)                    \
  {                                                                            \
    re_error err = 0;                                                          \
    re__exec_dfa_state_ptr current_state = exec->current_state;                \
    re__exec_dfa_state_ptr next_state;                                         \
    body RE__UNUSED(out_pos);                                                  \
    RE__UNUSED(out_match_index);                                               \
    return RE_ERROR_NOMATCH;                                                   \
  }

#define RE__EXEC_DFA_ADVANCE_STATE()                                           \
  next_state = current_state->next[*start];                                    \
  if (next_state == RE_NULL) {                                                 \
    exec->current_state = current_state;                                       \
    if ((err = re__exec_dfa_construct(exec, *start))) {                        \
      return err;                                                              \
    }                                                                          \
    current_state = exec->current_state;                                       \
  } else {                                                                     \
    current_state = next_state;                                                \
  }

#define RE__EXEC_DFA_FWD_LOOP(ex)                                              \
  while (start < end) {                                                        \
    ex start++;                                                                \
  }

#define RE__EXEC_DFA_REV_LOOP(ex)                                              \
  while (start > end) {                                                        \
    start--;                                                                   \
    ex                                                                         \
  }

#define RE__EXEC_DFA_CHECK_MATCH_BOOL_EXIT_EARLY()                             \
  if (re__exec_dfa_state_is_match(current_state)) {                            \
    return RE_MATCH;                                                           \
  }

#define RE__EXEC_DFA_MATCH_POS_DEFS()                                          \
  const re_uint8* last_found_start;                                            \
  re_uint32 last_found_match = 0;

#define RE__EXEC_DFA_CHECK_MATCH_POS()                                         \
  if (re__exec_dfa_state_is_match(current_state)) {                            \
    last_found_match = re__exec_dfa_state_get_match_index(current_state);      \
    last_found_start = start;                                                  \
    if (re__exec_dfa_state_is_priority(current_state)) {                       \
      *out_pos = last_found_start;                                             \
      *out_match_index = last_found_match;                                     \
      return RE_MATCH;                                                         \
    }                                                                          \
  } else if (re__exec_dfa_state_is_empty(current_state) && last_found_match) { \
    *out_pos = last_found_start;                                               \
    *out_match_index = last_found_match;                                       \
    return RE_MATCH;                                                           \
  }

/* Non-boolean match, don't exit early, forwards */
RE__EXEC_DFA_LOOP_DEF(fff, {
  RE__EXEC_DFA_MATCH_POS_DEFS();
  RE__EXEC_DFA_FWD_LOOP({
    RE__EXEC_DFA_ADVANCE_STATE();
    RE__EXEC_DFA_CHECK_MATCH_POS();
  });
})

/* Non-boolean match, don't exit early, reverse */
RE__EXEC_DFA_LOOP_DEF(fft, {
  RE__EXEC_DFA_MATCH_POS_DEFS();
  RE__EXEC_DFA_REV_LOOP({
    RE__EXEC_DFA_ADVANCE_STATE();
    RE__EXEC_DFA_CHECK_MATCH_POS();
  });
})

/* Can't have ft[ft] */
/* RE__EXEC_DFA_LOOP(ftf);
 * RE__EXEC_DFA_LOOP(ftt); */

/* Boolean match, don't bail early, forwards */
RE__EXEC_DFA_LOOP_DEF(tff, {RE__EXEC_DFA_FWD_LOOP({
                        RE__EXEC_DFA_ADVANCE_STATE();
                      })})

/* Boolean match, don't bail early, reverse */
RE__EXEC_DFA_LOOP_DEF(tft, {RE__EXEC_DFA_REV_LOOP({
                        RE__EXEC_DFA_ADVANCE_STATE();
                      })})

/* Boolean match, bail early, forwards */
RE__EXEC_DFA_LOOP_DEF(ttf, {RE__EXEC_DFA_FWD_LOOP({
                        RE__EXEC_DFA_ADVANCE_STATE();
                        RE__EXEC_DFA_CHECK_MATCH_BOOL_EXIT_EARLY();
                      })})

/* Boolean match, bail early, reverse */
RE__EXEC_DFA_LOOP_DEF(ttt, {RE__EXEC_DFA_REV_LOOP({
                        RE__EXEC_DFA_ADVANCE_STATE();
                        RE__EXEC_DFA_CHECK_MATCH_BOOL_EXIT_EARLY();
                      })})

#include <stdio.h>

/* Need to keep track of:
 * - Reversed
 * - Boolean match (check priority bit or not)
 * - Can exit early from boolean matches */
re_error re__exec_dfa_driver(
    re__exec_dfa* exec, re__prog_entry entry, int boolean_match,
    int boolean_match_exit_early, int reversed, const re_uint8* text,
    re_size text_size, re_size text_start_pos, re_uint32* out_match,
    re_size* out_pos)
{
  re__exec_dfa_start_state_flags start_state_flags = 0;
  re_error err = RE_ERROR_NONE;
  if (!reversed) {
    if (text_start_pos == 0) {
      start_state_flags |= RE__EXEC_DFA_START_STATE_FLAG_BEGIN_TEXT |
                           RE__EXEC_DFA_START_STATE_FLAG_BEGIN_LINE;
    } else {
      start_state_flags |=
          (RE__EXEC_DFA_START_STATE_FLAG_AFTER_WORD *
           re__is_word_char((unsigned char)(text[text_start_pos - 1])));
      start_state_flags |= RE__EXEC_DFA_START_STATE_FLAG_BEGIN_LINE *
                           (text[text_start_pos - 1] == '\n');
    }
  } else {
    if (text_start_pos == text_size) {
      start_state_flags |= RE__EXEC_DFA_START_STATE_FLAG_BEGIN_TEXT |
                           RE__EXEC_DFA_START_STATE_FLAG_BEGIN_LINE;
    } else {
      start_state_flags |=
          (RE__EXEC_DFA_START_STATE_FLAG_AFTER_WORD *
           re__is_word_char((unsigned char)(text[text_start_pos])));
      start_state_flags |= RE__EXEC_DFA_START_STATE_FLAG_BEGIN_LINE *
                           (text[text_start_pos] == '\n');
    }
  }
  if ((err = re__exec_dfa_start(exec, entry, start_state_flags))) {
    return err;
  }
  RE_ASSERT(text_start_pos <= text_size);
  RE_ASSERT(RE__IMPLIES(boolean_match, out_match == RE_NULL));
  RE_ASSERT(RE__IMPLIES(boolean_match, out_pos == RE_NULL));
  RE_ASSERT(RE__IMPLIES(!boolean_match, boolean_match_exit_early == 0));
  {
    const re_uint8* start;
    const re_uint8* end;
    const re_uint8* loop_out_pos;
    re_uint32 loop_out_index;
    static re_error (*funcs[8])(
        re__exec_dfa*, const re_uint8*, const re_uint8*, const re_uint8**,
        re_uint32*) = {
        re__exec_dfa_search_fff,
        re__exec_dfa_search_fft,
        RE_NULL,
        RE_NULL,
        re__exec_dfa_search_tff,
        re__exec_dfa_search_tft,
        re__exec_dfa_search_ttf,
        re__exec_dfa_search_ttt};
    start = text + text_start_pos;
    if (!reversed) {
      end = text + text_size;
    } else {
      end = text;
    }
    err = funcs
        [reversed | (boolean_match_exit_early << 1) | (boolean_match << 2)](
            exec, start, end, &loop_out_pos, &loop_out_index);
    if (err == RE_MATCH) {
      /* Exited early */
      if (boolean_match) {
        return err;
      } else {
        RE_ASSERT(loop_out_pos >= text);
        RE_ASSERT(loop_out_pos < text + text_size);
        *out_pos = (re_size)(loop_out_pos - text);
        *out_match = loop_out_index;
        return err;
      }
    } else if (err != RE_ERROR_NOMATCH) {
      /* Trouble's afoot... */
      return err;
    }
    if ((err = re__exec_dfa_end(exec))) {
      return err;
    }
    if (re__exec_dfa_state_is_match(exec->current_state)) {
      if (!boolean_match) {
        *out_pos = (re_size)(end - text);
        *out_match = re__exec_dfa_state_get_match_index(exec->current_state);
      }
      return RE_MATCH;
    } else {
      return RE_NOMATCH;
    }
  }
}

RE_INTERNAL re_uint32 re__exec_dfa_get_match_index(re__exec_dfa* exec)
{
  return exec->current_state->match_index;
}

RE_INTERNAL re_uint32 re__exec_dfa_get_match_priority(re__exec_dfa* exec)
{
  return exec->current_state->flags & RE__EXEC_DFA_FLAG_MATCH_PRIORITY;
}

RE_INTERNAL int re__exec_dfa_get_exhaustion(re__exec_dfa* exec)
{
  return (exec->current_state->thrd_locs_end -
          exec->current_state->thrd_locs_begin) == 0;
}

#if RE_DEBUG

#include <stdio.h>

RE_INTERNAL void re__exec_dfa_debug_dump_state_idx(int sym)
{
  if (sym < RE__EXEC_SYM_EOT) {
    re__byte_debug_dump((re_uint8)sym);
  } else {
    printf("<max>");
  }
}

RE_INTERNAL void re__exec_dfa_debug_dump(re__exec_dfa* exec)
{
  const re__exec_dfa_state* state = exec->current_state;
  printf("---------------------------------\n");
  printf("DFA State Debug Dump (%p, dfa: %p):\n", (void*)state, (void*)exec);
  if (state == RE_NULL) {
    printf("  NULL STATE\n");
    return;
  }
  printf(
      "  Flags: %c%c%c%c%c\n",
      (state->flags & RE__EXEC_DFA_FLAG_FROM_WORD ? 'W' : '-'),
      (state->flags & RE__EXEC_DFA_FLAG_BEGIN_TEXT ? 'A' : '-'),
      (state->flags & RE__EXEC_DFA_FLAG_BEGIN_LINE ? '^' : '-'),
      (state->flags & RE__EXEC_DFA_FLAG_MATCH ? 'M' : '-'),
      (state->flags & RE__EXEC_DFA_FLAG_MATCH_PRIORITY ? 'P' : '-'));
  printf("  Match Index: %i\n", state->match_index);
  printf(
      "  Match Priority: %i\n",
      state->flags & RE__EXEC_DFA_FLAG_MATCH_PRIORITY);
  printf(
      "  Threads: %u\n    ",
      (unsigned int)(state->thrd_locs_end - state->thrd_locs_begin));
  {
    re_uint32* thrd_ptr = state->thrd_locs_begin;
    while (thrd_ptr != state->thrd_locs_end) {
      printf("%04X ", *thrd_ptr);
      thrd_ptr++;
    }
    printf("\n");
  }
  printf("  Nexts:\n");
  {
    int sym;
    for (sym = 0; sym < RE__EXEC_SYM_MAX; sym++) {
      re__exec_dfa_state* next = state->next[sym];
      if (next != RE_NULL) {
        printf("    [");
        re__exec_dfa_debug_dump_state_idx(sym);
        printf("]: %p\n", (void*)next);
      }
    }
  }
  printf("---------------------------------\n");
}

#endif

/* note:
 * start state is taken on re__exec_dfa_start.
 * then, when ch is taken, call re__exec_nfa_start with appropriate params.
 * this accomodates the only zero-width match (the empty string.)
 * DFA match is delayed by one byte. */

/* re */
#define RE__EXEC_SAVE_REF_NONE -1

RE__VEC_IMPL_FUNC(re_size, init)
RE__VEC_IMPL_FUNC(re_size, destroy)
RE__VEC_IMPL_FUNC(re_size, clear)
RE__VEC_IMPL_FUNC(re_size, size)
RE__VEC_IMPL_FUNC(re_size, reserve)
RE__VEC_IMPL_FUNC(re_size, push)
RE__VEC_IMPL_FUNC(re_size, getref)
RE__VEC_IMPL_FUNC(re_size, getcref)

RE_INTERNAL void re__exec_save_init(re__exec_save* save)
{
  re_size_vec_init(&save->slots);
  save->last_empty_ref = RE__EXEC_SAVE_REF_NONE;
  save->slots_per_thrd = 1; /* +1 for ref count */
}

RE_INTERNAL void re__exec_save_start(re__exec_save* save)
{
  re_size_vec_clear(&save->slots);
}

RE_INTERNAL re_uint32 re__exec_save_get_slots_per_thrd(re__exec_save* save)
{
  return save->slots_per_thrd - 1;
}

RE_INTERNAL void
re__exec_save_set_slots_per_thrd(re__exec_save* save, re_uint32 slots_per_thrd)
{
  save->slots_per_thrd = slots_per_thrd + 1;
}

RE_INTERNAL void re__exec_save_destroy(re__exec_save* save)
{
  re_size_vec_destroy(&save->slots);
}

RE_INTERNAL const re_size*
re__exec_save_get_slots_const(const re__exec_save* save, re_int32 slots_ref)
{
  RE_ASSERT(slots_ref != RE__EXEC_SAVE_REF_NONE);
  return re_size_vec_getcref(&save->slots, (re_size)slots_ref);
}

RE_INTERNAL re_size*
re__exec_save_get_slots(re__exec_save* save, re_int32 slots_ref)
{
  RE_ASSERT(slots_ref != RE__EXEC_SAVE_REF_NONE);
  return re_size_vec_getref(&save->slots, (re_size)slots_ref);
}

RE_INTERNAL void re__exec_save_inc_refs(re__exec_save* save, re_int32 slots_ref)
{
  re_size* slots = re__exec_save_get_slots(save, slots_ref);
  slots[save->slots_per_thrd - 1] += 1;
}

RE_INTERNAL void re__exec_save_dec_refs(re__exec_save* save, re_int32 slots_ref)
{
  re_size* slots;
  if (slots_ref == -1) {
    return;
  }
  slots = re__exec_save_get_slots(save, slots_ref);
  if (--slots[save->slots_per_thrd - 1] == 0) {
    re_int32* next_empty_save =
        ((re_int32*)(slots + (save->slots_per_thrd - 1)));
    save->last_empty_ref = slots_ref;
    /* hacky */
    *next_empty_save = save->last_empty_ref;
  }
}

RE_INTERNAL re_size
re__exec_save_get_refs(const re__exec_save* save, re_int32 slots_ref)
{
  const re_size* slots = re__exec_save_get_slots_const(save, slots_ref);
  return slots[save->slots_per_thrd - 1];
}

RE_INTERNAL re_error
re__exec_save_get_new(re__exec_save* save, re_int32* slots_out_ref)
{
  re_size* slots;
  re_error err = RE_ERROR_NONE;
  if (save->last_empty_ref != RE__EXEC_SAVE_REF_NONE) {
    *slots_out_ref = save->last_empty_ref;
    slots = re__exec_save_get_slots(save, save->last_empty_ref);
    save->last_empty_ref = *((re_int32*)(slots + (save->slots_per_thrd - 1)));
    re__exec_save_inc_refs(save, *slots_out_ref);
    return err;
  } else {
    /* need realloc */
    re_uint32 i;
    *slots_out_ref = (re_int32)re_size_vec_size(&save->slots);
    if ((err = re_size_vec_reserve(
             &save->slots,
             re_size_vec_size(&save->slots) + save->slots_per_thrd))) {
      return err;
    }
    for (i = 0; i < save->slots_per_thrd; i++) {
      if ((err = re_size_vec_push(&save->slots, 0))) {
        return err;
      }
    }
    re__exec_save_inc_refs(save, *slots_out_ref);
    return err;
  }
}

RE_INTERNAL re_error re__exec_save_do_save(
    re__exec_save* save, re_int32* slots_inout_ref, re_uint32 slot_number,
    re_size data)
{
  re_error err = RE_ERROR_NONE;
  re_size* slots;
  if (*slots_inout_ref == RE__EXEC_SAVE_REF_NONE) {
    if ((err = re__exec_save_get_new(save, slots_inout_ref))) {
      return err;
    }
    slots = re__exec_save_get_slots(save, *slots_inout_ref);
  } else if (re__exec_save_get_refs(save, *slots_inout_ref) > 1) {
    re_int32 old_slots_ref = *slots_inout_ref;
    re_uint32 i;
    re_size* old_slots;
    re__exec_save_dec_refs(save, old_slots_ref);
    if ((err = re__exec_save_get_new(save, slots_inout_ref))) {
      return err;
    }
    slots = re__exec_save_get_slots(save, *slots_inout_ref);
    old_slots = re__exec_save_get_slots(save, old_slots_ref);
    /* copy over */
    for (i = 0; i < save->slots_per_thrd - 1; i++) {
      slots[i] = old_slots[i];
    }
  } else {
    slots = re__exec_save_get_slots(save, *slots_inout_ref);
  }
  RE_ASSERT(slot_number < save->slots_per_thrd - 1);
  slots[slot_number] = data;
  return err;
}

RE_INTERNAL void
re__exec_thrd_init(re__exec_thrd* thrd, re__prog_loc loc, re_int32 save_slot)
{
  thrd->loc = loc;
  thrd->save_slot = save_slot;
}

RE_INTERNAL void re__exec_thrd_set_init(re__exec_thrd_set* set)
{
  set->size = 0;
  set->n = 0;
  set->dense = RE_NULL;
  set->sparse = RE_NULL;
  set->match_index = 0;
  set->match_priority = 0;
}

RE_INTERNAL void re__exec_thrd_set_destroy(re__exec_thrd_set* set)
{
  if (set->dense) {
    RE_FREE(set->dense);
    set->dense = RE_NULL;
  }
  if (set->sparse) {
    RE_FREE(set->sparse);
    set->sparse = RE_NULL;
  }
}

RE_INTERNAL re_error
re__exec_thrd_set_alloc(re__exec_thrd_set* set, re__prog_loc size)
{
  re_error err = RE_ERROR_NONE;
  re__exec_thrd* new_dense = RE_NULL;
  re__prog_loc* new_sparse = RE_NULL;
  set->size = size;
  if (!set->dense) {
    new_dense = (re__exec_thrd*)RE_MALLOC(sizeof(re__exec_thrd) * size);
  } else {
    new_dense =
        (re__exec_thrd*)RE_REALLOC(set->dense, sizeof(re__exec_thrd) * size);
  }
  if (new_dense == RE_NULL) {
    return RE_ERROR_NOMEM;
  }
  set->dense = new_dense;
  if (!set->sparse) {
    new_sparse = (re__prog_loc*)RE_MALLOC(sizeof(re__prog_loc) * size);
  } else {
    new_sparse =
        (re__prog_loc*)RE_REALLOC(set->sparse, sizeof(re__prog_loc) * size);
  }
  if (new_sparse == RE_NULL) {
    return RE_ERROR_NOMEM;
  }
  set->sparse = new_sparse;
  return err;
}

RE_INTERNAL void
re__exec_thrd_set_add(re__exec_thrd_set* set, re__exec_thrd thrd)
{
  RE_ASSERT(thrd.loc < set->size);
  RE_ASSERT(set->n < set->size);
  set->dense[set->n] = thrd;
  set->sparse[thrd.loc] = set->n;
  set->n++;
}

RE_INTERNAL void re__exec_thrd_set_clear(re__exec_thrd_set* set)
{
  set->n = 0;
  set->match_index = 0;
  set->match_priority = 0;
}

RE_INTERNAL int
re__exec_thrd_set_ismemb(re__exec_thrd_set* set, re__exec_thrd thrd)
{
  RE_ASSERT(thrd.loc < set->size);
  return set->sparse[thrd.loc] < set->n &&
         set->dense[set->sparse[thrd.loc]].loc == thrd.loc;
}

#if RE_DEBUG

#include <stdio.h>

RE_INTERNAL void
re__exec_nfa_debug_dump(const re__exec_nfa* exec, int with_save)
{
  printf("n: %u\n", exec->set_a.n);
  printf("s: %u\n", exec->set_a.size);
  printf("match: %u\n", exec->set_a.match_index);
  printf("match_priority: %u\n", exec->set_a.match_priority);
  printf("memb:\n");
  {
    re_uint32 i;
    for (i = 0; i < exec->set_a.n; i++) {
      printf(
          "  %04X, %i\n", exec->set_a.dense[i].loc,
          exec->set_a.dense[i].save_slot);
    }
  }
  if (with_save) {
    printf("slots:\n");
    {
      re_uint32 i;
      for (i = 0; i < exec->set_a.n; i++) {
        re_int32 slot_ref = exec->set_a.dense[i].save_slot;
        const re_size* slots;
        re_uint32 j;
        if (slot_ref == RE__EXEC_SAVE_REF_NONE) {
          continue;
        }
        slots = re__exec_save_get_slots_const(&exec->save_slots, slot_ref);
        printf(
            "%i: %u\n", slot_ref,
            (re_uint32)re__exec_save_get_refs(&exec->save_slots, slot_ref));
        for (j = 0; j < exec->save_slots.slots_per_thrd - 1; j++) {
          printf("  %i: %u\n", j, (re_uint32)slots[j]);
        }
      }
    }
  }
}

#endif

RE__VEC_IMPL_FUNC(re__exec_thrd, init)
RE__VEC_IMPL_FUNC(re__exec_thrd, destroy)
RE__VEC_IMPL_FUNC(re__exec_thrd, push)
RE__VEC_IMPL_FUNC(re__exec_thrd, pop)
RE__VEC_IMPL_FUNC(re__exec_thrd, clear)
RE__VEC_IMPL_FUNC(re__exec_thrd, size)

RE_INTERNAL void re__exec_nfa_init(
    re__exec_nfa* exec, const re__prog* prog, re_uint32 num_groups)
{
  exec->prog = prog;
  exec->num_groups = num_groups;
  re__exec_thrd_set_init(&exec->set_a);
  re__exec_thrd_set_init(&exec->set_b);
  re__exec_thrd_set_init(&exec->set_c);
  re__exec_thrd_vec_init(&exec->thrd_stk);
  re__exec_save_init(&exec->save_slots);
}

RE_INTERNAL void re__exec_nfa_destroy(re__exec_nfa* exec)
{
  re__exec_save_destroy(&exec->save_slots);
  re__exec_thrd_vec_destroy(&exec->thrd_stk);
  re__exec_thrd_set_destroy(&exec->set_c);
  re__exec_thrd_set_destroy(&exec->set_b);
  re__exec_thrd_set_destroy(&exec->set_a);
}

RE_INTERNAL re__prog_loc re__exec_nfa_get_thrds_size(re__exec_nfa* exec)
{
  return exec->set_a.n;
}

RE_INTERNAL const re__exec_thrd* re__exec_nfa_get_thrds(re__exec_nfa* exec)
{
  return exec->set_a.dense;
}

RE_INTERNAL void re__exec_nfa_set_thrds(
    re__exec_nfa* exec, const re__prog_loc* in_thrds,
    re__prog_loc in_thrds_size)
{
  re__prog_loc i;
  re__exec_thrd_set_clear(&exec->set_a);
  re__exec_thrd_set_clear(&exec->set_b);
  for (i = 0; i < in_thrds_size; i++) {
    re__exec_thrd new_thrd;
    re__exec_thrd_init(&new_thrd, in_thrds[i], RE__EXEC_SAVE_REF_NONE);
    re__exec_thrd_set_add(&exec->set_a, new_thrd);
  }
}

RE_INTERNAL re_uint32 re__exec_nfa_get_match_index(re__exec_nfa* exec)
{
  return exec->set_a.match_index;
}

RE_INTERNAL re_uint32 re__exec_nfa_get_match_priority(re__exec_nfa* exec)
{
  return exec->set_a.match_priority;
}

RE_INTERNAL void
re__exec_nfa_set_match_index(re__exec_nfa* exec, re_uint32 match_index)
{
  exec->set_a.match_index = match_index;
}

RE_INTERNAL void
re__exec_nfa_set_match_priority(re__exec_nfa* exec, re_uint32 match_priority)
{
  exec->set_a.match_priority = match_priority;
}

RE_INTERNAL void re__exec_nfa_swap(re__exec_nfa* exec)
{
  re__exec_thrd_set temp = exec->set_a;
  exec->set_a = exec->set_b;
  exec->set_b = temp;
}

RE_INTERNAL re_error
re__exec_nfa_start(re__exec_nfa* exec, re__prog_entry entry)
{
  re_error err = RE_ERROR_NONE;
  re__prog_loc set_size = re__prog_size(exec->prog);
  re__exec_thrd initial;
  re__exec_save_start(&exec->save_slots);
  if (exec->num_groups < 1) {
    re__exec_save_set_slots_per_thrd(&exec->save_slots, 0);
  } else {
    /* cast is ok since exec->num_groups guaranteed > 1 */
    re__exec_save_set_slots_per_thrd(
        &exec->save_slots, (re_uint32)(exec->num_groups * 2));
  }
  if ((err = re__exec_thrd_set_alloc(&exec->set_a, set_size))) {
    return err;
  }
  if ((err = re__exec_thrd_set_alloc(&exec->set_b, set_size))) {
    return err;
  }
  if ((err = re__exec_thrd_set_alloc(&exec->set_c, set_size))) {
    return err;
  }
  re__exec_thrd_init(&initial, re__prog_get_entry(exec->prog, entry), -1);
  re__exec_thrd_set_clear(&exec->set_a);
  re__exec_thrd_set_clear(&exec->set_b);
  re__exec_thrd_set_clear(&exec->set_c);
  re__exec_thrd_set_add(&exec->set_a, initial);
  return err;
}

RE_INTERNAL re_error re__exec_nfa_run_follow(
    re__exec_nfa* exec, re__assert_type assert_type, re_size pos)
{
  re_error err = RE_ERROR_NONE;
  re__prog_loc i;
  re__exec_thrd_vec_clear(&exec->thrd_stk);
  re__exec_thrd_set_clear(&exec->set_b);
  for (i = 0; i < exec->set_a.n; i++) {
    re__exec_thrd current_outer = exec->set_a.dense[i];
    if ((err = re__exec_thrd_vec_push(&exec->thrd_stk, current_outer))) {
      return err;
    }
    re__exec_thrd_set_clear(&exec->set_c);
    while (re__exec_thrd_vec_size(&exec->thrd_stk)) {
      re__exec_thrd top = re__exec_thrd_vec_pop(&exec->thrd_stk);
      const re__prog_inst* inst = re__prog_get_const(exec->prog, top.loc);
      re__prog_inst_type inst_type;
      RE_ASSERT(top.loc != 0);
      if (re__exec_thrd_set_ismemb(&exec->set_c, top)) {
        /* target is already a member */
        continue;
      } else {
        re__exec_thrd_set_add(&exec->set_c, top);
      }
      inst_type = re__prog_inst_get_type(inst);
      if (inst_type == RE__PROG_INST_TYPE_BYTE) {
        re__exec_thrd_set_add(&exec->set_b, top);
      } else if (inst_type == RE__PROG_INST_TYPE_BYTE_RANGE) {
        re__exec_thrd_set_add(&exec->set_b, top);
      } else if (inst_type == RE__PROG_INST_TYPE_SPLIT) {
        /* push secondary first to maintain priority */
        re__exec_thrd secondary_thrd, primary_thrd;
        re__exec_thrd_init(
            &secondary_thrd, re__prog_inst_get_split_secondary(inst),
            top.save_slot);
        re__exec_thrd_init(
            &primary_thrd, re__prog_inst_get_primary(inst), top.save_slot);
        if (top.save_slot != RE__EXEC_SAVE_REF_NONE) {
          re__exec_save_inc_refs(&exec->save_slots, top.save_slot);
        }
        if ((err = re__exec_thrd_vec_push(&exec->thrd_stk, secondary_thrd))) {
          return err;
        }
        if ((err = re__exec_thrd_vec_push(&exec->thrd_stk, primary_thrd))) {
          return err;
        }
      } else if (inst_type == RE__PROG_INST_TYPE_MATCH) {
        re__exec_thrd_set_add(&exec->set_b, top);
      } else if (inst_type == RE__PROG_INST_TYPE_SAVE) {
        re__exec_thrd primary_thrd;
        re_uint32 save_idx = re__prog_inst_get_save_idx(inst);
        re__exec_thrd_init(
            &primary_thrd, re__prog_inst_get_primary(inst), top.save_slot);
        if (save_idx < re__exec_save_get_slots_per_thrd(&exec->save_slots)) {
          if ((err = re__exec_save_do_save(
                   &exec->save_slots, &primary_thrd.save_slot,
                   re__prog_inst_get_save_idx(inst), pos))) {
            return err;
          }
        }
        if ((err = re__exec_thrd_vec_push(&exec->thrd_stk, primary_thrd))) {
          return err;
        }
      } else if (inst_type == RE__PROG_INST_TYPE_ASSERT) {
        re__exec_thrd primary_thrd;
        re__assert_type required_ctx = re__prog_inst_get_assert_ctx(inst);
        re__exec_thrd_init(
            &primary_thrd, re__prog_inst_get_primary(inst), top.save_slot);
        if ((required_ctx & assert_type) == required_ctx) {
          if ((err = re__exec_thrd_vec_push(&exec->thrd_stk, primary_thrd))) {
            return err;
          }
        } else {
          re__exec_save_dec_refs(&exec->save_slots, primary_thrd.save_slot);
        }
      } else {
        RE__ASSERT_UNREACHED();
      }
    }
  }
  re__exec_thrd_set_clear(&exec->set_a);
  return err;
}

RE_INTERNAL re_error re__exec_nfa_run_byte(
    re__exec_nfa* exec, re__assert_type assert_type, unsigned int ch,
    re_size pos)
{
  re_error err = RE_ERROR_NONE;
  re__prog_loc i;
  if ((err = re__exec_nfa_run_follow(exec, assert_type, pos))) {
    return err;
  }
  for (i = 0; i < exec->set_b.n; i++) {
    re__exec_thrd cur_thrd = exec->set_b.dense[i];
    const re__prog_inst* cur_inst =
        re__prog_get_const(exec->prog, cur_thrd.loc);
    re__prog_inst_type cur_inst_type = re__prog_inst_get_type(cur_inst);
    if (cur_inst_type == RE__PROG_INST_TYPE_BYTE) {
      if (ch == re__prog_inst_get_byte(cur_inst)) {
        re__exec_thrd primary_thrd;
        re__exec_thrd_init(
            &primary_thrd, re__prog_inst_get_primary(cur_inst),
            cur_thrd.save_slot);
        re__exec_thrd_set_add(&exec->set_a, primary_thrd);
      } else {
        re__exec_save_dec_refs(&exec->save_slots, cur_thrd.save_slot);
      }
    } else if (cur_inst_type == RE__PROG_INST_TYPE_BYTE_RANGE) {
      if (ch >= re__prog_inst_get_byte_min(cur_inst) &&
          ch <= re__prog_inst_get_byte_max(cur_inst)) {
        re__exec_thrd primary_thrd;
        re__exec_thrd_init(
            &primary_thrd, re__prog_inst_get_primary(cur_inst),
            cur_thrd.save_slot);
        re__exec_thrd_set_add(&exec->set_a, primary_thrd);
      } else {
        re__exec_save_dec_refs(&exec->save_slots, cur_thrd.save_slot);
      }
    } else if (cur_inst_type == RE__PROG_INST_TYPE_MATCH) {
      re_uint32 match_index = re__prog_inst_get_match_idx(cur_inst);
      if (!exec->set_a.match_index) {
        exec->set_a.match_index = match_index;
        exec->set_a.match_priority = exec->set_a.n;
      }
      re__exec_save_dec_refs(&exec->save_slots, cur_thrd.save_slot);
    } else {
      RE__ASSERT_UNREACHED();
    }
  }
  return err;
}

RE_INTERNAL re_error
re__exec_nfa_finish(re__exec_nfa* exec, re_span* out, re_size pos)
{
  /* check if there are any threads left */
  if (exec->set_a.n) {
    /* extract matches from the top thread */
    re__exec_thrd top_thrd = exec->set_a.dense[0];
    /* check if the top thread has save slots */
    if (top_thrd.save_slot != RE__EXEC_SAVE_REF_NONE) {
      re_size* slots =
          re__exec_save_get_slots(&exec->save_slots, top_thrd.save_slot);
      re_span out_span;
      re_uint32 i;
      /* Set first span (match boundaries) */
      out_span.begin = 0;
      out_span.end = pos;
      *(out++) = out_span;
      /* Write all other groups */
      for (i = 0; i < exec->num_groups; i++) {
        out_span.begin = slots[i * 2];
        out_span.end = slots[i * 2 + 1];
        *(out++) = out_span;
      }
      return RE_MATCH;
    } else {
      RE_ASSERT(exec->num_groups == 0);
      return RE_MATCH;
    }
  } else {
    return RE_NOMATCH;
  }
}

RE_INTERNAL unsigned int re__is_word_char(re__exec_sym ch)
{
  return ((ch >= 'a') && (ch <= 'z')) || ((ch >= 'A') && (ch <= 'Z')) ||
         ((ch >= '0') && (ch <= '9')) || ch == '_';
}

RE_INTERNAL unsigned int re__is_word_boundary_start(re__exec_sym right)
{
  if (re__is_word_char(right)) {
    return 1;
  } else if (right == RE__EXEC_SYM_EOT) {
    return 0;
  } else {
    return 0;
  }
}

RE_INTERNAL unsigned int
re__is_word_boundary(int left_is_word, re__exec_sym right)
{
  if (left_is_word) {
    return (!re__is_word_char(right)) || right == RE__EXEC_SYM_EOT;
  } else {
    return re__is_word_char(right);
  }
}

/* re */
void re__parse_frame_init(
    re__parse_frame* frame, re_int32 ast_root_ref, re__parse_flags flags)
{
  frame->ast_root_ref = ast_root_ref;
  frame->ast_prev_child_ref = RE__AST_NONE;
  frame->flags = flags;
}

RE__VEC_IMPL_FUNC(re__parse_frame, init)
RE__VEC_IMPL_FUNC(re__parse_frame, destroy)
RE__VEC_IMPL_FUNC(re__parse_frame, size)
RE__VEC_IMPL_FUNC(re__parse_frame, getref)
RE__VEC_IMPL_FUNC(re__parse_frame, push)
RE__VEC_IMPL_FUNC(re__parse_frame, pop)

void re__parse_init(re__parse* parse, re* reg)
{
  parse->reg = reg;
  re__str_view_init_null(&parse->str);
  parse->str_pos = 0;
  re__parse_frame_vec_init(&parse->frames);
  re__charclass_builder_init(&parse->charclass_builder, &reg->data->rune_data);
}

void re__parse_destroy(re__parse* parse)
{
  re__parse_frame_vec_destroy(&parse->frames);
  re__charclass_builder_destroy(&parse->charclass_builder);
}

re__parse_frame* re__parse_get_frame(re__parse* parse)
{
  RE_ASSERT(re__parse_frame_vec_size(&parse->frames));
  return re__parse_frame_vec_getref(
      &parse->frames, re__parse_frame_vec_size(&parse->frames) - 1);
}

re__ast_type re__parse_get_frame_type(re__parse* parse)
{
  re__parse_frame* frame = re__parse_get_frame(parse);
  re__ast_type ret = RE__AST_TYPE_NONE;
  if (re__parse_frame_vec_size(&parse->frames) != 1) {
    ret = re__ast_root_get(&parse->reg->data->ast_root, frame->ast_root_ref)
              ->type;
  }
  RE_ASSERT(
      ret == RE__AST_TYPE_GROUP || ret == RE__AST_TYPE_CONCAT ||
      ret == RE__AST_TYPE_ALT || ret == RE__AST_TYPE_NONE);
  return ret;
}

re_error re__parse_push_frame(
    re__parse* parse, re_int32 ast_root_ref, re__parse_flags flags)
{
  re__parse_frame new_frame;
  re__parse_frame_init(&new_frame, ast_root_ref, flags);
  return re__parse_frame_vec_push(&parse->frames, new_frame);
}

void re__parse_pop_frame(re__parse* parse)
{
  re__parse_frame_vec_pop(&parse->frames);
}

int re__parse_frame_is_empty(re__parse* parse)
{
  return re__parse_get_frame(parse)->ast_prev_child_ref == RE__AST_NONE;
}

/* Convenience function to set the error to a literal string. */
RE_INTERNAL re_error re__parse_error(re__parse* parse, const char* err_chars)
{
  re__str err_str;
  re_error err = re__str_init_s(&err_str, (const re_char*)err_chars);
  if (err) {
    re__str_destroy(&err_str);
    return err;
  }
  if ((err = re__set_error_str(parse->reg, &err_str))) {
    re__str_destroy(&err_str);
    return err;
  }
  return RE_ERROR_PARSE;
}

#define RE__PARSE_UTF8_ACCEPT 0
#define RE__PARSE_UTF8_REJECT 12

static const re_uint8 re__parse_utf8_tt[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  9,  9,  9,  9,  9,  9,  9,  9,
    9,  9,  9,  9,  9,  9,  9,  9,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,
    7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,
    7,  7,  8,  8,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  10, 3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  4,  3,  3,  11, 6,  6,  6,  5,  8,  8,
    8,  8,  8,  8,  8,  8,  8,  8,  8,

    0,  12, 24, 36, 60, 96, 84, 12, 12, 12, 48, 72, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 0,  12, 12, 12, 12, 12, 0,  12, 0,  12, 12, 12, 24,
    12, 12, 12, 12, 12, 24, 12, 24, 12, 12, 12, 12, 12, 12, 12, 12, 12, 24, 12,
    12, 12, 12, 12, 24, 12, 12, 12, 12, 12, 12, 12, 24, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 36, 12, 36, 12, 12, 12, 36, 12, 12, 12, 12, 12, 36, 12, 36, 12,
    12, 12, 36, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
};

RE_INTERNAL re_uint32
re__parse_utf8_decode(re_uint32* state, re_uint32* codep, re_uint32 byte)
{
  re_uint32 type = re__parse_utf8_tt[byte];

  *codep =
      (*state != 0) ? (byte & 0x3fu) | (*codep << 6) : (0xff >> type) & (byte);

  *state = re__parse_utf8_tt[256 + *state + type];
  return *state;
}

#define RE__PARSE_EOF (-1)

re_error re__parse_next_char(re__parse* parse, re_rune* ch)
{
  re_uint32 codep = 0;
  re_uint32 state = 0;
  while (1) {
    RE_ASSERT(parse->str_pos <= re__str_view_size(&parse->str));
    if (parse->str_pos == re__str_view_size(&parse->str)) {
      if (state == RE__PARSE_UTF8_ACCEPT) {
        parse->str_pos++;
        *ch = RE__PARSE_EOF;
        return RE_ERROR_NONE;
      } else {
        parse->str_pos++;
        return re__parse_error(parse, "invalid UTF-8 byte");
      }
    } else {
      re_uint8 in_byte =
          (re_uint8)(re__str_view_get_data(&parse->str)[parse->str_pos]);
      if (!re__parse_utf8_decode(&state, &codep, in_byte)) {
        parse->str_pos++;
        *ch = (re_rune)codep;
        return RE_ERROR_NONE;
      } else if (state == RE__PARSE_UTF8_REJECT) {
        parse->str_pos++;
        return re__parse_error(parse, "invalid UTF-8 byte");
      } else {
        parse->str_pos++;
      }
    }
  }
}

/* Add a node after the previous child. */
RE_INTERNAL re_error re__parse_push_node(re__parse* parse, re__ast ast)
{
  re_error err = RE_ERROR_NONE;
  re__parse_frame* frame = re__parse_get_frame(parse);
  re_int32 new_ref = RE__AST_NONE;
  if ((err = re__ast_root_add_child(
           &parse->reg->data->ast_root, frame->ast_root_ref, ast, &new_ref))) {
    return err;
  }
  frame->ast_prev_child_ref = new_ref;
  return err;
}

/* Insert a node right before the previous child, making the previous child the
 * new node's parent. */
RE_INTERNAL re_error re__parse_wrap_node(re__parse* parse, re__ast outer)
{
  re_error err = RE_ERROR_NONE;
  re__parse_frame* frame = re__parse_get_frame(parse);
  re_int32 new_outer_ref = RE__AST_NONE;
  if ((err = re__ast_root_add_wrap(
           &parse->reg->data->ast_root, frame->ast_root_ref,
           frame->ast_prev_child_ref, outer, &new_outer_ref))) {
    return err;
  }
  frame->ast_prev_child_ref = new_outer_ref;
  return err;
}

RE_INTERNAL re_error
re__parse_opt_fuse_concat(re__parse* parse, re__ast* next, int* did_fuse)
{
  re__ast* prev;
  re_int32 prev_child_ref;
  re__ast_type t_prev, t_next;
  re_error err = RE_ERROR_NONE;
  RE_ASSERT(!re__parse_frame_is_empty(parse));
  prev_child_ref = re__parse_get_frame(parse)->ast_prev_child_ref;
  prev = re__ast_root_get(&parse->reg->data->ast_root, prev_child_ref);
  t_prev = prev->type;
  t_next = next->type;
  *did_fuse = 0;
  if (t_next == RE__AST_TYPE_RUNE) {
    if (t_prev == RE__AST_TYPE_STR || t_prev == RE__AST_TYPE_RUNE) {
      re_int32 str_ref;
      re_char rune_bytes[16]; /* 16 oughta be good */
      int rune_bytes_ptr = 0;
      re__str* out_str;
      if (t_prev == RE__AST_TYPE_RUNE) {
        re__str new_str;
        re__ast new_ast;
        re__str_init(&new_str);
        if ((err = re__ast_root_add_str(
                 &parse->reg->data->ast_root, new_str, &str_ref))) {
          return err;
        }
        rune_bytes_ptr += re__compile_gen_utf8(
            re__ast_get_rune(prev), (re_uint8*)rune_bytes + rune_bytes_ptr);
        re__ast_init_str(&new_ast, str_ref);
        re__ast_root_replace(
            &parse->reg->data->ast_root, prev_child_ref, new_ast);
      } else {
        str_ref = re__ast_get_str_ref(prev);
      }
      rune_bytes_ptr += re__compile_gen_utf8(
          re__ast_get_rune(next), (re_uint8*)rune_bytes + rune_bytes_ptr);
      out_str = re__ast_root_get_str(&parse->reg->data->ast_root, str_ref);
      if ((err = re__str_cat_n(out_str, rune_bytes, (re_size)rune_bytes_ptr))) {
        return err;
      }
      re__ast_destroy(next);
      *did_fuse = 1;
    }
  }
  return err;
}

/* Add a new node to the end of the stack, while maintaining these invariants:
 * - Group nodes can only hold one immediate node.
 * - Alt nodes can only hold one immediate node per branch.
 * - Concat nodes can hold an infinite number of nodes.
 *
 * To maintain these, when we have to add a second child to an alt/group node,
 * we convert it into a concatenation of the first and second children. */
RE_INTERNAL re_error re__parse_link_node(re__parse* parse, re__ast new_ast)
{
  re__ast_type frame_type = RE__AST_TYPE_NONE;
  re_error err = RE_ERROR_NONE;
  /* Firstly, attempt an optimization by fusing the nodes, if possible. */
  if (!re__parse_frame_is_empty(parse)) {
    int did_fuse;
    if ((err = re__parse_opt_fuse_concat(parse, &new_ast, &did_fuse))) {
      return err;
    }
    if (did_fuse) {
      /* We successfully fused the node, so there is no need to create
       * a new concatenation. */
      return err;
    }
  }
  /* If we are here, then the node couldn't be optimized away and we have to
   * push it. */
  frame_type = re__parse_get_frame_type(parse);
  /* Weird control flow -- it's the only way I figured out how to do the
   * assertion below. */
  if ((frame_type == RE__AST_TYPE_GROUP || frame_type == RE__AST_TYPE_ALT ||
       frame_type == RE__AST_TYPE_NONE) &&
      !re__parse_frame_is_empty(parse)) {
    re__ast new_concat;
    /* Wrap the last child(ren) in a concatenation */
    re__ast_init_concat(&new_concat);
    if ((err = re__parse_wrap_node(parse, new_concat))) {
      return err;
    }
    /* new_concat is moved */
    /* Push a new frame */
    if ((err = re__parse_push_frame(
             parse, re__parse_get_frame(parse)->ast_prev_child_ref,
             re__parse_get_frame(parse)->flags))) {
      return err;
    }
  }
  /* Add the new node to the frame. */
  if ((err = re__parse_push_node(parse, new_ast))) {
    return err;
  }
  return err;
}

/* Called after '(' */
re_error re__parse_group_begin(re__parse* parse)
{
  re_error err = RE_ERROR_NONE;
  re_rune ch;
  re_size begin_name_pos = 0;
  re_size end_name_pos = 0;
  re__parse_flags flags = re__parse_get_frame(parse)->flags;
  re_size saved_pos = parse->str_pos;
  re__ast_group_flags group_flags = 0;
  if ((err = re__parse_next_char(parse, &ch))) {
    return err;
  }
  if (ch == RE__PARSE_EOF) {
    return re__parse_error(parse, "unmatched '('");
  } else if (ch == '?') {
    int set_bit = 1;
    /* (? | Start of group flags/name */
    while (1) {
      if ((err = re__parse_next_char(parse, &ch))) {
        return err;
      }
      if (ch == ')') {
        /* (?) | Go back to ground without creating a group, retain flags */
        re__parse_get_frame(parse)->flags = flags;
        return err;
      } else if (ch == '-') {
        /* (?- | Negate remaining flags */
        set_bit = 0;
      } else if (ch == ':') {
        /* (?: | Non-matching group, also signals end of flags */
        group_flags |= RE__AST_GROUP_FLAG_NONMATCHING;
        break;
      } else if (ch == 'U') {
        /* (?U | Ungreedy mode: *+? operators have priority swapped */
        if (set_bit) {
          flags |= RE__PARSE_FLAG_UNGREEDY;
        } else {
          flags &= ~(unsigned int)RE__PARSE_FLAG_UNGREEDY;
        }
      } else if (ch == 'i') {
        /* (?i: Case insensitive matching */
        if (set_bit) {
          flags |= RE__PARSE_FLAG_CASE_INSENSITIVE;
        } else {
          flags &= ~(unsigned int)RE__PARSE_FLAG_CASE_INSENSITIVE;
        }
      } else if (ch == 'm') {
        /* (?m: Multi-line mode: ^$ match line boundaries */
        if (set_bit) {
          flags |= RE__PARSE_FLAG_MULTILINE;
        } else {
          flags &= ~(unsigned int)RE__PARSE_FLAG_MULTILINE;
        }
      } else if (ch == 's') {
        /* (?s: Stream (?) mode: . matches \n */
        if (set_bit) {
          flags |= RE__PARSE_FLAG_DOT_NEWLINE;
        } else {
          flags &= ~(unsigned int)RE__PARSE_FLAG_DOT_NEWLINE;
        }
      } else if (ch == '<' || ch == 'P') {
        re_size saved_name_pos = 0;
        if (ch == 'P') {
          /* (?P | Alternative way to start group name */
          if ((err = re__parse_next_char(parse, &ch))) {
            return err;
          }
          if (ch != '<') {
            /* Handles EOF */
            return re__parse_error(parse, "expected '<' to begin group name");
          }
        }
        /* (?< | Start of group name */
        begin_name_pos = parse->str_pos;
        end_name_pos = begin_name_pos;
        if ((err = re__parse_next_char(parse, &ch))) {
          return err;
        }
        if (ch == '>') {
          return re__parse_error(parse, "cannot create empty group name");
        } else if (ch == RE__PARSE_EOF) {
          return re__parse_error(parse, "unfinished group name");
        }
        while (1) {
          saved_name_pos = parse->str_pos;
          if ((err = re__parse_next_char(parse, &ch))) {
            return err;
          }
          if (ch == '>') {
            end_name_pos = saved_name_pos;
            group_flags |= RE__AST_GROUP_FLAG_NAMED;
            break;
          } else if (ch == RE__PARSE_EOF) {
            return re__parse_error(parse, "expected '>' to close group name");
          }
        }
        /* create group */
        break;
      } else {
        /* Handles EOF */
        return re__parse_error(
            parse, "expected one of '-', ':', '<', 'P', 'U', 'i', 'm', 's' for "
                   "group flags or name");
      }
    }
  } else {
    /* Rewind, we consumed an extra character */
    parse->str_pos = saved_pos;
  }
  {
    re_uint32 new_group_idx = 0;
    re__str_view group_name;
    re__ast new_group;
    if (group_flags & RE__AST_GROUP_FLAG_NAMED) {
      RE_ASSERT(!(group_flags & RE__AST_GROUP_FLAG_NONMATCHING));
      re__str_view_init_n(
          &group_name, re__str_view_get_data(&parse->str) + begin_name_pos,
          end_name_pos - begin_name_pos);
      new_group_idx = re__ast_root_get_num_groups(&parse->reg->data->ast_root);
      if ((err = re__ast_root_add_group(
               &parse->reg->data->ast_root, group_name))) {
        return err;
      }
    } else if (!(group_flags & RE__AST_GROUP_FLAG_NONMATCHING)) {
      re__str_view_init_null(&group_name);
      new_group_idx = re__ast_root_get_num_groups(&parse->reg->data->ast_root);
      if ((err = re__ast_root_add_group(
               &parse->reg->data->ast_root, group_name))) {
        return err;
      }
    }
    re__ast_init_group(&new_group, new_group_idx, group_flags);
    if ((err = re__parse_link_node(parse, new_group))) {
      return err;
    }
    if ((err = re__parse_push_frame(
             parse, re__parse_get_frame(parse)->ast_prev_child_ref, flags))) {
      return err;
    }
  }
  return err;
}

/* Act on a '|' character. If this is the first alteration, we wrap the current
 * group node in an ALT. If not, we simply add to the previous ALT. */
RE_INTERNAL re_error re__parse_alt(re__parse* parse)
{
  re__ast_type peek_type;
  re_error err = RE_ERROR_NONE;
  while (1) {
    peek_type = re__parse_get_frame_type(parse);
    if (peek_type == RE__AST_TYPE_CONCAT) {
      /* Pop all concatenations, alt takes priority */
      re__parse_pop_frame(parse);
    } else if (
        peek_type == RE__AST_TYPE_GROUP || peek_type == RE__AST_TYPE_NONE) {
      /* This is the initial alteration: "a|" or "(a|" */
      /* Note: the group in question could be the base frame. */
      /* In any case, we shim an ALT node in before the previous child. */
      /* Since GROUP nodes are defined to have a maximum of one child, we
       * don't need to mess around with the amount of children for either
       * node. */
      re__ast new_alt;
      re__ast_init_alt(&new_alt);
      if (re__parse_frame_is_empty(parse)) {
        /* Empty frame -- null alteration. */
        re__ast new_concat;
        re__ast_init_concat(&new_concat);
        if ((err = re__parse_link_node(parse, new_concat))) {
          return err;
        }
      }
      if ((err = re__parse_wrap_node(parse, new_alt))) {
        return err;
      }
      /* Push a new ALT frame */
      if ((err = re__parse_push_frame(
               parse, re__parse_get_frame(parse)->ast_prev_child_ref,
               re__parse_get_frame(parse)->flags))) {
        return err;
      }
      return err;
    } else { /* peek_type == RE__AST_TYPE_ALT */
      /* Third+ part of the alteration: "a|b|" or "(a|b|" */
      /* Indicate that there are no new children (this is the beginning
       * of the second+ part of an alteration) */
      re__parse_get_frame(parse)->ast_prev_child_ref = RE__AST_NONE;
      return err;
    }
  }
}

/* Create an empty concatenation for an ending ALT, if necessary. */
RE_INTERNAL re_error re__parse_alt_finish(re__parse* parse)
{
  if (re__parse_frame_is_empty(parse)) {
    re__ast new_concat;
    re__ast_init_concat(&new_concat);
    return re__parse_link_node(parse, new_concat);
  }
  return RE_ERROR_NONE;
}

/* Called after '(' */
re_error re__parse_group_end(re__parse* parse)
{
  re_error err = RE_ERROR_NONE;
  while (1) {
    re__ast_type peek_type = re__parse_get_frame_type(parse);
    if (peek_type == RE__AST_TYPE_NONE) {
      /* If we are at the absolute bottom of the stack, there was no opening
       * parentheses to begin with. */
      return re__parse_error(parse, "unmatched ')'");
    }
    if (peek_type == RE__AST_TYPE_ALT) {
      if ((err = re__parse_alt_finish(parse))) {
        return err;
      }
    }
    /* Now pop the current frame */
    re__parse_pop_frame(parse);
    /* If we just popped a group, finish */
    if (peek_type == RE__AST_TYPE_GROUP) {
      break;
    }
  }
  return err;
}

/* Create a new assert */
RE_INTERNAL re_error
re__parse_create_assert(re__parse* parse, re__assert_type assert_type)
{
  re__ast new_node;
  re__ast_init_assert(&new_node, assert_type);
  return re__parse_link_node(parse, new_node);
}

RE_INTERNAL re_error re__parse_create_star(re__parse* parse)
{
  re__ast new_star;
  if (re__parse_frame_is_empty(parse)) {
    return re__parse_error(parse, "cannot use '*' operator with nothing");
  }
  re__ast_init_quantifier(&new_star, 0, RE__AST_QUANTIFIER_INFINITY);
  re__ast_set_quantifier_greediness(
      &new_star,
      !!!(re__parse_get_frame(parse)->flags & RE__PARSE_FLAG_UNGREEDY));
  return re__parse_wrap_node(parse, new_star);
}

RE_INTERNAL re_error re__parse_create_question(re__parse* parse)
{
  re__ast new_star;
  if (re__parse_frame_is_empty(parse)) {
    return re__parse_error(parse, "cannot use '?' operator with nothing");
  }
  re__ast_init_quantifier(&new_star, 0, 2);
  re__ast_set_quantifier_greediness(
      &new_star,
      !!!(re__parse_get_frame(parse)->flags & RE__PARSE_FLAG_UNGREEDY));
  return re__parse_wrap_node(parse, new_star);
}

RE_INTERNAL re_error re__parse_create_plus(re__parse* parse)
{
  re__ast new_star;
  if (re__parse_frame_is_empty(parse)) {
    return re__parse_error(parse, "cannot use '?' operator with nothing");
  }
  re__ast_init_quantifier(&new_star, 1, RE__AST_QUANTIFIER_INFINITY);
  re__ast_set_quantifier_greediness(
      &new_star,
      !!!(re__parse_get_frame(parse)->flags & RE__PARSE_FLAG_UNGREEDY));
  return re__parse_wrap_node(parse, new_star);
}

RE_INTERNAL void re__parse_swap_greedy(re__parse* parse)
{
  re__ast* quant;
  /* Cannot make nothing ungreedy */
  RE_ASSERT(!re__parse_frame_is_empty(parse));
  quant = re__ast_root_get(
      &parse->reg->data->ast_root,
      re__parse_get_frame(parse)->ast_prev_child_ref);
  /* Must be a quantifier */
  RE_ASSERT(quant->type == RE__AST_TYPE_QUANTIFIER);
  re__ast_set_quantifier_greediness(
      quant, !re__ast_get_quantifier_greediness(quant));
}

RE_INTERNAL re_error re__parse_create_any_char(re__parse* parse)
{
  re__ast new_dot;
  if (!(re__parse_get_frame(parse)->flags & RE__PARSE_FLAG_DOT_NEWLINE)) {
    re__ast_init_any_char(&new_dot);
  } else {
    re__ast_init_any_char_newline(&new_dot);
  }
  return re__parse_link_node(parse, new_dot);
}

RE_INTERNAL re_error re__parse_create_any_byte(re__parse* parse)
{
  re__ast new_c;
  re__ast_init_any_byte(&new_c);
  return re__parse_link_node(parse, new_c);
}

/* Ingest a single rune, or casefold it if required. */
RE_INTERNAL re_error re__parse_create_rune(re__parse* parse, re_rune ch)
{
  re_error err = RE_ERROR_NONE;
  if (!(re__parse_get_frame(parse)->flags & RE__PARSE_FLAG_CASE_INSENSITIVE)) {
    re__ast new_rune;
    re__ast_init_rune(&new_rune, ch);
    return re__parse_link_node(parse, new_rune);
  } else {
    int num_fold_runes =
        re__rune_data_casefold(&parse->reg->data->rune_data, ch, RE_NULL);
    if (num_fold_runes == 1) {
      re__ast new_rune;
      re__ast_init_rune(&new_rune, ch);
      return re__parse_link_node(parse, new_rune);
    } else {
      re__charclass out;
      re__ast new_charclass;
      re_int32 out_ref;
      re__rune_range new_range;
      re__charclass_builder_begin(&parse->charclass_builder);
      re__charclass_builder_fold(&parse->charclass_builder);
      new_range.min = ch;
      new_range.max = ch;
      if ((err = re__charclass_builder_insert_range(
               &parse->charclass_builder, new_range))) {
        return err;
      }
      if ((err =
               re__charclass_builder_finish(&parse->charclass_builder, &out))) {
        return err;
      }
      if ((err = re__ast_root_add_charclass(
               &parse->reg->data->ast_root, out, &out_ref))) {
        re__charclass_destroy(&out);
        return err;
      }
      re__ast_init_charclass(&new_charclass, out_ref);
      if ((err = re__parse_link_node(parse, new_charclass))) {
        return err;
      }
      return err;
    }
  }
}

/* Add characters to charclass builder, folding if necessary.
 * This function is potentially very slow. */
RE_INTERNAL re_error
re__parse_charclass_insert_range(re__parse* parse, re__rune_range range)
{
  if (range.min > range.max) {
    RE__SWAP(range.min, range.max, re_rune);
  }
  return re__charclass_builder_insert_range(&parse->charclass_builder, range);
}

/* Create an ASCII charclass. */
RE_INTERNAL re_error re__parse_create_charclass_ascii(
    re__parse* parse, re__charclass_ascii_type ascii_cc, int inverted)
{
  re_error err = RE_ERROR_NONE;
  re__ast new_node;
  re_int32 new_class_ref;
  re__charclass new_class;
  re__charclass_builder_begin(&parse->charclass_builder);
  if (re__parse_get_frame(parse)->flags & RE__PARSE_FLAG_CASE_INSENSITIVE) {
    re__charclass_builder_fold(&parse->charclass_builder);
  }
  if ((err = re__charclass_builder_insert_ascii_class(
           &parse->charclass_builder, ascii_cc, inverted))) {
    return err;
  }
  if ((err = re__charclass_builder_finish(
           &parse->charclass_builder, &new_class))) {
    return err;
  }
  if ((err = re__ast_root_add_charclass(
           &parse->reg->data->ast_root, new_class, &new_class_ref))) {
    re__charclass_destroy(&new_class);
    return err;
  }
  /* ast_root now owns new_class */
  re__ast_init_charclass(&new_node, new_class_ref);
  return re__parse_link_node(parse, new_node);
}

/* Either insert a Unicode property into the builder or create a new charclass
 * out of the property. */
RE_INTERNAL re_error re__parse_unicode_property(
    re__parse* parse, int inverted, int accept_classes, int within_charclass)
{
  re_error err = RE_ERROR_NONE;
  re_rune ch;
  re_size name_start;
  re_size name_end;
  re__rune_range* ranges;
  re_size ranges_size;
  re__str_view view;
  if (!accept_classes) {
    return re__parse_error(
        parse, "cannot use Unicode property as range ending "
               "character for character class");
  }
  if ((err = re__parse_next_char(parse, &ch))) {
    return err;
  }
  if (ch != '{') {
    return re__parse_error(
        parse, "expected '{' to begin Unicode property name");
  }
  name_start = parse->str_pos;
  while (1) {
    if ((err = re__parse_next_char(parse, &ch))) {
      return err;
    }
    if (ch == '}') {
      break;
    } else if (ch == RE__PARSE_EOF) {
      return re__parse_error(
          parse, "expected '}' to close Unicode property name");
    } else {
      name_end = parse->str_pos;
    }
  }
  if ((err = re__rune_data_get_property(
           &parse->reg->data->rune_data,
           re__str_view_get_data(&parse->str) + name_start,
           name_end - name_start, &ranges, &ranges_size))) {
    if (err == RE_ERROR_INVALID) {
      return re__parse_error(parse, "invalid Unicode property name");
    }
    return err;
  }
  re__str_view_init_n(
      &view, re__str_view_get_data(&parse->str) + name_start,
      name_end - name_start);
  if (within_charclass) {
    if ((err = re__charclass_builder_insert_property(
             &parse->charclass_builder, view, inverted))) {
      if (err == RE_ERROR_INVALID) {
        return re__parse_error(parse, "invalid Unicode property name");
      }
      return err;
    }
    return err;
  } else {
    re__charclass out;
    re__ast new_node;
    re_int32 out_charclass_ref;
    re__charclass_builder_begin(&parse->charclass_builder);
    if (re__parse_get_frame(parse)->flags & RE__PARSE_FLAG_CASE_INSENSITIVE) {
      re__charclass_builder_fold(&parse->charclass_builder);
    }
    if ((err = re__charclass_builder_insert_property(
             &parse->charclass_builder, view, inverted))) {
      if (err == RE_ERROR_INVALID) {
        return re__parse_error(parse, "invalid Unicode property name");
      }
      return err;
    }
    if ((err = re__charclass_builder_finish(&parse->charclass_builder, &out))) {
      return err;
    }
    if ((err = re__ast_root_add_charclass(
             &parse->reg->data->ast_root, out, &out_charclass_ref))) {
      re__charclass_destroy(&out);
      return err;
    }
    re__ast_init_charclass(&new_node, out_charclass_ref);
    return re__parse_link_node(parse, new_node);
  }
}

RE_INTERNAL int re__parse_oct(re_rune ch)
{
  switch (ch) {
  case '0':
    return 0;
  case '1':
    return 1;
  case '2':
    return 2;
  case '3':
    return 3;
  case '4':
    return 4;
  case '5':
    return 5;
  case '6':
    return 6;
  case '7':
    return 7;
  default:
    return -1;
  }
}

RE_INTERNAL int re__parse_hex(re_rune ch)
{
  switch (ch) {
  case '0':
    return 0;
  case '1':
    return 1;
  case '2':
    return 2;
  case '3':
    return 3;
  case '4':
    return 4;
  case '5':
    return 5;
  case '6':
    return 6;
  case '7':
    return 7;
  case '8':
    return 8;
  case '9':
    return 9;
  case 'A':
    return 10;
  case 'B':
    return 11;
  case 'C':
    return 12;
  case 'D':
    return 13;
  case 'E':
    return 14;
  case 'F':
    return 15;
  case 'a':
    return 10;
  case 'b':
    return 11;
  case 'c':
    return 12;
  case 'd':
    return 13;
  case 'e':
    return 14;
  case 'f':
    return 15;
  default:
    return -1;
  }
}

RE_INTERNAL int re__parse_dec(re_rune ch)
{
  switch (ch) {
  case '0':
    return 0;
  case '1':
    return 1;
  case '2':
    return 2;
  case '3':
    return 3;
  case '4':
    return 4;
  case '5':
    return 5;
  case '6':
    return 6;
  case '7':
    return 7;
  case '8':
    return 8;
  case '9':
    return 9;
  default:
    return -1;
  }
}

/* Parse an escape character */
/* Called after '\' */
RE_INTERNAL re_error re__parse_escape(
    re__parse* parse, re_rune* out_char, int accept_classes,
    int within_charclass)
{
  re_error err = RE_ERROR_NONE;
  re_rune ch;
  re_int32 accum = 0;
  re_int32 accum2 = 0;
  re_size saved_pos;
  int i = 0;
  if ((err = re__parse_next_char(parse, &ch))) {
    return err;
  }
  saved_pos = parse->str_pos;
  *out_char = RE__PARSE_EOF;
  if (ch == RE__PARSE_EOF) {
    return re__parse_error(parse, "unfinished escape sequence");
  } else if ((accum = re__parse_oct(ch)) != -1) {
    /* \[0-7] | Octal digits */
    for (i = 0; i < 2; i++) {
      if ((err = re__parse_next_char(parse, &ch))) {
        return err;
      }
      if ((accum2 = re__parse_oct(ch)) != -1) {
        /* \[0-7][0-7] | Found second/third octal digit */
        accum *= 8;
        accum += accum2;
        saved_pos = parse->str_pos;
      } else {
        /* \[0-7]<*> | Did not find second/third octal digit */
        break;
      }
    }
    *out_char = accum;
    parse->str_pos = saved_pos;
  } else if (ch == 'A') {
    /* \A | Absolute text start */
    if (!within_charclass) {
      if ((err = re__parse_create_assert(
               parse, RE__ASSERT_TYPE_TEXT_START_ABSOLUTE))) {
        return err;
      }
    } else {
      return re__parse_error(parse, "cannot use \\A inside character class");
    }
  } else if (ch == 'B') {
    /* \B | Not a word boundary */
    if (!within_charclass) {
      if ((err = re__parse_create_assert(parse, RE__ASSERT_TYPE_WORD_NOT))) {
        return err;
      }
    } else {
      return re__parse_error(parse, "cannot use \\B inside character class");
    }
  } else if (ch == 'C') {
    /* \C: Any *byte* (NOT any char) */
    if (!within_charclass) {
      if ((err = re__parse_create_any_byte(parse))) {
        return err;
      }
    } else {
      return re__parse_error(parse, "cannot use \\C inside character class");
    }
  } else if (ch == 'D') {
    /* \D: Non-digit characters */
    if (accept_classes) {
      if (!within_charclass) {
        if ((err = re__parse_create_charclass_ascii(
                 parse, RE__CHARCLASS_ASCII_TYPE_DIGIT, 1))) {
          return err;
        }
      } else {
        if ((err = re__charclass_builder_insert_ascii_class(
                 &parse->charclass_builder, RE__CHARCLASS_ASCII_TYPE_DIGIT,
                 1))) {
          return err;
        }
      }
    } else {
      return re__parse_error(
          parse, "cannot use \\D as ending character for range");
    }
  } else if (ch == 'E') {
    /* \E | Invalid here */
    return re__parse_error(parse, "\\E can only be used from within \\Q");
  } else if (ch == 'P') {
    /* \P | Inverted Unicode character class */
    return re__parse_unicode_property(
        parse, 1, accept_classes, within_charclass);
  } else if (ch == 'Q') {
    /* \Q | Quote begin */
    if (!within_charclass) {
      int is_escape = 0;
      while (1) {
        saved_pos = parse->str_pos;
        if ((err = re__parse_next_char(parse, &ch))) {
          return err;
        }
        if (!is_escape) {
          if (ch == RE__PARSE_EOF) {
            /* \Q<*><EOF> | End, having added all previous chars */
            parse->str_pos = saved_pos;
            break;
          } else if (ch == '\\') {
            /* \Q\ | Escape within '\Q' */
            is_escape = 1;
          } else {
            /* \Q<*> | Add character */
            if ((err = re__parse_create_rune(parse, ch))) {
              return err;
            }
          }
        } else {
          if (ch == RE__PARSE_EOF) {
            /* \Q\<EOF> | Error */
            return re__parse_error(
                parse, "expected 'E' or a character after '\\' within \"\\Q\"");
          } else if (ch == 'E') {
            /* \Q<*>\E | Finish quote */
            break;
          } else {
            if ((err = re__parse_create_rune(parse, ch))) {
              return err;
            }
            is_escape = 0;
          }
        }
      }
    } else {
      return re__parse_error(parse, "cannot use \\Q inside character class");
    }
  } else if (ch == 'S') {
    /* \S | Non-whitespace characters (Perl) */
    if (accept_classes) {
      if (!within_charclass) {
        if ((err = re__parse_create_charclass_ascii(
                 parse, RE__CHARCLASS_ASCII_TYPE_PERL_SPACE, 1))) {
          return err;
        }
      } else {
        if ((err = re__charclass_builder_insert_ascii_class(
                 &parse->charclass_builder, RE__CHARCLASS_ASCII_TYPE_PERL_SPACE,
                 1))) {
          return err;
        }
      }
    } else {
      return re__parse_error(
          parse, "cannot use \\S as ending character for range");
    }
  } else if (ch == 'W') {
    /* \W | Not a word character */
    if (accept_classes) {
      if (!within_charclass) {
        if ((err = re__parse_create_charclass_ascii(
                 parse, RE__CHARCLASS_ASCII_TYPE_WORD, 1))) {
          return err;
        }
      } else {
        if ((err = re__charclass_builder_insert_ascii_class(
                 &parse->charclass_builder, RE__CHARCLASS_ASCII_TYPE_WORD,
                 1))) {
          return err;
        }
      }
    } else {
      return re__parse_error(
          parse, "cannot use \\W as ending character for range");
    }
  } else if (ch == 'a') {
    /* \a | Bell character */
    *out_char = 0x7;
  } else if (ch == 'b') {
    /* \b | Word boundary */
    if (!within_charclass) {
      if ((err = re__parse_create_assert(parse, RE__ASSERT_TYPE_WORD))) {
        return err;
      }
    } else {
      return re__parse_error(parse, "cannot use \\b inside character class");
    }
  } else if (ch == 'd') {
    /* \d | Digit characters */
    if (accept_classes) {
      if (!within_charclass) {
        if ((err = re__parse_create_charclass_ascii(
                 parse, RE__CHARCLASS_ASCII_TYPE_DIGIT, 0))) {
          return err;
        }
      } else {
        if ((err = re__charclass_builder_insert_ascii_class(
                 &parse->charclass_builder, RE__CHARCLASS_ASCII_TYPE_DIGIT,
                 0))) {
          return err;
        }
      }
    } else {
      return re__parse_error(
          parse, "cannot use \\d as ending character for range");
    }
  } else if (ch == 'f') {
    /* \f | Form feed */
    *out_char = 0xC;
  } else if (ch == 'n') {
    /* \n | Newline */
    *out_char = 0xA;
  } else if (ch == 'p') {
    /* \p | Unicode character class */
    return re__parse_unicode_property(
        parse, 0, accept_classes, within_charclass);
  } else if (ch == 'r') {
    /* \r | Carriage return */
    *out_char = 0xD;
  } else if (ch == 's') {
    /* \s | Whitespace (Perl) */
    if (accept_classes) {
      if (!within_charclass) {
        if ((err = re__parse_create_charclass_ascii(
                 parse, RE__CHARCLASS_ASCII_TYPE_PERL_SPACE, 0))) {
          return err;
        }
      } else {
        if ((err = re__charclass_builder_insert_ascii_class(
                 &parse->charclass_builder, RE__CHARCLASS_ASCII_TYPE_PERL_SPACE,
                 0))) {
          return err;
        }
      }
    } else {
      return re__parse_error(
          parse, "cannot use \\s as ending character for range");
    }
  } else if (ch == 't') {
    /* \t | Horizontal tab */
    *out_char = 0x9;
  } else if (ch == 'v') {
    /* \v | Vertical tab */
    *out_char = 0xB;
  } else if (ch == 'w') {
    /* \w | Word character */
    if (accept_classes) {
      if (!within_charclass) {
        if ((err = re__parse_create_charclass_ascii(
                 parse, RE__CHARCLASS_ASCII_TYPE_WORD, 0))) {
          return err;
        }
      } else {
        if ((err = re__charclass_builder_insert_ascii_class(
                 &parse->charclass_builder, RE__CHARCLASS_ASCII_TYPE_WORD,
                 0))) {
          return err;
        }
      }
    } else {
      return re__parse_error(
          parse, "cannot use \\w as ending character for range");
    }
  } else if (ch == 'x') {
    /* \x | Two-digit hex literal or one-to-six digit hex literal */
    accum = 0;
    if ((err = re__parse_next_char(parse, &ch))) {
      return err;
    }
    if (ch == '{') {
      /* \x{ | Bracketed hex literal */
      int hex_idx = 0;
      while (1) {
        if ((err = re__parse_next_char(parse, &ch))) {
          return err;
        }
        if (ch == RE__PARSE_EOF) {
          /* \x{<EOF> | Error condition */
          return re__parse_error(
              parse, "expected one to six hex characters for bracketed hex "
                     "escape \"\\x{\"");
        } else if (ch == '}') {
          if (hex_idx == 0) {
            /* \x{} | Error condition */
            return re__parse_error(
                parse, "expected one to six hex characters for bracketed hex "
                       "escape \"\\x{\"");
          } else {
            /* \x{[0-9a-fA-F]} | Finish */
            *out_char = accum;
            break;
          }
        } else if ((accum2 = re__parse_hex(ch)) == -1) {
          /* \x{<*> | Invalid, error condition */
          return re__parse_error(
              parse, "expected one to six hex characters for bracketed hex "
                     "escape \"\\x{\"");
        } else {
          /* \x{[0-9a-fA-F] | Add to accumulator */
          accum *= 16;
          accum += accum2;
          if (accum > RE_RUNE_MAX) {
            return re__parse_error(
                parse, "bracketed hex literal out of range [0, 0x10FFFF]");
          }
        }
        hex_idx++;
      }
    } else if ((accum = re__parse_hex(ch)) == -1) {
      /* Handles EOF */
      return re__parse_error(
          parse, "expected two hex characters or a bracketed hex literal for "
                 "hex escape \"\\x\"");
    } else {
      /* \x[0-9a-fA-F] | Two-digit hex sequence */
      if ((err = re__parse_next_char(parse, &ch))) {
        return err;
      }
      if ((accum2 = re__parse_hex(ch)) == -1) {
        /* Handles EOF */
        return re__parse_error(
            parse, "expected two hex characters or a bracketed hex literal for "
                   "hex escape \"\\x\"");
      }
      accum *= 16;
      accum += accum2;
      *out_char = accum;
    }
  } else if (ch == 'z') {
    /* \z | Absolute text end */
    if (!within_charclass) {
      if ((err = re__parse_create_assert(
               parse, RE__ASSERT_TYPE_TEXT_END_ABSOLUTE))) {
        return err;
      }
    } else {
      return re__parse_error(parse, "cannot use \\z inside character class");
    }
  } else if (ch >= 'A' && ch <= 'Z') {
    /* \[A-Z] | Catch-all for invalid escape sequences */
    return re__parse_error(parse, "invalid escape sequence");
  } else if (ch >= 'a' && ch <= 'z') {
    /* \[a-z] | Catch-all for invalid escape sequences */
    return re__parse_error(parse, "invalid escape sequence");
  } else {
    /* \* | Literal * */
    *out_char = ch;
  }
  return err;
}

/* Parse a character class. */
RE_INTERNAL re_error re__parse_charclass(re__parse* parse)
{
  /* After first charclass bracket: [ */
  re_error err = RE_ERROR_NONE;
  re_rune ch;
  re__rune_range range;
  re_rune invert_char =
      (re__parse_get_frame(parse)->flags & RE__PARSE_FLAG_GLOB) ? '!' : '^';
  re__charclass_builder_begin(&parse->charclass_builder);
  if (re__parse_get_frame(parse)->flags & RE__PARSE_FLAG_CASE_INSENSITIVE) {
    re__charclass_builder_fold(&parse->charclass_builder);
  }
  if ((err = re__parse_next_char(parse, &ch))) {
    return err;
  }
  if (ch == RE__PARSE_EOF) {
    return re__parse_error(
        parse, "expected '^', characters, character classes, or character "
               "ranges for character class expression '['");
  } else if (ch == ']') {
    /* [] | Set right bracket as low character, look for dash */
    range.min = ']';
    goto before_dash;
  } else if (ch == invert_char) {
    /* [^ | Start of invert */
    re__charclass_builder_invert(&parse->charclass_builder);
  } else {
    /* All other characters: begin parsing loop */
    goto range_loop;
  }
  while (1) {
    range.min = -1;
    range.max = -1;
    if ((err = re__parse_next_char(parse, &ch))) {
      return err;
    }
  range_loop:
    if (ch == RE__PARSE_EOF) {
      return re__parse_error(
          parse, "expected characters, character classes, or character "
                 "ranges for character class expression '['");
    } else if (ch == '[') {
      /* [[ | Literal [ or char class */
      range.min = '[';
      range.max = '[';
      if ((err = re__parse_next_char(parse, &ch))) {
        return err;
      }
      if (ch == RE__PARSE_EOF) {
        /* [[<EOF> | Error */
        return re__parse_error(
            parse, "expected characters, character classes, or character "
                   "ranges for character class expression '['");
      } else if (ch == '\\') {
        /* [[\ | Push bracket, begin escape */
        if ((err = re__parse_charclass_insert_range(parse, range))) {
          return err;
        }
        /* Go to charclass escape */
        goto range_loop;
      } else if (ch == ']') {
        /* [[] | A single [. */
        if ((err = re__parse_charclass_insert_range(parse, range))) {
          return err;
        }
        /* Stop. */
        break;
      } else if (ch == ':') {
        /* [[: | Start of ASCII charclass */
        re_size name_start_pos = parse->str_pos;
        re_size name_end_pos = parse->str_pos;
        int inverted = 0;
        if ((err = re__parse_next_char(parse, &ch))) {
          return err;
        }
        if (ch == invert_char) {
          /* [[:^ | Invert ASCII charclass */
          inverted = 1;
          name_start_pos = parse->str_pos;
          name_end_pos = parse->str_pos;
        } else {
          goto ascii_name;
        }
        while (1) {
          if ((err = re__parse_next_char(parse, &ch))) {
            return err;
          }
        ascii_name:
          if (ch == RE__PARSE_EOF) {
            /* [[:<EOF> | Error */
            return re__parse_error(
                parse, "expected named character class for \"[[:\"");
          } else if (ch == ':') {
            /* [[:<*>: | Look for right bracket to finish */
            if ((err = re__parse_next_char(parse, &ch))) {
              return err;
            }
            if (ch == RE__PARSE_EOF || ch != ']') {
              /* [[:<*>:<EOF> | Error */
              return re__parse_error(
                  parse, "expected named character class for \"[[:\"");
            } else { /* ch == ']' */
              /* [[:<*>:] | Finish named char class */
              break;
            }
          } else {
            name_end_pos = parse->str_pos;
          }
        }
        {
          re__str_view name_view;
          re__str_view_init_n(
              &name_view, re__str_view_get_data(&parse->str) + name_start_pos,
              name_end_pos - name_start_pos);
          if ((err = re__charclass_builder_insert_ascii_class_by_str(
                   &parse->charclass_builder, name_view, inverted))) {
            if (err == RE_ERROR_INVALID) {
              /* couldn't find charclass with name */
              return re__parse_error(
                  parse, "unknown ASCII character class name");
            } else {
              return err;
            }
          }
          continue;
        }
      } else if (ch == '-') {
        /* [[- | Start of range. Set low rune to bracket. Look for high rune. */
        range.min = '[';
        goto before_high_rune;
      } else {
        /* [[<*> | Add bracket. Set low rune to next char. Look for -. */
        if ((err = re__parse_charclass_insert_range(parse, range))) {
          return err;
        }
        range.min = ch;
        goto before_dash;
      }
    } else if (ch == '\\') {
      /* [\ | Starting character escape. */
      if ((err = re__parse_escape(parse, &range.min, 1, 1))) {
        return err;
      }
      if (range.min == RE__PARSE_EOF) {
        /* We parsed a character class, go back to start */
        continue;
      }
      /* We parsed a single character, it is set as the minimum value */
    } else if (ch == ']') {
      /* [] | End char class */
      break;
    } else {
      /* [<*> | Set low character of char class */
      range.min = ch;
    }
  before_dash:
    if ((err = re__parse_next_char(parse, &ch))) {
      return err;
    }
    if (ch == RE__PARSE_EOF) {
      /* Cannot EOF here */
      return re__parse_error(
          parse, "expected characters, character classes, or character "
                 "ranges for character class expression '['");
    } else if (ch == ']') {
      /* [<*>] | Add lo_rune, finish. */
      range.max = range.min;
      if ((err = re__parse_charclass_insert_range(parse, range))) {
        return err;
      }
      break;
    } else if (ch != '-') {
      /* [<*><*> | Add lo_rune, continue. */
      range.max = range.min;
      if ((err = re__parse_charclass_insert_range(parse, range))) {
        return err;
      }
      goto range_loop;
    }
    /* [<*>- | Look for end character for range */
  before_high_rune:
    if ((err = re__parse_next_char(parse, &ch))) {
      return err;
    }
    if (ch == RE__PARSE_EOF) {
      /* Cannot EOF here */
      return re__parse_error(
          parse, "expected characters, character classes, or character "
                 "ranges for character class expression '['");
    } else if (ch == ']') {
      /* [<*>-] | End character class. Add lo_rune. Add dash. */
      range.max = range.min;
      if ((err = re__parse_charclass_insert_range(parse, range))) {
        return err;
      }
      range.min = '-';
      range.max = '-';
      if ((err = re__parse_charclass_insert_range(parse, range))) {
        return err;
      }
      break;
    } else if (ch == '\\') {
      /* [<*>-\ | Ending character escape. */
      if ((err = re__parse_escape(parse, &range.max, 0, 1))) {
        return err;
      }
      if ((err = re__parse_charclass_insert_range(parse, range))) {
        return err;
      }
    } else {
      /* [<*>-<*> | Add range. */
      range.max = ch;
      if ((err = re__parse_charclass_insert_range(parse, range))) {
        return err;
      }
    }
  }
  {
    re__ast new_node;
    re__charclass new_charclass;
    re_int32 new_charclass_ref;
    if ((err = re__charclass_builder_finish(
             &parse->charclass_builder, &new_charclass))) {
      return err;
    }
    if (re__charclass_size(&new_charclass) == 1) {
      re__rune_range first_range = re__charclass_get_ranges(&new_charclass)[0];
      if (first_range.min == first_range.max) {
        re__ast_init_rune(&new_node, first_range.min);
        if ((err = re__parse_link_node(parse, new_node))) {
          re__charclass_destroy(&new_charclass);
          return err;
        }
        re__charclass_destroy(&new_charclass);
        return err;
      }
    }
    /* We own new_charclass */
    if ((err = re__ast_root_add_charclass(
             &parse->reg->data->ast_root, new_charclass, &new_charclass_ref))) {
      re__charclass_destroy(&new_charclass);
      return err;
    }
    /* ast_root owns new_charclass */
    re__ast_init_charclass(&new_node, new_charclass_ref);
    if ((err = re__parse_link_node(parse, new_node))) {
      return err;
    }
  }
  return err;
}

RE_INTERNAL re_error re__parse_count(re__parse* parse)
{
  re_error err = RE_ERROR_NONE;
  re_rune ch;
  re_int32 rep_min = 0;
  re_int32 rep_max = 0;
  re_int32 temp_num;
  re__ast out;
  if (re__parse_frame_is_empty(parse)) {
    return re__parse_error(parse, "cannot use '{' operator with nothing");
  }
  if ((err = re__parse_next_char(parse, &ch))) {
    return err;
  }
  if (ch == ',' || ch == '}') {
    return re__parse_error(
        parse, "must specify lower bound for repeat expression");
  }
  if ((rep_min = re__parse_dec(ch)) == -1) {
    return re__parse_error(
        parse, "invalid decimal literal for repeat expression");
  }
  while (1) {
    if ((err = re__parse_next_char(parse, &ch))) {
      return err;
    }
    if (ch == ',') {
      break;
    } else if (ch == '}') {
      rep_max = rep_min + 1;
      goto construct;
    }
    if ((temp_num = re__parse_dec(ch)) == -1) {
      return re__parse_error(
          parse, "invalid decimal literal for repeat expression");
    }
    rep_min *= 10;
    rep_min += temp_num;
    if (rep_min > RE__AST_QUANTIFIER_MAX) {
      return re__parse_error(parse, "repeat amount exceeds maximum");
    }
  }
  if ((err = re__parse_next_char(parse, &ch))) {
    return err;
  }
  if (ch == '}') {
    rep_max = RE__AST_QUANTIFIER_INFINITY;
    goto construct;
  }
  goto afterfirst;
  while (1) {
    if ((err = re__parse_next_char(parse, &ch))) {
      return err;
    }
  afterfirst:
    if (ch == '}') {
      rep_max += 1;
      break;
    }
    if ((temp_num = re__parse_dec(ch)) == -1) {
      return re__parse_error(
          parse, "invalid decimal literal for repeat expression");
    }
    rep_max *= 10;
    rep_max += temp_num;
    /* >= used here because rep_max is one less than the internal rep. */
    if (rep_max >= RE__AST_QUANTIFIER_MAX) {
      return re__parse_error(parse, "repeat amount exceeds maximum");
    }
  }
construct:
  if (rep_max <= rep_min) {
    return re__parse_error(parse, "max repeat cannot be less than min repeat");
  }
  re__ast_init_quantifier(&out, rep_min, rep_max);
  re__ast_set_quantifier_greediness(
      &out, !!!(re__parse_get_frame(parse)->flags & RE__PARSE_FLAG_UNGREEDY));
  return re__parse_wrap_node(parse, out);
}

RE_INTERNAL re_error re__parse_build_flags(
    re_syntax_flags syntax_flags, re__parse_flags* out_parse_flags)
{
  re__parse_flags out = 0;
  if (syntax_flags & RE_SYNTAX_FLAG_IGNORECASE) {
    out |= RE__PARSE_FLAG_CASE_INSENSITIVE;
  }
  if (syntax_flags & RE_SYNTAX_FLAG_MULTILINE) {
    out |= RE__PARSE_FLAG_MULTILINE;
  }
  if (syntax_flags & RE_SYNTAX_FLAG_DOTALL) {
    out |= RE__PARSE_FLAG_DOT_NEWLINE;
  }
  if (syntax_flags & RE_SYNTAX_FLAG_GLOB) {
    out |= RE__PARSE_FLAG_GLOB;
  }
  *out_parse_flags = out;
  return RE_ERROR_NONE;
}

RE_INTERNAL re_error
re__parse_str(re__parse* parse, re__str_view str, re_syntax_flags syntax_flags)
{
  re_error err = RE_ERROR_NONE;
  re_int32 start_ref = parse->reg->data->ast_root.root_ref;
  re__parse_flags initial_flags;
  /* Set string */
  parse->str = str;
  parse->str_pos = 0;
  /* Reset frame vector */
  while (re__parse_frame_vec_size(&parse->frames)) {
    re__parse_frame_vec_pop(&parse->frames);
  }
  if (parse->reg->data->set != RE__AST_NONE) {
    /* Set mode, push the set root. */
    start_ref = parse->reg->data->set;
  }
  if ((err = re__parse_build_flags(syntax_flags, &initial_flags))) {
    return err;
  }
  if ((err = re__parse_push_frame(parse, start_ref, 0))) {
    return err;
  }
  while (1) {
    re_rune ch;
    if ((err = re__parse_next_char(parse, &ch))) {
      goto error;
    }
    if (!(re__parse_get_frame(parse)->flags & RE__PARSE_FLAG_GLOB)) {
      if (ch == '$') {
        re__assert_type assert_type = RE__ASSERT_TYPE_TEXT_END_ABSOLUTE;
        if (re__parse_get_frame(parse)->flags & RE__PARSE_FLAG_MULTILINE) {
          assert_type = RE__ASSERT_TYPE_TEXT_END;
        }
        if ((err = re__parse_create_assert(parse, assert_type))) {
          goto error;
        }
      } else if (ch == '(') {
        /* ( | Begin a group. */
        if ((err = re__parse_group_begin(parse))) {
          goto error;
        }
      } else if (ch == ')') {
        /* ) | End a group. */
        if ((err = re__parse_group_end(parse))) {
          goto error;
        }
      } else if (ch == '*' || ch == '?' || ch == '+') {
        re_size saved_pos;
        if (ch == '*') {
          if ((err = re__parse_create_star(parse))) {
            goto error;
          }
        } else if (ch == '?') {
          if ((err = re__parse_create_question(parse))) {
            goto error;
          }
        } else { /* ch == '+' */
          if ((err = re__parse_create_plus(parse))) {
            goto error;
          }
        }
        saved_pos = parse->str_pos;
        if ((err = re__parse_next_char(parse, &ch))) {
          goto error;
        }
        if (ch == '?') {
          /* [*+?]? | Make previous operator non-greedy. */
          re__parse_swap_greedy(parse);
        } else {
          /* Handles EOF */
          parse->str_pos = saved_pos;
        }
      } else if (ch == '.') {
        /* . | Create an "any character." */
        if ((err = re__parse_create_any_char(parse))) {
          goto error;
        }
      } else if (ch == '[') {
        /* [ | Start of a character class. */
        if ((err = re__parse_charclass(parse))) {
          goto error;
        }
      } else if (ch == '\\') {
        /* \ | Start of escape sequence. */
        re_rune esc_char;
        if ((err = re__parse_escape(parse, &esc_char, 1, 0))) {
          goto error;
        }
        if (esc_char != RE__PARSE_EOF) {
          if ((err = re__parse_create_rune(parse, esc_char))) {
            goto error;
          }
        }
      } else if (ch == '^') {
        /* ^ | Text start assert. */
        re__assert_type assert_type = RE__ASSERT_TYPE_TEXT_START_ABSOLUTE;
        if (re__parse_get_frame(parse)->flags & RE__PARSE_FLAG_MULTILINE) {
          assert_type = RE__ASSERT_TYPE_TEXT_START;
        }
        if ((err = re__parse_create_assert(parse, assert_type))) {
          goto error;
        }
      } else if (ch == '{') {
        /* { | Start of counting form. */
        if ((err = re__parse_count(parse))) {
          goto error;
        }
      } else if (ch == '|') {
        /* | | Alternation. */
        if ((err = re__parse_alt(parse))) {
          goto error;
        }
      } else if (ch == RE__PARSE_EOF) {
        /* <EOF> | Finish. */
        break;
      } else {
        /* Any other character. */
        if ((err = re__parse_create_rune(parse, ch))) {
          goto error;
        }
      }
    } else {
      /* glob syntax */
      if (ch == '?') {
        /* ? | Any character */
        if ((err = re__parse_create_any_char(parse))) {
          goto error;
        }
      } else if (ch == '*') {
        /* * | .* */
        if ((err = re__parse_create_any_char(parse))) {
          goto error;
        }
        if ((err = re__parse_create_star(parse))) {
          goto error;
        }
      } else if (ch == '[') {
        /* [ | Build charclass */
        if ((err = re__parse_charclass(parse))) {
          goto error;
        }
      } else if (ch == '/') {
        /* / | Disallowed */
        if ((err =
                 re__parse_error(parse, "cannot use '/' in glob expression"))) {
          goto error;
        }
      } else {
        /* <*> | Add rune */
        if ((err = re__parse_create_rune(parse, ch))) {
          goto error;
        }
      }
    }
  }
  /* Pop all frames. */
  while (1) {
    re__ast_type peek_type = re__parse_get_frame_type(parse);
    if (peek_type == RE__AST_TYPE_NONE) {
      /* Successfully hit bottom frame. */
      if (parse->reg->data->set != RE__AST_NONE) {
        /* Finish the alt frame. */
        if ((err = re__parse_alt_finish(parse))) {
          goto error;
        }
      }
      break;
    } else if (peek_type == RE__AST_TYPE_CONCAT) {
      re__parse_pop_frame(parse);
    } else if (peek_type == RE__AST_TYPE_ALT) {
      if ((err = re__parse_alt_finish(parse))) {
        goto error;
      }
      re__parse_pop_frame(parse);
    } else { /* peek_type == RE__AST_TYPE_GROUP */
      /* If we find a group, that means it has not been closed. */
      err = re__parse_error(parse, "unmatched '('");
      goto error;
    }
  }
error:
  return err;
}

#undef RE__PARSE_EOF

/* re */
#define RE__PROG_INST_TYPE_MASK 7

RE_INTERNAL void
re__prog_inst_init(re__prog_inst* inst, re__prog_inst_type inst_type)
{
  inst->data0 = inst_type & 7;
#if RE_DEBUG
  inst->data1 = 0;
#endif
}

RE_INTERNAL void re__prog_inst_init_byte(re__prog_inst* inst, re_uint8 byte)
{
  re__prog_inst_init(inst, RE__PROG_INST_TYPE_BYTE);
  inst->data1 = byte;
}

RE_INTERNAL void
re__prog_inst_init_byte_range(re__prog_inst* inst, re__byte_range br)
{
  re__prog_inst_init(inst, RE__PROG_INST_TYPE_BYTE_RANGE);
  RE_ASSERT(br.min <= br.max);
  inst->data1 = (re_uint32)br.min | ((re_uint32)br.max << 16);
}

RE_INTERNAL void re__prog_inst_init_split(
    re__prog_inst* inst, re__prog_loc primary, re__prog_loc secondary)
{
  re__prog_inst_init(inst, RE__PROG_INST_TYPE_SPLIT);
  inst->data0 = (inst->data0 & 7) | (primary << 3);
  inst->data1 = secondary;
}

RE_INTERNAL void
re__prog_inst_init_match(re__prog_inst* inst, re_uint32 match_idx)
{
  re__prog_inst_init(inst, RE__PROG_INST_TYPE_MATCH);
  inst->data1 = match_idx;
}

RE_INTERNAL void re__prog_inst_init_fail(re__prog_inst* inst)
{
  re__prog_inst_init(inst, RE__PROG_INST_TYPE_FAIL);
}

RE_INTERNAL void
re__prog_inst_init_assert(re__prog_inst* inst, re_uint32 assert_context)
{
  re__prog_inst_init(inst, RE__PROG_INST_TYPE_ASSERT);
  inst->data1 = assert_context;
}

RE_INTERNAL void
re__prog_inst_init_save(re__prog_inst* inst, re_uint32 save_idx)
{
  re__prog_inst_init(inst, RE__PROG_INST_TYPE_SAVE);
  inst->data1 = save_idx;
}

RE_INTERNAL re__prog_loc re__prog_inst_get_primary(const re__prog_inst* inst)
{
  return inst->data0 >> 3;
}

RE_INTERNAL void
re__prog_inst_set_primary(re__prog_inst* inst, re__prog_loc loc)
{
  inst->data0 = (inst->data0 & 7) | (loc << 3);
}

RE_INTERNAL re_uint8 re__prog_inst_get_byte(const re__prog_inst* inst)
{
  RE_ASSERT(re__prog_inst_get_type(inst) == RE__PROG_INST_TYPE_BYTE);
  return (re_uint8)(inst->data1 & 0xFF);
}

RE_INTERNAL re_uint8 re__prog_inst_get_byte_min(const re__prog_inst* inst)
{
  RE_ASSERT(re__prog_inst_get_type(inst) == RE__PROG_INST_TYPE_BYTE_RANGE);
  return inst->data1 & 0xFF;
}

RE_INTERNAL re_uint8 re__prog_inst_get_byte_max(const re__prog_inst* inst)
{
  RE_ASSERT(re__prog_inst_get_type(inst) == RE__PROG_INST_TYPE_BYTE_RANGE);
  return (re_uint8)((inst->data1 >> 16) & 0xFF);
}

RE_INTERNAL re__prog_loc
re__prog_inst_get_split_secondary(const re__prog_inst* inst)
{
  RE_ASSERT(re__prog_inst_get_type(inst) == RE__PROG_INST_TYPE_SPLIT);
  return inst->data1;
}

RE_INTERNAL void
re__prog_inst_set_split_secondary(re__prog_inst* inst, re__prog_loc loc)
{
  RE_ASSERT(re__prog_inst_get_type(inst) == RE__PROG_INST_TYPE_SPLIT);
  inst->data1 = loc;
}

RE_INTERNAL re__prog_inst_type re__prog_inst_get_type(const re__prog_inst* inst)
{
  return inst->data0 & 7;
}

RE_INTERNAL re_uint32 re__prog_inst_get_match_idx(const re__prog_inst* inst)
{
  RE_ASSERT(re__prog_inst_get_type(inst) == RE__PROG_INST_TYPE_MATCH);
  return inst->data1;
}

RE_INTERNAL re_uint32 re__prog_inst_get_save_idx(const re__prog_inst* inst)
{
  RE_ASSERT(re__prog_inst_get_type(inst) == RE__PROG_INST_TYPE_SAVE);
  return inst->data1;
}

RE_INTERNAL re_uint32 re__prog_inst_get_assert_ctx(const re__prog_inst* inst)
{
  RE_ASSERT(re__prog_inst_get_type(inst) == RE__PROG_INST_TYPE_ASSERT);
  return inst->data1;
}

RE_INTERNAL int
re__prog_inst_equals(const re__prog_inst* a, const re__prog_inst* b)
{
  int eq = 1;
  if (re__prog_inst_get_type(a) != re__prog_inst_get_type(b)) {
    return 0;
  }
  if (re__prog_inst_get_type(a) == RE__PROG_INST_TYPE_BYTE) {
    eq &= (re__prog_inst_get_byte(a) == re__prog_inst_get_byte(b));
    eq &= (re__prog_inst_get_primary(a) == re__prog_inst_get_primary(b));
  } else if (re__prog_inst_get_type(a) == RE__PROG_INST_TYPE_BYTE_RANGE) {
    eq &= (re__prog_inst_get_byte_min(a) == re__prog_inst_get_byte_min(b));
    eq &= (re__prog_inst_get_byte_max(a) == re__prog_inst_get_byte_max(b));
    eq &= (re__prog_inst_get_primary(a) == re__prog_inst_get_primary(b));
  } else if (re__prog_inst_get_type(a) == RE__PROG_INST_TYPE_SPLIT) {
    eq &= (re__prog_inst_get_primary(a) == re__prog_inst_get_primary(b));
    eq &=
        (re__prog_inst_get_split_secondary(a) ==
         re__prog_inst_get_split_secondary(b));
  } else if (re__prog_inst_get_type(a) == RE__PROG_INST_TYPE_MATCH) {
    eq &= (re__prog_inst_get_match_idx(a) == re__prog_inst_get_match_idx(b));
  } else if (re__prog_inst_get_type(a) == RE__PROG_INST_TYPE_SAVE) {
    eq &= (re__prog_inst_get_save_idx(a) == re__prog_inst_get_save_idx(b));
    eq &= (re__prog_inst_get_primary(a) == re__prog_inst_get_primary(b));
  } else if (re__prog_inst_get_type(a) == RE__PROG_INST_TYPE_ASSERT) {
    eq &= (re__prog_inst_get_assert_ctx(a) == re__prog_inst_get_assert_ctx(b));
    eq &= (re__prog_inst_get_primary(a) == re__prog_inst_get_primary(b));
  }
  return eq;
}

RE__VEC_IMPL_FUNC(re__prog_inst, init)
RE__VEC_IMPL_FUNC(re__prog_inst, destroy)
RE__VEC_IMPL_FUNC(re__prog_inst, push)
RE__VEC_IMPL_FUNC(re__prog_inst, get)
RE__VEC_IMPL_FUNC(re__prog_inst, getref)
RE__VEC_IMPL_FUNC(re__prog_inst, getcref)
RE__VEC_IMPL_FUNC(re__prog_inst, set)
RE__VEC_IMPL_FUNC(re__prog_inst, size)

RE_INTERNAL void re__prog_init(re__prog* prog)
{
  re__prog_inst_vec_init(&prog->_instructions);
  {
    int i;
    for (i = 0; i < RE__PROG_ENTRY_MAX; i++) {
      prog->_entrypoints[i] = RE__PROG_LOC_INVALID;
    }
  }
}

RE_INTERNAL void re__prog_destroy(re__prog* prog)
{
  re__prog_inst_vec_destroy(&prog->_instructions);
}

RE_INTERNAL re__prog_loc re__prog_size(const re__prog* prog)
{
  return (re__prog_loc)re__prog_inst_vec_size(&prog->_instructions);
}

RE_INTERNAL re__prog_inst* re__prog_get(re__prog* prog, re__prog_loc loc)
{
  return re__prog_inst_vec_getref(&prog->_instructions, loc);
}

RE_INTERNAL const re__prog_inst*
re__prog_get_const(const re__prog* prog, re__prog_loc loc)
{
  return re__prog_inst_vec_getcref(&prog->_instructions, loc);
}

RE_INTERNAL re_error re__prog_add(re__prog* prog, re__prog_inst inst)
{
  if (re__prog_size(prog) == RE__PROG_SIZE_MAX) {
    return RE__ERROR_PROGMAX;
  } else {
    return re__prog_inst_vec_push(&prog->_instructions, inst);
  }
}

RE_INTERNAL void
re__prog_set_entry(re__prog* prog, re__prog_entry idx, re__prog_loc loc)
{
  RE_ASSERT(idx < RE__PROG_ENTRY_MAX);
  prog->_entrypoints[idx] = loc;
}

RE_INTERNAL re__prog_loc
re__prog_get_entry(const re__prog* prog, re__prog_entry idx)
{
  RE_ASSERT(idx < RE__PROG_ENTRY_MAX);
  return prog->_entrypoints[idx];
}

#if RE_DEBUG

#include <stdio.h>

RE_INTERNAL void re__prog_debug_dump(const re__prog* prog)
{
  re__prog_loc i;
  for (i = 0; i < re__prog_size(prog); i++) {
    const re__prog_inst* inst = re__prog_get_const(prog, i);
    printf("%04X | ", i);
    switch (re__prog_inst_get_type(inst)) {
    case RE__PROG_INST_TYPE_BYTE:
      printf(
          "BYTE v=%0X ('%c')", re__prog_inst_get_byte(inst),
          re__prog_inst_get_byte(inst));
      break;
    case RE__PROG_INST_TYPE_BYTE_RANGE:
      printf(
          "BYTE_RANGE min=%X ('%c') max=%X ('%c')",
          re__prog_inst_get_byte_min(inst), re__prog_inst_get_byte_min(inst),
          re__prog_inst_get_byte_max(inst), re__prog_inst_get_byte_max(inst));
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
    default:
      RE__ASSERT_UNREACHED();
      break;
    }
    printf(" -> %04X", re__prog_inst_get_primary(inst));
    if (re__prog_inst_get_type(inst) == RE__PROG_INST_TYPE_SPLIT) {
      printf(", %04X", re__prog_inst_get_split_secondary(inst));
    }
    printf("\n");
  }
}

#endif

/* re */
/* python brute_force_utf8_compiler.py fwd 0-10FFFF */
RE_INTERNAL_DATA re_uint8 re__prog_data_dot_fwd_accsurr_accnl[] = {
    /* 0001 */ 0x02, 0x02, 0x03,       /* SPLIT           -> 0x02, 0x03 */
    /* 0002 */ 0x01, 0x00, 0x7F, 0x00, /* RANGE 0x00-0x7F -> out */
    /* 0003 */ 0x02, 0x04, 0x06,       /* SPLIT           -> 0x04, 0x06 */
    /* 0004 */ 0x01, 0xC2, 0xDF, 0x05, /* RANGE 0xC2-0xDF -> 0x05 */
    /* 0005 */ 0x01, 0x80, 0xBF, 0x00, /* RANGE 0x80-0xBF -> out */
    /* 0006 */ 0x02, 0x07, 0x09,       /* SPLIT           -> 0x07, 0x09 */
    /* 0007 */ 0x00, 0xE0, 0x08,       /* BYTE  0xE0      -> 0x08 */
    /* 0008 */ 0x01, 0xA0, 0xBF, 0x05, /* RANGE 0xA0-0xBF -> 0x05 */
    /* 0009 */ 0x02, 0x0A, 0x0C,       /* SPLIT           -> 0x0A, 0x0C */
    /* 000A */ 0x01, 0xE1, 0xEF, 0x0B, /* RANGE 0xE1-0xEF -> 0x0B */
    /* 000B */ 0x01, 0x80, 0xBF, 0x05, /* RANGE 0x80-0xBF -> 0x05 */
    /* 000C */ 0x02, 0x0D, 0x0F,       /* SPLIT           -> 0x0D, 0x0F */
    /* 000D */ 0x00, 0xF0, 0x0E,       /* BYTE  0xF0      -> 0x0E */
    /* 000E */ 0x01, 0x90, 0xBF, 0x0B, /* RANGE 0x90-0xBF -> 0x0B */
    /* 000F */ 0x02, 0x10, 0x12,       /* SPLIT           -> 0x10, 0x12 */
    /* 0010 */ 0x01, 0xF1, 0xF3, 0x11, /* RANGE 0xF1-0xF3 -> 0x11 */
    /* 0011 */ 0x01, 0x80, 0xBF, 0x0B, /* RANGE 0x80-0xBF -> 0x0B */
    /* 0012 */ 0x00, 0xF4, 0x13,       /* BYTE  0xF4      -> 0x13 */
    /* 0013 */ 0x01, 0x80, 0x8F, 0x0B, /* RANGE 0x80-0x8F -> 0x0B */
};

/* python brute_force_utf8_compiler.py rev 0-10FFFF */
RE_INTERNAL_DATA re_uint8 re__prog_data_dot_rev_accsurr_accnl[] = {
    /* 0001 */ 0x02, 0x02, 0x03,       /* SPLIT           -> 0x02, 0x03 */
    /* 0002 */ 0x01, 0x00, 0x7F, 0x00, /* RANGE 0x00-0x7F -> out */
    /* 0003 */ 0x01, 0x80, 0xBF, 0x04, /* RANGE 0x80-0xBF -> 0x04 */
    /* 0004 */ 0x02, 0x05, 0x0D,       /* SPLIT           -> 0x05, 0x0D */
    /* 0005 */ 0x01, 0x80, 0x9F, 0x06, /* RANGE 0x80-0x9F -> 0x06 */
    /* 0006 */ 0x02, 0x07, 0x09,       /* SPLIT           -> 0x07, 0x09 */
    /* 0007 */ 0x01, 0x80, 0x8F, 0x08, /* RANGE 0x80-0x8F -> 0x08 */
    /* 0008 */ 0x01, 0xF1, 0xF4, 0x00, /* RANGE 0xF1-0xF4 -> out */
    /* 0009 */ 0x02, 0x0A, 0x0C,       /* SPLIT           -> 0x0A, 0x0C */
    /* 000A */ 0x01, 0x90, 0xBF, 0x0B, /* RANGE 0x90-0xBF -> 0x0B */
    /* 000B */ 0x01, 0xF0, 0xF3, 0x00, /* RANGE 0xF0-0xF3 -> out */
    /* 000C */ 0x01, 0xE1, 0xEF, 0x00, /* RANGE 0xE1-0xEF -> out */
    /* 000D */ 0x02, 0x0E, 0x14,       /* SPLIT           -> 0x0E, 0x14 */
    /* 000E */ 0x01, 0xA0, 0xBF, 0x0F, /* RANGE 0xA0-0xBF -> 0x0F */
    /* 000F */ 0x02, 0x10, 0x11,       /* SPLIT           -> 0x10, 0x11 */
    /* 0010 */ 0x01, 0x80, 0x8F, 0x08, /* RANGE 0x80-0x8F -> 0x08 */
    /* 0011 */ 0x02, 0x12, 0x13,       /* SPLIT           -> 0x12, 0x13 */
    /* 0012 */ 0x01, 0x90, 0xBF, 0x0B, /* RANGE 0x90-0xBF -> 0x0B */
    /* 0013 */ 0x01, 0xE0, 0xEF, 0x00, /* RANGE 0xE0-0xEF -> out */
    /* 0014 */ 0x01, 0xC2, 0xDF, 0x00, /* RANGE 0xC2-0xDF -> out */
};

/* python brute_force_utf8_compiler.py fwd 0-D7FF E000-10FFFF */
RE_INTERNAL_DATA re_uint8 re__prog_data_dot_fwd_rejsurr_accnl[] = {
    /* 0001 */ 0x02, 0x02, 0x03,       /* SPLIT           -> 0x02, 0x03 */
    /* 0002 */ 0x01, 0x00, 0x7F, 0x00, /* RANGE 0x00-0x7F -> out */
    /* 0003 */ 0x02, 0x04, 0x06,       /* SPLIT           -> 0x04, 0x06 */
    /* 0004 */ 0x01, 0xC2, 0xDF, 0x05, /* RANGE 0xC2-0xDF -> 0x05 */
    /* 0005 */ 0x01, 0x80, 0xBF, 0x00, /* RANGE 0x80-0xBF -> out */
    /* 0006 */ 0x02, 0x07, 0x09,       /* SPLIT           -> 0x07, 0x09 */
    /* 0007 */ 0x00, 0xE0, 0x08,       /* BYTE  0xE0      -> 0x08 */
    /* 0008 */ 0x01, 0xA0, 0xBF, 0x05, /* RANGE 0xA0-0xBF -> 0x05 */
    /* 0009 */ 0x02, 0x0A, 0x0C,       /* SPLIT           -> 0x0A, 0x0C */
    /* 000A */ 0x01, 0xE1, 0xEC, 0x0B, /* RANGE 0xE1-0xEC -> 0x0B */
    /* 000B */ 0x01, 0x80, 0xBF, 0x05, /* RANGE 0x80-0xBF -> 0x05 */
    /* 000C */ 0x02, 0x0D, 0x0F,       /* SPLIT           -> 0x0D, 0x0F */
    /* 000D */ 0x00, 0xED, 0x0E,       /* BYTE  0xED      -> 0x0E */
    /* 000E */ 0x01, 0x80, 0x9F, 0x05, /* RANGE 0x80-0x9F -> 0x05 */
    /* 000F */ 0x02, 0x10, 0x11,       /* SPLIT           -> 0x10, 0x11 */
    /* 0010 */ 0x01, 0xEE, 0xEF, 0x0B, /* RANGE 0xEE-0xEF -> 0x0B */
    /* 0011 */ 0x02, 0x12, 0x14,       /* SPLIT           -> 0x12, 0x14 */
    /* 0012 */ 0x00, 0xF0, 0x13,       /* BYTE  0xF0      -> 0x13 */
    /* 0013 */ 0x01, 0x90, 0xBF, 0x0B, /* RANGE 0x90-0xBF -> 0x0B */
    /* 0014 */ 0x02, 0x15, 0x17,       /* SPLIT           -> 0x15, 0x17 */
    /* 0015 */ 0x01, 0xF1, 0xF3, 0x16, /* RANGE 0xF1-0xF3 -> 0x16 */
    /* 0016 */ 0x01, 0x80, 0xBF, 0x0B, /* RANGE 0x80-0xBF -> 0x0B */
    /* 0017 */ 0x00, 0xF4, 0x18,       /* BYTE  0xF4      -> 0x18 */
    /* 0018 */ 0x01, 0x80, 0x8F, 0x0B, /* RANGE 0x80-0x8F -> 0x0B */
};

/* python brute_force_utf8_compiler.py rev 0-D7FF E000-10FFFF */
RE_INTERNAL_DATA re_uint8 re__prog_data_dot_rev_rejsurr_accnl[] = {
    /* 0001 */ 0x02, 0x02, 0x03,       /* SPLIT           -> 0x02, 0x03 */
    /* 0002 */ 0x01, 0x00, 0x7F, 0x00, /* RANGE 0x00-0x7F -> out */
    /* 0003 */ 0x01, 0x80, 0xBF, 0x04, /* RANGE 0x80-0xBF -> 0x04 */
    /* 0004 */ 0x02, 0x05, 0x0D,       /* SPLIT           -> 0x05, 0x0D */
    /* 0005 */ 0x01, 0x80, 0x9F, 0x06, /* RANGE 0x80-0x9F -> 0x06 */
    /* 0006 */ 0x02, 0x07, 0x09,       /* SPLIT           -> 0x07, 0x09 */
    /* 0007 */ 0x01, 0x80, 0x8F, 0x08, /* RANGE 0x80-0x8F -> 0x08 */
    /* 0008 */ 0x01, 0xF1, 0xF4, 0x00, /* RANGE 0xF1-0xF4 -> out */
    /* 0009 */ 0x02, 0x0A, 0x0C,       /* SPLIT           -> 0x0A, 0x0C */
    /* 000A */ 0x01, 0x90, 0xBF, 0x0B, /* RANGE 0x90-0xBF -> 0x0B */
    /* 000B */ 0x01, 0xF0, 0xF3, 0x00, /* RANGE 0xF0-0xF3 -> out */
    /* 000C */ 0x01, 0xE1, 0xEF, 0x00, /* RANGE 0xE1-0xEF -> out */
    /* 000D */ 0x02, 0x0E, 0x16,       /* SPLIT           -> 0x0E, 0x16 */
    /* 000E */ 0x01, 0xA0, 0xBF, 0x0F, /* RANGE 0xA0-0xBF -> 0x0F */
    /* 000F */ 0x02, 0x10, 0x11,       /* SPLIT           -> 0x10, 0x11 */
    /* 0010 */ 0x01, 0x80, 0x8F, 0x08, /* RANGE 0x80-0x8F -> 0x08 */
    /* 0011 */ 0x02, 0x12, 0x13,       /* SPLIT           -> 0x12, 0x13 */
    /* 0012 */ 0x01, 0x90, 0xBF, 0x0B, /* RANGE 0x90-0xBF -> 0x0B */
    /* 0013 */ 0x02, 0x14, 0x15,       /* SPLIT           -> 0x14, 0x15 */
    /* 0014 */ 0x01, 0xE0, 0xEC, 0x00, /* RANGE 0xE0-0xEC -> out */
    /* 0015 */ 0x01, 0xEE, 0xEF, 0x00, /* RANGE 0xEE-0xEF -> out */
    /* 0016 */ 0x01, 0xC2, 0xDF, 0x00, /* RANGE 0xC2-0xDF -> out */
};

/* python brute_force_utf8_compiler.py fwd 0-9 B-10FFFF */
RE_INTERNAL_DATA re_uint8 re__prog_data_dot_fwd_accsurr_rejnl[] = {
    /* 0001 */ 0x02, 0x02, 0x03,       /* SPLIT           -> 0x02, 0x03 */
    /* 0002 */ 0x01, 0x00, 0x09, 0x00, /* RANGE 0x00-0x09 -> out */
    /* 0003 */ 0x02, 0x04, 0x05,       /* SPLIT           -> 0x04, 0x05 */
    /* 0004 */ 0x01, 0x0B, 0x7F, 0x00, /* RANGE 0x0B-0x7F -> out */
    /* 0005 */ 0x02, 0x06, 0x08,       /* SPLIT           -> 0x06, 0x08 */
    /* 0006 */ 0x01, 0xC2, 0xDF, 0x07, /* RANGE 0xC2-0xDF -> 0x07 */
    /* 0007 */ 0x01, 0x80, 0xBF, 0x00, /* RANGE 0x80-0xBF -> out */
    /* 0008 */ 0x02, 0x09, 0x0B,       /* SPLIT           -> 0x09, 0x0B */
    /* 0009 */ 0x00, 0xE0, 0x0A,       /* BYTE  0xE0      -> 0x0A */
    /* 000A */ 0x01, 0xA0, 0xBF, 0x07, /* RANGE 0xA0-0xBF -> 0x07 */
    /* 000B */ 0x02, 0x0C, 0x0E,       /* SPLIT           -> 0x0C, 0x0E */
    /* 000C */ 0x01, 0xE1, 0xEF, 0x0D, /* RANGE 0xE1-0xEF -> 0x0D */
    /* 000D */ 0x01, 0x80, 0xBF, 0x07, /* RANGE 0x80-0xBF -> 0x07 */
    /* 000E */ 0x02, 0x0F, 0x11,       /* SPLIT           -> 0x0F, 0x11 */
    /* 000F */ 0x00, 0xF0, 0x10,       /* BYTE  0xF0      -> 0x10 */
    /* 0010 */ 0x01, 0x90, 0xBF, 0x0D, /* RANGE 0x90-0xBF -> 0x0D */
    /* 0011 */ 0x02, 0x12, 0x14,       /* SPLIT           -> 0x12, 0x14 */
    /* 0012 */ 0x01, 0xF1, 0xF3, 0x13, /* RANGE 0xF1-0xF3 -> 0x13 */
    /* 0013 */ 0x01, 0x80, 0xBF, 0x0D, /* RANGE 0x80-0xBF -> 0x0D */
    /* 0014 */ 0x00, 0xF4, 0x15,       /* BYTE  0xF4      -> 0x15 */
    /* 0015 */ 0x01, 0x80, 0x8F, 0x0D, /* RANGE 0x80-0x8F -> 0x0D */
};

/* python brute_force_utf8_compiler.py rev 0-9 B-10FFFF */
RE_INTERNAL_DATA re_uint8 re__prog_data_dot_rev_accsurr_rejnl[] = {
    /* 0001 */ 0x02, 0x02, 0x03,       /* SPLIT           -> 0x02, 0x03 */
    /* 0002 */ 0x01, 0x00, 0x09, 0x00, /* RANGE 0x00-0x09 -> out */
    /* 0003 */ 0x02, 0x04, 0x05,       /* SPLIT           -> 0x04, 0x05 */
    /* 0004 */ 0x01, 0x0B, 0x7F, 0x00, /* RANGE 0x0B-0x7F -> out */
    /* 0005 */ 0x01, 0x80, 0xBF, 0x06, /* RANGE 0x80-0xBF -> 0x06 */
    /* 0006 */ 0x02, 0x07, 0x0F,       /* SPLIT           -> 0x07, 0x0F */
    /* 0007 */ 0x01, 0x80, 0x9F, 0x08, /* RANGE 0x80-0x9F -> 0x08 */
    /* 0008 */ 0x02, 0x09, 0x0B,       /* SPLIT           -> 0x09, 0x0B */
    /* 0009 */ 0x01, 0x80, 0x8F, 0x0A, /* RANGE 0x80-0x8F -> 0x0A */
    /* 000A */ 0x01, 0xF1, 0xF4, 0x00, /* RANGE 0xF1-0xF4 -> out */
    /* 000B */ 0x02, 0x0C, 0x0E,       /* SPLIT           -> 0x0C, 0x0E */
    /* 000C */ 0x01, 0x90, 0xBF, 0x0D, /* RANGE 0x90-0xBF -> 0x0D */
    /* 000D */ 0x01, 0xF0, 0xF3, 0x00, /* RANGE 0xF0-0xF3 -> out */
    /* 000E */ 0x01, 0xE1, 0xEF, 0x00, /* RANGE 0xE1-0xEF -> out */
    /* 000F */ 0x02, 0x10, 0x16,       /* SPLIT           -> 0x10, 0x16 */
    /* 0010 */ 0x01, 0xA0, 0xBF, 0x11, /* RANGE 0xA0-0xBF -> 0x11 */
    /* 0011 */ 0x02, 0x12, 0x13,       /* SPLIT           -> 0x12, 0x13 */
    /* 0012 */ 0x01, 0x80, 0x8F, 0x0A, /* RANGE 0x80-0x8F -> 0x0A */
    /* 0013 */ 0x02, 0x14, 0x15,       /* SPLIT           -> 0x14, 0x15 */
    /* 0014 */ 0x01, 0x90, 0xBF, 0x0D, /* RANGE 0x90-0xBF -> 0x0D */
    /* 0015 */ 0x01, 0xE0, 0xEF, 0x00, /* RANGE 0xE0-0xEF -> out */
    /* 0016 */ 0x01, 0xC2, 0xDF, 0x00, /* RANGE 0xC2-0xDF -> out */
};

/* python brute_force_utf8_compiler.py fwd 0-9 B-D7FF E000-10FFFF */
RE_INTERNAL_DATA re_uint8 re__prog_data_dot_fwd_rejsurr_rejnl[] = {
    /* 0001 */ 0x02, 0x02, 0x03,       /* SPLIT           -> 0x02, 0x03 */
    /* 0002 */ 0x01, 0x00, 0x09, 0x00, /* RANGE 0x00-0x09 -> out */
    /* 0003 */ 0x02, 0x04, 0x05,       /* SPLIT           -> 0x04, 0x05 */
    /* 0004 */ 0x01, 0x0B, 0x7F, 0x00, /* RANGE 0x0B-0x7F -> out */
    /* 0005 */ 0x02, 0x06, 0x08,       /* SPLIT           -> 0x06, 0x08 */
    /* 0006 */ 0x01, 0xC2, 0xDF, 0x07, /* RANGE 0xC2-0xDF -> 0x07 */
    /* 0007 */ 0x01, 0x80, 0xBF, 0x00, /* RANGE 0x80-0xBF -> out */
    /* 0008 */ 0x02, 0x09, 0x0B,       /* SPLIT           -> 0x09, 0x0B */
    /* 0009 */ 0x00, 0xE0, 0x0A,       /* BYTE  0xE0      -> 0x0A */
    /* 000A */ 0x01, 0xA0, 0xBF, 0x07, /* RANGE 0xA0-0xBF -> 0x07 */
    /* 000B */ 0x02, 0x0C, 0x0E,       /* SPLIT           -> 0x0C, 0x0E */
    /* 000C */ 0x01, 0xE1, 0xEC, 0x0D, /* RANGE 0xE1-0xEC -> 0x0D */
    /* 000D */ 0x01, 0x80, 0xBF, 0x07, /* RANGE 0x80-0xBF -> 0x07 */
    /* 000E */ 0x02, 0x0F, 0x11,       /* SPLIT           -> 0x0F, 0x11 */
    /* 000F */ 0x00, 0xED, 0x10,       /* BYTE  0xED      -> 0x10 */
    /* 0010 */ 0x01, 0x80, 0x9F, 0x07, /* RANGE 0x80-0x9F -> 0x07 */
    /* 0011 */ 0x02, 0x12, 0x13,       /* SPLIT           -> 0x12, 0x13 */
    /* 0012 */ 0x01, 0xEE, 0xEF, 0x0D, /* RANGE 0xEE-0xEF -> 0x0D */
    /* 0013 */ 0x02, 0x14, 0x16,       /* SPLIT           -> 0x14, 0x16 */
    /* 0014 */ 0x00, 0xF0, 0x15,       /* BYTE  0xF0      -> 0x15 */
    /* 0015 */ 0x01, 0x90, 0xBF, 0x0D, /* RANGE 0x90-0xBF -> 0x0D */
    /* 0016 */ 0x02, 0x17, 0x19,       /* SPLIT           -> 0x17, 0x19 */
    /* 0017 */ 0x01, 0xF1, 0xF3, 0x18, /* RANGE 0xF1-0xF3 -> 0x18 */
    /* 0018 */ 0x01, 0x80, 0xBF, 0x0D, /* RANGE 0x80-0xBF -> 0x0D */
    /* 0019 */ 0x00, 0xF4, 0x1A,       /* BYTE  0xF4      -> 0x1A */
    /* 001A */ 0x01, 0x80, 0x8F, 0x0D, /* RANGE 0x80-0x8F -> 0x0D */
};

/* python brute_force_utf8_compiler.py rev 0-9 B-D7FF E000-10FFFF */
RE_INTERNAL_DATA re_uint8 re__prog_data_dot_rev_rejsurr_rejnl[] = {
    /* 0001 */ 0x02, 0x02, 0x03,       /* SPLIT           -> 0x02, 0x03 */
    /* 0002 */ 0x01, 0x00, 0x09, 0x00, /* RANGE 0x00-0x09 -> out */
    /* 0003 */ 0x02, 0x04, 0x05,       /* SPLIT           -> 0x04, 0x05 */
    /* 0004 */ 0x01, 0x0B, 0x7F, 0x00, /* RANGE 0x0B-0x7F -> out */
    /* 0005 */ 0x01, 0x80, 0xBF, 0x06, /* RANGE 0x80-0xBF -> 0x06 */
    /* 0006 */ 0x02, 0x07, 0x0F,       /* SPLIT           -> 0x07, 0x0F */
    /* 0007 */ 0x01, 0x80, 0x9F, 0x08, /* RANGE 0x80-0x9F -> 0x08 */
    /* 0008 */ 0x02, 0x09, 0x0B,       /* SPLIT           -> 0x09, 0x0B */
    /* 0009 */ 0x01, 0x80, 0x8F, 0x0A, /* RANGE 0x80-0x8F -> 0x0A */
    /* 000A */ 0x01, 0xF1, 0xF4, 0x00, /* RANGE 0xF1-0xF4 -> out */
    /* 000B */ 0x02, 0x0C, 0x0E,       /* SPLIT           -> 0x0C, 0x0E */
    /* 000C */ 0x01, 0x90, 0xBF, 0x0D, /* RANGE 0x90-0xBF -> 0x0D */
    /* 000D */ 0x01, 0xF0, 0xF3, 0x00, /* RANGE 0xF0-0xF3 -> out */
    /* 000E */ 0x01, 0xE1, 0xEF, 0x00, /* RANGE 0xE1-0xEF -> out */
    /* 000F */ 0x02, 0x10, 0x18,       /* SPLIT           -> 0x10, 0x18 */
    /* 0010 */ 0x01, 0xA0, 0xBF, 0x11, /* RANGE 0xA0-0xBF -> 0x11 */
    /* 0011 */ 0x02, 0x12, 0x13,       /* SPLIT           -> 0x12, 0x13 */
    /* 0012 */ 0x01, 0x80, 0x8F, 0x0A, /* RANGE 0x80-0x8F -> 0x0A */
    /* 0013 */ 0x02, 0x14, 0x15,       /* SPLIT           -> 0x14, 0x15 */
    /* 0014 */ 0x01, 0x90, 0xBF, 0x0D, /* RANGE 0x90-0xBF -> 0x0D */
    /* 0015 */ 0x02, 0x16, 0x17,       /* SPLIT           -> 0x16, 0x17 */
    /* 0016 */ 0x01, 0xE0, 0xEC, 0x00, /* RANGE 0xE0-0xEC -> out */
    /* 0017 */ 0x01, 0xEE, 0xEF, 0x00, /* RANGE 0xEE-0xEF -> out */
    /* 0018 */ 0x01, 0xC2, 0xDF, 0x00, /* RANGE 0xC2-0xDF -> out */
};

RE_INTERNAL_DATA re_uint8* re__prog_data[RE__PROG_DATA_ID_MAX] = {
    re__prog_data_dot_fwd_accsurr_accnl, re__prog_data_dot_rev_accsurr_accnl,
    re__prog_data_dot_fwd_rejsurr_accnl, re__prog_data_dot_rev_rejsurr_accnl,
    re__prog_data_dot_fwd_accsurr_rejnl, re__prog_data_dot_rev_accsurr_rejnl,
    re__prog_data_dot_fwd_rejsurr_rejnl, re__prog_data_dot_rev_rejsurr_rejnl};

RE_INTERNAL_DATA re_size re__prog_data_size[RE__PROG_DATA_ID_MAX] = {
    sizeof(re__prog_data_dot_fwd_accsurr_accnl),
    sizeof(re__prog_data_dot_rev_accsurr_accnl),
    sizeof(re__prog_data_dot_fwd_rejsurr_accnl),
    sizeof(re__prog_data_dot_rev_rejsurr_accnl),
    sizeof(re__prog_data_dot_fwd_accsurr_rejnl),
    sizeof(re__prog_data_dot_rev_accsurr_rejnl),
    sizeof(re__prog_data_dot_fwd_rejsurr_rejnl),
    sizeof(re__prog_data_dot_rev_rejsurr_rejnl)};

RE_INTERNAL re_error re__prog_data_decompress_read_loc(
    re_uint8* compressed, re_size compressed_size, re_size* ptr,
    re__prog_loc* out_loc)
{
  re__prog_loc loc = 0;
  re_uint8 byte;
  int len = 0;
  if (compressed_size == *ptr) {
    return RE__ERROR_COMPRESSION_FORMAT;
  }
  byte = compressed[*ptr];
  (*ptr)++;
  loc = byte & 0x7F;
  len++;
  while ((byte & 0x80) == 0x80) {
    if (compressed_size == *ptr) {
      return RE__ERROR_COMPRESSION_FORMAT;
    }
    byte = compressed[*ptr];
    (*ptr)++;
    loc <<= 7;
    loc |= byte & 0x7F;
    len++;
    if (len == 3) {
      return RE__ERROR_COMPRESSION_FORMAT;
    }
  }
  *out_loc = loc;
  return RE_ERROR_NONE;
}

RE_INTERNAL re_error re__prog_data_decompress(
    re__prog* prog, re_uint8* compressed, re_size compressed_size,
    re__compile_patches* patches)
{
  re_size ptr = 0;
  re_error err;
  re__prog_inst inst;
  re__prog_loc offset = re__prog_size(prog) - 1;
  while (1) {
    re_uint8 inst_type;
    if (ptr == compressed_size) {
      break;
    }
    inst_type = compressed[ptr++];
    if (inst_type == 0) { /* BYTE */
      re_uint8 byte_val;
      re__prog_loc primary;
      if (ptr == compressed_size) {
        return RE__ERROR_COMPRESSION_FORMAT;
      }
      byte_val = compressed[ptr++];
      if ((err = re__prog_data_decompress_read_loc(
               compressed, compressed_size, &ptr, &primary))) {
        return err;
      }
      re__prog_inst_init_byte(&inst, byte_val);
      if (primary == 0) {
        re__compile_patches_append(patches, prog, re__prog_size(prog), 0);
      } else {
        re__prog_inst_set_primary(&inst, primary + offset);
      }
    } else if (inst_type == 1) { /* RANGE */
      re__byte_range range;
      re__prog_loc primary;
      if (ptr == compressed_size) {
        return RE__ERROR_COMPRESSION_FORMAT;
      }
      range.min = compressed[ptr++];
      if (ptr == compressed_size) {
        return RE__ERROR_COMPRESSION_FORMAT;
      }
      range.max = compressed[ptr++];
      if ((err = re__prog_data_decompress_read_loc(
               compressed, compressed_size, &ptr, &primary))) {
        return err;
      }
      re__prog_inst_init_byte_range(&inst, range);
      if (primary == 0) {
        re__compile_patches_append(patches, prog, re__prog_size(prog), 0);
      } else {
        re__prog_inst_set_primary(&inst, primary + offset);
      }
    } else if (inst_type == 2) { /* SPLIT */
      re__prog_loc primary;
      re__prog_loc secondary;
      if ((err = re__prog_data_decompress_read_loc(
               compressed, compressed_size, &ptr, &primary))) {
        return err;
      }
      if ((err = re__prog_data_decompress_read_loc(
               compressed, compressed_size, &ptr, &secondary))) {
        return err;
      }
      re__prog_inst_init_split(&inst, 0, 0);
      if (primary == 0) {
        re__compile_patches_append(patches, prog, re__prog_size(prog), 0);
      } else {
        re__prog_inst_set_primary(&inst, primary + offset);
      }
      if (secondary == 0) {
        re__compile_patches_append(patches, prog, re__prog_size(prog), 1);
      } else {
        re__prog_inst_set_split_secondary(&inst, secondary + offset);
      }
    } else {
      return RE__ERROR_COMPRESSION_FORMAT;
    }
    if ((err = re__prog_add(prog, inst))) {
      return err;
    }
  }
  return RE_ERROR_NONE;
}

/* re */
int re__byte_range_equals(re__byte_range range, re__byte_range other)
{
  return range.min == other.min && range.max == other.max;
}

int re__byte_range_adjacent(re__byte_range range, re__byte_range other)
{
  return ((re_uint32)other.min) == ((re_uint32)range.max) + 1;
}

int re__byte_range_intersects(re__byte_range range, re__byte_range clip)
{
  return range.min <= clip.max && clip.min <= range.max;
}

re__byte_range
re__byte_range_intersection(re__byte_range range, re__byte_range clip)
{
  re__byte_range out;
  RE_ASSERT(re__byte_range_intersects(range, clip));
  out.min = RE__MAX(range.min, clip.min);
  out.max = RE__MIN(range.max, clip.max);
  return out;
}

re__byte_range re__byte_range_merge(re__byte_range range, re__byte_range other)
{
  re__byte_range out;
  RE_ASSERT(re__byte_range_adjacent(range, other));
  out.min = range.min;
  out.max = other.max;
  return out;
}

int re__rune_range_equals(re__rune_range range, re__rune_range other)
{
  return range.min == other.min && range.max == other.max;
}

int re__rune_range_intersects(re__rune_range range, re__rune_range clip)
{
  return range.min <= clip.max && clip.min <= range.max;
}

re__rune_range re__rune_range_clamp(re__rune_range range, re__rune_range bounds)
{
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

#if RE_DEBUG

#include <stdio.h>

void re__byte_debug_dump(re_uint8 byte)
{
  if ((byte >= ' ') && (byte < 0x7F)) {
    printf("0x%02X ('%c')", byte, byte);
  } else {
    printf("0x%02X", byte);
  }
}

void re__rune_debug_dump(re_rune rune)
{
  re_uint8 utf_buf[5];
  int nchar;
  if ((rune >= ' ') && (rune < 0x7F)) {
    printf("0x%02X ('%c')", rune, rune);
  } else {
    printf("0x%02X", rune);
  }
  nchar = re__compile_gen_utf8(rune, utf_buf);
  if (nchar != 0) {
    int i;
    printf(" {");
    for (i = 0; i < nchar; i++) {
      printf("%02X", utf_buf[i]);
    }
    printf("}");
  }
}

void re__byte_range_debug_dump(re__byte_range br)
{
  printf("[");
  re__byte_debug_dump(br.min);
  printf(" - ");
  re__byte_debug_dump(br.max);
  printf("]");
}

void re__rune_range_debug_dump(re__rune_range rr)
{
  printf("[");
  re__rune_debug_dump(rr.min);
  printf(" - ");
  re__rune_debug_dump(rr.max);
  printf("]");
}

#endif

/* re */
RE_INTERNAL void re__rune_data_init(re__rune_data* rune_data)
{
  int i;
  for (i = 0; i < RE__RUNE_DATA_MAX_FOLD_CLASS; i++) {
    rune_data->fold_storage[i] = 0;
  }
  rune_data->properties_head = RE_NULL;
}

RE_INTERNAL void re__rune_data_destroy(re__rune_data* rune_data)
{
  {
    re__rune_data_prop* prop = rune_data->properties_head;
    while (prop) {
      re__rune_data_prop* next = prop->next;
      RE_FREE(prop->ranges);
      RE_FREE(prop);
      prop = next;
    }
  }
}

RE_INTERNAL re_rune re__rune_data_casefold_next(re_rune r);

RE_INTERNAL int
re__rune_data_casefold(re__rune_data* rune_data, re_rune ch, re_rune** runes)
{
  re_rune current = ch;
  int runes_size = 0;
  RE_ASSERT(ch <= RE_RUNE_MAX);
  do {
    rune_data->fold_storage[runes_size++] = current;
    current += re__rune_data_casefold_next(current);
  } while (current != ch);
  if (runes != RE_NULL) {
    *runes = rune_data->fold_storage;
  }
  return runes_size;
}

typedef struct re__rune_data_prop_compressed {
  const char* prop_name;
  re_uint32 offset;
} re__rune_data_prop_compressed;

RE_INTERNAL_DATA re__rune_data_prop_compressed re__rune_data_prop_refs[];
RE_INTERNAL_DATA re_uint8 re__rune_data_prop_bytes[];

RE_INTERNAL re_uint32 re__rune_data_prop_decomp_uleb128(re_uint8** read_ptr)
{
  re_uint32 val = 0;
  do {
    val <<= 7;
    val |= (**read_ptr & 0x7F);
    if (!(**read_ptr & 0x80)) {
      break;
    }
    (*read_ptr)++;
  } while (1);
  (*read_ptr)++;
  return val;
}

RE_INTERNAL re_error re__rune_data_prop_decomp(
    re__rune_data* rune_data, re__rune_data_prop_compressed* prop)
{
  re_uint8* read_ptr = re__rune_data_prop_bytes + prop->offset;
  re_uint32 len = re__rune_data_prop_decomp_uleb128(&read_ptr);
  re_uint32 i;
  re__rune_range* ranges = RE_NULL;
  re__rune_data_prop* prop_out = RE_NULL;
  re_rune run = 0;
  ranges = RE_MALLOC(sizeof(re__rune_range) * len);
  if (ranges == RE_NULL) {
    return RE_ERROR_NOMEM;
  }
  for (i = 0; i < len; i++) {
    run += re__rune_data_prop_decomp_uleb128(&read_ptr);
    ranges[i].min = run;
    run += re__rune_data_prop_decomp_uleb128(&read_ptr);
    ranges[i].max = run;
  }
  prop_out = RE_MALLOC(sizeof(re__rune_data_prop));
  if (prop_out == RE_NULL) {
    RE_FREE(ranges);
    return RE_ERROR_NOMEM;
  }
  prop_out->property_name = prop->prop_name;
  prop_out->ranges = ranges;
  prop_out->ranges_size = len;
  prop_out->next = rune_data->properties_head;
  rune_data->properties_head = prop_out;
  return RE_ERROR_NONE;
}

RE_INTERNAL re_error re__rune_data_get_property(
    re__rune_data* rune_data, const char* property_name,
    re_size property_name_size, re__rune_range** ranges_out,
    re_size* ranges_size_out)
{
  re__rune_data_prop* search = rune_data->properties_head;
  re_error err = RE_ERROR_NONE;
  re__str_view a, b;
  re__str_view_init_n(&a, property_name, property_name_size);
  while (search) {
    re__str_view_init_s(&b, search->property_name);
    if (re__str_view_cmp(&a, &b) == 0) {
      /* found property */
      *ranges_out = search->ranges;
      *ranges_size_out = search->ranges_size;
      return RE_ERROR_NONE;
    }
    search = search->next;
  }
  /* couldn't find property */
  {
    re__rune_data_prop_compressed* search_comp = re__rune_data_prop_refs;
    while (search_comp->prop_name != RE_NULL) {
      re__str_view_init_s(&b, search_comp->prop_name);
      if (re__str_view_cmp(&a, &b) == 0) {
        /* found property */
        if ((err = re__rune_data_prop_decomp(rune_data, search_comp))) {
          return err;
        }
        *ranges_out = rune_data->properties_head->ranges;
        *ranges_size_out = rune_data->properties_head->ranges_size;
        return err;
      }
      search_comp++;
    }
  }
  return RE_ERROR_INVALID;
}

/* Boolean property data is optimized for sequential reading. We don't typically
 * need to test whether a random character has a property (yet) but we need to
 * be able to enumerate through all ranges that a property has. */
/* We store each property as a set of rune ranges. This set is compressed using
 * the following technique (see tools/unicode_data.py):
 * 1. Store the number of ranges as ULEB128.
 * 2. Flatten ranges to get an array of integers where every even index has the
 *    start of the range and every odd index has the end of the range.
 * 3. Compute the deltas between each integer in this array.
 * 4. Encode these deltas as ULEB128. */
RE_INTERNAL_DATA re__rune_data_prop_compressed re__rune_data_prop_refs[] = {
    {"Cc", 0},    {"Cf", 5},    {"Co", 64},   {"Cs", 86},   {"Ll", 99},
    {"Lm", 1439}, {"Lo", 1614}, {"Lt", 2711}, {"Lu", 2734}, {"Mc", 4050},
    {"Me", 4436}, {"Mn", 4452}, {"Nd", 5178}, {"Nl", 5341}, {"No", 5376},
    {"Pc", 5559}, {"Pd", 5576}, {"Pe", 5625}, {"Pf", 5790}, {"Pi", 5814},
    {"Po", 5840}, {"Ps", 6268}, {"Sc", 6439}, {"Sk", 6500}, {"Sm", 6576},
    {"So", 6723}, {"Zl", 7160}, {"Zp", 7164}, {"Zs", 7168}, {RE_NULL, 0},
};

RE_INTERNAL_DATA re_uint8 re__rune_data_prop_bytes[] = {
    0x02, 0x00, 0x1F, 0x60, 0x20, 0x15, 0x01, 0xAD, 0x00, 0x0A, 0xD3, 0x05,
    0x17, 0x00, 0x01, 0xC1, 0x00, 0x32, 0x00, 0x03, 0x81, 0x01, 0x51, 0x00,
    0x1E, 0xAC, 0x00, 0x0F, 0xFD, 0x04, 0x1B, 0x04, 0x32, 0x04, 0x02, 0x09,
    0x03, 0xBD, 0x90, 0x00, 0x01, 0xFA, 0x02, 0x21, 0xC2, 0x00, 0x10, 0x00,
    0x46, 0xE3, 0x08, 0x02, 0x90, 0xE8, 0x03, 0x29, 0xD0, 0x07, 0x30, 0xDD,
    0x87, 0x00, 0x1F, 0x5F, 0x06, 0x03, 0xC0, 0x80, 0x00, 0x31, 0xFF, 0x00,
    0x38, 0x8E, 0x81, 0x00, 0x03, 0xFF, 0xFD, 0x00, 0x03, 0x00, 0x03, 0xFF,
    0xFD, 0x00, 0x04, 0x03, 0xB0, 0x80, 0x00, 0x06, 0xFF, 0x01, 0x7F, 0x01,
    0x07, 0xFF, 0x00, 0x05, 0x91, 0x61, 0x19, 0x3B, 0x00, 0x2A, 0x17, 0x02,
    0x07, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x01, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x01, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x03,
    0x00, 0x02, 0x00, 0x02, 0x02, 0x03, 0x00, 0x02, 0x00, 0x03, 0x00, 0x04,
    0x01, 0x05, 0x00, 0x03, 0x00, 0x04, 0x02, 0x03, 0x00, 0x03, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x03, 0x00, 0x02, 0x01, 0x02, 0x00, 0x03, 0x00, 0x04,
    0x00, 0x02, 0x00, 0x03, 0x01, 0x03, 0x02, 0x07, 0x00, 0x03, 0x00, 0x03,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x01, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x01, 0x03,
    0x00, 0x02, 0x00, 0x04, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x06, 0x03, 0x00, 0x03, 0x01, 0x02, 0x00, 0x05, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x44, 0x02, 0x1A, 0x01, 0xC2, 0x00,
    0x02, 0x00, 0x04, 0x00, 0x04, 0x02, 0x13, 0x00, 0x1C, 0x22, 0x02, 0x01,
    0x04, 0x02, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x04, 0x02, 0x00, 0x03, 0x00, 0x03, 0x01, 0x34, 0x2F, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x0A, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x03, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x01, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x31, 0x28, 0x16, 0xC8, 0x2A, 0x03, 0x02, 0x05,
    0xF9, 0x05, 0x11, 0x83, 0x08, 0x78, 0x2B, 0x40, 0x0C, 0x02, 0x21, 0x67,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x08, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x08, 0x09, 0x05, 0x0B, 0x07, 0x09,
    0x07, 0x09, 0x05, 0x0B, 0x07, 0x09, 0x07, 0x09, 0x0D, 0x03, 0x07, 0x09,
    0x07, 0x09, 0x07, 0x09, 0x04, 0x02, 0x01, 0x07, 0x00, 0x04, 0x02, 0x02,
    0x01, 0x09, 0x03, 0x03, 0x01, 0x09, 0x07, 0x0B, 0x02, 0x02, 0x01, 0x02,
    0x93, 0x00, 0x04, 0x01, 0x04, 0x00, 0x1C, 0x00, 0x05, 0x00, 0x05, 0x00,
    0x03, 0x01, 0x09, 0x03, 0x05, 0x00, 0x36, 0x00, 0x15, 0xAC, 0x2F, 0x02,
    0x00, 0x04, 0x01, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x05, 0x00, 0x02,
    0x01, 0x02, 0x05, 0x06, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x01, 0x08, 0x00, 0x02, 0x00, 0x05,
    0x00, 0x0D, 0x25, 0x02, 0x00, 0x06, 0x00, 0x01, 0xF2, 0x94, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x14, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x01,
    0x88, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x02, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x07, 0x02, 0x00, 0x02, 0x00, 0x03, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x05, 0x00, 0x02, 0x00,
    0x03, 0x00, 0x02, 0x02, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x06, 0x00, 0x06, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x05, 0x00, 0x02, 0x00, 0x07, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x1D, 0x00, 0x04, 0x00,
    0x06, 0xB6, 0x2A, 0x06, 0x08, 0x08, 0x4F, 0x01, 0x9E, 0xC1, 0x06, 0x0D,
    0x04, 0x08, 0xAA, 0x19, 0x09, 0xCE, 0x27, 0x01, 0x89, 0x23, 0x01, 0x9C,
    0x0A, 0x02, 0x0E, 0x02, 0x06, 0x02, 0x01, 0x0E, 0x84, 0x32, 0x17, 0xCE,
    0x1F, 0x01, 0xAB, 0x81, 0x1F, 0x01, 0xCB, 0x9B, 0x19, 0x1B, 0x06, 0x02,
    0x11, 0x1B, 0x19, 0x1B, 0x03, 0x02, 0x00, 0x02, 0x06, 0x02, 0x0A, 0x1B,
    0x19, 0x1B, 0x19, 0x1B, 0x19, 0x1B, 0x19, 0x1B, 0x19, 0x1B, 0x19, 0x1B,
    0x19, 0x1B, 0x19, 0x1B, 0x1B, 0x1D, 0x18, 0x02, 0x05, 0x1B, 0x18, 0x02,
    0x05, 0x1B, 0x18, 0x02, 0x05, 0x1B, 0x18, 0x02, 0x05, 0x1B, 0x18, 0x02,
    0x05, 0x02, 0x00, 0x0E, 0xB5, 0x09, 0x02, 0x13, 0x14, 0x84, 0x21, 0x45,
    0x05, 0xB0, 0x11, 0x05, 0x0B, 0x0F, 0x04, 0x08, 0x00, 0x02, 0x00, 0x01,
    0x86, 0x00, 0x06, 0x00, 0x03, 0xDF, 0x00, 0x01, 0xE7, 0x00, 0x01, 0xA5,
    0x01, 0x02, 0x8E, 0x01, 0x05, 0x00, 0x20, 0x00, 0x0A, 0x00, 0x04, 0x00,
    0x01, 0xA1, 0x00, 0x01, 0xA8, 0x00, 0x09, 0xD5, 0x00, 0x01, 0x80, 0x00,
    0x04, 0xB6, 0x00, 0x0D, 0xDB, 0x00, 0x6C, 0x00, 0x04, 0xE4, 0x00, 0x03,
    0xD1, 0x05, 0x01, 0xAF, 0x3E, 0x0E, 0x00, 0x23, 0x24, 0x05, 0xB2, 0x00,
    0x0E, 0x00, 0x11, 0x0C, 0x17, 0xE0, 0x01, 0x01, 0xF2, 0x00, 0x01, 0xC0,
    0x00, 0x03, 0xD6, 0x00, 0x2C, 0x04, 0x06, 0x00, 0x62, 0x01, 0x5E, 0x02,
    0x01, 0xDE, 0x97, 0x00, 0x09, 0xE3, 0x05, 0x02, 0x8F, 0x00, 0x73, 0x00,
    0x1D, 0x01, 0x7A, 0x08, 0x51, 0x00, 0x18, 0x00, 0x6A, 0x02, 0x04, 0x01,
    0x03, 0xD6, 0x00, 0x17, 0x00, 0x01, 0x8A, 0x00, 0x6D, 0x00, 0x16, 0x01,
    0x68, 0x03, 0x0A, 0x00, 0x01, 0xA8, 0x87, 0x00, 0x2E, 0x01, 0x0F, 0xE1,
    0x05, 0x02, 0x29, 0x02, 0x08, 0x01, 0xC7, 0x86, 0x03, 0x08, 0xD0, 0x0C,
    0x41, 0x01, 0x02, 0x00, 0x01, 0x80, 0x8D, 0x03, 0x02, 0x06, 0x02, 0x01,
    0x62, 0xB9, 0x06, 0x10, 0x8E, 0x00, 0x04, 0x80, 0x01, 0xAA, 0x00, 0x10,
    0x00, 0x02, 0x81, 0x00, 0x05, 0x03, 0x01, 0xD1, 0x00, 0x06, 0xBC, 0x1A,
    0x05, 0x03, 0x2E, 0x1F, 0x02, 0x09, 0x24, 0x01, 0x02, 0x62, 0x02, 0x00,
    0x19, 0x01, 0x0B, 0x02, 0x03, 0x00, 0x11, 0x00, 0x02, 0x1D, 0x1E, 0x58,
    0x0C, 0x00, 0x19, 0x20, 0x16, 0x15, 0x2B, 0x18, 0x08, 0x0A, 0x06, 0x17,
    0x02, 0x05, 0x12, 0x28, 0x3C, 0x35, 0x04, 0x00, 0x13, 0x00, 0x08, 0x09,
    0x11, 0x0E, 0x05, 0x07, 0x03, 0x01, 0x03, 0x15, 0x02, 0x06, 0x02, 0x00,
    0x04, 0x03, 0x04, 0x00, 0x11, 0x00, 0x0E, 0x01, 0x02, 0x02, 0x0F, 0x01,
    0x0B, 0x00, 0x09, 0x05, 0x05, 0x01, 0x03, 0x15, 0x02, 0x06, 0x02, 0x01,
    0x02, 0x01, 0x02, 0x01, 0x20, 0x03, 0x02, 0x00, 0x14, 0x02, 0x11, 0x08,
    0x02, 0x02, 0x02, 0x15, 0x02, 0x06, 0x02, 0x01, 0x02, 0x04, 0x04, 0x00,
    0x13, 0x00, 0x10, 0x01, 0x18, 0x00, 0x0C, 0x07, 0x03, 0x01, 0x03, 0x15,
    0x02, 0x06, 0x02, 0x01, 0x02, 0x04, 0x04, 0x00, 0x1F, 0x01, 0x02, 0x02,
    0x10, 0x00, 0x12, 0x00, 0x02, 0x05, 0x04, 0x02, 0x02, 0x03, 0x04, 0x01,
    0x02, 0x00, 0x02, 0x01, 0x04, 0x01, 0x04, 0x02, 0x04, 0x0B, 0x17, 0x00,
    0x35, 0x07, 0x02, 0x02, 0x02, 0x16, 0x02, 0x0F, 0x04, 0x00, 0x1B, 0x02,
    0x03, 0x00, 0x03, 0x01, 0x1F, 0x00, 0x05, 0x07, 0x02, 0x02, 0x02, 0x16,
    0x02, 0x09, 0x02, 0x04, 0x04, 0x00, 0x20, 0x01, 0x02, 0x01, 0x10, 0x01,
    0x12, 0x08, 0x02, 0x02, 0x02, 0x28, 0x03, 0x00, 0x11, 0x00, 0x06, 0x02,
    0x09, 0x02, 0x19, 0x05, 0x06, 0x11, 0x04, 0x17, 0x02, 0x08, 0x02, 0x00,
    0x03, 0x06, 0x3B, 0x2F, 0x02, 0x01, 0x0D, 0x05, 0x3C, 0x01, 0x02, 0x00,
    0x02, 0x04, 0x02, 0x17, 0x02, 0x00, 0x02, 0x09, 0x02, 0x01, 0x0A, 0x00,
    0x03, 0x04, 0x18, 0x03, 0x21, 0x00, 0x40, 0x07, 0x02, 0x23, 0x1C, 0x04,
    0x74, 0x2A, 0x15, 0x00, 0x11, 0x05, 0x05, 0x03, 0x04, 0x00, 0x04, 0x01,
    0x08, 0x02, 0x05, 0x0C, 0x0D, 0x00, 0x72, 0x02, 0xC8, 0x02, 0x03, 0x03,
    0x06, 0x02, 0x00, 0x02, 0x03, 0x03, 0x28, 0x02, 0x03, 0x03, 0x20, 0x02,
    0x03, 0x03, 0x06, 0x02, 0x00, 0x02, 0x03, 0x03, 0x0E, 0x02, 0x38, 0x02,
    0x03, 0x03, 0x42, 0x26, 0x0F, 0x72, 0x04, 0xEB, 0x03, 0x10, 0x02, 0x19,
    0x06, 0x4A, 0x07, 0x07, 0x08, 0x11, 0x0E, 0x12, 0x0F, 0x11, 0x0F, 0x0C,
    0x02, 0x02, 0x10, 0x33, 0x29, 0x00, 0x44, 0x22, 0x02, 0x34, 0x08, 0x04,
    0x03, 0x21, 0x02, 0x00, 0x06, 0x45, 0x0B, 0x1E, 0x32, 0x1D, 0x03, 0x04,
    0x0C, 0x2B, 0x05, 0x19, 0x37, 0x16, 0x0A, 0x34, 0x01, 0xB1, 0x2E, 0x12,
    0x07, 0x37, 0x1D, 0x0E, 0x01, 0x0B, 0x2B, 0x1B, 0x23, 0x2A, 0x02, 0x0B,
    0x1D, 0x72, 0x03, 0x02, 0x05, 0x02, 0x01, 0x04, 0x00, 0x08, 0xBB, 0x03,
    0x17, 0xF8, 0x37, 0x19, 0x16, 0x0A, 0x06, 0x02, 0x06, 0x02, 0x06, 0x02,
    0x06, 0x02, 0x06, 0x02, 0x06, 0x02, 0x06, 0x02, 0x06, 0x04, 0xA8, 0x00,
    0x36, 0x00, 0x05, 0x55, 0x09, 0x00, 0x02, 0x59, 0x05, 0x00, 0x06, 0x2A,
    0x02, 0x5D, 0x12, 0x1F, 0x31, 0x0F, 0x04, 0x81, 0x00, 0x33, 0xBF, 0x00,
    0x41, 0x00, 0x01, 0xA3, 0xFF, 0x15, 0x02, 0x08, 0xF6, 0x44, 0x27, 0x09,
    0x02, 0x8B, 0x05, 0x0F, 0x0B, 0x01, 0x43, 0x00, 0x32, 0x45, 0x01, 0xAA,
    0x00, 0x68, 0x00, 0x04, 0x06, 0x02, 0x02, 0x02, 0x03, 0x02, 0x16, 0x1E,
    0x33, 0x0F, 0x31, 0x3F, 0x05, 0x04, 0x00, 0x02, 0x01, 0x0C, 0x1B, 0x0B,
    0x16, 0x1A, 0x1C, 0x08, 0x2E, 0x2E, 0x04, 0x03, 0x08, 0x0B, 0x04, 0x02,
    0x28, 0x18, 0x02, 0x02, 0x07, 0x15, 0x0F, 0x02, 0x05, 0x04, 0x00, 0x04,
    0x31, 0x02, 0x00, 0x04, 0x01, 0x03, 0x04, 0x03, 0x00, 0x02, 0x00, 0x19,
    0x01, 0x04, 0x0A, 0x08, 0x00, 0x0F, 0x05, 0x03, 0x05, 0x03, 0x05, 0x0A,
    0x06, 0x02, 0x06, 0x01, 0x92, 0x22, 0x1E, 0x00, 0x57, 0xA3, 0x00, 0x0D,
    0x16, 0x05, 0x30, 0x42, 0x85, 0x02, 0xED, 0x03, 0x69, 0x44, 0x00, 0x02,
    0x09, 0x02, 0x0C, 0x02, 0x04, 0x02, 0x00, 0x02, 0x01, 0x02, 0x01, 0x02,
    0x6B, 0x22, 0x02, 0xEA, 0x13, 0x3F, 0x03, 0x35, 0x29, 0x0B, 0x75, 0x04,
    0x02, 0x01, 0x86, 0x6A, 0x09, 0x02, 0x2C, 0x03, 0x1E, 0x04, 0x05, 0x03,
    0x05, 0x03, 0x05, 0x03, 0x02, 0x24, 0x0B, 0x02, 0x19, 0x02, 0x12, 0x02,
    0x01, 0x02, 0x0E, 0x03, 0x0D, 0x23, 0x7A, 0x03, 0x86, 0x1C, 0x04, 0x30,
    0x30, 0x1F, 0x0E, 0x13, 0x02, 0x07, 0x07, 0x25, 0x0B, 0x1D, 0x03, 0x23,
    0x05, 0x07, 0x01, 0x81, 0x4D, 0x63, 0x27, 0x09, 0x33, 0x01, 0x9D, 0x02,
    0xB6, 0x0A, 0x15, 0x0B, 0x07, 0x01, 0x99, 0x05, 0x03, 0x00, 0x02, 0x2B,
    0x02, 0x01, 0x04, 0x00, 0x03, 0x16, 0x0B, 0x16, 0x0A, 0x1E, 0x42, 0x12,
    0x02, 0x01, 0x0B, 0x15, 0x0B, 0x19, 0x47, 0x37, 0x07, 0x01, 0x41, 0x00,
    0x10, 0x03, 0x02, 0x02, 0x02, 0x1C, 0x2B, 0x1C, 0x04, 0x1C, 0x24, 0x07,
    0x02, 0x1B, 0x1C, 0x35, 0x0B, 0x15, 0x0B, 0x12, 0x0E, 0x11, 0x6F, 0x48,
    0x01, 0xB8, 0x23, 0x02, 0xDD, 0x29, 0x07, 0x01, 0x4F, 0x1C, 0x0B, 0x00,
    0x09, 0x15, 0x2B, 0x11, 0x2F, 0x14, 0x1C, 0x16, 0x0D, 0x34, 0x3A, 0x01,
    0x03, 0x00, 0x0E, 0x2C, 0x21, 0x18, 0x1B, 0x23, 0x1E, 0x00, 0x03, 0x00,
    0x09, 0x22, 0x04, 0x00, 0x0D, 0x2F, 0x0F, 0x03, 0x16, 0x00, 0x02, 0x00,
    0x24, 0x11, 0x02, 0x18, 0x55, 0x06, 0x02, 0x00, 0x02, 0x03, 0x02, 0x0E,
    0x02, 0x09, 0x08, 0x2E, 0x27, 0x07, 0x03, 0x01, 0x03, 0x15, 0x02, 0x06,
    0x02, 0x01, 0x02, 0x04, 0x04, 0x00, 0x13, 0x00, 0x0D, 0x04, 0x01, 0x9F,
    0x34, 0x13, 0x03, 0x15, 0x02, 0x1F, 0x2F, 0x15, 0x01, 0x02, 0x00, 0x01,
    0xB9, 0x2E, 0x2A, 0x03, 0x25, 0x2F, 0x15, 0x00, 0x3C, 0x2A, 0x0E, 0x00,
    0x48, 0x1A, 0x26, 0x06, 0x01, 0xBA, 0x2B, 0x01, 0xD4, 0x07, 0x03, 0x00,
    0x03, 0x07, 0x02, 0x01, 0x02, 0x17, 0x10, 0x00, 0x02, 0x00, 0x5F, 0x07,
    0x03, 0x26, 0x11, 0x00, 0x02, 0x00, 0x1D, 0x00, 0x0B, 0x27, 0x08, 0x00,
    0x16, 0x00, 0x0C, 0x2D, 0x14, 0x00, 0x13, 0x48, 0x02, 0x88, 0x08, 0x02,
    0x24, 0x12, 0x00, 0x32, 0x1D, 0x71, 0x06, 0x02, 0x01, 0x02, 0x25, 0x16,
    0x00, 0x1A, 0x05, 0x02, 0x01, 0x02, 0x1F, 0x0F, 0x00, 0x02, 0xC8, 0x12,
    0x01, 0xBE, 0x00, 0x50, 0x07, 0x99, 0x01, 0xE7, 0x01, 0xC3, 0x14, 0xCD,
    0x60, 0x10, 0x08, 0xAE, 0x1F, 0xD2, 0x04, 0xC6, 0x43, 0xBA, 0x04, 0xB8,
    0x08, 0x1E, 0x12, 0x4E, 0x12, 0x1D, 0x13, 0x2F, 0x34, 0x14, 0x06, 0x12,
    0x06, 0xF1, 0x4A, 0x06, 0x00, 0x01, 0xB0, 0x00, 0x2F, 0xF7, 0x00, 0x09,
    0x09, 0xD5, 0x2B, 0x00, 0x08, 0x00, 0x45, 0xF8, 0x02, 0xA2, 0x2E, 0x02,
    0x12, 0x03, 0x09, 0x03, 0x8B, 0x12, 0x85, 0x6A, 0x06, 0x0C, 0x04, 0x08,
    0x08, 0x09, 0x44, 0xF1, 0x00, 0x03, 0xF6, 0x2C, 0x22, 0x00, 0x02, 0xC2,
    0x1D, 0x13, 0x2B, 0x09, 0xF5, 0x06, 0x02, 0x03, 0x02, 0x01, 0x02, 0x0E,
    0x02, 0x01, 0xC4, 0x0A, 0xBC, 0x03, 0x02, 0x1A, 0x02, 0x01, 0x02, 0x00,
    0x03, 0x00, 0x02, 0x09, 0x02, 0x03, 0x02, 0x00, 0x02, 0x00, 0x07, 0x00,
    0x05, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x02, 0x02, 0x01, 0x02, 0x00,
    0x03, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x01,
    0x02, 0x00, 0x03, 0x03, 0x02, 0x06, 0x02, 0x03, 0x02, 0x03, 0x02, 0x00,
    0x02, 0x09, 0x02, 0x10, 0x06, 0x02, 0x02, 0x04, 0x02, 0x10, 0x22, 0xC5,
    0x00, 0x02, 0xCD, 0xDF, 0x00, 0x21, 0x00, 0x20, 0xB8, 0x00, 0x08, 0x00,
    0x01, 0xDD, 0x00, 0x03, 0x00, 0x2D, 0x81, 0x00, 0x0F, 0x00, 0x3A, 0xB0,
    0x00, 0x18, 0xA0, 0x04, 0x9D, 0x0B, 0xE3, 0x00, 0x26, 0xCA, 0x00, 0x0A,
    0x03, 0xC5, 0x00, 0x03, 0x00, 0x03, 0x00, 0x27, 0x00, 0x3B, 0x96, 0x07,
    0x09, 0x07, 0x09, 0x07, 0x0D, 0x00, 0x10, 0x00, 0x30, 0x00, 0x05, 0x86,
    0x41, 0x19, 0x66, 0x16, 0x02, 0x06, 0x22, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x03, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x03, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x01, 0x02, 0x00, 0x02, 0x00, 0x04, 0x01,
    0x02, 0x00, 0x02, 0x01, 0x02, 0x02, 0x03, 0x03, 0x02, 0x01, 0x02, 0x02,
    0x04, 0x01, 0x02, 0x01, 0x02, 0x00, 0x02, 0x00, 0x02, 0x01, 0x02, 0x00,
    0x03, 0x00, 0x02, 0x01, 0x02, 0x02, 0x02, 0x00, 0x02, 0x01, 0x04, 0x00,
    0x08, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x03, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x03, 0x00, 0x03, 0x00, 0x02, 0x02, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x08, 0x01, 0x02, 0x01,
    0x03, 0x00, 0x02, 0x03, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0xA2, 0x00, 0x02, 0x00, 0x04, 0x00, 0x09, 0x00, 0x07, 0x00, 0x02,
    0x02, 0x02, 0x00, 0x02, 0x01, 0x02, 0x10, 0x02, 0x08, 0x24, 0x00, 0x03,
    0x02, 0x04, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x06, 0x00, 0x03, 0x00, 0x02, 0x01, 0x03, 0x32, 0x31, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x0A, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x01, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x03, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x03, 0x25, 0x16, 0xCA, 0x25, 0x02, 0x00, 0x06, 0x00,
    0x05, 0xD3, 0x55, 0x11, 0x9B, 0x2A, 0x03, 0x02, 0x02, 0xC1, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x0A, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x0A, 0x07, 0x09, 0x05, 0x0B, 0x07, 0x09,
    0x07, 0x09, 0x05, 0x0C, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x09,
    0x07, 0x49, 0x03, 0x0D, 0x03, 0x0D, 0x03, 0x0D, 0x04, 0x0C, 0x03, 0x02,
    0x87, 0x00, 0x05, 0x00, 0x04, 0x02, 0x03, 0x02, 0x03, 0x00, 0x04, 0x04,
    0x07, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x03, 0x03, 0x03, 0x0B, 0x01,
    0x06, 0x00, 0x3E, 0x00, 0x14, 0xFD, 0x2F, 0x31, 0x00, 0x02, 0x02, 0x03,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x03, 0x02, 0x00, 0x03, 0x00, 0x09,
    0x02, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x09, 0x00, 0x02, 0x00, 0x05, 0x00, 0x01, 0xF2, 0xCE,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x14, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x01, 0x88, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x04, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x0B, 0x00, 0x02, 0x00, 0x02, 0x01,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x05, 0x00, 0x02, 0x00,
    0x03, 0x00, 0x02, 0x00, 0x04, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x04, 0x02, 0x04, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x03, 0x02, 0x00, 0x07, 0x00,
    0x06, 0x00, 0x02, 0x00, 0x1D, 0x00, 0x01, 0xAE, 0xAC, 0x19, 0x09, 0xC6,
    0x27, 0x01, 0x89, 0x23, 0x01, 0x9D, 0x0A, 0x02, 0x0E, 0x02, 0x06, 0x02,
    0x01, 0x0D, 0xEB, 0x32, 0x17, 0xEE, 0x1F, 0x01, 0xAB, 0x81, 0x1F, 0x01,
    0xCB, 0xA1, 0x19, 0x1B, 0x19, 0x1B, 0x19, 0x1B, 0x00, 0x02, 0x01, 0x03,
    0x00, 0x03, 0x01, 0x03, 0x03, 0x02, 0x07, 0x1B, 0x19, 0x1B, 0x01, 0x02,
    0x03, 0x03, 0x07, 0x02, 0x06, 0x1C, 0x01, 0x02, 0x03, 0x02, 0x04, 0x02,
    0x00, 0x04, 0x06, 0x1C, 0x19, 0x1B, 0x19, 0x1B, 0x19, 0x1B, 0x19, 0x1B,
    0x19, 0x1B, 0x19, 0x1F, 0x18, 0x22, 0x18, 0x22, 0x18, 0x22, 0x18, 0x22,
    0x18, 0x22, 0x00, 0x22, 0xB6, 0x21, 0x01, 0xB1, 0x12, 0x83, 0x00, 0x38,
    0x00, 0x03, 0x02, 0x09, 0x03, 0x02, 0x01, 0x33, 0x01, 0x3B, 0x02, 0x07,
    0x01, 0x03, 0x01, 0x0B, 0x00, 0x2C, 0x00, 0x3B, 0x02, 0x43, 0x00, 0x3B,
    0x02, 0x09, 0x00, 0x02, 0x01, 0x36, 0x01, 0x3B, 0x00, 0x02, 0x00, 0x07,
    0x01, 0x03, 0x01, 0x0B, 0x00, 0x67, 0x01, 0x02, 0x01, 0x04, 0x02, 0x02,
    0x02, 0x0B, 0x00, 0x2A, 0x02, 0x3E, 0x03, 0x3E, 0x01, 0x3B, 0x00, 0x02,
    0x04, 0x03, 0x01, 0x02, 0x01, 0x0A, 0x01, 0x2C, 0x01, 0x3B, 0x02, 0x06,
    0x02, 0x02, 0x02, 0x0B, 0x00, 0x2B, 0x01, 0x4C, 0x02, 0x07, 0x07, 0x13,
    0x01, 0x02, 0xCB, 0x01, 0x40, 0x00, 0x01, 0xAC, 0x01, 0x05, 0x00, 0x07,
    0x00, 0x03, 0x01, 0x1A, 0x01, 0x0B, 0x02, 0x03, 0x06, 0x16, 0x01, 0x03,
    0x05, 0x03, 0x00, 0x0B, 0x02, 0x0C, 0xF9, 0x00, 0x1F, 0x00, 0x01, 0x82,
    0x00, 0x08, 0x07, 0x02, 0x01, 0x02, 0xDB, 0x03, 0x03, 0x02, 0x05, 0x01,
    0x02, 0x05, 0x01, 0xE1, 0x01, 0x3B, 0x00, 0x02, 0x00, 0x0A, 0x00, 0x02,
    0x01, 0x09, 0x05, 0x01, 0x92, 0x00, 0x31, 0x00, 0x06, 0x00, 0x02, 0x04,
    0x02, 0x01, 0x3E, 0x00, 0x1F, 0x00, 0x05, 0x01, 0x03, 0x00, 0x3D, 0x00,
    0x03, 0x02, 0x02, 0x00, 0x04, 0x01, 0x31, 0x07, 0x09, 0x01, 0x01, 0xAC,
    0x00, 0x16, 0x00, 0x26, 0xB7, 0x01, 0x01, 0xEF, 0xF4, 0x01, 0x03, 0x00,
    0x59, 0x01, 0x33, 0x0F, 0x01, 0x8F, 0x01, 0x30, 0x00, 0x31, 0x01, 0x05,
    0x01, 0x03, 0x02, 0x6F, 0x01, 0x03, 0x01, 0x19, 0x00, 0x2E, 0x00, 0x02,
    0x00, 0x6E, 0x00, 0x03, 0x01, 0x06, 0x00, 0x01, 0xEE, 0x01, 0x02, 0x01,
    0x02, 0x01, 0x02, 0x00, 0x01, 0xC8, 0x94, 0x00, 0x02, 0x00, 0x01, 0x80,
    0x00, 0x2E, 0x02, 0x05, 0x01, 0x74, 0x00, 0x19, 0x01, 0x3C, 0x00, 0x31,
    0x02, 0x0A, 0x01, 0x0E, 0x00, 0x5E, 0x02, 0x04, 0x01, 0x02, 0x00, 0x01,
    0xAB, 0x02, 0x20, 0x01, 0x3B, 0x01, 0x02, 0x03, 0x03, 0x01, 0x03, 0x02,
    0x0A, 0x00, 0x0B, 0x01, 0x01, 0xD2, 0x02, 0x09, 0x01, 0x04, 0x00, 0x6B,
    0x02, 0x07, 0x00, 0x02, 0x03, 0x03, 0x00, 0x01, 0xEE, 0x02, 0x07, 0x03,
    0x03, 0x00, 0x72, 0x02, 0x09, 0x01, 0x02, 0x00, 0x6E, 0x00, 0x02, 0x01,
    0x07, 0x00, 0x6A, 0x01, 0x05, 0x00, 0x02, 0x86, 0x02, 0x0A, 0x00, 0x01,
    0xF8, 0x05, 0x02, 0x01, 0x05, 0x00, 0x03, 0x00, 0x02, 0x00, 0x01, 0x8F,
    0x02, 0x09, 0x03, 0x05, 0x00, 0x55, 0x00, 0x1E, 0x01, 0x3F, 0x00, 0x03,
    0x98, 0x00, 0x0F, 0x00, 0x6B, 0x00, 0x08, 0x00, 0x03, 0x00, 0x01, 0xD6,
    0x04, 0x05, 0x01, 0x02, 0x00, 0x02, 0xDF, 0x01, 0x01, 0xA0, 0xDB, 0x36,
    0x69, 0x01, 0x01, 0xC2, 0xF4, 0x01, 0x07, 0x05, 0x05, 0x09, 0x88, 0x01,
    0x2C, 0xB5, 0x00, 0x0C, 0x9F, 0x03, 0x02, 0x02, 0x02, 0x8B, 0x8C, 0x02,
    0x02, 0xD0, 0x06, 0x80, 0x6F, 0x02, 0x94, 0x04, 0x02, 0x8A, 0x2C, 0x02,
    0x00, 0x02, 0x01, 0x02, 0x01, 0x02, 0x00, 0x49, 0x0A, 0x31, 0x14, 0x11,
    0x00, 0x66, 0x06, 0x03, 0x05, 0x03, 0x01, 0x02, 0x03, 0x24, 0x00, 0x1F,
    0x1A, 0x5C, 0x0A, 0x3B, 0x08, 0x0A, 0x00, 0x19, 0x03, 0x02, 0x08, 0x02,
    0x02, 0x02, 0x04, 0x2C, 0x02, 0x3D, 0x07, 0x2B, 0x17, 0x02, 0x1F, 0x38,
    0x00, 0x02, 0x00, 0x05, 0x07, 0x05, 0x00, 0x04, 0x06, 0x0B, 0x01, 0x1E,
    0x00, 0x3B, 0x00, 0x05, 0x03, 0x09, 0x00, 0x15, 0x01, 0x1B, 0x00, 0x03,
    0x01, 0x3A, 0x00, 0x05, 0x01, 0x05, 0x01, 0x03, 0x02, 0x04, 0x00, 0x1F,
    0x01, 0x04, 0x00, 0x0C, 0x01, 0x3A, 0x00, 0x05, 0x04, 0x02, 0x01, 0x05,
    0x00, 0x15, 0x01, 0x17, 0x05, 0x02, 0x00, 0x3B, 0x00, 0x03, 0x00, 0x02,
    0x03, 0x09, 0x00, 0x08, 0x01, 0x0C, 0x01, 0x1F, 0x00, 0x3E, 0x00, 0x0D,
    0x00, 0x33, 0x00, 0x04, 0x00, 0x38, 0x00, 0x02, 0x02, 0x06, 0x02, 0x02,
    0x03, 0x08, 0x01, 0x0C, 0x01, 0x1E, 0x00, 0x3B, 0x00, 0x03, 0x00, 0x07,
    0x00, 0x06, 0x01, 0x15, 0x01, 0x1D, 0x01, 0x3A, 0x01, 0x05, 0x03, 0x09,
    0x00, 0x15, 0x01, 0x1E, 0x00, 0x49, 0x00, 0x08, 0x02, 0x02, 0x00, 0x5B,
    0x00, 0x03, 0x06, 0x0D, 0x07, 0x63, 0x00, 0x03, 0x08, 0x0C, 0x05, 0x4B,
    0x01, 0x1C, 0x00, 0x02, 0x00, 0x02, 0x00, 0x38, 0x0D, 0x02, 0x04, 0x02,
    0x01, 0x06, 0x0A, 0x02, 0x23, 0x0A, 0x00, 0x67, 0x03, 0x02, 0x05, 0x02,
    0x01, 0x03, 0x01, 0x1A, 0x01, 0x05, 0x02, 0x11, 0x03, 0x0E, 0x00, 0x03,
    0x01, 0x07, 0x00, 0x10, 0x00, 0x05, 0xC0, 0x02, 0x07, 0xB3, 0x02, 0x1E,
    0x01, 0x1F, 0x01, 0x1F, 0x01, 0x41, 0x01, 0x02, 0x06, 0x09, 0x00, 0x03,
    0x0A, 0x0A, 0x00, 0x2E, 0x02, 0x02, 0x00, 0x76, 0x01, 0x23, 0x00, 0x77,
    0x02, 0x05, 0x01, 0x0A, 0x00, 0x07, 0x02, 0x01, 0xDC, 0x01, 0x03, 0x00,
    0x3B, 0x00, 0x02, 0x06, 0x02, 0x00, 0x02, 0x00, 0x03, 0x07, 0x07, 0x09,
    0x03, 0x00, 0x31, 0x0D, 0x02, 0x0F, 0x32, 0x03, 0x31, 0x00, 0x02, 0x04,
    0x02, 0x00, 0x06, 0x00, 0x29, 0x08, 0x0D, 0x01, 0x21, 0x03, 0x03, 0x01,
    0x02, 0x02, 0x39, 0x00, 0x02, 0x01, 0x04, 0x00, 0x02, 0x02, 0x3B, 0x07,
    0x03, 0x01, 0x01, 0x99, 0x02, 0x02, 0x0C, 0x02, 0x06, 0x05, 0x00, 0x07,
    0x00, 0x04, 0x01, 0x01, 0xC7, 0x3F, 0x05, 0xD1, 0x0C, 0x05, 0x00, 0x04,
    0x0B, 0x17, 0xFF, 0x02, 0x01, 0x8E, 0x00, 0x61, 0x1F, 0x04, 0xAB, 0x03,
    0x6C, 0x01, 0x01, 0xEB, 0xD5, 0x00, 0x05, 0x09, 0x21, 0x01, 0x51, 0x01,
    0x02, 0x91, 0x00, 0x04, 0x00, 0x05, 0x00, 0x1A, 0x01, 0x06, 0x00, 0x01,
    0x98, 0x01, 0x1B, 0x11, 0x0E, 0x00, 0x27, 0x07, 0x1A, 0x0A, 0x2F, 0x02,
    0x31, 0x00, 0x03, 0x03, 0x03, 0x01, 0x28, 0x00, 0x44, 0x05, 0x03, 0x01,
    0x03, 0x01, 0x0D, 0x00, 0x09, 0x00, 0x30, 0x00, 0x34, 0x00, 0x02, 0x02,
    0x03, 0x01, 0x06, 0x01, 0x02, 0x00, 0x2B, 0x01, 0x09, 0x00, 0x01, 0xEF,
    0x00, 0x03, 0x00, 0x05, 0x00, 0x01, 0x9E, 0xB1, 0x00, 0x05, 0xE2, 0x0F,
    0x11, 0x0F, 0x07, 0xCE, 0x00, 0x01, 0xE3, 0x00, 0x01, 0x96, 0x04, 0x0D,
    0x87, 0x02, 0x02, 0x01, 0x06, 0x03, 0x29, 0x02, 0x05, 0x00, 0x01, 0xA6,
    0x01, 0x04, 0xBE, 0x03, 0x03, 0x84, 0x01, 0x01, 0x9A, 0x0A, 0x32, 0x03,
    0x7C, 0x00, 0x37, 0x0E, 0x2A, 0x00, 0x03, 0x01, 0x0B, 0x02, 0x32, 0x03,
    0x03, 0x01, 0x08, 0x00, 0x3E, 0x02, 0x25, 0x04, 0x02, 0x07, 0x3F, 0x00,
    0x0D, 0x01, 0x35, 0x08, 0x0B, 0x03, 0x03, 0x00, 0x60, 0x02, 0x03, 0x00,
    0x02, 0x01, 0x07, 0x00, 0x01, 0xA1, 0x00, 0x04, 0x07, 0x16, 0x01, 0x3A,
    0x01, 0x04, 0x00, 0x26, 0x06, 0x04, 0x04, 0x01, 0xC4, 0x07, 0x03, 0x02,
    0x02, 0x00, 0x18, 0x00, 0x55, 0x05, 0x02, 0x00, 0x05, 0x01, 0x02, 0x01,
    0x01, 0xEF, 0x03, 0x07, 0x01, 0x02, 0x01, 0x1C, 0x01, 0x56, 0x07, 0x03,
    0x00, 0x02, 0x01, 0x6B, 0x00, 0x02, 0x00, 0x03, 0x05, 0x02, 0x00, 0x66,
    0x02, 0x03, 0x03, 0x02, 0x04, 0x02, 0x84, 0x08, 0x02, 0x01, 0x02, 0x81,
    0x01, 0x02, 0x00, 0x05, 0x00, 0x01, 0x91, 0x03, 0x03, 0x01, 0x05, 0x00,
    0x21, 0x09, 0x29, 0x05, 0x03, 0x03, 0x09, 0x00, 0x0A, 0x05, 0x03, 0x02,
    0x2F, 0x0C, 0x02, 0x01, 0x03, 0x97, 0x06, 0x02, 0x05, 0x02, 0x00, 0x53,
    0x15, 0x03, 0x06, 0x02, 0x01, 0x02, 0x01, 0x7B, 0x05, 0x04, 0x00, 0x02,
    0x01, 0x02, 0x06, 0x02, 0x00, 0x49, 0x01, 0x04, 0x00, 0x02, 0x00, 0x02,
    0xDC, 0x01, 0x01, 0x97, 0xFC, 0x04, 0x3C, 0x06, 0x08, 0x99, 0x00, 0x40,
    0x03, 0x52, 0x00, 0x01, 0x99, 0xB9, 0x01, 0x24, 0xE2, 0x2D, 0x03, 0x16,
    0x04, 0xA1, 0x02, 0x12, 0x07, 0x03, 0x06, 0x1F, 0x03, 0x01, 0x95, 0x02,
    0x0F, 0xBC, 0x36, 0x05, 0x31, 0x09, 0x00, 0x0F, 0x00, 0x17, 0x04, 0x02,
    0x0E, 0x0A, 0xD1, 0x06, 0x02, 0x10, 0x03, 0x06, 0x02, 0x01, 0x02, 0x04,
    0x02, 0x86, 0x06, 0x02, 0xF8, 0x00, 0x3E, 0x03, 0x0B, 0xE1, 0x06, 0x6E,
    0x06, 0x30, 0xAF, 0xB6, 0x01, 0xEF, 0x3E, 0x30, 0x09, 0x0C, 0xA7, 0x09,
    0x01, 0x87, 0x09, 0x01, 0xC7, 0x09, 0x03, 0x9D, 0x09, 0x77, 0x09, 0x77,
    0x09, 0x77, 0x09, 0x77, 0x09, 0x77, 0x09, 0x77, 0x09, 0x77, 0x09, 0x77,
    0x09, 0x77, 0x09, 0x61, 0x09, 0x77, 0x09, 0x47, 0x09, 0x02, 0x97, 0x09,
    0x47, 0x09, 0x0E, 0xC7, 0x09, 0x27, 0x09, 0x02, 0xAD, 0x09, 0x01, 0x81,
    0x09, 0x01, 0xA7, 0x09, 0x07, 0x09, 0x01, 0xB7, 0x09, 0x57, 0x09, 0x01,
    0x87, 0x09, 0x07, 0x09, 0x02, 0x93, 0xC7, 0x09, 0x05, 0xA7, 0x09, 0x27,
    0x09, 0x01, 0xC7, 0x09, 0x17, 0x09, 0x57, 0x09, 0x03, 0x97, 0x09, 0x01,
    0xA6, 0x97, 0x09, 0x0B, 0x87, 0x09, 0x11, 0x87, 0x09, 0x06, 0xAD, 0x09,
    0x01, 0x81, 0x09, 0x3D, 0x09, 0x01, 0x91, 0x09, 0x02, 0x97, 0x09, 0x02,
    0xD7, 0x09, 0x77, 0x09, 0x02, 0xF7, 0x09, 0x67, 0x09, 0x67, 0x09, 0x03,
    0xA7, 0x09, 0x67, 0x09, 0x05, 0xF7, 0x09, 0x01, 0xF7, 0x09, 0x47, 0x09,
    0x01, 0x99, 0xB7, 0x09, 0x57, 0x09, 0x01, 0x87, 0x09, 0x01, 0xD8, 0xF5,
    0x31, 0x12, 0xC1, 0x09, 0x03, 0xA7, 0x09, 0x0C, 0xD7, 0x09, 0x25, 0x97,
    0x09, 0x0C, 0x2D, 0xEE, 0x02, 0x14, 0xF0, 0x22, 0x03, 0x03, 0x1C, 0xFF,
    0x00, 0x1A, 0x08, 0x0F, 0x02, 0x01, 0xED, 0xAC, 0x09, 0x01, 0xB4, 0xD1,
    0x34, 0x03, 0xCD, 0x00, 0x09, 0x00, 0x01, 0x87, 0x04, 0x40, 0xAB, 0x6E,
    0x47, 0x01, 0xB2, 0x01, 0x06, 0x00, 0x03, 0x02, 0x12, 0xB6, 0x05, 0x02,
    0xF9, 0x05, 0x79, 0x02, 0x01, 0x86, 0x06, 0x01, 0xDA, 0x06, 0x12, 0x08,
    0x03, 0xB2, 0x09, 0x08, 0xB6, 0x13, 0x08, 0xF4, 0x09, 0x03, 0xE1, 0x00,
    0x0D, 0x96, 0x00, 0x04, 0x05, 0x07, 0x09, 0x01, 0xC7, 0x0F, 0x2A, 0x00,
    0x05, 0xD7, 0x3B, 0x4F, 0x15, 0x04, 0xF7, 0x1D, 0x0A, 0xEA, 0x00, 0x09,
    0x95, 0x03, 0x01, 0x8B, 0x09, 0x1F, 0x07, 0x02, 0x0E, 0x21, 0x09, 0x28,
    0x0E, 0x01, 0xEA, 0xF1, 0x05, 0x01, 0xB1, 0xD2, 0x2C, 0x42, 0x03, 0x12,
    0x01, 0x02, 0xD6, 0x1A, 0x25, 0x03, 0x0A, 0xB5, 0x07, 0x1A, 0x06, 0x28,
    0x08, 0x4C, 0x04, 0x17, 0x05, 0x01, 0xA1, 0x01, 0x03, 0x0F, 0x03, 0x2D,
    0x41, 0x08, 0x35, 0x01, 0x1F, 0x02, 0x4C, 0x04, 0x69, 0x07, 0x19, 0x07,
    0x2A, 0x06, 0x02, 0xCB, 0x05, 0x02, 0xE1, 0x1E, 0x01, 0x9F, 0x09, 0x2B,
    0x03, 0x71, 0x06, 0x01, 0x87, 0x13, 0x02, 0xFC, 0x13, 0x0A, 0xC6, 0x01,
    0x03, 0xAF, 0x08, 0x06, 0xE8, 0x12, 0x06, 0xD4, 0x14, 0x01, 0x97, 0x87,
    0x06, 0x06, 0x9F, 0x16, 0x01, 0xC8, 0xCA, 0x13, 0x6D, 0x18, 0x2A, 0xCF,
    0x08, 0x07, 0xA2, 0x3A, 0x02, 0x02, 0x02, 0x03, 0x4D, 0x2C, 0x02, 0x0E,
    0x07, 0xC3, 0x0C, 0x06, 0x5F, 0x00, 0x3F, 0xE0, 0x01, 0x14, 0x00, 0x03,
    0xBB, 0xDF, 0x01, 0x19, 0x02, 0x01, 0xF0, 0x00, 0x13, 0x2D, 0x00, 0x0A,
    0xDD, 0x00, 0x34, 0x00, 0x1C, 0xC2, 0x00, 0x08, 0x86, 0x00, 0x10, 0x8A,
    0x05, 0x1C, 0x82, 0x00, 0x03, 0x00, 0x20, 0x01, 0x05, 0x00, 0x1D, 0x00,
    0x03, 0xBF, 0x00, 0x14, 0x00, 0x70, 0x00, 0x03, 0x9B, 0x91, 0x01, 0x26,
    0x00, 0x0B, 0x00, 0x01, 0xAA, 0x00, 0x1F, 0xA0, 0x00, 0x4C, 0x29, 0x00,
    0x34, 0x00, 0x20, 0x00, 0x1D, 0xBE, 0x00, 0x02, 0x00, 0x0E, 0xDF, 0x00,
    0x13, 0xAA, 0x00, 0x38, 0x00, 0x10, 0x00, 0x04, 0xFB, 0x00, 0x02, 0x00,
    0x1F, 0x00, 0x08, 0xBF, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x51, 0x00, 0x21, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x03, 0x95, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x41, 0x00, 0x02, 0x00, 0x22, 0x00, 0x08, 0xA6,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x2D, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x03, 0xAD, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x04, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x03, 0x01,
    0x03, 0x9A, 0x9F, 0x00, 0x01, 0xDA, 0x00, 0x1E, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x04,
    0x00, 0x12, 0x00, 0x02, 0x00, 0x02, 0x00, 0x01, 0xAB, 0x00, 0x34, 0x00,
    0x20, 0x00, 0x03, 0x00, 0x03, 0x00, 0x0A, 0x01, 0xBB, 0x00, 0x3E, 0xDE,
    0x00, 0x04, 0x00, 0x1D, 0x00, 0x1B, 0xC9, 0x00, 0x02, 0x00, 0x05, 0x00,
    0x03, 0x00, 0x10, 0x00, 0x04, 0x00, 0x0B, 0x01, 0xAB, 0x00, 0x3E, 0xED,
    0x00, 0x03, 0x01, 0x03, 0x00, 0x1A, 0x00, 0x1B, 0xC9, 0x00, 0x02, 0x00,
    0x05, 0x00, 0x03, 0x00, 0x10, 0x00, 0x04, 0x00, 0x01, 0xB9, 0x21, 0x02,
    0x02, 0x02, 0x03, 0x00, 0x02, 0x00, 0x02, 0x01, 0x0B, 0x01, 0x04, 0x01,
    0x1C, 0x00, 0x45, 0x00, 0x06, 0x00, 0x0F, 0x01, 0x08, 0x00, 0x05, 0xBF,
    0x00, 0x09, 0x00, 0x03, 0xD3, 0x05, 0x2A, 0x00, 0x37, 0x00, 0x03, 0x00,
    0x03, 0x00, 0x2D, 0x01, 0x15, 0x01, 0x02, 0x01, 0x0E, 0x00, 0x02, 0x02,
    0x4B, 0x03, 0x67, 0x00, 0x2C, 0x0D, 0x01, 0xEA, 0x02, 0x37, 0x0E, 0x20,
    0x00, 0x02, 0x86, 0x01, 0x0B, 0x00, 0x01, 0x8D, 0x00, 0x79, 0x00, 0x7A,
    0x00, 0x03, 0x87, 0x00, 0x0D, 0x00, 0x02, 0xF0, 0x00, 0x5B, 0x00, 0x0B,
    0x01, 0x01, 0xA9, 0x0E, 0x02, 0x00, 0x71, 0x00, 0x4B, 0x04, 0x05, 0x01,
    0x70, 0x05, 0x01, 0xAC, 0x00, 0x04, 0xE5, 0x08, 0x06, 0x86, 0x00, 0x7D,
    0x02, 0x48, 0x01, 0x01, 0x9E, 0x02, 0x02, 0x02, 0x26, 0x05, 0x02, 0x03,
    0x02, 0xBA, 0x01, 0x01, 0xD9, 0x01, 0x01, 0x81, 0x06, 0x02, 0x05, 0x01,
    0xAD, 0x06, 0x1D, 0x01, 0x7E, 0x03, 0x3C, 0x04, 0x3F, 0x01, 0x41, 0x07,
    0x0C, 0x00, 0x06, 0xC3, 0x01, 0x09, 0x07, 0x09, 0x08, 0x03, 0x03, 0x03,
    0x02, 0x04, 0x0A, 0x02, 0x00, 0x02, 0x09, 0x19, 0x9B, 0x03, 0x02, 0x01,
    0x71, 0x00, 0x01, 0x90, 0x01, 0x05, 0x02, 0x03, 0x00, 0x03, 0x08, 0x02,
    0x01, 0x02, 0x00, 0x03, 0x01, 0x0B, 0x04, 0x02, 0x09, 0x03, 0x03, 0x02,
    0x00, 0x02, 0x0C, 0x03, 0x02, 0x03, 0xAD, 0x02, 0x3A, 0x00, 0x01, 0xBE,
    0x00, 0x01, 0xE8, 0x83, 0x01, 0x02, 0x8E, 0x02, 0x64, 0x00, 0x0B, 0x00,
    0x74, 0x05, 0x02, 0xFD, 0x03, 0x57, 0x01, 0x29, 0x02, 0x02, 0x00, 0x32,
    0x01, 0x30, 0x00, 0x62, 0x0C, 0x11, 0x01, 0x7D, 0x03, 0x7F, 0x01, 0x11,
    0x01, 0x01, 0xFA, 0x00, 0x01, 0xA4, 0xA5, 0x06, 0x03, 0x00, 0x17, 0x00,
    0x15, 0x01, 0x03, 0x03, 0x04, 0x02, 0x02, 0x03, 0x08, 0x02, 0x07, 0x00,
    0x02, 0x01, 0x01, 0x96, 0x02, 0x02, 0x02, 0x03, 0x00, 0x02, 0x00, 0x02,
    0x01, 0x0B, 0x01, 0x04, 0x01, 0x1C, 0x00, 0x25, 0x00, 0x03, 0x01, 0x03,
    0x9B, 0x02, 0x05, 0x9D, 0x00, 0x31, 0x00, 0x03, 0x9F, 0x00, 0x05, 0xE8,
    0x00, 0x01, 0xC8, 0x00, 0x20, 0x00, 0x02, 0x91, 0x08, 0x27, 0x00, 0x71,
    0x06, 0x43, 0x06, 0x5A, 0x03, 0x07, 0xB9, 0x04, 0x2D, 0x03, 0x01, 0xBE,
    0x06, 0x6E, 0x01, 0x02, 0x03, 0x7F, 0x03, 0x31, 0x01, 0x50, 0x03, 0x05,
    0x00, 0x0E, 0x00, 0x02, 0x02, 0x59, 0x05, 0x6C, 0x00, 0x03, 0xA2, 0x04,
    0x0B, 0x01, 0x02, 0x00, 0x69, 0x00, 0x01, 0xFB, 0x16, 0x6A, 0x02, 0x1D,
    0x0C, 0x4D, 0x00, 0x01, 0x83, 0x02, 0x01, 0xFD, 0x00, 0x02, 0x89, 0x02,
    0x01, 0x9C, 0x00, 0x5D, 0x07, 0x54, 0x02, 0x02, 0x04, 0x03, 0x9F, 0x04,
    0x2B, 0x01, 0x05, 0x86, 0x01, 0x02, 0x87, 0x00, 0x08, 0xF1, 0x04, 0x16,
    0xFD, 0x01, 0x74, 0xFC, 0x01, 0x01, 0x86, 0x00, 0x42, 0x04, 0x09, 0x00,
    0x06, 0xD3, 0x03, 0x02, 0xC8, 0x00, 0x01, 0x99, 0xBD, 0x00, 0x3B, 0xE8,
    0x04, 0x1D, 0xD3, 0x01, 0x4F, 0x28, 0x00, 0x33, 0x00, 0x20, 0x00, 0x1D,
    0xBF, 0x00, 0x02, 0x00, 0x0E, 0xDF, 0x00, 0x12, 0xFF, 0x00, 0x04, 0x00,
    0x27, 0x00, 0x38, 0x00, 0x10, 0x00, 0x04, 0xFB, 0x00, 0x02, 0x00, 0x1F,
    0x00, 0x08, 0xBF, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x51, 0x00, 0x21, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x03, 0x95, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x41, 0x00, 0x02, 0x00, 0x22, 0x00, 0x08, 0xA6, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x1A, 0x00, 0x13, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x03, 0xAD, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02,
    0x00, 0x02, 0x00, 0x04, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x03,
    0x00, 0x03, 0x9A, 0xA2, 0x00, 0x01, 0xD8, 0x00, 0x1E, 0x00, 0x02, 0x00,
    0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00,
    0x04, 0x00, 0x12, 0x00, 0x02, 0x00, 0x02, 0x00, 0x01, 0xAB, 0x00, 0x33,
    0x00, 0x20, 0x00, 0x04, 0x00, 0x03, 0x00, 0x15, 0x24, 0x00, 0x7E, 0x03,
    0x09, 0xEA, 0x00, 0x7C, 0x00, 0x03, 0xF3, 0x01, 0x03, 0xF3, 0x01, 0x08,
    0x00, 0x01, 0xF6, 0x00, 0x02, 0x88, 0x00, 0x04, 0xC6, 0x00, 0x13, 0x9C,
    0x00, 0x11, 0xC5, 0x20, 0x02, 0x8E, 0xF8, 0x00, 0x01, 0xAB, 0xC4, 0x00,
    0x6D, 0x00, 0x01, 0x9B, 0x00, 0x01, 0xDC, 0x01, 0x04, 0x01, 0x3F, 0xF7,
    0x03, 0x03, 0x86, 0x9F, 0x00, 0x13, 0xB1, 0x00, 0x1F, 0x5E, 0x00, 0x02,
    0x00, 0x48, 0x00, 0x07, 0x00, 0x05, 0x00, 0x04, 0x00, 0x04, 0x8A, 0x03,
    0x0D, 0x0D, 0x06, 0x06, 0x02, 0x00, 0x02, 0x10, 0x76, 0x00, 0x0F, 0x01,
    0x0A, 0x83, 0x00, 0x2E, 0xB5, 0x00, 0x02, 0x02, 0x0C, 0x02, 0x0E, 0x02,
    0x0E, 0x02, 0x0E, 0x01, 0x21, 0x9D, 0x01, 0x01, 0xEC, 0xE4, 0x16, 0x0A,
    0x01, 0x68, 0x01, 0x07, 0xD1, 0x00, 0x0F, 0x01, 0x01, 0xA0, 0xC7, 0x10,
    0x06, 0xFC, 0x00, 0x02, 0x00, 0x01, 0xA3, 0x00, 0x03, 0xE8, 0x98, 0x04,
    0x40, 0x2B, 0x00, 0x11, 0x02, 0x3E, 0x00, 0x02, 0x00, 0x2E, 0x00, 0x05,
    0x00, 0x26, 0x00, 0x20, 0x00, 0x05, 0xFF, 0x00, 0x04, 0x90, 0x02, 0x34,
    0xBC, 0x00, 0x0E, 0x00, 0x28, 0x02, 0x0E, 0x02, 0x01, 0x8C, 0x00, 0x28,
    0x04, 0x07, 0x00, 0x45, 0x04, 0x06, 0x01, 0x05, 0x00, 0x03, 0x00, 0x03,
    0x00, 0x08, 0x00, 0x20, 0x01, 0x03, 0x00, 0x02, 0x00, 0x20, 0x02, 0x8B,
    0x21, 0x01, 0x5B, 0x00, 0x1F, 0x18, 0x29, 0x05, 0x03, 0xD6, 0x00, 0x0A,
    0x00, 0x37, 0x07, 0x70, 0x00, 0x02, 0xD1, 0x04, 0x03, 0x1E, 0x0B, 0x0F,
    0x02, 0x81, 0x01, 0x82, 0x17, 0x3E, 0x05, 0x1F, 0x03, 0x02, 0x81, 0x31,
    0x14, 0x03, 0x05, 0x03, 0x9F, 0xDD, 0x00, 0x06, 0xB9, 0x00, 0x02, 0x02,
    0x01, 0xA5, 0x00, 0x11, 0x02, 0x3E, 0x00, 0x02, 0x00, 0x01, 0x84, 0x00,
    0x07, 0x03, 0x03, 0xAD, 0xD5, 0x00, 0x1A, 0x00, 0x20, 0x00, 0x1A, 0x00,
    0x20, 0x00, 0x1A, 0x00, 0x20, 0x00, 0x1A, 0x00, 0x20, 0x00, 0x1A, 0x00,
    0x2E, 0xAD, 0x01, 0x01, 0xBA, 0x01, 0xA6, 0x00, 0x03, 0x00, 0x05, 0x00,
    0x02, 0x00, 0x07, 0xD2, 0x00, 0x02, 0x8B, 0x01, 0x01, 0x80, 0x01, 0x01,
    0xCF, 0x00, 0x0B, 0x00, 0x14, 0x01, 0x01, 0xF8, 0x00, 0x04, 0x84, 0x00,
    0x02, 0xF6, 0x00, 0x01, 0x83, 0x05, 0x02, 0x00, 0x01, 0x85, 0x00, 0x01,
    0xD0, 0x00, 0x2A, 0x00, 0x03, 0x88, 0x02, 0x10, 0x00, 0x02, 0x02, 0x03,
    0x05, 0x15, 0x00, 0x02, 0x00, 0x02, 0x00, 0x01, 0x86, 0x07, 0x02, 0x05,
    0x02, 0x01, 0x06, 0x03, 0x01, 0xC6, 0x01, 0x05, 0xF1, 0x09, 0x05, 0xD4,
    0x00, 0x05, 0xD3, 0x00, 0x01, 0x9E, 0x21, 0x02, 0xE2, 0x09, 0x0A, 0x08,
    0x0B, 0x84, 0x01, 0x02, 0x03, 0x02, 0x01, 0x0B, 0x00, 0x02, 0x01, 0x07,
    0x05, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x05, 0x00, 0x0C, 0x01, 0x0F,
    0x00, 0x02, 0x01, 0x02, 0x00, 0x3B, 0x01, 0x0A, 0x04, 0x03, 0x03, 0x02,
    0x01, 0x02, 0x01, 0x02, 0x06, 0x02, 0x1E, 0x03, 0x01, 0x02, 0x00, 0x02,
    0x1E, 0x02, 0x8D, 0x07, 0x05, 0x13, 0x03, 0x06, 0x03, 0x50, 0x02, 0x1D,
    0x1A, 0x27, 0x07, 0x44, 0x1A, 0x0A, 0x52, 0x4D, 0x17, 0x01, 0xB6, 0x02,
    0x08, 0x02, 0x35, 0x09, 0x6E, 0x02, 0x01, 0xF7, 0x2D, 0x2B, 0x41, 0x01,
    0xFF, 0x04, 0x81, 0x2F, 0x16, 0x01, 0x07, 0x26, 0x03, 0x1F, 0x02, 0x68,
    0x01, 0xE6, 0x05, 0x02, 0xE6, 0x01, 0x2F, 0x19, 0x02, 0x58, 0x0D, 0x01,
    0xD5, 0x1B, 0x0B, 0x09, 0x00, 0x0E, 0x01, 0x0D, 0x00, 0x16, 0x01, 0x07,
    0x01, 0x02, 0xD1, 0x01, 0x05, 0x09, 0x21, 0x23, 0x1D, 0x1E, 0x0C, 0x1D,
    0x09, 0x00, 0x10, 0x1F, 0x0B, 0x26, 0x10, 0x02, 0xBF, 0x33, 0xC1, 0x3F,
    0x01, 0xAD, 0x91, 0x36, 0x06, 0xE2, 0x03, 0x0B, 0x01, 0x02, 0x00, 0x04,
    0xBE, 0x02, 0x01, 0xA5, 0xC7, 0x0F, 0x01, 0x80, 0x00, 0x2E, 0x02, 0x03,
    0xE5, 0x00, 0x04, 0x00, 0x05, 0x01, 0x0E, 0x01, 0x02, 0xBA, 0x08, 0x3A,
    0x10, 0x03, 0x02, 0x02, 0x0C, 0x04, 0x00, 0x30, 0x2C, 0x0C, 0xFB, 0x01,
    0x04, 0xD0, 0x00, 0x18, 0xF7, 0x00, 0x11, 0x96, 0x07, 0x05, 0x10, 0x01,
    0x96, 0xCB, 0x03, 0x06, 0x00, 0x01, 0xA2, 0xD7, 0x00, 0x25, 0xB4, 0x73,
    0x3D, 0x01, 0xF5, 0x0B, 0x26, 0x03, 0x3B, 0x06, 0x02, 0x17, 0x01, 0x08,
    0x1D, 0x05, 0x3C, 0x16, 0x41, 0x04, 0x00, 0x01, 0xBB, 0x56, 0x09, 0xAA,
    0x03, 0xFF, 0x38, 0x03, 0x33, 0x07, 0x02, 0x0D, 0x02, 0x01, 0x0D, 0xC9,
    0x00, 0x16, 0xDD, 0x00, 0x01, 0x82, 0x00, 0x05, 0xD2, 0x2B, 0x05, 0x63,
    0x0D, 0x0E, 0x03, 0x0E, 0x02, 0x0E, 0x02, 0x24, 0x18, 0x01, 0xA0, 0x39,
    0x1C, 0x0E, 0x2B, 0x05, 0x08, 0x08, 0x01, 0x0F, 0x05, 0x01, 0x9B, 0x01,
    0xFA, 0x06, 0x05, 0xD7, 0x06, 0x0F, 0x04, 0x0C, 0x04, 0x73, 0x0D, 0x58,
    0x08, 0x0B, 0x05, 0x00, 0x10, 0x0B, 0x05, 0x37, 0x09, 0x09, 0x07, 0x27,
    0x09, 0x1D, 0x03, 0x01, 0x4F, 0x02, 0xD3, 0x0D, 0x0D, 0x03, 0x04, 0x04,
    0x04, 0x04, 0x06, 0x0A, 0x1C, 0x04, 0x0A, 0x06, 0x05, 0x0B, 0x09, 0x07,
    0x07, 0x09, 0x06, 0x0A, 0x01, 0x92, 0x02, 0x36, 0x01, 0x40, 0xA8, 0x00,
    0x01, 0x40, 0xA9, 0x00, 0x07, 0x20, 0x00, 0x01, 0x80, 0x00, 0x2B, 0xE0,
    0x00, 0x13, 0x80, 0x0A, 0x25, 0x00, 0x30, 0x00, 0x1F, 0xA1, 0x00,
};

RE_INTERNAL_DATA re_int32 re__rune_data_casefold_0[] = {
    0x0000,  0x0020,  0x0000,  0x0000,  0x0020,  0x0020,  -0x0020, -0x0020,
    0x0001,  0x0001,  -0x00CD, -0x00CD, -0x00D9, -0x00D9, 0x0082,  0x0082,
    0x0025,  0x0025,  0x003F,  0x003F,  -0x0025, -0x0025, -0x0082, -0x0082,
    0x0050,  0x0050,  -0x0050, -0x0050, 0x0030,  0x0030,  -0x0030, -0x0030,
    0x1C60,  0x1C60,  0x0BC0,  0x0BC0,  0x97D0,  0x97D0,  0x0008,  0x0008,
    -0x0008, -0x0008, -0x0BC0, -0x0BC0, 0x004A,  0x004A,  0x0056,  0x0056,
    0x0064,  0x0064,  0x0080,  0x0080,  0x0070,  0x0070,  0x007E,  0x007E,
    -0x004A, -0x004A, -0x0056, -0x0056, -0x0064, -0x0064, -0x0070, -0x0070,
    -0x0080, -0x0080, -0x007E, -0x007E, 0x0010,  0x0010,  -0x0010, -0x0010,
    0x001A,  0x001A,  -0x001A, -0x001A, -0x2A3F, -0x2A3F, -0x1C60, -0x1C60,
    -0x97D0, -0x97D0, 0x0028,  0x0028,  -0x0028, -0x0028, 0x0027,  0x0027,
    -0x0027, -0x0027, 0x0040,  0x0040,  -0x0040, -0x0040, 0x0022,  0x0022,
    -0x0022, -0x0022, 0x0020,  0x0000,  -0x0020, 0x20BF,  -0x0020, 0x010C,
    -0x0020, 0x0000,  0x02E7,  0x0020,  0x1DBF,  -0x0020, 0x2046,  -0x0020,
    0x0079,  0x0001,  -0x0001, 0x0000,  0x0001,  -0x0001, 0x0001,  -0x0001,
    0x0000,  -0x0079, 0x0001,  -0x0001, -0x012C, 0x00C3,  0x00D2,  0x00CE,
    0x0001,  -0x0001, 0x00CD,  0x0001,  0x004F,  0x00CA,  0x00CB,  0x0001,
    0x00CF,  0x0061,  0x00D3,  0x00D1,  0x00A3,  0x0000,  0x00D3,  0x00D5,
    0x0082,  0x00D6,  0x00DA,  0x0001,  -0x0001, 0x00DA,  -0x0001, 0x00D9,
    0x0001,  -0x0001, 0x00DB,  0x0000,  0x0038,  -0x0002, 0x0001,  -0x0002,
    -0x0001, -0x004F, -0x0061, -0x0038, -0x0082, 0x0000,  0x2A2B,  0x0001,
    -0x0001, -0x00A3, 0x2A28,  0x2A3F,  0x0001,  -0x0001, -0x00C3, 0x0045,
    0x0047,  0x2A1F,  0x2A1C,  0x2A1E,  -0x00D2, -0x00CE, 0x0000,  -0x00CA,
    0x0000,  -0x00CB, 0xA54F,  0x0000,  -0x00CD, 0xA54B,  0x0000,  -0x00CF,
    0x0000,  0xA528,  0xA544,  0x0000,  -0x00D1, -0x00D3, 0xA544,  0x29F7,
    0xA541,  0x0000,  -0x00D3, 0x0000,  0x29FD,  -0x00D5, 0x0000,  -0x00D6,
    0x0000,  0x29E7,  -0x00DA, 0x0000,  0xA543,  -0x00DA, 0x0000,  0xA52A,
    -0x00DA, -0x0045, -0x0047, 0x0000,  -0x00DB, 0x0000,  0xA515,  0xA512,
    0x0000,  0x0054,  0x0000,  0x0082,  0x0000,  0x0074,  0x0026,  0x0000,
    0x0025,  0x0000,  0x0040,  0x0000,  0x001F,  -0x0026, -0x0025, 0x001E,
    -0x0020, 0x0040,  0x0019,  0x1C05,  0x0036,  -0x0020, -0x0307, -0x0020,
    0x0016,  0x0030,  0x0001,  -0x0020, 0x000F,  -0x0020, 0x1D5D,  -0x0040,
    -0x003F, 0x0008,  -0x003E, 0x0023,  0x0000,  -0x002F, -0x0036, -0x0008,
    -0x0056, -0x0050, 0x0007,  -0x0074, -0x005C, -0x0060, -0x0001, -0x0007,
    0x0000,  -0x0082, 0x184E,  -0x0020, 0x184D,  -0x0020, 0x1844,  -0x0020,
    0x1842,  -0x0020, 0x183C,  -0x0020, 0x0001,  0x1824,  0x000F,  0x0001,
    -0x0001, -0x000F, 0x0000,  0x0030,  0x0000,  -0x0030, 0x0000,  0x1C60,
    0x0BC0,  0x0000,  0x0BC0,  -0x186E, -0x186D, -0x1864, -0x1862, 0x0001,
    -0x1863, -0x185C, -0x1825, 0x89C2,  0x0000,  -0x0BC0, 0x0000,  -0x0BC0,
    0x0000,  0x8A04,  0x0000,  0x0EE6,  0x8A38,  0x0000,  0x0001,  0x003A,
    0x0000,  -0x003B, -0x1DBF, 0x0000,  0x0008,  0x0000,  -0x0008, 0x0000,
    0x0009,  -0x0009, 0x0000,  -0x1C79, 0x0000,  0x0007,  -0x0007, 0x0000,
    -0x1D7D, 0x0000,  -0x20DF, -0x2066, 0x001C,  0x0000,  -0x001C, 0x0000,
    -0x29F7, -0x0EE6, -0x29E7, -0x2A2B, -0x2A28, 0x0001,  -0x0001, -0x2A1C,
    -0x29FD, -0x2A1F, -0x2A1E, 0x0000,  -0x1C60, 0x0001,  -0x89C3, -0x0001,
    -0x8A04, -0x0001, -0xA528, -0xA544, -0xA54F, -0xA54B, -0xA541, -0xA544,
    0x0000,  -0xA512, -0xA52A, -0xA515, 0x03A0,  -0x0030, -0xA543, -0x8A38,
    0x0001,  0x0000,  -0x03A0, 0x0027,  0x0000,  -0x0027, 0x0000,  -0x0040,
    0x0000,
};

RE_INTERNAL_DATA re_uint16 re__rune_data_casefold_1[] = {
    0x000, 0x004, 0x004, 0x004, 0x002, 0x002, 0x002, 0x002, 0x004, 0x004, 0x004,
    0x004, 0x006, 0x006, 0x006, 0x006, 0x071, 0x071, 0x071, 0x071, 0x075, 0x075,
    0x075, 0x075, 0x018, 0x018, 0x018, 0x018, 0x01A, 0x01A, 0x01A, 0x01A, 0x01C,
    0x01C, 0x01C, 0x01C, 0x01E, 0x01E, 0x01E, 0x01E, 0x020, 0x020, 0x020, 0x020,
    0x022, 0x022, 0x022, 0x022, 0x024, 0x024, 0x024, 0x024, 0x02A, 0x02A, 0x02A,
    0x02A, 0x026, 0x026, 0x026, 0x026, 0x028, 0x028, 0x028, 0x028, 0x143, 0x143,
    0x143, 0x143, 0x145, 0x145, 0x145, 0x145, 0x044, 0x044, 0x044, 0x044, 0x046,
    0x046, 0x046, 0x046, 0x048, 0x048, 0x048, 0x048, 0x04A, 0x04A, 0x04A, 0x04A,
    0x04E, 0x04E, 0x04E, 0x04E, 0x050, 0x050, 0x050, 0x050, 0x052, 0x052, 0x052,
    0x052, 0x054, 0x054, 0x054, 0x054, 0x056, 0x056, 0x056, 0x056, 0x058, 0x058,
    0x058, 0x058, 0x05A, 0x05A, 0x05A, 0x05A, 0x05C, 0x05C, 0x05C, 0x05C, 0x05E,
    0x05E, 0x05E, 0x05E, 0x060, 0x060, 0x060, 0x060, 0x002, 0x002, 0x069, 0x002,
    0x006, 0x064, 0x006, 0x006, 0x06D, 0x006, 0x066, 0x006, 0x006, 0x002, 0x002,
    0x0D0, 0x002, 0x0DC, 0x002, 0x002, 0x152, 0x002, 0x002, 0x071, 0x071, 0x002,
    0x071, 0x004, 0x0EB, 0x004, 0x004, 0x002, 0x002, 0x126, 0x002, 0x006, 0x112,
    0x114, 0x006, 0x11A, 0x006, 0x006, 0x002, 0x002, 0x163, 0x002, 0x071, 0x11C,
    0x071, 0x071, 0x123, 0x071, 0x022, 0x128, 0x129, 0x022, 0x02A, 0x135, 0x136,
    0x02A, 0x002, 0x147, 0x002, 0x002, 0x0E0, 0x002, 0x154, 0x002, 0x002, 0x073,
    0x077, 0x002, 0x071, 0x002, 0x002, 0x071, 0x165, 0x071, 0x071, 0x002, 0x073,
    0x169, 0x002, 0x071, 0x002, 0x002, 0x071, 0x002, 0x179, 0x002, 0x002, 0x056,
    0x17B, 0x056, 0x056, 0x058, 0x17C, 0x058, 0x058, 0x004, 0x062, 0x002, 0x002,
    0x063, 0x006, 0x006, 0x006, 0x068, 0x002, 0x002, 0x004, 0x004, 0x004, 0x062,
    0x004, 0x004, 0x004, 0x06B, 0x006, 0x006, 0x006, 0x068, 0x006, 0x006, 0x006,
    0x06F, 0x002, 0x071, 0x071, 0x071, 0x073, 0x075, 0x075, 0x075, 0x077, 0x071,
    0x071, 0x071, 0x079, 0x075, 0x075, 0x07B, 0x07D, 0x071, 0x071, 0x07F, 0x081,
    0x082, 0x077, 0x084, 0x086, 0x081, 0x088, 0x08A, 0x071, 0x08C, 0x08E, 0x090,
    0x071, 0x071, 0x071, 0x092, 0x094, 0x002, 0x071, 0x092, 0x096, 0x097, 0x075,
    0x099, 0x071, 0x002, 0x071, 0x09B, 0x002, 0x002, 0x008, 0x09D, 0x09E, 0x008,
    0x09D, 0x075, 0x075, 0x0A0, 0x071, 0x073, 0x09E, 0x071, 0x0A2, 0x0A4, 0x071,
    0x071, 0x071, 0x002, 0x002, 0x0A6, 0x0A8, 0x0AA, 0x0AB, 0x0AD, 0x0AF, 0x071,
    0x0B1, 0x0B3, 0x0B5, 0x00A, 0x0B6, 0x0B8, 0x0BA, 0x002, 0x0BC, 0x0BE, 0x0C0,
    0x0C2, 0x0C4, 0x0C6, 0x0C8, 0x0C9, 0x0CB, 0x0CD, 0x0CE, 0x002, 0x0D2, 0x0D4,
    0x002, 0x0D6, 0x0D8, 0x00C, 0x0DA, 0x002, 0x002, 0x0DD, 0x0DF, 0x002, 0x0E2,
    0x00E, 0x0E4, 0x002, 0x002, 0x002, 0x0E6, 0x010, 0x0E8, 0x0EA, 0x012, 0x004,
    0x004, 0x0ED, 0x014, 0x063, 0x0EF, 0x0F0, 0x006, 0x0F2, 0x0F4, 0x0F6, 0x006,
    0x0F8, 0x0FA, 0x006, 0x0FC, 0x0FD, 0x006, 0x0FF, 0x100, 0x102, 0x002, 0x104,
    0x106, 0x108, 0x10A, 0x10C, 0x073, 0x10E, 0x071, 0x110, 0x016, 0x006, 0x006,
    0x006, 0x116, 0x117, 0x118, 0x006, 0x006, 0x071, 0x002, 0x002, 0x002, 0x11E,
    0x075, 0x075, 0x075, 0x120, 0x122, 0x01C, 0x01C, 0x01C, 0x123, 0x124, 0x01E,
    0x01E, 0x01E, 0x125, 0x020, 0x020, 0x020, 0x126, 0x026, 0x026, 0x026, 0x002,
    0x028, 0x028, 0x028, 0x002, 0x12B, 0x12D, 0x12F, 0x131, 0x133, 0x002, 0x002,
    0x002, 0x138, 0x002, 0x13A, 0x002, 0x002, 0x002, 0x13C, 0x13E, 0x071, 0x071,
    0x071, 0x002, 0x140, 0x002, 0x142, 0x02C, 0x02E, 0x02E, 0x030, 0x032, 0x034,
    0x036, 0x002, 0x026, 0x147, 0x002, 0x002, 0x028, 0x038, 0x149, 0x14B, 0x03A,
    0x03A, 0x149, 0x002, 0x026, 0x002, 0x002, 0x002, 0x028, 0x03C, 0x002, 0x002,
    0x026, 0x002, 0x14C, 0x002, 0x028, 0x03E, 0x14E, 0x002, 0x040, 0x042, 0x149,
    0x002, 0x002, 0x002, 0x150, 0x002, 0x002, 0x002, 0x156, 0x002, 0x002, 0x002,
    0x048, 0x04A, 0x002, 0x002, 0x002, 0x071, 0x158, 0x15A, 0x15C, 0x075, 0x075,
    0x15E, 0x160, 0x162, 0x071, 0x073, 0x077, 0x002, 0x002, 0x002, 0x04C, 0x002,
    0x073, 0x075, 0x077, 0x04E, 0x04E, 0x04E, 0x163, 0x073, 0x075, 0x167, 0x071,
    0x16B, 0x16D, 0x16F, 0x171, 0x173, 0x071, 0x071, 0x175, 0x177, 0x075, 0x077,
    0x002, 0x002, 0x073, 0x077, 0x052, 0x052, 0x002, 0x002, 0x054, 0x054, 0x002,
    0x002, 0x056, 0x17B, 0x056, 0x17C, 0x058, 0x17C, 0x17D, 0x002, 0x05A, 0x0EA,
    0x002, 0x002, 0x05C, 0x17F, 0x002, 0x002, 0x05E, 0x060, 0x060, 0x060, 0x002,
    0x002,
};

RE_INTERNAL_DATA re_uint16 re__rune_data_casefold_2[] = {
    0x000, 0x008, 0x004, 0x004, 0x008, 0x008, 0x010, 0x010, 0x018, 0x018, 0x01C,
    0x01C, 0x020, 0x020, 0x024, 0x024, 0x028, 0x028, 0x02C, 0x02C, 0x030, 0x030,
    0x034, 0x034, 0x048, 0x048, 0x04C, 0x04C, 0x050, 0x050, 0x054, 0x054, 0x058,
    0x058, 0x05C, 0x05C, 0x060, 0x060, 0x064, 0x064, 0x070, 0x070, 0x074, 0x074,
    0x00C, 0x00C, 0x078, 0x078, 0x07C, 0x07C, 0x008, 0x0E3, 0x0E7, 0x084, 0x089,
    0x0EA, 0x080, 0x004, 0x0EE, 0x0F2, 0x086, 0x00C, 0x0F6, 0x0FA, 0x0FE, 0x102,
    0x014, 0x106, 0x010, 0x10A, 0x10E, 0x112, 0x116, 0x11A, 0x11E, 0x122, 0x126,
    0x12A, 0x12E, 0x132, 0x014, 0x135, 0x139, 0x010, 0x13D, 0x010, 0x13F, 0x142,
    0x146, 0x010, 0x14A, 0x14E, 0x152, 0x156, 0x15A, 0x08D, 0x15E, 0x162, 0x090,
    0x165, 0x0BE, 0x004, 0x097, 0x169, 0x16D, 0x171, 0x09B, 0x175, 0x179, 0x17D,
    0x181, 0x185, 0x189, 0x010, 0x18D, 0x191, 0x0A3, 0x195, 0x199, 0x0A6, 0x0AE,
    0x010, 0x19D, 0x0FE, 0x1A1, 0x1A2, 0x1A6, 0x020, 0x1A7, 0x004, 0x1AB, 0x024,
    0x1AC, 0x004, 0x1B0, 0x09F, 0x02C, 0x0B4, 0x1B4, 0x1B8, 0x1BC, 0x1C0, 0x034,
    0x0B8, 0x004, 0x1C4, 0x004, 0x1C7, 0x1CB, 0x010, 0x1CC, 0x1CF, 0x038, 0x03C,
    0x040, 0x044, 0x1D3, 0x1D7, 0x1DB, 0x1DF, 0x0BC, 0x1E3, 0x1E7, 0x1EB, 0x1EF,
    0x1F3, 0x0BC, 0x1F7, 0x1FA, 0x093, 0x0C1, 0x004, 0x1FE, 0x0C4, 0x004, 0x202,
    0x050, 0x054, 0x206, 0x20A, 0x20E, 0x212, 0x216, 0x13F, 0x21A, 0x0C7, 0x004,
    0x21E, 0x0AA, 0x010, 0x0CB, 0x010, 0x1CC, 0x010, 0x13F, 0x0FE, 0x010, 0x004,
    0x222, 0x010, 0x0CF, 0x0B0, 0x010, 0x225, 0x229, 0x010, 0x22B, 0x22F, 0x0D3,
    0x19D, 0x231, 0x004, 0x0D7, 0x004, 0x0E7, 0x00C, 0x0EA, 0x060, 0x064, 0x235,
    0x064, 0x239, 0x068, 0x0DB, 0x23D, 0x06C, 0x0DF, 0x06C, 0x0DF, 0x241, 0x245,
    0x004, 0x249, 0x004, 0x24D, 0x07C, 0x24F, 0x004,
};

RE_INTERNAL_DATA re_uint8 re__rune_data_casefold_3[] = {
    0x08, 0x04, 0x04, 0x74, 0x76, 0x0A, 0x78, 0x06, 0x7A, 0x06, 0x06, 0x06,
    0x7C, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x7E, 0x0C, 0x80, 0x82, 0x0E,
    0x84, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x04, 0x04, 0x2C, 0x2C,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0xD4, 0x02, 0x22, 0x22,
    0x22, 0x22, 0x22, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x04, 0x04, 0x2C, 0x2C, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0xA8,
    0xAA, 0xAB, 0x02, 0x18, 0x1A, 0xAD, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x28, 0x28, 0x28, 0xE6, 0x2A, 0x2A, 0x2A, 0xE8, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0xAF, 0x1C, 0x1E, 0xB1, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x00, 0x32, 0xD6, 0xD7, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x10, 0x10, 0x86, 0x12, 0x12, 0x88, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x2E, 0x2E, 0xEA, 0x30, 0xEC, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x8C, 0x16, 0x16, 0x8E, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x90, 0x92, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x00, 0x32, 0x34, 0x36, 0x02, 0x02, 0x02, 0x38,
    0x04, 0x3A, 0x3C, 0x3E, 0x06, 0x06, 0x06, 0x40, 0x42, 0x06, 0x06, 0x44,
    0x46, 0x48, 0x4A, 0x4C, 0x4E, 0x50, 0x06, 0x52, 0x06, 0x06, 0x54, 0x56,
    0x58, 0x5A, 0x5C, 0x5E, 0x60, 0x62, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x64, 0x02, 0x02, 0x66, 0x68, 0x00, 0x6A, 0x6C,
    0x6E, 0x70, 0x06, 0x72, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x14, 0x14, 0x14, 0x14, 0x14, 0x8A,
    0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x94, 0x06, 0x06, 0x96, 0x06, 0x06,
    0x06, 0x06, 0x06, 0x06, 0x98, 0x8A, 0x98, 0x98, 0x8A, 0x9A, 0x98, 0x9C,
    0x98, 0x98, 0x98, 0x9E, 0xA0, 0xA2, 0xA4, 0xA6, 0x0C, 0x0C, 0x0C, 0x0E,
    0x0E, 0x0E, 0xB3, 0xB5, 0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0xB7, 0xB9,
    0x20, 0x20, 0xBB, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0xBD, 0x06, 0xBF, 0x02, 0x06, 0xC1, 0x02, 0x02,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0xC3, 0xC3, 0x06, 0x06, 0x06, 0xC5,
    0xC7, 0xC9, 0xCA, 0xCC, 0xCE, 0xD0, 0x02, 0xD2, 0x24, 0x24, 0xD9, 0x26,
    0x26, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x24, 0x24, 0xDB, 0x26, 0xDC,
    0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0xDE, 0xDE, 0xE0, 0xE2, 0xE4,
    0x02, 0x02, 0x02, 0x02,
};

RE_INTERNAL_DATA re_uint16 re__rune_data_casefold_4[] = {
    0x059, 0x024, 0x087, 0x024, 0x024, 0x024, 0x170, 0x024, 0x024, 0x024, 0x024,
    0x024, 0x024, 0x024, 0x024, 0x024, 0x0BC, 0x024, 0x024, 0x024, 0x024, 0x024,
    0x024, 0x024, 0x024, 0x024, 0x024, 0x024, 0x024, 0x024, 0x024, 0x024, 0x024,
    0x01C, 0x024, 0x024, 0x024, 0x024, 0x024, 0x024, 0x024, 0x024, 0x18C, 0x024,
    0x02F, 0x024, 0x024, 0x024, 0x024, 0x024, 0x024, 0x024, 0x024, 0x024, 0x024,
    0x1AC, 0x024, 0x024, 0x024, 0x072, 0x024, 0x024, 0x024, 0x024, 0x024, 0x04B,
    0x024, 0x024, 0x024, 0x0F0, 0x110, 0x000, 0x024, 0x024, 0x024, 0x024, 0x024,
    0x0AC, 0x130, 0x024, 0x024, 0x024, 0x024, 0x0D4, 0x150, 0x024, 0x024, 0x024,
    0x024, 0x024, 0x024, 0x024, 0x024, 0x024, 0x024, 0x024, 0x024, 0x024, 0x024,
    0x024, 0x096,
};

RE_INTERNAL_DATA re_uint8 re__rune_data_casefold_5[] = {
    0x45, 0x00, 0x11, 0x11, 0x11, 0x27, 0x11, 0x55, 0x35, 0x11, 0x11, 0x1A,
    0x11, 0x11, 0x11, 0x0C, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
    0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
    0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
    0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
    0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
    0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
    0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
    0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
    0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
    0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11,
    0x11, 0x11, 0x11, 0x11,
};

/* Naive     : 4 bytes * 1114112 runes:         4456448 bytes
 * Wine-style: 2-level, 256x4352, squish:         29952 bytes
 * This:     : 6-level, 2x4x2x32x16x136, squish:   4004 bytes (-86.63%) */
/* This version is almost certainly slower, but is also almost certainly more
 * cache-friendly. */
/* I thought it was cool that my program figured out that it's most efficient
 * to compress the bottom-level to pairs of 2 rune deltas. Especially at higher
 * codepoints, there are large runs of +1/-1 repetitions, which are easily
 * compressible. In addition, there's a lot of gain to be had by compressing
 * the smallest units possible, because the bottom level stores deltas as
 * int32s since a very small percentage (<1%) of deltas are outside the range
 * of an int16. We end up losing fewer zero bytes this way. */
/* See tools/unicode_data.py */
RE_INTERNAL re_rune re__rune_data_casefold_next(re_rune r)
{
  return re__rune_data_casefold_0
      [re__rune_data_casefold_1
           [re__rune_data_casefold_2
                [re__rune_data_casefold_3
                     [re__rune_data_casefold_4
                          [re__rune_data_casefold_5[r >> 13] +
                           ((r >> 9) & 0x0F)] +
                      ((r >> 4) & 0x1F)] +
                 ((r >> 3) & 0x01)] +
            ((r >> 1) & 0x03)] +
       ((r)&0x01)];
}

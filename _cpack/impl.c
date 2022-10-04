#include "../tests/test_config.h"
#include "internal.h"

/* bits/types/char */
MN__STATIC_ASSERT(mn__char_is_one_byte, sizeof(mn_char) == 1);

/* bits/container/str */
/* Maximum size, without null terminator */
#define MN__STR_SHORT_SIZE_MAX                                                \
    (((sizeof(mn__str) - sizeof(mn_size)) / (sizeof(mn_char)) - 1))

#define MN__STR_GET_SHORT(str) !((str)->_size_short & 1)
#define MN__STR_SET_SHORT(str, short)                                         \
    do {                                                                      \
        mn_size temp = short;                                                 \
        (str)->_size_short &= ~((mn_size)1);                                  \
        (str)->_size_short |= !temp;                                          \
    } while (0)
#define MN__STR_GET_SIZE(str) ((str)->_size_short >> 1)
#define MN__STR_SET_SIZE(str, size)                                           \
    do {                                                                      \
        mn_size temp = size;                                                  \
        (str)->_size_short &= 1;                                              \
        (str)->_size_short |= temp << 1;                                      \
    } while (0)
#define MN__STR_DATA(str)                                                     \
    (MN__STR_GET_SHORT(str) ? ((mn_char*)&((str)->_alloc)) : (str)->_data)

/* Round up to multiple of 32 */
#define MN__STR_ROUND_ALLOC(alloc) (((alloc + 1) + 32) & (~((mn_size)32)))

#if MN_DEBUG

#define MN__STR_CHECK(str)                                                    \
    do {                                                                      \
        if (MN__STR_GET_SHORT(str)) {                                         \
            /* If string is short, the size must always be less than */       \
            /* MN__STR_SHORT_SIZE_MAX. */                                     \
            MN_ASSERT(MN__STR_GET_SIZE(str) <= MN__STR_SHORT_SIZE_MAX);       \
        } else {                                                              \
            /* If string is long, the size can still be less, but the other   \
             */                                                               \
            /* fields must be valid. */                                       \
            /* Ensure there is enough space */                                \
            MN_ASSERT((str)->_alloc >= MN__STR_GET_SIZE(str));                \
            /* Ensure that the _data field isn't NULL if the size is 0 */     \
            if (MN__STR_GET_SIZE(str) > 0) {                                  \
                MN_ASSERT((str)->_data != MN_NULL);                           \
            }                                                                 \
            /* Ensure that if _alloc is 0 then _data is NULL */               \
            if ((str)->_alloc == 0) {                                         \
                MN_ASSERT((str)->_data == MN_NULL);                           \
            }                                                                 \
        }                                                                     \
        /* Ensure that there is a null-terminator */                          \
        MN_ASSERT(MN__STR_DATA(str)[MN__STR_GET_SIZE(str)] == '\0');          \
    } while (0)

#else

#define MN__STR_CHECK(str) MN__UNUSED(str)

#endif

void mn__str_init(mn__str* str) {
    str->_size_short = 0;
    MN__STR_DATA(str)[0] = '\0';
}

void mn__str_destroy(mn__str* str) {
    if (!MN__STR_GET_SHORT(str)) {
        if (str->_data != MN_NULL) {
            MN_FREE(str->_data);
        }
    }
}

mn_size mn__str_size(const mn__str* str) { return MN__STR_GET_SIZE(str); }

MN_INTERNAL int mn__str_grow(mn__str* str, mn_size new_size) {
    mn_size old_size = MN__STR_GET_SIZE(str);
    MN__STR_CHECK(str);
    if (MN__STR_GET_SHORT(str)) {
        if (new_size <= MN__STR_SHORT_SIZE_MAX) {
            /* Can still be a short str */
            MN__STR_SET_SIZE(str, new_size);
        } else {
            /* Needs allocation */
            mn_size new_alloc =
              MN__STR_ROUND_ALLOC(new_size + (new_size >> 1));
            mn_char* new_data =
              (mn_char*)MN_MALLOC(sizeof(mn_char) * (new_alloc + 1));
            mn_size i;
            if (new_data == MN_NULL) {
                return -1;
            }
            /* Copy data from old string */
            for (i = 0; i < old_size; i++) {
                new_data[i] = MN__STR_DATA(str)[i];
            }
            /* Fill in the remaining fields */
            MN__STR_SET_SHORT(str, 0);
            MN__STR_SET_SIZE(str, new_size);
            str->_data = new_data;
            str->_alloc = new_alloc;
        }
    } else {
        if (new_size > str->_alloc) {
            /* Needs allocation */
            mn_size new_alloc =
              MN__STR_ROUND_ALLOC(new_size + (new_size >> 1));
            mn_char* new_data;
            if (str->_alloc == 0) {
                new_data =
                  (mn_char*)MN_MALLOC(sizeof(mn_char) * (new_alloc + 1));
            } else {
                new_data = (mn_char*)MN_REALLOC(
                  str->_data, sizeof(mn_char) * (new_alloc + 1));
            }
            if (new_data == MN_NULL) {
                return -1;
            }
            str->_data = new_data;
            str->_alloc = new_alloc;
        }
        MN__STR_SET_SIZE(str, new_size);
    }
    /* Null terminate */
    MN__STR_DATA(str)[MN__STR_GET_SIZE(str)] = '\0';
    MN__STR_CHECK(str);
    return 0;
}

int mn__str_push(mn__str* str, mn_char chr) {
    int err = 0;
    mn_size old_size = MN__STR_GET_SIZE(str);
    if ((err = mn__str_grow(str, old_size + 1))) {
        return err;
    }
    MN__STR_DATA(str)[old_size] = chr;
    MN__STR_CHECK(str);
    return err;
}

mn_size mn__str_slen(const mn_char* s) {
    mn_size out = 0;
    while (*(s++)) {
        out++;
    }
    return out;
}

int mn__str_init_s(mn__str* str, const mn_char* s) {
    int err = 0;
    mn_size i;
    mn_size sz = mn__str_slen(s);
    mn__str_init(str);
    if ((err = mn__str_grow(str, sz))) {
        return err;
    }
    for (i = 0; i < sz; i++) {
        MN__STR_DATA(str)[i] = s[i];
    }
    return err;
}

int mn__str_init_n(mn__str* str, const mn_char* chrs, mn_size n) {
    int err = 0;
    mn_size i;
    mn__str_init(str);
    if ((err = mn__str_grow(str, n))) {
        return err;
    }
    for (i = 0; i < n; i++) {
        MN__STR_DATA(str)[i] = chrs[i];
    }
    return err;
}

int mn__str_cat(mn__str* str, const mn__str* other) {
    int err = 0;
    mn_size i;
    mn_size n = MN__STR_GET_SIZE(other);
    mn_size old_size = MN__STR_GET_SIZE(str);
    if ((err = mn__str_grow(str, old_size + n))) {
        return err;
    }
    /* Copy data */
    for (i = 0; i < n; i++) {
        MN__STR_DATA(str)[old_size + i] = MN__STR_DATA(other)[i];
    }
    MN__STR_CHECK(str);
    return err;
}

int mn__str_cat_n(mn__str* str, const mn_char* chrs, mn_size n) {
    int err = 0;
    mn_size i;
    mn_size old_size = MN__STR_GET_SIZE(str);
    if ((err = mn__str_grow(str, old_size + n))) {
        return err;
    }
    /* Copy data */
    for (i = 0; i < n; i++) {
        MN__STR_DATA(str)[old_size + i] = chrs[i];
    }
    MN__STR_CHECK(str);
    return err;
}

int mn__str_cat_s(mn__str* str, const mn_char* chrs) {
    mn_size chrs_size = mn__str_slen(chrs);
    return mn__str_cat_n(str, chrs, chrs_size);
}

int mn__str_insert(mn__str* str, mn_size index, mn_char chr) {
    int err = 0;
    mn_size i;
    mn_size old_size = MN__STR_GET_SIZE(str);
    /* bounds check */
    MN_ASSERT(index <= MN__STR_GET_SIZE(str));
    if ((err = mn__str_grow(str, old_size + 1))) {
        return err;
    }
    /* Shift data */
    if (old_size != 0) {
        for (i = old_size; i >= index + 1; i--) {
            MN__STR_DATA(str)[i] = MN__STR_DATA(str)[i - 1];
        }
    }
    MN__STR_DATA(str)[index] = chr;
    MN__STR_CHECK(str);
    return err;
}

const mn_char* mn__str_get_data(const mn__str* str) {
    return MN__STR_DATA(str);
}

int mn__str_init_copy(mn__str* str, const mn__str* in) {
    mn_size i;
    int err = 0;
    mn__str_init(str);
    if ((err = mn__str_grow(str, mn__str_size(in)))) {
        return err;
    }
    for (i = 0; i < mn__str_size(str); i++) {
        MN__STR_DATA(str)[i] = MN__STR_DATA(in)[i];
    }
    return err;
}

void mn__str_init_move(mn__str* str, mn__str* old) {
    MN__STR_CHECK(old);
    *str = *old;
    mn__str_init(old);
}

int mn__str_cmp(const mn__str* str_a, const mn__str* str_b) {
    mn_size a_len = mn__str_size(str_a);
    mn_size b_len = mn__str_size(str_b);
    mn_size i = 0;
    const mn_char* a_data = mn__str_get_data(str_a);
    const mn_char* b_data = mn__str_get_data(str_b);
    while (1) {
        if (i == a_len || i == b_len) {
            break;
        }
        if (a_data[i] < b_data[i]) {
            return -1;
        } else if (a_data[i] > b_data[i]) {
            return 1;
        }
        i++;
    }
    if (i == a_len) {
        if (i == b_len) {
            return 0;
        } else {
            return -1;
        }
    }
    return 1;
}

void mn__str_clear(mn__str* str) {
    MN__STR_SET_SIZE(str, 0);
    MN__STR_DATA(str)[0] = '\0';
}

void mn__str_cut_end(mn__str* str, mn_size new_size) {
    MN_ASSERT(new_size <= MN__STR_GET_SIZE(str));
    MN__STR_SET_SIZE(str, new_size);
    MN__STR_DATA(str)[new_size] = '\0';
}

/* bits/container/str_view */
void mn__str_view_init(mn__str_view* view, const mn__str* other) {
    view->_size = mn__str_size(other);
    view->_data = mn__str_get_data(other);
}

void mn__str_view_init_s(mn__str_view* view, const mn_char* chars) {
    view->_size = mn__str_slen(chars);
    view->_data = chars;
}

void mn__str_view_init_n(mn__str_view* view, const mn_char* chars, mn_size n) {
    view->_size = n;
    view->_data = chars;
}

void mn__str_view_init_null(mn__str_view* view) {
    view->_size = 0;
    view->_data = MN_NULL;
}

mn_size mn__str_view_size(const mn__str_view* view) {
    return view->_size;
}

const mn_char* mn__str_view_get_data(const mn__str_view* view) {
    return view->_data;
}

int mn__str_view_cmp(const mn__str_view* view_a, const mn__str_view* view_b) {
    mn_size a_len = mn__str_view_size(view_a);
    mn_size b_len = mn__str_view_size(view_b);
    const mn_char* a_data = mn__str_view_get_data(view_a);
    const mn_char* b_data = mn__str_view_get_data(view_b);
    mn_size i;
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
/* If this fails, you need to define MN_INT32_TYPE to a signed integer type
 * that is 32 bits wide. */
MN__STATIC_ASSERT(mn__int32_is_4_bytes, sizeof(mn_int32) == 4);

/* bits/types/fixed/uint16 */
/* If this fails, you need to define MN_UINT16_TYPE to a unsigned integer type
 * that is 16 bits wide. */
MN__STATIC_ASSERT(mn__uint16_is_2_bytes, sizeof(mn_uint16) == 2);
/* bits/types/fixed/uint32 */
/* If this fails, you need to define MN_UINT32_TYPE to a unsigned integer type
 * that is 32 bits wide. */
MN__STATIC_ASSERT(mn__uint32_is_4_bytes, sizeof(mn_uint32) == 4);
/* bits/types/fixed/uint8 */
/* If this fails, you need to define MN_UINT8_TYPE to a unsigned integer type
 * that is 8 bits wide. */
MN__STATIC_ASSERT(mn__uint8_is_1_bytes, sizeof(mn_uint8) == 1);
/* bits/hooks/memset */
void mn__memset(void* ptr, int value, mn_size count) {
    mn_size i;
    mn_uint8 trunc = (mn_uint8)value;
    for (i = 0; i < count; i++) {
        ((mn_uint8*)ptr)[i] = trunc;
    }
}

/* bits/algorithm/hash/murmur3 */
MN_INTERNAL mn_uint32 mn__murmurhash3_rotl32(mn_uint32 x, mn_uint8 r) {
    return (x << r) | (x >> (32 - r));
}

MN_INTERNAL mn_uint32 mn__murmurhash3_fmix32(mn_uint32 h) {
    h ^= h >> 16;
    h *= 0x85EBCA6B;
    h ^= h >> 13;
    h *= 0xC2B2AE35;
    h ^= h >> 16;
    return h;
}

/* Note: this behaves differently on machines with different endians. */
MN_INTERNAL mn_uint32 mn__murmurhash3_32(mn_uint32 h1, const mn_uint8* data, mn_size data_len) {
    const mn_size num_blocks = data_len / 4;
    const mn_uint32 c1 = 0xCC9E2D51;
    const mn_uint32 c2 = 0x1B873593;
    mn_size i;
    const mn_uint32* blocks = (const mn_uint32*)data;
    const mn_uint8* tail = (const mn_uint8*)(data + num_blocks * 4);
    mn_uint32 k1;
    for (i = 0; i < num_blocks; i++) {
        k1 = blocks[i];
        k1 *= c1;
        k1 = mn__murmurhash3_rotl32(k1, 15);
        k1 *= c2;

        h1 ^= k1;
        h1 = mn__murmurhash3_rotl32(h1, 13);
        h1 = h1 * 5 + 0xE6546B64;
    }

    k1 = 0;
    switch (data_len & 3) {
        case 3:
            k1 ^= ((mn_uint32)(tail[2])) << 16;
            /* fall through */
        case 2:
            k1 ^= ((mn_uint32)(tail[1])) << 8;
            /* fall through */
        case 1:
            k1 ^= tail[0];
            k1 *= c1;
            k1 = mn__murmurhash3_rotl32(k1, 15);
            k1 *= c2;
            h1 ^= k1;
    }

    h1 ^= (mn_uint32)data_len;
    h1 = mn__murmurhash3_fmix32(h1);

    return h1;
}


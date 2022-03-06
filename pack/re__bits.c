
#include "re__bits.h"

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

#endif

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

int re__str_init_n(re__str* str, const re_char* chrs, re_size n) {
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

int re__str_cat_n(re__str* str, const re_char* chrs, re_size n) {
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
    return re__str_cat_n(str, chrs, chrs_size);
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


void re__str_view_init(re__str_view* view, const re__str* other) {
    view->_size = re__str_size(other);
    view->_data = re__str_get_data(other);
}

void re__str_view_init_s(re__str_view* view, const re_char* chars) {
    view->_size = re__str_slen(chars);
    view->_data = chars;
}

void re__str_view_init_n(re__str_view* view, const re_char* chars, re_size n) {
    view->_size = n;
    view->_data = chars;
}

void re__str_view_init_null(re__str_view* view) {
    view->_size = 0;
    view->_data = RE_NULL;
}

re_size re__str_view_size(const re__str_view* view) {
    return view->_size;
}

const re_char* re__str_view_get_data(const re__str_view* view) {
    return view->_data;
}

int re__str_view_cmp(const re__str_view* view_a, const re__str_view* view_b) {
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

RE_INTERNAL void re__zero_mem(re_size size, void* mem) {
    re_size i;
    for (i = 0; i < size; i++) {
        ((re_uint8*)mem)[i] = 0;
    }
}

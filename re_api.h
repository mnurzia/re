#ifndef RE_API_H
#define RE_API_H

#include "pack/re_common_api.h"

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
void re_destroy(re* re);
const char* re_get_error(re* reg, re_size* error_len);

#endif

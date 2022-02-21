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
typedef struct re_match_data re_match_data;

typedef struct re {
    re_data* data;
} re;

typedef struct re_match {
    re_match_data* data;
} re_match;

typedef struct re_span {
    re_size begin;
    re_size end;
} re_span;

re_error re_init(re* re, const char* regex);
void re_destroy(re* re);

const char* re_get_error(re* reg, re_size* error_len);

re_error re_search(re* re, re_match* output);

re_span re_match_get_span(re_match* match, re_uint32 submatch_index);
void re_match_destroy(re_match* match);

#endif

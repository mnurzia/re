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

typedef struct re_span {
    re_size begin;
    re_size end;
} re_span;

re_error re_init(re* reg, const char* regex);
void re_destroy(re* reg);

const char* re_get_error(re* reg, re_size* error_len);

#define RE__MATCH_GROUPS_MAX 10000

typedef enum re_match_anchor_type {
    RE_MATCH_ANCHOR_FULL,
    RE_MATCH_ANCHOR_START,
    RE_MATCH_ANCHOR_END,
    RE_MATCH_UNANCHORED
} re_match_anchor_type;

typedef enum re_match_groups_type {
    RE_MATCH_GROUPS_NONE = -1,
    RE_MATCH_GROUPS_MATCH_BOUNDARIES = 0,
    RE_MATCH_GROUPS_ALL = RE__MATCH_GROUPS_MAX
} re_match_groups_type;

typedef struct re_match_data {
    int matched;
    re_span match_boundaries;
    re_uint32 groups_size;
    re_span* groups;
} re_match_data;

re_error re_match(re* reg, re_match_anchor_type anchor_type, re_match_groups_type groups_type, const char* string, re_size string_size, re_match_data* out);

#endif

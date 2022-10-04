#ifndef RE_API_H
#define RE_API_H

#include "_cpack/api.h"

typedef int re_error;
typedef mn_int32 re_rune;

#define RE_RUNE_MAX 0x10FFFF

#define RE_ERROR_NONE 0
#define RE_ERROR_MATCH 0
#define RE_ERROR_NOMEM -1
#define RE_ERROR_INVALID -2
#define RE_ERROR_PARSE -3
#define RE_ERROR_COMPILE -4
#define RE_ERROR_NOMATCH -5
#define RE_ERROR_INTERNAL -6

#define RE_MATCH 0
#define RE_NOMATCH RE_ERROR_NOMATCH

typedef struct re_data re_data;

typedef struct re {
  re_data* data;
} re;

typedef struct re_span {
  mn_size begin;
  mn_size end;
} re_span;

typedef enum re_anchor_type {
  RE_ANCHOR_BOTH = 'B',
  RE_ANCHOR_START = 'S',
  RE_ANCHOR_END = 'E',
  RE_UNANCHORED = 'U'
} re_anchor_type;

typedef enum re_syntax_flags {
  RE_SYNTAX_FLAG_ASCII = 1,
  RE_SYNTAX_FLAG_IGNORECASE = 2,
  RE_SYNTAX_FLAG_MULTILINE = 4,
  RE_SYNTAX_FLAG_DOTALL = 8,
  RE_SYNTAX_FLAG_GLOB = 16
} re_syntax_flags;

MN_API re_error re_init(re* reg, const char* regex_nt);
MN_API re_error
re_init_flags(re* reg, const char* regex_nt, re_syntax_flags syntax_flags);
MN_API re_error re_init_sz_flags(
    re* reg, const char* regex, mn_size regex_size,
    re_syntax_flags syntax_flags);
MN_API re_error re_init_set(re* reg);

MN_API re_error re_set_add(re* reg, const char* regex_nt);
MN_API re_error
re_set_add_flags(re* reg, const char* regex_nt, re_syntax_flags syntax_flags);
MN_API re_error re_set_add_sz_flags(
    re* reg, const char* regex, mn_size regex_size,
    re_syntax_flags syntax_flags);

#if 0
/* Check whether a given string contains any matches */
MN_API re_error re_is_match(
    re* reg, const char* text, mn_size text_size, re_anchor_type anchor_type);
/* Retrieve group information about the first match in a string */
/* Call with max_group = 1 to get match boundaries, max_group = 2+ subs */
MN_API re_error re_match_groups(
    re* reg, const char* text, mn_size text_size, re_anchor_type anchor_type,
    mn_uint32 max_group, re_span** out_groups);
/* Retrieve group and set information about the first match in a string */
/* Get up to the top max_set matches, the number of set index matches is
 * returned in *out_set_indexes_size, the set indexes are returned in
 * *out_set_indexes.
 * if max_group, *out_groups is a (*out_set_indexes_size) * (max_group) array of
 * spans */
MN_API re_error re_match_full(
    re* reg, const char* text, mn_size text_size, re_anchor_type anchor_type,
    mn_uint32 max_set, mn_uint32** out_set_indexes,
    mn_uint32* out_set_indexes_size, mn_uint32 max_group,
    re_span*** out_groups);
#endif

MN_API re_error re_is_match(
    re* reg, const char* text, mn_size text_size, re_anchor_type anchor_type);
MN_API re_error re_match_groups(
    re* reg, const char* text, mn_size text_size, re_anchor_type anchor_type,
    mn_uint32 max_group, re_span* out_groups);
MN_API re_error re_match_groups_set(
    re* reg, const char* text, mn_size text_size, re_anchor_type anchor_type,
    mn_uint32 max_group, re_span* out_groups, mn_uint32* out_set_index);
MN_API re_error re_match_groups_set_pri(
    re* reg, const char* text, mn_size text_size, re_anchor_type anchor_type,
    mn_uint32 max_group, re_span* out_groups, mn_uint32** out_set_indexes,
    mn_uint32* out_set_indexes_size);

MN_API const char* re_get_error(const re* reg, mn_size* error_len);
MN_API mn_uint32 re_get_max_groups(const re* reg);

MN_API void re_destroy(re* reg);

#if RE_USE_THREAD

typedef struct re_mt_data re_mt_data;

typedef struct re_mt {
  re_mt_data* data;
} re_mt;

MN_API re_error re_mt_init(re_mt* mt, re* source);
MN_API void re_mt_destroy(re_mt* mt);

MN_API const char* re_mt_get_error(const re_mt* reg_mt, mn_size* error_len);

MN_API re_error re_mt_is_match(
    re_mt* reg_mt, const char* text, mn_size text_size,
    re_anchor_type anchor_type);
MN_API re_error re_mt_match_groups(
    re_mt* reg_mt, const char* text, mn_size text_size,
    re_anchor_type anchor_type, mn_uint32 max_group, re_span* out_groups);
MN_API re_error re_mt_match_groups_set(
    re_mt* reg_mt, const char* text, mn_size text_size,
    re_anchor_type anchor_type, mn_uint32 max_group, re_span* out_groups,
    mn_uint32* out_set_index);
MN_API re_error re_mt_match_groups_set_pri(
    re_mt* reg_mt, const char* text, mn_size text_size,
    re_anchor_type anchor_type, mn_uint32 max_group, re_span* out_groups,
    mn_uint32** out_set_indexes, mn_uint32* out_set_indexes_size);

#endif

#endif

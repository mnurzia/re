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
    RE_SYNTAX_FLAG_DOTALL = 8
} re_syntax_flags;

/* Comparison of different matching APIs:
 * PCRE2: 
 *   int pcre2_match(
 *     const pcre2_code *code,
 *     PCRE2_SPTR subject,
 *     PCRE2_SIZE length,
 *     PCRE2_SIZE startoffset,
 *     uint32_t options,
 *     pcre2_match_data *match_data,
 *     pcre2_match_context *mcontext);
 * POSIX:
 *   int regexec(
 *     const regex_t *restrict preg, 
 *     const char *restrict string,
 *     size_t nmatch, 
 *     regmatch_t pmatch[restrict], 
 *     int eflags);
 * GRegex:
 *   gboolean g_regex_match (
 *     const GRegex* regex,
 *     const gchar* string,
 *     GRegexMatchFlags match_options,
 *     GMatchInfo** match_info);
 *   gboolean
 *     g_regex_match_all (
 *     const GRegex* regex,
 *     const gchar* string,
 *     GRegexMatchFlags match_options,
 *     GMatchInfo** match_info);
 *   gboolean
 *     g_regex_match_all_full (
 *     const GRegex* regex,
 *     const gchar* string,
 *     gssize string_len,
 *     gint start_position,
 *     GRegexMatchFlags match_options,
 *     GMatchInfo** match_info,
 *     GError** error);
 *   gboolean
 *     g_regex_match_full (
 *     const GRegex* regex,
 *     const gchar* string,
 *     gssize string_len,
 *     gint start_position,
 *     GRegexMatchFlags match_options,
 *     GMatchInfo** match_info,
 *     GError** error);
 * Hyperscan:
 *   hs_error_t hs_scan(
 *     const hs_database_t *db,
 *     const char *data,
 *     unsigned int length,
 *     unsigned int flags,
 *     hs_scratch_t *scratch,
 *     match_event_handler onEvent,
 *     void *context);
 *   hs_error_t hs_scan_vector(const hs_database_t *db,
 *     const char *const *data,
 *     const unsigned int *length,
 *     unsigned int count, 
 *     unsigned int flags,
 *     hs_scratch_t *scratch,
 *     match_event_handler onEvent, 
 *     void *context);
 * RE2 is omitted because it takes advantage of C++ features.
 */

/* Inputs:
 * Text (always)
 * Text size (optional)
 * Start anchoring (optional, implied anchored)
 * End anchoring (optional, implied anchored)
 * | Match result boolean
 * | Match result boundaries
 * | Match result some groups
 * | Match result all groups
 * Match index (set)
 * Scan options
 *
 * Outputs:
 * Match result boolean -> whether or not the string matched
 * Match result boundaries -> the boundaries of the match
 * Match result some groups -> the boundaries of the match, and optionally group boundaries
 * Match result all groups -> the boundaries of the match and group boundaries
 * Match index -> which index regex was matched
 * Error value
 * 
 */
re_error re_init(re* reg, const char* regex_nt);
re_error re_init_flags(re* reg, const char* regex_nt, re_syntax_flags syntax_flags);
re_error re_init_sz_flags(re* reg, const char* regex, mn_size regex_size, re_syntax_flags syntax_flags);
re_error re_init_set(re* reg);

re_error re_set_add(re* reg, const char* regex_nt);
re_error re_set_add_flags(re* reg, const char* regex_nt, re_syntax_flags syntax_flags);
re_error re_set_add_sz_flags(re* reg, const char* regex, mn_size regex_size, re_syntax_flags syntax_flags);

re_error re_is_match(re* reg, const char* text, mn_size text_size, re_anchor_type anchor_type);
re_error re_match_groups(re* reg, const char* text, mn_size text_size, re_anchor_type anchor_type, mn_uint32 max_group, re_span* out_groups);
re_error re_match_groups_set(re* reg, const char* text, mn_size text_size, re_anchor_type anchor_type, mn_uint32 max_group, re_span* out_groups, mn_uint32* out_set_index);

const char* re_get_error(const re* reg, mn_size* error_len);
mn_uint32 re_get_max_groups(const re* reg);

void re_destroy(re* reg);

#endif

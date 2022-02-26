#include "re_internal.h"

void re__match_data_init(re_match_data* match_data) {
    match_data->matched = 0;
    match_data->groups = RE_NULL;
    match_data->groups_size = 0;
}

void re_match_data_destroy(re_match_data* match_data) {
    if (match_data->groups) {
        RE_FREE(match_data->groups);
    }
}

re_error re__match_data_add_groups(re_match_data* match_data, re_size* sizes, re_uint32 num_groups) {
    re_uint32 i;
    match_data->groups = RE_MALLOC(sizeof(re_span) * num_groups);
    if (match_data->groups == RE_NULL) {
        return RE_ERROR_NOMEM;
    }
    for (i = 0; i < num_groups; i++) {
        re_span span;
        span.begin = sizes[i * 2];
        span.end = sizes[i * 2 +1];
        match_data->groups[i] = span;
    }
    match_data->groups_size = num_groups;
    return RE_ERROR_NONE;
}

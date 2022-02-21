#include "re_internal.h"

RE_VEC_IMPL_FUNC(re_span, init)
RE_VEC_IMPL_FUNC(re_span, destroy)
RE_VEC_IMPL_FUNC(re_span, push)
RE_VEC_IMPL_FUNC(re_span, get)

re_error re__match_init(re_match* match) {
    match->data = RE_MALLOC(sizeof(re_match_data));
    if (match->data == RE_NULL) {
        return RE_ERROR_NOMEM;
    }
    re_span_vec_init(&match->data->submatches);
}

re_span re_match_get_span(re_match* match, re_uint32 submatch_index) {
    return re_span_vec_get(&match->data->submatches, submatch_index);
}

void re_match_destroy(re_match* match) {
    re_span_vec_destroy(&match->data->submatches);
    RE_FREE(match->data);
}

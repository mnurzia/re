#include "re_internal.h"

MN__VEC_IMPL_FUNC(re__exec_dfa_state, init)
MN__VEC_IMPL_FUNC(re__exec_dfa_state, destroy)
MN__VEC_IMPL_FUNC(re__prog_loc, init)
MN__VEC_IMPL_FUNC(re__prog_loc, destroy)

#define RE__EXEC_DFA_STATE_NONE 0

void re__exec_dfa_init(re__exec_dfa* exec) {
    re__exec_dfa_state_vec_init(&exec->states);
    re__prog_loc_vec_init(&exec->thrd_locs);
}

void re__exec_dfa_destroy(re__exec_dfa* exec) {
    re__prog_loc_vec_destroy(&exec->thrd_locs);
    re__exec_dfa_state_vec_destroy(&exec->states);
}

void re__exec_dfa_state_init(re__exec_dfa_state* state, mn_uint32 thrd_locs_begin, mn_uint32 thrd_locs_end) {
    mn_uint32 i;
    for (i = 0; i < 256; i++) {
        state->next[i] = RE__EXEC_DFA_STATE_NONE;
    }
    state->match_idx = 0;
    state->thrd_locs_begin = thrd_locs_begin;
    state->thrd_locs_end = thrd_locs_end;
}

re_error re__exec_dfa_do(re__exec_nfa* exec, re__prog* prog, mn__str_view str_view, mn_uint32* out_end, mn_uint32* out_match) {
    MN__UNUSED(exec);
    MN__UNUSED(prog);
    MN__UNUSED(str_view);
    MN__UNUSED(out_end);
    MN__UNUSED(out_match);
    return 0;
}

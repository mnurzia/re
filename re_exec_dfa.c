#include "re_internal.h"

MN__VEC_IMPL_FUNC(re__exec_dfa_state_ptr, init)
MN__VEC_IMPL_FUNC(re__exec_dfa_state_ptr, destroy)
MN__VEC_IMPL_FUNC(re__exec_dfa_state_ptr, push)
MN__VEC_IMPL_FUNC(re__exec_dfa_state_ptr, size)
MN__VEC_IMPL_FUNC(re__exec_dfa_state_ptr, get)

MN__VEC_IMPL_FUNC(mn_uint32_ptr, init)
MN__VEC_IMPL_FUNC(mn_uint32_ptr, destroy)
MN__VEC_IMPL_FUNC(mn_uint32_ptr, push)
MN__VEC_IMPL_FUNC(mn_uint32_ptr, size)
MN__VEC_IMPL_FUNC(mn_uint32_ptr, get)

void re__exec_dfa_init(re__exec_dfa* exec, const re__prog* prog) {
    exec->current_state = MN_NULL;
    re__exec_dfa_state_ptr_vec_init(&exec->state_pages);
    exec->state_page_idx = 0;
    mn_uint32_ptr_vec_init(&exec->thrd_loc_pages);
    exec->thrd_loc_page_idx = 0;
    re__exec_nfa_init(&exec->nfa, prog, RE_MATCH_GROUPS_NONE);
    exec->cache = MN_NULL;
    exec->cache_stored = 0;
    exec->cache_alloc = 0;
}

void re__exec_dfa_destroy(re__exec_dfa* exec) {
    if (exec->cache) {
        MN_FREE(exec->cache);
    }
    re__exec_nfa_destroy(&exec->nfa);
    {
        mn_size i;
        for (i = 0; i < mn_uint32_ptr_vec_size(&exec->thrd_loc_pages); i++) {
            MN_FREE(mn_uint32_ptr_vec_get(&exec->thrd_loc_pages, i));
        }
    }
    mn_uint32_ptr_vec_destroy(&exec->thrd_loc_pages);
    re__exec_dfa_state_ptr_vec_destroy(&exec->state_pages);
}

void re__exec_dfa_state_init(re__exec_dfa_state* state, mn_uint32* thrd_locs_begin, mn_uint32* thrd_locs_end) {
    mn_uint32 i;
    for (i = 0; i < 256; i++) {
        state->next[i] = MN_NULL;
    }
    state->assert_ctx = 0;
    state->match_idx = 0;
    state->thrd_locs_begin = thrd_locs_begin;
    state->thrd_locs_end = thrd_locs_end;
}

re_error re__exec_dfa_stash_loc_set(re__exec_dfa* exec, const re__exec_thrd* thrds, mn_size thrds_size, mn_uint32** out_locs_begin, mn_uint32** out_locs_end) {
    re_error err = RE_ERROR_NONE;
    mn_uint32* thrds_page;
    /* we should handle empty states elsewhere */
    MN_ASSERT(thrds_size != 0);
    if (exec->thrd_loc_page_idx != 0) {
        mn_uint32 logical_page_idx = exec->thrd_loc_page_idx % RE__EXEC_DFA_THRD_LOCS_PAGE_SIZE;
        mn_uint32 space_left = RE__EXEC_DFA_THRD_LOCS_PAGE_SIZE - logical_page_idx;
        MN_ASSERT(mn_uint32_ptr_vec_size(&exec->thrd_loc_pages));
        /* space left in previous page */
        if (space_left >= thrds_size) {
            mn_uint32* last_page = mn_uint32_ptr_vec_get(
                &exec->thrd_loc_pages, mn_uint32_ptr_vec_size(&exec->thrd_loc_pages) - 1);
            thrds_page = last_page + exec->thrd_loc_page_idx;
        } else {
            /* need a new page */
            /* fallthrough to next if block */
            exec->thrd_loc_page_idx = 0;
        }
    }
    if (exec->thrd_loc_page_idx == 0) {
        /* need to allocate a page */
        /* round down to figure out how many pages to allocate */
        mn_size pages_needed = (thrds_size + RE__EXEC_DFA_THRD_LOCS_PAGE_SIZE) / RE__EXEC_DFA_THRD_LOCS_PAGE_SIZE;
        thrds_page = MN_MALLOC(sizeof(mn_size) * RE__EXEC_DFA_THRD_LOCS_PAGE_SIZE * pages_needed);
        if (thrds_page == MN_NULL) {
            return RE_ERROR_NOMEM;
        }
        if ((err = mn_uint32_ptr_vec_push(&exec->thrd_loc_pages, thrds_page))) {
            return err;
        }
    }
    {
        /* assert that there is enough space left */
        MN_ASSERT(RE__EXEC_DFA_THRD_LOCS_PAGE_SIZE - (exec->thrd_loc_page_idx % RE__EXEC_DFA_THRD_LOCS_PAGE_SIZE) >= thrds_size);
    }
    {
        mn_size i;
        *out_locs_begin = thrds_page;
        for (i = 0; i < thrds_size; i++) {
            re__prog_loc loc = thrds[i].loc;
            thrds_page[i] = loc;
        }
        *out_locs_end = thrds_page + i;
        exec->thrd_loc_page_idx += thrds_size;
        if (exec->thrd_loc_page_idx % RE__EXEC_DFA_THRD_LOCS_PAGE_SIZE == 0) {
            /* exhausted this page, indicate a reallocation for the next time */
            exec->thrd_loc_page_idx = 0;
        }
    }
    return err;
}

re_error re__exec_dfa_get_new_state(re__exec_dfa* exec, const re__exec_thrd* thrds, mn_size thrds_size, re__exec_dfa_state** out_state) {
    re_error err = RE_ERROR_NONE;
    re__exec_dfa_state* page;
    mn_uint32* state_thrds_begin;
    mn_uint32* state_thrds_end;
    if (exec->state_page_idx == 0) {
        /* need to allocate a new page */
        page = MN_MALLOC(sizeof(re__exec_dfa_state) * RE__EXEC_DFA_PAGE_SIZE);
        if (page == MN_NULL) {
            return RE_ERROR_NOMEM;
        }
        if ((err = re__exec_dfa_state_ptr_vec_push(&exec->state_pages, page))) {
            return err;
        }
    } else {
        page = re__exec_dfa_state_ptr_vec_get(&exec->state_pages,
            re__exec_dfa_state_ptr_vec_size(&exec->state_pages) - 1);
    }
    *out_state = page + exec->state_page_idx;
    if ((err = re__exec_dfa_stash_loc_set(exec, thrds, thrds_size, &state_thrds_begin, &state_thrds_end))) {
        return err;
    }
    re__exec_dfa_state_init(*out_state, state_thrds_begin, state_thrds_end);
    if (exec->state_page_idx == RE__EXEC_DFA_PAGE_SIZE) {
        exec->state_page_idx = 0;
    }
    return err;
}

#define RE__EXEC_DFA_CACHE_INITIAL_SIZE 16

int re__exec_dfa_state_equal(const re__exec_thrd* thrds, re__prog_loc thrds_size, re__exec_dfa_state* state) {
    mn_size i;
    for (i = 0; i < thrds_size; i++) {
        if (state->thrd_locs_begin[i] != thrds[i].loc) {
            return 0;
            break;
        }
    }
    return 1;
}

re_error re__exec_dfa_get_state(re__exec_dfa* exec, re__ast_assert_type assert_ctx, re__exec_dfa_state** out_state) {
    re_error err = RE_ERROR_NONE;
    re__prog_loc thrds_size;
    const re__exec_thrd* thrds = re__exec_nfa_get_thrds(&exec->nfa, &thrds_size);
    mn_uint32 hash = 0;
    if (thrds_size == 0) {
        /* no threads, no match */
        out_state = MN_NULL;
        return err;
    }
    /* ok to cast to uint8, no ambiguous padding inside of assert_ctx */
    hash = mn__murmurhash3_32(hash, (const mn_uint8*)&assert_ctx, sizeof(assert_ctx));
    {
        mn_size i;
        for (i = 0; i < thrds_size; i++) {
            /* ok to cast to uint8, also no ambiguous padding */
            hash = mn__murmurhash3_32(hash, (const mn_uint8*)&thrds[i].loc, sizeof(re__prog_loc));
        }
    }
    /* lookup in cache */
    if (!exec->cache) {
        /* cache has not been initialized */
        exec->cache_alloc = RE__EXEC_DFA_CACHE_INITIAL_SIZE;
        exec->cache = MN_MALLOC(sizeof(re__exec_dfa_cache_entry) * exec->cache_alloc);
        mn__memset(exec->cache, 0, sizeof(re__exec_dfa_cache_entry) * exec->cache_alloc);
        if (exec->cache == MN_NULL) {
            return RE_ERROR_NOMEM;
        }
    }
    {
        /* lookup in cache */
        re__exec_dfa_cache_entry* probe = exec->cache + (hash % exec->cache_alloc);
        mn_size q = 1;
        while (1) {
            if (probe->state_ptr == MN_NULL) {
                /* not found, but slot is empty so we can use that */
                break;
            } else {
                if (probe->hash == hash) {
                    /* collision or found */
                    re__exec_dfa_state* state = probe->state_ptr;
                    /* check if state is equal */
                    if (state->assert_ctx == assert_ctx
                        && (state->thrd_locs_end - state->thrd_locs_begin) == thrds_size
                        && re__exec_dfa_state_equal(thrds, thrds_size, state)) {
                        *out_state = state;
                        return RE_ERROR_NONE;
                    }
                }
            }
            /* otherwise, find a new slot */
            probe = exec->cache + (((q * q) + hash) % exec->cache_alloc);
            q++;
        }
        /* if not found in cache, we are here */
        /* store in cache */
        {
            re__exec_dfa_state* new_state;
            if ((err = re__exec_dfa_get_new_state(exec, thrds, thrds_size, &new_state))) {
                return err;
            }
            probe->hash = hash;
            probe->state_ptr = new_state;
            exec->cache_stored++;
            *out_state = new_state;
        }
        /* check load factor and resize if necessary (0.75 in this case) */
        /* calculation: x - 0.25 * x == 0.75 * x    |    (0.25*x == x >> 2) */
        if (exec->cache_stored == exec->cache_alloc - (exec->cache_alloc >> 2)) {
            /* need to resize */
            mn_size old_alloc = exec->cache_alloc;
            mn_size i;
            re__exec_dfa_cache_entry* old_cache = exec->cache;
            exec->cache_alloc *= 2;
            exec->cache = MN_MALLOC(sizeof(re__exec_dfa_cache_entry) * exec->cache_alloc);
            mn__memset(exec->cache, 0, sizeof(re__exec_dfa_cache_entry) * exec->cache_alloc);
            if (exec->cache == MN_NULL) {
                return RE_ERROR_NOMEM;
            }
            /* rehash */
            for (i = 0; i < old_alloc; i++) {
                re__exec_dfa_cache_entry* old_entry = old_cache + i;
                probe = exec->cache + (old_entry->hash % exec->cache_alloc);
                q = 1;
                while (probe->state_ptr != MN_NULL) {
                    probe = exec->cache + (((q * q) + old_entry->hash) % exec->cache_alloc);
                    q++;
                }
                *probe = *old_entry;
            }
            MN_FREE(old_cache);
        }
    }
    {
        /* check load factor and resize if necessary (0.75 in this case) */
        /* calculation: x - 0.25 * x == 0.75 * x    |    (0.25*x == x >> 2) */
        if (exec->cache_stored == exec->cache_alloc - (exec->cache_alloc >> 2)) {
            /* need to resize */
            mn_size old_alloc = exec->cache_alloc;
            mn_size i;
            re__exec_dfa_cache_entry* old_cache = exec->cache;
            exec->cache_alloc *= 2;
            exec->cache = MN_MALLOC(sizeof(re__exec_dfa_cache_entry) * exec->cache_alloc);
            mn__memset(exec->cache, 0, sizeof(re__exec_dfa_cache_entry) * exec->cache_alloc);
            if (exec->cache == MN_NULL) {
                return RE_ERROR_NOMEM;
            }
            /* rehash */
            for (i = 0; i < old_alloc; i++) {
                mn_size q = 1;
                re__exec_dfa_cache_entry* old_entry = old_cache + i;
                re__exec_dfa_cache_entry* probe = exec->cache + (old_entry->hash % exec->cache_alloc);
                while (probe->state_ptr != MN_NULL) {
                    probe = exec->cache + (((q * q) + old_entry->hash) % exec->cache_alloc);
                    q++;
                }
                *probe = *old_entry;
            }
            MN_FREE(old_cache);
        }
    }
    return err;
}

/* returns NULL-ness of current state. */
re_error re__exec_dfa_start(re__exec_dfa* exec, re__ast_assert_type assert_ctx, re__prog_loc start_loc) {
    re_error err = RE_ERROR_NONE;
    if ((err = re__exec_nfa_start(&exec->nfa, assert_ctx, start_loc))) {
        return err;
    }
    /* nfa now contains the states we need. */
    /* initialize the current state. */
    if ((err = re__exec_dfa_get_state(exec, assert_ctx, &exec->current_state))) {
        return err;
    }
    return !!(exec->current_state);
}

re_error re__exec_dfa_run(re__exec_dfa* exec, mn_char ch, re__ast_assert_type assert_ctx) {
    MN__UNUSED(ch);
    MN__UNUSED(assert_ctx);
    /* for now, ensure it's not null */
    MN_ASSERT(exec->current_state != MN_NULL);
    return 0; 
}

re_error re__exec_dfa_do(re__exec_nfa* exec, re__prog* prog, mn__str_view str_view, mn_uint32* out_end, mn_uint32* out_match) {
    MN__UNUSED(exec);
    MN__UNUSED(prog);
    MN__UNUSED(str_view);
    MN__UNUSED(out_end);
    MN__UNUSED(out_match);
    return 0;
}

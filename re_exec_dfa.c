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

#define RE__EXEC_DFA_STATE_INVALID (MN_NULL + 1)

void re__exec_dfa_init(re__exec_dfa* exec, const re__prog* prog) {
    exec->current_state = MN_NULL;
    {
        int i;
        for (i = 0; i < RE__EXEC_DFA_START_STATE_COUNT * RE__PROG_ENTRY_MAX; i++) {
            exec->start_states[i] = MN_NULL;
        }
    }
    re__exec_dfa_state_ptr_vec_init(&exec->state_pages);
    exec->state_page_idx = 0;
    mn_uint32_ptr_vec_init(&exec->thrd_loc_pages);
    exec->thrd_loc_page_idx = 0;
    re__exec_nfa_init(&exec->nfa, prog, 0);
    exec->cache = MN_NULL;
    exec->cache_stored = 0;
    exec->cache_alloc = 0;
    exec->prev_sym = 0;
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
    {
        mn_size i;
        for (i = 0; i < re__exec_dfa_state_ptr_vec_size(&exec->state_pages); i++) {
            MN_FREE(re__exec_dfa_state_ptr_vec_get(&exec->state_pages, i));
        }
    }
    re__exec_dfa_state_ptr_vec_destroy(&exec->state_pages);
}

void re__exec_dfa_state_init(re__exec_dfa_state* state, mn_uint32* thrd_locs_begin, mn_uint32* thrd_locs_end) {
    mn_uint32 i;
    for (i = 0; i < RE__EXEC_SYM_MAX; i++) {
        state->next[i] = MN_NULL;
    }
    state->flags = 0;
    state->match_index = 0;
    state->match_priority = 0;
    state->thrd_locs_begin = thrd_locs_begin;
    state->thrd_locs_end = thrd_locs_end;
    state->empty = state->thrd_locs_begin == state->thrd_locs_end;
}

re_error re__exec_dfa_stash_loc_set(re__exec_dfa* exec, const re__exec_thrd* thrds, mn_size thrds_size, mn_uint32** out_locs_begin, mn_uint32** out_locs_end) {
    re_error err = RE_ERROR_NONE;
    mn_uint32* thrds_page;
    if (thrds_size == 0) {
        /* set to NULL, it's fine because these will never be dereferenced */
        *out_locs_begin = MN_NULL;
        *out_locs_end = MN_NULL;
    }
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

int re__exec_dfa_state_equal(const re__exec_thrd* thrds, re__prog_loc thrds_size, re__exec_dfa_flags flags, re__exec_dfa_state* state) {
    mn_size i;
    if (state->flags != flags) {
        return 0;
    }
    if ((state->thrd_locs_end - state->thrd_locs_begin) != thrds_size) {
        return 0;
    }
    for (i = 0; i < thrds_size; i++) {
        if (state->thrd_locs_begin[i] != thrds[i].loc) {
            return 0;
            break;
        }
    }
    return 1;
}

re_error re__exec_dfa_get_state(re__exec_dfa* exec, re__exec_dfa_flags flags, re__exec_dfa_state** out_state) {
    re_error err = RE_ERROR_NONE;
    re__prog_loc thrds_size = re__exec_nfa_get_thrds_size(&exec->nfa);
    const re__exec_thrd* thrds = re__exec_nfa_get_thrds(&exec->nfa);
    mn_uint32 match_index = re__exec_nfa_get_match_index(&exec->nfa);
    mn_uint32 match_priority = re__exec_nfa_get_match_priority(&exec->nfa);
    mn_uint32 hash = 0;
    if (match_index) {
        flags |= RE__EXEC_DFA_FLAG_MATCH;
    }
    if (match_priority) {
        flags |= RE__EXEC_DFA_FLAG_MATCH_PRIORITY;
    }
    /* ok to cast to uint8, no ambiguous padding inside of flags */
    hash = mn__murmurhash3_32(hash, (const mn_uint8*)&flags, sizeof(flags));
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
                    if (re__exec_dfa_state_equal(thrds, thrds_size, flags, state)) {
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
            new_state->flags = flags;
            new_state->match_index = match_index;
            new_state->match_priority = match_priority;
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
    return err;
}
/*
re__ast_assert_type re__exec_dfa_get_assert_ctx(mn_uint32 left_char, mn_uint32 right_char) {
    re__ast_assert_type out = 0;
    if (left_char == RE__EXEC_SYM_SOT) {
        out |= RE__AST_ASSERT_TYPE_TEXT_START_ABSOLUTE;
        out |= RE__AST_ASSERT_TYPE_TEXT_START;
    }
    if (left_char == '\n') {
        out |= RE__AST_ASSERT_TYPE_TEXT_START;
    }
    if (right_char == '\n') {
        out |= RE__AST_ASSERT_TYPE_TEXT_END;
    }
    if (right_char == RE__EXEC_SYM_EOT) {
        out |= RE__AST_ASSERT_TYPE_TEXT_END_ABSOLUTE;
        out |= RE__AST_ASSERT_TYPE_TEXT_END;
    }
}
*/

re_error re__exec_dfa_start(re__exec_dfa* exec, re__prog_entry entry, re__exec_dfa_start_state_flags start_state_flags) {
    re_error err = RE_ERROR_NONE;
    unsigned int start_state_idx = start_state_flags + (entry * RE__EXEC_DFA_START_STATE_COUNT);
    re__exec_dfa_state** start_state = &exec->start_states[start_state_idx];
    MN_ASSERT(entry < RE__PROG_ENTRY_MAX);
    if (*start_state == MN_NULL) {
        re__exec_dfa_flags dfa_flags = RE__EXEC_DFA_FLAG_START_STATE;
        if (start_state_flags & RE__EXEC_DFA_START_STATE_FLAG_AFTER_WORD) {
            dfa_flags |= RE__EXEC_DFA_FLAG_FROM_WORD;
        }
        if (start_state_flags & RE__EXEC_DFA_START_STATE_FLAG_BEGIN_LINE) {
            dfa_flags |= RE__EXEC_DFA_FLAG_START_STATE_BEGIN_LINE;
        }
        if (start_state_flags & RE__EXEC_DFA_START_STATE_FLAG_BEGIN_TEXT) {
            dfa_flags |= RE__EXEC_DFA_FLAG_START_STATE_BEGIN_TEXT;
        }
        if ((err = re__exec_nfa_start(&exec->nfa, entry))) {
            return err;
        }
        if ((err = re__exec_dfa_get_state(exec, dfa_flags, start_state))) {
            return err;
        }
        exec->start_states[start_state_idx]->start_entry = entry;
    }
    exec->current_state = exec->start_states[start_state_idx];
    return err;
}

re_error re__exec_dfa_run(re__exec_dfa* exec, mn_uint32 next_sym) {
    re__exec_dfa_state* current_state = exec->current_state;
    re__exec_dfa_state* next_state;
    re_error err = RE_ERROR_NONE;
    /* for now, ensure it's not null */
    MN_ASSERT(current_state != MN_NULL);
    MN_ASSERT(next_sym <= RE__EXEC_SYM_EOT);
    next_state = current_state->next[next_sym];
    if (next_state == MN_NULL) {
        re__assert_type assert_ctx = 0;
        int is_word_boundary;
        re__exec_dfa_flags new_flags = 0;
        if (current_state->flags & RE__EXEC_DFA_FLAG_START_STATE) {
            is_word_boundary = re__is_word_boundary_start(next_sym);
        } else {
            is_word_boundary = re__is_word_boundary(!!(current_state->flags & RE__EXEC_DFA_FLAG_FROM_WORD), next_sym);
        }
        if (is_word_boundary) {
            assert_ctx |= RE__ASSERT_TYPE_WORD;
        } else {
            assert_ctx |= RE__ASSERT_TYPE_WORD_NOT;
        }
        if (next_sym == RE__EXEC_SYM_EOT) {
            assert_ctx |= RE__ASSERT_TYPE_TEXT_END_ABSOLUTE;
            assert_ctx |= RE__ASSERT_TYPE_TEXT_END;
        }
        if (current_state->flags & RE__EXEC_DFA_FLAG_START_STATE_BEGIN_LINE) {
            assert_ctx |= RE__ASSERT_TYPE_TEXT_START;
        }
        if (current_state->flags & RE__EXEC_DFA_FLAG_START_STATE_BEGIN_TEXT) {
            assert_ctx |= RE__ASSERT_TYPE_TEXT_START_ABSOLUTE;
        }
        re__exec_nfa_set_thrds(
            &exec->nfa, current_state->thrd_locs_begin, 
            (re__prog_loc)(current_state->thrd_locs_end - current_state->thrd_locs_begin)
        );
        re__exec_nfa_set_match_index(&exec->nfa, current_state->match_index);
        re__exec_nfa_set_match_priority(&exec->nfa, current_state->match_priority);
        if ((err = re__exec_nfa_run_byte(&exec->nfa, assert_ctx, next_sym, 0))) {
            return err;
        }
        if (re__is_word_char(next_sym)) {
            new_flags |= RE__EXEC_DFA_FLAG_FROM_WORD;
        }
        /* if (is_word_char(sym)) dfa_flags |= from_word */
        if ((err = re__exec_dfa_get_state(exec, new_flags, &next_state))) {
            return err;
        }
        current_state->next[next_sym] = next_state;
    }
    exec->prev_sym = next_sym;
    exec->current_state = next_state;
    return 0;
}

MN_INTERNAL mn_uint32 re__exec_dfa_get_match_index(re__exec_dfa* exec) {
    return exec->current_state->match_index;
}

MN_INTERNAL mn_uint32 re__exec_dfa_get_match_priority(re__exec_dfa* exec) {
    return exec->current_state->match_priority;
}

MN_INTERNAL int re__exec_dfa_get_exhaustion(re__exec_dfa* exec) {
    return (exec->current_state->thrd_locs_end - exec->current_state->thrd_locs_begin) == 0;
}

#if MN_DEBUG

#include <stdio.h>

MN_INTERNAL void re__exec_dfa_debug_dump_state_idx(int sym) {
    if (sym < RE__EXEC_SYM_EOT) {
        re__byte_debug_dump((mn_uint8)sym);
    } else {
        printf("<max>");
    }
}

MN_INTERNAL void re__exec_dfa_debug_dump(re__exec_dfa* exec) {
    const re__exec_dfa_state* state = exec->current_state;
    printf("---------------------------------\n");
    printf("DFA State Debug Dump (%p, dfa: %p):\n", (void*)state, (void*)exec);
    if (state == MN_NULL) {
        printf("  NULL STATE\n");
        return;
    }
    if (state == (re__exec_dfa_state*)RE__EXEC_DFA_STATE_INVALID) {
        printf("  INVALID STATE\n");
        return;
    }
    printf("  Flags: 0x%04X\n", state->flags);
    printf("  Match Index: %i\n", state->match_index);
    printf("  Match Priority: %i\n", state->match_priority);
    printf("  Threads: %u\n    ", (unsigned int)(state->thrd_locs_end - state->thrd_locs_begin));
    {
        mn_uint32* thrd_ptr = state->thrd_locs_begin;
        while (thrd_ptr != state->thrd_locs_end) {
            printf("%04X ", *thrd_ptr);
            thrd_ptr++;
        }
        printf("\n");
    }
    printf("  Nexts:\n");
    {
        int sym;
        for (sym = 0; sym < RE__EXEC_SYM_MAX; sym++) {
            re__exec_dfa_state* next = state->next[sym];
            if (next != MN_NULL) {
                printf("    [");
                re__exec_dfa_debug_dump_state_idx(sym);
                printf("]: %p\n", (void*)next);
            }
        }
    }
    printf("---------------------------------\n");
}

#endif

/* note:
 * start state is taken on re__exec_dfa_start.
 * then, when ch is taken, call re__exec_nfa_start with appropriate params.
 * this accomodates the only zero-width match (the empty string.)
 * DFA match is delayed by one byte. */

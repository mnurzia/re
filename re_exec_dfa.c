#include "re_internal.h"

void re__exec_dfa_state_init(
    re__exec_dfa_state* state, mn_uint32* thrd_locs_begin,
    mn_uint32* thrd_locs_end)
{
  mn_uint32 i;
  for (i = 0; i < RE__EXEC_SYM_MAX; i++) {
    state->next[i] = MN_NULL;
  }
  state->flags = 0;
  state->thrd_locs_begin = thrd_locs_begin;
  state->thrd_locs_end = thrd_locs_end;
  state->match_index = 0;
  state->hash = 0;
  state->uniq = 0;
}

#define RE__EXEC_DFA_PTR_MASK (~0x3)

MN_INTERNAL re__exec_dfa_state*
re__exec_dfa_state_get_next(re__exec_dfa_state* state, mn_uint8 sym)
{
#if !RE__EXEC_DFA_SMALL_STATE
  return state->next[sym];
#else
  return state->next[sym] & RE__EXEC_DFA_PTR_MASK;
#endif
}

MN_INTERNAL int re__exec_dfa_state_is_match(re__exec_dfa_state* state)
{
  return state->flags & RE__EXEC_DFA_FLAG_MATCH;
}

MN_INTERNAL int re__exec_dfa_state_is_priority(re__exec_dfa_state* state)
{
  return !(state->flags & RE__EXEC_DFA_FLAG_MATCH_PRIORITY);
}

MN_INTERNAL int re__exec_dfa_state_is_empty(re__exec_dfa_state* state)
{
  return (state->thrd_locs_end - state->thrd_locs_begin) == 0;
}

MN_INTERNAL mn_uint32
re__exec_dfa_state_get_match_index(re__exec_dfa_state* state)
{
  return state->match_index;
}

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

MN_INTERNAL re_error
re__exec_dfa_cache_init(re__exec_dfa_cache* cache, const re__prog* prog)
{
  re_error err = RE_ERROR_NONE;
  re__exec_nfa_init(&cache->nfa, prog, 0);
  {
    int i;
    for (i = 0; i < RE__EXEC_DFA_START_STATE_COUNT * RE__PROG_ENTRY_MAX; i++) {
      cache->start_states[i] = MN_NULL;
    }
  }
  re__exec_dfa_state_ptr_vec_init(&cache->state_pages);
  cache->state_page_idx = 0;
  mn_uint32_ptr_vec_init(&cache->thrd_loc_pages);
  cache->thrd_loc_page_idx = 0;
  cache->cache = MN_NULL;
  cache->cache_stored = 0;
  cache->cache_alloc = 0;
  cache->uniq = 0;
#if RE_USE_THREAD
  mn__memset(&cache->read_try_mutex, 0, sizeof(mn__mutex));
  mn__memset(&cache->read_mutex, 0, sizeof(mn__mutex));
  mn__memset(&cache->write_mutex, 0, sizeof(mn__mutex));
  mn__memset(&cache->cache_mutex, 0, sizeof(mn__mutex));
  if ((err = mn__mutex_init(&cache->read_try_mutex))) {
    goto error;
  }
  if ((err = mn__mutex_init(&cache->read_mutex))) {
    goto error;
  }
  if ((err = mn__mutex_init(&cache->write_mutex))) {
    goto error;
  }
  if ((err = mn__mutex_init(&cache->cache_mutex))) {
    goto error;
  }
#endif
error:
  return err;
}

MN_INTERNAL void re__exec_dfa_cache_destroy(re__exec_dfa_cache* cache)
{
  if (cache->cache) {
    MN_FREE(cache->cache);
  }
  re__exec_nfa_destroy(&cache->nfa);
  {
    mn_size i;
    for (i = 0; i < mn_uint32_ptr_vec_size(&cache->thrd_loc_pages); i++) {
      MN_FREE(mn_uint32_ptr_vec_get(&cache->thrd_loc_pages, i));
    }
  }
  mn_uint32_ptr_vec_destroy(&cache->thrd_loc_pages);
  {
    mn_size i;
    for (i = 0; i < re__exec_dfa_state_ptr_vec_size(&cache->state_pages); i++) {
      MN_FREE(re__exec_dfa_state_ptr_vec_get(&cache->state_pages, i));
    }
  }
  re__exec_dfa_state_ptr_vec_destroy(&cache->state_pages);
#if RE_USE_THREAD
  mn__mutex_destroy(&cache->read_try_mutex);
  mn__mutex_destroy(&cache->read_mutex);
  mn__mutex_destroy(&cache->write_mutex);
  mn__mutex_destroy(&cache->cache_mutex);
#endif
}

re_error re__exec_dfa_cache_stash_loc_set(
    re__exec_dfa_cache* cache, const re__exec_thrd* thrds, mn_size thrds_size,
    mn_uint32** out_locs_begin, mn_uint32** out_locs_end)
{
  re_error err = RE_ERROR_NONE;
  mn_uint32* thrds_page;
  if (thrds_size == 0) {
    /* set to NULL, it's fine because these will never be dereferenced */
    *out_locs_begin = MN_NULL;
    *out_locs_end = MN_NULL;
  }
  if (cache->thrd_loc_page_idx != 0) {
    mn_uint32 logical_page_idx =
        cache->thrd_loc_page_idx % RE__EXEC_DFA_THRD_LOCS_PAGE_SIZE;
    mn_uint32 space_left = RE__EXEC_DFA_THRD_LOCS_PAGE_SIZE - logical_page_idx;
    MN_ASSERT(mn_uint32_ptr_vec_size(&cache->thrd_loc_pages));
    /* space left in previous page */
    if (space_left >= thrds_size) {
      mn_uint32* last_page = mn_uint32_ptr_vec_get(
          &cache->thrd_loc_pages,
          mn_uint32_ptr_vec_size(&cache->thrd_loc_pages) - 1);
      thrds_page = last_page + cache->thrd_loc_page_idx;
    } else {
      /* need a new page */
      /* fallthrough to next if block */
      cache->thrd_loc_page_idx = 0;
    }
  }
  if (cache->thrd_loc_page_idx == 0) {
    /* need to allocate a page */
    /* round down to figure out how many pages to allocate */
    mn_size pages_needed = (thrds_size + RE__EXEC_DFA_THRD_LOCS_PAGE_SIZE) /
                           RE__EXEC_DFA_THRD_LOCS_PAGE_SIZE;
    thrds_page = MN_MALLOC(
        sizeof(mn_size) * RE__EXEC_DFA_THRD_LOCS_PAGE_SIZE * pages_needed);
    if (thrds_page == MN_NULL) {
      return RE_ERROR_NOMEM;
    }
    if ((err = mn_uint32_ptr_vec_push(&cache->thrd_loc_pages, thrds_page))) {
      MN_FREE(thrds_page);
      return err;
    }
  }
  {
    /* assert that there is enough space left */
    MN_ASSERT(
        RE__EXEC_DFA_THRD_LOCS_PAGE_SIZE -
            (cache->thrd_loc_page_idx % RE__EXEC_DFA_THRD_LOCS_PAGE_SIZE) >=
        thrds_size);
  }
  {
    mn_size i;
    *out_locs_begin = thrds_page;
    for (i = 0; i < thrds_size; i++) {
      re__prog_loc loc = thrds[i].loc;
      thrds_page[i] = loc;
    }
    *out_locs_end = thrds_page + i;
    cache->thrd_loc_page_idx += thrds_size;
    if (cache->thrd_loc_page_idx % RE__EXEC_DFA_THRD_LOCS_PAGE_SIZE == 0) {
      /* exhausted this page, indicate a reallocation for the next time */
      cache->thrd_loc_page_idx = 0;
    }
  }
  return err;
}

re_error re__exec_dfa_cache_get_new_state(
    re__exec_dfa_cache* cache, const re__exec_thrd* thrds, mn_size thrds_size,
    re__exec_dfa_state** out_state)
{
  re_error err = RE_ERROR_NONE;
  re__exec_dfa_state* page;
  mn_uint32* state_thrds_begin;
  mn_uint32* state_thrds_end;
  if (cache->state_page_idx == 0) {
    /* need to allocate a new page */
    page = MN_MALLOC(sizeof(re__exec_dfa_state) * RE__EXEC_DFA_PAGE_SIZE);
    if (page == MN_NULL) {
      return RE_ERROR_NOMEM;
    }
    if ((err = re__exec_dfa_state_ptr_vec_push(&cache->state_pages, page))) {
      MN_FREE(page);
      return err;
    }
  } else {
    page = re__exec_dfa_state_ptr_vec_get(
        &cache->state_pages,
        re__exec_dfa_state_ptr_vec_size(&cache->state_pages) - 1);
  }
  *out_state = page + cache->state_page_idx;
  if ((err = re__exec_dfa_cache_stash_loc_set(
           cache, thrds, thrds_size, &state_thrds_begin, &state_thrds_end))) {
    return err;
  }
  re__exec_dfa_state_init(*out_state, state_thrds_begin, state_thrds_end);
  if (cache->state_page_idx == RE__EXEC_DFA_PAGE_SIZE) {
    cache->state_page_idx = 0;
  }
  return err;
}

#define RE__EXEC_DFA_CACHE_INITIAL_SIZE 16

int re__exec_dfa_state_equal(
    const re__exec_thrd* thrds, re__prog_loc thrds_size,
    re__exec_dfa_flags flags, re__exec_dfa_state* state)
{
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

re_error re__exec_dfa_cache_get_state(
    re__exec_dfa_cache* cache, re__exec_dfa_flags flags,
    re__exec_dfa_state** out_state)
{
  re_error err = RE_ERROR_NONE;
  re__prog_loc thrds_size = re__exec_nfa_get_thrds_size(&cache->nfa);
  const re__exec_thrd* thrds = re__exec_nfa_get_thrds(&cache->nfa);
  mn_uint32 match_index = re__exec_nfa_get_match_index(&cache->nfa);
  mn_uint32 match_priority = re__exec_nfa_get_match_priority(&cache->nfa);
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
      hash = mn__murmurhash3_32(
          hash, (const mn_uint8*)&thrds[i].loc, sizeof(re__prog_loc));
    }
  }
  /* lookup in cache */
  if (!cache->cache) {
    /* cache has not been initialized */
    cache->cache_alloc = RE__EXEC_DFA_CACHE_INITIAL_SIZE;
    cache->cache = MN_MALLOC(sizeof(re__exec_dfa_state*) * cache->cache_alloc);
    if (cache->cache == MN_NULL) {
      return RE_ERROR_NOMEM;
    }
    mn__memset(
        cache->cache, 0, sizeof(re__exec_dfa_state*) * cache->cache_alloc);
  }
  {
    /* lookup in cache */
    re__exec_dfa_state** probe = cache->cache + (hash % cache->cache_alloc);
    mn_size q = 1;
    while (1) {
      if (*probe == MN_NULL) {
        /* not found, but slot is empty so we can use that */
        break;
      } else {
        if ((*probe)->hash == hash) {
          /* collision or found */
          re__exec_dfa_state* state = (*probe);
          /* check if state is equal */
          if (re__exec_dfa_state_equal(thrds, thrds_size, flags, state)) {
            *out_state = state;
            return RE_ERROR_NONE;
          }
        }
      }
      /* otherwise, find a new slot */
      probe = cache->cache + ((q + hash) % cache->cache_alloc);
      q++;
    }
    /* if not found in cache, we are here */
    /* store in cache */
    {
      re__exec_dfa_state* new_state;
      if ((err = re__exec_dfa_cache_get_new_state(
               cache, thrds, thrds_size, &new_state))) {
        return err;
      }
      /* Set input flags (start state, etc) */
      new_state->flags |= flags;
      new_state->match_index = match_index;
      *probe = new_state;
      (*probe)->hash = hash;
      cache->cache_stored++;
      *out_state = new_state;
    }
    /* check load factor and resize if necessary (0.75 in this case) */
    /* calculation: x - 0.25 * x == 0.75 * x    |    (0.25*x == x >> 2) */
    if (cache->cache_stored ==
        (cache->cache_alloc - (cache->cache_alloc >> 2))) {
      /* need to resize */
      mn_size old_alloc = cache->cache_alloc;
      mn_size i;
      re__exec_dfa_state** old_cache = cache->cache;
      cache->cache_alloc *= 2;
      cache->cache =
          MN_MALLOC(sizeof(re__exec_dfa_state*) * cache->cache_alloc);
      if (cache->cache == MN_NULL) {
        return RE_ERROR_NOMEM;
      }
      mn__memset(
          cache->cache, 0, sizeof(re__exec_dfa_state*) * cache->cache_alloc);
      /* rehash */
      for (i = 0; i < old_alloc; i++) {
        re__exec_dfa_state** old_entry = old_cache + i;
        if (*old_entry == MN_NULL) {
          continue;
        }
        probe = cache->cache + ((*old_entry)->hash % cache->cache_alloc);
        q = 1;
        while (*probe != MN_NULL) {
          probe =
              cache->cache + ((q + (*old_entry)->hash) % cache->cache_alloc);
          q++;
        }
        *probe = *old_entry;
      }
      MN_FREE(old_cache);
    }
  }
  return err;
}

MN_INTERNAL re_error re__exec_dfa_cache_construct_start(
    re__exec_dfa_cache* cache, re__prog_entry entry,
    re__exec_dfa_start_state_flags start_state_flags,
    re__exec_dfa_state_ptr* out)
{
  re_error err = RE_ERROR_NONE;
  unsigned int start_state_idx =
      start_state_flags + (entry * RE__EXEC_DFA_START_STATE_COUNT);
  re__exec_dfa_state** start_state = &cache->start_states[start_state_idx];
  MN_ASSERT(entry < RE__PROG_ENTRY_MAX);
  if (*start_state == MN_NULL) {
    re__exec_dfa_flags dfa_flags = 0;
    if (start_state_flags & RE__EXEC_DFA_START_STATE_FLAG_AFTER_WORD) {
      dfa_flags |= RE__EXEC_DFA_FLAG_FROM_WORD;
    }
    if (start_state_flags & RE__EXEC_DFA_START_STATE_FLAG_BEGIN_LINE) {
      dfa_flags |= RE__EXEC_DFA_FLAG_BEGIN_LINE;
    }
    if (start_state_flags & RE__EXEC_DFA_START_STATE_FLAG_BEGIN_TEXT) {
      dfa_flags |= RE__EXEC_DFA_FLAG_BEGIN_TEXT;
    }
    if ((err = re__exec_nfa_start(&cache->nfa, entry))) {
      goto error;
    }
    if ((err = re__exec_dfa_cache_get_state(cache, dfa_flags, start_state))) {
      goto error;
    }
  }
  *out = *start_state;
error:
  return err;
}

MN_INTERNAL re_error re__exec_dfa_cache_construct(
    re__exec_dfa_cache* cache, re__exec_dfa_state_ptr state, mn_uint32 symbol,
    re__exec_dfa_state_ptr* out)
{
  re_error err = RE_ERROR_NONE;
  re__assert_type assert_ctx = 0;
  unsigned int is_word_boundary;
  re__exec_dfa_flags new_flags = 0;
  re__exec_dfa_state_ptr* next_state = &state->next[symbol];
  is_word_boundary = re__is_word_boundary(
      !!(state->flags & RE__EXEC_DFA_FLAG_FROM_WORD), symbol);
  if (is_word_boundary) {
    assert_ctx |= RE__ASSERT_TYPE_WORD;
  } else {
    assert_ctx |= RE__ASSERT_TYPE_WORD_NOT;
  }
  if (symbol == RE__EXEC_SYM_EOT) {
    assert_ctx |= RE__ASSERT_TYPE_TEXT_END_ABSOLUTE;
    assert_ctx |= RE__ASSERT_TYPE_TEXT_END;
  }
  if (state->flags & RE__EXEC_DFA_FLAG_BEGIN_LINE) {
    assert_ctx |= RE__ASSERT_TYPE_TEXT_START;
  }
  if (state->flags & RE__EXEC_DFA_FLAG_BEGIN_TEXT) {
    assert_ctx |= RE__ASSERT_TYPE_TEXT_START_ABSOLUTE;
  }
  re__exec_nfa_set_thrds(
      &cache->nfa, state->thrd_locs_begin,
      (re__prog_loc)(state->thrd_locs_end - state->thrd_locs_begin));
  re__exec_nfa_set_match_index(&cache->nfa, state->match_index);
  re__exec_nfa_set_match_priority(
      &cache->nfa, state->flags & RE__EXEC_DFA_FLAG_MATCH_PRIORITY);
  if ((err = re__exec_nfa_run_byte(&cache->nfa, assert_ctx, symbol, 0))) {
    return err;
  }
  if (re__is_word_char(symbol)) {
    new_flags |= RE__EXEC_DFA_FLAG_FROM_WORD;
  }
  if (symbol == '\n') {
    assert_ctx |= RE__ASSERT_TYPE_TEXT_END;
    new_flags |= RE__EXEC_DFA_FLAG_BEGIN_LINE;
  }
  if ((err = re__exec_dfa_cache_get_state(cache, new_flags, next_state))) {
    return err;
  }
  *out = *next_state;
  return err;
}

re_error re__exec_dfa_cache_construct_end(
    re__exec_dfa_cache* cache, re__exec_dfa_state_ptr current_state,
    re__exec_dfa_state_ptr* out)
{
  re__exec_dfa_state* next_state;
  re_error err = RE_ERROR_NONE;
  /* for now, ensure it's not null */
  MN_ASSERT(current_state != MN_NULL);
  next_state = current_state->next[RE__EXEC_SYM_EOT];
  if (next_state == MN_NULL) {
    if ((err = re__exec_dfa_cache_construct(
             cache, current_state, RE__EXEC_SYM_EOT, out))) {
      return err;
    }
  } else {
    *out = next_state;
  }
  return 0;
}

/* Need to keep track of:
 * - Reversed
 * - Boolean match (check priority bit or not)
 * - Can exit early from boolean matches */
re_error re__exec_dfa_cache_driver(
    re__exec_dfa_cache* cache, re__prog_entry entry, int boolean_match,
    int boolean_match_exit_early, int reversed, const mn_uint8* text,
    mn_size text_size, mn_size text_start_pos, mn_uint32* out_match,
    mn_size* out_pos)
{
  re__exec_dfa_start_state_flags start_state_flags = 0;
  re_error err = RE_ERROR_NONE;
  re__exec_dfa_state* current_state;
  if (!reversed) {
    if (text_start_pos == 0) {
      start_state_flags |= RE__EXEC_DFA_START_STATE_FLAG_BEGIN_TEXT |
                           RE__EXEC_DFA_START_STATE_FLAG_BEGIN_LINE;
    } else {
      start_state_flags |=
          (RE__EXEC_DFA_START_STATE_FLAG_AFTER_WORD *
           re__is_word_char((unsigned char)(text[text_start_pos - 1])));
      start_state_flags |=
          (unsigned int)RE__EXEC_DFA_START_STATE_FLAG_BEGIN_LINE *
          (unsigned int)(text[text_start_pos - 1] == '\n');
    }
  } else {
    if (text_start_pos == text_size) {
      start_state_flags |= RE__EXEC_DFA_START_STATE_FLAG_BEGIN_TEXT |
                           RE__EXEC_DFA_START_STATE_FLAG_BEGIN_LINE;
    } else {
      start_state_flags |=
          (RE__EXEC_DFA_START_STATE_FLAG_AFTER_WORD *
           re__is_word_char((unsigned char)(text[text_start_pos])));
      start_state_flags |=
          (unsigned int)RE__EXEC_DFA_START_STATE_FLAG_BEGIN_LINE *
          (unsigned int)(text[text_start_pos] == '\n');
    }
  }
  if ((err = re__exec_dfa_cache_construct_start(
           cache, entry, start_state_flags, &current_state))) {
    return err;
  }
  MN_ASSERT(text_start_pos <= text_size);
  MN_ASSERT(MN__IMPLIES(boolean_match, out_match == MN_NULL));
  MN_ASSERT(MN__IMPLIES(boolean_match, out_pos == MN_NULL));
  MN_ASSERT(MN__IMPLIES(!boolean_match, boolean_match_exit_early == 0));
  {
    const mn_uint8* start;
    const mn_uint8* last_found_start;
    const mn_uint8* end;
    const mn_uint8* loop_out_pos;
    mn_uint32 last_found_match = 0;
    mn_uint32 loop_out_index;
    re__exec_dfa_state_ptr next_state;
    start = text + text_start_pos;
    if (!reversed) {
      end = text + text_size;
    } else {
      end = text;
    }
    while (1) {
      if (start == end) {
        break;
      }
      if (reversed) {
        start--;
      }
      next_state = current_state->next[*start];
      if (next_state == MN_NULL) {
        if ((err = re__exec_dfa_cache_construct(
                 cache, current_state, *start, &current_state))) {
          return err;
        }
      } else {
        current_state = next_state;
      }
      if (boolean_match) {
        if (boolean_match_exit_early) {
          if (re__exec_dfa_state_is_match(current_state)) {
            err = RE_MATCH;
            goto loop_exit_match;
          }
        }
      } else {
        if (re__exec_dfa_state_is_match(current_state)) {
          last_found_match = re__exec_dfa_state_get_match_index(current_state);
          last_found_start = start;
          if (re__exec_dfa_state_is_priority(current_state)) {
            loop_out_pos = last_found_start;
            loop_out_index = last_found_match;
            err = RE_MATCH;
            goto loop_exit_match;
          } else if (
              re__exec_dfa_state_is_empty(current_state) && last_found_match) {
            loop_out_pos = last_found_start;
            loop_out_index = last_found_match;
            err = RE_MATCH;
            goto loop_exit_match;
          }
        }
      }
      if (!reversed) {
        start++;
      }
    }
    err = RE_ERROR_NOMATCH;
  loop_exit_match:
#if 0
    static re_error (*funcs[8])(
        re__exec_dfa*, const mn_uint8*, const mn_uint8*, const mn_uint8**,
        mn_uint32*) = {
        re__exec_dfa_search_fff,
        re__exec_dfa_search_fft,
        MN_NULL,
        MN_NULL,
        re__exec_dfa_search_tff,
        re__exec_dfa_search_tft,
        re__exec_dfa_search_ttf,
        re__exec_dfa_search_ttt};
    err = funcs
        [reversed | (boolean_match_exit_early << 1) | (boolean_match << 2)](
            exec, start, end, &loop_out_pos, &loop_out_index);
#endif
    if (err == RE_MATCH) {
      /* Exited early */
      if (boolean_match) {
        return err;
      } else {
        MN_ASSERT(loop_out_pos >= text);
        MN_ASSERT(loop_out_pos < text + text_size);
        if (!reversed) {
          *out_pos = (mn_size)(loop_out_pos - text);
        } else {
          *out_pos = (mn_size)(loop_out_pos - text) + 1;
        }
        *out_match = loop_out_index;
        return err;
      }
    } else if (err != RE_ERROR_NOMATCH) {
      return err;
    }
    if ((err = re__exec_dfa_cache_construct_end(
             cache, current_state, &current_state))) {
      return err;
    }
    if (re__exec_dfa_state_is_match(current_state)) {
      if (!boolean_match) {
        *out_pos = (mn_size)(end - text);
        *out_match = re__exec_dfa_state_get_match_index(current_state);
      }
      return RE_MATCH;
    } else {
      return RE_NOMATCH;
    }
  }
}

#if 0

#include <stdio.h>

MN_INTERNAL void re__exec_dfa_debug_dump_state_idx(int sym)
{
  if (sym < RE__EXEC_SYM_EOT) {
    re__byte_debug_dump((mn_uint8)sym);
  } else {
    printf("<max>");
  }
}

MN_INTERNAL void re__exec_dfa_debug_dump(re__exec_dfa* exec)
{
  const re__exec_dfa_state* state = exec->current_state;
  printf("---------------------------------\n");
  printf("DFA State Debug Dump (%p, dfa: %p):\n", (void*)state, (void*)exec);
  if (state == MN_NULL) {
    printf("  NULL STATE\n");
    return;
  }
  printf(
      "  Flags: %c%c%c%c%c\n",
      (state->flags & RE__EXEC_DFA_FLAG_FROM_WORD ? 'W' : '-'),
      (state->flags & RE__EXEC_DFA_FLAG_BEGIN_TEXT ? 'A' : '-'),
      (state->flags & RE__EXEC_DFA_FLAG_BEGIN_LINE ? '^' : '-'),
      (state->flags & RE__EXEC_DFA_FLAG_MATCH ? 'M' : '-'),
      (state->flags & RE__EXEC_DFA_FLAG_MATCH_PRIORITY ? 'P' : '-'));
  printf("  Match Index: %i\n", state->match_index);
  printf(
      "  Match Priority: %i\n",
      state->flags & RE__EXEC_DFA_FLAG_MATCH_PRIORITY);
  printf(
      "  Threads: %u\n    ",
      (unsigned int)(state->thrd_locs_end - state->thrd_locs_begin));
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

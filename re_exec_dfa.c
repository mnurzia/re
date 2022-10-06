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

MN_INTERNAL re_error re__exec_dfa_cache_init(re__exec_dfa_cache* cache)
{
  re_error err = RE_ERROR_NONE;
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
  mn__memset(&cache->rwlock, 0, sizeof(re__rwlock));
  if ((err = re__rwlock_init(&cache->rwlock))) {
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
  re__rwlock_destroy(&cache->rwlock);
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
    re__exec_dfa_cache* cache, re__exec_dfa_state** out_state, re__exec* exec)
{
  re_error err = RE_ERROR_NONE;
  re__prog_loc thrds_size = re__exec_nfa_get_thrds_size(&exec->nfa);
  const re__exec_thrd* thrds = re__exec_nfa_get_thrds(&exec->nfa);
  mn_uint32 hash = exec->dfa_state_hash;
  MN__UNUSED(exec);
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
          if (re__exec_dfa_state_equal(
                  thrds, thrds_size, exec->dfa_state_flags, state)) {
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
      new_state->flags |= exec->dfa_state_flags;
      new_state->match_index = exec->dfa_state_match_index;
      new_state->hash = hash;
      new_state->uniq = cache->uniq++;
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

void re__exec_dfa_derive(re__exec* exec)
{
  mn_uint32 hash = 0;
  /* ok to cast to uint8, no ambiguous padding inside of flags */
  exec->dfa_state_match_index = re__exec_nfa_get_match_index(&exec->nfa);
  exec->dfa_state_match_priority = re__exec_nfa_get_match_priority(&exec->nfa);
  if (exec->dfa_state_match_index) {
    exec->dfa_state_flags |= RE__EXEC_DFA_FLAG_MATCH;
  }
  if (exec->dfa_state_match_priority) {
    exec->dfa_state_flags |= RE__EXEC_DFA_FLAG_MATCH_PRIORITY;
  }
  hash = mn__murmurhash3_32(
      hash, (const mn_uint8*)&exec->dfa_state_flags,
      sizeof(exec->dfa_state_flags));
  {
    mn_size i;
    mn_size sz = re__exec_nfa_get_thrds_size(&exec->nfa);
    const re__exec_thrd* thrds = re__exec_nfa_get_thrds(&exec->nfa);
    for (i = 0; i < sz; i++) {
      /* ok to cast to uint8, also no ambiguous padding */
      hash = mn__murmurhash3_32(
          hash, (const mn_uint8*)&thrds[i].loc, sizeof(re__prog_loc));
    }
  }
  exec->dfa_state_hash = hash;
}

re_error re__exec_dfa_construct_start(
    re__exec* exec, re__prog_entry entry,
    re__exec_dfa_start_state_flags start_state_flags)
{
  re_error err = RE_ERROR_NONE;
  exec->dfa_state_flags = 0;
  if (start_state_flags & RE__EXEC_DFA_START_STATE_FLAG_AFTER_WORD) {
    exec->dfa_state_flags |= RE__EXEC_DFA_FLAG_FROM_WORD;
  }
  if (start_state_flags & RE__EXEC_DFA_START_STATE_FLAG_BEGIN_LINE) {
    exec->dfa_state_flags |= RE__EXEC_DFA_FLAG_BEGIN_LINE;
  }
  if (start_state_flags & RE__EXEC_DFA_START_STATE_FLAG_BEGIN_TEXT) {
    exec->dfa_state_flags |= RE__EXEC_DFA_FLAG_BEGIN_TEXT;
  }
  if ((err = re__exec_nfa_start(&exec->nfa, entry))) {
    return err;
  }
  re__exec_dfa_derive(exec);
  return err;
}

re_error re__exec_dfa_construct(
    re__exec* exec, re__exec_dfa_state_ptr current_state, mn_uint32 symbol)
{
  re_error err = RE_ERROR_NONE;
  re__assert_type assert_ctx = 0;
  unsigned int is_word_boundary;
  is_word_boundary = re__is_word_boundary(
      !!(current_state->flags & RE__EXEC_DFA_FLAG_FROM_WORD), symbol);
  if (is_word_boundary) {
    assert_ctx |= RE__ASSERT_TYPE_WORD;
  } else {
    assert_ctx |= RE__ASSERT_TYPE_WORD_NOT;
  }
  if (symbol == RE__EXEC_SYM_EOT) {
    assert_ctx |= RE__ASSERT_TYPE_TEXT_END_ABSOLUTE;
    assert_ctx |= RE__ASSERT_TYPE_TEXT_END;
  }
  if (current_state->flags & RE__EXEC_DFA_FLAG_BEGIN_LINE) {
    assert_ctx |= RE__ASSERT_TYPE_TEXT_START;
  }
  if (current_state->flags & RE__EXEC_DFA_FLAG_BEGIN_TEXT) {
    assert_ctx |= RE__ASSERT_TYPE_TEXT_START_ABSOLUTE;
  }
  re__exec_nfa_set_thrds(
      &exec->nfa, current_state->thrd_locs_begin,
      (re__prog_loc)(current_state->thrd_locs_end - current_state->thrd_locs_begin));
  re__exec_nfa_set_match_index(&exec->nfa, current_state->match_index);
  re__exec_nfa_set_match_priority(
      &exec->nfa, current_state->flags & RE__EXEC_DFA_FLAG_MATCH_PRIORITY);
  if ((err = re__exec_nfa_run_byte(&exec->nfa, assert_ctx, symbol, 0))) {
    return err;
  }
  exec->dfa_state_flags = 0;
  if (re__is_word_char(symbol)) {
    exec->dfa_state_flags |= RE__EXEC_DFA_FLAG_FROM_WORD;
  }
  if (symbol == '\n') {
    assert_ctx |= RE__ASSERT_TYPE_TEXT_END;
    exec->dfa_state_flags |= RE__EXEC_DFA_FLAG_BEGIN_LINE;
  }
  re__exec_dfa_derive(exec);
  return err;
}

re_error
re__exec_dfa_construct_end(re__exec* exec, re__exec_dfa_state_ptr current_state)
{
  return re__exec_dfa_construct(exec, current_state, RE__EXEC_SYM_EOT);
}

void re__exec_dfa_temp_save(
    re__exec* exec, re__exec_dfa_state_ptr current_state)
{
  re__exec_nfa_set_thrds(
      &exec->nfa, current_state->thrd_locs_begin,
      (re__prog_loc)(current_state->thrd_locs_end - current_state->thrd_locs_begin));
  re__exec_nfa_set_match_index(&exec->nfa, current_state->match_index);
  re__exec_nfa_set_match_priority(
      &exec->nfa, current_state->flags & RE__EXEC_DFA_FLAG_MATCH_PRIORITY);
  exec->dfa_state_flags = current_state->flags;
  exec->dfa_state_hash = current_state->hash;
}

re__exec_dfa_state_ptr
re__exec_dfa_cache_lookup(re__exec_dfa_cache* cache, re__exec* exec)
{
  re__exec_dfa_state** probe =
      cache->cache + (exec->dfa_state_hash % cache->cache_alloc);
  mn_size q = 1;
  while (1) {
    if (*probe) {
      return NULL;
    }
    if ((*probe)->hash == exec->dfa_state_hash) {
      /* collision or found */
      re__exec_dfa_state* state = (*probe);
      if (re__exec_dfa_state_equal(
              re__exec_nfa_get_thrds(&exec->nfa),
              re__exec_nfa_get_thrds_size(&exec->nfa), exec->dfa_state_flags,
              state)) {
        return state;
      }
    }
    /* otherwise, find a new slot */
    probe = cache->cache + ((q + exec->dfa_state_hash) % cache->cache_alloc);
    q++;
  }
}

#if RE_USE_THREAD

void re__exec_dfa_crit_reader_enter(re__exec_dfa_cache* cache)
{
  re__rwlock_rlock(&cache->rwlock);
}

void re__exec_dfa_crit_reader_exit(re__exec_dfa_cache* cache)
{
  re__rwlock_runlock(&cache->rwlock);
}

void re__exec_dfa_crit_writer_enter(re__exec_dfa_cache* cache)
{
  re__rwlock_wlock(&cache->rwlock);
}

void re__exec_dfa_crit_writer_exit(re__exec_dfa_cache* cache)
{
  re__rwlock_wunlock(&cache->rwlock);
}

#endif

#define RE__EXEC_DFA_LOCK_BLOCK_SIZE 32

/* In its current state, the DFA relies pretty heavily on speculative execution
 * because it checks all flags at its most inner loop. On a Mac M1, the CPU does
 * a really good job, other processors may vary. I could do what RE2 did and
 * rewrite inlined versions of this loop, but I'd prefer not to and if I did it
 * would probably involve code-gen (Python scripts generating C) rather than
 * using macros or something. */
re_error re__exec_dfa_cache_driver(
    re__exec_dfa_cache* cache, re__prog_entry entry, const mn_uint8* text,
    mn_size text_size, mn_size text_start_pos, mn_uint32* out_match,
    mn_size* out_pos, re__exec_dfa_run_flags run_flags, re__exec* exec)
{
  re_error err = RE_ERROR_NONE;
  re__exec_dfa_state* current_state;
  /* Cache int flags here */
  int boolean_match = run_flags & RE__EXEC_DFA_RUN_FLAG_BOOLEAN_MATCH;
  int boolean_match_exit_early =
      run_flags & RE__EXEC_DFA_RUN_FLAG_BOOLEAN_MATCH_EXIT_EARLY;
  int reversed = run_flags & RE__EXEC_DFA_RUN_FLAG_REVERSED;
  int locked = run_flags & RE__EXEC_DFA_RUN_FLAG_LOCKED;
  re__exec_dfa_start_state_flags start_state_flags = 0;
  unsigned int start_state_idx;
  /* Obvious OOB check */
  MN_ASSERT(text_start_pos <= text_size);
  /* Boolean matches never return a match index */
  MN_ASSERT(MN__IMPLIES(boolean_match, out_match == MN_NULL));
  /* Boolean matches never return a position */
  MN_ASSERT(MN__IMPLIES(boolean_match, out_pos == MN_NULL));
  /* Keep flags normalized, this one isn't strictly necessary */
  MN_ASSERT(MN__IMPLIES(!boolean_match, boolean_match_exit_early == 0));
#if !RE_USE_THREAD
  /* invoke DCE */
  locked = 0;
#endif
  /* Determine the start state flags for construction */
  if (!reversed) {
    /* Forward execution */
    if (text_start_pos == 0) {
      /* Absolute start */
      start_state_flags |= RE__EXEC_DFA_START_STATE_FLAG_BEGIN_TEXT |
                           RE__EXEC_DFA_START_STATE_FLAG_BEGIN_LINE;
    } else {
      /* After word / line character (cannot occur at start) */
      start_state_flags |=
          (RE__EXEC_DFA_START_STATE_FLAG_AFTER_WORD *
           re__is_word_char((unsigned char)(text[text_start_pos - 1])));
      start_state_flags |=
          (unsigned int)RE__EXEC_DFA_START_STATE_FLAG_BEGIN_LINE *
          (unsigned int)(text[text_start_pos - 1] == '\n');
    }
  } else {
    /* Reversed execution */
    if (text_start_pos == text_size) {
      /* Absolute end */
      start_state_flags |= RE__EXEC_DFA_START_STATE_FLAG_BEGIN_TEXT |
                           RE__EXEC_DFA_START_STATE_FLAG_BEGIN_LINE;
    } else {
      /* After word / line character (cannot occur at end) */
      start_state_flags |=
          (RE__EXEC_DFA_START_STATE_FLAG_AFTER_WORD *
           re__is_word_char((unsigned char)(text[text_start_pos])));
      start_state_flags |=
          (unsigned int)RE__EXEC_DFA_START_STATE_FLAG_BEGIN_LINE *
          (unsigned int)(text[text_start_pos] == '\n');
    }
  }
  /* Compute index into cache->start_states */
  start_state_idx =
      start_state_flags + (entry * RE__EXEC_DFA_START_STATE_COUNT);
  if (locked) {
    /* Hold cache as a reader */
    re__exec_dfa_crit_reader_enter(cache);
  }
  if (!cache->start_states[start_state_idx]) {
    /* Need to construct start state */
    /* Use NFA to construct, this can be done in reader thread */
    if ((err = re__exec_dfa_construct_start(exec, entry, start_state_flags))) {
      goto reader_exit;
    }
    do { /* Analagous to a compare-swap loop, kinda? */
      /* Construction complete */
      if (locked) {
        /* Switch to writer */
        re__exec_dfa_crit_reader_exit(cache);
        re__exec_dfa_crit_writer_enter(cache);
      }
      /* Cache the state */
      if ((err = re__exec_dfa_cache_get_state(cache, &current_state, exec))) {
        goto writer_error;
      }
      /* Write it to the start state table */
      cache->start_states[start_state_idx] = current_state;
      if (locked) {
        /* Switch to reader and lookup in cache */
        re__exec_dfa_crit_writer_exit(cache);
        re__exec_dfa_crit_reader_enter(cache);
        current_state = re__exec_dfa_cache_lookup(cache, exec);
      }
      /* Keep trying to get the state */
    } while (locked && !current_state);
  } else {
    /* Grab start state, it's ready - no need to write to cache */
    current_state = cache->start_states[start_state_idx];
  }
  {
    /* -- Main loop -- */
    /* Current processed character */
    const mn_uint8* start;
    /* The last location a match was found */
    const mn_uint8* last_found_start;
    /* Final character (!reversed ? start + text_size : start - text_size) */
    const mn_uint8* end;
    /* Set to the position that will be returned */
    const mn_uint8* loop_out_pos;
    /* Last match index, correlates with last_found_start */
    mn_uint32 last_found_match = 0;
    /* Set to the match index that will be returned */
    mn_uint32 loop_out_index;
    /* For threading: How many characters have been processed since the last
     * time the writers were allowed to breathe */
    mn_uint32 block_idx = 0;
    /* Next state (temporary variable) */
    re__exec_dfa_state_ptr next_state;

    /* Compute start/end locations */
    start = text + text_start_pos;
    if (!reversed) {
      end = text + text_size;
    } else {
      end = text;
    }
    while (1) {
      /* Break if done */
      if (start == end) {
        break;
      }
      /* Pre-decrement if we're reversing (this is how it's done) */
      if (reversed) {
        start--;
      }
      if (!(next_state = current_state->next[*start])) {
        /* Need to construct next state */
        if ((err = re__exec_dfa_construct(exec, current_state, *start))) {
          goto reader_exit;
        }
        do {
          if (locked) {
            /* Switch to writer */
            re__exec_dfa_crit_reader_exit(cache);
            re__exec_dfa_crit_writer_enter(cache);
          }
          /* Cache the state */
          if ((err = re__exec_dfa_cache_get_state(cache, &next_state, exec))) {
            goto writer_error;
          }
          /* Write back to the current state */
          current_state->next[*start] = next_state;
          if (locked) {
            /* Switch to reader */
            re__exec_dfa_crit_writer_exit(cache);
            re__exec_dfa_crit_reader_enter(cache);
            next_state = re__exec_dfa_cache_lookup(cache, exec);
          }
        } while (locked && !next_state);
      }
      /* Advance state */
      current_state = next_state;
      if (boolean_match) {
        if (boolean_match_exit_early) {
          if (re__exec_dfa_state_is_match(current_state)) {
            /* We found *a* match, but not *the* match. Doesn't matter though,
             * all we need to do is report if there is *a* match. */
            err = RE_MATCH;
            goto loop_exit_match;
          }
        }
      } else {
        if (re__exec_dfa_state_is_match(current_state)) {
          /* Record the index of this match and its starting position */
          last_found_match = re__exec_dfa_state_get_match_index(current_state);
          last_found_start = start;
          if (re__exec_dfa_state_is_priority(current_state)) {
            /* If this is the highest-priority match, then we can exit */
            loop_out_pos = last_found_start;
            loop_out_index = last_found_match;
            err = RE_MATCH;
            goto loop_exit_match;
          } else if (
              re__exec_dfa_state_is_empty(current_state) && last_found_match) {
            /* OR, if there are no more threads left, and we previously found a
             * match, we can exit */
            loop_out_pos = last_found_start;
            loop_out_index = last_found_match;
            err = RE_MATCH;
            goto loop_exit_match;
          }
        }
      }
      /* Forwards-running: increment text pointer */
      if (!reversed) {
        start++;
      }
      if (locked) {
        /* Writer anti-starvation logic, in theory, this should allow writers
         * to get a chance at the cache */
        block_idx++;
        if (block_idx == RE__EXEC_DFA_LOCK_BLOCK_SIZE) {
          re__exec_dfa_temp_save(exec, current_state);
          /* Let the writers breathe */
          re__exec_dfa_crit_reader_exit(cache);
          re__exec_dfa_crit_reader_enter(cache);
          current_state = re__exec_dfa_cache_lookup(cache, exec);
          while (!current_state) {
            /* Switch to writer */
            re__exec_dfa_crit_reader_exit(cache);
            re__exec_dfa_crit_writer_enter(cache);
            if ((err = re__exec_dfa_cache_get_state(
                     cache, &current_state, exec))) {
              goto writer_error;
            }
            re__exec_dfa_crit_writer_exit(cache);
            re__exec_dfa_crit_reader_enter(cache);
            current_state = re__exec_dfa_cache_lookup(cache, exec);
          }
          block_idx = 0;
        }
      }
    } /* end main loop */
    /* If we exited this loop normally, no suitable match was ever found. */
    err = RE_ERROR_NOMATCH;
  loop_exit_match:
    if (err == RE_MATCH) {
      /* Exited early */
      if (boolean_match) {
        /* Boolean: we can just head out and report *a* match */
        goto reader_exit;
      } else {
        /* These asserts ensure we can cast to mn_size in the following lines */
        MN_ASSERT(loop_out_pos >= text);
        MN_ASSERT(loop_out_pos < text + text_size);
        /* Normalize the out position */
        if (!reversed) {
          *out_pos = (mn_size)(loop_out_pos - text);
        } else {
          *out_pos = (mn_size)(loop_out_pos - text) + 1;
        }
        *out_match = loop_out_index;
        goto reader_exit;
      }
    } else if (err != RE_ERROR_NOMATCH) {
      /* Some other error occurred - report that */
      goto reader_exit;
    }
    /* If we are here, err = RE_ERROR_NOMATCH, and we will try to execute the
     * end state */
    if (!(next_state = current_state->next[RE__EXEC_SYM_EOT])) {
      /* Need to construct next state */
      if ((err = re__exec_dfa_construct_end(exec, current_state))) {
        goto reader_exit;
      }
      do {
        if (locked) {
          /* Switch to writer */
          re__exec_dfa_crit_reader_exit(cache);
          re__exec_dfa_crit_writer_enter(cache);
        }
        /* Cache the state */
        if ((err = re__exec_dfa_cache_get_state(cache, &next_state, exec))) {
          goto writer_error;
        }
        /* Write back to the current state */
        current_state->next[RE__EXEC_SYM_EOT] = next_state;
        if (locked) {
          /* Switch to reader */
          re__exec_dfa_crit_writer_exit(cache);
          re__exec_dfa_crit_reader_enter(cache);
          next_state = re__exec_dfa_cache_lookup(cache, exec);
        }
      } while (locked && !next_state);
    }
    current_state = next_state;
    if (re__exec_dfa_state_is_match(current_state)) {
      /* Matched on the end state */
      if (!boolean_match) {
        /* Works for both reverse and forwards */
        *out_pos = (mn_size)(end - text);
        *out_match = re__exec_dfa_state_get_match_index(current_state);
      }
      err = RE_MATCH;
      goto reader_exit;
    } else {
      /* Didn't match on the end state */
      err = RE_NOMATCH;
      goto reader_exit;
    }
  }
  /* Unreachable fallthrough - all previous branches go to reader_exit */
writer_error:
  /* This label is only reached if we encounter an error during a writer crit */
  if (locked) {
    re__exec_dfa_crit_writer_exit(cache);
  }
  return err;
reader_exit:
  /* This label is only reached if we wish to leave OR encounter an error during
   * a reader crit */
  if (locked) {
    re__exec_dfa_crit_reader_exit(cache);
  }
  return err;
}

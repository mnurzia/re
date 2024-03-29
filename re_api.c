#include "re_internal.h"

MN_INTERNAL void re__error_init(re* reg)
{
  mn__str_init(&reg->data->error_string);
  mn__str_view_init_null(&reg->data->error_string_view);
}

MN_INTERNAL void re__error_destroy(re* reg)
{
  mn__str_destroy(&reg->data->error_string);
}

/* Doesn't take ownership of error_str */
/* We don't return an error here because it's expected that you are already in
 * the middle of cleaning up after an error */
/* This is useful because it allows error_string to be a const pointer,
 * allowing error messages to be saved as just const strings */
MN_INTERNAL re_error re__set_error_str(re* reg, const mn__str* error_str)
{
  re_error err = RE_ERROR_NONE;
  /* Clear the last error */
  re__error_destroy(reg);
  if ((err = mn__str_init_copy(&reg->data->error_string, error_str))) {
    re__set_error_generic(reg, err);
  } else {
    mn__str_view_init(&reg->data->error_string_view, &reg->data->error_string);
  }
  return err;
}

MN_INTERNAL void re__set_error_generic(re* reg, re_error err)
{
  if (err == RE_ERROR_NOMEM) {
    /* No memory: not guaranteed if we can allocate a string. */
    mn__str_view_init_s(&reg->data->error_string_view, "out of memory");
  } else {
    /* This function is only valid for generic errors */
    MN__ASSERT_UNREACHED();
  }
}

re_error re__init(re* reg, int is_set)
{
  re_error err = RE_ERROR_NONE;
  reg->data = (re_data*)MN_MALLOC(sizeof(re_data));
  if (!reg->data) {
    return RE_ERROR_NOMEM;
  }
  reg->data->set = RE__AST_NONE;
  re__error_init(reg);
  re__rune_data_init(&reg->data->rune_data);
  re__parse_init(&reg->data->parse, reg);
  re__ast_root_init(&reg->data->ast_root);
  re__prog_init(&reg->data->program);
  re__prog_init(&reg->data->program_reverse);
  re__compile_init(&reg->data->compile);
  re__exec_init(&reg->data->exec, reg);
  mn__memset(&reg->data->dfa_cache, 0, sizeof(re__exec_dfa_cache));
  mn__memset(&reg->data->dfa_cache_reverse, 0, sizeof(re__exec_dfa_cache));
  if ((err = re__exec_dfa_cache_init(&reg->data->dfa_cache))) {
    goto error;
  }
  if ((err = re__exec_dfa_cache_init(&reg->data->dfa_cache_reverse))) {
    goto error;
  }
#if RE_USE_THREAD
  mn__memset(&reg->data->program_mutex, 0, sizeof(re__mutex));
  mn__memset(&reg->data->program_reverse_mutex, 0, sizeof(re__mutex));
  if ((err = re__mutex_init(&reg->data->program_mutex))) {
    goto error;
  }
  if ((err = re__mutex_init(&reg->data->program_reverse_mutex))) {
    goto error;
  }
#endif
  if (is_set) {
    re__ast alt;
    re__ast_init_alt(&alt);
    if ((err = re__ast_root_add_child(
             &reg->data->ast_root, reg->data->ast_root.root_ref, alt,
             &reg->data->set))) {
      goto error;
    }
  }
error:
  return err;
}

re_error re_init(re* reg, const char* regex_nt)
{
  return re_init_sz_flags(reg, regex_nt, mn__str_slen(regex_nt), 0);
}

re_error
re_init_flags(re* reg, const char* regex_nt, re_syntax_flags syntax_flags)
{
  return re_init_sz_flags(reg, regex_nt, mn__str_slen(regex_nt), syntax_flags);
}

re_error re_init_sz_flags(
    re* reg, const char* regex, mn_size regex_size,
    re_syntax_flags syntax_flags)
{
  re_error err = RE_ERROR_NONE;
  mn__str_view regex_view;
  if ((err = re__init(reg, 0))) {
    return err;
  }
  mn__str_view_init_n(&regex_view, regex, regex_size);
  if ((err = re__parse_str(&reg->data->parse, regex_view, syntax_flags))) {
    return err;
  }
  return err;
}

re_error re_init_set(re* reg) { return re__init(reg, 1); }

re_error re_set_add(re* reg, const char* regex_nt)
{
  return re_set_add_sz_flags(reg, regex_nt, mn__str_slen(regex_nt), 0);
}

re_error
re_set_add_flags(re* reg, const char* regex_nt, re_syntax_flags syntax_flags)
{
  return re_set_add_sz_flags(
      reg, regex_nt, mn__str_slen(regex_nt), syntax_flags);
}

re_error re_set_add_sz_flags(
    re* reg, const char* regex, mn_size regex_size,
    re_syntax_flags syntax_flags)
{
  mn__str_view regex_view;
  MN__UNUSED(syntax_flags);
  mn__str_view_init_n(&regex_view, regex, regex_size);
  return re__parse_str(&reg->data->parse, regex_view, syntax_flags);
}

void re_destroy(re* reg)
{
  if (reg->data == MN_NULL) {
    return;
  }
  re__compile_destroy(&reg->data->compile);
  re__prog_destroy(&reg->data->program);
  re__prog_destroy(&reg->data->program_reverse);
  re__ast_root_destroy(&reg->data->ast_root);
  re__rune_data_destroy(&reg->data->rune_data);
  re__parse_destroy(&reg->data->parse);
  re__error_destroy(reg);
  re__exec_destroy(&reg->data->exec);
#if RE_USE_THREAD
  re__mutex_destroy(&reg->data->program_mutex);
  re__mutex_destroy(&reg->data->program_reverse_mutex);
#endif
  re__exec_dfa_cache_destroy(&reg->data->dfa_cache);
  re__exec_dfa_cache_destroy(&reg->data->dfa_cache_reverse);
  if (reg->data) {
    MN_FREE(reg->data);
  }
}

const char* re_get_error(const re* reg, mn_size* error_len)
{
  if (error_len != MN_NULL) {
    *error_len = mn__str_view_size(&reg->data->error_string_view);
  }
  return (const char*)mn__str_view_get_data(&reg->data->error_string_view);
}

mn_uint32 re_get_max_groups(const re* reg)
{
  return re__ast_root_get_num_groups(&reg->data->ast_root);
}

MN_INTERNAL re__assert_type re__match_next_assert_ctx(mn_size pos, mn_size len)
{
  re__assert_type out = 0;
  if (pos == 0) {
    out |= RE__ASSERT_TYPE_TEXT_START_ABSOLUTE | RE__ASSERT_TYPE_TEXT_START;
  }
  if (pos == len) {
    out |= RE__ASSERT_TYPE_TEXT_END_ABSOLUTE | RE__ASSERT_TYPE_TEXT_END;
  }
  return out;
}

re_error re__match_prepare_progs(
    re* reg, int fwd, int rev, int fwd_dotstar, int rev_dotstar, int locked)
{
  re_error err = RE_ERROR_NONE;
  MN_ASSERT(MN__IMPLIES(fwd_dotstar, fwd));
  MN_ASSERT(MN__IMPLIES(rev_dotstar, rev));
  MN__UNUSED(locked);
  if (fwd) {
    if (!re__prog_size(&reg->data->program)) {
      if ((err = re__compile_regex(
               &reg->data->compile, &reg->data->ast_root, &reg->data->program,
               0, reg->data->set))) {
        return err;
      }
    }
  }
  if (fwd_dotstar) {
    if (re__prog_get_entry(&reg->data->program, RE__PROG_ENTRY_DOTSTAR) ==
        RE__PROG_LOC_INVALID) {
      if ((err = re__compile_dotstar(
               &reg->data->program, RE__PROG_DATA_ID_DOT_FWD_REJSURR_REJNL))) {
        return err;
      }
    }
  }
  if (rev) {
    if (!re__prog_size(&reg->data->program_reverse)) {
      if ((err = re__compile_regex(
               &reg->data->compile, &reg->data->ast_root,
               &reg->data->program_reverse, 1, reg->data->set))) {
        return err;
      }
    }
  }
  if (rev_dotstar) {
    if (re__prog_get_entry(
            &reg->data->program_reverse, RE__PROG_ENTRY_DOTSTAR) ==
        RE__PROG_LOC_INVALID) {
      if ((err = re__compile_dotstar(
               &reg->data->program_reverse,
               RE__PROG_DATA_ID_DOT_REV_REJSURR_REJNL))) {
        return err;
      }
    }
  }
  return err;
}

#if MN_DEBUG

#include <stdio.h>

#endif

#if 0

re_error re__match_dfa_driver(
    re__prog* program, re__prog_entry entry,
    int request, /* 0 for boolean, 1 for match pos + index */
    int bool_bail, int reversed, mn_size start_pos, const char* text,
    mn_size text_size, mn_uint32* out_match, mn_size* out_pos)
{
  mn_size pos;
  re_error err = RE_ERROR_NONE;
  re__exec_dfa exec_dfa;
  re__exec_dfa_start_state_flags start_state_flags = 0;
  mn_size last_found_pos = 0;
  mn_uint32 last_found_match = 0;
  mn_uint32 match_status = 0;
  if (!reversed) {
    if (start_pos == 0) {
      start_state_flags |= RE__EXEC_DFA_START_STATE_FLAG_BEGIN_TEXT |
                           RE__EXEC_DFA_START_STATE_FLAG_BEGIN_LINE;
    } else {
      start_state_flags |=
          (RE__EXEC_DFA_START_STATE_FLAG_AFTER_WORD *
           re__is_word_char((unsigned char)(text[start_pos - 1])));
      start_state_flags |= RE__EXEC_DFA_START_STATE_FLAG_BEGIN_LINE *
                           (text[start_pos - 1] == '\n');
    }
  } else {
    if (start_pos == text_size) {
      start_state_flags |= RE__EXEC_DFA_START_STATE_FLAG_BEGIN_TEXT |
                           RE__EXEC_DFA_START_STATE_FLAG_BEGIN_LINE;
    } else {
      start_state_flags |=
          (RE__EXEC_DFA_START_STATE_FLAG_AFTER_WORD *
           re__is_word_char((unsigned char)(text[start_pos])));
      start_state_flags |=
          RE__EXEC_DFA_START_STATE_FLAG_BEGIN_LINE * (text[start_pos] == '\n');
    }
  }
  re__exec_dfa_init(&exec_dfa, program);
  if ((err = re__exec_dfa_start(&exec_dfa, entry, start_state_flags))) {
    goto err_destroy_dfa;
  }
  if (!reversed) {
    pos = start_pos;
  } else {
    pos = text_size - start_pos;
  }
  MN_ASSERT(pos <= text_size);
  MN_ASSERT(MN__IMPLIES(request, out_match != MN_NULL));
  MN_ASSERT(MN__IMPLIES(request, out_pos != MN_NULL));
  MN_ASSERT(MN__IMPLIES(bool_bail, !request));
  while (pos < text_size) {
    unsigned char ch = 0;
    if (!reversed) {
      ch = (unsigned char)(text[pos]);
    } else {
      ch = (unsigned char)(text[(text_size - pos) - 1]);
    }
    if ((err = re__exec_dfa_run_byte(&exec_dfa, ch))) {
      goto err_destroy_dfa;
    }
    if (request == 0) {
      if (bool_bail && re__exec_dfa_get_match_index(&exec_dfa)) {
        goto match_early_boolean;
      }
    } else {
      match_status = re__exec_dfa_get_match_index(&exec_dfa);
      if (match_status) {
        last_found_match = match_status;
        last_found_pos = pos;
        if (re__exec_dfa_get_match_priority(&exec_dfa) == 0) {
          goto match_early_priority;
        }
      } else {
        if (re__exec_dfa_get_exhaustion(&exec_dfa)) {
          if (last_found_match) {
            goto match_early_priority;
          }
        }
      }
    }
    pos++;
  }
  if ((err = re__exec_dfa_end(&exec_dfa))) {
    goto err_destroy_dfa;
  }
  if ((last_found_match = re__exec_dfa_get_match_index(&exec_dfa))) {
    last_found_pos = pos;
    if (request) {
      MN_ASSERT(last_found_pos <= text_size);
      if (!reversed) {
        *out_pos = last_found_pos;
      } else {
        *out_pos = text_size - last_found_pos;
      }
    }
  } else {
    err = RE_NOMATCH;
  }
  re__exec_dfa_destroy(&exec_dfa);
  return err;
match_early_boolean:
  re__exec_dfa_destroy(&exec_dfa);
  return RE_MATCH;
match_early_priority:
  re__exec_dfa_destroy(&exec_dfa);
  MN_ASSERT(last_found_pos <= text_size);
  if (!reversed) {
    *out_pos = last_found_pos;
  } else {
    *out_pos = text_size - last_found_pos;
  }
  *out_match = last_found_match;
  return RE_MATCH;
err_destroy_dfa:
  re__exec_dfa_destroy(&exec_dfa);
  return err;
}

#endif

MN__VEC_IMPL_FUNC(re_span, init)
MN__VEC_IMPL_FUNC(re_span, destroy)
MN__VEC_IMPL_FUNC(re_span, reserve)
MN__VEC_IMPL_FUNC(re_span, get_data)

MN__VEC_IMPL_FUNC(mn_uint32, init)
MN__VEC_IMPL_FUNC(mn_uint32, destroy)
MN__VEC_IMPL_FUNC(mn_uint32, reserve)
MN__VEC_IMPL_FUNC(mn_uint32, get_data)
MN__VEC_IMPL_FUNC(mn_uint32, capacity)

MN_INTERNAL void re__exec_init(re__exec* exec, re* reg)
{
  exec->reg = reg;
  exec->num_groups = 0;
  exec->num_sets = 0;
  re_span_vec_init(&exec->spans);
  mn_uint32_vec_init(&exec->set_indexes);
  mn_uint32_vec_init(&exec->pri_bitmap);
  exec->compile_status = 0;
  re__exec_nfa_init(&exec->nfa);
  exec->dfa_state_hash = 0;
  exec->dfa_state_flags = 0;
  exec->dfa_state_match_index = 0;
  exec->dfa_state_match_priority = 0;
}

MN_INTERNAL void re__exec_destroy(re__exec* exec)
{
  re_span_vec_destroy(&exec->spans);
  mn_uint32_vec_destroy(&exec->set_indexes);
  mn_uint32_vec_destroy(&exec->pri_bitmap);
  re__exec_nfa_destroy(&exec->nfa);
}

MN_INTERNAL re_error re__exec_reserve(
    re__exec* exec, mn_uint32 max_group, mn_uint32 max_set,
    int reserve_priority_bitmap)
{
  re_error err = RE_ERROR_NONE;
  exec->num_groups = max_group;
  exec->num_sets = max_set;
  if ((err = re_span_vec_reserve(
           &exec->spans, exec->num_groups * exec->num_sets))) {
    return err;
  }
  if ((err = mn_uint32_vec_reserve(&exec->set_indexes, exec->num_sets))) {
    return err;
  }
  mn__memset(
      re_span_vec_get_data(&exec->spans), 0,
      sizeof(re_span) * exec->num_groups);
  mn__memset(
      mn_uint32_vec_get_data(&exec->set_indexes), 0,
      sizeof(mn_uint32) * exec->num_sets);
  if (reserve_priority_bitmap) {
    /* Reserve just enough space in the bitmap */
    if ((err = mn_uint32_vec_reserve(
             &exec->pri_bitmap, (exec->num_sets + 15) / 16))) {
    }
    mn__memset(
        mn_uint32_vec_get_data(&exec->pri_bitmap), 0,
        sizeof(mn_uint32) * ((exec->num_sets + 15) / 16));
  }
  return RE_ERROR_NONE;
}

MN_INTERNAL re_span* re__exec_get_spans(re__exec* exec)
{
  return re_span_vec_get_data(&exec->spans);
}

MN_INTERNAL mn_uint32* re__exec_get_set_indexes(re__exec* exec)
{
  return mn_uint32_vec_get_data(&exec->set_indexes);
}

MN_INTERNAL void
re__exec_prepare(re__exec* exec, re__prog* prog, mn_uint32 max_group)
{
  re__exec_nfa_set_prog(&exec->nfa, prog);
  re__exec_nfa_set_num_groups(&exec->nfa, max_group);
}

/*
re_error re__match_dfa_driver(
    re__exec_dfa_cache* cache, re__prog_entry entry,
    int request,
    int bool_bail, int reversed, mn_size start_pos, const char* text,
    mn_size text_size, mn_uint32* out_match, mn_size* out_pos)*/
re_error re__match_dfa_driver(
    re__exec_dfa_cache* cache, re__prog_entry entry, mn_size start_pos,
    const char* text, mn_size text_size, mn_uint32* out_match, mn_size* out_pos,
    re__exec_dfa_run_flags run_flags, re__exec* exec)
{
  re_error err = RE_ERROR_NONE;
  if ((err = re__exec_dfa_cache_driver(
           cache, entry, (const mn_uint8*)text, text_size, start_pos, out_match,
           out_pos, run_flags, exec))) {
    goto error;
  }
error:
  return err;
}

re_error re_is_match(
    re* reg, const char* text, mn_size text_size, re_anchor_type anchor_type)
{
  /* no groups -- dfa can be used in all cases */
  re_error err = RE_ERROR_NONE;
  if (anchor_type == RE_ANCHOR_BOTH) {
    if ((err = re__match_prepare_progs(reg, 1, 0, 0, 0, 0))) {
      return err;
    }
    re__exec_prepare(&reg->data->exec, &reg->data->program, 0);
    return re__match_dfa_driver(
        &reg->data->dfa_cache, RE__PROG_ENTRY_DEFAULT, 0, text, text_size,
        MN_NULL, MN_NULL, RE__EXEC_DFA_RUN_FLAG_BOOLEAN_MATCH,
        &reg->data->exec);
  } else if (anchor_type == RE_ANCHOR_START) {
    if ((err = re__match_prepare_progs(reg, 1, 0, 0, 0, 0))) {
      return err;
    }
    re__exec_prepare(&reg->data->exec, &reg->data->program, 0);
    return re__match_dfa_driver(
        &reg->data->dfa_cache, RE__PROG_ENTRY_DEFAULT, 0, text, text_size,
        MN_NULL, MN_NULL,
        RE__EXEC_DFA_RUN_FLAG_BOOLEAN_MATCH |
            RE__EXEC_DFA_RUN_FLAG_BOOLEAN_MATCH_EXIT_EARLY,
        &reg->data->exec);
  } else if (anchor_type == RE_ANCHOR_END) {
    if ((err = re__match_prepare_progs(reg, 0, 1, 0, 0, 0))) {
      return err;
    }
    re__exec_prepare(&reg->data->exec, &reg->data->program_reverse, 0);
    return re__match_dfa_driver(
        &reg->data->dfa_cache_reverse, RE__PROG_ENTRY_DEFAULT, text_size, text,
        text_size, MN_NULL, MN_NULL,
        RE__EXEC_DFA_RUN_FLAG_BOOLEAN_MATCH |
            RE__EXEC_DFA_RUN_FLAG_BOOLEAN_MATCH_EXIT_EARLY |
            RE__EXEC_DFA_RUN_FLAG_REVERSED,
        &reg->data->exec);
  } else if (anchor_type == RE_UNANCHORED) {
    if ((err = re__match_prepare_progs(reg, 1, 0, 1, 0, 0))) {
      return err;
    }
    re__exec_prepare(&reg->data->exec, &reg->data->program, 0);
    return re__match_dfa_driver(
        &reg->data->dfa_cache, RE__PROG_ENTRY_DOTSTAR, 0, text, text_size,
        MN_NULL, MN_NULL,
        RE__EXEC_DFA_RUN_FLAG_BOOLEAN_MATCH |
            RE__EXEC_DFA_RUN_FLAG_BOOLEAN_MATCH_EXIT_EARLY,
        &reg->data->exec);
  } else {
    return RE_ERROR_INVALID;
  }
}

re_error re_match_groups_set(
    re* reg, const char* text, mn_size text_size, re_anchor_type anchor_type,
    mn_uint32 max_group, re_span* out_groups, mn_uint32* out_set_index)
{
  re_error err = RE_ERROR_NONE;
  mn_uint32 out_match;
  if (max_group == 0 && out_set_index == MN_NULL) {
    if (anchor_type == RE_ANCHOR_BOTH) {
      if ((err = re__match_prepare_progs(reg, 1, 0, 0, 0, 0))) {
        return err;
      }
      re__exec_prepare(&reg->data->exec, &reg->data->program, 0);
      return re__match_dfa_driver(
          &reg->data->dfa_cache, RE__PROG_ENTRY_DEFAULT, 0, text, text_size,
          MN_NULL, MN_NULL, RE__EXEC_DFA_RUN_FLAG_BOOLEAN_MATCH,
          &reg->data->exec);
    } else if (anchor_type == RE_ANCHOR_START) {
      if ((err = re__match_prepare_progs(reg, 1, 0, 0, 0, 0))) {
        return err;
      }
      re__exec_prepare(&reg->data->exec, &reg->data->program, 0);
      return re__match_dfa_driver(
          &reg->data->dfa_cache, RE__PROG_ENTRY_DEFAULT, 0, text, text_size,
          MN_NULL, MN_NULL,
          RE__EXEC_DFA_RUN_FLAG_BOOLEAN_MATCH |
              RE__EXEC_DFA_RUN_FLAG_BOOLEAN_MATCH_EXIT_EARLY,
          &reg->data->exec);
    } else if (anchor_type == RE_ANCHOR_END) {
      if ((err = re__match_prepare_progs(reg, 0, 1, 0, 0, 0))) {
        return err;
      }
      re__exec_prepare(&reg->data->exec, &reg->data->program_reverse, 0);
      return re__match_dfa_driver(
          &reg->data->dfa_cache_reverse, RE__PROG_ENTRY_DEFAULT, text_size,
          text, text_size, MN_NULL, MN_NULL,
          RE__EXEC_DFA_RUN_FLAG_BOOLEAN_MATCH |
              RE__EXEC_DFA_RUN_FLAG_BOOLEAN_MATCH_EXIT_EARLY |
              RE__EXEC_DFA_RUN_FLAG_REVERSED,
          &reg->data->exec);
    } else if (anchor_type == RE_UNANCHORED) {
      if ((err = re__match_prepare_progs(reg, 1, 0, 1, 0, 0))) {
        return err;
      }
      re__exec_prepare(&reg->data->exec, &reg->data->program, 0);
      return re__match_dfa_driver(
          &reg->data->dfa_cache, RE__PROG_ENTRY_DOTSTAR, 0, text, text_size,
          MN_NULL, MN_NULL,
          RE__EXEC_DFA_RUN_FLAG_BOOLEAN_MATCH |
              RE__EXEC_DFA_RUN_FLAG_BOOLEAN_MATCH_EXIT_EARLY,
          &reg->data->exec);
    } else {
      return RE_ERROR_INVALID;
    }
  } else if (max_group == 0 || max_group == 1) {
    mn_size out_pos;
    re_error match_err;
    if (anchor_type == RE_ANCHOR_BOTH) {
      if ((err = re__match_prepare_progs(reg, 1, 0, 0, 0, 0))) {
        goto error;
      }
      re__exec_prepare(&reg->data->exec, &reg->data->program, 0);
      match_err = re__match_dfa_driver(
          &reg->data->dfa_cache, RE__PROG_ENTRY_DEFAULT, 0, text, text_size,
          &out_match, &out_pos, 0, &reg->data->exec);
      if (match_err == RE_MATCH) {
        if (out_pos != text_size) {
          err = RE_NOMATCH;
          goto success;
        } else {
          if (max_group) {
            out_groups[0].begin = 0;
            out_groups[0].end = out_pos;
          }
          err = RE_MATCH;
          goto success;
        }
      } else {
        err = match_err;
        goto error;
      }
    } else if (anchor_type == RE_ANCHOR_START) {
      if ((err = re__match_prepare_progs(reg, 1, 0, 0, 0, 0))) {
        goto error;
      }
      re__exec_prepare(&reg->data->exec, &reg->data->program, 0);
      match_err = re__match_dfa_driver(
          &reg->data->dfa_cache, RE__PROG_ENTRY_DEFAULT, 0, text, text_size,
          &out_match, &out_pos, 0, &reg->data->exec);
      if (match_err == RE_MATCH) {
        if (max_group) {
          out_groups[0].begin = 0;
          out_groups[0].end = out_pos;
        }
        err = RE_MATCH;
        goto success;
      } else {
        err = match_err;
        goto error;
      }
    } else if (anchor_type == RE_ANCHOR_END) {
      if ((err = re__match_prepare_progs(reg, 0, 1, 0, 0, 0))) {
        goto error;
      }
      re__exec_prepare(&reg->data->exec, &reg->data->program_reverse, 0);
      match_err = re__match_dfa_driver(
          &reg->data->dfa_cache_reverse, RE__PROG_ENTRY_DEFAULT, text_size,
          text, text_size, &out_match, &out_pos, RE__EXEC_DFA_RUN_FLAG_REVERSED,
          &reg->data->exec);
      if (match_err == RE_MATCH) {
        if (max_group) {
          out_groups[0].begin = out_pos;
          out_groups[0].end = text_size;
        }
        err = RE_MATCH;
        goto success;
      } else {
        err = match_err;
        goto error;
      }
    } else if (anchor_type == RE_UNANCHORED) {
      if ((err = re__match_prepare_progs(reg, 1, 1, 1, 0, 0))) {
        goto error;
      }
      re__exec_prepare(&reg->data->exec, &reg->data->program, 0);
      match_err = re__match_dfa_driver(
          &reg->data->dfa_cache, RE__PROG_ENTRY_DOTSTAR, 0, text, text_size,
          &out_match, &out_pos, 0, &reg->data->exec);
      if (match_err == RE_MATCH) {
        if (max_group) {
          out_groups[0].end = out_pos;
          /* scan back to find the start */
          re__exec_prepare(&reg->data->exec, &reg->data->program_reverse, 0);
          match_err = re__match_dfa_driver(
              &reg->data->dfa_cache_reverse, RE__PROG_ENTRY_DEFAULT, out_pos,
              text, text_size, &out_match, &out_pos,
              RE__EXEC_DFA_RUN_FLAG_REVERSED, &reg->data->exec);
          /* should ALWAYS match. */
          if (match_err != RE_MATCH) {
            err = match_err;
            goto error;
          }
          MN_ASSERT(match_err == RE_MATCH);
          out_groups[0].begin = out_pos;
        }
        err = RE_MATCH;
        goto success;
      } else {
        err = match_err;
        goto error;
      }
    } else {
      err = RE_ERROR_INVALID;
      goto error;
    }
  } else {
    re__exec_nfa exec_nfa;
    mn__str_view string_view;
    re__exec_nfa_init(&exec_nfa);
    re__exec_nfa_set_prog(&exec_nfa, &reg->data->program);
    re__exec_nfa_set_num_groups(&exec_nfa, max_group + 1);
    mn__str_view_init_n(&string_view, text, text_size);
    {
      mn_size pos;
      re__assert_type assert_ctx =
          RE__ASSERT_TYPE_TEXT_START | RE__ASSERT_TYPE_TEXT_START_ABSOLUTE;
      if ((err = re__exec_nfa_start(&exec_nfa, 0))) {
        goto nfa_error;
      }
      for (pos = 0; pos < mn__str_view_size(&string_view); pos++) {
        mn_uint8 ch = (mn_uint8)mn__str_view_get_data(&string_view)[pos];
        mn_uint32 match;
        assert_ctx =
            re__match_next_assert_ctx(pos, mn__str_view_size(&string_view));
        if ((err = re__exec_nfa_run_byte(&exec_nfa, assert_ctx, ch, pos))) {
          goto nfa_error;
        }
        match = re__exec_nfa_get_match_index(&exec_nfa);
        if (match) {
          if (anchor_type == RE_ANCHOR_BOTH || anchor_type == RE_ANCHOR_END) {
            /* can bail! */
            break;
          }
        }
      }
      if ((err = re__exec_nfa_run_byte(
               &exec_nfa, assert_ctx, RE__EXEC_SYM_EOT, pos))) {
        goto nfa_error;
      }
      err = re__exec_nfa_finish(&exec_nfa, out_groups, pos);
      if (err == RE_MATCH) {
        out_match = re__exec_nfa_get_match_index(&exec_nfa);
      }
    }
    goto success;
  nfa_error:
    re__exec_nfa_destroy(&exec_nfa);
    goto error;
  }
success:
  if (out_set_index != MN_NULL && err == RE_MATCH) {
    *out_set_index = out_match;
  }
  return err;
error:
  return err;
}

re_error re_match_groups(
    re* reg, const char* text, mn_size text_size, re_anchor_type anchor_type,
    mn_uint32 max_group, re_span* out_groups)
{
  return re_match_groups_set(
      reg, text, text_size, anchor_type, max_group, out_groups, MN_NULL);
}

re_error re__match_prepare_progs_new(
    re* reg, re__exec* exec, int fwd, int rev, int fwd_dotstar, int rev_dotstar,
    int locked)
{
  re_error err = RE_ERROR_NONE;
  MN_ASSERT(MN__IMPLIES(fwd_dotstar, fwd));
  MN_ASSERT(MN__IMPLIES(rev_dotstar, rev));
  if (fwd && !(exec->compile_status & 1)) {
    if (locked) {
      re__mutex_lock(&reg->data->program_mutex);
    }
    if (!re__prog_size(&reg->data->program)) {
      if ((err = re__compile_regex(
               &reg->data->compile, &reg->data->ast_root, &reg->data->program,
               0, reg->data->set))) {
        return err;
      }
    }
    if (locked) {
      re__mutex_unlock(&reg->data->program_mutex);
    }
    exec->compile_status |= 1;
  }
  if (fwd_dotstar && !(exec->compile_status & 2)) {
    if (locked) {
      re__mutex_lock(&reg->data->program_mutex);
    }
    if (re__prog_get_entry(&reg->data->program, RE__PROG_ENTRY_DOTSTAR) ==
        RE__PROG_LOC_INVALID) {
      if ((err = re__compile_dotstar(
               &reg->data->program, RE__PROG_DATA_ID_DOT_FWD_REJSURR_REJNL))) {
        return err;
      }
    }
    if (locked) {
      re__mutex_unlock(&reg->data->program_mutex);
    }
    exec->compile_status |= 2;
  }
  if (rev && !(exec->compile_status & 4)) {
    if (locked) {
      re__mutex_lock(&reg->data->program_reverse_mutex);
    }
    if (!re__prog_size(&reg->data->program_reverse)) {
      if ((err = re__compile_regex(
               &reg->data->compile, &reg->data->ast_root,
               &reg->data->program_reverse, 1, reg->data->set))) {
        return err;
      }
    }
    if (locked) {
      re__mutex_unlock(&reg->data->program_reverse_mutex);
    }
    exec->compile_status |= 4;
  }
  if (rev_dotstar && !(exec->compile_status & 8)) {
    if (locked) {
      re__mutex_lock(&reg->data->program_reverse_mutex);
    }
    if (re__prog_get_entry(
            &reg->data->program_reverse, RE__PROG_ENTRY_DOTSTAR) ==
        RE__PROG_LOC_INVALID) {
      if ((err = re__compile_dotstar(
               &reg->data->program_reverse,
               RE__PROG_DATA_ID_DOT_REV_REJSURR_REJNL))) {
        return err;
      }
    }
    if (locked) {
      re__mutex_unlock(&reg->data->program_reverse_mutex);
    }
    exec->compile_status |= 8;
  }
  return err;
}

#if 1
re_error re__match(
    re* reg, re__exec* exec, const char* text, mn_size text_size,
    re_anchor_type anchor_type, mn_uint32 max_group, re_span** out_spans,
    mn_uint32 max_set, mn_uint32** set_indexes, int locked)
{
  re_error err = RE_ERROR_NONE;
  re_span* spans;
  re__exec_dfa_run_flags run_flags = locked ? RE__EXEC_DFA_RUN_FLAG_LOCKED : 0;
  if ((err = re__exec_reserve(exec, max_group, max_set, 0))) {
    return err;
  }
  MN__UNUSED(set_indexes);
  MN__UNUSED(out_spans);
  MN__UNUSED(locked);
  spans = re_span_vec_get_data(&exec->spans);
  if (max_group == 0 && max_set == 0) {
    /* Fully boolean match (is_match) */
    if (anchor_type == RE_ANCHOR_BOTH) {
      if ((err = re__match_prepare_progs_new(reg, exec, 1, 0, 0, 0, 1))) {
        return err;
      }
      re__exec_prepare(exec, &reg->data->program, 0);
      return re__match_dfa_driver(
          &reg->data->dfa_cache, RE__PROG_ENTRY_DEFAULT, 0, text, text_size,
          MN_NULL, MN_NULL, RE__EXEC_DFA_RUN_FLAG_BOOLEAN_MATCH | run_flags,
          exec);
    } else if (anchor_type == RE_ANCHOR_START) {
      if ((err = re__match_prepare_progs_new(reg, exec, 1, 0, 0, 0, 0))) {
        return err;
      }
      re__exec_prepare(exec, &reg->data->program, 0);
      return re__match_dfa_driver(
          &reg->data->dfa_cache, RE__PROG_ENTRY_DEFAULT, 0, text, text_size,
          MN_NULL, MN_NULL,
          RE__EXEC_DFA_RUN_FLAG_BOOLEAN_MATCH |
              RE__EXEC_DFA_RUN_FLAG_BOOLEAN_MATCH_EXIT_EARLY | run_flags,
          exec);
    } else if (anchor_type == RE_ANCHOR_END) {
      if ((err = re__match_prepare_progs_new(reg, exec, 0, 1, 0, 0, 0))) {
        return err;
      }
      re__exec_prepare(exec, &reg->data->program_reverse, 0);
      return re__match_dfa_driver(
          &reg->data->dfa_cache_reverse, RE__PROG_ENTRY_DEFAULT, text_size,
          text, text_size, MN_NULL, MN_NULL,
          RE__EXEC_DFA_RUN_FLAG_BOOLEAN_MATCH |
              RE__EXEC_DFA_RUN_FLAG_BOOLEAN_MATCH_EXIT_EARLY |
              RE__EXEC_DFA_RUN_FLAG_REVERSED | run_flags,
          exec);
    } else if (anchor_type == RE_UNANCHORED) {
      if ((err = re__match_prepare_progs_new(reg, exec, 1, 0, 1, 0, 0))) {
        return err;
      }
      re__exec_prepare(exec, &reg->data->program, 0);
      return re__match_dfa_driver(
          &reg->data->dfa_cache, RE__PROG_ENTRY_DOTSTAR, 0, text, text_size,
          MN_NULL, MN_NULL,
          RE__EXEC_DFA_RUN_FLAG_BOOLEAN_MATCH |
              RE__EXEC_DFA_RUN_FLAG_BOOLEAN_MATCH_EXIT_EARLY | run_flags,
          exec);
    } else {
      return RE_ERROR_INVALID;
    }
  } else if (max_group == 0 || max_group == 1) {
    /* Bounds-only or set-index-only match [DFA] */
    mn_size out_pos;
    mn_uint32 out_match;
    re_error match_err;
    if (anchor_type == RE_ANCHOR_BOTH) {
      if ((err = re__match_prepare_progs(reg, 1, 0, 0, 0, 0))) {
        return err;
      }
      re__exec_prepare(&reg->data->exec, &reg->data->program, 0);
      match_err = re__match_dfa_driver(
          &reg->data->dfa_cache, RE__PROG_ENTRY_DEFAULT, 0, text, text_size,
          &out_match, &out_pos, run_flags, &reg->data->exec);
      if (match_err == RE_MATCH) {
        if (out_pos != text_size) {
          err = RE_NOMATCH;
          goto success;
        } else {
          if (max_group) {
            spans[0].begin = 0;
            spans[0].end = out_pos;
          }
          err = RE_MATCH;
          goto success;
        }
      } else {
        err = match_err;
        goto error;
      }
    } else if (anchor_type == RE_ANCHOR_START) {
      if ((err = re__match_prepare_progs(reg, 1, 0, 0, 0, 0))) {
        goto error;
      }
      re__exec_prepare(&reg->data->exec, &reg->data->program, 0);
      match_err = re__match_dfa_driver(
          &reg->data->dfa_cache, RE__PROG_ENTRY_DEFAULT, 0, text, text_size,
          &out_match, &out_pos, run_flags, &reg->data->exec);
      if (match_err == RE_MATCH) {
        if (max_group) {
          spans[0].begin = 0;
          spans[0].end = out_pos;
        }
        err = RE_MATCH;
        goto success;
      } else {
        err = match_err;
        goto error;
      }
    } else if (anchor_type == RE_ANCHOR_END) {
      if ((err = re__match_prepare_progs(reg, 0, 1, 0, 0, 0))) {
        goto error;
      }
      re__exec_prepare(&reg->data->exec, &reg->data->program_reverse, 0);
      match_err = re__match_dfa_driver(
          &reg->data->dfa_cache_reverse, RE__PROG_ENTRY_DEFAULT, text_size,
          text, text_size, &out_match, &out_pos,
          RE__EXEC_DFA_RUN_FLAG_REVERSED | run_flags, &reg->data->exec);
      if (match_err == RE_MATCH) {
        if (max_group) {
          spans[0].begin = out_pos;
          spans[0].end = text_size;
        }
        err = RE_MATCH;
        goto success;
      } else {
        err = match_err;
        goto error;
      }
    } else if (anchor_type == RE_UNANCHORED) {
      if ((err = re__match_prepare_progs(reg, 1, 1, 1, 0, 0))) {
        goto error;
      }
      re__exec_prepare(&reg->data->exec, &reg->data->program, 0);
      match_err = re__match_dfa_driver(
          &reg->data->dfa_cache, RE__PROG_ENTRY_DOTSTAR, 0, text, text_size,
          &out_match, &out_pos, run_flags, &reg->data->exec);
      if (match_err == RE_MATCH) {
        if (max_group) {
          spans[0].end = out_pos;
          /* scan back to find the start */
          re__exec_prepare(&reg->data->exec, &reg->data->program_reverse, 0);
          match_err = re__match_dfa_driver(
              &reg->data->dfa_cache_reverse, RE__PROG_ENTRY_DEFAULT, out_pos,
              text, text_size, &out_match, &out_pos,
              RE__EXEC_DFA_RUN_FLAG_REVERSED | run_flags, &reg->data->exec);
          /* should ALWAYS match. */
          if (match_err != RE_MATCH) {
            err = match_err;
            goto error;
          }
          MN_ASSERT(match_err == RE_MATCH);
          spans[0].begin = out_pos;
        }
        err = RE_MATCH;
        goto success;
      } else {
        err = match_err;
        goto error;
      }
    } else {
      err = RE_ERROR_INVALID;
      goto error;
    }
  }
success:
  return err;
error:
  return err;
}
#endif

#if 0
re_error re__match(
    re* reg, const char* text, mn_size text_size, re_anchor_type anchor_type,
    mn_uint32 max_group, mn_uint32 max_set, re__match_data* out_data,
    int locked)
{
  if (max_group == 0 && max_set == 0) {
    /* Fully boolean match [DFA] ~1GB/s */
  } else if (max_group == 0) {
    /* Set-index only match [DFA] ~900MB/s */
  } else if (max_group == 1) {
    /* Match boundaries [DFA] ~800MB/s */
  } else {
    /* Group / group-set search [NFA] ~300MB/s */
  }
}
#endif

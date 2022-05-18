#include "re_internal.h"

MN_INTERNAL void re__error_init(re* reg) {
    mn__str_init(&reg->data->error_string);
    mn__str_view_init_null(&reg->data->error_string_view);
}

MN_INTERNAL void re__error_destroy(re* reg) {
    mn__str_destroy(&reg->data->error_string);
}

/* Doesn't take ownership of error_str */
/* We don't return an error here because it's expected that you are already in
 * the middle of cleaning up after an error */
/* This is useful because it allows error_string to be a const pointer,
 * allowing error messages to be saved as just const strings */
MN_INTERNAL void re__set_error_str(re* reg, const mn__str* error_str) {
    re_error err = RE_ERROR_NONE;
    /* Clear the last error */
    re__error_destroy(reg);
    if ((err = mn__str_init_copy(&reg->data->error_string, error_str))) {
        re__set_error_generic(reg, err);
    }
    mn__str_view_init(&reg->data->error_string_view, &reg->data->error_string);
}

MN_INTERNAL void re__set_error_generic(re* reg, re_error err) {
    if (err == RE_ERROR_NOMEM) {
        /* No memory: not guaranteed if we can allocate a string. */
        mn__str_view_init_s(&reg->data->error_string_view, "out of memory");
    } else {
        /* This function is only valid for generic errors */
        MN__ASSERT_UNREACHED();
    }
}

re_error re_init(re* reg, const char* regex_nt) {
    return re_init_sz_flags(reg, regex_nt, mn__str_slen(regex_nt), 0);
}

re_error re_init_flags(re* reg, const char* regex_nt, re_syntax_flags syntax_flags) {
    return re_init_sz_flags(reg, regex_nt, mn__str_slen(regex_nt), syntax_flags);
}

re_error re_init_sz_flags(re* reg, const char* regex, mn_size regex_size, re_syntax_flags syntax_flags) {
    re_error err = RE_ERROR_NONE;
    mn__str_view regex_view;
    MN__UNUSED(syntax_flags);
    reg->data = (re_data*)MN_MALLOC(sizeof(re_data));
    if (!reg->data) {
        return RE_ERROR_NOMEM;
    }
    re__error_init(reg);
    mn__str_view_init_n(&regex_view, regex, regex_size);
    re__parse_init(&reg->data->parse, reg);
    re__ast_root_init(&reg->data->ast_root);
    re__prog_init(&reg->data->program);
    re__prog_init(&reg->data->program_reverse);
    re__compile_init(&reg->data->compile);
    if ((err = re__parse_str(&reg->data->parse, &regex_view))) {
        return err;
    }
    return err;
}

re_error re_init_set(re* reg) {
    MN__UNUSED(reg);
    return RE_ERROR_NONE;
}

void re_destroy(re* reg) {
    re__compile_destroy(&reg->data->compile);
    re__prog_destroy(&reg->data->program);
    re__prog_destroy(&reg->data->program_reverse);
    re__ast_root_destroy(&reg->data->ast_root);
    re__parse_destroy(&reg->data->parse);
    re__error_destroy(reg);
    if (reg->data) {
        MN_FREE(reg->data);
    }
}

const char* re_get_error(const re* reg, mn_size* error_len) {
    if (error_len != MN_NULL) {
        *error_len = mn__str_view_size(&reg->data->error_string_view);
    }
    return (const char*)mn__str_view_get_data(&reg->data->error_string_view);
}

mn_uint32 re_get_max_groups(const re* reg) {
    return re__ast_root_get_num_groups(&reg->data->ast_root);
}

MN_INTERNAL re__assert_type re__match_next_assert_ctx(mn_size pos, mn_size len) {
    re__assert_type out = 0;
    if (pos == 0) {
        out |= RE__ASSERT_TYPE_TEXT_START_ABSOLUTE | RE__ASSERT_TYPE_TEXT_START;
    }
    if (pos == len) {
        out |= RE__ASSERT_TYPE_TEXT_END_ABSOLUTE | RE__ASSERT_TYPE_TEXT_END;
    }
    return out;
}

re_error re__match_prepare_progs(re* reg, int fwd, int rev, int fwd_dotstar, int rev_dotstar) {
    re_error err = RE_ERROR_NONE;
    MN_ASSERT(MN__IMPLIES(fwd_dotstar, fwd));
    MN_ASSERT(MN__IMPLIES(rev_dotstar, rev));
    if (fwd) {
        if (!re__prog_size(&reg->data->program)) {
            if ((err = re__compile_regex(&reg->data->compile, &reg->data->ast_root, &reg->data->program, 0))) {
                return err;
            }
        }
    }
    if (fwd_dotstar) {
        if (re__prog_get_entry(&reg->data->program, RE__PROG_ENTRY_DOTSTAR) == RE__PROG_LOC_INVALID) {
            if ((err = re__compile_dotstar(&reg->data->program, 0))) {
                return err;
            }
        }
    }
    if (rev) {
        if (!re__prog_size(&reg->data->program_reverse)) {
            if ((err = re__compile_regex(&reg->data->compile, &reg->data->ast_root, &reg->data->program_reverse, 1))) {
                return err;
            }
        }
    }
    if (rev_dotstar) {
        if (re__prog_get_entry(&reg->data->program_reverse, RE__PROG_ENTRY_DOTSTAR) == RE__PROG_LOC_INVALID) {
            if ((err = re__compile_dotstar(&reg->data->program_reverse, 1))) {
                return err;
            }
        }
    }
    return err;
}

/*    | Match?  | Bounds? | Subs?
 * ---+---------+---------+-------
 * ^$ | DFA-F   | DFA-F   | NFA-F
 * ^- | DFA-F   | DFA-F   | NFA-F
 * -$ | DFA-R   | DFA-R   | NFA-R
 * -- | DFA-F   | DFA-F+R | NFA-F
 */

/* DFA parameters:
 * - Entrypoint
 * - Program
 * - Can bail early in thread exhaustion? -> return nomatch
 * - Can bail early if any match state found? -> return match (bool)
 * - Can bail early if highest-priority match state found? -> return match index, pos
 */

#if MN_DEBUG

#include <stdio.h>

#endif

re_error re__match_dfa_driver(
    re__prog* program, 
    re__prog_entry entry, 
    int request, /* 0 for boolean, 1 for match pos + index */
    int bool_bail,
    int reversed,
    mn_size start_pos,
    const char* text, 
    mn_size text_size,
    mn_uint32* out_match,
    mn_size* out_pos) {
    mn_size pos = start_pos;
    re_error err = RE_ERROR_NONE;
    re__exec_dfa exec_dfa;
    re__exec_dfa_start_state_flags start_state_flags =
        RE__EXEC_DFA_START_STATE_FLAG_BEGIN_TEXT
        | RE__EXEC_DFA_START_STATE_FLAG_BEGIN_LINE;
    mn_size last_found_pos = 0;
    mn_uint32 last_found_match = 0;
    mn_uint32 match_status = 0;
    MN_ASSERT(pos <= text_size);
    re__exec_dfa_init(&exec_dfa, program);
    if ((err = re__exec_dfa_start(&exec_dfa, entry, start_state_flags))) {
        return err;
    }
    MN_ASSERT(MN__IMPLIES(request, out_match != MN_NULL));
    MN_ASSERT(MN__IMPLIES(request, out_pos != MN_NULL));
    while (pos < text_size) {
        unsigned char ch = 0;
        if (!reversed) {
            ch = (unsigned char)(text[pos]);
        } else {
            ch = (unsigned char)(text[(text_size - pos) - 1]);
        }
        if ((err = re__exec_dfa_run(&exec_dfa, ch))) {
            goto err_destroy_dfa;
        }
        if (request == 0) {
            if (re__exec_dfa_get_match_index(&exec_dfa) && bool_bail) {
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
    if ((err = re__exec_dfa_run(&exec_dfa, RE__EXEC_DFA_SYM_EOT))) {
        goto err_destroy_dfa;
    }
    if ((last_found_match = re__exec_dfa_get_match_index(&exec_dfa))) {
        last_found_pos = pos;
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

re_error re_is_match(re* reg, const char* text, mn_size text_size, re_anchor_type anchor_type) {
    /* no groups -- dfa can be used in all cases */
    re_error err = RE_ERROR_NONE;
    if (anchor_type == RE_ANCHOR_BOTH) {
        /* use dfa to search forward.
            * - can bail early if no threads left
            * - cannot bail early if match state found (does not need to check for match every byte) */
        if ((err = re__match_prepare_progs(reg, 1, 0, 0, 0))) {
            return err;
        }
        return re__match_dfa_driver(
            &reg->data->program,
            RE__PROG_ENTRY_DEFAULT,
            0, 0, 0, 0,
            text,
            text_size,
            MN_NULL,
            MN_NULL
        );
    } else if (anchor_type == RE_ANCHOR_START) {
        /* use dfa to search forward.
            * - can bail early if no threads left
            * - can bail early if match state found */
        if ((err = re__match_prepare_progs(reg, 1, 0, 0, 0))) {
            return err;
        }
        return re__match_dfa_driver(
            &reg->data->program,
            RE__PROG_ENTRY_DEFAULT,
            0, 1, 0, 0,
            text,
            text_size,
            MN_NULL,
            MN_NULL
        );
    } else if (anchor_type == RE_ANCHOR_END) {
        if ((err = re__match_prepare_progs(reg, 0, 1, 0, 0))) {
            return err;
        }
        return re__match_dfa_driver(
            &reg->data->program_reverse,
            RE__PROG_ENTRY_DEFAULT,
            0, 1, 1, 0,
            text,
            text_size,
            MN_NULL,
            MN_NULL
        );
    } else if (anchor_type == RE_UNANCHORED) {
        if ((err = re__match_prepare_progs(reg, 1, 0, 1, 0))) {
            return err;
        }
        return re__match_dfa_driver(
            &reg->data->program,
            RE__PROG_ENTRY_DOTSTAR,
            0, 1, 0, 0,
            text,
            text_size,
            MN_NULL,
            MN_NULL
        );
    } else {
        return RE_ERROR_INVALID;
    }
}

re_error re_match_groups(re* reg, const char* text, mn_size text_size, re_anchor_type anchor_type, mn_uint32 max_group, re_span* out_groups) {
    re_error err = RE_ERROR_NONE;
    if (max_group == 0) {
        return re_is_match(reg, text, text_size, anchor_type);
    } else if (max_group == 1) {
        mn_uint32 out_match;
        mn_size out_pos;
        re_error match_err;
        if (anchor_type == RE_ANCHOR_BOTH) {
            if ((err = re__match_prepare_progs(reg, 1, 0, 0, 0))) {
                return err;
            }
            match_err = re__match_dfa_driver(
                &reg->data->program,
                RE__PROG_ENTRY_DEFAULT,
                1, 0, 0, 0,
                text,
                text_size,
                &out_match,
                &out_pos
            );
            if (match_err == RE_MATCH) {
                if (out_pos != text_size) {
                    return RE_NOMATCH;
                } else {
                    out_groups[0].begin = 0;
                    out_groups[0].end = out_pos;
                    return RE_MATCH;
                }
            } else {
                return match_err;
            }
        } else if (anchor_type == RE_ANCHOR_START) {
            if ((err = re__match_prepare_progs(reg, 1, 0, 0, 0))) {
                return err;
            }
            match_err = re__match_dfa_driver(
                &reg->data->program,
                RE__PROG_ENTRY_DEFAULT,
                1, 0, 0, 0,
                text,
                text_size,
                &out_match,
                &out_pos
            );
            if (match_err == RE_MATCH) {
                out_groups[0].begin = 0;
                out_groups[0].end = out_pos;
                return RE_MATCH;
            } else {
                return match_err;
            }
        } else if (anchor_type == RE_ANCHOR_END) {
            if ((err = re__match_prepare_progs(reg, 0, 1, 0, 0))) {
                return err;
            }
            match_err = re__match_dfa_driver(
                &reg->data->program_reverse,
                RE__PROG_ENTRY_DEFAULT,
                1, 0, 1, 0,
                text,
                text_size,
                &out_match,
                &out_pos
            );
            if (match_err == RE_MATCH) {
                out_groups[0].begin = out_pos;
                out_groups[0].end = text_size;
                return RE_MATCH;
            } else {
                return match_err;
            }
        } else if (anchor_type == RE_UNANCHORED) {
            if ((err = re__match_prepare_progs(reg, 1, 1, 1, 0))) {
                return err;
            }
            match_err = re__match_dfa_driver(
                &reg->data->program,
                RE__PROG_ENTRY_DOTSTAR,
                1, 0, 0, 0,
                text,
                text_size,
                &out_match,
                &out_pos
            );
            if (match_err == RE_MATCH) {
                out_groups[0].end = out_pos;
                /* scan back to find the start */
                match_err = re__match_dfa_driver(
                    &reg->data->program_reverse,
                    RE__PROG_ENTRY_DEFAULT,
                    1, 0, 1, out_pos,
                    text,
                    text_size,
                    &out_match,
                    &out_pos
                );
                /* should ALWAYS match. */
                MN_ASSERT(match_err == RE_MATCH);
                out_groups[0].begin = out_pos;
                return RE_MATCH;
            } else {
                return match_err;
            }
        } else {
            return RE_ERROR_INVALID;
        }
    } else {
        re__exec_nfa exec_nfa;
        mn__str_view string_view;
        re__exec_nfa_init(&exec_nfa, &reg->data->program, max_group + 1);
        mn__str_view_init_n(&string_view, text, text_size);
        {
            mn_size pos;
            re__assert_type assert_ctx = RE__ASSERT_TYPE_TEXT_START | RE__ASSERT_TYPE_TEXT_START_ABSOLUTE;
            if ((err = re__exec_nfa_start(&exec_nfa, assert_ctx, 1))) {
                goto error;
            }
            for (pos = 0; pos < mn__str_view_size(&string_view); pos++) {
                mn_uint8 ch = (mn_uint8)mn__str_view_get_data(&string_view)[pos];
                mn_uint32 match;
                assert_ctx = re__match_next_assert_ctx(pos, mn__str_view_size(&string_view));
                if ((err = re__exec_nfa_run(&exec_nfa, ch, pos, assert_ctx))) {
                    return err;
                }
                match = re__exec_nfa_get_match_index(&exec_nfa);
                if (match) {
                    if (anchor_type == RE_ANCHOR_BOTH || anchor_type == RE_ANCHOR_END) {
                        /* can bail! */
                        break;
                    }
                }
            }
            err = re__exec_nfa_finish(&exec_nfa, out_groups, pos);
        }
    error:
        re__exec_nfa_destroy(&exec_nfa);
        return err;
    }
}

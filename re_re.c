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

re_error re_init(re* reg, const char* regex) {
    re_error err = RE_ERROR_NONE;
    mn__str_view regex_view;
    reg->data = (re_data*)MN_MALLOC(sizeof(re_data));
    if (!reg->data) {
        return RE_ERROR_NOMEM;
    }
    re__error_init(reg);
    mn__str_view_init_s(&regex_view, regex);
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

const char* re_get_error(re* reg, mn_size* error_len) {
    if (error_len != MN_NULL) {
        *error_len = mn__str_view_size(&reg->data->error_string_view);
    }
    return (const char*)mn__str_view_get_data(&reg->data->error_string_view);
}

mn_uint32 re_get_max_groups(re* reg) {
    return re__ast_root_get_num_groups(&reg->data->ast_root);
}

/*    | Match?  | Bounds? | Subs?
 * ---+---------+---------+-------
 * ^$ | DFA-F   | DFA-F   | NFA-F
 * ^- | DFA-F   | DFA-F   | NFA-F
 * -$ | DFA-R   | DFA-R   | NFA-R
 * -- | DFA-F   | DFA-F+R | NFA-F
 */

MN_INTERNAL re__ast_assert_type re__match_next_assert_ctx(mn_size pos, mn_size len) {
    re__ast_assert_type out = 0;
    if (pos == 0) {
        out |= RE__AST_ASSERT_TYPE_TEXT_START_ABSOLUTE | RE__AST_ASSERT_TYPE_TEXT_START;
    }
    if (pos == len) {
        out |= RE__AST_ASSERT_TYPE_TEXT_END_ABSOLUTE | RE__AST_ASSERT_TYPE_TEXT_END;
    }
    return out;
}

re_error re_match(re* reg, re_match_anchor_type anchor_type, re_match_groups_type groups_type, const char* string, mn_size string_size, re_span* out) {
    re_error err = RE_ERROR_NONE;
    re__exec_nfa exec_nfa;
    mn__str_view string_view;
    if (groups_type < RE_MATCH_GROUPS_NONE) {
        return RE_ERROR_INVALID;
    }
    if (groups_type > (re_match_groups_type)re__ast_root_get_num_groups(&reg->data->ast_root)) {
        return RE_ERROR_INVALID;
    }
    if (!re__prog_size(&reg->data->program)) {
        if ((err = re__compile_regex(&reg->data->compile, &reg->data->ast_root, &reg->data->program, 0))) {
            return err;
        }
    }
    if (!re__prog_size(&reg->data->program_reverse)) {
        if (anchor_type == RE_MATCH_ANCHOR_END || anchor_type == RE_MATCH_UNANCHORED) {
            if ((err = re__compile_regex(&reg->data->compile, &reg->data->ast_root, &reg->data->program_reverse, 1))) {
                return err;
            }
        }
    }
    if (groups_type == RE_MATCH_GROUPS_ALL) {
        groups_type = re__ast_root_get_num_groups(&reg->data->ast_root);
    }
    re__exec_nfa_init(&exec_nfa, &reg->data->program, groups_type);
    mn__str_view_init_n(&string_view, string, string_size);
    {
        mn_size pos;
        mn_size assert_ctx = RE__AST_ASSERT_TYPE_TEXT_START | RE__AST_ASSERT_TYPE_TEXT_START_ABSOLUTE;
        re_error res;
        if ((err = re__exec_nfa_start(&exec_nfa, assert_ctx, 1))) {
            goto error;
        }
        for (pos = 0; pos < mn__str_view_size(&string_view); pos++) {
            mn_char ch = mn__str_view_get_data(&string_view)[pos];
            assert_ctx = re__match_next_assert_ctx(pos, mn__str_view_size(&string_view));
            res = re__exec_nfa_run(&exec_nfa, ch, pos, assert_ctx);
            if (res >= 0) {
                if (anchor_type == RE_MATCH_ANCHOR_BOTH || anchor_type == RE_MATCH_ANCHOR_END) {
                    /* can bail! */
                    break;
                }
            } else if (res == RE_NOMATCH) {
                /* continue... */
            } else {
                goto error;
            }
        }
        err = re__exec_nfa_finish(&exec_nfa, out, pos);
    }
error:
    re__exec_nfa_destroy(&exec_nfa);
    return err;
}

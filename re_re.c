#include "re_internal.h"

RE_INTERNAL void re__error_init(re* reg) {
    re__str_init(&reg->data->error_string);
    re__str_view_init_null(&reg->data->error_string_view);
}

RE_INTERNAL void re__error_destroy(re* reg) {
    re__str_destroy(&reg->data->error_string);
}

/* Doesn't take ownership of error_str */
/* We don't return an error here because it's expected that you are already in
 * the middle of cleaning up after an error */
/* This is useful because it allows error_string to be a const pointer,
 * allowing error messages to be saved as just const strings */
RE_INTERNAL void re__set_error_str(re* reg, const re__str* error_str) {
    re_error err = RE_ERROR_NONE;
    /* Clear the last error */
    re__error_destroy(reg);
    if ((err = re__str_init_copy(&reg->data->error_string, error_str))) {
        re__set_error_generic(reg, err);
    }
    re__str_view_init(&reg->data->error_string_view, &reg->data->error_string);
}

RE_INTERNAL void re__set_error_generic(re* reg, re_error err) {
    if (err == RE_ERROR_NOMEM) {
        /* No memory: not guaranteed if we can allocate a string. */
        re__str_view_init_s(&reg->data->error_string_view, "out of memory");
    } else {
        /* This function is only valid for generic errors */
        RE__ASSERT_UNREACHED();
    }
}

re_error re_init(re* reg, const char* regex) {
    re_error err = RE_ERROR_NONE;
    re__str_view regex_view;
    reg->data = (re_data*)RE_MALLOC(sizeof(re_data));
    if (!reg->data) {
        return RE_ERROR_NOMEM;
    }
    re__error_init(reg);
    re__str_view_init_s(&regex_view, regex);
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
    RE_FREE(reg->data);
}

const char* re_get_error(re* reg, re_size* error_len) {
    if (error_len != RE_NULL) {
        *error_len = re__str_view_size(&reg->data->error_string_view);
    }
    return (const char*)re__str_view_get_data(&reg->data->error_string_view);
}

re_uint32 re_get_max_groups(re* reg) {
    return re__ast_root_get_num_groups(&reg->data->ast_root);
}

/*    | Match?  | Bounds? | Subs?
 * ---+---------+---------+-------
 * ^$ | DFA-F   | DFA-F   | NFA-F
 * ^- | DFA-F   | DFA-F   | NFA-F
 * -$ | DFA-R   | DFA-R   | NFA-R
 * -- | DFA-F   | DFA-F+R | NFA-F
 */

re_error re_match(re* reg, re_match_anchor_type anchor_type, re_match_groups_type groups_type, const char* string, re_size string_size, re_span* out) {
    re_error err = RE_ERROR_NONE;
    re__exec_nfa exec_nfa;
    re__str_view string_view;
    re_uint32 nfa_num_groups;
    if (groups_type < RE_MATCH_GROUPS_NONE) {
        return RE_ERROR_INVALID;
    }
    nfa_num_groups = (re_uint32)(groups_type + 1);
    if (nfa_num_groups > re__ast_root_get_num_groups(&reg->data->ast_root) + 1) {
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
    re__exec_nfa_init(&exec_nfa);
    re__str_view_init_n(&string_view, string, string_size);
    nfa_num_groups = (re_uint32)(groups_type + 1);
    if ((err = re__exec_nfa_do(&exec_nfa, &reg->data->program, anchor_type, nfa_num_groups, string_view, out))) {
        re__exec_nfa_destroy(&exec_nfa);
        return err;
    }
    re__exec_nfa_destroy(&exec_nfa);
    return err;
}

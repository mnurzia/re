#include "re_internal.h"

#include <string.h>

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
    re__compile_init(&reg->data->compile, reg);
    re__exec_init(&reg->data->exec, reg);
    if ((err = re__parse_str(&reg->data->parse, &regex_view))) {
        return err;
    }
    return err;
}

void re_destroy(re* reg) {
    re__exec_destroy(&reg->data->exec);
    re__compile_destroy(&reg->data->compile);
    re__prog_destroy(&reg->data->program);
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

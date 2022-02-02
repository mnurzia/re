#include "re_internal.h"

#include <string.h>

int main(void) {
    re reg;
    printf("\x1b[3JSTART\n");
    re_init(&reg, "");
    {
        re_error err = RE_ERROR_NONE;
        const char* regex = "[0-9a-fA-F]*";
        char text[] = "092321af32c";
        if ((err = re__parse_regex(&reg.data->parse, strlen(regex), (const re_char*)regex))) {
            printf("error: %s\n", re_get_error(&reg, RE_NULL));
            goto err;
        }
        if ((err = re__compile_regex(&reg.data->compile))) {
            printf("error: %s\n", re_get_error(&reg, RE_NULL));
            goto err;
        }
        if ((err = re__exec_nfa(&reg.data->exec, (const re_char*)text, sizeof(text) - 1))) {
            printf("error: %s\n", re_get_error(&reg, RE_NULL));
            goto err;
        }
        /*re_error err = RE_ERROR_NONE;*/
        /*
        re__charclass cls;
        re__rune_range range;
        re__compile_charclass comp;
        re__prog prog;
        re__prog_inst fail_inst;
        re__compile_patches patches;
        re_int32 i;
        re__prog_init(&prog);
        re__prog_inst_init_fail(&fail_inst);
        re__prog_add(&prog, fail_inst);
        re__compile_patches_init(&patches);
        re__charclass_init(&cls);
        range.min = 0;
        range.max = 0x10FFFF;
        re__charclass_push(&cls, range);
        re__compile_charclass_init(&comp);
        re__compile_charclass_gen(&comp, &cls, &prog, &patches);
        re__compile_charclass_dump(&comp, 0, 0);
        re__prog_debug_dump(&prog);
        re__compile_patches_dump(&patches, &prog);*/
    }
err:
    re_destroy(&reg);
    return 0;
}

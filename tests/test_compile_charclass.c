#include "test_compile_charclass.h"

TEST(t_compile_charclass_init) {
    re__compile_charclass cc;
    re__compile_charclass_init(&cc);
    re__compile_charclass_destroy(&cc);
    PASS();
}

TEST(t_compile_charclass_gen) {
    re__prog prog;
    re__compile_patches patches;
    re__compile_charclass cc;
    re__prog_inst fail_inst;
    re__charclass charclass;
    re__rune_range full_range;
    full_range.min = 0;
    full_range.max = 0x10FFFF;
    re__prog_init(&prog);
    re__prog_inst_init_fail(&fail_inst);
    re__prog_add(&prog, fail_inst);
    re__compile_patches_init(&patches);
    re__charclass_init(&charclass);
    re__charclass_push(&charclass, full_range);
    re__compile_charclass_init(&cc);
    re__compile_charclass_gen(&cc, &charclass, &prog, &patches);
    re__compile_charclass_destroy(&cc);
    re__charclass_destroy(&charclass);
    re__prog_destroy(&prog);
    PASS();
}

SUITE(s_compile_charclass) {
    RUN_TEST(t_compile_charclass_init);
    RUN_TEST(t_compile_charclass_gen);
}

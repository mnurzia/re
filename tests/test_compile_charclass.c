#include "test_compile_charclass.h"

TEST(t_compile_charclass_init) {
    re__compile_charclass cc;
    re__compile_charclass_init(&cc);
    re__compile_charclass_destroy(&cc);
    PASS();
}

SUITE(s_compile_charclass) {
    RUN_TEST(t_compile_charclass_init);
}

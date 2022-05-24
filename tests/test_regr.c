#include "test_regr.h"

TEST(t_regr_parse_fuzz) {
    re reg;
    /* 2022-05-04: libfuzzer */
    re_init_sz_flags(&reg, "\x28\x00\x00\x3f", 4, 0);
    re_destroy(&reg);
    PASS();
}

SUITE(s_regr) {
    RUN_TEST(t_regr_parse_fuzz);
}

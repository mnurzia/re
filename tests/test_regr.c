#include "test_regr.h"

TEST(t_regr_parse_fuzz) {
    re reg;
    /* 2022-05-04: libfuzzer */
    re_init_sz_flags(&reg, "\x28\x00\x00\x3f", 4, 0);
    re_destroy(&reg);
    re_init_sz_flags(&reg, "|\xf8$\xe5", 4, 0);
    re_destroy(&reg);
    re_init_sz_flags(&reg, "||[\x01~-||", 8, 0);
    re_destroy(&reg);
    re_init_sz_flags(&reg, "[^?\0E\0\\D]", 8, 0);
    re_destroy(&reg);
    re_init_sz_flags(&reg, ".\\QN|Q", 6, 0);
    re_destroy(&reg);
    re_init_sz_flags(&reg, "[^\0\0\x08\0=============[\\W..[\\\0([\\..5=M,]", 37, 0);
    re_destroy(&reg);
    PASS();
}

SUITE(s_regr) {
    RUN_TEST(t_regr_parse_fuzz);
}

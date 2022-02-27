#include "test_exec.h"

TEST(t_exec_nfa) {
    re reg;
    re_match_data match_data;
    re_init(&reg, "(abc)(def)");
    re_match(&reg, RE_MATCH_ANCHOR_FULL, RE_MATCH_GROUPS_ALL, "abcdef", 6, &match_data);
    re_destroy(&reg);
    re_match_data_destroy(&match_data);
    PASS();
}

SUITE(s_exec) {
    RUN_TEST(t_exec_nfa);   
}

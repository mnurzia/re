#include "test_exec.h"

TEST(t_exec_nfa) {
    /*re reg;
    re__str_view in_text;
    re_init(&reg, "a*");
    re__str_view_init_s(&in_text, "aaaaaa");
    re__compile_regex(&reg.data->compile, &reg.data->ast_root, &reg.data->program);
    re__exec_nfa(&reg.data->exec, &reg.data->program, re__ast_root_get_num_groups(&reg.data->ast_root), in_text);
    re_destroy(&reg);*/
    PASS();
}

SUITE(s_exec) {
    RUN_TEST(t_exec_nfa);   
}

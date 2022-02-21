#include "test_exec.h"

TEST(t_exec_nfa) {
    re re;
    re__str_view in_data;
    re__str_view_init_s(&in_data, "aaaaaaaaa");
    ASSERT(!re_init(&re, "a*"));
    ASSERT(!re__compile_regex(&re.data->compile));
    ASSERT(!re__exec_nfa(&re.data->exec, in_data));
    re_destroy(&re);
    PASS();
}

SUITE(s_exec) {
    RUN_TEST(t_exec_nfa);   
}

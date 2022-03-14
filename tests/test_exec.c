#include "test_exec.h"

TEST(t_exec_save_init) {
    re__exec_save exec_save;
    re__exec_save_init(&exec_save);
    PASS();
}

TEST(t_exec_save_get_new) {
    re__exec_save exec_save;
    mn_uint32 slots_per_thrd = (mn_uint32)RAND_PARAM(60);
    mn_int32 new_ref;
    mn_uint32 i;
    mn_size refcount;
    re__exec_save_init(&exec_save);
    re__exec_save_set_slots_per_thrd(&exec_save, slots_per_thrd);
    ASSERT(!re__exec_save_get_new(&exec_save, &new_ref));
    refcount = re__exec_save_get_refs(&exec_save, new_ref);
    ASSERT_EQ(refcount, 1);
    {
        mn_size* data = re__exec_save_get_slots(&exec_save, new_ref);
        for (i = 0; i < slots_per_thrd; i++) {
            ASSERT(data[i] == 0);
        }
    }
    re__exec_save_destroy(&exec_save);
    PASS();
}

SUITE(s_exec_save) {
    RUN_TEST(t_exec_save_init);
    FUZZ_TEST(t_exec_save_get_new);
}

TEST(t_exec_nfa) {
    re reg;
    re_span* groups = MN_MALLOC(sizeof(re_span) * 1);
    ASSERT(!re_init(&reg, "(abc)(def)(ghi)"));
    ASSERT(!re_match(&reg, RE_MATCH_ANCHOR_BOTH, RE_MATCH_GROUPS_MATCH_BOUNDARIES, "abcdefghi", 9, groups));
    re_destroy(&reg);
    MN_FREE(groups);
    PASS();
}

SUITE(s_exec) {
    RUN_SUITE(s_exec_save);
    RUN_TEST(t_exec_nfa);
}

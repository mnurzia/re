#include "test_prog.h"

#include "test_range.h"

TEST(t_prog_inst_init_byte) {
    re__prog_inst inst;
    re_uint8 byte = RAND_PARAM(0xFF);
    re__prog_inst_init_byte(&inst, byte);
    ASSERT_EQ(re__prog_inst_get_type(&inst), RE__PROG_INST_TYPE_BYTE);
    ASSERT_EQ(re__prog_inst_get_byte(&inst), byte);
    PASS();
}

TEST(t_prog_inst_init_byte_range) {
    re__prog_inst inst;
    re__byte_range range = re__byte_range_random();
    re__prog_inst_init_byte_range(&inst, range);
    ASSERT_EQ(re__prog_inst_get_type(&inst), RE__PROG_INST_TYPE_BYTE_RANGE);
    ASSERT_EQ(re__prog_inst_get_byte_min(&inst), range.min);
    ASSERT_EQ(re__prog_inst_get_byte_max(&inst), range.max);
    PASS();
}

TEST(t_prog_inst_init_split) {
    re__prog_inst inst;
    re__prog_loc primary = RAND_PARAM(RE__PROG_SIZE_MAX);
    re__prog_loc secondary = RAND_PARAM(RE__PROG_SIZE_MAX);
    re__prog_inst_init_split(&inst, primary, secondary);
    ASSERT_EQ(re__prog_inst_get_type(&inst), RE__PROG_INST_TYPE_SPLIT);
    ASSERT_EQ(re__prog_inst_get_primary(&inst), primary);
    ASSERT_EQ(re__prog_inst_get_split_secondary(&inst), secondary);
    PASS();
}

TEST(t_prog_inst_init_match) {
    re__prog_inst inst;
    re_uint32 match_idx = RAND_PARAM(RE__PROG_SIZE_MAX);
    re__prog_inst_init_match(&inst, match_idx);
    ASSERT_EQ(re__prog_inst_get_type(&inst), RE__PROG_INST_TYPE_MATCH);
    ASSERT_EQ(re__prog_inst_get_match_idx(&inst), match_idx);
    PASS();
}

TEST(t_prog_inst_init_fail) {
    re__prog_inst inst;
    re__prog_inst_init_fail(&inst);
    ASSERT_EQ(re__prog_inst_get_type(&inst), RE__PROG_INST_TYPE_FAIL);
    PASS();
}

TEST(t_prog_inst_init_assert) {
    re__prog_inst inst;
    re_uint32 assert_ctx = RAND_PARAM(RE__AST_ASSERT_TYPE_MAX);
    re__prog_inst_init_assert(&inst, assert_ctx);
    ASSERT_EQ(re__prog_inst_get_type(&inst), RE__PROG_INST_TYPE_ASSERT);
    ASSERT_EQ(re__prog_inst_get_assert_ctx(&inst), assert_ctx);
    PASS();
}

TEST(t_prog_inst_init_save) {
    re__prog_inst inst;
    re_uint32 save_idx = RAND_PARAM(RE__PROG_SIZE_MAX);
    re__prog_inst_init_save(&inst, save_idx);
    ASSERT_EQ(re__prog_inst_get_type(&inst), RE__PROG_INST_TYPE_SAVE);
    ASSERT_EQ(re__prog_inst_get_save_idx(&inst), save_idx);
    PASS();
}


SUITE(s_prog_inst_init) {
    FUZZ_TEST(t_prog_inst_init_byte);
    FUZZ_TEST(t_prog_inst_init_byte_range);
    FUZZ_TEST(t_prog_inst_init_split);
    FUZZ_TEST(t_prog_inst_init_match);
    RUN_TEST(t_prog_inst_init_fail);
    FUZZ_TEST(t_prog_inst_init_assert);
    FUZZ_TEST(t_prog_inst_init_save);
}

SUITE(s_prog_inst) {
    RUN_SUITE(s_prog_inst_init);
}

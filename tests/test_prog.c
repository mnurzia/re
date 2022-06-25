#include "test_prog.h"

#include "test_ast.h"
#include "test_range.h"

int re__prog_test_equals(re__prog* a, re__prog* b)
{
  re__prog_loc i;
  if (re__prog_size(a) != re__prog_size(b)) {
    return 0;
  }
  for (i = 0; i < re__prog_size(a); i++) {
    if (!re__prog_inst_equals(re__prog_get(a, i), re__prog_get(b, i))) {
      return 0;
    }
  }
  return 1;
}

const char* re__prog_inst_type_names[] = {
    "byte", "byte_range", "split", "match", "fail", "save", "assert"};

int re__prog_inst_to_sym(sym_build* parent, re__prog_inst prog_inst)
{
  sym_build build;
  re__prog_inst_type inst_type = re__prog_inst_get_type(&prog_inst);
  SYM_PUT_EXPR(parent, &build);
  SYM_PUT_STR(&build, re__prog_inst_type_names[inst_type]);
  if (inst_type == RE__PROG_INST_TYPE_BYTE) {
    SYM_PUT_NUM(&build, re__prog_inst_get_byte(&prog_inst));
  } else if (inst_type == RE__PROG_INST_TYPE_BYTE_RANGE) {
    SYM_PUT_NUM(&build, re__prog_inst_get_byte_min(&prog_inst));
    SYM_PUT_NUM(&build, re__prog_inst_get_byte_max(&prog_inst));
  } else if (inst_type == RE__PROG_INST_TYPE_MATCH) {
    SYM_PUT_NUM(&build, (mn_int32)re__prog_inst_get_match_idx(&prog_inst));
  } else if (inst_type == RE__PROG_INST_TYPE_SAVE) {
    SYM_PUT_NUM(&build, (mn_int32)re__prog_inst_get_save_idx(&prog_inst));
  } else if (inst_type == RE__PROG_INST_TYPE_ASSERT) {
    re__assert_type_to_sym(&build, re__prog_inst_get_assert_ctx(&prog_inst));
  }
  if (inst_type == RE__PROG_INST_TYPE_BYTE ||
      inst_type == RE__PROG_INST_TYPE_BYTE_RANGE ||
      inst_type == RE__PROG_INST_TYPE_SPLIT ||
      inst_type == RE__PROG_INST_TYPE_MATCH ||
      inst_type == RE__PROG_INST_TYPE_SAVE ||
      inst_type == RE__PROG_INST_TYPE_ASSERT) {
    SYM_PUT_NUM(&build, (mn_int32)re__prog_inst_get_primary(&prog_inst));
  }
  if (inst_type == RE__PROG_INST_TYPE_SPLIT) {
    SYM_PUT_NUM(
        &build, (mn_int32)re__prog_inst_get_split_secondary(&prog_inst));
  }
  return SYM_OK;
}

int re__prog_inst_from_sym(sym_walk* parent, re__prog_inst* prog_inst)
{
  sym_walk walk;
  const char* type_str;
  mptest_size type_str_len;
  mn__str_view a, b;
  re__prog_inst_type inst_type;
  SYM_GET_EXPR(parent, &walk);
  SYM_GET_STR(&walk, &type_str, &type_str_len);
  mn__str_view_init_n(&a, type_str, type_str_len);
  for (inst_type = 0; inst_type < RE__PROG_INST_TYPE_MAX; inst_type++) {
    mn__str_view_init_s(&b, re__prog_inst_type_names[inst_type]);
    if (mn__str_view_cmp(&a, &b) == 0) {
      break;
    }
  }
  if (inst_type == RE__PROG_INST_TYPE_MAX) {
    return SYM_INVALID;
  }
  if (inst_type == RE__PROG_INST_TYPE_BYTE) {
    mn_int32 num;
    SYM_GET_NUM(&walk, &num);
    re__prog_inst_init_byte(prog_inst, (mn_uint8)num);
  } else if (inst_type == RE__PROG_INST_TYPE_BYTE_RANGE) {
    mn_int32 min, max;
    re__byte_range br;
    SYM_GET_NUM(&walk, &min);
    SYM_GET_NUM(&walk, &max);
    br.min = (mn_uint8)min;
    br.max = (mn_uint8)max;
    re__prog_inst_init_byte_range(prog_inst, br);
  } else if (inst_type == RE__PROG_INST_TYPE_SPLIT) {
    re__prog_inst_init_split(
        prog_inst, RE__PROG_LOC_INVALID, RE__PROG_LOC_INVALID);
  } else if (inst_type == RE__PROG_INST_TYPE_MATCH) {
    mn_int32 match_idx;
    SYM_GET_NUM(&walk, &match_idx);
    re__prog_inst_init_match(prog_inst, (mn_uint32)match_idx);
  } else if (inst_type == RE__PROG_INST_TYPE_FAIL) {
    re__prog_inst_init_fail(prog_inst);
  } else if (inst_type == RE__PROG_INST_TYPE_SAVE) {
    mn_int32 save_idx;
    SYM_GET_NUM(&walk, &save_idx);
    re__prog_inst_init_save(prog_inst, (mn_uint32)save_idx);
  } else if (inst_type == RE__PROG_INST_TYPE_ASSERT) {
    re__assert_type assert_ctx;
    SYM_GET_SUB(&walk, re__assert_type, &assert_ctx);
    re__prog_inst_init_assert(prog_inst, (mn_uint32)assert_ctx);
  }
  if (inst_type == RE__PROG_INST_TYPE_BYTE ||
      inst_type == RE__PROG_INST_TYPE_BYTE_RANGE ||
      inst_type == RE__PROG_INST_TYPE_SPLIT ||
      inst_type == RE__PROG_INST_TYPE_MATCH ||
      inst_type == RE__PROG_INST_TYPE_SAVE ||
      inst_type == RE__PROG_INST_TYPE_ASSERT) {
    mn_int32 primary;
    SYM_GET_NUM(&walk, &primary);
    re__prog_inst_set_primary(prog_inst, (re__prog_loc)primary);
  }
  if (inst_type == RE__PROG_INST_TYPE_SPLIT) {
    mn_int32 secondary;
    SYM_GET_NUM(&walk, &secondary);
    re__prog_inst_set_split_secondary(prog_inst, (re__prog_loc)secondary);
  }
  return SYM_OK;
}

int re__prog_to_sym(sym_build* parent, re__prog prog)
{
  sym_build build;
  re__prog_loc i;
  SYM_PUT_EXPR(parent, &build);
  SYM_PUT_TYPE(&build, "prog");
  for (i = 0; i < re__prog_size(&prog); i++) {
    re__prog_inst inst = *re__prog_get(&prog, i);
    SYM_PUT_SUB(&build, re__prog_inst, inst);
  }
  return SYM_OK;
}

int re__prog_from_sym(sym_walk* parent, re__prog* prog)
{
  sym_walk walk;
  SYM_GET_EXPR(parent, &walk);
  SYM_CHECK_TYPE(&walk, "prog");
  re__prog_init(prog);
  while (SYM_MORE(&walk)) {
    re__prog_inst inst;
    SYM_GET_SUB(&walk, re__prog_inst, &inst);
    re__prog_add(prog, inst);
  }
  return SYM_OK;
}

TEST(t_prog_inst_init_byte)
{
  re__prog_inst inst;
  mn_uint8 byte = (mn_uint8)RAND_PARAM(0xFF);
  re__prog_inst_init_byte(&inst, byte);
  ASSERT_EQ(re__prog_inst_get_type(&inst), RE__PROG_INST_TYPE_BYTE);
  ASSERT_EQ(re__prog_inst_get_byte(&inst), byte);
  PASS();
}

TEST(t_prog_inst_init_byte_range)
{
  re__prog_inst inst;
  re__byte_range range = re__byte_range_random();
  re__prog_inst_init_byte_range(&inst, range);
  ASSERT_EQ(re__prog_inst_get_type(&inst), RE__PROG_INST_TYPE_BYTE_RANGE);
  ASSERT_EQ(re__prog_inst_get_byte_min(&inst), range.min);
  ASSERT_EQ(re__prog_inst_get_byte_max(&inst), range.max);
  PASS();
}

TEST(t_prog_inst_init_split)
{
  re__prog_inst inst;
  re__prog_loc primary = (re__prog_loc)RAND_PARAM(RE__PROG_SIZE_MAX);
  re__prog_loc secondary = (re__prog_loc)RAND_PARAM(RE__PROG_SIZE_MAX);
  re__prog_inst_init_split(&inst, primary, secondary);
  ASSERT_EQ(re__prog_inst_get_type(&inst), RE__PROG_INST_TYPE_SPLIT);
  ASSERT_EQ(re__prog_inst_get_primary(&inst), primary);
  ASSERT_EQ(re__prog_inst_get_split_secondary(&inst), secondary);
  PASS();
}

TEST(t_prog_inst_init_match)
{
  re__prog_inst inst;
  mn_uint32 match_idx = (mn_uint32)RAND_PARAM(RE__PROG_SIZE_MAX);
  re__prog_inst_init_match(&inst, match_idx);
  ASSERT_EQ(re__prog_inst_get_type(&inst), RE__PROG_INST_TYPE_MATCH);
  ASSERT_EQ(re__prog_inst_get_match_idx(&inst), match_idx);
  PASS();
}

TEST(t_prog_inst_init_fail)
{
  re__prog_inst inst;
  re__prog_inst_init_fail(&inst);
  ASSERT_EQ(re__prog_inst_get_type(&inst), RE__PROG_INST_TYPE_FAIL);
  PASS();
}

TEST(t_prog_inst_init_assert)
{
  re__prog_inst inst;
  mn_uint32 assert_ctx = (mn_uint32)RAND_PARAM(RE__ASSERT_TYPE_MAX);
  re__prog_inst_init_assert(&inst, assert_ctx);
  ASSERT_EQ(re__prog_inst_get_type(&inst), RE__PROG_INST_TYPE_ASSERT);
  ASSERT_EQ(re__prog_inst_get_assert_ctx(&inst), assert_ctx);
  PASS();
}

TEST(t_prog_inst_init_save)
{
  re__prog_inst inst;
  mn_uint32 save_idx = (mn_uint32)RAND_PARAM(RE__PROG_SIZE_MAX);
  re__prog_inst_init_save(&inst, save_idx);
  ASSERT_EQ(re__prog_inst_get_type(&inst), RE__PROG_INST_TYPE_SAVE);
  ASSERT_EQ(re__prog_inst_get_save_idx(&inst), save_idx);
  PASS();
}

void re__inst_random(re__prog_inst* inst)
{
  re__prog_inst_type inst_type =
      (re__prog_inst_type)RAND_PARAM(RE__PROG_INST_TYPE_MAX);
  if (inst_type == RE__PROG_INST_TYPE_BYTE) {
    re__prog_inst_init_byte(inst, (mn_uint8)RAND_PARAM(256));
  } else if (inst_type == RE__PROG_INST_TYPE_BYTE_RANGE) {
    re__prog_inst_init_byte_range(inst, re__byte_range_random());
  } else if (inst_type == RE__PROG_INST_TYPE_SPLIT) {
    re__prog_inst_init_split(
        inst, (re__prog_loc)RAND_PARAM(RE__PROG_SIZE_MAX),
        (re__prog_loc)RAND_PARAM(RE__PROG_SIZE_MAX));
  } else if (inst_type == RE__PROG_INST_TYPE_MATCH) {
    re__prog_inst_init_match(inst, (mn_uint32)RAND_PARAM(256));
  } else if (inst_type == RE__PROG_INST_TYPE_SAVE) {
    re__prog_inst_init_save(inst, (mn_uint32)RAND_PARAM(256));
  } else if (inst_type == RE__PROG_INST_TYPE_ASSERT) {
    re__prog_inst_init_assert(
        inst, (re__assert_type)RAND_PARAM(RE__ASSERT_TYPE_MAX - 1));
  }
}

TEST(t_prog_inst_set_get_primary)
{
  re__prog_inst inst;
  re__prog_loc primary = (re__prog_loc)RAND_PARAM(RE__PROG_SIZE_MAX);
  re__inst_random(&inst);
  re__prog_inst_set_primary(&inst, primary);
  ASSERT_EQ(re__prog_inst_get_primary(&inst), primary);
  if (re__prog_inst_get_type(&inst) == RE__PROG_INST_TYPE_SPLIT) {
    re__prog_loc secondary = (re__prog_loc)RAND_PARAM(RE__PROG_SIZE_MAX);
    re__prog_inst_set_split_secondary(&inst, secondary);
    ASSERT_EQ(re__prog_inst_get_split_secondary(&inst), secondary);
  }
  PASS();
}

TEST(t_prog_inst_equals_difftype)
{
  re__prog_inst a;
  re__prog_inst b;
  re__prog_inst_init_fail(&a);
  re__prog_inst_init_byte(&b, 5);
  ASSERT(!re__prog_inst_equals(&a, &b));
  PASS();
}

TEST(t_prog_inst_equals_fail)
{
  re__prog_inst a;
  re__prog_inst b;
  re__prog_inst_init_fail(&a);
  re__prog_inst_init_fail(&b);
  ASSERT(re__prog_inst_equals(&a, &b));
  PASS();
}

TEST(t_prog_inst_equals_byte)
{
  re__prog_inst a;
  re__prog_inst b;
  re__prog_inst_init_byte(&a, 0x5);
  re__prog_inst_init_byte(&b, 0x5);
  ASSERT(re__prog_inst_equals(&a, &b));
  PASS();
}

TEST(t_prog_inst_equals_byte_not)
{
  re__prog_inst a;
  re__prog_inst b;
  re__prog_inst_init_byte(&a, 0x6);
  re__prog_inst_init_byte(&b, 0x5);
  ASSERT(!re__prog_inst_equals(&a, &b));
  PASS();
}

TEST(t_prog_inst_equals_byte_range)
{
  re__prog_inst a;
  re__prog_inst b;
  re__byte_range ar;
  re__byte_range br;
  ar.min = 0x5;
  ar.max = 0x10;
  br.min = 0x5;
  br.max = 0x10;
  re__prog_inst_init_byte_range(&a, ar);
  re__prog_inst_init_byte_range(&b, br);
  ASSERT(re__prog_inst_equals(&a, &b));
  PASS();
}

TEST(t_prog_inst_equals_byte_range_not)
{
  re__prog_inst a;
  re__prog_inst b;
  re__byte_range ar;
  re__byte_range br;
  ar.min = 0x6;
  ar.max = 0x9;
  br.min = 0x5;
  br.max = 0x10;
  re__prog_inst_init_byte_range(&a, ar);
  re__prog_inst_init_byte_range(&b, br);
  ASSERT(!re__prog_inst_equals(&a, &b));
  PASS();
}

TEST(t_prog_inst_equals_split)
{
  re__prog_inst a;
  re__prog_inst b;
  re__prog_inst_init_split(&a, 5, 5);
  re__prog_inst_init_split(&b, 5, 5);
  ASSERT(re__prog_inst_equals(&a, &b));
  PASS();
}

TEST(t_prog_inst_equals_split_not)
{
  re__prog_inst a;
  re__prog_inst b;
  re__prog_inst_init_split(&a, 5, 5);
  re__prog_inst_init_split(&b, 100, 1000);
  ASSERT(!re__prog_inst_equals(&a, &b));
  PASS();
}

TEST(t_prog_inst_equals_match)
{
  re__prog_inst a;
  re__prog_inst b;
  re__prog_inst_init_match(&a, 5);
  re__prog_inst_init_match(&b, 5);
  ASSERT(re__prog_inst_equals(&a, &b));
  PASS();
}

TEST(t_prog_inst_equals_match_not)
{
  re__prog_inst a;
  re__prog_inst b;
  re__prog_inst_init_match(&a, 5);
  re__prog_inst_init_match(&b, 10);
  ASSERT(!re__prog_inst_equals(&a, &b));
  PASS();
}

TEST(t_prog_inst_equals_save)
{
  re__prog_inst a;
  re__prog_inst b;
  re__prog_inst_init_save(&a, 5);
  re__prog_inst_init_save(&b, 5);
  ASSERT(re__prog_inst_equals(&a, &b));
  PASS();
}

TEST(t_prog_inst_equals_save_not)
{
  re__prog_inst a;
  re__prog_inst b;
  re__prog_inst_init_save(&a, 5);
  re__prog_inst_init_save(&b, 10);
  ASSERT(!re__prog_inst_equals(&a, &b));
  PASS();
}

TEST(t_prog_inst_equals_assert)
{
  re__prog_inst a;
  re__prog_inst b;
  re__prog_inst_init_assert(&a, RE__ASSERT_TYPE_TEXT_START);
  re__prog_inst_init_assert(&b, RE__ASSERT_TYPE_TEXT_START);
  ASSERT(re__prog_inst_equals(&a, &b));
  PASS();
}

TEST(t_prog_inst_equals_assert_not)
{
  re__prog_inst a;
  re__prog_inst b;
  re__prog_inst_init_save(&a, RE__ASSERT_TYPE_TEXT_START);
  re__prog_inst_init_save(&b, RE__ASSERT_TYPE_TEXT_END);
  ASSERT(!re__prog_inst_equals(&a, &b));
  PASS();
}

SUITE(s_prog_inst)
{
  FUZZ_TEST(t_prog_inst_init_byte);
  FUZZ_TEST(t_prog_inst_init_byte_range);
  FUZZ_TEST(t_prog_inst_init_split);
  FUZZ_TEST(t_prog_inst_init_match);
  RUN_TEST(t_prog_inst_init_fail);
  FUZZ_TEST(t_prog_inst_init_assert);
  FUZZ_TEST(t_prog_inst_init_save);
  FUZZ_TEST(t_prog_inst_set_get_primary);
  RUN_TEST(t_prog_inst_equals_difftype);
  RUN_TEST(t_prog_inst_equals_fail);
  RUN_TEST(t_prog_inst_equals_byte);
  RUN_TEST(t_prog_inst_equals_byte_not);
  RUN_TEST(t_prog_inst_equals_byte_range);
  RUN_TEST(t_prog_inst_equals_byte_range_not);
  RUN_TEST(t_prog_inst_equals_split);
  RUN_TEST(t_prog_inst_equals_split_not);
  RUN_TEST(t_prog_inst_equals_match);
  RUN_TEST(t_prog_inst_equals_match_not);
  RUN_TEST(t_prog_inst_equals_save);
  RUN_TEST(t_prog_inst_equals_save_not);
  RUN_TEST(t_prog_inst_equals_assert);
  RUN_TEST(t_prog_inst_equals_assert_not);
}

TEST(t_prog_init)
{
  re__prog prog;
  re__prog_entry idx;
  re__prog_init(&prog);
  ASSERT_EQ(re__prog_size(&prog), 0);
  for (idx = 0; idx < RE__PROG_ENTRY_MAX; idx++) {
    ASSERT_EQ(re__prog_get_entry(&prog, idx), RE__PROG_LOC_INVALID);
  }
  re__prog_destroy(&prog);
  PASS();
}

TEST(t_prog_size)
{
  re__prog prog;
  re__prog_inst a;
  re__prog_inst_init_fail(&a);
  re__prog_init(&prog);
  ASSERT_ERR_NOMEM(re__prog_add(&prog, a), error);
  ASSERT_ERR_NOMEM(re__prog_add(&prog, a), error);
  ASSERT_EQ(re__prog_size(&prog), 2);
error:
  re__prog_destroy(&prog);
  PASS();
}

TEST(t_prog_get)
{
  re__prog prog;
  re__prog_inst a;
  re__prog_inst* b;
  re__prog_inst_init_fail(&a);
  re__prog_init(&prog);
  ASSERT_ERR_NOMEM(re__prog_add(&prog, a), error);
  b = re__prog_get(&prog, 0);
  ASSERT(re__prog_inst_equals(&a, b));
error:
  re__prog_destroy(&prog);
  PASS();
}

TEST(t_prog_get_const)
{
  re__prog prog;
  re__prog_inst a;
  const re__prog_inst* b;
  re__prog_inst_init_fail(&a);
  re__prog_init(&prog);
  ASSERT_ERR_NOMEM(re__prog_add(&prog, a), error);
  b = re__prog_get_const(&prog, 0);
  ASSERT(re__prog_inst_equals(&a, b));
error:
  re__prog_destroy(&prog);
  PASS();
}

TEST(t_prog_add)
{
  re__prog prog;
  re__prog_inst a;
  re__prog_inst* b;
  re__prog_inst_init_fail(&a);
  re__prog_init(&prog);
  ASSERT_ERR_NOMEM(re__prog_add(&prog, a), error);
  b = re__prog_get(&prog, 0);
  ASSERT(re__prog_inst_equals(&a, b));
  ASSERT_EQ(re__prog_size(&prog), 1);
error:
  re__prog_destroy(&prog);
  PASS();
}

TEST(t_prog_add_limit)
{
  re__prog prog;
  re__prog_inst a;
  re__prog_loc i = 0;
  re__prog_inst_init_fail(&a);
  re__prog_init(&prog);
  while (1) {
    if (i == RE__PROG_SIZE_MAX) {
      ASSERT_EQ(re__prog_add(&prog, a), RE__ERROR_PROGMAX);
      break;
    } else {
      ASSERT_ERR_NOMEM(re__prog_add(&prog, a), error);
    }
    i++;
  }
error:
  re__prog_destroy(&prog);
  PASS();
}

TEST(t_prog_setget_entry)
{
  re__prog_entry idx;
  re__prog prog;
  re__prog_init(&prog);
  for (idx = 0; idx < RE__PROG_ENTRY_MAX; idx++) {
    re__prog_set_entry(&prog, idx, 5);
    ASSERT_EQ(re__prog_get_entry(&prog, idx), 5);
  }
  re__prog_destroy(&prog);
  PASS();
}

SUITE(s_prog)
{
  RUN_SUITE(s_prog_inst);
  RUN_TEST(t_prog_init);
  RUN_TEST(t_prog_size);
  RUN_TEST(t_prog_get);
  RUN_TEST(t_prog_get_const);
  RUN_TEST(t_prog_add);
  RUN_TEST(t_prog_add_limit);
  RUN_TEST(t_prog_setget_entry);
}

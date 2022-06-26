#include "test_compile_charclass.h"

int re__compile_charclass_to_sym_r(
    sym_build* parent, re__compile_charclass* cc, mn_uint32 tree_ref)
{
  sym_build build;
  while (tree_ref != RE__COMPILE_CHARCLASS_TREE_NONE) {
    const re__compile_charclass_tree* tree =
        re__compile_charclass_tree_get(cc, tree_ref);
    SYM_PUT_EXPR(parent, &build);
    SYM_PUT_NUM(&build, tree->byte_range.min);
    SYM_PUT_NUM(&build, tree->byte_range.max);
    re__compile_charclass_to_sym_r(&build, cc, tree->child_ref);
    tree_ref = tree->sibling_ref;
  }
  return SYM_OK;
}

/* Currently, we aren't keeping track of the sparse and dense arrays in our
 * *sym* testing. */
int re__compile_charclass_to_sym(sym_build* parent, re__compile_charclass cc)
{
  sym_build build;
  SYM_PUT_EXPR(parent, &build);
  SYM_PUT_TYPE(&build, "compile_charclass");
  return re__compile_charclass_to_sym_r(&build, &cc, cc.root_last_child_ref);
}

int re__compile_charclass_from_sym_r(
    sym_walk* parent, re__compile_charclass* cc, mn_uint32 root_ref)
{
  if (SYM_PEEK_EXPR(parent)) {
    sym_walk walk;
    mn_int32 min, max;
    re__byte_range br;
    mn_uint32 this_ref;
    int err;
    SYM_GET_EXPR(parent, &walk);
    SYM_GET_NUM(&walk, &min);
    SYM_GET_NUM(&walk, &max);
    br.min = (mn_uint8)min;
    br.max = (mn_uint8)max;
    re__compile_charclass_new_node(cc, root_ref, br, &this_ref, 0);
    while (SYM_MORE(&walk)) {
      err = re__compile_charclass_from_sym_r(&walk, cc, this_ref);
      if (err) {
        return err;
      }
    }
  }
  return SYM_OK;
}

int re__compile_charclass_from_sym(sym_walk* parent, re__compile_charclass* cc)
{
  sym_walk walk;
  SYM_GET_EXPR(parent, &walk);
  SYM_CHECK_TYPE(&walk, "compile_charclass");
  return re__compile_charclass_from_sym_r(
      &walk, cc, RE__COMPILE_CHARCLASS_TREE_NONE);
}

TEST(t_compile_charclass_init)
{
  re__compile_charclass cc;
  re__compile_charclass_init(&cc);
  re__compile_charclass_destroy(&cc);
  PASS();
}
#if 0
TEST(t_compile_charclass_gen)
{
  re__prog prog;
  re__compile_patches patches;
  re__compile_charclass cc;
  re__prog_inst fail_inst;
  re__charclass charclass;
  re__rune_range full_range;
  re_error err = RE_ERROR_NONE;
  full_range.min = 0;
  full_range.max = 0x10FFFF;
  re__prog_init(&prog);
  re__prog_inst_init_fail(&fail_inst);
  re__compile_patches_init(&patches);
  re__charclass_init(&charclass);
  re__compile_charclass_init(&cc);
  if ((err = re__prog_add(&prog, fail_inst)) == RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  if ((err = re__charclass_push(&charclass, full_range)) == RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  if ((err = re__compile_charclass_gen(&cc, &charclass, &prog, &patches, 0)) ==
      RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
error:
  re__compile_charclass_destroy(&cc);
  re__charclass_destroy(&charclass);
  re__prog_destroy(&prog);
  PASS();
}
/*
TEST(t_compile_charclass_split_ranges) {
    re__compile_charclass cc;
    re__rune_range r;
    r.min = 0;
    r.max = 0x10FFFF;
    re__compile_charclass_init(&cc);
    re__compile_charclass_split_rune_range(&cc,r);
    re__compile_charclass_dump(&cc, cc.root_last_child_ref, 0);
    ASSERT_SYMEQ(re__compile_charclass, cc,
        "(compile_charclass"
        "  (0xF4 0xF4"
        "    (0x80 0x8F"
        "      (0x80 0xBF"
        "        (0x80 0xBF");
    re__compile_charclass_destroy(&cc);
    PASS();
}*/
#endif

SUITE(s_compile_charclass)
{
  RUN_TEST(t_compile_charclass_init);
  /*RUN_TEST(t_compile_charclass_gen);*/
  /*RUN_TEST(t_compile_charclass_split_ranges);*/
}

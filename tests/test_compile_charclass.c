#include "test_compile_charclass.h"

#include "test_charclass.h"
#include "test_helpers.h"
#include "test_prog.h"
#include "test_range.h"

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

int re__compile_charclass_test_get_ascii(
    re__charclass* charclass, re__charclass_ascii_type atype, int inverted)
{
  re__rune_data rune_data;
  re__charclass_builder builder;
  re__rune_data_init(&rune_data);
  re__charclass_builder_init(&builder, &rune_data);
  re__charclass_builder_begin(&builder);
  ASSERT_ERR_NOMEM(
      re__charclass_builder_insert_ascii_class(&builder, atype, inverted),
      error);
  ASSERT_ERR_NOMEM(re__charclass_builder_finish(&builder, charclass), error);
error:
  re__charclass_builder_destroy(&builder);
  re__rune_data_destroy(&rune_data);
  PASS();
}

int re__compile_charclass_test(const char* charclass_sym, const char* prog_sym)
{
  re__compile_charclass compile;
  re__charclass charclass;
  re__prog prog;
  re__prog_inst fail_inst;
  re__compile_patches patches;
  SYM(re__charclass, charclass_sym, &charclass);
  re__prog_init(&prog);
  re__compile_patches_init(&patches);
  re__compile_charclass_init(&compile);
  re__prog_inst_init_fail(&fail_inst);
  ASSERT_ERR_NOMEM(re__prog_add(&prog, fail_inst), error);
  ASSERT_ERR_NOMEM(
      re__compile_charclass_gen(&compile, &charclass, &prog, &patches, 0),
      error);
  re__compile_patches_patch(&patches, &prog, 0);
  ASSERT_SYMEQ(re__prog, prog, prog_sym);
error:
  re__prog_destroy(&prog);
  re__compile_charclass_destroy(&compile);
  re__charclass_destroy(&charclass);
  PASS();
}

int re__compile_charclass_test_reverse(
    const char* charclass_sym, const char* prog_sym)
{
  re__compile_charclass compile;
  re__charclass charclass;
  re__prog prog;
  re__prog_inst fail_inst;
  re__compile_patches patches;
  SYM(re__charclass, charclass_sym, &charclass);
  re__prog_init(&prog);
  re__compile_patches_init(&patches);
  re__compile_charclass_init(&compile);
  re__prog_inst_init_fail(&fail_inst);
  ASSERT_ERR_NOMEM(re__prog_add(&prog, fail_inst), error);
  ASSERT_ERR_NOMEM(
      re__compile_charclass_gen(&compile, &charclass, &prog, &patches, 1),
      error);
  re__compile_patches_patch(&patches, &prog, 0);
  ASSERT_SYMEQ(re__prog, prog, prog_sym);
error:
  re__prog_destroy(&prog);
  re__compile_charclass_destroy(&compile);
  re__charclass_destroy(&charclass);
  PASS();
}

TEST(t_compile_charclass_init)
{
  re__compile_charclass cc;
  re__compile_charclass_init(&cc);
  re__compile_charclass_destroy(&cc);
  PASS();
}

TEST(t_compile_charclass_ascii_rune)
{
  PROPAGATE(re__compile_charclass_test(
      "(charclass ("
      "  (rune_range 0 0)))",
      "(prog"
      "  (fail)"
      "  (byte 0 0))"));
  PROPAGATE(re__compile_charclass_test(
      "(charclass ("
      "  (rune_range 'a' 'a')))",
      "(prog"
      "  (fail)"
      "  (byte 'a' 0))"));
  PROPAGATE(re__compile_charclass_test(
      "(charclass ("
      "  (rune_range 127 127)))",
      "(prog"
      "  (fail)"
      "  (byte 127 0))"));
  PASS();
}

TEST(t_compile_charclass_ascii_runes)
{
  PROPAGATE(re__compile_charclass_test(
      "(charclass ("
      "  (rune_range 0 5)))",
      "(prog"
      "  (fail)"
      "  (byte_range 0 5 0))"));
  PROPAGATE(re__compile_charclass_test(
      "(charclass ("
      "  (rune_range 'a' 'z')))",
      "(prog"
      "  (fail)"
      "  (byte_range 'a' 'z' 0))"));
  PASS();
}

TEST(t_compile_charclass_unicode_rune_2)
{
  PROPAGATE(re__compile_charclass_test(
      "(charclass ("
      "  (rune_range 128 128)))",
      "(prog"
      "  (fail)"
      "  (byte 0xC2 2)"
      "  (byte 0x80 0))"));
  PROPAGATE(re__compile_charclass_test(
      "(charclass ("
      "  (rune_range 0x341 0x341)))",
      "(prog"
      "  (fail)"
      "  (byte 0xCD 2)"
      "  (byte 0x81 0))"));
  PROPAGATE(re__compile_charclass_test(
      "(charclass ("
      "  (rune_range 0x7FF 0x7FF)))",
      "(prog"
      "  (fail)"
      "  (byte 0xDF 2)"
      "  (byte 0xBF 0))"));
  PASS();
}

TEST(t_compile_charclass_unicode_rune_3)
{
  PROPAGATE(re__compile_charclass_test(
      "(charclass ("
      "  (rune_range 0x800 0x800)))",
      "(prog"
      "  (fail)"
      "  (byte 0xE0 2)"
      "  (byte 0xA0 3)"
      "  (byte 0x80 0))"));
  PROPAGATE(re__compile_charclass_test(
      "(charclass ("
      "  (rune_range 0x5678 0x5678)))",
      "(prog"
      "  (fail)"
      "  (byte 0xE5 2)"
      "  (byte 0x99 3)"
      "  (byte 0xB8 0))"));
  PROPAGATE(re__compile_charclass_test(
      "(charclass ("
      "  (rune_range 0xFFFF 0xFFFF)))",
      "(prog"
      "  (fail)"
      "  (byte 0xEF 2)"
      "  (byte 0xBF 3)"
      "  (byte 0xBF 0))"));
  PASS();
}

TEST(t_compile_charclass_unicode_rune_4)
{
  PROPAGATE(re__compile_charclass_test(
      "(charclass ("
      "  (rune_range 0x10000 0x10000)))",
      "(prog"
      "  (fail)"
      "  (byte 0xF0 2)"
      "  (byte 0x90 3)"
      "  (byte 0x80 4)"
      "  (byte 0x80 0))"));
  PROPAGATE(re__compile_charclass_test(
      "(charclass ("
      "  (rune_range 0x78901 0x78901)))",
      "(prog"
      "  (fail)"
      "  (byte 0xF1 2)"
      "  (byte 0xB8 3)"
      "  (byte 0xA4 4)"
      "  (byte 0x81 0))"));
  PROPAGATE(re__compile_charclass_test(
      "(charclass ("
      "  (rune_range 0x10FFFF 0x10FFFF)))",
      "(prog"
      "  (fail)"
      "  (byte 0xF4 2)"
      "  (byte 0x8F 3)"
      "  (byte 0xBF 4)"
      "  (byte 0xBF 0))"));
  PASS();
}

TEST(t_compile_charclass_unicode_range_mixed_size)
{
  PROPAGATE(re__compile_charclass_test(
      "(charclass ("
      "  (rune_range 0 0x7FF)))",
      "(prog"
      "  (fail)"
      "  (split 2 3)"
      "  (byte_range 0 0x7F 0)"
      "  (byte_range 0xC2 0xDF 4)"
      "  (byte_range 0x80 0xBF 0))"));
  PROPAGATE(re__compile_charclass_test(
      "(charclass ("
      "  (rune_range 129 0x8FF)))",
      "(prog"
      "  (fail)"
      "  (split 2 4)"
      "  (byte 0xC2 3)"
      "  (byte_range 0x81 0xBF 0)"
      "  (split 5 7)"
      "  (byte_range 0xC3 0xDF 6)"
      "  (byte_range 0x80 0xBF 0)"
      "  (byte 0xE0 8)"
      "  (byte_range 0xA0 0xA3 6))"));
  PASS();
}

TEST(t_compile_charclass_range_full_page)
{
  PROPAGATE(re__compile_charclass_test(
      "(charclass ("
      "  (rune_range 0x80 0xBF)))",
      "(prog"
      "  (fail)"
      "  (byte 0xC2 2)"
      "  (byte_range 0x80 0xBF 0))"));
  PASS();
}

TEST(t_compile_charclass_range_incomp_yx)
{
  PROPAGATE(re__compile_charclass_test(
      "(charclass ("
      "  (rune_range 0x85 0xF0)))",
      "(prog"
      "  (fail)"
      "  (split 2 4)"
      "  (byte 0xC2 3)"
      "  (byte_range 0x85 0xBF 0)"
      "  (byte 0xC3 5)"
      "  (byte_range 0x80 0xB0 0))"));
  PROPAGATE(re__compile_charclass_test(
      "(charclass ("
      "  (rune_range 0x80 0xF0)))",
      "(prog"
      "  (fail)"
      "  (split 2 4)"
      "  (byte 0xC2 3)"
      "  (byte_range 0x80 0xBF 0)"
      "  (byte 0xC3 5)"
      "  (byte_range 0x80 0xB0 0))"));
  PASS();
}

TEST(t_compile_charclass_range_three_splits)
{
  PROPAGATE(re__compile_charclass_test(
      "(charclass ("
      "  (rune_range 0x85 0x130)))",
      "(prog"
      "  (fail)"
      "  (split 2 4)"
      "  (byte 0xC2 3)"
      "  (byte_range 0x85 0xBF 0)"
      "  (split 5 7)"
      "  (byte 0xC3 6)"
      "  (byte_range 0x80 0xBF 0)"
      "  (byte 0xC4 8)"
      "  (byte_range 0x80 0xB0 0))"));
  PASS();
}

TEST(t_compile_charclass_range_overlap_y_startend)
{
  PROPAGATE(re__compile_charclass_test(
      "(charclass ("
      "  (rune_range 0x85 0xA0)"
      "  (rune_range 0xA5 0xBF)))",
      "(prog"
      "  (fail)"
      "  (byte 0xC2 2)"
      "  (split 3 4)"
      "  (byte_range 0x85 0xA0 0)"
      "  (byte_range 0xA5 0xBF 0))"));
  PROPAGATE(re__compile_charclass_test(
      "(charclass ("
      "  (rune_range 0x90556 0x90565)"
      "  (rune_range 0x90570 0x9057F)))",
      "(prog"
      "  (fail)"
      "  (byte 0xF2 2)"
      "  (byte 0x90 3)"
      "  (byte 0x95 4)"
      "  (split 5 6)"
      "  (byte_range 0x96 0xA5 0)"
      "  (byte_range 0xB0 0xBF 0))"));
  PASS();
}

TEST(t_compile_charclass_range_overlap_y_start)
{
  PROPAGATE(re__compile_charclass_test(
      "(charclass ("
      "  (rune_range 0x85 0xA0)"
      "  (rune_range 0xA5 0xC7)))",
      "(prog"
      "  (fail)"
      "  (split 2 6)"
      "  (byte 0xC2 3)"
      "  (split 4 5)"
      "  (byte_range 0x85 0xA0 0)"
      "  (byte_range 0xA5 0xBF 0)"
      "  (byte 0xC3 7)"
      "  (byte_range 0x80 0x87 0))"));
  PASS();
}

TEST(t_compile_charclass_ranges_sameend)
{
  PROPAGATE(re__compile_charclass_test(
      "(charclass ("
      "  (rune_range 0x80 0xBF)"
      "  (rune_range 0x100 0x13F)))",
      "(prog"
      "  (fail)"
      "  (split 2 4)"
      "  (byte 0xC2 3)"
      "  (byte_range 0x80 0xBF 0)"
      "  (byte 0xC4 3))"));
  PASS();
}

TEST(t_compile_charclass_ranges_merge)
{
  PROPAGATE(re__compile_charclass_test(
      "(charclass ("
      "  (rune_range 0x80 0x85)"
      "  (rune_range 0xC0 0xC5)))",
      "(prog"
      "  (fail)"
      "  (byte_range 0xC2 0xC3 2)"
      "  (byte_range 0x80 0x85 0))"));
  PROPAGATE(re__compile_charclass_test(
      "(charclass ("
      "  (rune_range 0x800 0x830)"
      "  (rune_range 0x840 0x870)))",
      "(prog"
      "  (fail)"
      "  (byte 0xE0 2)"
      "  (byte_range 0xA0 0xA1 3)"
      "  (byte_range 0x80 0xB0 0))"));
  PASS();
}

TEST(t_compile_charclass_collision_nomerge_0)
{
  /* This range is known to induce a collision with a hash weakened to 16b */
  /* This forces inequality in the case where trees have their first subtree
   * compare unequal */
  PROPAGATE(re__compile_charclass_test(
      "(charclass ("
      "  (rune_range 0xECA4 0xF394)))",
      "(prog"
      "  (fail)"
      "  (split 2 8)"
      "  (byte 238 3)"
      "  (split 4 6)"
      "  (byte 178 5)"
      "  (byte_range 164 191 0)"
      "  (byte_range 179 191 7)"
      "  (byte_range 128 191 0)"
      "  (byte 239 9)"
      "  (split 10 11)"
      "  (byte_range 128 141 7)"
      "  (byte 142 12)"
      "  (byte_range 128 148 0))"));
  PASS();
}

TEST(t_compile_charclass_collision_nomerge_1)
{
  /* This range is known to induce a collision with a hash weakened to 16b */
  /* This forces inequality in the case where trees have their second or third
   * subtree compare unequal */
  PROPAGATE(re__compile_charclass_test(
      "(charclass ("
      "  (rune_range 0x5CB0 0xBC056)))",
      "(prog"
      "  (fail)"
      "  (split 2 8)"
      "  (byte 229 3)"
      "  (split 4 6)"
      "  (byte 178 5)"
      "  (byte_range 176 191 0)"
      "  (byte_range 179 191 7)"
      "  (byte_range 128 191 0)"
      "  (split 9 11)"
      "  (byte_range 230 239 10)"
      "  (byte_range 128 191 7)"
      "  (split 12 14)"
      "  (byte 240 13)"
      "  (byte_range 144 191 10)"
      "  (split 15 17)"
      "  (byte 241 16)"
      "  (byte_range 128 191 10)"
      "  (byte 242 18)"
      "  (split 19 20)"
      "  (byte_range 128 187 10)"
      "  (byte 188 21)"
      "  (split 22 23)"
      "  (byte 128 7)"
      "  (byte 129 24)"
      "  (byte_range 128 150 0))"));
  PASS();
}

TEST(t_compile_charclass_range_no_cached_0)
{
  /* Force a cache miss due to hash entry bucket exhaustion. */
  PROPAGATE(re__compile_charclass_test(
      "(charclass ("
      "  (rune_range 0x7B 0xCB)))",
      "(prog"
      "  (fail)"
      "  (split 2 3)"
      "  (byte_range 0x7B 0x7F 0)"
      "  (split 4 6)"
      "  (byte 0xC2 5)"
      "  (byte_range 0x80 0xBF 0)"
      "  (byte 0xC3 7)"
      "  (byte_range 0x80 0x8B 0))"));
  PASS();
}

TEST(t_compile_charclass_range_no_cached_1)
{
  /* Force a cache miss due to hash entry bucket exhaustion, but not on the
   * first iteration. */
  PROPAGATE(re__compile_charclass_test(
      "(charclass ("
      "  (rune_range 0xCF7A 0xCFB8)"
      "  (rune_range 0xCFBD 0xD001)))",
      "(prog"
      "  (fail)"
      "  (split 2 13)"
      "  (byte 236 3)"
      "  (split 4 6)"
      "  (byte 189 5)"
      "  (byte_range 186 191 0)"
      "  (split 7 11)"
      "  (byte 190 8)"
      "  (split 9 10)"
      "  (byte_range 128 184 0)"
      "  (byte_range 189 191 0)"
      "  (byte 191 12)"
      "  (byte_range 128 191 0)"
      "  (byte 237 14)"
      "  (byte 128 15)"
      "  (byte_range 128 129 0))"));
  PASS();
}

TEST(t_compile_charclass_range_no_cached_2)
{
  /* Force a cache miss due to the sparse and dense pointers mismatching. */
  PROPAGATE(re__compile_charclass_test(
      "(charclass ("
      "  (rune_range 0xE26 0xE40)"
      "  (rune_range 0xE43 0xE83)))",
      "(prog"
      "  (fail)"
      "  (byte 224 2)"
      "  (split 3 5)"
      "  (byte 184 4)"
      "  (byte_range 166 191 0)"
      "  (split 6 10)"
      "  (byte 185 7)"
      "  (split 8 9)"
      "  (byte 128 0)"
      "  (byte_range 131 191 0)"
      "  (byte 186 11)"
      "  (byte_range 128 131 0))"));
  PASS();
}

TEST(t_compile_charclass_range_cache_split_from)
{
  /* Force a cache hit due to the split_from of secondary being found. */
  PROPAGATE(re__compile_charclass_test(
      "(charclass ("
      "  (rune_range 0x3238 0x3276)"
      "  (rune_range 0x3278 0x32AC)))",
      "(prog"
      "  (fail)"
      "  (byte 227 2)"
      "  (split 3 5)"
      "  (byte 136 4)"
      "  (byte_range 184 191 0)"
      "  (split 6 9)"
      "  (byte 137 7)"
      "  (split 8 4)"
      "  (byte_range 128 182 0)"
      "  (byte 138 10)"
      "  (byte_range 128 172 0))"));
  PASS();
}

TEST(t_compile_charclass_reverse_simple)
{
  PROPAGATE(re__compile_charclass_test_reverse(
      "(charclass ("
      "  (rune_range 0x70 0x7F)))",
      "(prog"
      "  (fail)"
      "  (byte_range 0x70 0x7F 0))"));
  PASS();
}

TEST(t_compile_charclass_reverse_nested)
{
  /* Force a cache hit due to the split_from of secondary being found. */
  PROPAGATE(re__compile_charclass_test_reverse(
      "(charclass ("
      "  (rune_range 0 0x10FFFF)))",
      "(prog"
      "  (fail)"
      "  (split 2 5)"
      "  (byte_range 128 191 3)"
      "  (byte_range 128 131 4)"
      "  (byte 225 0)"
      "  (split 6 9)"
      "  (byte_range 128 191 7)"
      "  (byte_range 165 191 8)"
      "  (byte 224 0)"
      "  (byte_range 134 191 10)"
      "  (byte 164 8))"));
  PASS();
}

#if !MN__COVERAGE && 0

#include <stdio.h>

TEST(t_compile_charclass_make_collisions)
{
  mn_size niters = 5000000000;
  mn_size i;
  mn_size j;
  re__rune_data rune_data;
  re__charclass_builder builder;
  re__compile_charclass char_comp;
  re__charclass cc;
  re__prog prog;
  re__prog_inst inst;
  re__compile_patches patches;
  re__rune_data_init(&rune_data);
  re__charclass_builder_init(&builder, &rune_data);
  re__compile_charclass_init(&char_comp);
  for (i = 0; i < niters; i++) {
    mn_size njters = RAND_PARAM(2) + 1;
    re_rune min = re__rune_rand_above(0);
    re__prog_init(&prog);
    re__prog_inst_init_fail(&inst);
    ASSERT_ERR_NOMEM(re__prog_add(&prog, inst), destroy_prog);
    re__compile_patches_init(&patches);
    re__charclass_builder_begin(&builder);
    for (j = 0; j < njters; j++) {
      re__rune_range rr;
      if (min > RE_RUNE_MAX - 0x60) {
        break;
      }
      min = min + (re_rune)RAND_PARAM(5) + 1;
      rr.min = min;
      min = min + (re_rune)RAND_PARAM(0x50) + 1;
      rr.max = min;
      ASSERT_ERR_NOMEM(
          re__charclass_builder_insert_range(&builder, rr), destroy_prog);
    }
    ASSERT_ERR_NOMEM(re__charclass_builder_finish(&builder, &cc), destroy_prog);
    ASSERT_ERR_NOMEM(
        re__compile_charclass_gen(&char_comp, &cc, &prog, &patches, 0),
        destroy_charclass);
    re__charclass_destroy(&cc);
    re__prog_destroy(&prog);
    if (i % 100000 == 0) {
      printf("%u...\n", (unsigned int)i);
    }
  }
  goto error;
destroy_charclass:
  re__charclass_destroy(&cc);
destroy_prog:
  re__prog_destroy(&prog);
error:
  re__compile_charclass_init(&char_comp);
  re__charclass_builder_destroy(&builder);
  re__rune_data_destroy(&rune_data);
  PASS();
}

#else

TEST(t_compile_charclass_make_collisions) { PASS(); }

#endif

SUITE(s_compile_charclass)
{
  RUN_TEST(t_compile_charclass_init);
  RUN_TEST(t_compile_charclass_ascii_rune);
  RUN_TEST(t_compile_charclass_ascii_runes);
  RUN_TEST(t_compile_charclass_unicode_rune_2);
  RUN_TEST(t_compile_charclass_unicode_rune_3);
  RUN_TEST(t_compile_charclass_unicode_rune_4);
  RUN_TEST(t_compile_charclass_unicode_range_mixed_size);
  RUN_TEST(t_compile_charclass_range_full_page);
  RUN_TEST(t_compile_charclass_range_incomp_yx);
  RUN_TEST(t_compile_charclass_range_three_splits);
  RUN_TEST(t_compile_charclass_range_overlap_y_startend);
  RUN_TEST(t_compile_charclass_range_overlap_y_start);
  RUN_TEST(t_compile_charclass_ranges_sameend);
  RUN_TEST(t_compile_charclass_ranges_merge);
  RUN_TEST(t_compile_charclass_collision_nomerge_0);
  RUN_TEST(t_compile_charclass_collision_nomerge_1);
  RUN_TEST(t_compile_charclass_range_no_cached_0);
  RUN_TEST(t_compile_charclass_range_no_cached_1);
  RUN_TEST(t_compile_charclass_range_no_cached_2);
  RUN_TEST(t_compile_charclass_range_cache_split_from);
  RUN_TEST(t_compile_charclass_reverse_simple);
  RUN_TEST(t_compile_charclass_reverse_nested);

  RUN_TEST(t_compile_charclass_make_collisions);
}

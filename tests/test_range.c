#include "test_range.h"

int re__byte_range_to_sym(mptest_sym_build* parent, re__byte_range br)
{
  mptest_sym_build build;
  SYM_PUT_EXPR(parent, &build);
  SYM_PUT_TYPE(&build, "byte_range");
  SYM_PUT_NUM(&build, br.min);
  SYM_PUT_NUM(&build, br.max);
  return SYM_OK;
}

int re__byte_range_from_sym(mptest_sym_walk* parent, re__byte_range* br)
{
  mn_int32 min;
  mn_int32 max;
  mptest_sym_walk walk;
  SYM_GET_EXPR(parent, &walk);
  SYM_CHECK_TYPE(&walk, "byte_range");
  SYM_GET_NUM(&walk, &min);
  SYM_GET_NUM(&walk, &max);
  br->min = (mn_uint8)min;
  br->max = (mn_uint8)max;
  return SYM_OK;
}

re__byte_range re__byte_range_random(void)
{
  re__byte_range br;
  br.min = (mn_uint8)RAND_PARAM(0xFF);
  br.max = (mn_uint8)RAND_PARAM((0xFF - br.min)) + br.min;
  return br;
}

TEST(t_byte_range_equals)
{
  re__byte_range a, b;

  SYM(re__byte_range, "(byte_range 0 5)", &a);
  SYM(re__byte_range, "(byte_range 0 5)", &b);
  ASSERT(re__byte_range_equals(a, b));

  SYM(re__byte_range, "(byte_range 10 15)", &a);
  SYM(re__byte_range, "(byte_range 10 15)", &b);
  ASSERT(re__byte_range_equals(a, b));

  SYM(re__byte_range, "(byte_range 10 15)", &a);
  SYM(re__byte_range, "(byte_range 10 11)", &b);
  ASSERT(!re__byte_range_equals(a, b));

  SYM(re__byte_range, "(byte_range 5 10)", &a);
  SYM(re__byte_range, "(byte_range 10 15)", &b);
  ASSERT(!re__byte_range_equals(a, b));

  PASS();
}

TEST(t_byte_range_adjacent)
{
  re__byte_range a, b;

  SYM(re__byte_range, "(byte_range 0 5)", &a);
  SYM(re__byte_range, "(byte_range 6 10)", &b);
  ASSERT(re__byte_range_adjacent(a, b));

  SYM(re__byte_range, "(byte_range 0 5)", &a);
  SYM(re__byte_range, "(byte_range 8 10)", &b);
  ASSERT(!re__byte_range_adjacent(a, b));

  SYM(re__byte_range, "(byte_range 250 255)", &a);
  SYM(re__byte_range, "(byte_range 0 5)", &b);
  ASSERT(!re__byte_range_adjacent(a, b));

  PASS();
}

TEST(t_byte_range_merge)
{
  re__byte_range a, b, out;

  SYM(re__byte_range, "(byte_range 0 5)", &a);
  SYM(re__byte_range, "(byte_range 6 10)", &b);
  out = re__byte_range_merge(a, b);
  ASSERT_SYMEQ(re__byte_range, out, "(byte_range 0 10)");

  SYM(re__byte_range, "(byte_range 20 23)", &a);
  SYM(re__byte_range, "(byte_range 24 26)", &b);
  out = re__byte_range_merge(a, b);
  ASSERT_SYMEQ(re__byte_range, out, "(byte_range 20 26)");

  PASS();
}

TEST(t_byte_range_intersects)
{
  re__byte_range a, b;
  SYM(re__byte_range, "(byte_range 0 2)", &a);
  SYM(re__byte_range, "(byte_range 0 2)", &b);
  ASSERTm(re__byte_range_intersects(a, b), "equal ranges must intersect");

  SYM(re__byte_range, "(byte_range 0 2)", &a);
  SYM(re__byte_range, "(byte_range 1 2)", &b);
  ASSERTm(re__byte_range_intersects(a, b), "subset range must intersect");

  SYM(re__byte_range, "(byte_range 1 2)", &a);
  SYM(re__byte_range, "(byte_range 0 2)", &b);
  ASSERTm(re__byte_range_intersects(a, b), "subset range must intersect");

  SYM(re__byte_range, "(byte_range 0 2)", &a);
  SYM(re__byte_range, "(byte_range 2 3)", &b);
  ASSERTm(re__byte_range_intersects(a, b), "overlapping range must intersect");

  SYM(re__byte_range, "(byte_range 2 3)", &a);
  SYM(re__byte_range, "(byte_range 0 2)", &b);
  ASSERTm(re__byte_range_intersects(a, b), "overlapping range must intersect");

  SYM(re__byte_range, "(byte_range 0 2)", &a);
  SYM(re__byte_range, "(byte_range 3 4)", &b);
  ASSERTm(
      !re__byte_range_intersects(a, b),
      "unoverlapping ranges must not intersect");

  SYM(re__byte_range, "(byte_range 3 4)", &a);
  SYM(re__byte_range, "(byte_range 0 2)", &b);
  ASSERTm(
      !re__byte_range_intersects(a, b),
      "unoverlapping ranges must not intersect");

  SYM(re__byte_range, "(byte_range 0 255)", &a);
  SYM(re__byte_range, "(byte_range 0 255)", &b);
  ASSERTm(re__byte_range_intersects(a, b), "full ranges must intersect");

  SYM(re__byte_range, "(byte_range 0 127)", &a);
  SYM(re__byte_range, "(byte_range 128 255)", &b);
  ASSERTm(
      !re__byte_range_intersects(a, b),
      "perfectly split ranges must not intersect");

  PASS();
}

TEST(t_byte_range_intersection)
{
  re__byte_range a, b, out;

  SYM(re__byte_range, "(byte_range 0 5)", &a);
  SYM(re__byte_range, "(byte_range 5 6)", &b);
  out = re__byte_range_intersection(a, b);
  ASSERT_SYMEQ(re__byte_range, out, "(byte_range 5 5)");

  SYM(re__byte_range, "(byte_range 10 15)", &a);
  SYM(re__byte_range, "(byte_range 0 12)", &b);
  out = re__byte_range_intersection(a, b);
  ASSERT_SYMEQ(re__byte_range, out, "(byte_range 10 12)");

  PASS();
}

SUITE(s_range_byte_range)
{
  RUN_TEST(t_byte_range_equals);
  RUN_TEST(t_byte_range_adjacent);
  RUN_TEST(t_byte_range_merge);
  RUN_TEST(t_byte_range_intersects);
  RUN_TEST(t_byte_range_intersection);
}

int re__rune_range_to_sym(mptest_sym_build* parent, re__rune_range rr)
{
  mptest_sym_build build;
  SYM_PUT_EXPR(parent, &build);
  SYM_PUT_TYPE(&build, "rune_range");
  SYM_PUT_NUM(&build, rr.min);
  SYM_PUT_NUM(&build, rr.max);
  return SYM_OK;
}

int re__rune_range_from_sym(mptest_sym_walk* parent, re__rune_range* rr)
{
  mptest_sym_walk walk;
  SYM_GET_EXPR(parent, &walk);
  SYM_CHECK_TYPE(&walk, "rune_range");
  SYM_GET_NUM(&walk, &rr->min);
  SYM_GET_NUM(&walk, &rr->max);
  return SYM_OK;
}

re__rune_range re__rune_range_random(void)
{
  int class = RAND_PARAM(4);
  mptest_rand min;
  mptest_rand max;
  re__rune_range rr;
  if (class == 0) {
    min = 0;
    max = 0x7F;
  } else if (class == 1) {
    min = 0x80;
    max = 0x7FF;
  } else if (class == 2) {
    min = 0x800;
    max = 0xFFFF;
  } else {
    min = 0x10000;
    max = 0x10FFFF;
  }
  rr.min = (mn_int32)RAND_PARAM(max - min);
  rr.max = (mn_int32)RAND_PARAM((max - (mptest_rand)rr.min)) + rr.min;
  return rr;
}

TEST(t_rune_range_equals)
{
  re__rune_range a, b;

  SYM(re__rune_range, "(rune_range 50 55)", &a);
  SYM(re__rune_range, "(rune_range 50 55)", &b);
  ASSERT(re__rune_range_equals(a, b));

  SYM(re__rune_range, "(rune_range 50 52)", &a);
  SYM(re__rune_range, "(rune_range 50 55)", &b);
  ASSERT(!re__rune_range_equals(a, b));

  SYM(re__rune_range, "(rune_range 51 55)", &a);
  SYM(re__rune_range, "(rune_range 50 55)", &b);
  ASSERT(!re__rune_range_equals(a, b));

  SYM(re__rune_range, "(rune_range 0x10FFFE 0x10FFFF)", &a);
  SYM(re__rune_range, "(rune_range 0x10FFFE 0x10FFFF)", &b);
  ASSERT(re__rune_range_equals(a, b));

  PASS();
}

TEST(t_rune_range_intersects)
{
  re__rune_range a, b;
  SYM(re__rune_range, "(rune_range 0 2)", &a);
  SYM(re__rune_range, "(rune_range 0 2)", &b);
  ASSERTm(re__rune_range_intersects(a, b), "equal ranges must intersect");

  SYM(re__rune_range, "(rune_range 0 2)", &a);
  SYM(re__rune_range, "(rune_range 1 2)", &b);
  ASSERTm(re__rune_range_intersects(a, b), "subset range must intersect");

  SYM(re__rune_range, "(rune_range 1 2)", &a);
  SYM(re__rune_range, "(rune_range 0 2)", &b);
  ASSERTm(re__rune_range_intersects(a, b), "subset range must intersect");

  SYM(re__rune_range, "(rune_range 0 2)", &a);
  SYM(re__rune_range, "(rune_range 2 3)", &b);
  ASSERTm(re__rune_range_intersects(a, b), "overlapping range must intersect");

  SYM(re__rune_range, "(rune_range 2 3)", &a);
  SYM(re__rune_range, "(rune_range 0 2)", &b);
  ASSERTm(re__rune_range_intersects(a, b), "overlapping range must intersect");

  SYM(re__rune_range, "(rune_range 0 2)", &a);
  SYM(re__rune_range, "(rune_range 3 4)", &b);
  ASSERTm(
      !re__rune_range_intersects(a, b),
      "unoverlapping ranges must not intersect");

  SYM(re__rune_range, "(rune_range 3 4)", &a);
  SYM(re__rune_range, "(rune_range 0 2)", &b);
  ASSERTm(
      !re__rune_range_intersects(a, b),
      "unoverlapping ranges must not intersect");

  PASS();
}

TEST(t_rune_range_clamp)
{
  re__rune_range a, b, out;

  SYM(re__rune_range, "(rune_range 0 5)", &a);
  SYM(re__rune_range, "(rune_range 5 6)", &b);
  out = re__rune_range_clamp(a, b);
  ASSERT_SYMEQ(re__rune_range, out, "(rune_range 5 5)");

  SYM(re__rune_range, "(rune_range 0 10)", &a);
  SYM(re__rune_range, "(rune_range 5 6)", &b);
  out = re__rune_range_clamp(a, b);
  ASSERT_SYMEQ(re__rune_range, out, "(rune_range 5 6)");

  SYM(re__rune_range, "(rune_range 6 10)", &a);
  SYM(re__rune_range, "(rune_range 5 6)", &b);
  out = re__rune_range_clamp(a, b);
  ASSERT_SYMEQ(re__rune_range, out, "(rune_range 6 6)");

  SYM(re__rune_range, "(rune_range 5 6)", &a);
  SYM(re__rune_range, "(rune_range 5 6)", &b);
  out = re__rune_range_clamp(a, b);
  ASSERT_SYMEQ(re__rune_range, out, "(rune_range 5 6)");

  SYM(re__rune_range, "(rune_range 3 4)", &a);
  SYM(re__rune_range, "(rune_range 0 10)", &b);
  out = re__rune_range_clamp(a, b);
  ASSERT_SYMEQ(re__rune_range, out, "(rune_range 3 4)");

  PASS();
}

SUITE(s_range_rune_range)
{
  RUN_TEST(t_rune_range_equals);
  RUN_TEST(t_rune_range_intersects);
  RUN_TEST(t_rune_range_clamp);
}

SUITE(s_range)
{
  RUN_SUITE(s_range_byte_range);
  RUN_SUITE(s_range_rune_range);
}

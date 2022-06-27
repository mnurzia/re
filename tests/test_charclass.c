#include "test_charclass.h"
#include "test_range.h"

int re__charclass_to_sym(sym_build* parent, const re__charclass charclass)
{
  sym_build build;
  SYM_PUT_EXPR(parent, &build);
  SYM_PUT_TYPE(&build, "charclass");
  {
    sym_build range_list;
    mptest_size i;
    SYM_PUT_EXPR(&build, &range_list);
    for (i = 0; i < re__charclass_size(&charclass); i++) {
      re__rune_range rr = re__charclass_get_ranges(&charclass)[i];
      SYM_PUT_SUB(&range_list, re__rune_range, rr);
    }
  }
  return SYM_OK;
}

int re__charclass_from_sym(sym_walk* parent, re__charclass* charclass)
{
  sym_walk walk;
  SYM_GET_EXPR(parent, &walk);
  SYM_CHECK_TYPE(&walk, "charclass");
  return re__charclass_from_sym_ranges_only(&walk, charclass);
}

int re__charclass_from_sym_ranges_only(sym_walk* walk, re__charclass* charclass)
{
  re__charclass_builder builder;
  re__rune_data rune_data;
  sym_walk range_list;
  re_error err = RE_ERROR_NONE;
  re__rune_data_init(&rune_data);
  re__charclass_builder_init(&builder, &rune_data);
  SYM_GET_EXPR(walk, &range_list);
  while (SYM_MORE(&range_list)) {
    re__rune_range rr;
    SYM_GET_SUB(&range_list, re__rune_range, &rr);
    if ((err = re__charclass_builder_insert_range(&builder, rr))) {
      return err;
    }
  }
  if ((err = re__charclass_builder_finish(&builder, charclass))) {
    return err;
  }
  re__charclass_builder_destroy(&builder);
  re__rune_data_destroy(&rune_data);
  return err;
}

re_error re__charclass_test_from_class(
    re__charclass* charclass, re__charclass_ascii_type ascii_type, int inverted)
{
  re_error err = RE_ERROR_NONE;
  re__rune_data rune_data;
  re__charclass_builder builder;
  re__rune_data_init(&rune_data);
  re__charclass_builder_init(&builder, &rune_data);
  if ((err = re__charclass_builder_insert_ascii_class(
           &builder, ascii_type, inverted))) {
    goto error;
  }
  if ((err = re__charclass_builder_finish(&builder, charclass))) {
    goto error;
  }
error:
  re__charclass_builder_destroy(&builder);
  re__rune_data_destroy(&rune_data);
  return err;
}

re_error re__charclass_test_from_str(
    re__charclass* charclass, mn__str_view str_view, int inverted)
{
  re_error err = RE_ERROR_NONE;
  re__rune_data rune_data;
  re__charclass_builder builder;
  re__rune_data_init(&rune_data);
  re__charclass_builder_init(&builder, &rune_data);
  if ((err = re__charclass_builder_insert_ascii_class_by_str(
           &builder, str_view, inverted))) {
    goto error;
  }
  if ((err = re__charclass_builder_finish(&builder, charclass))) {
    goto error;
  }
error:
  re__charclass_builder_destroy(&builder);
  re__rune_data_destroy(&rune_data);
  return err;
}

int re__charclass_test_ascii(
    re__charclass_ascii_type ascii_type, const char* expected_sym, int inverted)
{
  re__charclass charclass;
  ASSERT_ERR_NOMEM(
      re__charclass_test_from_class(&charclass, ascii_type, inverted), pass);
  ASSERT_SYMEQ(re__charclass, charclass, expected_sym);
  re__charclass_destroy(&charclass);
pass:
  PASS();
}

TEST(t_charclass_builder_init_from_class_alnum)
{
  PROPAGATE(re__charclass_test_ascii(
      RE__CHARCLASS_ASCII_TYPE_ALNUM,
      "(charclass ("
      "  (rune_range 48 57)"
      "  (rune_range 65 90)"
      "  (rune_range 97 122)))",
      0));
  PROPAGATE(re__charclass_test_ascii(
      RE__CHARCLASS_ASCII_TYPE_ALNUM,
      "(charclass ("
      "  (rune_range 0 47)"
      "  (rune_range 58 64)"
      "  (rune_range 91 96)"
      "  (rune_range 123 0x10FFFF)))",
      1));
  PASS();
}

TEST(t_charclass_builder_init_from_class_alpha)
{
  PROPAGATE(re__charclass_test_ascii(
      RE__CHARCLASS_ASCII_TYPE_ALPHA,
      "(charclass ("
      "  (rune_range 65 90)"
      "  (rune_range 97 122)))",
      0));
  PROPAGATE(re__charclass_test_ascii(
      RE__CHARCLASS_ASCII_TYPE_ALPHA,
      "(charclass ("
      "  (rune_range 0 64)"
      "  (rune_range 91 96)"
      "  (rune_range 123 0x10FFFF)))",
      1));
  PASS();
}

TEST(t_charclass_builder_init_from_class_ascii)
{
  PROPAGATE(re__charclass_test_ascii(
      RE__CHARCLASS_ASCII_TYPE_ASCII,
      "(charclass ("
      "  (rune_range 0 127)))",
      0));
  PROPAGATE(re__charclass_test_ascii(
      RE__CHARCLASS_ASCII_TYPE_ASCII,
      "(charclass ("
      "  (rune_range 128 0x10FFFF)))",
      1));
  PASS();
}

TEST(t_charclass_builder_init_from_class_blank)
{
  PROPAGATE(re__charclass_test_ascii(
      RE__CHARCLASS_ASCII_TYPE_BLANK,
      "(charclass ("
      "  (rune_range '\t' '\t')"
      "  (rune_range ' ' ' ')))",
      0));
  PROPAGATE(re__charclass_test_ascii(
      RE__CHARCLASS_ASCII_TYPE_BLANK,
      "(charclass ("
      "  (rune_range 0 8)"
      "  (rune_range 10 31)"
      "  (rune_range 33 0x10FFFF)))",
      1));
  PASS();
}

TEST(t_charclass_builder_init_from_class_cntrl)
{
  PROPAGATE(re__charclass_test_ascii(
      RE__CHARCLASS_ASCII_TYPE_CNTRL,
      "(charclass ("
      "  (rune_range 0 31)"
      "  (rune_range 127 127)))",
      0));
  PROPAGATE(re__charclass_test_ascii(
      RE__CHARCLASS_ASCII_TYPE_CNTRL,
      "(charclass ("
      "  (rune_range 32 126)"
      "  (rune_range 128 0x10FFFF)))",
      1));
  PASS();
}

TEST(t_charclass_builder_init_from_class_digit)
{
  PROPAGATE(re__charclass_test_ascii(
      RE__CHARCLASS_ASCII_TYPE_DIGIT,
      "(charclass ("
      "  (rune_range '0' '9')))",
      0));
  PROPAGATE(re__charclass_test_ascii(
      RE__CHARCLASS_ASCII_TYPE_DIGIT,
      "(charclass ("
      "  (rune_range 0 47)"
      "  (rune_range 58 0x10FFFF)))",
      1));
  PASS();
}

TEST(t_charclass_builder_init_from_class_graph)
{
  PROPAGATE(re__charclass_test_ascii(
      RE__CHARCLASS_ASCII_TYPE_GRAPH,
      "(charclass ("
      "  (rune_range 33 126)))",
      0));
  PROPAGATE(re__charclass_test_ascii(
      RE__CHARCLASS_ASCII_TYPE_GRAPH,
      "(charclass ("
      "  (rune_range 0 32)"
      "  (rune_range 127 0x10FFFF)))",
      1));
  PASS();
}

TEST(t_charclass_builder_init_from_class_lower)
{
  PROPAGATE(re__charclass_test_ascii(
      RE__CHARCLASS_ASCII_TYPE_LOWER,
      "(charclass ("
      "  (rune_range 'a' 'z')))",
      0));
  PROPAGATE(re__charclass_test_ascii(
      RE__CHARCLASS_ASCII_TYPE_LOWER,
      "(charclass ("
      "   (rune_range 0 96)"
      "    (rune_range 123 0x10FFFF)))",
      1));
  PASS();
}

TEST(t_charclass_builder_init_from_class_perl_space)
{
  PROPAGATE(re__charclass_test_ascii(
      RE__CHARCLASS_ASCII_TYPE_PERL_SPACE,
      "(charclass ("
      "   (rune_range 9 10)"
      "    (rune_range 12 13)"
      "    (rune_range 32 32)))",
      0));
  PROPAGATE(re__charclass_test_ascii(
      RE__CHARCLASS_ASCII_TYPE_PERL_SPACE,
      "(charclass ("
      "   (rune_range 0 8)"
      "    (rune_range 11 11)"
      "    (rune_range 14 31)"
      "    (rune_range 33 0x10FFFF)))",
      1));
  PASS();
}

TEST(t_charclass_builder_init_from_class_print)
{
  PROPAGATE(re__charclass_test_ascii(
      RE__CHARCLASS_ASCII_TYPE_PRINT,
      "(charclass ("
      "   (rune_range ' ' 126)))",
      0));
  PROPAGATE(re__charclass_test_ascii(
      RE__CHARCLASS_ASCII_TYPE_PRINT,
      "(charclass ("
      "   (rune_range 0 31)"
      "    (rune_range 127 0x10FFFF)))",
      1));
  PASS();
}

TEST(t_charclass_builder_init_from_class_punct)
{
  PROPAGATE(re__charclass_test_ascii(
      RE__CHARCLASS_ASCII_TYPE_PUNCT,
      "(charclass ("
      "   (rune_range 0x21 0x2F)"
      "    (rune_range 0x3A 0x40)"
      "    (rune_range 0x5B 0x60)"
      "    (rune_range 0x7B 0x7E)))",
      0));
  PROPAGATE(re__charclass_test_ascii(
      RE__CHARCLASS_ASCII_TYPE_PUNCT,
      "(charclass ("
      "   (rune_range 0 0x20)"
      "    (rune_range 0x30 0x39)"
      "    (rune_range 0x41 0x5A)"
      "    (rune_range 0x61 0x7A)"
      "    (rune_range 0x7F 0x10FFFF)))",
      1));
  PASS();
}

TEST(t_charclass_builder_init_from_class_space)
{
  PROPAGATE(re__charclass_test_ascii(
      RE__CHARCLASS_ASCII_TYPE_SPACE,
      "(charclass ("
      "   (rune_range 0x09 0x0D)"
      "    (rune_range ' ' ' ')))",
      0));
  PROPAGATE(re__charclass_test_ascii(
      RE__CHARCLASS_ASCII_TYPE_SPACE,
      "(charclass ("
      "   (rune_range 0 8)"
      "    (rune_range 0xE 31)"
      "    (rune_range 33 0x10FFFF)))",
      1));
  PASS();
}

TEST(t_charclass_builder_init_from_class_upper)
{
  PROPAGATE(re__charclass_test_ascii(
      RE__CHARCLASS_ASCII_TYPE_UPPER,
      "(charclass ("
      "   (rune_range 'A' 'Z')))",
      0));
  PROPAGATE(re__charclass_test_ascii(
      RE__CHARCLASS_ASCII_TYPE_UPPER,
      "(charclass ("
      "   (rune_range 0 64)"
      "    (rune_range 91 0x10FFFF)))",
      1));
  PASS();
}

TEST(t_charclass_builder_init_from_class_word)
{
  PROPAGATE(re__charclass_test_ascii(
      RE__CHARCLASS_ASCII_TYPE_WORD,
      "(charclass ("
      "   (rune_range '0' '9')"
      "    (rune_range 'A' 'Z')"
      "    (rune_range '_' '_')"
      "    (rune_range 'a' 'z')))",
      0));
  PROPAGATE(re__charclass_test_ascii(
      RE__CHARCLASS_ASCII_TYPE_WORD,
      "(charclass ("
      "   (rune_range 0 47)"
      "    (rune_range 58 64)"
      "    (rune_range 91 94)"
      "    (rune_range 96 96)"
      "    (rune_range 123 0x10FFFF)))",
      1));
  PASS();
}

TEST(t_charclass_builder_init_from_class_xdigit)
{
  PROPAGATE(re__charclass_test_ascii(
      RE__CHARCLASS_ASCII_TYPE_XDIGIT,
      "(charclass ("
      "   (rune_range '0' '9')"
      "    (rune_range 'A' 'F')"
      "    (rune_range 'a' 'f')))",
      0));
  PROPAGATE(re__charclass_test_ascii(
      RE__CHARCLASS_ASCII_TYPE_XDIGIT,
      "(charclass ("
      "   (rune_range 0 47)"
      "    (rune_range 58 64)"
      "    (rune_range 71 96)"
      "    (rune_range 103 0x10FFFF)))",
      1));
  PASS();
}

TEST(t_charclass_builder_init_from_class_invalid)
{
  re__rune_data rune_data;
  re__charclass_builder builder;
  mn__str_view name;
  re__rune_data_init(&rune_data);
  re__charclass_builder_init(&builder, &rune_data);
  re__charclass_builder_begin(&builder);
  mn__str_view_init_s(&name, "laksdjfhlskdj");
  ASSERT_EQ(
      re__charclass_builder_insert_ascii_class_by_str(&builder, name, 0),
      RE_ERROR_INVALID);
  re__charclass_builder_destroy(&builder);
  re__rune_data_destroy(&rune_data);
  PASS();
}

SUITE(s_charclass_builder_init_from_class)
{
  RUN_TEST(t_charclass_builder_init_from_class_alnum);
  RUN_TEST(t_charclass_builder_init_from_class_alpha);
  RUN_TEST(t_charclass_builder_init_from_class_ascii);
  RUN_TEST(t_charclass_builder_init_from_class_blank);
  RUN_TEST(t_charclass_builder_init_from_class_cntrl);
  RUN_TEST(t_charclass_builder_init_from_class_digit);
  RUN_TEST(t_charclass_builder_init_from_class_graph);
  RUN_TEST(t_charclass_builder_init_from_class_lower);
  RUN_TEST(t_charclass_builder_init_from_class_perl_space);
  RUN_TEST(t_charclass_builder_init_from_class_print);
  RUN_TEST(t_charclass_builder_init_from_class_punct);
  RUN_TEST(t_charclass_builder_init_from_class_space);
  RUN_TEST(t_charclass_builder_init_from_class_upper);
  RUN_TEST(t_charclass_builder_init_from_class_word);
  RUN_TEST(t_charclass_builder_init_from_class_xdigit);
  RUN_TEST(t_charclass_builder_init_from_class_invalid);
}

TEST(t_charclass_builder_init_from_str)
{
  const char* names[] = {"alnum", "alpha", "ascii", "blank",      "cntrl",
                         "digit", "graph", "lower", "perl_space", "print",
                         "punct", "space", "upper", "word",       "xdigit"};
  re__charclass_ascii_type i = RE__CHARCLASS_ASCII_TYPE_ALNUM;
  re__rune_data rune_data;
  re__charclass_builder builder;
  re__charclass expected;
  re__charclass actual;
  int inverted = 0;
  re__rune_data_init(&rune_data);
  re__charclass_builder_init(&builder, &rune_data);
  for (; inverted < 2; inverted++) {
    for (; i < RE__CHARCLASS_ASCII_TYPE_MAX; i++) {
      mn__str_view name;
      mn__str_view_init_s(&name, names[i]);
      re__charclass_builder_begin(&builder);
      ASSERT_ERR_NOMEM(
          re__charclass_builder_insert_ascii_class_by_str(
              &builder, name, inverted),
          finish);
      ASSERT_ERR_NOMEM(
          re__charclass_builder_finish(&builder, &expected), finish);
      re__charclass_builder_begin(&builder);
      ASSERT_ERR_NOMEM(
          re__charclass_builder_insert_ascii_class(&builder, i, inverted),
          destroy_expected);
      ASSERT_ERR_NOMEM(
          re__charclass_builder_finish(&builder, &actual), destroy_expected);
      ASSERT(re__charclass_equals(&expected, &actual));
      re__charclass_destroy(&expected);
      re__charclass_destroy(&actual);
    }
  }
  goto finish;
destroy_expected:
  re__charclass_destroy(&expected);
finish:
  re__charclass_builder_destroy(&builder);
  re__rune_data_destroy(&rune_data);
  PASS();
}

TEST(t_charclass_builder_init)
{
  re__charclass_builder builder;
  re__rune_data rune_data;
  re__rune_data_init(&rune_data);
  re__charclass_builder_init(&builder, &rune_data);
  ASSERT(builder.should_invert == 0);
  ASSERT(builder.highest == -1);
  re__charclass_builder_destroy(&builder);
  re__rune_data_destroy(&rune_data);
  PASS();
}

TEST(t_charclass_builder_begin)
{
  re__charclass_builder builder;
  re__rune_data rune_data;
  mn_size l = RAND_PARAM(600);
  mn_size i;
  re__rune_data_init(&rune_data);
  re__charclass_builder_init(&builder, &rune_data);
  re__charclass_builder_begin(&builder);
  ASSERT(builder.should_invert == 0);
  ASSERT(builder.highest == -1);
  for (i = 0; i < l; i++) {
    re__rune_range rr;
    rr.min = (re_rune)RAND_PARAM(0x10FFFF);
    rr.max = ((mn_int32)RAND_PARAM((mptest_rand)(0x10FFFF - rr.min))) + rr.min;
    re__charclass_builder_insert_range(&builder, rr);
  }
  re__charclass_builder_begin(&builder);
  ASSERT(builder.should_invert == 0);
  ASSERT(builder.highest == -1);
  re__charclass_builder_destroy(&builder);
  re__rune_data_destroy(&rune_data);
  PASS();
}

TEST(t_charclass_builder_invert)
{
  re__charclass_builder builder;
  re__rune_data rune_data;
  re__rune_data_init(&rune_data);
  re__charclass_builder_init(&builder, &rune_data);
  ASSERT(builder.should_invert == 0);
  re__charclass_builder_invert(&builder);
  ASSERT(builder.should_invert == 1);
  re__charclass_builder_destroy(&builder);
  re__rune_data_destroy(&rune_data);
  PASS();
}

TEST(t_charclass_builder_insert_range)
{
  mn_size l = RAND_PARAM(60);
  mn_size i;
  re__charclass_builder builder;
  re__rune_data rune_data;
  re__rune_data_init(&rune_data);
  re__charclass_builder_init(&builder, &rune_data);
  for (i = 0; i < l; i++) {
    re__rune_range rr;
    rr.min = (re_rune)RAND_PARAM(0x10FFFF);
    rr.max = ((mn_int32)RAND_PARAM((mptest_rand)(0x10FFFF - rr.min))) + rr.min;
    re__charclass_builder_insert_range(&builder, rr);
  }
  re__charclass_builder_destroy(&builder);
  re__rune_data_destroy(&rune_data);
  PASS();
}

TEST(t_charclass_builder_init_empty)
{
  re__rune_data rune_data;
  re__charclass_builder builder;
  re__charclass charclass;
  re__rune_data_init(&rune_data);
  re__charclass_builder_init(&builder, &rune_data);
  re__charclass_builder_begin(&builder);
  ASSERT_ERR_NOMEM(re__charclass_builder_finish(&builder, &charclass), error);
  ASSERT_SYMEQ(re__charclass, charclass, "(charclass ())");
  re__charclass_destroy(&charclass);
error:
  re__charclass_builder_destroy(&builder);
  re__rune_data_destroy(&rune_data);
  PASS();
}

TEST(t_charclass_builder_init_empty_inverted)
{
  re__rune_data rune_data;
  re__charclass_builder builder;
  re__charclass charclass;
  re__rune_data_init(&rune_data);
  re__charclass_builder_init(&builder, &rune_data);
  re__charclass_builder_begin(&builder);
  re__charclass_builder_invert(&builder);
  ASSERT_ERR_NOMEM(re__charclass_builder_finish(&builder, &charclass), error);
  ASSERT_SYMEQ(
      re__charclass, charclass, "(charclass ((rune_range 0 0x10FFFF)))");
  re__charclass_destroy(&charclass);
error:
  re__charclass_builder_destroy(&builder);
  re__rune_data_destroy(&rune_data);
  PASS();
}

TEST(t_charclass_builder_normalize_overlap)
{
  re__rune_data rune_data;
  re__charclass_builder builder;
  re__charclass charclass;
  re__rune_range range;
  re__rune_data_init(&rune_data);
  re__charclass_builder_init(&builder, &rune_data);
  re__charclass_builder_begin(&builder);
  range.min = 0;
  range.max = 5;
  ASSERT_ERR_NOMEM(re__charclass_builder_insert_range(&builder, range), error);
  range.min = 4;
  range.max = 10;
  ASSERT_ERR_NOMEM(re__charclass_builder_insert_range(&builder, range), error);
  ASSERT_ERR_NOMEM(re__charclass_builder_finish(&builder, &charclass), error);
  ASSERT_SYMEQ(re__charclass, charclass, "(charclass ((rune_range 0 10)))");
  re__charclass_destroy(&charclass);
error:
  re__charclass_builder_destroy(&builder);
  re__rune_data_destroy(&rune_data);
  PASS();
}

TEST(t_charclass_builder_normalize_adjacent)
{
  re__rune_data rune_data;
  re__charclass_builder builder;
  re__charclass charclass;
  re__rune_range range;
  re__rune_data_init(&rune_data);
  re__charclass_builder_init(&builder, &rune_data);
  re__charclass_builder_begin(&builder);
  range.min = 0;
  range.max = 5;
  ASSERT_ERR_NOMEM(re__charclass_builder_insert_range(&builder, range), error);
  range.min = 6;
  range.max = 10;
  ASSERT_ERR_NOMEM(re__charclass_builder_insert_range(&builder, range), error);
  ASSERT_ERR_NOMEM(re__charclass_builder_finish(&builder, &charclass), error);
  ASSERT_SYMEQ(re__charclass, charclass, "(charclass ((rune_range 0 10)))");
  re__charclass_destroy(&charclass);
error:
  re__charclass_builder_destroy(&builder);
  re__rune_data_destroy(&rune_data);
  PASS();
}

TEST(t_charclass_builder_normalize_disjoint)
{
  re__rune_data rune_data;
  re__charclass_builder builder;
  re__charclass charclass;
  re__rune_range range;
  re__rune_data_init(&rune_data);
  re__charclass_builder_init(&builder, &rune_data);
  re__charclass_builder_begin(&builder);
  range.min = 0;
  range.max = 5;
  ASSERT_ERR_NOMEM(re__charclass_builder_insert_range(&builder, range), error);
  range.min = 8;
  range.max = 10;
  ASSERT_ERR_NOMEM(re__charclass_builder_insert_range(&builder, range), error);
  ASSERT_ERR_NOMEM(re__charclass_builder_finish(&builder, &charclass), error);
  ASSERT_SYMEQ(
      re__charclass, charclass,
      "(charclass ("
      "  (rune_range 0 5)"
      "  (rune_range 8 10)))");
  re__charclass_destroy(&charclass);
error:
  re__charclass_builder_destroy(&builder);
  re__rune_data_destroy(&rune_data);
  PASS();
}

TEST(t_charclass_builder_normalize_containing)
{
  re__rune_data rune_data;
  re__charclass_builder builder;
  re__charclass charclass;
  re__rune_range range;
  re__rune_data_init(&rune_data);
  re__charclass_builder_init(&builder, &rune_data);
  re__charclass_builder_begin(&builder);
  range.min = 0;
  range.max = 5;
  ASSERT_ERR_NOMEM(re__charclass_builder_insert_range(&builder, range), error);
  range.min = 2;
  range.max = 3;
  ASSERT_ERR_NOMEM(re__charclass_builder_insert_range(&builder, range), error);
  ASSERT_ERR_NOMEM(re__charclass_builder_finish(&builder, &charclass), error);
  ASSERT_SYMEQ(re__charclass, charclass, "(charclass ((rune_range 0 5)))");
  re__charclass_destroy(&charclass);
error:
  re__charclass_builder_destroy(&builder);
  re__rune_data_destroy(&rune_data);
  PASS();
}

TEST(t_charclass_builder_full)
{
  re__rune_data rune_data;
  re__charclass_builder builder;
  re__charclass charclass;
  re__rune_range range;
  re__rune_data_init(&rune_data);
  re__charclass_builder_init(&builder, &rune_data);
  re__charclass_builder_begin(&builder);
  range.min = 0;
  range.max = 0x10FFFF;
  ASSERT_ERR_NOMEM(re__charclass_builder_insert_range(&builder, range), error);
  ASSERT_ERR_NOMEM(re__charclass_builder_finish(&builder, &charclass), error);
  ASSERT_SYMEQ(
      re__charclass, charclass, "(charclass ((rune_range 0 0x10FFFF)))");
  re__charclass_destroy(&charclass);
error:
  re__charclass_builder_destroy(&builder);
  re__rune_data_destroy(&rune_data);
  PASS();
}

TEST(t_charclass_builder_invert_full)
{
  re__rune_data rune_data;
  re__charclass_builder builder;
  re__charclass charclass;
  re__rune_range range;
  re__rune_data_init(&rune_data);
  re__charclass_builder_init(&builder, &rune_data);
  re__charclass_builder_begin(&builder);
  re__charclass_builder_invert(&builder);
  range.min = 0;
  range.max = 0x10FFFF;
  ASSERT_ERR_NOMEM(re__charclass_builder_insert_range(&builder, range), error);
  ASSERT_ERR_NOMEM(re__charclass_builder_finish(&builder, &charclass), error);
  ASSERT_SYMEQ(re__charclass, charclass, "(charclass ())");
  re__charclass_destroy(&charclass);
error:
  re__charclass_builder_destroy(&builder);
  re__rune_data_destroy(&rune_data);
  PASS();
}

TEST(t_charclass_builder_invert_zero)
{
  re__rune_data rune_data;
  re__charclass_builder builder;
  re__charclass charclass;
  re__rune_range range;
  re__rune_data_init(&rune_data);
  re__charclass_builder_init(&builder, &rune_data);
  re__charclass_builder_begin(&builder);
  re__charclass_builder_invert(&builder);
  range.min = 0;
  range.max = 5;
  ASSERT_ERR_NOMEM(re__charclass_builder_insert_range(&builder, range), error);
  range.min = 40;
  range.max = 45;
  ASSERT_ERR_NOMEM(re__charclass_builder_insert_range(&builder, range), error);
  ASSERT_ERR_NOMEM(re__charclass_builder_finish(&builder, &charclass), error);
  ASSERT_SYMEQ(
      re__charclass, charclass,
      "(charclass ("
      "  (rune_range 6 39)"
      "  (rune_range 46 0x10FFFF)))");
  re__charclass_destroy(&charclass);
error:
  re__charclass_builder_destroy(&builder);
  re__rune_data_destroy(&rune_data);
  PASS();
}

TEST(t_charclass_builder_fold)
{
  re__rune_data rune_data;
  re__charclass_builder builder;
  re__charclass charclass;
  re__rune_range range;
  re__rune_data_init(&rune_data);
  re__charclass_builder_init(&builder, &rune_data);
  re__charclass_builder_begin(&builder);
  re__charclass_builder_fold(&builder);
  range.min = 'A';
  range.max = 'Z';
  ASSERT_ERR_NOMEM(re__charclass_builder_insert_range(&builder, range), error);
  ASSERT_ERR_NOMEM(re__charclass_builder_finish(&builder, &charclass), error);
  ASSERT_SYMEQ(
      re__charclass, charclass,
      "(charclass ("
      "  (rune_range 'A' 'Z')"
      "  (rune_range 'a' 'z')"
      "  (rune_range 0x17F 0x17F)"
      "  (rune_range 0x212A 0x212A)))");
  re__charclass_destroy(&charclass);
error:
  re__charclass_builder_destroy(&builder);
  re__rune_data_destroy(&rune_data);
  PASS();
}

TEST(t_charclass_builder_property)
{
  re__rune_data rune_data;
  re__charclass_builder builder;
  re__charclass charclass;
  mn__str_view view;
  re__rune_data_init(&rune_data);
  re__charclass_builder_init(&builder, &rune_data);
  re__charclass_builder_begin(&builder);
  mn__str_view_init_s(&view, "Cc");
  ASSERT_ERR_NOMEM(
      re__charclass_builder_insert_property(&builder, view, 0), error);
  ASSERT_ERR_NOMEM(re__charclass_builder_finish(&builder, &charclass), error);
  ASSERT_SYMEQ(
      re__charclass, charclass,
      "(charclass ("
      "  (rune_range 0 0x1F)"
      "  (rune_range 0x7F 0x9F)))");
  re__charclass_destroy(&charclass);
error:
  re__charclass_builder_destroy(&builder);
  re__rune_data_destroy(&rune_data);
  PASS();
}

TEST(t_charclass_builder_property_inverted)
{
  re__rune_data rune_data;
  re__charclass_builder builder;
  re__charclass charclass;
  mn__str_view view;
  re__rune_data_init(&rune_data);
  re__charclass_builder_init(&builder, &rune_data);
  re__charclass_builder_begin(&builder);
  mn__str_view_init_s(&view, "Cc");
  ASSERT_ERR_NOMEM(
      re__charclass_builder_insert_property(&builder, view, 1), error);
  ASSERT_ERR_NOMEM(re__charclass_builder_finish(&builder, &charclass), error);
  ASSERT_SYMEQ(
      re__charclass, charclass,
      "(charclass ("
      "  (rune_range 0x20 0x7E)"
      "  (rune_range 0xA0 0x10FFFF)))");
  re__charclass_destroy(&charclass);
error:
  re__charclass_builder_destroy(&builder);
  re__rune_data_destroy(&rune_data);
  PASS();
}

SUITE(s_charclass_builder)
{
  RUN_TEST(t_charclass_builder_init);
  RUN_SUITE(s_charclass_builder_init_from_class);
  RUN_TEST(t_charclass_builder_init_from_str);
  RUN_TEST(t_charclass_builder_init_empty);
  RUN_TEST(t_charclass_builder_init_empty_inverted);
  FUZZ_TEST(t_charclass_builder_begin);
  RUN_TEST(t_charclass_builder_invert);
  FUZZ_TEST(t_charclass_builder_insert_range);
  RUN_TEST(t_charclass_builder_normalize_overlap);
  RUN_TEST(t_charclass_builder_normalize_adjacent);
  RUN_TEST(t_charclass_builder_normalize_disjoint);
  RUN_TEST(t_charclass_builder_normalize_containing);
  RUN_TEST(t_charclass_builder_full);
  RUN_TEST(t_charclass_builder_invert_full);
  RUN_TEST(t_charclass_builder_invert_zero);
  RUN_TEST(t_charclass_builder_fold);
  RUN_TEST(t_charclass_builder_property);
  RUN_TEST(t_charclass_builder_property_inverted);
}

TEST(t_charclass_destroy_empty)
{
  re__rune_data rune_data;
  re__charclass_builder builder;
  re__charclass charclass;
  re__rune_data_init(&rune_data);
  re__charclass_builder_init(&builder, &rune_data);
  re__charclass_builder_begin(&builder);
  ASSERT_ERR_NOMEM(re__charclass_builder_finish(&builder, &charclass), error);
  re__charclass_destroy(&charclass);
error:
  re__charclass_builder_destroy(&builder);
  re__rune_data_destroy(&rune_data);
  PASS();
}

TEST(t_charclass_equals)
{
  re__rune_data rune_data;
  re__charclass_builder builder;
  re__charclass charclass;
  mn__str_view view;
  re__rune_data_init(&rune_data);
  re__charclass_builder_init(&builder, &rune_data);
  re__charclass_builder_begin(&builder);
  mn__str_view_init_s(&view, "Cc");
  ASSERT_ERR_NOMEM(
      re__charclass_builder_insert_property(&builder, view, 1), error);
  ASSERT_ERR_NOMEM(re__charclass_builder_finish(&builder, &charclass), error);
  ASSERT(re__charclass_equals(&charclass, &charclass));
  re__charclass_destroy(&charclass);
error:
  re__charclass_builder_destroy(&builder);
  re__rune_data_destroy(&rune_data);
  PASS();
}

TEST(t_charclass_equals_diffsz)
{
  re__rune_data rune_data;
  re__charclass_builder builder;
  re__charclass a;
  re__charclass b;
  mn__str_view view;
  re__rune_data_init(&rune_data);
  re__charclass_builder_init(&builder, &rune_data);
  re__charclass_builder_begin(&builder);
  mn__str_view_init_s(&view, "Cc");
  ASSERT_ERR_NOMEM(
      re__charclass_builder_insert_property(&builder, view, 1), error);
  ASSERT_ERR_NOMEM(re__charclass_builder_finish(&builder, &a), error);
  re__charclass_builder_begin(&builder);
  mn__str_view_init_s(&view, "Lu");
  ASSERT_ERR_NOMEM(
      re__charclass_builder_insert_property(&builder, view, 1), destroy_a);
  ASSERT_ERR_NOMEM(re__charclass_builder_finish(&builder, &b), destroy_a);
  ASSERT(!re__charclass_equals(&a, &b));
  re__charclass_destroy(&b);
destroy_a:
  re__charclass_destroy(&a);
error:
  re__charclass_builder_destroy(&builder);
  re__rune_data_destroy(&rune_data);
  PASS();
}

TEST(t_charclass_equals_samesz)
{
  re__rune_data rune_data;
  re__charclass_builder builder;
  re__charclass a;
  re__charclass b;
  mn__str_view view;
  re__rune_data_init(&rune_data);
  re__charclass_builder_init(&builder, &rune_data);
  re__charclass_builder_begin(&builder);
  mn__str_view_init_s(&view, "alnum");
  ASSERT_ERR_NOMEM(
      re__charclass_builder_insert_ascii_class_by_str(&builder, view, 0),
      error);
  ASSERT_ERR_NOMEM(re__charclass_builder_finish(&builder, &a), error);
  re__charclass_builder_begin(&builder);
  mn__str_view_init_s(&view, "alpha");
  ASSERT_ERR_NOMEM(
      re__charclass_builder_insert_ascii_class_by_str(&builder, view, 1),
      destroy_a);
  ASSERT_ERR_NOMEM(re__charclass_builder_finish(&builder, &b), destroy_a);
  ASSERT(!re__charclass_equals(&a, &b));
  re__charclass_destroy(&b);
destroy_a:
  re__charclass_destroy(&a);
error:
  re__charclass_builder_destroy(&builder);
  re__rune_data_destroy(&rune_data);
  PASS();
}

SUITE(s_charclass)
{
  RUN_SUITE(s_charclass_builder);
  RUN_TEST(t_charclass_destroy_empty);
  RUN_TEST(t_charclass_equals);
  RUN_TEST(t_charclass_equals_diffsz);
  RUN_TEST(t_charclass_equals_samesz);
}

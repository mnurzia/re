#include "test_compile.h"

#include "test_ast.h"
#include "test_helpers.h"
#include "test_prog.h"
#include "test_range.h"

TEST(t_compile_gen_utf8_one)
{
  mn_uint8 chars[10];
  int charslen = re__compile_gen_utf8(0x24, chars);
  ASSERT_EQ(charslen, 1);
  ASSERT_EQ(chars[0], EX_UTF8_VALID_1[0]);
  PASS();
}

TEST(t_compile_gen_utf8_two)
{
  mn_uint8 chars[10];
  int charslen = re__compile_gen_utf8(0xA3, chars);
  ASSERT_EQ(charslen, 2);
  ASSERT_EQ(chars[0], (mn_uint8)EX_UTF8_VALID_2[0]);
  ASSERT_EQ(chars[1], (mn_uint8)EX_UTF8_VALID_2[1]);
  PASS();
}

TEST(t_compile_gen_utf8_three)
{
  mn_uint8 chars[10];
  int charslen = re__compile_gen_utf8(0x939, chars);
  ASSERT_EQ(charslen, 3);
  ASSERT_EQ(chars[0], (mn_uint8)EX_UTF8_VALID_3[0]);
  ASSERT_EQ(chars[1], (mn_uint8)EX_UTF8_VALID_3[1]);
  ASSERT_EQ(chars[2], (mn_uint8)EX_UTF8_VALID_3[2]);
  PASS();
}

TEST(t_compile_gen_utf8_four)
{
  mn_uint8 chars[10];
  int charslen = re__compile_gen_utf8(0x10348, chars);
  ASSERT_EQ(charslen, 4);
  ASSERT_EQ(chars[0], (mn_uint8)EX_UTF8_VALID_4[0]);
  ASSERT_EQ(chars[1], (mn_uint8)EX_UTF8_VALID_4[1]);
  ASSERT_EQ(chars[2], (mn_uint8)EX_UTF8_VALID_4[2]);
  ASSERT_EQ(chars[3], (mn_uint8)EX_UTF8_VALID_4[3]);
  PASS();
}

TEST(t_compile_gen_utf8_toobig)
{
  mn_uint8 chars[10];
  int charslen = re__compile_gen_utf8(0x110000, chars);
  ASSERT_EQ(charslen, 0);
  PASS();
}

int re__compile_test(const char* ast, const char* prog, int reversed)
{
  re__ast_root ast_root;
  re__compile compile;
  re__prog prog_actual;
  re_error err = RE_ERROR_NONE;
  SYM(re__ast_root, ast, &ast_root);
  re__compile_init(&compile);
  re__prog_init(&prog_actual);
  if ((err = re__compile_regex(&compile, &ast_root, &prog_actual, reversed)) ==
      RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  ASSERT_SYMEQ(re__prog, prog_actual, prog);
error:
  re__prog_destroy(&prog_actual);
  re__compile_destroy(&compile);
  re__ast_root_destroy(&ast_root);
  PASS();
}

int re__compile_test_long(const char* ast, const char** progs, int reversed)
{
  mn__str temp;
  const char** i = progs;
  mn__str_init(&temp);
  while (*i) {
    if (mn__str_cat_s(&temp, *i)) {
      goto error;
    }
    i++;
  }
  PROPAGATE(re__compile_test(ast, mn__str_get_data(&temp), reversed));
error:
  mn__str_destroy(&temp);
  PASS();
}

TEST(t_compile_empty)
{
  PROPAGATE(re__compile_test(
      "(ast)",
      "(prog"
      "  (fail)"
      "  (match 1 0))",
      0));
  PASS();
}

TEST(t_compile_rune_ascii)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (rune 'a'))",
      "(prog"
      "  (fail)"
      "  (byte 'a' 2)"
      "  (match 1 0))",
      0));
  PASS();
}

TEST(t_compile_rune_ascii_reversed)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (rune 'a'))",
      "(prog"
      "  (fail)"
      "  (byte 'a' 2)"
      "  (match 1 0))",
      1));
  PASS();
}

TEST(t_compile_rune_unicode)
{
  re__ast_root ast_root;
  re__prog prog, prog_actual;
  re__compile compile;
  re__ast ast;
  mn_int32 ast_ref;
  re_rune rune = re_rune_rand();
  mn_uint8 chars[16];
  mn_uint32 chars_len = (mn_uint32)re__compile_gen_utf8(rune, chars);
  re__prog_loc i;
  re__prog_inst inst;
  re_error err = RE_ERROR_NONE;
  re__ast_init_rune(&ast, rune);
  re__ast_root_init(&ast_root);
  re__prog_init(&prog);
  re__prog_init(&prog_actual);
  re__compile_init(&compile);
  if ((err = re__ast_root_add_child(&ast_root, RE__AST_NONE, ast, &ast_ref)) ==
      RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  re__prog_inst_init_fail(&inst);
  if ((err = re__prog_add(&prog, inst)) == RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  for (i = 0; i < chars_len; i++) {
    re__prog_inst_init_byte(&inst, chars[i]);
    re__prog_inst_set_primary(&inst, 2 + i);
    if ((err = re__prog_add(&prog, inst)) == RE_ERROR_NOMEM) {
      goto error;
    } else if (err) {
      FAIL();
    }
  }
  re__prog_inst_init_match(&inst, 1);
  if ((err = re__prog_add(&prog, inst)) == RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  if ((err = re__compile_regex(&compile, &ast_root, &prog_actual, 0)) ==
      RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  ASSERT(re__prog_test_equals(&prog, &prog_actual));
error:
  re__prog_destroy(&prog);
  re__prog_destroy(&prog_actual);
  re__compile_destroy(&compile);
  re__ast_root_destroy(&ast_root);
  PASS();
}

TEST(t_compile_str)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (str test))",
      "(prog"
      "  (fail)"
      "  (byte 't' 2)"
      "  (byte 'e' 3)"
      "  (byte 's' 4)"
      "  (byte 't' 5)"
      "  (match 1 0))",
      0));
  PASS();
}

TEST(t_compile_str_reversed)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (str test))",
      "(prog"
      "  (fail)"
      "  (byte 't' 2)"
      "  (byte 's' 3)"
      "  (byte 'e' 4)"
      "  (byte 't' 5)"
      "  (match 1 0))",
      1));
  PASS();
}

TEST(t_compile_str_thrash)
{
  re__ast_root ast_root;
  re__prog prog, prog_actual;
  re__compile compile;
  re__ast ast;
  mn_int32 ast_ref;
  mn_int32 str_ref;
  mn__str in_str, copy_str;
  re__prog_loc i;
  re__prog_inst inst;
  mn_size j, num_runes = RAND_PARAM(600);
  re_error err = RE_ERROR_NONE;
  mn__str_init(&in_str);
  mn__str_init(&copy_str);
  re__ast_root_init(&ast_root);
  re__prog_init(&prog);
  re__prog_init(&prog_actual);
  re__compile_init(&compile);
  for (j = 0; j < num_runes; j++) {
    mn_uint8 chars[16];
    re_rune rune = re_rune_rand();
    mn_uint32 chars_len = (mn_uint32)re__compile_gen_utf8(rune, chars);
    if ((err = mn__str_cat_n(&in_str, (mn_char*)chars, chars_len)) ==
        RE_ERROR_NOMEM) {
      goto error;
    } else if (err) {
      FAIL();
    }
  }
  if ((err = mn__str_init_copy(&copy_str, &in_str)) == RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  if ((err = re__ast_root_add_str(&ast_root, copy_str, &str_ref)) ==
      RE_ERROR_NOMEM) {
    mn__str_destroy(&copy_str);
    goto error;
  } else if (err) {
    FAIL();
  }
  re__ast_init_str(&ast, str_ref);
  if ((err = re__ast_root_add_child(&ast_root, RE__AST_NONE, ast, &ast_ref)) ==
      RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  re__prog_inst_init_fail(&inst);
  if ((err = re__prog_add(&prog, inst)) == RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  for (i = 0; i < (re__prog_loc)mn__str_size(&in_str); i++) {
    re__prog_inst_init_byte(&inst, (mn_uint8)mn__str_get_data(&in_str)[i]);
    re__prog_inst_set_primary(&inst, 2 + i);
    if ((err = re__prog_add(&prog, inst)) == RE_ERROR_NOMEM) {
      goto error;
    } else if (err) {
      FAIL();
    }
  }
  re__prog_inst_init_match(&inst, 1);
  if ((err = re__prog_add(&prog, inst)) == RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  if ((err = re__compile_regex(&compile, &ast_root, &prog_actual, 0)) ==
      RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  ASSERT(re__prog_test_equals(&prog, &prog_actual));
error:
  re__prog_destroy(&prog);
  re__prog_destroy(&prog_actual);
  re__compile_destroy(&compile);
  re__ast_root_destroy(&ast_root);
  mn__str_destroy(&in_str);
  PASS();
}

TEST(t_compile_concat)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (concat ("
      "    (rune 'a')"
      "    (rune 'b'))))",
      "(prog"
      "  (fail)"
      "  (byte 'a' 2)"
      "  (byte 'b' 3)"
      "  (match 1 0))",
      0));
  PASS();
}

TEST(t_compile_concat_reversed)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (concat ("
      "    (rune 'a')"
      "    (rune 'b'))))",
      "(prog"
      "  (fail)"
      "  (byte 'b' 2)"
      "  (byte 'a' 3)"
      "  (match 1 0))",
      1));
  PASS();
}

TEST(t_compile_alt)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (alt ("
      "    (rune 'a')"
      "    (rune 'b'))))",
      "(prog"
      "  (fail)"
      "  (split 2 3)"
      "  (byte 'a' 4)"
      "  (byte 'b' 4)"
      "  (match 1 0))",
      0));
  PASS();
}

TEST(t_compile_alts)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (alt ("
      "    (rune 'a')"
      "    (rune 'b')"
      "    (rune 'c')"
      "    (rune 'd'))))",
      "(prog"
      "  (fail)"
      "  (split 2 3)"
      "  (byte 'a' 8)"
      "  (split 4 5)"
      "  (byte 'b' 8)"
      "  (split 6 7)"
      "  (byte 'c' 8)"
      "  (byte 'd' 8)"
      "  (match 1 0))",
      0));
  PASS();
}

TEST(t_compile_alts_after)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (concat ("
      "    (rune 'q')"
      "    (alt ("
      "      (rune 'a')"
      "      (rune 'b')"
      "      (rune 'c')"
      "      (rune 'd'))))))",
      "(prog"
      "  (fail)"
      "  (byte 'q' 2)"
      "  (split 3 4)"
      "  (byte 'a' 9)"
      "  (split 5 6)"
      "  (byte 'b' 9)"
      "  (split 7 8)"
      "  (byte 'c' 9)"
      "  (byte 'd' 9)"
      "  (match 1 0))",
      0));
  PASS();
}

TEST(t_compile_star)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (quantifier 0 inf greedy"
      "    (rune 'a')))",
      "(prog"
      "  (fail)"
      "  (split 2 3)"
      "  (byte 'a' 1)"
      "  (match 1 0))",
      0));
  PASS();
}

TEST(t_compile_star_nongreedy)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (quantifier 0 inf nongreedy"
      "    (rune 'a')))",
      "(prog"
      "  (fail)"
      "  (split 3 2)"
      "  (byte 'a' 1)"
      "  (match 1 0))",
      0));
  PASS();
}

TEST(t_compile_question)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (quantifier 0 2 greedy"
      "    (rune 'a')))",
      "(prog"
      "  (fail)"
      "  (split 2 3)"
      "  (byte 'a' 3)"
      "  (match 1 0))",
      0));
  PASS();
}

TEST(t_compile_question_nongreedy)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (quantifier 0 2 nongreedy"
      "    (rune 'a')))",
      "(prog"
      "  (fail)"
      "  (split 3 2)"
      "  (byte 'a' 3)"
      "  (match 1 0))",
      0));
  PASS();
}

TEST(t_compile_plus)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (quantifier 1 inf greedy"
      "    (rune 'a')))",
      "(prog"
      "  (fail)"
      "  (byte 'a' 2)"
      "  (split 1 3)"
      "  (match 1 0))",
      0));
  PASS();
}

TEST(t_compile_plus_nongreedy)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (quantifier 1 inf nongreedy"
      "    (rune 'a')))",
      "(prog"
      "  (fail)"
      "  (byte 'a' 2)"
      "  (split 3 1)"
      "  (match 1 0))",
      0));
  PASS();
}

TEST(t_compile_quantifier_nomax)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (quantifier 3 inf greedy"
      "    (rune 'a')))",
      "(prog"
      "  (fail)"
      "  (byte 'a' 2)"
      "  (byte 'a' 3)"
      "  (byte 'a' 4)"
      "  (split 3 5)"
      "  (match 1 0))",
      0));
  PASS();
}

TEST(t_compile_quantifier_nomax_nongreedy)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (quantifier 3 inf nongreedy"
      "    (rune 'a')))",
      "(prog"
      "  (fail)"
      "  (byte 'a' 2)"
      "  (byte 'a' 3)"
      "  (byte 'a' 4)"
      "  (split 5 3)"
      "  (match 1 0))",
      0));
  PASS();
}

TEST(t_compile_quantifier_minmax)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (quantifier 3 6 greedy"
      "    (rune 'a')))",
      "(prog"
      "  (fail)"
      "  (byte 'a' 2)"
      "  (byte 'a' 3)"
      "  (byte 'a' 4)"
      "  (split 5 8)"
      "  (byte 'a' 6)"
      "  (split 7 8)"
      "  (byte 'a' 8)"
      "  (match 1 0))",
      0));
  PASS();
}

TEST(t_compile_quantifier_minmax_nongreedy)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (quantifier 3 6 nongreedy"
      "    (rune 'a')))",
      "(prog"
      "  (fail)"
      "  (byte 'a' 2)"
      "  (byte 'a' 3)"
      "  (byte 'a' 4)"
      "  (split 8 5)"
      "  (byte 'a' 6)"
      "  (split 8 7)"
      "  (byte 'a' 8)"
      "  (match 1 0))",
      0));
  PASS();
}

TEST(t_compile_group)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (group () 0"
      "    (rune 'a')))",
      "(prog"
      "  (fail)"
      "  (save 0 2)"
      "  (byte 'a' 3)"
      "  (save 1 4)"
      "  (match 1 0))",
      0));
  PASS();
}

TEST(t_compile_group_nested)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (group () 0"
      "    (group () 1"
      "      (rune 'a'))))",
      "(prog"
      "  (fail)"
      "  (save 0 2)"
      "  (save 2 3)"
      "  (byte 'a' 4)"
      "  (save 3 5)"
      "  (save 1 6)"
      "  (match 1 0))",
      0));
  PASS();
}

TEST(t_compile_group_reversed)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (group () 0"
      "    (rune 'a')))",
      "(prog"
      "  (fail)"
      "  (save 1 2)"
      "  (byte 'a' 3)"
      "  (save 0 4)"
      "  (match 1 0))",
      1));
  PASS();
}

TEST(t_compile_group_nonmatching)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (group (nonmatching)"
      "    (rune 'a')))",
      "(prog"
      "  (fail)"
      "  (byte 'a' 2)"
      "  (match 1 0))",
      0));
  PASS();
}

TEST(t_compile_assert_text_start)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (assert (text_start)))",
      "(prog"
      "  (fail)"
      "  (assert (text_start) 2)"
      "  (match 1 0))",
      0));
  PASS();
}

TEST(t_compile_assert_text_end)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (assert (text_end)))",
      "(prog"
      "  (fail)"
      "  (assert (text_end) 2)"
      "  (match 1 0))",
      0));
  PASS();
}

TEST(t_compile_assert_text_start_absolute)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (assert (text_start_absolute)))",
      "(prog"
      "  (fail)"
      "  (assert (text_start_absolute) 2)"
      "  (match 1 0))",
      0));
  PASS();
}

TEST(t_compile_assert_text_end_absolute)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (assert (text_end_absolute)))",
      "(prog"
      "  (fail)"
      "  (assert (text_end_absolute) 2)"
      "  (match 1 0))",
      0));
  PASS();
}

TEST(t_compile_assert_word)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (assert (word)))",
      "(prog"
      "  (fail)"
      "  (assert (word) 2)"
      "  (match 1 0))",
      0));
  PASS();
}

TEST(t_compile_assert_word_not)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (assert (word_not)))",
      "(prog"
      "  (fail)"
      "  (assert (word_not) 2)"
      "  (match 1 0))",
      0));
  PASS();
}

TEST(t_compile_assert_text_start_reversed)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (assert (text_start)))",
      "(prog"
      "  (fail)"
      "  (assert (text_end) 2)"
      "  (match 1 0))",
      1));
  PASS();
}

TEST(t_compile_assert_text_end_reversed)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (assert (text_end)))",
      "(prog"
      "  (fail)"
      "  (assert (text_start) 2)"
      "  (match 1 0))",
      1));
  PASS();
}

TEST(t_compile_assert_text_start_absolute_reversed)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (assert (text_start_absolute)))",
      "(prog"
      "  (fail)"
      "  (assert (text_end_absolute) 2)"
      "  (match 1 0))",
      1));
  PASS();
}

TEST(t_compile_assert_text_end_absolute_reversed)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (assert (text_end_absolute)))",
      "(prog"
      "  (fail)"
      "  (assert (text_start_absolute) 2)"
      "  (match 1 0))",
      1));
  PASS();
}

TEST(t_compile_assert_word_reversed)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (assert (word)))",
      "(prog"
      "  (fail)"
      "  (assert (word) 2)"
      "  (match 1 0))",
      1));
  PASS();
}

TEST(t_compile_assert_word_not_reversed)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (assert (word_not)))",
      "(prog"
      "  (fail)"
      "  (assert (word_not) 2)"
      "  (match 1 0))",
      1));
  PASS();
}

TEST(t_compile_assert_multiple)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (assert (text_start text_end_absolute word)))",
      "(prog"
      "  (fail)"
      "  (assert (text_start text_end_absolute word) 2)"
      "  (match 1 0))",
      0));
  PASS();
}

/* just test simple compilation, the heavy lifting is done in
 * re_compile_charclass.c */
TEST(t_compile_charclass)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (charclass ("
      "    (rune_range 'a' 'z'))))",
      "(prog"
      "  (fail)"
      "  (byte_range 'a' 'z' 2)"
      "  (match 1 0))",
      0));
  PASS();
}

TEST(t_compile_any_char)
{
  /* over 512 chars. thanks c89 */
  /* shamelessly copy-pasted this. not good test etiquette. */
  const char* expected_prog[] = {
      "(prog ",
      "  (fail)",
      "  (split 2 3)",
      "  (byte_range 0 9 27)",
      "  (split 4 5)",
      "  (byte_range 11 127 27)",
      "  (split 6 8)",
      "  (byte_range 194 223 7)",
      "  (byte_range 128 191 27)",
      "  (split 9 11)",
      "  (byte 224 10)",
      "  (byte_range 160 191 7)",
      "  (split 12 14)",
      "  (byte_range 225 236 13)",
      "  (byte_range 128 191 7)",
      "  (split 15 17)",
      "  (byte 237 16)",
      "  (byte_range 128 159 7)",
      "  (split 18 19)",
      "  (byte_range 238 239 13)",
      "  (split 20 22)",
      "  (byte 240 21)",
      "  (byte_range 144 191 13)",
      "  (split 23 25)",
      "  (byte_range 241 243 24)",
      "  (byte_range 128 191 13)",
      "  (byte 244 26)",
      "  (byte_range 128 143 13)",
      "  (match 1 0))",
      NULL};
  PROPAGATE(re__compile_test_long(
      "(ast"
      "  (any_char))",
      expected_prog, 0));
  PASS();
}

TEST(t_compile_any_char_reversed)
{
  const char* expected_prog[] = {
      "(prog",
      "(fail)",
      "(split 2 3)",
      "(byte_range 0 9 25)",
      "(split 4 5)",
      "(byte_range 11 127 25)",
      "(byte_range 128 191 6)",
      "(split 7 15)",
      "(byte_range 128 159 8)",
      "(split 9 11)",
      "(byte_range 128 143 10)",
      "(byte_range 241 244 25)",
      "(split 12 14)",
      "(byte_range 144 191 13)",
      "(byte_range 240 243 25)",
      "(byte_range 225 239 25)",
      "(split 16 24)",
      "(byte_range 160 191 17)",
      "(split 18 19)",
      "(byte_range 128 143 10)",
      "(split 20 21)",
      "(byte_range 144 191 13)",
      "(split 22 23)",
      "(byte_range 224 236 25)",
      "(byte_range 238 239 25)",
      "(byte_range 194 223 25)",
      "(match 1 0))",
      NULL};
  PROPAGATE(re__compile_test_long(
      "(ast"
      "  (any_char))",
      expected_prog, 1));
  PASS();
}

TEST(t_compile_any_char_newline)
{
  const char* expected_prog[] = {
      "(prog",
      "  (fail)",
      "  (split 2 3)",
      "  (byte_range 0 127 25)",
      "  (split 4 6)",
      "  (byte_range 194 223 5)",
      "  (byte_range 128 191 25)",
      "  (split 7 9)",
      "  (byte 224 8)",
      "  (byte_range 160 191 5)",
      "  (split 10 12)",
      "  (byte_range 225 236 11)",
      "  (byte_range 128 191 5)",
      "  (split 13 15)",
      "  (byte 237 14)",
      "  (byte_range 128 159 5)",
      "  (split 16 17)",
      "  (byte_range 238 239 11)",
      "  (split 18 20)",
      "  (byte 240 19)",
      "  (byte_range 144 191 11)",
      "  (split 21 23)",
      "  (byte_range 241 243 22)",
      "  (byte_range 128 191 11)",
      "  (byte 244 24)",
      "  (byte_range 128 143 11)",
      "  (match 1 0))",
      NULL};
  PROPAGATE(re__compile_test_long(
      "(ast"
      "  (any_char_newline))",
      expected_prog, 0));
  PASS();
}

TEST(t_compile_any_char_newline_reversed)
{
  const char* expected_prog[] = {
      "(prog",
      "  (fail)",
      "  (split 2 3)",
      "  (byte_range 0 127 23)",
      "  (byte_range 128 191 4)",
      "  (split 5 13)",
      "  (byte_range 128 159 6)",
      "  (split 7 9)",
      "  (byte_range 128 143 8)",
      "  (byte_range 241 244 23)",
      "  (split 10 12)",
      "  (byte_range 144 191 11)",
      "  (byte_range 240 243 23)",
      "  (byte_range 225 239 23)",
      "  (split 14 22)",
      "  (byte_range 160 191 15)",
      "  (split 16 17)",
      "  (byte_range 128 143 8)",
      "  (split 18 19)",
      "  (byte_range 144 191 11)",
      "  (split 20 21)",
      "  (byte_range 224 236 23)",
      "  (byte_range 238 239 23)",
      "  (byte_range 194 223 23)",
      "  (match 1 0))",
      NULL};
  PROPAGATE(re__compile_test_long(
      "(ast"
      "  (any_char_newline))",
      expected_prog, 1));
  PASS();
}

TEST(t_compile_any_byte)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (any_byte))",
      "(prog"
      "  (fail)"
      "  (byte_range 0 255 2)"
      "  (match 1 0))",
      0));
  PASS();
}

TEST(t_compile_any_byte_reversed)
{
  PROPAGATE(re__compile_test(
      "(ast"
      "  (any_byte))",
      "(prog"
      "  (fail)"
      "  (byte_range 0 255 2)"
      "  (match 1 0))",
      1));
  PASS();
}

TEST(t_compile_dotstar)
{
  re__prog out_prog;
  re__prog_inst inst;
  re__prog_init(&out_prog);
  re__prog_inst_init_fail(&inst);
  ASSERT_ERR_NOMEM(re__prog_add(&out_prog, inst), error);
  re__prog_inst_init_match(&inst, 0);
  ASSERT_ERR_NOMEM(re__prog_add(&out_prog, inst), error);
  re__prog_set_entry(&out_prog, RE__PROG_ENTRY_DEFAULT, 1);
  ASSERT_ERR_NOMEM(
      re__compile_dotstar(&out_prog, RE__PROG_DATA_ID_DOT_FWD_ACCSURR_ACCNL),
      error);
error:
  re__prog_destroy(&out_prog);
  PASS();
}

SUITE(s_compile)
{
  RUN_TEST(t_compile_gen_utf8_one);
  RUN_TEST(t_compile_gen_utf8_two);
  RUN_TEST(t_compile_gen_utf8_three);
  RUN_TEST(t_compile_gen_utf8_four);
  RUN_TEST(t_compile_gen_utf8_toobig);
  RUN_TEST(t_compile_empty);
  RUN_TEST(t_compile_rune_ascii);
  RUN_TEST(t_compile_rune_ascii_reversed);
  FUZZ_TEST(t_compile_rune_unicode);
  RUN_TEST(t_compile_str);
  RUN_TEST(t_compile_str_reversed);
  FUZZ_TEST(t_compile_str_thrash);
  RUN_TEST(t_compile_concat);
  RUN_TEST(t_compile_concat_reversed);
  RUN_TEST(t_compile_alt);
  RUN_TEST(t_compile_alts);
  RUN_TEST(t_compile_alts_after);
  RUN_TEST(t_compile_star);
  RUN_TEST(t_compile_star_nongreedy);
  RUN_TEST(t_compile_question);
  RUN_TEST(t_compile_question_nongreedy);
  RUN_TEST(t_compile_plus);
  RUN_TEST(t_compile_plus_nongreedy);
  RUN_TEST(t_compile_quantifier_nomax);
  RUN_TEST(t_compile_quantifier_nomax_nongreedy);
  RUN_TEST(t_compile_quantifier_minmax);
  RUN_TEST(t_compile_quantifier_minmax_nongreedy);
  RUN_TEST(t_compile_group);
  RUN_TEST(t_compile_group_nested);
  RUN_TEST(t_compile_group_reversed);
  RUN_TEST(t_compile_group_nonmatching);
  RUN_TEST(t_compile_assert_text_start);
  RUN_TEST(t_compile_assert_text_end);
  RUN_TEST(t_compile_assert_text_start_absolute);
  RUN_TEST(t_compile_assert_text_end_absolute);
  RUN_TEST(t_compile_assert_word);
  RUN_TEST(t_compile_assert_word_not);
  RUN_TEST(t_compile_assert_text_start_reversed);
  RUN_TEST(t_compile_assert_text_end_reversed);
  RUN_TEST(t_compile_assert_text_start_absolute_reversed);
  RUN_TEST(t_compile_assert_text_end_absolute_reversed);
  RUN_TEST(t_compile_assert_word_reversed);
  RUN_TEST(t_compile_assert_word_not_reversed);
  RUN_TEST(t_compile_assert_multiple);
  RUN_TEST(t_compile_charclass);
  RUN_TEST(t_compile_any_char);
  RUN_TEST(t_compile_any_char_reversed);
  RUN_TEST(t_compile_any_char_newline);
  RUN_TEST(t_compile_any_char_newline_reversed);
  RUN_TEST(t_compile_any_byte);
  RUN_TEST(t_compile_any_byte_reversed);
  RUN_TEST(t_compile_dotstar);
}

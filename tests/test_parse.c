#include "test_parse.h"

#include "test_ast.h"
#include "test_helpers.h"

int re__parse_test(const char* regex, const char* ast)
{
  re reg;
  re_error err = RE_ERROR_NONE;
  if ((err = re_init(&reg, regex))) {
    if (err == RE_ERROR_NOMEM) {
      goto error;
    } else {
      FAIL();
    }
  }
  ASSERT_SYMEQ(re__ast_root, reg.data->ast_root, ast);
error:
  re_destroy(&reg);
  PASS();
}

int re__parse_test_err(const char* regex)
{
  re reg;
  re_error err = RE_ERROR_NONE;
  if ((err = re_init(&reg, regex))) {
    if (err == RE_ERROR_NOMEM) {
      goto error;
    } else if (err == RE_ERROR_PARSE) {
      /* fallthrough */
    } else {
      FAIL();
    }
  } else {
    FAIL();
  }
error:
  re_destroy(&reg);
  PASS();
}

int re__parse_testm(const char* regex, const char* ast, const char* message)
{
  re reg;
  re_error err = RE_ERROR_NONE;
  if ((err = re_init(&reg, regex))) {
    if (err == RE_ERROR_NOMEM) {
      goto error;
    } else {
      FAIL();
    }
  }
  ASSERT_SYMEQm(re__ast_root, reg.data->ast_root, ast, message);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_empty)
{
  PROPAGATE(re__parse_testm("", "(ast)", "empty regex should compile"));
  PASS();
}

TEST(t_parse_rune)
{
  PROPAGATE(re__parse_test(
      "a", "(ast"
           "    (rune 'a'))"));
  PASS();
}

TEST(t_parse_rune_case_insensitive_fold)
{
  PROPAGATE(re__parse_test(
      "(?i)a", "(ast"
               "  (charclass ("
               "    (rune_range 'A' 'A')"
               "    (rune_range 'a' 'a'))))"));
  PASS();
}

TEST(t_parse_rune_case_insensitive_nofold)
{
  PROPAGATE(re__parse_test(
      "(?i)&", "(ast"
               "  (rune '&'))"));
  PASS();
}

TEST(t_parse_escape_unfinished)
{
  PROPAGATE(re__parse_test_err("\\"));
  PASS();
}

TEST(t_parse_escape_invalid)
{
  PROPAGATE(re__parse_test_err("\\\xff"));
  PASS();
}

TEST(t_parse_escape_invalid_char)
{
  char* invalid_escapes = "FGHIJKLMNORTUVXYZceghijklmoquy";
  while (*invalid_escapes) {
    char regs[3];
    regs[0] = '\\';
    regs[1] = *invalid_escapes;
    regs[2] = '\0';
    PROPAGATE(re__parse_test_err(regs));
    invalid_escapes++;
  }
  PASS();
}

TEST(t_parse_escape_literal_hi)
{
  PROPAGATE(re__parse_test(
      "\\~", "(ast"
             "  (rune '~'))"));
  PASS();
}

TEST(t_parse_escape_literal_lo)
{
  PROPAGATE(re__parse_test(
      "\\&", "(ast"
             "  (rune '&'))"));
  PASS();
}

TEST(t_parse_escape_octal_one)
{
  PROPAGATE(re__parse_test(
      "\\0\\1\\2\\3\\4\\5\\6\\7",
      "(ast"
      "  (str \"\\x00\\x01\\x02\\x03\\x04\\x05\\x06\\x07\"))"));
  PASS();
}

TEST(t_parse_escape_octal_two)
{
  PROPAGATE(re__parse_test(
      "\\01\\12\\23\\34\\45\\56\\67\\70",
      "(ast"
      "  (str \"\\x01\\x0a\\x13\\x1c\\x25\\x2e\\x37\\x38\"))"));
  PASS();
}

TEST(t_parse_escape_octal_three)
{
  PROPAGATE(re__parse_test(
      "\\141\\142", "(ast"
                    "  (str ab))"));
  PASS();
}

TEST(t_parse_escape_octal_invalid)
{
  PROPAGATE(re__parse_test_err("\\0\xFF"));
  PASS();
}

TEST(t_parse_escape_text_start_absolute)
{
  PROPAGATE(re__parse_test(
      "\\A", "(ast"
             "  (assert (text_start_absolute)))"));
  PASS();
}

TEST(t_parse_escape_text_start_absolute_in_charclass)
{
  PROPAGATE(re__parse_test_err("[\\A]"));
  PASS();
}

TEST(t_parse_escape_assert_word_not)
{
  PROPAGATE(re__parse_test(
      "\\B", "(ast"
             "  (assert (word_not)))"));
  PASS();
}

TEST(t_parse_escape_assert_word_not_in_charclass)
{
  PROPAGATE(re__parse_test_err("[\\B]"));
  PASS();
}

TEST(t_parse_escape_any_byte)
{
  PROPAGATE(re__parse_test(
      "\\C", "(ast"
             "  (any_byte))"));
  PASS();
}

TEST(t_parse_escape_any_byte_in_charclass)
{
  PROPAGATE(re__parse_test_err("[\\C]"));
  PASS();
}

TEST(t_parse_escape_digit_not)
{
  PROPAGATE(re__parse_test(
      "\\D", "(ast"
             "  (charclass ("
             "    (rune_range 0 47)"
             "    (rune_range 58 0x10FFFF))))"));
  PASS();
}

TEST(t_parse_escape_digit_not_in_charclass)
{
  PROPAGATE(re__parse_test(
      "[\\D]", "(ast"
               "  (charclass ("
               "    (rune_range 0 47)"
               "    (rune_range 58 0x10FFFF))))"));
  PASS();
}

TEST(t_parse_escape_digit_not_ending_range)
{
  PROPAGATE(re__parse_test_err("[a-\\D]"));
  PASS();
}

TEST(t_parse_escape_quote_end_unmatched)
{
  PROPAGATE(re__parse_test_err("\\E"));
  PASS();
}

TEST(t_parse_escape_quote_empty)
{
  PROPAGATE(re__parse_test("\\Q\\E", "(ast)"));
  PASS();
}

TEST(t_parse_escape_quote_empty_unfinished)
{
  PROPAGATE(re__parse_test("\\Q", "(ast)"));
  PASS();
}

TEST(t_parse_escape_quote_text)
{
  PROPAGATE(re__parse_test(
      "\\Qabc\\E", "(ast"
                   "    (str \"abc\"))"));
  PASS();
}

TEST(t_parse_escape_quote_invalid)
{
  PROPAGATE(re__parse_test_err("\\Q\xFF"));
  PASS();
}

TEST(t_parse_escape_quote_text_unfinished)
{
  PROPAGATE(re__parse_test(
      "\\Qabc", "(ast"
                "    (str \"abc\"))"));
  PASS();
}

TEST(t_parse_escape_quote_escape)
{
  PROPAGATE(re__parse_test(
      "\\Q\\\\\\E", "(ast"
                    "    (rune 0x5C))"));
  PASS();
}

TEST(t_parse_escape_quote_escape_slash)
{
  PROPAGATE(re__parse_test(
      "\\Q\\\\", "(ast"
                 "    (rune 0x5C))"));
  PASS();
}

TEST(t_parse_escape_quote_escape_unfinished)
{
  PROPAGATE(re__parse_test_err("\\Q\\"));
  PASS();
}

TEST(t_parse_escape_quote_in_charclass)
{
  PROPAGATE(re__parse_test_err("[\\Q]"));
  PASS();
}

TEST(t_parse_escape_unicode_property_ending_range)
{
  PROPAGATE(re__parse_test_err("[a-\\p{Cc}]"));
  PASS();
}

TEST(t_parse_escape_unicode_property_invalid_one)
{
  PROPAGATE(re__parse_test_err("\\p\xFF}"));
  PASS();
}

TEST(t_parse_escape_unicode_property_invalid_two)
{
  PROPAGATE(re__parse_test_err("\\pA"));
  PASS();
}

TEST(t_parse_escape_unicode_property_empty_name)
{
  PROPAGATE(re__parse_test_err("\\p{}"));
  PASS();
}

TEST(t_parse_escape_unicode_property_invalid_three)
{
  PROPAGATE(re__parse_test_err("\\p{A}"));
  PASS();
}

TEST(t_parse_escape_unicode_property_invalid_four)
{
  PROPAGATE(re__parse_test_err("\\p{\xFF}"));
  PASS();
}

TEST(t_parse_escape_unicode_property_unfinished)
{
  PROPAGATE(re__parse_test_err("\\p{"));
  PASS();
}

TEST(t_parse_escape_unicode_property_valid)
{
  PROPAGATE(re__parse_test(
      "\\p{Cc}", "(ast"
                 "  (charclass ("
                 "    (rune_range 0x00 0x1F)"
                 "    (rune_range 0x7F 0x9F))))"));
  PASS();
}

TEST(t_parse_escape_unicode_property_valid_in_charclass)
{
  PROPAGATE(re__parse_test(
      "[\\p{Cc}]", "(ast"
                   "  (charclass ("
                   "    (rune_range 0x00 0x1F)"
                   "    (rune_range 0x7F 0x9F))))"));
  PASS();
}

TEST(t_parse_escape_unicode_property_valid_inverted)
{
  PROPAGATE(re__parse_test(
      "\\P{Cc}", "(ast"
                 "  (charclass ("
                 "    (rune_range 0x20 0x7E)"
                 "    (rune_range 0xA0 0x10FFFF))))"));
  PASS();
}

TEST(t_parse_escape_unicode_property_valid_inverted_in_charclass)
{
  PROPAGATE(re__parse_test(
      "[\\P{Cc}]", "(ast"
                   "  (charclass ("
                   "    (rune_range 0x20 0x7E)"
                   "    (rune_range 0xA0 0x10FFFF))))"));
  PASS();
}

TEST(t_parse_escape_unicode_property_valid_case_insensitive)
{
  PROPAGATE(re__parse_test(
      "(?i)\\p{Cc}", "(ast"
                     "  (charclass ("
                     "    (rune_range 0x00 0x1F)"
                     "    (rune_range 0x7F 0x9F))))"));
  PASS();
}

TEST(t_parse_escape_perlspace_not)
{
  PROPAGATE(re__parse_test(
      "\\S", "(ast"
             "  (charclass ("
             "    (rune_range 0 8)"
             "    (rune_range 11 11)"
             "    (rune_range 14 31)"
             "    (rune_range 33 0x10FFFF))))"));
  PASS();
}

TEST(t_parse_escape_perlspace_not_in_charclass)
{
  PROPAGATE(re__parse_test(
      "[\\S]", "(ast"
               "  (charclass ("
               "    (rune_range 0 8)"
               "    (rune_range 11 11)"
               "    (rune_range 14 31)"
               "    (rune_range 33 0x10FFFF))))"));
  PASS();
}

TEST(t_parse_escape_perlspace_not_ending_range)
{
  PROPAGATE(re__parse_test_err("[a-\\S]"));
  PASS();
}

TEST(t_parse_escape_word_not)
{
  PROPAGATE(re__parse_test(
      "\\W", "(ast"
             "  (charclass ("
             "    (rune_range 0 47)"
             "    (rune_range 58 64)"
             "    (rune_range 91 94)"
             "    (rune_range 96 96)"
             "    (rune_range 123 0x10FFFF))))"));
  PASS();
}

TEST(t_parse_escape_word_not_in_charclass)
{
  PROPAGATE(re__parse_test(
      "[\\W]", "(ast"
               "  (charclass ("
               "    (rune_range 0 47)"
               "    (rune_range 58 64)"
               "    (rune_range 91 94)"
               "    (rune_range 96 96)"
               "    (rune_range 123 0x10FFFF))))"));
  PASS();
}

TEST(t_parse_escape_word_not_ending_range)
{
  PROPAGATE(re__parse_test_err("[a-\\W]"));
  PASS();
}

TEST(t_parse_escape_bell)
{
  PROPAGATE(re__parse_test(
      "\\a", "(ast"
             "  (rune 7))"));
  PASS();
}

TEST(t_parse_escape_bell_in_charclass)
{
  PROPAGATE(re__parse_test(
      "[\\a]", "(ast"
               "  (rune 7))"));
  PASS();
}

TEST(t_parse_escape_assert_word)
{
  PROPAGATE(re__parse_test(
      "\\b", "(ast"
             "  (assert (word)))"));
  PASS();
}

TEST(t_parse_escape_assert_word_in_charclass)
{
  PROPAGATE(re__parse_test_err("[\\b]"));
  PASS();
}

TEST(t_parse_escape_digit)
{
  PROPAGATE(re__parse_test(
      "\\d", "(ast"
             "  (charclass ("
             "    (rune_range '0' '9'))))"));
  PASS();
}

TEST(t_parse_escape_digit_in_charclass)
{
  PROPAGATE(re__parse_test(
      "[\\d]", "(ast"
               "  (charclass ("
               "    (rune_range '0' '9'))))"));
  PASS();
}

TEST(t_parse_escape_digit_ending_range)
{
  PROPAGATE(re__parse_test_err("[a-\\d]"));
  PASS();
}

TEST(t_parse_escape_form_feed)
{
  PROPAGATE(re__parse_test(
      "\\f", "(ast"
             "  (rune 0xC))"));
  PASS();
}

TEST(t_parse_escape_form_feed_in_charclass)
{
  PROPAGATE(re__parse_test(
      "[\\f]", "(ast"
               "  (rune 0xC))"));
  PASS();
}

TEST(t_parse_escape_newline)
{
  PROPAGATE(re__parse_test(
      "\\n", "(ast"
             "  (rune 0xA))"));
  PASS();
}

TEST(t_parse_escape_newline_in_charclass)
{
  PROPAGATE(re__parse_test(
      "[\\n]", "(ast"
               "  (rune 0xA))"));
  PASS();
}

TEST(t_parse_escape_carriage_return)
{
  PROPAGATE(re__parse_test(
      "\\r", "(ast"
             "  (rune 0xD))"));
  PASS();
}

TEST(t_parse_escape_carriage_return_in_charclass)
{
  PROPAGATE(re__parse_test(
      "[\\r]", "(ast"
               "  (rune 0xD))"));
  PASS();
}

TEST(t_parse_escape_perlspace)
{
  PROPAGATE(re__parse_test(
      "\\s", "(ast"
             "  (charclass ("
             "    (rune_range 9 10)"
             "    (rune_range 12 13)"
             "    (rune_range 32 32))))"));
  PASS();
}

TEST(t_parse_escape_perlspace_in_charclass)
{
  PROPAGATE(re__parse_test(
      "[\\s]", "(ast"
               "  (charclass ("
               "    (rune_range 9 10)"
               "    (rune_range 12 13)"
               "    (rune_range 32 32))))"));
  PASS();
}

TEST(t_parse_escape_perlspace_ending_range)
{
  PROPAGATE(re__parse_test_err("[a-\\s]"));
  PASS();
}

TEST(t_parse_escape_horizontal_tab)
{
  PROPAGATE(re__parse_test(
      "\\t", "(ast"
             "  (rune 0x9))"));
  PASS();
}

TEST(t_parse_escape_horizontal_tab_in_charclass)
{
  PROPAGATE(re__parse_test(
      "[\\t]", "(ast"
               "  (rune 0x9))"));
  PASS();
}

TEST(t_parse_escape_vertical_tab)
{
  PROPAGATE(re__parse_test(
      "\\v", "(ast"
             "  (rune 0xB))"));
  PASS();
}

TEST(t_parse_escape_vertical_tab_in_charclass)
{
  PROPAGATE(re__parse_test(
      "[\\v]", "(ast"
               "  (rune 0xB))"));
  PASS();
}

TEST(t_parse_escape_word)
{
  PROPAGATE(re__parse_test(
      "\\w", "(ast"
             "  (charclass ("
             "    (rune_range '0' '9')"
             "    (rune_range 'A' 'Z')"
             "    (rune_range '_' '_')"
             "    (rune_range 'a' 'z'))))"));
  PASS();
}

TEST(t_parse_escape_word_in_charclass)
{
  PROPAGATE(re__parse_test(
      "[\\w]", "(ast"
               "  (charclass ("
               "    (rune_range '0' '9')"
               "    (rune_range 'A' 'Z')"
               "    (rune_range '_' '_')"
               "    (rune_range 'a' 'z'))))"));
  PASS();
}

TEST(t_parse_escape_word_ending_range)
{
  PROPAGATE(re__parse_test_err("[a-\\w]"));
  PASS();
}

TEST(t_parse_escape_hex_invalid)
{
  PROPAGATE(re__parse_test_err("\\x\xFF"));
  PASS();
}

TEST(t_parse_escape_hex_eof)
{
  PROPAGATE(re__parse_test_err("\\x"));
  PASS();
}

TEST(t_parse_escape_hex_twodig_invalid_one)
{
  PROPAGATE(re__parse_test_err("\\xa\xFF"));
  PASS();
}

TEST(t_parse_escape_hex_twodig_invalid_two)
{
  PROPAGATE(re__parse_test_err("\\xt"));
  PASS();
}

TEST(t_parse_escape_hex_twodig_invalid_three)
{
  PROPAGATE(re__parse_test_err("\\xat"));
  PASS();
}

TEST(t_parse_escape_hex_twodig)
{
  re reg;
  ASSERT_ERR_NOMEM(
      re_init(&reg, "\\x01\\x23\\x45\\x67\\x89\\xAB\\xCD\\xEF\\xab\\xcd\\xef"),
      error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "   (str \"\\x01\\x23\\x45\\x67\\x89\\xAB\\xCD\\xEF\\xAB\\xCD\\xEF\"))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_hex_bracketed_eof)
{
  PROPAGATE(re__parse_test_err("\\x{"));
  PASS();
}

TEST(t_parse_escape_hex_bracketed_invalid_one)
{
  PROPAGATE(re__parse_test_err("\\x{\xFF"));
  PASS();
}

TEST(t_parse_escape_hex_bracketed_invalid_two)
{
  PROPAGATE(re__parse_test_err("\\x{t"));
  PASS();
}

TEST(t_parse_escape_hex_bracketed_empty)
{
  PROPAGATE(re__parse_test_err("\\x{}"));
  PASS();
}

TEST(t_parse_escape_hex_bracketed_toobig)
{
  PROPAGATE(re__parse_test_err("\\x{110000}"));
  PASS();
}

TEST(t_parse_escape_hex_bracketed)
{
  PROPAGATE(re__parse_test(
      "\\x{ABCD}", "(ast"
                   "   (rune 0xABCD))"));
  PASS();
}

TEST(t_parse_escape_absolute_text_end)
{
  PROPAGATE(re__parse_test(
      "\\z", "(ast"
             "   (assert (text_end_absolute)))"));
  PASS();
}

TEST(t_parse_escape_absolute_text_end_in_charclass)
{
  PROPAGATE(re__parse_test_err("[\\z]"));
  PASS();
}

TEST(t_parse_text_end)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "$"), error);
  ASSERT_SYMEQm(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "   (assert (text_end_absolute)))",
      "$ should create a text_end_absolute ast");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_text_end_multiline)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "(?m)$"), error);
  ASSERT_SYMEQm(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "   (assert (text_end)))",
      "$ should create a text_end ast when multiline flag is in effect");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_text_start)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "^"), error);
  ASSERT_SYMEQm(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "   (assert (text_start_absolute)))",
      "^ should create a text_start_absolute ast");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_text_start_multiline)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "(?m)^"), error);
  ASSERT_SYMEQm(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "   (assert (text_start)))",
      "^ should create a text_start ast when multiline flag is in effect");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_group)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "(a)"), error);
  ASSERT_SYMEQm(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "   (group () 0 (rune 'a')))",
      "group should create a group");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_group_unfinished)
{
  PROPAGATE(re__parse_test_err("("));
  PASS();
}

TEST(t_parse_group_unfinished_invalid)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "(\xff"), "error for invalid byte after group start",
      error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_group_unfinished_question_invalid)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "(?\xff"),
      "error for invalid byte after group question start", error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_group_flags_inline)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "(?U)a*"), error);
  ASSERT_SYMEQm(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (quantifier 0 inf nongreedy"
      "    (rune 'a')))",
      "inline flag should apply to outer nodes");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_group_flags_negated)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "(?U)a*(?-U)a*"), error);
  ASSERT_SYMEQm(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (concat ("
      "    (quantifier 0 inf nongreedy"
      "      (rune 'a'))"
      "    (quantifier 0 inf greedy"
      "      (rune 'a')))))",
      "inline negated flag should undo previously set flag");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_group_flags_nonmatching)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "(?:a)"), error);
  ASSERT_SYMEQm(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (group (nonmatching)"
      "    (rune 'a')))",
      ": should set group to be nonmatching");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_group_flags_ungreedy)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "(?U)a*"), error);
  ASSERT_SYMEQm(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (quantifier 0 inf nongreedy"
      "    (rune 'a')))",
      "ungreedy flag should make quantifiers not greedy");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_group_flags_ungreedy_negated)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "(?U)a*(?-U)a*"), error);
  ASSERT_SYMEQm(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (concat ("
      "    (quantifier 0 inf nongreedy"
      "      (rune 'a'))"
      "    (quantifier 0 inf greedy"
      "      (rune 'a')))))",
      "negated ungreedy flag should make quantifiers greedy");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_group_flags_case_insensitive)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "(?i)a"), error);
  ASSERT_SYMEQm(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (charclass ("
      "    (rune_range 'A' 'A')"
      "    (rune_range 'a' 'a'))))",
      "case insensitive flag should create a charclass");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_group_flags_case_insensitive_negated)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "(?i)a(?-i)a"), error);
  ASSERT_SYMEQm(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (concat ("
      "    (charclass ("
      "      (rune_range 'A' 'A')"
      "      (rune_range 'a' 'a')))"
      "    (rune 'a'))))",
      "negated case insensitive flag should create bare characters");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_group_flags_multiline)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "(?m)^"), error);
  ASSERT_SYMEQm(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (assert (text_start)))",
      "multiline flag should create an assert with text_start value");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_group_flags_multiline_negated)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "(?m)^(?-m)^"), error);
  ASSERT_SYMEQm(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (concat ("
      "    (assert (text_start))"
      "    (assert (text_start_absolute)))))",
      "negated multiline flag should create text_start_absolute assert");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_group_flags_stream)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "(?s)."), error);
  ASSERT_SYMEQm(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (any_char_newline))",
      "stream flag should create an any_char_newline node");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_group_flags_stream_negated)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "(?s).(?-s)."), error);
  ASSERT_SYMEQm(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (concat ("
      "    (any_char_newline)"
      "    (any_char))))",
      "negated stream flag should create any_char node");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_group_flags_invalid)
{
  PROPAGATE(re__parse_test_err("(?Q"));
  PASS();
}

TEST(t_parse_group_named_angle)
{
  PROPAGATE(re__parse_test(
      "(?<name>a)", "(ast"
                    "  (group (named) 0 name"
                    "    (rune 'a')))"));
  PASS();
}

TEST(t_parse_group_named_angle_invalid)
{
  PROPAGATE(re__parse_test_err("(?<\xff"));
  PASS();
}

TEST(t_parse_group_named_angle_invalid_second)
{
  PROPAGATE(re__parse_test_err("(?<a\xff"));
  PASS();
}

TEST(t_parse_group_named_angle_unfinished)
{
  PROPAGATE(re__parse_test_err("(?<"));
  PASS();
}

TEST(t_parse_group_named_angle_unfinished_second)
{
  PROPAGATE(re__parse_test_err("(?<a"));
  PASS();
}

TEST(t_parse_group_named_angle_empty)
{
  PROPAGATE(re__parse_test_err("(?<>"));
  PASS();
}

TEST(t_parse_group_named_p)
{
  PROPAGATE(re__parse_test(
      "(?P<name>a)", "(ast"
                     "  (group (named) 0 name"
                     "    (rune 'a')))"));
  PASS();
}

TEST(t_parse_group_named_p_invalid)
{
  PROPAGATE(re__parse_test_err("(?P\xff"));
  PASS();
}

TEST(t_parse_group_named_p_unfinished)
{
  PROPAGATE(re__parse_test_err("(?P"));
  PASS();
}

TEST(t_parse_group_unmatched)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEMm(re_init(&reg, ")"), "error for unmatched )", error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "a)"), "error for unmatched ) after contents", error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "())"), "error for unmatched ) after group", error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "(aa((()a))))"),
      "error for unmatched ) after complex nested groups", error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_groups)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "(a)(b)"), error);
  ASSERT_SYMEQm(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "    (concat"
      "        ("
      "            (group () 0 (rune 'a'))"
      "            (group () 1 (rune 'b')))))",
      "two adjacent groups should create a concat with two groups");
  re_destroy(&reg);
  ASSERT_ERR_NOMEM(re_init(&reg, "(a)(b)(c)"), error);
  ASSERT_SYMEQm(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "    (concat"
      "        ("
      "            (group () 0 (rune 'a'))"
      "            (group () 1 (rune 'b'))"
      "            (group () 2 (rune 'c')))))",
      "three adjacent groups should create a concat with three groups");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_group_balance)
{
  mn__str regs;
  int balance = 0;
  int stack = 0;
  int iters = 0;
  mn__str_init(&regs);
  while (iters < 100) {
    int choice = RAND_PARAM(16);
    if (choice < 5) {
      ASSERT_ERR_NOMEM(mn__str_cat_n(&regs, "(", 1), error);
      stack++;
    } else if (choice < 10) {
      ASSERT_ERR_NOMEM(mn__str_cat_n(&regs, ")", 1), error);
      if (stack == 0) {
        balance = -1;
      } else {
        stack--;
      }
    } else if (choice < 15) {
      mn_char ch = (mn_char)RAND_PARAM(26) + 'A';
      ASSERT_ERR_NOMEM(mn__str_cat_n(&regs, &ch, 1), error);
    } else {
      break;
    }
    iters++;
  }
  if (balance == 0) {
    if (stack) {
      balance = 1;
    }
  }
  {
    re reg;
    int res = re_init(&reg, mn__str_get_data(&regs));
    if (res == RE_ERROR_NOMEM) {
      re_destroy(&reg);
      goto error;
    }
    if (balance != 0) {
      ASSERT_EQm(res, RE_ERROR_PARSE, "error for arbitrary unbalanced group");
    } else {
      ASSERT_EQm(res, 0, "error for arbitrary balanced group");
    }
    re_destroy(&reg);
  }
error:
  mn__str_destroy(&regs);
  PASS();
}

TEST(t_parse_star)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "a*"), error);
  ASSERT_SYMEQm(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "   (quantifier 0 inf greedy"
      "       (rune 'a')))",
      "* operator should wrap previous nodes with quantifier");
  re_destroy(&reg);
  ASSERT_ERR_NOMEM(re_init(&reg, "a*?"), error);
  ASSERT_SYMEQm(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "   (quantifier 0 inf nongreedy"
      "       (rune 'a')))",
      "*? operator should non-greedily wrap previous nodes with quantifier");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_star_nothing)
{
  PROPAGATE(re__parse_test_err("*"));
  PASS();
}

TEST(t_parse_question)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "a?"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "   (quantifier 0 2 greedy"
      "       (rune 'a')))");
  re_destroy(&reg);
  ASSERT_ERR_NOMEM(re_init(&reg, "a??"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "   (quantifier 0 2 nongreedy"
      "       (rune 'a')))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_question_nothing)
{
  PROPAGATE(re__parse_test_err("?"));
  PASS();
}

TEST(t_parse_plus)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "a+"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "   (quantifier 1 inf greedy"
      "       (rune 'a')))");
  re_destroy(&reg);
  ASSERT_ERR_NOMEM(re_init(&reg, "a+?"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "   (quantifier 1 inf nongreedy"
      "       (rune 'a')))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_plus_nothing)
{
  PROPAGATE(re__parse_test_err("+"));
  PASS();
}

TEST(t_parse_invalid_after_quantifier)
{
  PROPAGATE(re__parse_test_err("a*\xFF"));
  PASS();
}

TEST(t_parse_any_char)
{
  PROPAGATE(re__parse_test(
      ".", "(ast"
           "   (any_char))"));
  PASS();
}

TEST(t_parse_charclass_one)
{
  PROPAGATE(re__parse_test(
      "[a]", "(ast"
             "   (rune 'a'))"));
  PASS();
}

TEST(t_parse_charclass_one_fold)
{
  PROPAGATE(re__parse_test(
      "(?i)[a]", "(ast"
                 "  (charclass ("
                 "    (rune_range 'A' 'A')"
                 "    (rune_range 'a' 'a'))))"));
  PASS();
}

TEST(t_parse_charclass_unfinished_invalid)
{
  PROPAGATE(re__parse_test_err("[\xFF"));
  PASS();
}

TEST(t_parse_charclass_unfinished_eof)
{
  PROPAGATE(re__parse_test_err("["));
  PASS();
}

TEST(t_parse_charclass_ascii)
{
  PROPAGATE(re__parse_test(
      "[[:alnum:]]", "(ast"
                     "  (charclass ("
                     "    (rune_range '0' '9')"
                     "    (rune_range 'A' 'Z')"
                     "    (rune_range 'a' 'z'))))"));
  PASS();
}

TEST(t_parse_escape_charclass_ascii_case_insensitive)
{
  PROPAGATE(re__parse_test(
      "(?i)\\w", "(ast"
                 "  (charclass ("
                 "    (rune_range '0' '9')"
                 "    (rune_range 'A' 'Z')"
                 "    (rune_range '_' '_')"
                 "    (rune_range 'a' 'z')"
                 "    (rune_range 0x017F 0x017F)"
                 "    (rune_range 0x212A 0x212A))))"));
  PASS();
}

TEST(t_parse_charclass_lbracket)
{
  PROPAGATE(re__parse_test(
      "[[]", "(ast"
             "   (rune '['))"));
  PASS();
}

TEST(t_parse_charclass_rbracket)
{
  PROPAGATE(re__parse_test(
      "[]]", "(ast"
             "   (rune ']'))"));
  PASS();
}

TEST(t_parse_charclass_hyphen_only)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "[-]"), error);
  ASSERT_SYMEQm(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "   (rune '-'))",
      "charclass with only hyphen should just include a hyphen rune");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_charclass_hyphen_after_rune)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "[a-]"), error);
  ASSERT_SYMEQm(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "   (charclass ("
      "       (rune_range '-' '-')"
      "       (rune_range 'a' 'a'))))",
      "charclass ending with hyphen should include a hyphen rune");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_charclass_hyphen_after_range)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "[a-z-]"), error);
  ASSERT_SYMEQm(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "   (charclass ("
      "       (rune_range '-' '-')"
      "       (rune_range 'a' 'z'))))",
      "charclass ending with hyphen should include a hyphen rune");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_charclass_range)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "[a-z]"), error);
  ASSERT_SYMEQm(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "   (charclass ("
      "       (rune_range 'a' 'z'))))",
      "charclass with middle hyphen should include a rune range");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_charclass_invalid_after_invert)
{
  PROPAGATE(re__parse_test_err("[^\xFF"));
  PASS();
}

TEST(t_parse_charclass_invalid_after_lbracket)
{
  PROPAGATE(re__parse_test_err("[[\xFF"));
  PASS();
}

TEST(t_parse_charclass_eof_after_lbracket)
{
  PROPAGATE(re__parse_test_err("[["));
  PASS();
}

TEST(t_parse_charclass_escape_after_lbracket)
{
  PROPAGATE(re__parse_test_err("[[\\a[\\a[\\a[\\a"));
  PASS();
}

TEST(t_parse_charclass_invalid_after_ascii_start)
{
  PROPAGATE(re__parse_test_err("[[:\xFF"));
  PASS();
}

TEST(t_parse_charclass_invalid_after_ascii_start_inverted)
{
  PROPAGATE(re__parse_test_err("[[:^\xFF"));
  PASS();
}

TEST(t_parse_charclass_eof_after_ascii_start)
{
  PROPAGATE(re__parse_test_err("[[:"));
  PASS();
}

TEST(t_parse_charclass_invalid_one_after_ascii_end)
{
  PROPAGATE(re__parse_test_err("[[:alnum:\xFF"));
  PASS();
}

TEST(t_parse_charclass_eof_after_ascii_end)
{
  PROPAGATE(re__parse_test_err("[[:alnum:"));
  PASS();
}

TEST(t_parse_charclass_invalid_two_after_ascii_end)
{
  PROPAGATE(re__parse_test_err("[[:alnum:t"));
  PASS();
}

TEST(t_parse_charclass_badname)
{
  PROPAGATE(re__parse_test_err("[[:grumbre:]]"));
  PASS();
}

TEST(t_parse_charclass_lbracket_start_range)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "[[-a]"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast "
      "  (charclass ("
      "    (rune_range '[' 'a'))))")
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_charclass_lbracket_single)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "[[a]"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast "
      "  (charclass ("
      "    (rune_range '[' '[')"
      "    (rune_range 'a' 'a'))))")
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_charclass_invalid_before_dash)
{
  PROPAGATE(re__parse_test_err("[[a\xFF"));
  PASS();
}

TEST(t_parse_charclass_eof_before_dash)
{
  PROPAGATE(re__parse_test_err("[[a"));
  PASS();
}

TEST(t_parse_charclass_invalid_after_dash)
{
  PROPAGATE(re__parse_test_err("[a-\xFF"));
  PASS();
}

TEST(t_parse_charclass_eof_after_dash)
{
  PROPAGATE(re__parse_test_err("[a-"));
  PASS();
}

TEST(t_parse_charclass_end_escape)
{
  PROPAGATE(re__parse_test(
      "[a-\\a]", "(ast "
                 "  (charclass ("
                 "    (rune_range 7 'a'))))"));
  PASS();
}

TEST(t_parse_err_charclass_unfinished)
{
#if 0
  re reg;
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "["), "error if charclass is missing ]", error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[["), "error if charclass with open bracket is missing ]",
      error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[]"), "error if charclass with close bracket is missing ]",
      error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[^"), "error if charclass with caret is missing ]", error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[\\"), "error if charclass with escape is missing ]",
      error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[^\\"),
      "error if charclass with caret and escape is missing ]", error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[\\a"), "error if charclass with escape char is missing ]",
      error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[^\\a"),
      "error if charclass with caret and escape is missing ]", error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[a"), "error if charclass with letter is missing ]",
      error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[a-"),
      "error if charclass with unfinished range is missing ]", error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[a-z"), "error if charclass with range is missing ]",
      error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[ab-z"),
      "error if charclass with letter and range is missing ]", error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[a-yz"),
      "error if charclass with range and letter is missing ]", error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[^ab-z"),
      "error if negated charclass with letter and range is missing ]", error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[^a-yz"),
      "error if negated charclass with range and letter is missing ]", error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[[:alnum:]"),
      "error if charclass with named charclass is missing ]", error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[[:"), "error if empty named charclass is missing :]",
      error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[[:^"), "error if named inverted charclass is missing :]",
      error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[[:aa"), "error if named charclass is missing :]", error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[[:aa:"), "error if named charclass is missing ]", error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[[::"), "error if named charclass is missing ]", error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[[::]"), "error if named charclass is missing ]", error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[[:^aa:"), "error if named charclass is missing ]", error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[[:^aa"), "error if named inverted charclass is missing ]",
      error);
error:
  re_destroy(&reg);
#endif
  PASS();
}

TEST(t_parse_charclass_inverted)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "[^a]"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast "
      "    (charclass ((rune_range 0 96) (rune_range 98 0x10FFFF))))");
  re_destroy(&reg);
  ASSERT_ERR_NOMEM(re_init(&reg, "[^a-z]"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast "
      "    (charclass ((rune_range 0 96) (rune_range 123 0x10FFFF))))");
  re_destroy(&reg);
  ASSERT_ERR_NOMEM(re_init(&reg, "[^a-zA-Z]"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast "
      "    (charclass ((rune_range 0 64)"
      "                (rune_range 91 96)"
      "                (rune_range 123 0x10FFFF))))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_charclass_named)
{
  PROPAGATE(re__parse_test(
      "[[:alnum:]]", "(ast "
                     "  (charclass "
                     "    ("
                     "      (rune_range 48 57)"
                     "      (rune_range 65 90)"
                     "      (rune_range 97 122))))"));
  PASS();
}

TEST(t_parse_charclass_named_inverted)
{
  PROPAGATE(re__parse_test(
      "[[:^alnum:]]", "(ast "
                      "  (charclass "
                      "    ("
                      "      (rune_range 0 47)"
                      "      (rune_range 58 64)"
                      "      (rune_range 91 96)"
                      "      (rune_range 123 0x10FFFF))))"));
  PASS();
}

TEST(t_parse_charclass_double_inverted)
{
  PROPAGATE(re__parse_test(
      "[^\\W]", "(ast (charclass "
                "   ((rune_range '0' '9')"
                "    (rune_range 'A' 'Z')"
                "    (rune_range '_' '_')"
                "    (rune_range 'a' 'z'))))"));
  PASS();
}

TEST(t_parse_charclass_reversed)
{
  PROPAGATE(re__parse_test(
      "[z-a]", "(ast "
               "  (charclass "
               "    ("
               "      (rune_range 97 122))))"));
  PASS();
}

TEST(t_parse_charclass_case_insensitive)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "(?i)[[:upper:]]"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast "
      "  (charclass ("
      "    (rune_range 'A' 'Z')"
      "    (rune_range 'a' 'z')"
      "    (rune_range 0x017F 0x017F)"      /* LATIN SMALL LETTER LONG S */
      "    (rune_range 0x212A 0x212A))))"); /* KELVIN SIGN */
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_charclass_eof_after_range)
{
  PROPAGATE(re__parse_test_err("[a-z"));
  PASS();
}

TEST(t_parse_word_boundary)
{
  PROPAGATE(re__parse_test(
      "\\b", "(ast "
             "  (assert (word)))"));
  PASS();
}

TEST(t_parse_word_boundary_not)
{
  PROPAGATE(re__parse_test(
      "\\B", "(ast "
             "  (assert (word_not)))"));
  PASS();
}

TEST(t_parse_opt_fuse_rune_rune)
{
  re reg;
  re_rune first = re_rune_rand();
  re_rune second = re_rune_rand();
  mn__str in_str;
  mn_uint8 utf8_bytes[32];
  int utf8_bytes_ptr = 0;
  utf8_bytes_ptr += re__compile_gen_utf8(first, utf8_bytes + utf8_bytes_ptr);
  utf8_bytes_ptr += re__compile_gen_utf8(second, utf8_bytes + utf8_bytes_ptr);
  utf8_bytes[utf8_bytes_ptr] = '\0';
  ASSERT_ERR_NOMEM(
      mn__str_init_n(&in_str, (mn_char*)utf8_bytes, (mn_size)utf8_bytes_ptr),
      error);
  ASSERT_ERR_NOMEM(re_init(&reg, mn__str_get_data(&in_str)), error);
  {
    re__ast* ast =
        re__ast_root_get(&reg.data->ast_root, reg.data->ast_root.root_ref);
    mn__str_view a, b;
    ASSERT(ast->type == RE__AST_TYPE_STR);
    a = re__ast_root_get_str_view(&reg.data->ast_root, ast->_data.str_ref);
    mn__str_view_init(&b, &in_str);
    ASSERT(mn__str_view_cmp(&a, &b) == 0);
  }
error:
  re_destroy(&reg);
  mn__str_destroy(&in_str);
  PASS();
}

TEST(t_parse_opt_fuse_str_rune)
{
  re reg;
  int n = (int)RAND_PARAM(25) + 2;
  int i;
  mn__str in_str;
  mn__str_init(&in_str);
  for (i = 0; i < n; i++) {
    mn_uint8 utf8_bytes[32];
    int utf8_bytes_ptr = 0;
    re_rune r = re_rune_rand();
    utf8_bytes_ptr += re__compile_gen_utf8(r, utf8_bytes + utf8_bytes_ptr);
    ASSERT_ERR_NOMEM(
        mn__str_cat_n(&in_str, (mn_char*)utf8_bytes, (mn_size)utf8_bytes_ptr),
        error_str_only);
  }
  ASSERT_ERR_NOMEM(re_init(&reg, mn__str_get_data(&in_str)), error);
  {
    re__ast* ast =
        re__ast_root_get(&reg.data->ast_root, reg.data->ast_root.root_ref);
    mn__str_view a, b;
    ASSERT(ast->type == RE__AST_TYPE_STR);
    a = re__ast_root_get_str_view(&reg.data->ast_root, ast->_data.str_ref);
    mn__str_view_init(&b, &in_str);
    ASSERT(mn__str_view_cmp(&a, &b) == 0);
  }
error:
  re_destroy(&reg);
error_str_only:
  mn__str_destroy(&in_str);
  PASS();
}

TEST(t_parse_end_maybe_question)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init_sz_flags(&reg, "(\x00\x00?", 4, 0), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_alt)
{
  PROPAGATE(re__parse_test(
      "a|b", "(ast "
             "    (alt"
             "        ((rune 0x61)"
             "         (rune 0x62))))"));
  PASS();
}

TEST(t_parse_alt_in_group)
{
  PROPAGATE(re__parse_test(
      "(a|b)", "(ast "
               "  (group () 0 "
               "    (alt"
               "      ((rune 0x61)"
               "       (rune 0x62)))))"));
  PASS();
}

/* Needed to hit a weird edge case with OOM errors. */
TEST(t_parse_alt_in_group_thrash)
{
  re reg;
  mn__str expr;
  int i, j;
  for (i = 2; i < 16; i++) {
    mn__str_init(&expr);
    if (mn__str_cat_n(&expr, "(", 1)) {
      goto error_str;
    }
    for (j = 0; j < i; j++) {
      char c = 'A' + (char)j;
      if (mn__str_cat_n(&expr, &c, 1)) {
        goto error_str;
      }
      if (mn__str_cat_n(&expr, "|", 1)) {
        goto error_str;
      }
    }
    if (mn__str_cat_n(&expr, ")", 1)) {
      goto error_str;
    }
    ASSERT_ERR_NOMEM(re_init(&reg, mn__str_get_data(&expr)), error);
    mn__str_destroy(&expr);
    re_destroy(&reg);
  }
  PASS();
error:
  re_destroy(&reg);
error_str:
  mn__str_destroy(&expr);
  PASS();
}

TEST(t_parse_alt_after_concat)
{
  PROPAGATE(re__parse_test(
      "a[bc]|d[ef]", "(ast "
                     "  (alt ("
                     "    (concat ("
                     "      (rune 'a')"
                     "      (charclass ("
                     "        (rune_range 'b' 'c')))))"
                     "    (concat ("
                     "      (rune 'd')"
                     "      (charclass ("
                     "        (rune_range 'e' 'f'))))))))"));
  PASS();
}

TEST(t_parse_err_alt_nostartend)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "|"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast "
      "    (alt ("
      "       (concat ())"
      "       (concat ()))))");
  re_destroy(&reg);
  ASSERT_ERR_NOMEM(re_init(&reg, "(|)"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast "
      "    (group () 0"
      "        (alt ("
      "           (concat ())"
      "           (concat ())))))");
  re_destroy(&reg);
  ASSERT_ERR_NOMEM(re_init(&reg, "|a|"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast "
      "    (alt ("
      "       (concat ())"
      "       (rune 0x61)"
      "       (concat ()))))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_err_alt_nostart)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "|a"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast "
      "    (alt ("
      "       (concat ())"
      "       (rune 0x61))))");
  re_destroy(&reg);
  ASSERT_ERR_NOMEM(re_init(&reg, "|a|a"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast "
      "    (alt ("
      "       (concat ())"
      "       (rune 0x61)"
      "       (rune 0x61))))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_err_alt_noend)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "a|"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast "
      "    (alt ("
      "       (rune 0x61)"
      "       (concat ())))))");
  re_destroy(&reg);
  ASSERT_ERR_NOMEM(re_init(&reg, "a|a|"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast "
      "    (alt ("
      "       (rune 0x61)"
      "       (rune 0x61)"
      "       (concat ())))))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_utf_valid_1)
{
  PROPAGATE(re__parse_test(
      "$", "(ast"
           "    (assert (text_end_absolute)))"));
  PASS();
}

TEST(t_parse_utf_valid_2)
{
  PROPAGATE(re__parse_test(
      "\xc2\xa3", "(ast"
                  "    (rune 163))"));
  PASS();
}

TEST(t_parse_utf_valid_3)
{
  PROPAGATE(re__parse_test(
      "\xe0\xa4\xb9", "(ast"
                      "    (rune 2361))"));
  PASS();
}

TEST(t_parse_utf_valid_4)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, EX_UTF8_VALID_4), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "    (rune 66376))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_utf_invalid_overlong)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, EX_UTF8_INVALID_OVERLONG), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_utf_invalid_surrogate)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, EX_UTF8_INVALID_SURROGATE), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_utf_invalid_toobig)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, EX_UTF8_INVALID_TOOBIG), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_utf_invalid_unfinished)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, EX_UTF8_INVALID_UNFINISHED), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_utf_invalid_unfinished_overlong)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(
      re_init(&reg, EX_UTF8_INVALID_UNFINISHED_OVERLONG), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_utf_invalid_undefined)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, EX_UTF8_INVALID_UNDEFINED), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_count_nothing)
{
  PROPAGATE(re__parse_test_err("{0,2}"));
  PASS();
}

TEST(t_parse_count_invalid_one)
{
  PROPAGATE(re__parse_test_err("a{\xFF"));
  PASS();
}

TEST(t_parse_count_comma_early)
{
  PROPAGATE(re__parse_test_err("a{,}"));
  PASS();
}

TEST(t_parse_count_empty)
{
  PROPAGATE(re__parse_test_err("a{}"));
  PASS();
}

TEST(t_parse_count_invalid_two)
{
  PROPAGATE(re__parse_test_err("a{-"));
  PASS();
}

TEST(t_parse_count_lower_zero)
{
  PROPAGATE(re__parse_test(
      "a{0,}", "(ast"
               "  (quantifier 0 inf greedy"
               "    (rune 'a')))"));
  PASS();
}

TEST(t_parse_count_lower_many)
{
  PROPAGATE(re__parse_test(
      "a{5,}", "(ast"
               "  (quantifier 5 inf greedy"
               "    (rune 'a')))"));
  PASS();
}

TEST(t_parse_count_lower_exact_zero)
{
  PROPAGATE(re__parse_test(
      "a{0}", "(ast"
              "  (quantifier 0 1 greedy"
              "    (rune 'a')))"));
  PASS();
}

TEST(t_parse_count_lower_exact_many)
{
  PROPAGATE(re__parse_test(
      "a{123}", "(ast"
                "  (quantifier 123 124 greedy"
                "    (rune 'a')))"));
  PASS();
}

TEST(t_parse_count_lower_invalid_one)
{
  PROPAGATE(re__parse_test_err("a{4\xFF"));
  PASS();
}

TEST(t_parse_count_lower_invalid_two)
{
  PROPAGATE(re__parse_test_err("a{6&"));
  PASS();
}

TEST(t_parse_count_lower_toobig)
{
  PROPAGATE(re__parse_test_err("a{2001,}"));
  PASS();
}

TEST(t_parse_count_upper_invalid_one)
{
  PROPAGATE(re__parse_test_err("a{78,\xFF"));
  PASS();
}

TEST(t_parse_count_upper)
{
  PROPAGATE(re__parse_test(
      "a{78,90}", "(ast"
                  "  (quantifier 78 91 greedy"
                  "    (rune 'a')))"));
  PASS();
}

TEST(t_parse_count_upper_invalid_two)
{
  PROPAGATE(re__parse_test_err("a{78,9\xFF"));
  PASS();
}

TEST(t_parse_count_upper_invalid_three)
{
  PROPAGATE(re__parse_test_err("a{78,9&"));
  PASS();
}

TEST(t_parse_count_upper_toobig)
{
  PROPAGATE(re__parse_test_err("a{1,2002}"));
  PASS();
}

TEST(t_parse_count_swapped)
{
  PROPAGATE(re__parse_test_err("a{5,2}"));
  PASS();
}

TEST(t_parse_count_equal)
{
  PROPAGATE(re__parse_test(
      "a{5,5}", "(ast"
                "  (quantifier 5 6 greedy"
                "    (rune 'a')))"));
  PASS();
}

SUITE(s_parse)
{
  RUN_TEST(t_parse_empty);
  RUN_TEST(t_parse_rune);
  RUN_TEST(t_parse_rune_case_insensitive_fold);
  RUN_TEST(t_parse_rune_case_insensitive_nofold);
  RUN_TEST(t_parse_escape_unfinished);
  RUN_TEST(t_parse_escape_invalid);
  RUN_TEST(t_parse_escape_invalid_char);
  RUN_TEST(t_parse_escape_literal_hi);
  RUN_TEST(t_parse_escape_literal_lo);
  RUN_TEST(t_parse_escape_octal_one);
  RUN_TEST(t_parse_escape_octal_two);
  RUN_TEST(t_parse_escape_octal_three);
  RUN_TEST(t_parse_escape_octal_invalid);
  RUN_TEST(t_parse_escape_text_start_absolute);
  RUN_TEST(t_parse_escape_text_start_absolute_in_charclass);
  RUN_TEST(t_parse_escape_assert_word_not);
  RUN_TEST(t_parse_escape_assert_word_not_in_charclass);
  RUN_TEST(t_parse_escape_any_byte);
  RUN_TEST(t_parse_escape_any_byte_in_charclass);
  RUN_TEST(t_parse_escape_digit_not);
  RUN_TEST(t_parse_escape_digit_not_in_charclass);
  RUN_TEST(t_parse_escape_digit_not_ending_range);
  RUN_TEST(t_parse_escape_quote_end_unmatched);
  RUN_TEST(t_parse_escape_unicode_property_ending_range);
  RUN_TEST(t_parse_escape_unicode_property_invalid_one);
  RUN_TEST(t_parse_escape_unicode_property_invalid_two);
  RUN_TEST(t_parse_escape_unicode_property_empty_name);
  RUN_TEST(t_parse_escape_unicode_property_invalid_three);
  RUN_TEST(t_parse_escape_unicode_property_invalid_four);
  RUN_TEST(t_parse_escape_unicode_property_unfinished);
  RUN_TEST(t_parse_escape_unicode_property_valid);
  RUN_TEST(t_parse_escape_unicode_property_valid_in_charclass);
  RUN_TEST(t_parse_escape_unicode_property_valid_inverted);
  RUN_TEST(t_parse_escape_unicode_property_valid_inverted_in_charclass);
  RUN_TEST(t_parse_escape_unicode_property_valid_case_insensitive);
  RUN_TEST(t_parse_escape_quote_empty);
  RUN_TEST(t_parse_escape_quote_empty_unfinished);
  RUN_TEST(t_parse_escape_quote_text);
  RUN_TEST(t_parse_escape_quote_invalid);
  RUN_TEST(t_parse_escape_quote_text_unfinished);
  RUN_TEST(t_parse_escape_quote_escape);
  RUN_TEST(t_parse_escape_quote_escape_slash);
  RUN_TEST(t_parse_escape_quote_escape_unfinished);
  RUN_TEST(t_parse_escape_quote_in_charclass);
  RUN_TEST(t_parse_escape_perlspace_not);
  RUN_TEST(t_parse_escape_perlspace_not_in_charclass);
  RUN_TEST(t_parse_escape_perlspace_not_ending_range);
  RUN_TEST(t_parse_escape_word_not);
  RUN_TEST(t_parse_escape_word_not_in_charclass);
  RUN_TEST(t_parse_escape_word_not_ending_range);
  RUN_TEST(t_parse_escape_bell);
  RUN_TEST(t_parse_escape_bell_in_charclass);
  RUN_TEST(t_parse_escape_assert_word);
  RUN_TEST(t_parse_escape_assert_word_in_charclass);
  RUN_TEST(t_parse_escape_digit);
  RUN_TEST(t_parse_escape_digit_in_charclass);
  RUN_TEST(t_parse_escape_digit_ending_range);
  RUN_TEST(t_parse_escape_form_feed);
  RUN_TEST(t_parse_escape_form_feed_in_charclass);
  RUN_TEST(t_parse_escape_newline);
  RUN_TEST(t_parse_escape_newline_in_charclass);
  RUN_TEST(t_parse_escape_carriage_return);
  RUN_TEST(t_parse_escape_carriage_return_in_charclass);
  RUN_TEST(t_parse_escape_perlspace);
  RUN_TEST(t_parse_escape_perlspace_in_charclass);
  RUN_TEST(t_parse_escape_perlspace_ending_range);
  RUN_TEST(t_parse_escape_horizontal_tab);
  RUN_TEST(t_parse_escape_horizontal_tab_in_charclass);
  RUN_TEST(t_parse_escape_vertical_tab);
  RUN_TEST(t_parse_escape_vertical_tab_in_charclass);
  RUN_TEST(t_parse_escape_word);
  RUN_TEST(t_parse_escape_word_in_charclass);
  RUN_TEST(t_parse_escape_word_ending_range);
  RUN_TEST(t_parse_escape_hex_invalid);
  RUN_TEST(t_parse_escape_hex_eof);
  RUN_TEST(t_parse_escape_hex_twodig_invalid_one);
  RUN_TEST(t_parse_escape_hex_twodig_invalid_two);
  RUN_TEST(t_parse_escape_hex_twodig_invalid_three);
  RUN_TEST(t_parse_escape_hex_twodig);
  RUN_TEST(t_parse_escape_hex_bracketed_eof);
  RUN_TEST(t_parse_escape_hex_bracketed_invalid_one);
  RUN_TEST(t_parse_escape_hex_bracketed_invalid_two);
  RUN_TEST(t_parse_escape_hex_bracketed_empty);
  RUN_TEST(t_parse_escape_hex_bracketed_toobig);
  RUN_TEST(t_parse_escape_hex_bracketed);
  RUN_TEST(t_parse_escape_absolute_text_end);
  RUN_TEST(t_parse_escape_absolute_text_end_in_charclass);
  RUN_TEST(t_parse_escape_charclass_ascii_case_insensitive);
  RUN_TEST(t_parse_text_end);
  RUN_TEST(t_parse_text_end_multiline);
  RUN_TEST(t_parse_text_start);
  RUN_TEST(t_parse_text_start_multiline);
  RUN_TEST(t_parse_group);
  RUN_TEST(t_parse_group_unfinished);
  RUN_TEST(t_parse_group_unmatched);
  RUN_TEST(t_parse_group_unfinished_invalid);
  RUN_TEST(t_parse_group_unfinished_question_invalid);
  RUN_TEST(t_parse_group_flags_inline);
  RUN_TEST(t_parse_group_flags_negated);
  RUN_TEST(t_parse_group_flags_nonmatching);
  RUN_TEST(t_parse_group_flags_ungreedy);
  RUN_TEST(t_parse_group_flags_ungreedy_negated);
  RUN_TEST(t_parse_group_flags_case_insensitive);
  RUN_TEST(t_parse_group_flags_case_insensitive_negated);
  RUN_TEST(t_parse_group_flags_multiline);
  RUN_TEST(t_parse_group_flags_multiline_negated);
  RUN_TEST(t_parse_group_flags_stream);
  RUN_TEST(t_parse_group_flags_stream_negated);
  RUN_TEST(t_parse_group_flags_invalid);
  FUZZ_TEST(t_parse_group_balance);
  RUN_TEST(t_parse_group_named_angle);
  RUN_TEST(t_parse_group_named_angle_invalid);
  RUN_TEST(t_parse_group_named_angle_invalid_second);
  RUN_TEST(t_parse_group_named_angle_unfinished);
  RUN_TEST(t_parse_group_named_angle_unfinished_second);
  RUN_TEST(t_parse_group_named_angle_empty);
  RUN_TEST(t_parse_group_named_p);
  RUN_TEST(t_parse_group_named_p_invalid);
  RUN_TEST(t_parse_group_named_p_unfinished);
  RUN_TEST(t_parse_groups);
  RUN_TEST(t_parse_star);
  RUN_TEST(t_parse_star_nothing);
  RUN_TEST(t_parse_question);
  RUN_TEST(t_parse_question_nothing);
  RUN_TEST(t_parse_plus);
  RUN_TEST(t_parse_plus_nothing);
  RUN_TEST(t_parse_invalid_after_quantifier);
  RUN_TEST(t_parse_any_char);
  RUN_TEST(t_parse_charclass_one);
  RUN_TEST(t_parse_charclass_one_fold);
  RUN_TEST(t_parse_charclass_unfinished_invalid);
  RUN_TEST(t_parse_charclass_unfinished_eof);
  RUN_TEST(t_parse_charclass_ascii);
  RUN_TEST(t_parse_charclass_lbracket);
  RUN_TEST(t_parse_charclass_rbracket);
  RUN_TEST(t_parse_charclass_hyphen_only);
  RUN_TEST(t_parse_charclass_hyphen_after_rune);
  RUN_TEST(t_parse_charclass_hyphen_after_range);
  RUN_TEST(t_parse_charclass_range);
  RUN_TEST(t_parse_charclass_invalid_after_invert);
  RUN_TEST(t_parse_charclass_invalid_after_lbracket);
  RUN_TEST(t_parse_charclass_escape_after_lbracket);
  RUN_TEST(t_parse_charclass_eof_after_lbracket);
  RUN_TEST(t_parse_charclass_invalid_after_ascii_start);
  RUN_TEST(t_parse_charclass_invalid_after_ascii_start_inverted);
  RUN_TEST(t_parse_charclass_eof_after_ascii_start);
  RUN_TEST(t_parse_charclass_invalid_one_after_ascii_end);
  RUN_TEST(t_parse_charclass_eof_after_ascii_end);
  RUN_TEST(t_parse_charclass_invalid_two_after_ascii_end);
  RUN_TEST(t_parse_charclass_badname);
  RUN_TEST(t_parse_charclass_lbracket_start_range);
  RUN_TEST(t_parse_charclass_lbracket_single);
  RUN_TEST(t_parse_charclass_invalid_before_dash);
  RUN_TEST(t_parse_charclass_eof_before_dash);
  RUN_TEST(t_parse_charclass_invalid_after_dash);
  RUN_TEST(t_parse_charclass_eof_after_dash);
  RUN_TEST(t_parse_charclass_end_escape);
  RUN_TEST(t_parse_err_charclass_unfinished);
  RUN_TEST(t_parse_charclass_inverted);
  RUN_TEST(t_parse_charclass_named);
  RUN_TEST(t_parse_charclass_named_inverted);
  RUN_TEST(t_parse_charclass_double_inverted);
  RUN_TEST(t_parse_charclass_reversed);
  RUN_TEST(t_parse_charclass_case_insensitive);
  RUN_TEST(t_parse_charclass_eof_after_range);
  RUN_TEST(t_parse_word_boundary);
  RUN_TEST(t_parse_word_boundary_not);
  FUZZ_TEST(t_parse_opt_fuse_rune_rune);
  FUZZ_TEST(t_parse_opt_fuse_str_rune);
  RUN_TEST(t_parse_end_maybe_question);
  RUN_TEST(t_parse_alt);
  RUN_TEST(t_parse_alt_after_concat);
  RUN_TEST(t_parse_alt_in_group);
  RUN_TEST(t_parse_alt_in_group_thrash);
  RUN_TEST(t_parse_err_alt_nostartend);
  RUN_TEST(t_parse_err_alt_nostart);
  RUN_TEST(t_parse_utf_valid_1);
  RUN_TEST(t_parse_utf_valid_2);
  RUN_TEST(t_parse_utf_valid_3);
  RUN_TEST(t_parse_utf_valid_4);
  RUN_TEST(t_parse_utf_invalid_overlong);
  RUN_TEST(t_parse_utf_invalid_surrogate);
  RUN_TEST(t_parse_utf_invalid_toobig);
  RUN_TEST(t_parse_utf_invalid_unfinished);
  RUN_TEST(t_parse_utf_invalid_unfinished_overlong);
  RUN_TEST(t_parse_utf_invalid_undefined);
  RUN_TEST(t_parse_count_nothing);
  RUN_TEST(t_parse_count_invalid_one);
  RUN_TEST(t_parse_count_comma_early);
  RUN_TEST(t_parse_count_empty);
  RUN_TEST(t_parse_count_invalid_two);
  RUN_TEST(t_parse_count_lower_zero);
  RUN_TEST(t_parse_count_lower_many);
  RUN_TEST(t_parse_count_lower_exact_zero);
  RUN_TEST(t_parse_count_lower_exact_many);
  RUN_TEST(t_parse_count_lower_invalid_one);
  RUN_TEST(t_parse_count_lower_invalid_two);
  RUN_TEST(t_parse_count_lower_toobig);
  RUN_TEST(t_parse_count_upper_invalid_one);
  RUN_TEST(t_parse_count_upper);
  RUN_TEST(t_parse_count_upper_invalid_two);
  RUN_TEST(t_parse_count_upper_invalid_three);
  RUN_TEST(t_parse_count_upper_toobig);
  RUN_TEST(t_parse_count_swapped);
  RUN_TEST(t_parse_count_equal);
}

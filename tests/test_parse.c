#include "test_parse.h"

#include "test_ast.h"
#include "test_helpers.h"

TEST(t_parse_empty)
{
  re reg;
  ASSERT_ERR_NOMEMm(re_init(&reg, ""), "empty regex should compile", error);
  ASSERT_SYMEQm(
      re__ast_root, reg.data->ast_root, "(ast)",
      "empty regex should return an empty ast");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_rune)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "a"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "    (rune 'a'))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_rune_case_insensitive_fold)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "(?i)a"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (charclass ("
      "    (rune_range 'A' 'A')"
      "    (rune_range 'a' 'a'))))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_rune_case_insensitive_nofold)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "(?i)&"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (rune '&'))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_invalid)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "\\\xff"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_octal_one)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "\\0\\1\\2\\3\\4\\5\\6\\7"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (str \"\\x00\\x01\\x02\\x03\\x04\\x05\\x06\\x07\"))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_octal_two)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "\\01\\12\\23\\34\\45\\56\\67\\70"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (str \"\\x01\\x0a\\x13\\x1c\\x25\\x2e\\x37\\x38\"))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_octal_three)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "\\141\\142"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (str ab))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_octal_invalid)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "\\0\xFF"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_text_start_absolute)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "\\A"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (assert (text_start_absolute)))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_text_start_absolute_in_charclass)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "[\\A]"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_assert_word_not)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "\\B"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (assert (word_not)))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_assert_word_not_in_charclass)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "[\\B]"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_any_byte)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "\\C"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (any_byte))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_any_byte_in_charclass)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "[\\C]"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_digit_not)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "\\D"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (charclass ("
      "    (rune_range 0 47)"
      "    (rune_range 58 0x10FFFF))))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_digit_not_in_charclass)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "[\\D]"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (charclass ("
      "    (rune_range 0 47)"
      "    (rune_range 58 0x10FFFF))))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_digit_not_ending_range)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "[a-\\D]"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_quote_end_unmatched)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "\\E"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_quote_empty)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "\\Q\\E"), error);
  ASSERT_SYMEQ(re__ast_root, reg.data->ast_root, "(ast)");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_quote_empty_unfinished)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "\\Q"), error);
  ASSERT_SYMEQ(re__ast_root, reg.data->ast_root, "(ast)");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_quote_text)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "\\Qabc\\E"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "    (str \"abc\"))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_quote_invalid)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "\\Q\xFF"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_quote_text_unfinished)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "\\Qabc"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "    (str \"abc\"))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_quote_escape)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "\\Q\\\\\\E"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "    (rune 0x5C))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_quote_escape_slash)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "\\Q\\\\"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "    (rune 0x5C))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_quote_escape_unfinished)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "\\Q\\"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_quote_in_charclass)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "[\\Q]"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_unicode_property)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "\\P"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_perlspace_not)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "\\S"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (charclass ("
      "    (rune_range 0 8)"
      "    (rune_range 11 11)"
      "    (rune_range 14 31)"
      "    (rune_range 33 0x10FFFF))))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_perlspace_not_in_charclass)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "[\\S]"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (charclass ("
      "    (rune_range 0 8)"
      "    (rune_range 11 11)"
      "    (rune_range 14 31)"
      "    (rune_range 33 0x10FFFF))))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_perlspace_not_ending_range)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "[a-\\S]"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_word_not)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "\\W"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (charclass ("
      "    (rune_range 0 47)"
      "    (rune_range 58 64)"
      "    (rune_range 91 94)"
      "    (rune_range 96 96)"
      "    (rune_range 123 0x10FFFF))))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_word_not_in_charclass)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "[\\W]"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (charclass ("
      "    (rune_range 0 47)"
      "    (rune_range 58 64)"
      "    (rune_range 91 94)"
      "    (rune_range 96 96)"
      "    (rune_range 123 0x10FFFF))))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_word_not_ending_range)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "[a-\\W]"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_bell)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "\\a"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (rune 7))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_bell_in_charclass)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "[\\a]"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (rune 7))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_assert_word)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "\\b"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (assert (word)))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_assert_word_in_charclass)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "[\\b]"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_digit)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "\\d"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (charclass ("
      "    (rune_range '0' '9'))))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_digit_in_charclass)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "[\\d]"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (charclass ("
      "    (rune_range '0' '9'))))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_digit_ending_range)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "[a-\\d]"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_form_feed)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "\\f"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (rune 0xC))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_form_feed_in_charclass)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "[\\f]"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (rune 0xC))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_newline)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "\\n"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (rune 0xA))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_newline_in_charclass)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "[\\n]"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (rune 0xA))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_carriage_return)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "\\r"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (rune 0xD))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_carriage_return_in_charclass)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "[\\r]"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (rune 0xD))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_perlspace)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "\\s"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (charclass ("
      "    (rune_range 9 10)"
      "    (rune_range 12 13)"
      "    (rune_range 32 32))))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_perlspace_in_charclass)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "[\\s]"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (charclass ("
      "    (rune_range 9 10)"
      "    (rune_range 12 13)"
      "    (rune_range 32 32))))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_perlspace_ending_range)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "[a-\\s]"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_horizontal_tab)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "\\t"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (rune 0x9))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_horizontal_tab_in_charclass)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "[\\t]"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (rune 0x9))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_vertical_tab)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "\\v"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (rune 0xB))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_vertical_tab_in_charclass)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "[\\v]"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (rune 0xB))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_word)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "\\w"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (charclass ("
      "    (rune_range '0' '9')"
      "    (rune_range 'A' 'Z')"
      "    (rune_range '_' '_')"
      "    (rune_range 'a' 'z'))))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_word_in_charclass)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "[\\w]"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (charclass ("
      "    (rune_range '0' '9')"
      "    (rune_range 'A' 'Z')"
      "    (rune_range '_' '_')"
      "    (rune_range 'a' 'z'))))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_word_ending_range)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "[a-\\w]"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_hex_invalid)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "\\x\xFF"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_hex_eof)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "\\x"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_hex_twodig_invalid_one)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "\\xa\xFF"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_hex_twodig_invalid_two)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "\\xt"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_hex_twodig_invalid_three)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "\\xat"), error);
error:
  re_destroy(&reg);
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
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "\\x{"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_hex_bracketed_invalid_one)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "\\x{\xFF"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_hex_bracketed_invalid_two)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "\\x{t"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_hex_bracketed_empty)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "\\x{}"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_hex_bracketed_toobig)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "\\x{110000}"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_hex_bracketed)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "\\x{ABCD}"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "   (rune 0xABCD))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_absolute_text_end)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "\\z"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "   (assert (text_end_absolute)))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_escape_absolute_text_end_in_charclass)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "[\\z]"), error);
error:
  re_destroy(&reg);
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
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "("), error);
error:
  re_destroy(&reg);
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

TEST(t_parse_group_named_angle)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "(?<name>a)"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (group (named) 0 name"
      "    (rune 'a')))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_group_named_angle_invalid)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "(?<\xff"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_group_named_angle_invalid_second)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "(?<a\xff"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_group_named_angle_unfinished)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "(?<"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_group_named_angle_unfinished_second)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "(?<a"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_group_named_angle_empty)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "(?<>"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_group_named_p)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "(?P<name>a)"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (group (named) 0 name"
      "    (rune 'a')))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_group_named_p_invalid)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "(?P\xff"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_group_named_p_unfinished)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "(?P"), error);
error:
  re_destroy(&reg);
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

TEST(t_parse_err_star_nostart)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "*"), error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "(*)"), error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "(?:*)"), error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "*?"), error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "(*?)"), error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "(?:*?)"), error);
error:
  re_destroy(&reg);
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

TEST(t_parse_err_question_nostart)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "?"), error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "(?:?)"), error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "??"), error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEM(
      re_init(
          &reg, "(?"
                "?)"),
      error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEM(
      re_init(
          &reg, "(?:?"
                "?)"),
      error);
error:
  re_destroy(&reg);
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

TEST(t_parse_err_plus_nostart)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "+"), error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "(+)"), error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "(?:+)"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_invalid_after_quantifier)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "a*\xFF"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_any_char)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "."), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "   (any_char))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_charclass_one)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "[a]"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "   (rune 'a'))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_charclass_one_fold)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "(?i)[a]"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (charclass ("
      "    (rune_range 'A' 'A')"
      "    (rune_range 'a' 'a'))))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_charclass_unfinished_invalid)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "[\xFF"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_charclass_unfinished_eof)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "["), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_charclass_ascii)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "[[:alnum:]]"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "  (charclass ("
      "    (rune_range '0' '9')"
      "    (rune_range 'A' 'Z')"
      "    (rune_range 'a' 'z'))))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_charclass_lbracket)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "[[]"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "   (rune '['))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_charclass_rbracket)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "[]]"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "   (rune ']'))");
error:
  re_destroy(&reg);
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
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "[^\xFF"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_charclass_invalid_after_lbracket)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "[[\xFF"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_charclass_eof_after_lbracket)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "[["), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_charclass_escape_after_lbracket)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "[[\\a[\\a[\\a[\\a"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_charclass_invalid_after_ascii_start)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "[[:\xFF"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_charclass_invalid_after_ascii_start_inverted)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "[[:^\xFF"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_charclass_eof_after_ascii_start)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "[[:"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_charclass_invalid_one_after_ascii_end)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "[[:alnum:\xFF"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_charclass_eof_after_ascii_end)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "[[:alnum:"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_charclass_invalid_two_after_ascii_end)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "[[:alnum:t"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_charclass_badname)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "[[:grumbre:]]"), error);
error:
  re_destroy(&reg);
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
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "[[a\xFF"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_charclass_eof_before_dash)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "[[a"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_charclass_invalid_after_dash)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "[a-\xFF"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_charclass_eof_after_dash)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEM(re_init(&reg, "[a-"), error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_charclass_end_escape)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "[a-\\a]"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast "
      "  (charclass ("
      "    (rune_range 7 'a'))))");
error:
  re_destroy(&reg);
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
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "[[:alnum:]]"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast "
      "  (charclass "
      "    ("
      "      (rune_range 48 57)"
      "      (rune_range 65 90)"
      "      (rune_range 97 122))))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_charclass_named_inverted)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "[[:^alnum:]]"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast "
      "  (charclass "
      "    ("
      "      (rune_range 0 47)"
      "      (rune_range 58 64)"
      "      (rune_range 91 96)"
      "      (rune_range 123 0x10FFFF))))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_charclass_double_inverted)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "[^\\W]"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast (charclass "
      "   ((rune_range '0' '9')"
      "    (rune_range 'A' 'Z')"
      "    (rune_range '_' '_')"
      "    (rune_range 'a' 'z'))))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_charclass_reversed)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "[z-a]"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast "
      "  (charclass "
      "    ("
      "      (rune_range 97 122))))");
error:
  re_destroy(&reg);
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
      "    (rune_range 'a' 'z'))))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_word_boundary)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "\\b"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast "
      "  (assert (word)))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_word_boundary_not)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "\\B"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast "
      "  (assert (word_not)))");
error:
  re_destroy(&reg);
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
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "a|b"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast "
      "    (alt"
      "        ((rune 0x61)"
      "         (rune 0x62))))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_alt_in_group)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "(a|b)"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast "
      "  (group () 0 "
      "    (alt"
      "      ((rune 0x61)"
      "       (rune 0x62)))))");
error:
  re_destroy(&reg);
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
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "a[bc]|d[ef]"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast "
      "  (alt ("
      "    (concat ("
      "      (rune 'a')"
      "      (charclass ("
      "        (rune_range 'b' 'c')))))"
      "    (concat ("
      "      (rune 'd')"
      "      (charclass ("
      "        (rune_range 'e' 'f'))))))))");
error:
  re_destroy(&reg);
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
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "$"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "    (assert (text_end_absolute)))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_utf_valid_2)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "\xc2\xa3"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "    (rune 163))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_utf_valid_3)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "\xe0\xa4\xb9"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "    (rune 2361))");
error:
  re_destroy(&reg);
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

SUITE(s_parse)
{
  RUN_TEST(t_parse_empty);
  RUN_TEST(t_parse_rune);
  RUN_TEST(t_parse_rune_case_insensitive_fold);
  RUN_TEST(t_parse_rune_case_insensitive_nofold);
  RUN_TEST(t_parse_escape_invalid);
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
  RUN_TEST(t_parse_escape_unicode_property);
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
  RUN_TEST(t_parse_err_star_nostart);
  RUN_TEST(t_parse_question);
  RUN_TEST(t_parse_err_question_nostart);
  RUN_TEST(t_parse_plus);
  RUN_TEST(t_parse_err_plus_nostart);
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
}

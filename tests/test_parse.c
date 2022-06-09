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

TEST(t_parse_text_end)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "$"), error);
  ASSERT_SYMEQm(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "   (assert (text_end)))",
      "$ should create a text_end ast");
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
  re_destroy(&reg);
error:
  PASS();
}

TEST(t_parse_err_group_unfinished)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "("), "error for unfinished group", error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "(a"), "error for unfinished group with contents", error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "(a("), "error for unfinished nested group", error);
  re_destroy(&reg);
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "(a(b(())("), "error for unfinished complex nested group",
      error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_err_group_unfinished_invalid)
{
  re reg;
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "(\xff"), "error for invalid byte after group start",
      error);
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_err_group_unfinished_question_invalid)
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
TEST(t_parse_err_group_unmatched)
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
      "            (group () 1 (rune 'b'))))",
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
      "            (group () 2 (rune 'c'))))",
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

TEST(t_parse_group_named)
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
      "   (charclass ((rune_range 'a' 'a'))))");
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
      "   (charclass ((rune_range '[' '['))))");
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
      "   (charclass ((rune_range ']' ']'))))");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_charclass_hyphen)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "[-]"), error);
  ASSERT_SYMEQm(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "   (charclass ((rune_range '-' '-'))))",
      "charclass with only hyphen should just include a hyphen rune");
  re_destroy(&reg);
  ASSERT_ERR_NOMEM(re_init(&reg, "[a-]"), error);
  ASSERT_SYMEQm(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "   (charclass ("
      "       (rune_range '-' '-')"
      "       (rune_range 'a' 'a'))))",
      "charclass ending with hyphen should include a hyphen rune");
  re_destroy(&reg);
  ASSERT_ERR_NOMEM(re_init(&reg, "[a-z-]"), error);
  ASSERT_SYMEQm(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "   (charclass ("
      "       (rune_range '-' '-')"
      "       (rune_range 'a' 'z'))))",
      "charclass ending with hyphen should include a hyphen rune");
  re_destroy(&reg);
  ASSERT_ERR_NOMEM(re_init(&reg, "[a-z]"), error);
  ASSERT_SYMEQm(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "   (charclass ("
      "       (rune_range 'a' 'z')))",
      "charclass with middle hyphen should include a rune range");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_err_charclass_unfinished)
{
  re reg;
  /* RE__PARSE_STATE_CHARCLASS_INITIAL */
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "["), "error if charclass is missing ]", error);
  re_destroy(&reg);
  /* RE__PARSE_STATE_CHARCLASS_AFTER_BRACKET */
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[["), "error if charclass with open bracket is missing ]",
      error);
  re_destroy(&reg);
  /* RE__PARSE_STATE_CHARCLASS_AFTER_LO */
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[]"), "error if charclass with close bracket is missing ]",
      error);
  re_destroy(&reg);
  /* RE__PARSE_STATE_CHARCLASS_AFTER_CARET */
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[^"), "error if charclass with caret is missing ]", error);
  re_destroy(&reg);
  /* RE__PARSE_STATE_CHARCLASS_AFTER_ESCAPE */
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[\\"), "error if charclass with escape is missing ]",
      error);
  re_destroy(&reg);
  /* RE__PARSE_STATE_CHARCLASS_AFTER_ESCAPE */
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[^\\"),
      "error if charclass with caret and escape is missing ]", error);
  re_destroy(&reg);
  /* RE__PARSE_STATE_CHARCLASS_AFTER_LO */
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[\\a"), "error if charclass with escape char is missing ]",
      error);
  re_destroy(&reg);
  /* RE__PARSE_STATE_CHARCLASS_AFTER_LO */
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[^\\a"),
      "error if charclass with caret and escape is missing ]", error);
  re_destroy(&reg);
  /* RE__PARSE_STATE_CHARCLASS_AFTER_LO */
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[a"), "error if charclass with letter is missing ]",
      error);
  re_destroy(&reg);
  /* RE__PARSE_STATE_CHARCLASS_HI */
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[a-"),
      "error if charclass with unfinished range is missing ]", error);
  re_destroy(&reg);
  /* RE__PARSE_STATE_CHARCLASS_LO */
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[a-z"), "error if charclass with range is missing ]",
      error);
  re_destroy(&reg);
  /* RE__PARSE_STATE_CHARCLASS_LO */
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[ab-z"),
      "error if charclass with letter and range is missing ]", error);
  re_destroy(&reg);
  /* RE__PARSE_STATE_CHARCLASS_AFTER_LO */
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[a-yz"),
      "error if charclass with range and letter is missing ]", error);
  re_destroy(&reg);
  /* RE__PARSE_STATE_CHARCLASS_LO */
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[^ab-z"),
      "error if negated charclass with letter and range is missing ]", error);
  re_destroy(&reg);
  /* RE__PARSE_STATE_CHARCLASS_LO */
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[^a-yz"),
      "error if negated charclass with range and letter is missing ]", error);
  re_destroy(&reg);
  /* RE__PARSE_STATE_CHARCLASS_LO */
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[[:alnum:]"),
      "error if charclass with named charclass is missing ]", error);
  re_destroy(&reg);
  /* RE__PARSE_STATE_CHARCLASS_NAMED_INITIAL */
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[[:"), "error if empty named charclass is missing :]",
      error);
  re_destroy(&reg);
  /* RE__PARSE_STATE_CHARCLASS_NAMED_INVERTED */
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[[:^"), "error if named inverted charclass is missing :]",
      error);
  re_destroy(&reg);
  /* RE__PARSE_STATE_CHARCLASS_NAMED */
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[[:aa"), "error if named charclass is missing :]", error);
  re_destroy(&reg);
  /* RE__PARSE_STATE_CHARCLASS_NAMED */
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[[:aa:"), "error if named charclass is missing ]", error);
  re_destroy(&reg);
  /* RE__PARSE_STATE_CHARCLASS_NAMED */
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[[::"), "error if named charclass is missing ]", error);
  re_destroy(&reg);
  /* RE__PARSE_STATE_CHARCLASS_NAMED */
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[[::]"), "error if named charclass is missing ]", error);
  re_destroy(&reg);
  /* RE__PARSE_STATE_CHARCLASS_NAMED_INVERTED */
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[[:^aa:"), "error if named charclass is missing ]", error);
  re_destroy(&reg);
  /* RE__PARSE_STATE_CHARCLASS_NAMED_INVERTED */
  ASSERT_PARSE_ERR_NOMEMm(
      re_init(&reg, "[[:^aa"), "error if named inverted charclass is missing ]",
      error);
error:
  re_destroy(&reg);
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
      "      (rune_range 97 122)))");
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
      "      (rune_range 123 0x10FFFF))");
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
      "    (rune_range 'a' 'z')))");
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
      "         (rune 0x62)))");
error:
  re_destroy(&reg);
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
      "        (rune_range 'e' 'f'))))))");
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

TEST(t_parse_quote_empty)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "\\Q\\E"), error);
  ASSERT_SYMEQ(re__ast_root, reg.data->ast_root, "(ast)");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_quote_empty_unfinished)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "\\Q"), error);
  ASSERT_SYMEQ(re__ast_root, reg.data->ast_root, "(ast)");
error:
  re_destroy(&reg);
  PASS();
}

TEST(t_parse_quote_text)
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

TEST(t_parse_quote_text_unfinished)
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

TEST(t_parse_quote_escape)
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

TEST(t_parse_quote_escape_unfinished)
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

TEST(t_parse_utf_valid_1)
{
  re reg;
  ASSERT_ERR_NOMEM(re_init(&reg, "$"), error);
  ASSERT_SYMEQ(
      re__ast_root, reg.data->ast_root,
      "(ast"
      "    (assert (text_end)))");
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
  RUN_TEST(t_parse_text_end);
  RUN_TEST(t_parse_group);
  RUN_TEST(t_parse_err_group_unfinished);
  RUN_TEST(t_parse_err_group_unmatched);
  RUN_TEST(t_parse_err_group_unfinished_invalid);
  RUN_TEST(t_parse_err_group_unfinished_question_invalid);
  RUN_TEST(t_parse_group_flags_inline);
  RUN_TEST(t_parse_group_flags_negated);
  RUN_TEST(t_parse_group_flags_nonmatching);
  RUN_TEST(t_parse_group_flags_ungreedy);
  RUN_TEST(t_parse_group_flags_ungreedy_negated);
  RUN_TEST(t_parse_group_flags_case_insensitive);
  RUN_TEST(t_parse_group_flags_case_insensitive_negated);
  FUZZ_TEST(t_parse_group_balance);
  RUN_TEST(t_parse_group_named);
  RUN_TEST(t_parse_groups);
  RUN_TEST(t_parse_star);
  RUN_TEST(t_parse_err_star_nostart);
  RUN_TEST(t_parse_question);
  RUN_TEST(t_parse_err_question_nostart);
  RUN_TEST(t_parse_plus);
  RUN_TEST(t_parse_err_plus_nostart);
  RUN_TEST(t_parse_any_char);
  RUN_TEST(t_parse_charclass_one);
  RUN_TEST(t_parse_charclass_rbracket);
  RUN_TEST(t_parse_charclass_hyphen);
  RUN_TEST(t_parse_err_charclass_unfinished);
  RUN_TEST(t_parse_charclass_inverted);
  RUN_TEST(t_parse_charclass_named);
  RUN_TEST(t_parse_charclass_named_inverted);
  RUN_TEST(t_parse_charclass_double_inverted);
  RUN_TEST(t_parse_charclass_reversed);
  RUN_TEST(t_parse_word_boundary);
  RUN_TEST(t_parse_word_boundary_not);
  FUZZ_TEST(t_parse_opt_fuse_rune_rune);
  FUZZ_TEST(t_parse_opt_fuse_str_rune);
  RUN_TEST(t_parse_end_maybe_question);
  RUN_TEST(t_parse_alt);
  RUN_TEST(t_parse_alt_after_concat);
  RUN_TEST(t_parse_err_alt_nostartend);
  RUN_TEST(t_parse_err_alt_nostart);
  RUN_TEST(t_parse_quote_empty);
  RUN_TEST(t_parse_quote_empty_unfinished);
  RUN_TEST(t_parse_quote_text);
  RUN_TEST(t_parse_quote_text_unfinished);
  RUN_TEST(t_parse_quote_escape);
  RUN_TEST(t_parse_quote_escape_unfinished);
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

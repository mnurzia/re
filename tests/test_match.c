#include "test_match.h"

#include "test_harness.h"

static mptest__result test_match(
    const char* regex, const char* text, re_anchor_type anchor_type,
    int bound_start, int bound_end)
{
  mn__str_view text_view;
  re reg;
  re_error rv = 0;
  re_error rv2 = 0;
  re_span out_groups;
  mn__str_view_init_s(&text_view, text);
  rv = re_init(&reg, regex);
  if (rv == RE_ERROR_NOMEM) {
    re_destroy(&reg);
    return RE_ERROR_NOMEM;
  } else if (rv != 0) {
    return RE_ERROR_INVALID;
  }
  rv = re_is_match(&reg, text, mn__str_view_size(&text_view), anchor_type);
  if (rv != RE_MATCH) {
    if (rv == RE_ERROR_NOMEM) {
      re_destroy(&reg);
      return RE_ERROR_NOMEM;
    }
    if (rv != RE_NOMATCH) {
      return RE_ERROR_INVALID;
    }
  }
  rv2 = re_match_groups(
      &reg, text, mn__str_view_size(&text_view), anchor_type, 1, &out_groups);
  if (rv2 == RE_ERROR_NOMEM) {
    re_destroy(&reg);
    return RE_ERROR_NOMEM;
  }
  if (rv != rv2) {
    return RE_ERROR_INVALID;
  }
  if (rv2 != RE_MATCH) {
    if (rv != RE_NOMATCH) {
      return RE_ERROR_INVALID;
    }
    if (bound_start != -1) {
      return RE_ERROR_INVALID;
    }
    if (bound_end != -1) {
      return RE_ERROR_INVALID;
    }
  } else {
    if ((int)out_groups.begin != bound_start) {
      return RE_ERROR_INVALID;
    }
    if ((int)out_groups.end != bound_end) {
      return RE_ERROR_INVALID;
    }
  }
  re_destroy(&reg);
  return rv == RE_MATCH;
}

#define ASSERT_MATCH(a)                                                        \
  do {                                                                         \
    re_error _err = RE_ERROR_NONE;                                             \
    if ((_err = a) == 0) {                                                     \
      FAIL();                                                                  \
    } else if (_err == RE_ERROR_NOMEM) {                                       \
    } else if (_err != 1) {                                                    \
      FAIL();                                                                  \
    }                                                                          \
  } while (0)

#define ASSERT_NOMATCH(a)                                                      \
  do {                                                                         \
    re_error _err = RE_ERROR_NONE;                                             \
    if ((_err = a) == 1) {                                                     \
      FAIL();                                                                  \
    } else if (_err == RE_ERROR_NOMEM) {                                       \
    } else if (_err != 0) {                                                    \
      FAIL();                                                                  \
    }                                                                          \
  } while (0)

TEST(t_match_nogroups_one_char_s)
{
  ASSERT_MATCH(test_match("a", "a", 'S', 0, 1));
  ASSERT_MATCH(test_match("a", "abc", 'S', 0, 1));
  ASSERT_NOMATCH(test_match("a", "bc", 'S', -1, -1));
  ASSERT_NOMATCH(test_match("a", "", 'S', -1, -1));
  PASS();
}

TEST(t_match_nogroups_one_char_e)
{
  ASSERT_MATCH(test_match("a", "a", 'E', 0, 1));
  ASSERT_MATCH(test_match("a", "ba", 'E', 1, 2));
  ASSERT_NOMATCH(test_match("a", "bac", 'E', -1, -1));
  ASSERT_NOMATCH(test_match("a", "", 'E', -1, -1));
  PASS();
}

TEST(t_match_nogroups_one_char_b)
{
  ASSERT_MATCH(test_match("a", "a", 'B', 0, 1));
  ASSERT_NOMATCH(test_match("a", "ba", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("a", "ab", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("a", "", 'B', -1, -1));
  PASS();
}

TEST(t_match_nogroups_one_char_u)
{
  ASSERT_MATCH(test_match("a", "a", 'U', 0, 1));
  ASSERT_MATCH(test_match("a", "ba", 'U', 1, 2));
  ASSERT_MATCH(test_match("a", "bac", 'U', 1, 2));
  ASSERT_MATCH(test_match("a", "cab", 'U', 1, 2));
  ASSERT_NOMATCH(test_match("a", "cb", 'U', -1, -1));
  ASSERT_NOMATCH(test_match("a", "", 'U', -1, -1));
  PASS();
}

SUITE(s_match_nogroups_one_char)
{
  RUN_TEST(t_match_nogroups_one_char_s);
  RUN_TEST(t_match_nogroups_one_char_e);
  RUN_TEST(t_match_nogroups_one_char_b);
  RUN_TEST(t_match_nogroups_one_char_u);
}

TEST(t_match_nogroups_two_chars_s)
{
  ASSERT_MATCH(test_match("ab", "ab", 'S', 0, 2));
  ASSERT_MATCH(test_match("ab", "abc", 'S', 0, 2));
  ASSERT_NOMATCH(test_match("ab", "bac", 'S', -1, -1));
  ASSERT_NOMATCH(test_match("ab", "babc", 'S', -1, -1));
  ASSERT_NOMATCH(test_match("ab", "", 'S', -1, -1));
  PASS();
}

TEST(t_match_nogroups_two_chars_e)
{
  ASSERT_MATCH(test_match("ab", "ab", 'E', 0, 2));
  ASSERT_MATCH(test_match("ab", "bab", 'E', 1, 3));
  ASSERT_MATCH(test_match("ab", "cbab", 'E', 2, 4));
  ASSERT_NOMATCH(test_match("ab", "b", 'E', -1, -1));
  ASSERT_NOMATCH(test_match("ab", "ba", 'E', -1, -1));
  ASSERT_NOMATCH(test_match("ab", "", 'E', -1, -1));
  ASSERT_NOMATCH(test_match("ab", "abc", 'E', -1, -1));
  PASS();
}

TEST(t_match_nogroups_two_chars_b)
{
  ASSERT_MATCH(test_match("ab", "ab", 'B', 0, 2));
  ASSERT_NOMATCH(test_match("ab", "bab", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("ab", "abb", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("ab", "abab", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("ab", "a", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("ab", "", 'B', -1, -1));
  PASS();
}

TEST(t_match_nogroups_two_chars_u)
{
  ASSERT_MATCH(test_match("ab", "ab", 'U', 0, 2));
  ASSERT_MATCH(test_match("ab", "abab", 'U', 0, 2));
  ASSERT_MATCH(test_match("ab", "ccabcc", 'U', 2, 4));
  ASSERT_MATCH(test_match("ab", "cbabc", 'U', 2, 4));
  ASSERT_NOMATCH(test_match("ab", "a", 'U', -1, -1));
  ASSERT_NOMATCH(test_match("ab", "", 'U', -1, -1));
  ASSERT_NOMATCH(test_match("ab", "cbac", 'U', -1, -1));
  PASS();
}

SUITE(s_match_nogroups_two_chars)
{
  RUN_TEST(t_match_nogroups_two_chars_s);
  RUN_TEST(t_match_nogroups_two_chars_e);
  RUN_TEST(t_match_nogroups_two_chars_b);
  RUN_TEST(t_match_nogroups_two_chars_u);
}

TEST(t_match_nogroups_alt_s)
{
  ASSERT_MATCH(test_match("a|b", "a", 'S', 0, 1));
  ASSERT_MATCH(test_match("a|b", "b", 'S', 0, 1));
  ASSERT_MATCH(test_match("a|b", "aa", 'S', 0, 1));
  ASSERT_MATCH(test_match("a|b", "ab", 'S', 0, 1));
  ASSERT_MATCH(test_match("a|b", "ba", 'S', 0, 1));
  ASSERT_MATCH(test_match("a|b", "bb", 'S', 0, 1));
  ASSERT_NOMATCH(test_match("a|b", "c", 'S', -1, -1));
  ASSERT_NOMATCH(test_match("a|b", "ca", 'S', -1, -1));
  ASSERT_NOMATCH(test_match("a|b", "caa", 'S', -1, -1));
  ASSERT_NOMATCH(test_match("a|b", "cbb", 'S', -1, -1));
  ASSERT_NOMATCH(test_match("a|b", "", 'S', -1, -1));
  PASS();
}

TEST(t_match_nogroups_alt_e)
{

  ASSERT_MATCH(test_match("a|b", "a", 'E', 0, 1));
  ASSERT_MATCH(test_match("a|b", "b", 'E', 0, 1));
  ASSERT_MATCH(test_match("a|b", "aa", 'E', 1, 2));
  ASSERT_MATCH(test_match("a|b", "ba", 'E', 1, 2));
  ASSERT_MATCH(test_match("a|b", "bb", 'E', 1, 2));
  ASSERT_MATCH(test_match("a|b", "ba", 'E', 1, 2));
  ASSERT_NOMATCH(test_match("a|b", "ac", 'E', -1, -1));
  ASSERT_NOMATCH(test_match("a|b", "bc", 'E', -1, -1));
  ASSERT_NOMATCH(test_match("a|b", "", 'E', -1, -1));
  PASS();
}

TEST(t_match_nogroups_alt_b)
{

  ASSERT_MATCH(test_match("a|b", "a", 'B', 0, 1));
  ASSERT_MATCH(test_match("a|b", "b", 'B', 0, 1));
  ASSERT_NOMATCH(test_match("a|b", "", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("a|b", "c", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("a|b", "ca", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("a|b", "cb", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("a|b", "ac", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("a|b", "bc", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("a|b", "cc", 'B', -1, -1));
  PASS();
}

TEST(t_match_nogroups_alt_u)
{

  ASSERT_MATCH(test_match("a|b", "a", 'U', 0, 1));
  ASSERT_MATCH(test_match("a|b", "b", 'U', 0, 1));
  ASSERT_MATCH(test_match("a|b", "ca", 'U', 1, 2));
  ASSERT_MATCH(test_match("a|b", "cb", 'U', 1, 2));
  ASSERT_MATCH(test_match("a|b", "ac", 'U', 0, 1));
  ASSERT_MATCH(test_match("a|b", "cac", 'U', 1, 2));
  ASSERT_MATCH(test_match("a|b", "cbc", 'U', 1, 2));
  ASSERT_MATCH(test_match("a|b", "cabc", 'U', 1, 2));
  ASSERT_MATCH(test_match("a|b", "cbac", 'U', 1, 2));
  ASSERT_MATCH(test_match("a|b", "ccccbaabc", 'U', 4, 5));
  ASSERT_MATCH(test_match("a|b", "aaccbecc", 'U', 0, 1));
  ASSERT_NOMATCH(test_match("a|b", "", 'U', -1, -1));
  ASSERT_NOMATCH(test_match("a|b", "c", 'U', -1, -1));
  ASSERT_NOMATCH(test_match("a|b", "cd", 'U', -1, -1));
  ASSERT_NOMATCH(test_match("a|b", "cdef", 'U', -1, -1));
  PASS();
}

SUITE(s_match_nogroups_alt)
{
  RUN_TEST(t_match_nogroups_alt_s);
  RUN_TEST(t_match_nogroups_alt_e);
  RUN_TEST(t_match_nogroups_alt_b);
  RUN_TEST(t_match_nogroups_alt_u);
}

TEST(t_match_nogroups_star_s)
{
  ASSERT_MATCH(test_match("a*", "", 'S', 0, 0));
  ASSERT_MATCH(test_match("a*", "a", 'S', 0, 1));
  ASSERT_MATCH(test_match("a*", "aa", 'S', 0, 2));
  ASSERT_MATCH(test_match("a*", "aab", 'S', 0, 2));
  ASSERT_MATCH(test_match("a*", "b", 'S', 0, 0));
  PASS();
}

TEST(t_match_nogroups_star_e)
{
  ASSERT_MATCH(test_match("a*", "", 'E', 0, 0));
  ASSERT_MATCH(test_match("a*", "a", 'E', 0, 1));
  ASSERT_MATCH(test_match("a*", "aa", 'E', 0, 2));
  ASSERT_MATCH(test_match("a*", "aab", 'E', 3, 3));
  ASSERT_MATCH(test_match("a*", "b", 'E', 1, 1));
  PASS();
}

TEST(t_match_nogroups_star_b)
{
  ASSERT_MATCH(test_match("a*", "", 'B', 0, 0));
  ASSERT_MATCH(test_match("a*", "a", 'B', 0, 1));
  ASSERT_MATCH(test_match("a*", "aa", 'B', 0, 2));
  ASSERT_NOMATCH(test_match("a*", "aab", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("a*", "b", 'B', -1, -1));
  PASS();
}

TEST(t_match_nogroups_star_u)
{
  ASSERT_MATCH(test_match("a*", "", 'U', 0, 0));
  ASSERT_MATCH(test_match("a*", "a", 'U', 0, 1));
  ASSERT_MATCH(test_match("a*", "aa", 'U', 0, 2));
  ASSERT_MATCH(test_match("a*", "aab", 'U', 0, 2));
  ASSERT_MATCH(test_match("a*", "b", 'U', 0, 0));
  PASS();
}

SUITE(s_match_nogroups_star)
{
  RUN_TEST(t_match_nogroups_star_s);
  RUN_TEST(t_match_nogroups_star_e);
  RUN_TEST(t_match_nogroups_star_b);
  RUN_TEST(t_match_nogroups_star_u);
}

TEST(t_match_nogroups_plus_s)
{
  ASSERT_MATCH(test_match("a+", "a", 'S', 0, 1));
  ASSERT_MATCH(test_match("a+", "aa", 'S', 0, 2));
  ASSERT_MATCH(test_match("a+", "aab", 'S', 0, 2));
  ASSERT_NOMATCH(test_match("a+", "baa", 'S', -1, -1));
  ASSERT_NOMATCH(test_match("a+", "", 'S', -1, -1));
  ASSERT_NOMATCH(test_match("a+", "b", 'S', -1, -1));
  PASS();
}

TEST(t_match_nogroups_plus_e)
{
  ASSERT_MATCH(test_match("a+", "a", 'E', 0, 1));
  ASSERT_MATCH(test_match("a+", "aa", 'E', 0, 2));
  ASSERT_NOMATCH(test_match("a+", "aab", 'E', -1, -1));
  ASSERT_MATCH(test_match("a+", "baa", 'E', 1, 3));
  ASSERT_NOMATCH(test_match("a+", "", 'E', -1, -1));
  ASSERT_NOMATCH(test_match("a+", "b", 'E', -1, -1));
  PASS();
}

TEST(t_match_nogroups_plus_b)
{
  ASSERT_MATCH(test_match("a+", "a", 'B', 0, 1));
  ASSERT_MATCH(test_match("a+", "aa", 'B', 0, 2));
  ASSERT_NOMATCH(test_match("a+", "aab", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("a+", "baa", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("a+", "", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("a+", "b", 'B', -1, -1));
  PASS();
}

TEST(t_match_nogroups_plus_u)
{
  ASSERT_MATCH(test_match("a+", "a", 'U', 0, 1));
  ASSERT_MATCH(test_match("a+", "aa", 'U', 0, 2));
  ASSERT_MATCH(test_match("a+", "aab", 'U', 0, 2));
  ASSERT_MATCH(test_match("a+", "baa", 'U', 1, 3));
  ASSERT_NOMATCH(test_match("a+", "", 'U', -1, -1));
  ASSERT_NOMATCH(test_match("a+", "b", 'U', -1, -1));
  PASS();
}

SUITE(s_match_nogroups_plus)
{
  RUN_TEST(t_match_nogroups_plus_s);
  RUN_TEST(t_match_nogroups_plus_e);
  RUN_TEST(t_match_nogroups_plus_b);
  RUN_TEST(t_match_nogroups_plus_u);
}

TEST(t_match_nogroups_question_s)
{
  ASSERT_MATCH(test_match("a?", "", 'S', 0, 0));
  ASSERT_MATCH(test_match("a?", "a", 'S', 0, 1));
  ASSERT_MATCH(test_match("a?", "aa", 'S', 0, 1));
  ASSERT_MATCH(test_match("a?", "b", 'S', 0, 0));
  ASSERT_MATCH(test_match("a?", "ba", 'S', 0, 0));
  ASSERT_MATCH(test_match("a?", "ab", 'S', 0, 1));
  ASSERT_MATCH(test_match("a?", "baa", 'S', 0, 0));
  ASSERT_MATCH(test_match("a?", "aab", 'S', 0, 1));
  PASS();
}

TEST(t_match_nogroups_question_e)
{
  ASSERT_MATCH(test_match("a?", "", 'E', 0, 0));
  ASSERT_MATCH(test_match("a?", "a", 'E', 0, 1));
  ASSERT_MATCH(test_match("a?", "aa", 'E', 1, 2));
  ASSERT_MATCH(test_match("a?", "b", 'E', 1, 1));
  ASSERT_MATCH(test_match("a?", "ba", 'E', 1, 2));
  ASSERT_MATCH(test_match("a?", "ab", 'E', 2, 2));
  ASSERT_MATCH(test_match("a?", "baa", 'E', 2, 3));
  ASSERT_MATCH(test_match("a?", "aab", 'E', 3, 3));
  PASS();
}

TEST(t_match_nogroups_question_b)
{
  ASSERT_MATCH(test_match("a?", "", 'B', 0, 0));
  ASSERT_MATCH(test_match("a?", "a", 'B', 0, 1));
  ASSERT_NOMATCH(test_match("a?", "aa", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("a?", "b", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("a?", "ba", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("a?", "ab", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("a?", "baa", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("a?", "aab", 'B', -1, -1));
  PASS();
}

TEST(t_match_nogroups_question_u)
{
  ASSERT_MATCH(test_match("a?", "", 'U', 0, 0));
  ASSERT_MATCH(test_match("a?", "a", 'U', 0, 1));
  ASSERT_MATCH(test_match("a?", "aa", 'U', 0, 1));
  ASSERT_MATCH(test_match("a?", "b", 'U', 0, 0));
  ASSERT_MATCH(test_match("a?", "ba", 'U', 0, 0));
  ASSERT_MATCH(test_match("a?", "ab", 'U', 0, 1));
  ASSERT_MATCH(test_match("a?", "baa", 'U', 0, 0));
  ASSERT_MATCH(test_match("a?", "aab", 'U', 0, 1));
  PASS();
}

SUITE(s_match_nogroups_question)
{
  RUN_TEST(t_match_nogroups_question_s);
  RUN_TEST(t_match_nogroups_question_e);
  RUN_TEST(t_match_nogroups_question_b);
  RUN_TEST(t_match_nogroups_question_u);
}

TEST(t_match_nogroups_quantifier_s)
{
  ASSERT_MATCH(test_match("a{1,2}", "a", 'S', 0, 1));
  ASSERT_MATCH(test_match("a{1,2}", "aa", 'S', 0, 2));
  ASSERT_NOMATCH(test_match("a{1,2}", "", 'S', -1, -1));
  ASSERT_MATCH(test_match("a{1,2}", "ab", 'S', 0, 1));
  ASSERT_MATCH(test_match("a{1,2}", "aab", 'S', 0, 2));
  ASSERT_NOMATCH(test_match("a{1,2}", "ba", 'S', -1, -1));
  ASSERT_NOMATCH(test_match("a{1,2}", "baa", 'S', -1, -1));
  PASS();
}

TEST(t_match_nogroups_quantifier_e)
{
  ASSERT_MATCH(test_match("a{1,2}", "a", 'E', 0, 1));
  ASSERT_MATCH(test_match("a{1,2}", "aa", 'E', 0, 2));
  ASSERT_NOMATCH(test_match("a{1,2}", "", 'E', -1, -1));
  ASSERT_NOMATCH(test_match("a{1,2}", "ab", 'E', -1, -1));
  ASSERT_NOMATCH(test_match("a{1,2}", "aab", 'E', -1, -1));
  ASSERT_MATCH(test_match("a{1,2}", "ba", 'E', 1, 2));
  ASSERT_MATCH(test_match("a{1,2}", "baa", 'E', 1, 3));
  PASS();
}

TEST(t_match_nogroups_quantifier_b)
{
  ASSERT_MATCH(test_match("a{1,2}", "a", 'B', 0, 1));
  ASSERT_MATCH(test_match("a{1,2}", "aa", 'B', 0, 2));
  ASSERT_NOMATCH(test_match("a{1,2}", "", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("a{1,2}", "ab", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("a{1,2}", "aab", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("a{1,2}", "ba", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("a{1,2}", "baa", 'B', -1, -1));
  PASS();
}

TEST(t_match_nogroups_quantifier_u)
{
  ASSERT_MATCH(test_match("a{1,2}", "a", 'U', 0, 1));
  ASSERT_MATCH(test_match("a{1,2}", "aa", 'U', 0, 2));
  ASSERT_NOMATCH(test_match("a{1,2}", "", 'U', -1, -1));
  ASSERT_MATCH(test_match("a{1,2}", "ab", 'U', 0, 1));
  ASSERT_MATCH(test_match("a{1,2}", "aab", 'U', 0, 2));
  ASSERT_MATCH(test_match("a{1,2}", "ba", 'U', 1, 2));
  ASSERT_MATCH(test_match("a{1,2}", "baa", 'U', 1, 3));
  PASS();
}

SUITE(s_match_nogroups_quantifier)
{
  RUN_TEST(t_match_nogroups_quantifier_s);
  RUN_TEST(t_match_nogroups_quantifier_e);
  RUN_TEST(t_match_nogroups_quantifier_b);
  RUN_TEST(t_match_nogroups_quantifier_u);
}

TEST(t_match_nogroups_dot_s)
{
  ASSERT_MATCH(test_match(".", "a", 'S', 0, 1));
  ASSERT_MATCH(test_match(".", "aa", 'S', 0, 1));
  ASSERT_MATCH(test_match(".", "z", 'S', 0, 1));
  ASSERT_MATCH(test_match(".", EX_UTF8_VALID_1, 'S', 0, 1));
  ASSERT_MATCH(test_match(".", EX_UTF8_VALID_2, 'S', 0, 2));
  ASSERT_MATCH(test_match(".", EX_UTF8_VALID_3, 'S', 0, 3));
  ASSERT_MATCH(test_match(".", EX_UTF8_VALID_4, 'S', 0, 4));
  ASSERT_NOMATCH(test_match(".", "", 'S', -1, -1));
  ASSERT_NOMATCH(test_match(".", EX_UTF8_INVALID_OVERLONG, 'S', -1, -1));
  ASSERT_NOMATCH(test_match(".", EX_UTF8_INVALID_TOOBIG, 'S', -1, -1));
  ASSERT_NOMATCH(test_match(".", EX_UTF8_INVALID_SURROGATE, 'S', -1, -1));

  PASS();
}

TEST(t_match_nogroups_dot_e)
{
  ASSERT_MATCH(test_match(".", "a", 'E', 0, 1));
  ASSERT_MATCH(test_match(".", "aa", 'E', 1, 2));
  ASSERT_MATCH(test_match(".", "z", 'E', 0, 1));
  ASSERT_MATCH(test_match(".", EX_UTF8_VALID_1, 'E', 0, 1));
  ASSERT_MATCH(test_match(".", EX_UTF8_VALID_2, 'E', 0, 2));
  ASSERT_MATCH(test_match(".", EX_UTF8_VALID_3, 'E', 0, 3));
  ASSERT_MATCH(test_match(".", EX_UTF8_VALID_4, 'E', 0, 4));
  ASSERT_NOMATCH(test_match(".", "", 'E', -1, -1));
  ASSERT_NOMATCH(test_match(".", EX_UTF8_INVALID_OVERLONG, 'E', -1, -1));
  ASSERT_NOMATCH(test_match(".", EX_UTF8_INVALID_TOOBIG, 'E', -1, -1));
  ASSERT_NOMATCH(test_match(".", EX_UTF8_INVALID_SURROGATE, 'E', -1, -1));
  PASS();
}

TEST(t_match_nogroups_dot_b)
{
  ASSERT_MATCH(test_match(".", "a", 'B', 0, 1));
  ASSERT_NOMATCH(test_match(".", "aa", 'B', -1, -1));
  ASSERT_MATCH(test_match(".", "z", 'B', 0, 1));
  ASSERT_MATCH(test_match(".", EX_UTF8_VALID_1, 'B', 0, 1));
  ASSERT_MATCH(test_match(".", EX_UTF8_VALID_2, 'B', 0, 2));
  ASSERT_MATCH(test_match(".", EX_UTF8_VALID_3, 'B', 0, 3));
  ASSERT_MATCH(test_match(".", EX_UTF8_VALID_4, 'B', 0, 4));
  ASSERT_NOMATCH(test_match(".", "", 'B', -1, -1));
  ASSERT_NOMATCH(test_match(".", EX_UTF8_INVALID_OVERLONG, 'B', -1, -1));
  ASSERT_NOMATCH(test_match(".", EX_UTF8_INVALID_TOOBIG, 'B', -1, -1));
  ASSERT_NOMATCH(test_match(".", EX_UTF8_INVALID_SURROGATE, 'B', -1, -1));
  PASS();
}

TEST(t_match_nogroups_dot_u)
{
  ASSERT_MATCH(test_match(".", "a", 'U', 0, 1));
  ASSERT_MATCH(test_match(".", "aa", 'U', 0, 1));
  ASSERT_MATCH(test_match(".", "z", 'U', 0, 1));
  ASSERT_MATCH(test_match(".", EX_UTF8_VALID_1, 'U', 0, 1));
  ASSERT_MATCH(test_match(".", EX_UTF8_VALID_2, 'U', 0, 2));
  ASSERT_MATCH(test_match(".", EX_UTF8_VALID_3, 'U', 0, 3));
  ASSERT_MATCH(test_match(".", EX_UTF8_VALID_4, 'U', 0, 4));
  ASSERT_NOMATCH(test_match(".", "", 'U', -1, -1));
  ASSERT_NOMATCH(test_match(".", EX_UTF8_INVALID_OVERLONG, 'U', -1, -1));
  ASSERT_NOMATCH(test_match(".", EX_UTF8_INVALID_TOOBIG, 'U', -1, -1));
  ASSERT_NOMATCH(test_match(".", EX_UTF8_INVALID_SURROGATE, 'U', -1, -1));
  PASS();
}

SUITE(s_match_nogroups_dot)
{
  RUN_TEST(t_match_nogroups_dot_s);
  RUN_TEST(t_match_nogroups_dot_e);
  RUN_TEST(t_match_nogroups_dot_b);
  RUN_TEST(t_match_nogroups_dot_u);
}

TEST(t_match_nogroups_begin_text_s)
{
  ASSERT_MATCH(test_match("^a", "a", 'S', 0, 1));
  ASSERT_MATCH(test_match("^a", "aa", 'S', 0, 1));
  ASSERT_MATCH(test_match("^a", "ab", 'S', 0, 1));
  ASSERT_NOMATCH(test_match("^a", "", 'S', -1, -1));
  ASSERT_NOMATCH(test_match("^a", "ba", 'S', -1, -1));
  PASS();
}

TEST(t_match_nogroups_begin_text_e)
{
  ASSERT_MATCH(test_match("^a", "a", 'E', 0, 1));
  ASSERT_NOMATCH(test_match("^a", "aa", 'E', -1, -1));
  ASSERT_NOMATCH(test_match("^a", "ab", 'E', -1, -1));
  ASSERT_NOMATCH(test_match("^a", "", 'E', -1, -1));
  ASSERT_NOMATCH(test_match("^a", "ba", 'E', -1, -1));
  PASS();
}

TEST(t_match_nogroups_begin_text_b)
{
  ASSERT_MATCH(test_match("^a", "a", 'B', 0, 1));
  ASSERT_NOMATCH(test_match("^a", "aa", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("^a", "ab", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("^a", "", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("^a", "ba", 'B', -1, -1));
  PASS();
}

TEST(t_match_nogroups_begin_text_u)
{
  ASSERT_MATCH(test_match("^a", "a", 'U', 0, 1));
  ASSERT_MATCH(test_match("^a", "aa", 'U', 0, 1));
  ASSERT_MATCH(test_match("^a", "ab", 'U', 0, 1));
  ASSERT_NOMATCH(test_match("^a", "", 'U', -1, -1));
  ASSERT_NOMATCH(test_match("^a", "ba", 'U', -1, -1));
  PASS();
}

SUITE(s_match_nogroups_begin_text)
{
  RUN_TEST(t_match_nogroups_begin_text_s);
  RUN_TEST(t_match_nogroups_begin_text_e);
  RUN_TEST(t_match_nogroups_begin_text_b);
  RUN_TEST(t_match_nogroups_begin_text_u);
}

TEST(t_match_nogroups_end_text_s)
{
  ASSERT_MATCH(test_match("a$", "a", 'S', 0, 1));
  ASSERT_NOMATCH(test_match("a$", "aa", 'S', -1, -1));
  ASSERT_NOMATCH(test_match("a$", "ab", 'S', -1, -1));
  ASSERT_NOMATCH(test_match("a$", "", 'S', -1, -1));
  ASSERT_NOMATCH(test_match("a$", "ba", 'S', -1, -1));
  PASS();
}

TEST(t_match_nogroups_end_text_e)
{
  ASSERT_MATCH(test_match("a$", "a", 'E', 0, 1));
  ASSERT_MATCH(test_match("a$", "aa", 'E', 1, 2));
  ASSERT_NOMATCH(test_match("a$", "ab", 'E', -1, -1));
  ASSERT_NOMATCH(test_match("a$", "", 'E', -1, -1));
  ASSERT_MATCH(test_match("a$", "ba", 'E', 1, 2));
  PASS();
}

TEST(t_match_nogroups_end_text_b)
{

  ASSERT_MATCH(test_match("a$", "a", 'B', 0, 1));
  ASSERT_NOMATCH(test_match("a$", "aa", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("a$", "ab", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("a$", "", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("a$", "ba", 'B', -1, -1));
  PASS();
}

TEST(t_match_nogroups_end_text_u)
{
  ASSERT_MATCH(test_match("a$", "a", 'U', 0, 1));
  ASSERT_MATCH(test_match("a$", "aa", 'U', 1, 2));
  ASSERT_NOMATCH(test_match("a$", "ab", 'U', -1, -1));
  ASSERT_NOMATCH(test_match("a$", "", 'U', -1, -1));
  ASSERT_MATCH(test_match("a$", "ba", 'U', 1, 2));
  PASS();
}

SUITE(s_match_nogroups_end_text)
{
  RUN_TEST(t_match_nogroups_end_text_s);
  RUN_TEST(t_match_nogroups_end_text_e);
  RUN_TEST(t_match_nogroups_end_text_b);
  RUN_TEST(t_match_nogroups_end_text_u);
}

TEST(t_match_nogroups_word_bound_s)
{
  ASSERT_NOMATCH(test_match("\\b", "", 'S', -1, -1));
  ASSERT_MATCH(test_match("\\b", "a", 'S', 0, 0));
  ASSERT_NOMATCH(test_match("\\b", " ", 'S', -1, -1));
  ASSERT_NOMATCH(test_match("\\b", "#", 'S', -1, -1));
  ASSERT_MATCH(test_match("a\\b", "a", 'S', 0, 1));
  ASSERT_NOMATCH(test_match("a\\b", "aa", 'S', -1, -1));
  ASSERT_MATCH(test_match("a\\b", "a ", 'S', 0, 1));
  ASSERT_MATCH(test_match("a\\b", "a#", 'S', 0, 1));
  ASSERT_NOMATCH(test_match("a\\b", "", 'S', -1, -1));
  PASS();
}

TEST(t_match_nogroups_word_bound_e)
{
  ASSERT_NOMATCH(test_match("\\b", "", 'E', -1, -1));
  ASSERT_MATCH(test_match("\\b", "a", 'E', 1, 1));
  ASSERT_NOMATCH(test_match("\\b", " ", 'E', -1, -1));
  ASSERT_NOMATCH(test_match("\\b", "#", 'E', -1, -1));
  ASSERT_MATCH(test_match("a\\b", "a", 'E', 0, 1));
  ASSERT_MATCH(test_match("a\\b", "aa", 'E', 1, 2));
  ASSERT_NOMATCH(test_match("a\\b", "a ", 'E', -1, -1));
  ASSERT_NOMATCH(test_match("a\\b", "a#", 'E', -1, -1));
  ASSERT_NOMATCH(test_match("a\\b", "", 'E', -1, -1));
  PASS();
}

TEST(t_match_nogroups_word_bound_b)
{
  ASSERT_NOMATCH(test_match("\\b", "", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("\\b", "a", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("\\b", " ", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("\\b", "#", 'B', -1, -1));
  ASSERT_MATCH(test_match("a\\b", "a", 'B', 0, 1));
  ASSERT_NOMATCH(test_match("a\\b", "aa", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("a\\b", "a ", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("a\\b", "a#", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("a\\b", "", 'B', -1, -1));
  PASS();
}

TEST(t_match_nogroups_word_bound_u)
{
  ASSERT_NOMATCH(test_match("\\b", "", 'U', -1, -1));
  ASSERT_MATCH(test_match("\\b", "a", 'U', 0, 0));
  ASSERT_NOMATCH(test_match("\\b", " ", 'U', -1, -1));
  ASSERT_NOMATCH(test_match("\\b", "#", 'U', -1, -1));
  ASSERT_MATCH(test_match("a\\b", "a", 'U', 0, 1));
  ASSERT_MATCH(test_match("a\\b", "aa", 'U', 1, 2));
  ASSERT_MATCH(test_match("a\\b", "a ", 'U', 0, 1));
  ASSERT_MATCH(test_match("a\\b", "a#", 'U', 0, 1));
  ASSERT_NOMATCH(test_match("a\\b", "", 'U', -1, -1));
  PASS();
}

SUITE(s_match_nogroups_word_bound)
{
  RUN_TEST(t_match_nogroups_word_bound_s);
  RUN_TEST(t_match_nogroups_word_bound_e);
  RUN_TEST(t_match_nogroups_word_bound_b);
  RUN_TEST(t_match_nogroups_word_bound_u);
}

TEST(t_match_nogroups_word_bound_not_s)
{
  ASSERT_MATCH(test_match("\\B", "", 'S', 0, 0));
  ASSERT_NOMATCH(test_match("\\B", "a", 'S', -1, -1));
  ASSERT_MATCH(test_match("\\B", " ", 'S', 0, 0));
  ASSERT_MATCH(test_match("\\B", "#", 'S', 0, 0));
  ASSERT_NOMATCH(test_match("a\\B", "a", 'S', -1, -1));
  ASSERT_MATCH(test_match("a\\B", "aa", 'S', 0, 1));
  ASSERT_NOMATCH(test_match("a\\B", "a ", 'S', -1, -1));
  ASSERT_NOMATCH(test_match("a\\B", "a#", 'S', -1, -1));
  ASSERT_NOMATCH(test_match("a\\B", "", 'S', -1, -1));
  PASS();
}

TEST(t_match_nogroups_word_bound_not_e)
{
  ASSERT_MATCH(test_match("\\B", "", 'E', 0, 0));
  ASSERT_NOMATCH(test_match("\\B", "a", 'E', -1, -1));
  ASSERT_MATCH(test_match("\\B", " ", 'E', 1, 1));
  ASSERT_MATCH(test_match("\\B", "#", 'E', 1, 1));
  ASSERT_NOMATCH(test_match("a\\B", "a", 'E', -1, -1));
  ASSERT_NOMATCH(test_match("a\\B", "aa", 'E', -1, -1));
  ASSERT_NOMATCH(test_match("a\\B", "a ", 'E', -1, -1));
  ASSERT_NOMATCH(test_match("a\\B", "a#", 'E', -1, -1));
  ASSERT_NOMATCH(test_match("a\\B", "", 'E', -1, -1));
  PASS();
}

TEST(t_match_nogroups_word_bound_not_b)
{
  ASSERT_MATCH(test_match("\\B", "", 'B', 0, 0));
  ASSERT_NOMATCH(test_match("\\B", "a", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("\\B", " ", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("\\B", "#", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("a\\B", "a", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("a\\B", "aa", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("a\\B", "a ", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("a\\B", "a#", 'B', -1, -1));
  ASSERT_NOMATCH(test_match("a\\B", "", 'B', -1, -1));
  PASS();
}

TEST(t_match_nogroups_word_bound_not_u)
{
  ASSERT_MATCH(test_match("\\B", "", 'U', 0, 0));
  ASSERT_NOMATCH(test_match("\\B", "a", 'U', -1, -1));
  ASSERT_MATCH(test_match("\\B", " ", 'U', 0, 0));
  ASSERT_MATCH(test_match("\\B", "#", 'U', 0, 0));
  ASSERT_NOMATCH(test_match("a\\B", "a", 'U', -1, -1));
  ASSERT_MATCH(test_match("a\\B", "aa", 'U', 0, 1));
  ASSERT_NOMATCH(test_match("a\\B", "a ", 'U', -1, -1));
  ASSERT_NOMATCH(test_match("a\\B", "a#", 'U', -1, -1));
  ASSERT_NOMATCH(test_match("a\\B", "", 'U', -1, -1));
  PASS();
}

SUITE(s_match_nogroups_word_bound_not)
{
  RUN_TEST(t_match_nogroups_word_bound_not_s);
  RUN_TEST(t_match_nogroups_word_bound_not_e);
  RUN_TEST(t_match_nogroups_word_bound_not_b);
  RUN_TEST(t_match_nogroups_word_bound_not_u);
}

SUITE(s_match_nogroups)
{
  RUN_SUITE(s_match_nogroups_one_char);
  RUN_SUITE(s_match_nogroups_two_chars);
  RUN_SUITE(s_match_nogroups_alt);
  RUN_SUITE(s_match_nogroups_star);
  RUN_SUITE(s_match_nogroups_plus);
  RUN_SUITE(s_match_nogroups_question);
  RUN_SUITE(s_match_nogroups_quantifier);
  RUN_SUITE(s_match_nogroups_begin_text);
  RUN_SUITE(s_match_nogroups_end_text);
  RUN_SUITE(s_match_nogroups_word_bound);
  RUN_SUITE(s_match_nogroups_word_bound_not);
}

TEST(t_match_long)
{
  mn_size subject_size = 1 << 20;
  mn_size i = 0;
  re reg;
  re_span grp;
  char* subject = (char*)MN_MALLOC(sizeof(char) * subject_size);
  if (subject == NULL) {
    PASS();
  }
  for (i = 0; i < subject_size; i++) {
    subject[i] = (char)RAND_PARAM(256);
  }
  ASSERT_ERR_NOMEM(re_init(&reg, "aaaaa"), error);
  re_match_groups(&reg, subject, subject_size, 'U', 1, &grp);
error:
  re_destroy(&reg);
  MN_FREE(subject);
  PASS();
}

TEST(t_match_set)
{
  re reg;
  mn_uint32 set_idx = 0;
  re_error err = RE_ERROR_NONE;
  if (re_init_set(&reg)) {
    re_destroy(&reg);
    PASS();
  }
  if (re_set_add(&reg, "abc")) {
    re_destroy(&reg);
    PASS();
  }
  if (re_set_add(&reg, "def")) {
    re_destroy(&reg);
    PASS();
  }
  if ((err = re_match_groups_set(
           &reg, "abc", 3, RE_UNANCHORED, 0, NULL, &set_idx)) == RE_MATCH) {
    ASSERT_EQ(set_idx, 1);
  } else if (err == RE_NOMATCH) {
    FAIL();
  } else {
    re_destroy(&reg);
    PASS();
  }
  re_destroy(&reg);
  PASS();
}

SUITE(s_match)
{
  RUN_SUITE(s_match_nogroups);
  RUN_TEST(t_match_long);
  RUN_TEST(t_match_set);
}

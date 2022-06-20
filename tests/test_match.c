#include "test_match.h"

static int
test_match(const char* regex, const char* text, re_anchor_type anchor_type)
{
  mn__str_view text_view;
  re reg;
  re_error rv = 0;
  mn__str_view_init_s(&text_view, text);
  ASSERT(!re_init(&reg, regex));
  rv = re_is_match(&reg, text, mn__str_view_size(&text_view), anchor_type);
  if (rv != RE_MATCH) {
    if (rv == RE_ERROR_NOMEM) {
      re_destroy(&reg);
      return RE_ERROR_NOMEM;
    } else {
      ASSERT(rv == RE_NOMATCH);
    }
  }
  re_destroy(&reg);
  return rv == RE_MATCH;
}

static mptest__result test_match2(
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

TEST(t_match_one_char_groups_none)
{
  ASSERT_MATCH(test_match2("a", "a", 'S', 0, 1));
  ASSERT_MATCH(test_match2("a", "abc", 'S', 0, 1));
  ASSERT_NOMATCH(test_match2("a", "bc", 'S', -1, -1));
  ASSERT_NOMATCH(test_match2("a", "", 'S', -1, -1));

  ASSERT_MATCH(test_match2("a", "a", 'E', 0, 1));
  ASSERT_MATCH(test_match2("a", "ba", 'E', 1, 2));
  ASSERT_NOMATCH(test_match2("a", "bac", 'E', -1, -1));
  ASSERT_NOMATCH(test_match2("a", "", 'E', -1, -1));

  ASSERT_MATCH(test_match2("a", "a", 'B', 0, 1));
  ASSERT_NOMATCH(test_match2("a", "ba", 'B', -1, -1));
  ASSERT_NOMATCH(test_match2("a", "ab", 'B', -1, -1));
  ASSERT_NOMATCH(test_match2("a", "", 'B', -1, -1));

  ASSERT_MATCH(test_match2("a", "a", 'U', 0, 1));
  ASSERT_MATCH(test_match2("a", "ba", 'U', 1, 2));
  ASSERT_MATCH(test_match2("a", "bac", 'U', 1, 2));
  ASSERT_MATCH(test_match2("a", "cab", 'U', 1, 2));
  ASSERT_NOMATCH(test_match2("a", "cb", 'U', -1, -1));
  ASSERT_NOMATCH(test_match2("a", "", 'U', -1, -1));
  PASS();
}

TEST(t_match_two_chars_groups_none)
{
  ASSERT_MATCH(test_match2("ab", "ab", 'S', 0, 2));
  ASSERT_MATCH(test_match2("ab", "abc", 'S', 0, 2));
  ASSERT_NOMATCH(test_match2("ab", "bac", 'S', -1, -1));
  ASSERT_NOMATCH(test_match2("ab", "babc", 'S', -1, -1));
  ASSERT_NOMATCH(test_match2("ab", "", 'S', -1, -1));

  ASSERT_MATCH(test_match2("ab", "ab", 'E', 0, 2));
  ASSERT_MATCH(test_match2("ab", "bab", 'E', 1, 3));
  ASSERT_MATCH(test_match2("ab", "cbab", 'E', 2, 4));
  ASSERT_NOMATCH(test_match2("ab", "b", 'E', -1, -1));
  ASSERT_NOMATCH(test_match2("ab", "ba", 'E', -1, -1));
  ASSERT_NOMATCH(test_match2("ab", "", 'E', -1, -1));
  ASSERT_NOMATCH(test_match2("ab", "abc", 'E', -1, -1));

  ASSERT_MATCH(test_match2("ab", "ab", 'B', 0, 2));
  ASSERT_NOMATCH(test_match2("ab", "bab", 'B', -1, -1));
  ASSERT_NOMATCH(test_match2("ab", "abb", 'B', -1, -1));
  ASSERT_NOMATCH(test_match2("ab", "abab", 'B', -1, -1));
  ASSERT_NOMATCH(test_match2("ab", "a", 'B', -1, -1));
  ASSERT_NOMATCH(test_match2("ab", "", 'B', -1, -1));

  ASSERT_MATCH(test_match2("ab", "ab", 'U', 0, 2));
  ASSERT_MATCH(test_match2("ab", "abab", 'U', 0, 2));
  ASSERT_MATCH(test_match2("ab", "ccabcc", 'U', 2, 4));
  ASSERT_MATCH(test_match2("ab", "cbabc", 'U', 2, 4));
  ASSERT_NOMATCH(test_match2("ab", "a", 'U', -1, -1));
  ASSERT_NOMATCH(test_match2("ab", "", 'U', -1, -1));
  ASSERT_NOMATCH(test_match2("ab", "cbac", 'U', -1, -1));

  PASS();
}

TEST(t_match_alt_groups_none)
{
  ASSERT_MATCH(test_match2("a|b", "a", 'S', 0, 1));
  ASSERT_MATCH(test_match2("a|b", "b", 'S', 0, 1));
  ASSERT_MATCH(test_match2("a|b", "aa", 'S', 0, 1));
  ASSERT_MATCH(test_match2("a|b", "ab", 'S', 0, 1));
  ASSERT_MATCH(test_match2("a|b", "ba", 'S', 0, 1));
  ASSERT_MATCH(test_match2("a|b", "bb", 'S', 0, 1));
  ASSERT_NOMATCH(test_match2("a|b", "c", 'S', -1, -1));
  ASSERT_NOMATCH(test_match2("a|b", "ca", 'S', -1, -1));
  ASSERT_NOMATCH(test_match2("a|b", "caa", 'S', -1, -1));
  ASSERT_NOMATCH(test_match2("a|b", "cbb", 'S', -1, -1));
  ASSERT_NOMATCH(test_match2("a|b", "", 'S', -1, -1));

  ASSERT_MATCH(test_match2("a|b", "a", 'E', 0, 1));
  ASSERT_MATCH(test_match2("a|b", "b", 'E', 0, 1));
  ASSERT_MATCH(test_match2("a|b", "aa", 'E', 1, 2));
  ASSERT_MATCH(test_match2("a|b", "ba", 'E', 1, 2));
  ASSERT_MATCH(test_match2("a|b", "bb", 'E', 1, 2));
  ASSERT_MATCH(test_match2("a|b", "ba", 'E', 1, 2));
  ASSERT_NOMATCH(test_match2("a|b", "ac", 'E', -1, -1));
  ASSERT_NOMATCH(test_match2("a|b", "bc", 'E', -1, -1));
  ASSERT_NOMATCH(test_match2("a|b", "", 'E', -1, -1));

  ASSERT_MATCH(test_match2("a|b", "a", 'B', 0, 1));
  ASSERT_MATCH(test_match2("a|b", "b", 'B', 0, 1));
  ASSERT_NOMATCH(test_match2("a|b", "", 'B', -1, -1));
  ASSERT_NOMATCH(test_match2("a|b", "c", 'B', -1, -1));
  ASSERT_NOMATCH(test_match2("a|b", "ca", 'B', -1, -1));
  ASSERT_NOMATCH(test_match2("a|b", "cb", 'B', -1, -1));
  ASSERT_NOMATCH(test_match2("a|b", "ac", 'B', -1, -1));
  ASSERT_NOMATCH(test_match2("a|b", "bc", 'B', -1, -1));
  ASSERT_NOMATCH(test_match2("a|b", "cc", 'B', -1, -1));

  ASSERT_MATCH(test_match2("a|b", "a", 'U', 0, 1));
  ASSERT_MATCH(test_match2("a|b", "b", 'U', 0, 1));
  ASSERT_MATCH(test_match2("a|b", "ca", 'U', 1, 2));
  ASSERT_MATCH(test_match2("a|b", "cb", 'U', 1, 2));
  ASSERT_MATCH(test_match2("a|b", "ac", 'U', 0, 1));
  ASSERT_MATCH(test_match2("a|b", "cac", 'U', 1, 2));
  ASSERT_MATCH(test_match2("a|b", "cbc", 'U', 1, 2));
  ASSERT_MATCH(test_match2("a|b", "cabc", 'U', 1, 2));
  ASSERT_MATCH(test_match2("a|b", "cbac", 'U', 1, 2));
  ASSERT_MATCH(test_match2("a|b", "ccccbaabc", 'U', 4, 5));
  ASSERT_MATCH(test_match2("a|b", "aaccbecc", 'U', 0, 1));
  ASSERT_NOMATCH(test_match2("a|b", "", 'U', -1, -1));
  ASSERT_NOMATCH(test_match2("a|b", "c", 'U', -1, -1));
  ASSERT_NOMATCH(test_match2("a|b", "cd", 'U', -1, -1));
  ASSERT_NOMATCH(test_match2("a|b", "cdef", 'U', -1, -1));

  PASS();
}

TEST(t_match_star_groups_none)
{
  ASSERT_MATCH(test_match2("a*", "", 'S', 0, 0));
  ASSERT_MATCH(test_match2("a*", "a", 'S', 0, 1));
  ASSERT_MATCH(test_match2("a*", "aa", 'S', 0, 2));
  ASSERT_MATCH(test_match2("a*", "aab", 'S', 0, 2));
  ASSERT_MATCH(test_match2("a*", "b", 'S', 0, 0));

  ASSERT_MATCH(test_match2("a*", "", 'E', 0, 0));
  ASSERT_MATCH(test_match2("a*", "a", 'E', 0, 1));
  ASSERT_MATCH(test_match2("a*", "aa", 'E', 0, 2));
  ASSERT_MATCH(test_match2("a*", "aab", 'E', 3, 3));
  ASSERT_MATCH(test_match2("a*", "b", 'E', 1, 1));

  ASSERT_MATCH(test_match2("a*", "", 'B', 0, 0));
  ASSERT_MATCH(test_match2("a*", "a", 'B', 0, 1));
  ASSERT_MATCH(test_match2("a*", "aa", 'B', 0, 2));
  ASSERT_NOMATCH(test_match2("a*", "aab", 'B', -1, -1));
  ASSERT_NOMATCH(test_match2("a*", "b", 'B', -1, -1));

  ASSERT_MATCH(test_match2("a*", "", 'U', 0, 0));
  ASSERT_MATCH(test_match2("a*", "a", 'U', 0, 1));
  ASSERT_MATCH(test_match2("a*", "aa", 'U', 0, 2));
  ASSERT_MATCH(test_match2("a*", "aab", 'U', 0, 2));
  ASSERT_MATCH(test_match2("a*", "b", 'U', 0, 0));

  PASS();
}

TEST(t_match_plus_groups_none)
{
  ASSERT_MATCH(test_match2("a+", "a", 'S', 0, 1));
  ASSERT_MATCH(test_match2("a+", "aa", 'S', 0, 2));
  ASSERT_MATCH(test_match2("a+", "aab", 'S', 0, 2));
  ASSERT_NOMATCH(test_match2("a+", "baa", 'S', -1, -1));
  ASSERT_NOMATCH(test_match2("a+", "", 'S', -1, -1));
  ASSERT_NOMATCH(test_match2("a+", "b", 'S', -1, -1));

  ASSERT_MATCH(test_match2("a+", "a", 'E', 0, 1));
  ASSERT_MATCH(test_match2("a+", "aa", 'E', 0, 2));
  ASSERT_NOMATCH(test_match2("a+", "aab", 'E', -1, -1));
  ASSERT_MATCH(test_match2("a+", "baa", 'E', 1, 3));
  ASSERT_NOMATCH(test_match2("a+", "", 'E', -1, -1));
  ASSERT_NOMATCH(test_match2("a+", "b", 'E', -1, -1));

  ASSERT_MATCH(test_match2("a+", "a", 'B', 0, 1));
  ASSERT_MATCH(test_match2("a+", "aa", 'B', 0, 2));
  ASSERT_NOMATCH(test_match2("a+", "aab", 'B', -1, -1));
  ASSERT_NOMATCH(test_match2("a+", "baa", 'B', -1, -1));
  ASSERT_NOMATCH(test_match2("a+", "", 'B', -1, -1));
  ASSERT_NOMATCH(test_match2("a+", "b", 'B', -1, -1));

  ASSERT_MATCH(test_match2("a+", "a", 'U', 0, 1));
  ASSERT_MATCH(test_match2("a+", "aa", 'U', 0, 2));
  ASSERT_MATCH(test_match2("a+", "aab", 'U', 0, 2));
  ASSERT_MATCH(test_match2("a+", "baa", 'U', 1, 3));
  ASSERT_NOMATCH(test_match2("a+", "", 'U', -1, -1));
  ASSERT_NOMATCH(test_match2("a+", "b", 'U', -1, -1));

  PASS();
}

TEST(t_match_question_groups_none)
{
  ASSERT_MATCH(test_match2("a?", "", 'S', 0, 0));
  ASSERT_MATCH(test_match2("a?", "a", 'S', 0, 1));
  ASSERT_MATCH(test_match2("a?", "aa", 'S', 0, 1));
  ASSERT_MATCH(test_match2("a?", "b", 'S', 0, 0));
  ASSERT_MATCH(test_match2("a?", "ba", 'S', 0, 0));
  ASSERT_MATCH(test_match2("a?", "ab", 'S', 0, 1));
  ASSERT_MATCH(test_match2("a?", "baa", 'S', 0, 0));
  ASSERT_MATCH(test_match2("a?", "aab", 'S', 0, 1));

  ASSERT_MATCH(test_match2("a?", "", 'E', 0, 0));
  ASSERT_MATCH(test_match2("a?", "a", 'E', 0, 1));
  ASSERT_MATCH(test_match2("a?", "aa", 'E', 1, 2));
  ASSERT_MATCH(test_match2("a?", "b", 'E', 1, 1));
  ASSERT_MATCH(test_match2("a?", "ba", 'E', 1, 2));
  ASSERT_MATCH(test_match2("a?", "ab", 'E', 2, 2));
  ASSERT_MATCH(test_match2("a?", "baa", 'E', 2, 3));
  ASSERT_MATCH(test_match2("a?", "aab", 'E', 3, 3));

  ASSERT_MATCH(test_match2("a?", "", 'B', 0, 0));
  ASSERT_MATCH(test_match2("a?", "a", 'B', 0, 1));
  ASSERT_NOMATCH(test_match2("a?", "aa", 'B', -1, -1));
  ASSERT_NOMATCH(test_match2("a?", "b", 'B', -1, -1));
  ASSERT_NOMATCH(test_match2("a?", "ba", 'B', -1, -1));
  ASSERT_NOMATCH(test_match2("a?", "ab", 'B', -1, -1));
  ASSERT_NOMATCH(test_match2("a?", "baa", 'B', -1, -1));
  ASSERT_NOMATCH(test_match2("a?", "aab", 'B', -1, -1));

  ASSERT_MATCH(test_match2("a?", "", 'U', 0, 0));
  ASSERT_MATCH(test_match2("a?", "a", 'U', 0, 1));
  ASSERT_MATCH(test_match2("a?", "aa", 'U', 0, 1));
  ASSERT_MATCH(test_match2("a?", "b", 'U', 0, 0));
  ASSERT_MATCH(test_match2("a?", "ba", 'U', 0, 0));
  ASSERT_MATCH(test_match2("a?", "ab", 'U', 0, 1));
  ASSERT_MATCH(test_match2("a?", "baa", 'U', 0, 0));
  ASSERT_MATCH(test_match2("a?", "aab", 'U', 0, 1));

  PASS();
}

TEST(t_match_quantifier_groups_none)
{
  ASSERT_MATCH(test_match2("a{1,2}", "a", 'S', 0, 1));
  ASSERT_MATCH(test_match2("a{1,2}", "aa", 'S', 0, 2));
  ASSERT_NOMATCH(test_match2("a{1,2}", "", 'S', -1, -1));
  ASSERT_MATCH(test_match2("a{1,2}", "ab", 'S', 0, 1));
  ASSERT_MATCH(test_match2("a{1,2}", "aab", 'S', 0, 2));
  ASSERT_NOMATCH(test_match2("a{1,2}", "ba", 'S', -1, -1));
  ASSERT_NOMATCH(test_match2("a{1,2}", "baa", 'S', -1, -1));

  ASSERT_MATCH(test_match2("a{1,2}", "a", 'E', 0, 1));
  ASSERT_MATCH(test_match2("a{1,2}", "aa", 'E', 0, 2));
  ASSERT_NOMATCH(test_match2("a{1,2}", "", 'E', -1, -1));
  ASSERT_NOMATCH(test_match2("a{1,2}", "ab", 'E', -1, -1));
  ASSERT_NOMATCH(test_match2("a{1,2}", "aab", 'E', -1, -1));
  ASSERT_MATCH(test_match2("a{1,2}", "ba", 'E', 1, 2));
  ASSERT_MATCH(test_match2("a{1,2}", "baa", 'E', 1, 3));

  ASSERT_MATCH(test_match2("a{1,2}", "a", 'B', 0, 1));
  ASSERT_MATCH(test_match2("a{1,2}", "aa", 'B', 0, 2));
  ASSERT_NOMATCH(test_match2("a{1,2}", "", 'B', -1, -1));
  ASSERT_NOMATCH(test_match2("a{1,2}", "ab", 'B', -1, -1));
  ASSERT_NOMATCH(test_match2("a{1,2}", "aab", 'B', -1, -1));
  ASSERT_NOMATCH(test_match2("a{1,2}", "ba", 'B', -1, -1));
  ASSERT_NOMATCH(test_match2("a{1,2}", "baa", 'B', -1, -1));

  ASSERT_MATCH(test_match2("a{1,2}", "a", 'U', 0, 1));
  ASSERT_MATCH(test_match2("a{1,2}", "aa", 'U', 0, 2));
  ASSERT_NOMATCH(test_match2("a{1,2}", "", 'U', -1, -1));
  ASSERT_MATCH(test_match2("a{1,2}", "ab", 'U', 0, 1));
  ASSERT_MATCH(test_match2("a{1,2}", "aab", 'U', 0, 2));
  ASSERT_MATCH(test_match2("a{1,2}", "ba", 'U', 1, 2));
  ASSERT_MATCH(test_match2("a{1,2}", "baa", 'U', 1, 3));

  PASS();
}

TEST(t_match_dot_groups_none)
{
  ASSERT_MATCH(test_match2(".", "a", 'S', 0, 1));
  ASSERT_MATCH(test_match2(".", "aa", 'S', 0, 1));
  ASSERT_MATCH(test_match2(".", "z", 'S', 0, 1));
  ASSERT_MATCH(test_match2(".", EX_UTF8_VALID_1, 'S', 0, 1));
  ASSERT_MATCH(test_match2(".", EX_UTF8_VALID_2, 'S', 0, 2));
  ASSERT_MATCH(test_match2(".", EX_UTF8_VALID_3, 'S', 0, 3));
  ASSERT_MATCH(test_match2(".", EX_UTF8_VALID_4, 'S', 0, 4));
  ASSERT_NOMATCH(test_match2(".", "", 'S', -1, -1));
  ASSERT_NOMATCH(test_match2(".", EX_UTF8_INVALID_OVERLONG, 'S', -1, -1));
  ASSERT_NOMATCH(test_match2(".", EX_UTF8_INVALID_TOOBIG, 'S', -1, -1));
  ASSERT_NOMATCH(test_match2(".", EX_UTF8_INVALID_SURROGATE, 'S', -1, -1));

  ASSERT_MATCH(test_match2(".", "a", 'E', 0, 1));
  ASSERT_MATCH(test_match2(".", "aa", 'E', 1, 2));
  ASSERT_MATCH(test_match2(".", "z", 'E', 0, 1));
  ASSERT_MATCH(test_match2(".", EX_UTF8_VALID_1, 'E', 0, 1));
  ASSERT_MATCH(test_match2(".", EX_UTF8_VALID_2, 'E', 0, 2));
  ASSERT_MATCH(test_match2(".", EX_UTF8_VALID_3, 'E', 0, 3));
  ASSERT_MATCH(test_match2(".", EX_UTF8_VALID_4, 'E', 0, 4));
  ASSERT_NOMATCH(test_match2(".", "", 'E', -1, -1));
  ASSERT_NOMATCH(test_match2(".", EX_UTF8_INVALID_OVERLONG, 'E', -1, -1));
  ASSERT_NOMATCH(test_match2(".", EX_UTF8_INVALID_TOOBIG, 'E', -1, -1));
  ASSERT_NOMATCH(test_match2(".", EX_UTF8_INVALID_SURROGATE, 'E', -1, -1));

  ASSERT_MATCH(test_match2(".", "a", 'B', 0, 1));
  ASSERT_NOMATCH(test_match2(".", "aa", 'B', -1, -1));
  ASSERT_MATCH(test_match2(".", "z", 'B', 0, 1));
  ASSERT_MATCH(test_match2(".", EX_UTF8_VALID_1, 'B', 0, 1));
  ASSERT_MATCH(test_match2(".", EX_UTF8_VALID_2, 'B', 0, 2));
  ASSERT_MATCH(test_match2(".", EX_UTF8_VALID_3, 'B', 0, 3));
  ASSERT_MATCH(test_match2(".", EX_UTF8_VALID_4, 'B', 0, 4));
  ASSERT_NOMATCH(test_match2(".", "", 'B', -1, -1));
  ASSERT_NOMATCH(test_match2(".", EX_UTF8_INVALID_OVERLONG, 'B', -1, -1));
  ASSERT_NOMATCH(test_match2(".", EX_UTF8_INVALID_TOOBIG, 'B', -1, -1));
  ASSERT_NOMATCH(test_match2(".", EX_UTF8_INVALID_SURROGATE, 'B', -1, -1));

  ASSERT_MATCH(test_match2(".", "a", 'U', 0, 1));
  ASSERT_MATCH(test_match2(".", "aa", 'U', 0, 1));
  ASSERT_MATCH(test_match2(".", "z", 'U', 0, 1));
  ASSERT_MATCH(test_match2(".", EX_UTF8_VALID_1, 'U', 0, 1));
  ASSERT_MATCH(test_match2(".", EX_UTF8_VALID_2, 'U', 0, 2));
  ASSERT_MATCH(test_match2(".", EX_UTF8_VALID_3, 'U', 0, 3));
  ASSERT_MATCH(test_match2(".", EX_UTF8_VALID_4, 'U', 0, 4));
  ASSERT_NOMATCH(test_match2(".", "", 'U', -1, -1));
  ASSERT_NOMATCH(test_match2(".", EX_UTF8_INVALID_OVERLONG, 'U', -1, -1));
  ASSERT_NOMATCH(test_match2(".", EX_UTF8_INVALID_TOOBIG, 'U', -1, -1));
  ASSERT_NOMATCH(test_match2(".", EX_UTF8_INVALID_SURROGATE, 'U', -1, -1));

  PASS();
}

TEST(t_match_begin_text_groups_none)
{
  ASSERT_MATCH(test_match2("^a", "a", 'S', 0, 1));
  ASSERT_MATCH(test_match2("^a", "aa", 'S', 0, 1));
  ASSERT_MATCH(test_match2("^a", "ab", 'S', 0, 1));
  ASSERT_NOMATCH(test_match2("^a", "", 'S', -1, -1));
  ASSERT_NOMATCH(test_match2("^a", "ba", 'S', -1, -1));

  ASSERT_MATCH(test_match2("^a", "a", 'E', 0, 1));
  ASSERT_NOMATCH(test_match2("^a", "aa", 'E', -1, -1));
  ASSERT_NOMATCH(test_match2("^a", "ab", 'E', -1, -1));
  ASSERT_NOMATCH(test_match2("^a", "", 'E', -1, -1));
  ASSERT_NOMATCH(test_match2("^a", "ba", 'E', -1, -1));

  ASSERT_MATCH(test_match2("^a", "a", 'B', 0, 1));
  ASSERT_NOMATCH(test_match2("^a", "aa", 'B', -1, -1));
  ASSERT_NOMATCH(test_match2("^a", "ab", 'B', -1, -1));
  ASSERT_NOMATCH(test_match2("^a", "", 'B', -1, -1));
  ASSERT_NOMATCH(test_match2("^a", "ba", 'B', -1, -1));

  ASSERT_MATCH(test_match2("^a", "a", 'U', 0, 1));
  ASSERT_MATCH(test_match2("^a", "aa", 'U', 0, 1));
  ASSERT_MATCH(test_match2("^a", "ab", 'U', 0, 1));
  ASSERT_NOMATCH(test_match2("^a", "", 'U', -1, -1));
  ASSERT_NOMATCH(test_match2("^a", "ba", 'U', -1, -1));

  PASS();
}

TEST(t_match_end_text_groups_none)
{
  ASSERT_MATCH(test_match2("a$", "a", 'S', 0, 1));
  ASSERT_NOMATCH(test_match2("a$", "aa", 'S', -1, -1));
  ASSERT_NOMATCH(test_match2("a$", "ab", 'S', -1, -1));
  ASSERT_NOMATCH(test_match2("a$", "", 'S', -1, -1));
  ASSERT_NOMATCH(test_match2("a$", "ba", 'S', -1, -1));

  ASSERT_MATCH(test_match2("a$", "a", 'E', 0, 1));
  ASSERT_MATCH(test_match2("a$", "aa", 'E', 1, 2));
  ASSERT_NOMATCH(test_match2("a$", "ab", 'E', -1, -1));
  ASSERT_NOMATCH(test_match2("a$", "", 'E', -1, -1));
  ASSERT_MATCH(test_match2("a$", "ba", 'E', 1, 2));

  ASSERT_MATCH(test_match2("a$", "a", 'B', 0, 1));
  ASSERT_NOMATCH(test_match2("a$", "aa", 'B', -1, -1));
  ASSERT_NOMATCH(test_match2("a$", "ab", 'B', -1, -1));
  ASSERT_NOMATCH(test_match2("a$", "", 'B', -1, -1));
  ASSERT_NOMATCH(test_match2("a$", "ba", 'B', -1, -1));

  ASSERT_MATCH(test_match2("a$", "a", 'U', 0, 1));
  ASSERT_MATCH(test_match2("a$", "aa", 'U', 1, 2));
  ASSERT_NOMATCH(test_match2("a$", "ab", 'U', -1, -1));
  ASSERT_NOMATCH(test_match2("a$", "", 'U', -1, -1));
  ASSERT_MATCH(test_match2("a$", "ba", 'U', 1, 2));

  PASS();
}

TEST(t_match_word_bound_groups_none)
{
  ASSERT_NOMATCH(test_match2("\\b", "", 'S', -1, -1));
  ASSERT_MATCH(test_match2("\\b", "a", 'S', 0, 0));
  ASSERT_NOMATCH(test_match2("\\b", " ", 'S', -1, -1));
  ASSERT_NOMATCH(test_match2("\\b", "#", 'S', -1, -1));
  ASSERT_MATCH(test_match2("a\\b", "a", 'S', 0, 1));
  ASSERT_NOMATCH(test_match2("a\\b", "aa", 'S', -1, -1));
  ASSERT_MATCH(test_match2("a\\b", "a ", 'S', 0, 1));
  ASSERT_MATCH(test_match2("a\\b", "a#", 'S', 0, 1));
  ASSERT_NOMATCH(test_match2("a\\b", "", 'S', -1, -1));

  ASSERT_NOMATCH(test_match("\\b", "", 'E'));
  ASSERT_MATCH(test_match("\\b", "a", 'E'));
  ASSERT_NOMATCH(test_match("\\b", " ", 'E'));
  ASSERT_NOMATCH(test_match("\\b", "#", 'E'));
  ASSERT_MATCH(test_match("a\\b", "a", 'E'));
  ASSERT_MATCH(test_match("a\\b", "aa", 'E'));
  ASSERT_NOMATCH(test_match("a\\b", "a ", 'E'));
  ASSERT_NOMATCH(test_match("a\\b", "a#", 'E'));
  ASSERT_NOMATCH(test_match("a\\b", "", 'E'));

  ASSERT_NOMATCH(test_match("\\b", "", 'B'));
  ASSERT_NOMATCH(test_match("\\b", "a", 'B'));
  ASSERT_NOMATCH(test_match("\\b", " ", 'B'));
  ASSERT_NOMATCH(test_match("\\b", "#", 'B'));
  ASSERT_MATCH(test_match("a\\b", "a", 'B'));
  ASSERT_NOMATCH(test_match("a\\b", "aa", 'B'));
  ASSERT_NOMATCH(test_match("a\\b", "a ", 'B'));
  ASSERT_NOMATCH(test_match("a\\b", "a#", 'B'));
  ASSERT_NOMATCH(test_match("a\\b", "", 'B'));

  ASSERT_NOMATCH(test_match("\\b", "", 'U'));
  ASSERT_MATCH(test_match("\\b", "a", 'U'));
  ASSERT_NOMATCH(test_match("\\b", " ", 'U'));
  ASSERT_NOMATCH(test_match("\\b", "#", 'U'));
  ASSERT_MATCH(test_match("a\\b", "a", 'U'));
  ASSERT_MATCH(test_match("a\\b", "aa", 'U'));
  ASSERT_MATCH(test_match("a\\b", "a ", 'U'));
  ASSERT_MATCH(test_match("a\\b", "a#", 'U'));
  ASSERT_NOMATCH(test_match("a\\b", "", 'U'));

  PASS();
}

TEST(t_match_word_bound_not_groups_none)
{
  ASSERT_MATCH(test_match("\\B", "", 'S'));
  ASSERT_NOMATCH(test_match("\\B", "a", 'S'));
  ASSERT_MATCH(test_match("\\B", " ", 'S'));
  ASSERT_MATCH(test_match("\\B", "#", 'S'));
  ASSERT_NOMATCH(test_match("a\\B", "a", 'S'));
  ASSERT_MATCH(test_match("a\\B", "aa", 'S'));
  ASSERT_NOMATCH(test_match("a\\B", "a ", 'S'));
  ASSERT_NOMATCH(test_match("a\\B", "a#", 'S'));
  ASSERT_NOMATCH(test_match("a\\B", "", 'S'));

  ASSERT_MATCH(test_match("\\B", "", 'E'));
  ASSERT_NOMATCH(test_match("\\B", "a", 'E'));
  ASSERT_MATCH(test_match("\\B", " ", 'E'));
  ASSERT_MATCH(test_match("\\B", "#", 'E'));
  ASSERT_NOMATCH(test_match("a\\B", "a", 'E'));
  ASSERT_NOMATCH(test_match("a\\B", "aa", 'E'));
  ASSERT_NOMATCH(test_match("a\\B", "a ", 'E'));
  ASSERT_NOMATCH(test_match("a\\B", "a#", 'E'));
  ASSERT_NOMATCH(test_match("a\\B", "", 'E'));

  ASSERT_MATCH(test_match("\\B", "", 'B'));
  ASSERT_NOMATCH(test_match("\\B", "a", 'B'));
  ASSERT_NOMATCH(test_match("\\B", " ", 'B'));
  ASSERT_NOMATCH(test_match("\\B", "#", 'B'));
  ASSERT_NOMATCH(test_match("a\\B", "a", 'B'));
  ASSERT_NOMATCH(test_match("a\\B", "aa", 'B'));
  ASSERT_NOMATCH(test_match("a\\B", "a ", 'B'));
  ASSERT_NOMATCH(test_match("a\\B", "a#", 'B'));
  ASSERT_NOMATCH(test_match("a\\B", "", 'B'));

  ASSERT_MATCH(test_match("\\B", "", 'U'));
  ASSERT_NOMATCH(test_match("\\B", "a", 'U'));
  ASSERT_MATCH(test_match("\\B", " ", 'U'));
  ASSERT_MATCH(test_match("\\B", "#", 'U'));
  ASSERT_NOMATCH(test_match("a\\B", "a", 'U'));
  ASSERT_MATCH(test_match("a\\B", "aa", 'U'));
  ASSERT_NOMATCH(test_match("a\\B", "a ", 'U'));
  ASSERT_NOMATCH(test_match("a\\B", "a#", 'U'));
  ASSERT_NOMATCH(test_match("a\\B", "", 'U'));

  PASS();
}

SUITE(s_match)
{
  RUN_TEST(t_match_one_char_groups_none);
  RUN_TEST(t_match_two_chars_groups_none);
  RUN_TEST(t_match_alt_groups_none);
  RUN_TEST(t_match_star_groups_none);
  RUN_TEST(t_match_plus_groups_none);
  RUN_TEST(t_match_question_groups_none);
  RUN_TEST(t_match_quantifier_groups_none);
  RUN_TEST(t_match_dot_groups_none);
  RUN_TEST(t_match_begin_text_groups_none);
  RUN_TEST(t_match_end_text_groups_none);
  RUN_TEST(t_match_word_bound_groups_none);
  RUN_TEST(t_match_word_bound_not_groups_none);
}

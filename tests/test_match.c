#include "test_match.h"

/* https://en.wikipedia.org/wiki/UTF-8 */
#define EX_UTF8_VALID_1 "$"
#define EX_UTF8_VALID_2 "\xc2\xa3"
#define EX_UTF8_VALID_3 "\xe0\xa4\xb9"
#define EX_UTF8_VALID_4 "\xf0\x90\x8d\x88"
#define EX_UTF8_INVALID_OVERLONG "\xc0\x81"
#define EX_UTF8_INVALID_SURROGATE "\xed\xa0\xb0"
#define EX_UTF8_INVALID_TOOBIG "\xf4\x90\xb0\xb0"

static int test_match(const char* regex, const char* text, re_anchor_type anchor_type) {
    mn__str_view text_view;
    re reg;
    re_error rv = 0;
    mn__str_view_init_s(&text_view, text);
    ASSERT(!re_init(&reg, regex));
    rv = re_is_match(&reg, text, mn__str_view_size(&text_view), anchor_type);
    if (rv != RE_MATCH) {
        ASSERT(rv == RE_NOMATCH);
    }
    re_destroy(&reg);
    return rv == RE_MATCH;
}

TEST(t_match_one_char_groups_none) {
    ASSERT(test_match("a", "a", 'S'));
    ASSERT(test_match("a", "abc", 'S'));
    ASSERT(!test_match("a", "bc", 'S'));
    ASSERT(!test_match("a", "", 'S'));

    ASSERT(test_match("a", "a", 'E'));
    ASSERT(test_match("a", "ba", 'E'));
    ASSERT(!test_match("a", "bac", 'E'));
    ASSERT(!test_match("a", "", 'E'));
    
    ASSERT(test_match("a", "a", 'B'));
    ASSERT(!test_match("a", "ba", 'B'));
    ASSERT(!test_match("a", "ab", 'B'));
    ASSERT(!test_match("a", "", 'B'));

    ASSERT(test_match("a", "a", 'U'));
    ASSERT(test_match("a", "ba", 'U'));
    ASSERT(test_match("a", "bac", 'U'));
    ASSERT(test_match("a", "cab", 'U'));
    ASSERT(!test_match("a", "cb", 'U'));
    ASSERT(!test_match("a", "", 'U'));
    PASS();
}

TEST(t_match_two_chars_groups_none) {
    ASSERT(test_match("ab", "ab", 'S'));
    ASSERT(test_match("ab", "abc", 'S'));
    ASSERT(!test_match("ab", "bac", 'S'));
    ASSERT(!test_match("ab", "babc", 'S'));
    ASSERT(!test_match("ab", "", 'S'));

    ASSERT(test_match("ab", "ab", 'E'));
    ASSERT(test_match("ab", "bab", 'E'));
    ASSERT(test_match("ab", "cbab", 'E'));
    ASSERT(!test_match("ab", "b", 'E'));
    ASSERT(!test_match("ab", "ba", 'E'));
    ASSERT(!test_match("ab", "", 'E'));
    ASSERT(!test_match("ab", "abc", 'E'));

    ASSERT(test_match("ab", "ab", 'B'));
    ASSERT(!test_match("ab", "bab", 'B'));
    ASSERT(!test_match("ab", "abb", 'B'));
    ASSERT(!test_match("ab", "abab", 'B'));
    ASSERT(!test_match("ab", "a", 'B'));
    ASSERT(!test_match("ab", "", 'B'));

    ASSERT(test_match("ab", "ab", 'U'));
    ASSERT(test_match("ab", "abab", 'U'));
    ASSERT(test_match("ab", "ccabcc", 'U'));
    ASSERT(test_match("ab", "cbabc", 'U'));
    ASSERT(!test_match("ab", "a", 'U'));
    ASSERT(!test_match("ab", "", 'U'));
    ASSERT(!test_match("ab", "cbac", 'U'));

    PASS();
}

TEST(t_match_alt_groups_none) {
    ASSERT(test_match("a|b", "a", 'S'));
    ASSERT(test_match("a|b", "b", 'S'));
    ASSERT(test_match("a|b", "aa", 'S'));
    ASSERT(test_match("a|b", "ab", 'S'));
    ASSERT(test_match("a|b", "ba", 'S'));
    ASSERT(test_match("a|b", "bb", 'S'));
    ASSERT(!test_match("a|b", "c", 'S'));
    ASSERT(!test_match("a|b", "ca", 'S'));
    ASSERT(!test_match("a|b", "caa", 'S'));
    ASSERT(!test_match("a|b", "cbb", 'S'));
    ASSERT(!test_match("a|b", "", 'S'));

    ASSERT(test_match("a|b", "a", 'E'));
    ASSERT(test_match("a|b", "b", 'E'));
    ASSERT(test_match("a|b", "aa", 'E'));
    ASSERT(test_match("a|b", "ba", 'E'));
    ASSERT(test_match("a|b", "bb", 'E'));
    ASSERT(test_match("a|b", "ba", 'E'));
    ASSERT(!test_match("a|b", "ac", 'E'));
    ASSERT(!test_match("a|b", "bc", 'E'));
    ASSERT(!test_match("a|b", "", 'E'));

    ASSERT(test_match("a|b", "a", 'B'));
    ASSERT(test_match("a|b", "b", 'B'));
    ASSERT(!test_match("a|b", "", 'B'));
    ASSERT(!test_match("a|b", "c", 'B'));
    ASSERT(!test_match("a|b", "ca", 'B'));
    ASSERT(!test_match("a|b", "cb", 'B'));
    ASSERT(!test_match("a|b", "ac", 'B'));
    ASSERT(!test_match("a|b", "bc", 'B'));
    ASSERT(!test_match("a|b", "cc", 'B'));

    ASSERT(test_match("a|b", "a", 'U'));
    ASSERT(test_match("a|b", "b", 'U'));
    ASSERT(test_match("a|b", "ca", 'U'));
    ASSERT(test_match("a|b", "cb", 'U'));
    ASSERT(test_match("a|b", "ac", 'U'));
    ASSERT(test_match("a|b", "cac", 'U'));
    ASSERT(test_match("a|b", "cbc", 'U'));
    ASSERT(test_match("a|b", "cabc", 'U'));
    ASSERT(test_match("a|b", "cbac", 'U'));
    ASSERT(test_match("a|b", "ccccbaabc", 'U'));
    ASSERT(test_match("a|b", "aaccbecc", 'U'));
    ASSERT(!test_match("a|b", "", 'U'));
    ASSERT(!test_match("a|b", "c", 'U'));
    ASSERT(!test_match("a|b", "cd", 'U'));
    ASSERT(!test_match("a|b", "cdef", 'U'));

    PASS();
}

TEST(t_match_star_groups_none) {
    ASSERT(test_match("a*", "", 'S'));
    ASSERT(test_match("a*", "a", 'S'));
    ASSERT(test_match("a*", "aa", 'S'));
    ASSERT(test_match("a*", "aab", 'S'));
    ASSERT(test_match("a*", "b", 'S'));

    ASSERT(test_match("a*", "", 'E'));
    ASSERT(test_match("a*", "a", 'E'));
    ASSERT(test_match("a*", "aa", 'E'));
    ASSERT(test_match("a*", "aab", 'E'));
    ASSERT(test_match("a*", "b", 'E'));

    ASSERT(test_match("a*", "", 'B'));
    ASSERT(test_match("a*", "a", 'B'));
    ASSERT(test_match("a*", "aa", 'B'));
    ASSERT(!test_match("a*", "aab", 'B'));
    ASSERT(!test_match("a*", "b", 'B'));

    ASSERT(test_match("a*", "", 'U'));
    ASSERT(test_match("a*", "a", 'U'));
    ASSERT(test_match("a*", "aa", 'U'));
    ASSERT(test_match("a*", "aab", 'U'));
    ASSERT(test_match("a*", "b", 'U'));

    PASS();
}

TEST(t_match_plus_groups_none) {
    ASSERT(test_match("a+", "a", 'S'));
    ASSERT(test_match("a+", "aa", 'S'));
    ASSERT(test_match("a+", "aab", 'S'));
    ASSERT(!test_match("a+", "baa", 'S'));
    ASSERT(!test_match("a+", "", 'S'));
    ASSERT(!test_match("a+", "b", 'S'));

    ASSERT(test_match("a+", "a", 'E'));
    ASSERT(test_match("a+", "aa", 'E'));
    ASSERT(!test_match("a+", "aab", 'E'));
    ASSERT(test_match("a+", "baa", 'E'));
    ASSERT(!test_match("a+", "", 'E'));
    ASSERT(!test_match("a+", "b", 'E'));

    ASSERT(test_match("a+", "a", 'B'));
    ASSERT(test_match("a+", "aa", 'B'));
    ASSERT(!test_match("a+", "aab", 'B'));
    ASSERT(!test_match("a+", "baa", 'B'));
    ASSERT(!test_match("a+", "", 'B'));
    ASSERT(!test_match("a+", "b", 'B'));

    ASSERT(test_match("a+", "a", 'U'));
    ASSERT(test_match("a+", "aa", 'U'));
    ASSERT(test_match("a+", "aab", 'U'));
    ASSERT(test_match("a+", "baa", 'U'));
    ASSERT(!test_match("a+", "", 'U'));
    ASSERT(!test_match("a+", "b", 'U'));

    PASS();
}

TEST(t_match_question_groups_none) {
    ASSERT(test_match("a?", "", 'S'));
    ASSERT(test_match("a?", "a", 'S'));
    ASSERT(test_match("a?", "aa", 'S'));
    ASSERT(test_match("a?", "b", 'S'));
    ASSERT(test_match("a?", "ba", 'S'));
    ASSERT(test_match("a?", "ab", 'S'));
    ASSERT(test_match("a?", "baa", 'S'));
    ASSERT(test_match("a?", "aab", 'S'));

    ASSERT(test_match("a?", "", 'E'));
    ASSERT(test_match("a?", "a", 'E'));
    ASSERT(test_match("a?", "aa", 'E'));
    ASSERT(test_match("a?", "b", 'E'));
    ASSERT(test_match("a?", "ba", 'E'));
    ASSERT(test_match("a?", "ab", 'E'));
    ASSERT(test_match("a?", "baa", 'E'));
    ASSERT(test_match("a?", "aab", 'E'));

    ASSERT(test_match("a?", "", 'B'));
    ASSERT(test_match("a?", "a", 'B'));
    ASSERT(test_match("a?", "aa", 'B'));
    ASSERT(!test_match("a?", "b", 'B'));
    ASSERT(!test_match("a?", "ba", 'B'));
    ASSERT(!test_match("a?", "ab", 'B'));
    ASSERT(!test_match("a?", "baa", 'B'));
    ASSERT(!test_match("a?", "aab", 'B'));

    ASSERT(test_match("a?", "", 'U'));
    ASSERT(test_match("a?", "a", 'U'));
    ASSERT(test_match("a?", "aa", 'U'));
    ASSERT(test_match("a?", "b", 'U'));
    ASSERT(test_match("a?", "ba", 'U'));
    ASSERT(test_match("a?", "ab", 'U'));
    ASSERT(test_match("a?", "baa", 'U'));
    ASSERT(test_match("a?", "aab", 'U'));

    PASS();
}

TEST(t_match_quantifier_groups_none) {
    ASSERT(test_match("a{1,2}", "a", 'S'));
    ASSERT(test_match("a{1,2}", "aa", 'S'));
    ASSERT(!test_match("a{1,2}", "", 'S'));
    ASSERT(test_match("a{1,2}", "ab", 'S'));
    ASSERT(test_match("a{1,2}", "aab", 'S'));
    ASSERT(!test_match("a{1,2}", "ba", 'S'));
    ASSERT(!test_match("a{1,2}", "baa", 'S'));

    ASSERT(test_match("a{1,2}", "a", 'E'));
    ASSERT(test_match("a{1,2}", "aa", 'E'));
    ASSERT(!test_match("a{1,2}", "", 'E'));
    ASSERT(!test_match("a{1,2}", "ab", 'E'));
    ASSERT(!test_match("a{1,2}", "aab", 'E'));
    ASSERT(test_match("a{1,2}", "ba", 'E'));
    ASSERT(test_match("a{1,2}", "baa", 'E'));

    ASSERT(test_match("a{1,2}", "a", 'B'));
    ASSERT(test_match("a{1,2}", "aa", 'B'));
    ASSERT(!test_match("a{1,2}", "", 'B'));
    ASSERT(!test_match("a{1,2}", "ab", 'B'));
    ASSERT(!test_match("a{1,2}", "aab", 'B'));
    ASSERT(!test_match("a{1,2}", "ba", 'B'));
    ASSERT(!test_match("a{1,2}", "baa", 'B'));

    ASSERT(test_match("a{1,2}", "a", 'U'));
    ASSERT(test_match("a{1,2}", "aa", 'U'));
    ASSERT(!test_match("a{1,2}", "", 'U'));
    ASSERT(test_match("a{1,2}", "ab", 'U'));
    ASSERT(test_match("a{1,2}", "aab", 'U'));
    ASSERT(test_match("a{1,2}", "ba", 'U'));
    ASSERT(test_match("a{1,2}", "baa", 'U'));

    PASS();
}

TEST(t_match_dot_groups_none) {
    ASSERT(test_match(".", "a", 'S'));
    ASSERT(test_match(".", "aa", 'S'));
    ASSERT(test_match(".", "z", 'S'));
    ASSERT(test_match(".", EX_UTF8_VALID_1, 'S'));
    ASSERT(test_match(".", EX_UTF8_VALID_2, 'S'));
    ASSERT(test_match(".", EX_UTF8_VALID_3, 'S'));
    ASSERT(test_match(".", EX_UTF8_VALID_4, 'S'));
    ASSERT(!test_match(".", "", 'S'));
    ASSERT(!test_match(".", EX_UTF8_INVALID_OVERLONG, 'S'));
    ASSERT(!test_match(".", EX_UTF8_INVALID_TOOBIG, 'S'));
    ASSERT(!test_match(".", EX_UTF8_INVALID_SURROGATE, 'S'));

    ASSERT(test_match(".", "a", 'E'));
    ASSERT(test_match(".", "aa", 'E'));
    ASSERT(test_match(".", "z", 'E'));
    ASSERT(test_match(".", EX_UTF8_VALID_1, 'E'));
    ASSERT(test_match(".", EX_UTF8_VALID_2, 'E'));
    ASSERT(test_match(".", EX_UTF8_VALID_3, 'E'));
    ASSERT(test_match(".", EX_UTF8_VALID_4, 'E'));
    ASSERT(!test_match(".", "", 'E'));
    ASSERT(!test_match(".", EX_UTF8_INVALID_OVERLONG, 'E'));
    ASSERT(!test_match(".", EX_UTF8_INVALID_TOOBIG, 'E'));
    ASSERT(!test_match(".", EX_UTF8_INVALID_SURROGATE, 'E'));

    ASSERT(test_match(".", "a", 'B'));
    ASSERT(!test_match(".", "aa", 'B'));
    ASSERT(test_match(".", "z", 'B'));
    ASSERT(test_match(".", EX_UTF8_VALID_1, 'B'));
    ASSERT(test_match(".", EX_UTF8_VALID_2, 'B'));
    ASSERT(test_match(".", EX_UTF8_VALID_3, 'B'));
    ASSERT(test_match(".", EX_UTF8_VALID_4, 'B'));
    ASSERT(!test_match(".", "", 'B'));
    ASSERT(!test_match(".", EX_UTF8_INVALID_OVERLONG, 'B'));
    ASSERT(!test_match(".", EX_UTF8_INVALID_TOOBIG, 'B'));
    ASSERT(!test_match(".", EX_UTF8_INVALID_SURROGATE, 'B'));

    ASSERT(test_match(".", "a", 'U'));
    ASSERT(test_match(".", "aa", 'U'));
    ASSERT(test_match(".", "z", 'U'));
    ASSERT(test_match(".", EX_UTF8_VALID_1, 'U'));
    ASSERT(test_match(".", EX_UTF8_VALID_2, 'U'));
    ASSERT(test_match(".", EX_UTF8_VALID_3, 'U'));
    ASSERT(test_match(".", EX_UTF8_VALID_4, 'U'));
    ASSERT(!test_match(".", "", 'U'));
    ASSERT(!test_match(".", EX_UTF8_INVALID_OVERLONG, 'U'));
    ASSERT(!test_match(".", EX_UTF8_INVALID_TOOBIG, 'U'));
    ASSERT(!test_match(".", EX_UTF8_INVALID_SURROGATE, 'U'));

    PASS();
}

TEST(t_match_begin_text_groups_none) {
    ASSERT(test_match("^a", "a", 'S'));
    ASSERT(test_match("^a", "aa", 'S'));
    ASSERT(test_match("^a", "ab", 'S'));
    ASSERT(!test_match("^a", "", 'S'));
    ASSERT(!test_match("^a", "ba", 'S'));

    ASSERT(test_match("^a", "a", 'E'));
    ASSERT(!test_match("^a", "aa", 'E'));
    ASSERT(!test_match("^a", "ab", 'E'));
    ASSERT(!test_match("^a", "", 'E'));
    ASSERT(!test_match("^a", "ba", 'E'));

    ASSERT(test_match("^a", "a", 'B'));
    ASSERT(!test_match("^a", "aa", 'B'));
    ASSERT(!test_match("^a", "ab", 'B'));
    ASSERT(!test_match("^a", "", 'B'));
    ASSERT(!test_match("^a", "ba", 'B'));

    ASSERT(test_match("^a", "a", 'U'));
    ASSERT(test_match("^a", "aa", 'U'));
    ASSERT(test_match("^a", "ab", 'U'));
    ASSERT(!test_match("^a", "", 'U'));
    ASSERT(!test_match("^a", "ba", 'U'));

    PASS();
}

SUITE(s_match) {
    RUN_TEST(t_match_begin_text_groups_none);

    RUN_TEST(t_match_one_char_groups_none);
    RUN_TEST(t_match_two_chars_groups_none);
    RUN_TEST(t_match_alt_groups_none);
    RUN_TEST(t_match_star_groups_none);
    RUN_TEST(t_match_plus_groups_none);
    RUN_TEST(t_match_question_groups_none);
    RUN_TEST(t_match_quantifier_groups_none);
    RUN_TEST(t_match_dot_groups_none);
}

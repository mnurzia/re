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


static mptest__result test_match2(const char* regex, const char* text, re_anchor_type anchor_type, int bound_start, int bound_end) {
    mn__str_view text_view;
    re reg;
    re_error rv = 0;
    re_span out_groups;
    mn__str_view_init_s(&text_view, text);
    ASSERT(!re_init(&reg, regex));
    rv = re_is_match(&reg, text, mn__str_view_size(&text_view), anchor_type);
    if (rv != RE_MATCH) {
        ASSERT(rv == RE_NOMATCH);
    }
    rv = re_match_groups(&reg, text, mn__str_view_size(&text_view), anchor_type, 1, &out_groups);
    if (rv != RE_MATCH) {
        ASSERT(rv == RE_NOMATCH);
        ASSERT_EQ(bound_start, -1);
        ASSERT_EQ(bound_end, -1);
    } else {
        ASSERT_EQ((int)out_groups.begin, bound_start);
        ASSERT_EQ((int)out_groups.end, bound_end);
    }
    re_destroy(&reg);
    return rv == RE_MATCH;
}

#define ASSERT_MATCH(a) ASSERT_EQ(a, 1);
#define ASSERT_NOMATCH(a) ASSERT_EQ(a, 0);

TEST(t_match_one_char_groups_none) {
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

TEST(t_match_two_chars_groups_none) {
    ASSERT_MATCH(test_match("ab", "ab", 'S'));
    ASSERT_MATCH(test_match("ab", "abc", 'S'));
    ASSERT_NOMATCH(test_match("ab", "bac", 'S'));
    ASSERT_NOMATCH(test_match("ab", "babc", 'S'));
    ASSERT_NOMATCH(test_match("ab", "", 'S'));

    ASSERT_MATCH(test_match("ab", "ab", 'E'));
    ASSERT_MATCH(test_match("ab", "bab", 'E'));
    ASSERT_MATCH(test_match("ab", "cbab", 'E'));
    ASSERT_NOMATCH(test_match("ab", "b", 'E'));
    ASSERT_NOMATCH(test_match("ab", "ba", 'E'));
    ASSERT_NOMATCH(test_match("ab", "", 'E'));
    ASSERT_NOMATCH(test_match("ab", "abc", 'E'));

    ASSERT_MATCH(test_match("ab", "ab", 'B'));
    ASSERT_NOMATCH(test_match("ab", "bab", 'B'));
    ASSERT_NOMATCH(test_match("ab", "abb", 'B'));
    ASSERT_NOMATCH(test_match("ab", "abab", 'B'));
    ASSERT_NOMATCH(test_match("ab", "a", 'B'));
    ASSERT_NOMATCH(test_match("ab", "", 'B'));

    ASSERT_MATCH(test_match("ab", "ab", 'U'));
    ASSERT_MATCH(test_match("ab", "abab", 'U'));
    ASSERT_MATCH(test_match("ab", "ccabcc", 'U'));
    ASSERT_MATCH(test_match("ab", "cbabc", 'U'));
    ASSERT_NOMATCH(test_match("ab", "a", 'U'));
    ASSERT_NOMATCH(test_match("ab", "", 'U'));
    ASSERT_NOMATCH(test_match("ab", "cbac", 'U'));

    PASS();
}

TEST(t_match_alt_groups_none) {
    ASSERT_MATCH(test_match("a|b", "a", 'S'));
    ASSERT_MATCH(test_match("a|b", "b", 'S'));
    ASSERT_MATCH(test_match("a|b", "aa", 'S'));
    ASSERT_MATCH(test_match("a|b", "ab", 'S'));
    ASSERT_MATCH(test_match("a|b", "ba", 'S'));
    ASSERT_MATCH(test_match("a|b", "bb", 'S'));
    ASSERT_NOMATCH(test_match("a|b", "c", 'S'));
    ASSERT_NOMATCH(test_match("a|b", "ca", 'S'));
    ASSERT_NOMATCH(test_match("a|b", "caa", 'S'));
    ASSERT_NOMATCH(test_match("a|b", "cbb", 'S'));
    ASSERT_NOMATCH(test_match("a|b", "", 'S'));

    ASSERT_MATCH(test_match("a|b", "a", 'E'));
    ASSERT_MATCH(test_match("a|b", "b", 'E'));
    ASSERT_MATCH(test_match("a|b", "aa", 'E'));
    ASSERT_MATCH(test_match("a|b", "ba", 'E'));
    ASSERT_MATCH(test_match("a|b", "bb", 'E'));
    ASSERT_MATCH(test_match("a|b", "ba", 'E'));
    ASSERT_NOMATCH(test_match("a|b", "ac", 'E'));
    ASSERT_NOMATCH(test_match("a|b", "bc", 'E'));
    ASSERT_NOMATCH(test_match("a|b", "", 'E'));

    ASSERT_MATCH(test_match("a|b", "a", 'B'));
    ASSERT_MATCH(test_match("a|b", "b", 'B'));
    ASSERT_NOMATCH(test_match("a|b", "", 'B'));
    ASSERT_NOMATCH(test_match("a|b", "c", 'B'));
    ASSERT_NOMATCH(test_match("a|b", "ca", 'B'));
    ASSERT_NOMATCH(test_match("a|b", "cb", 'B'));
    ASSERT_NOMATCH(test_match("a|b", "ac", 'B'));
    ASSERT_NOMATCH(test_match("a|b", "bc", 'B'));
    ASSERT_NOMATCH(test_match("a|b", "cc", 'B'));

    ASSERT_MATCH(test_match("a|b", "a", 'U'));
    ASSERT_MATCH(test_match("a|b", "b", 'U'));
    ASSERT_MATCH(test_match("a|b", "ca", 'U'));
    ASSERT_MATCH(test_match("a|b", "cb", 'U'));
    ASSERT_MATCH(test_match("a|b", "ac", 'U'));
    ASSERT_MATCH(test_match("a|b", "cac", 'U'));
    ASSERT_MATCH(test_match("a|b", "cbc", 'U'));
    ASSERT_MATCH(test_match("a|b", "cabc", 'U'));
    ASSERT_MATCH(test_match("a|b", "cbac", 'U'));
    ASSERT_MATCH(test_match("a|b", "ccccbaabc", 'U'));
    ASSERT_MATCH(test_match("a|b", "aaccbecc", 'U'));
    ASSERT_NOMATCH(test_match("a|b", "", 'U'));
    ASSERT_NOMATCH(test_match("a|b", "c", 'U'));
    ASSERT_NOMATCH(test_match("a|b", "cd", 'U'));
    ASSERT_NOMATCH(test_match("a|b", "cdef", 'U'));

    PASS();
}

TEST(t_match_star_groups_none) {
    ASSERT_MATCH(test_match("a*", "", 'S'));
    ASSERT_MATCH(test_match("a*", "a", 'S'));
    ASSERT_MATCH(test_match("a*", "aa", 'S'));
    ASSERT_MATCH(test_match("a*", "aab", 'S'));
    ASSERT_MATCH(test_match("a*", "b", 'S'));

    ASSERT_MATCH(test_match("a*", "", 'E'));
    ASSERT_MATCH(test_match("a*", "a", 'E'));
    ASSERT_MATCH(test_match("a*", "aa", 'E'));
    ASSERT_MATCH(test_match("a*", "aab", 'E'));
    ASSERT_MATCH(test_match("a*", "b", 'E'));

    ASSERT_MATCH(test_match("a*", "", 'B'));
    ASSERT_MATCH(test_match("a*", "a", 'B'));
    ASSERT_MATCH(test_match("a*", "aa", 'B'));
    ASSERT_NOMATCH(test_match("a*", "aab", 'B'));
    ASSERT_NOMATCH(test_match("a*", "b", 'B'));

    ASSERT_MATCH(test_match("a*", "", 'U'));
    ASSERT_MATCH(test_match("a*", "a", 'U'));
    ASSERT_MATCH(test_match("a*", "aa", 'U'));
    ASSERT_MATCH(test_match("a*", "aab", 'U'));
    ASSERT_MATCH(test_match("a*", "b", 'U'));

    PASS();
}

TEST(t_match_plus_groups_none) {
    ASSERT_MATCH(test_match("a+", "a", 'S'));
    ASSERT_MATCH(test_match("a+", "aa", 'S'));
    ASSERT_MATCH(test_match("a+", "aab", 'S'));
    ASSERT_NOMATCH(test_match("a+", "baa", 'S'));
    ASSERT_NOMATCH(test_match("a+", "", 'S'));
    ASSERT_NOMATCH(test_match("a+", "b", 'S'));

    ASSERT_MATCH(test_match("a+", "a", 'E'));
    ASSERT_MATCH(test_match("a+", "aa", 'E'));
    ASSERT_NOMATCH(test_match("a+", "aab", 'E'));
    ASSERT_MATCH(test_match("a+", "baa", 'E'));
    ASSERT_NOMATCH(test_match("a+", "", 'E'));
    ASSERT_NOMATCH(test_match("a+", "b", 'E'));

    ASSERT_MATCH(test_match("a+", "a", 'B'));
    ASSERT_MATCH(test_match("a+", "aa", 'B'));
    ASSERT_NOMATCH(test_match("a+", "aab", 'B'));
    ASSERT_NOMATCH(test_match("a+", "baa", 'B'));
    ASSERT_NOMATCH(test_match("a+", "", 'B'));
    ASSERT_NOMATCH(test_match("a+", "b", 'B'));

    ASSERT_MATCH(test_match("a+", "a", 'U'));
    ASSERT_MATCH(test_match("a+", "aa", 'U'));
    ASSERT_MATCH(test_match("a+", "aab", 'U'));
    ASSERT_MATCH(test_match("a+", "baa", 'U'));
    ASSERT_NOMATCH(test_match("a+", "", 'U'));
    ASSERT_NOMATCH(test_match("a+", "b", 'U'));

    PASS();
}

TEST(t_match_question_groups_none) {
    ASSERT_MATCH(test_match("a?", "", 'S'));
    ASSERT_MATCH(test_match("a?", "a", 'S'));
    ASSERT_MATCH(test_match("a?", "aa", 'S'));
    ASSERT_MATCH(test_match("a?", "b", 'S'));
    ASSERT_MATCH(test_match("a?", "ba", 'S'));
    ASSERT_MATCH(test_match("a?", "ab", 'S'));
    ASSERT_MATCH(test_match("a?", "baa", 'S'));
    ASSERT_MATCH(test_match("a?", "aab", 'S'));

    ASSERT_MATCH(test_match("a?", "", 'E'));
    ASSERT_MATCH(test_match("a?", "a", 'E'));
    ASSERT_MATCH(test_match("a?", "aa", 'E'));
    ASSERT_MATCH(test_match("a?", "b", 'E'));
    ASSERT_MATCH(test_match("a?", "ba", 'E'));
    ASSERT_MATCH(test_match("a?", "ab", 'E'));
    ASSERT_MATCH(test_match("a?", "baa", 'E'));
    ASSERT_MATCH(test_match("a?", "aab", 'E'));

    ASSERT_MATCH(test_match("a?", "", 'B'));
    ASSERT_MATCH(test_match("a?", "a", 'B'));
    ASSERT_MATCH(test_match("a?", "aa", 'B'));
    ASSERT_NOMATCH(test_match("a?", "b", 'B'));
    ASSERT_NOMATCH(test_match("a?", "ba", 'B'));
    ASSERT_NOMATCH(test_match("a?", "ab", 'B'));
    ASSERT_NOMATCH(test_match("a?", "baa", 'B'));
    ASSERT_NOMATCH(test_match("a?", "aab", 'B'));

    ASSERT_MATCH(test_match("a?", "", 'U'));
    ASSERT_MATCH(test_match("a?", "a", 'U'));
    ASSERT_MATCH(test_match("a?", "aa", 'U'));
    ASSERT_MATCH(test_match("a?", "b", 'U'));
    ASSERT_MATCH(test_match("a?", "ba", 'U'));
    ASSERT_MATCH(test_match("a?", "ab", 'U'));
    ASSERT_MATCH(test_match("a?", "baa", 'U'));
    ASSERT_MATCH(test_match("a?", "aab", 'U'));

    PASS();
}

TEST(t_match_quantifier_groups_none) {
    ASSERT_MATCH(test_match("a{1,2}", "a", 'S'));
    ASSERT_MATCH(test_match("a{1,2}", "aa", 'S'));
    ASSERT_NOMATCH(test_match("a{1,2}", "", 'S'));
    ASSERT_MATCH(test_match("a{1,2}", "ab", 'S'));
    ASSERT_MATCH(test_match("a{1,2}", "aab", 'S'));
    ASSERT_NOMATCH(test_match("a{1,2}", "ba", 'S'));
    ASSERT_NOMATCH(test_match("a{1,2}", "baa", 'S'));

    ASSERT_MATCH(test_match("a{1,2}", "a", 'E'));
    ASSERT_MATCH(test_match("a{1,2}", "aa", 'E'));
    ASSERT_NOMATCH(test_match("a{1,2}", "", 'E'));
    ASSERT_NOMATCH(test_match("a{1,2}", "ab", 'E'));
    ASSERT_NOMATCH(test_match("a{1,2}", "aab", 'E'));
    ASSERT_MATCH(test_match("a{1,2}", "ba", 'E'));
    ASSERT_MATCH(test_match("a{1,2}", "baa", 'E'));

    ASSERT_MATCH(test_match("a{1,2}", "a", 'B'));
    ASSERT_MATCH(test_match("a{1,2}", "aa", 'B'));
    ASSERT_NOMATCH(test_match("a{1,2}", "", 'B'));
    ASSERT_NOMATCH(test_match("a{1,2}", "ab", 'B'));
    ASSERT_NOMATCH(test_match("a{1,2}", "aab", 'B'));
    ASSERT_NOMATCH(test_match("a{1,2}", "ba", 'B'));
    ASSERT_NOMATCH(test_match("a{1,2}", "baa", 'B'));

    ASSERT_MATCH(test_match("a{1,2}", "a", 'U'));
    ASSERT_MATCH(test_match("a{1,2}", "aa", 'U'));
    ASSERT_NOMATCH(test_match("a{1,2}", "", 'U'));
    ASSERT_MATCH(test_match("a{1,2}", "ab", 'U'));
    ASSERT_MATCH(test_match("a{1,2}", "aab", 'U'));
    ASSERT_MATCH(test_match("a{1,2}", "ba", 'U'));
    ASSERT_MATCH(test_match("a{1,2}", "baa", 'U'));

    PASS();
}

TEST(t_match_dot_groups_none) {
    ASSERT_MATCH(test_match(".", "a", 'S'));
    ASSERT_MATCH(test_match(".", "aa", 'S'));
    ASSERT_MATCH(test_match(".", "z", 'S'));
    ASSERT_MATCH(test_match(".", EX_UTF8_VALID_1, 'S'));
    ASSERT_MATCH(test_match(".", EX_UTF8_VALID_2, 'S'));
    ASSERT_MATCH(test_match(".", EX_UTF8_VALID_3, 'S'));
    ASSERT_MATCH(test_match(".", EX_UTF8_VALID_4, 'S'));
    ASSERT_NOMATCH(test_match(".", "", 'S'));
    ASSERT_NOMATCH(test_match(".", EX_UTF8_INVALID_OVERLONG, 'S'));
    ASSERT_NOMATCH(test_match(".", EX_UTF8_INVALID_TOOBIG, 'S'));
    ASSERT_NOMATCH(test_match(".", EX_UTF8_INVALID_SURROGATE, 'S'));

    ASSERT_MATCH(test_match(".", "a", 'E'));
    ASSERT_MATCH(test_match(".", "aa", 'E'));
    ASSERT_MATCH(test_match(".", "z", 'E'));
    ASSERT_MATCH(test_match(".", EX_UTF8_VALID_1, 'E'));
    ASSERT_MATCH(test_match(".", EX_UTF8_VALID_2, 'E'));
    ASSERT_MATCH(test_match(".", EX_UTF8_VALID_3, 'E'));
    ASSERT_MATCH(test_match(".", EX_UTF8_VALID_4, 'E'));
    ASSERT_NOMATCH(test_match(".", "", 'E'));
    ASSERT_NOMATCH(test_match(".", EX_UTF8_INVALID_OVERLONG, 'E'));
    ASSERT_NOMATCH(test_match(".", EX_UTF8_INVALID_TOOBIG, 'E'));
    ASSERT_NOMATCH(test_match(".", EX_UTF8_INVALID_SURROGATE, 'E'));

    ASSERT_MATCH(test_match(".", "a", 'B'));
    ASSERT_NOMATCH(test_match(".", "aa", 'B'));
    ASSERT_MATCH(test_match(".", "z", 'B'));
    ASSERT_MATCH(test_match(".", EX_UTF8_VALID_1, 'B'));
    ASSERT_MATCH(test_match(".", EX_UTF8_VALID_2, 'B'));
    ASSERT_MATCH(test_match(".", EX_UTF8_VALID_3, 'B'));
    ASSERT_MATCH(test_match(".", EX_UTF8_VALID_4, 'B'));
    ASSERT_NOMATCH(test_match(".", "", 'B'));
    ASSERT_NOMATCH(test_match(".", EX_UTF8_INVALID_OVERLONG, 'B'));
    ASSERT_NOMATCH(test_match(".", EX_UTF8_INVALID_TOOBIG, 'B'));
    ASSERT_NOMATCH(test_match(".", EX_UTF8_INVALID_SURROGATE, 'B'));

    ASSERT_MATCH(test_match(".", "a", 'U'));
    ASSERT_MATCH(test_match(".", "aa", 'U'));
    ASSERT_MATCH(test_match(".", "z", 'U'));
    ASSERT_MATCH(test_match(".", EX_UTF8_VALID_1, 'U'));
    ASSERT_MATCH(test_match(".", EX_UTF8_VALID_2, 'U'));
    ASSERT_MATCH(test_match(".", EX_UTF8_VALID_3, 'U'));
    ASSERT_MATCH(test_match(".", EX_UTF8_VALID_4, 'U'));
    ASSERT_NOMATCH(test_match(".", "", 'U'));
    ASSERT_NOMATCH(test_match(".", EX_UTF8_INVALID_OVERLONG, 'U'));
    ASSERT_NOMATCH(test_match(".", EX_UTF8_INVALID_TOOBIG, 'U'));
    ASSERT_NOMATCH(test_match(".", EX_UTF8_INVALID_SURROGATE, 'U'));

    PASS();
}

TEST(t_match_begin_text_groups_none) {
    ASSERT_MATCH(test_match("^a", "a", 'S'));
    ASSERT_MATCH(test_match("^a", "aa", 'S'));
    ASSERT_MATCH(test_match("^a", "ab", 'S'));
    ASSERT_NOMATCH(test_match("^a", "", 'S'));
    ASSERT_NOMATCH(test_match("^a", "ba", 'S'));

    ASSERT_MATCH(test_match("^a", "a", 'E'));
    ASSERT_NOMATCH(test_match("^a", "aa", 'E'));
    ASSERT_NOMATCH(test_match("^a", "ab", 'E'));
    ASSERT_NOMATCH(test_match("^a", "", 'E'));
    ASSERT_NOMATCH(test_match("^a", "ba", 'E'));

    ASSERT_MATCH(test_match("^a", "a", 'B'));
    ASSERT_NOMATCH(test_match("^a", "aa", 'B'));
    ASSERT_NOMATCH(test_match("^a", "ab", 'B'));
    ASSERT_NOMATCH(test_match("^a", "", 'B'));
    ASSERT_NOMATCH(test_match("^a", "ba", 'B'));

    ASSERT_MATCH(test_match("^a", "a", 'U'));
    ASSERT_MATCH(test_match("^a", "aa", 'U'));
    ASSERT_MATCH(test_match("^a", "ab", 'U'));
    ASSERT_NOMATCH(test_match("^a", "", 'U'));
    ASSERT_NOMATCH(test_match("^a", "ba", 'U'));

    PASS();
}

TEST(t_match_end_text_groups_none) {
    ASSERT_MATCH(test_match("a$", "a", 'S'));
    ASSERT_NOMATCH(test_match("a$", "aa", 'S'));
    ASSERT_NOMATCH(test_match("a$", "ab", 'S'));
    ASSERT_NOMATCH(test_match("a$", "", 'S'));
    ASSERT_NOMATCH(test_match("a$", "ba", 'S'));

    ASSERT_MATCH(test_match("a$", "a", 'E'));
    ASSERT_MATCH(test_match("a$", "aa", 'E'));
    ASSERT_NOMATCH(test_match("a$", "ab", 'E'));
    ASSERT_NOMATCH(test_match("a$", "", 'E'));
    ASSERT_MATCH(test_match("a$", "ba", 'E'));

    ASSERT_MATCH(test_match("a$", "a", 'B'));
    ASSERT_NOMATCH(test_match("a$", "aa", 'B'));
    ASSERT_NOMATCH(test_match("a$", "ab", 'B'));
    ASSERT_NOMATCH(test_match("a$", "", 'B'));
    ASSERT_NOMATCH(test_match("a$", "ba", 'B'));

    ASSERT_MATCH(test_match("a$", "a", 'U'));
    ASSERT_MATCH(test_match("a$", "aa", 'U'));
    ASSERT_NOMATCH(test_match("a$", "ab", 'U'));
    ASSERT_NOMATCH(test_match("a$", "", 'U'));
    ASSERT_MATCH(test_match("a$", "ba", 'U'));

    PASS();
}

TEST(t_match_word_bound_groups_none) {
    ASSERT_NOMATCH(test_match("\\b", "", 'S'));
    ASSERT_MATCH(test_match("\\b", "a", 'S'));
    ASSERT_NOMATCH(test_match("\\b", " ", 'S'));
    ASSERT_NOMATCH(test_match("\\b", "#", 'S'));
    ASSERT_MATCH(test_match("a\\b", "a", 'S'));
    ASSERT_NOMATCH(test_match("a\\b", "aa", 'S'));
    ASSERT_MATCH(test_match("a\\b", "a ", 'S'));
    ASSERT_MATCH(test_match("a\\b", "a#", 'S'));
    ASSERT_NOMATCH(test_match("a\\b", "", 'S'));

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

TEST(t_match_word_bound_not_groups_none) {
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

SUITE(s_match) {
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

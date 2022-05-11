#include "test_match.h"

static int test_match(const char* regex, const char* text, char anchor) {
    re_match_anchor_type anchor_type = RE_MATCH_UNANCHORED;
    mn__str_view text_view;
    re reg;
    re_error rv = 0;
    mn__str_view_init_s(&text_view, text);
    if (anchor == 'S') {
        anchor_type = RE_MATCH_ANCHOR_START;
    }
    if (anchor == 'E') {
        anchor_type = RE_MATCH_ANCHOR_END;
    }
    if (anchor == 'B') {
        anchor_type = RE_MATCH_ANCHOR_BOTH;
    }
    if (anchor == 'U') {
        anchor_type = RE_MATCH_UNANCHORED;
    }
    ASSERT(!re_init(&reg, regex));
    rv = re_match(&reg, anchor_type, RE_MATCH_GROUPS_NONE, text, mn__str_view_size(&text_view), MN_NULL);
    if (rv != RE_MATCH) {
        ASSERT(rv == RE_NOMATCH);
    }
    re_destroy(&reg);
    return rv == RE_MATCH;
}

TEST(t_match_one_character_groups_none) {
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

SUITE(s_match) {
    RUN_TEST(t_match_one_character_groups_none);
}

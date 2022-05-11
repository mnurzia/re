#include "test_match.h"

TEST(t_match_one_character_anchored_both_groups_none) {
    re reg;
    re_span spans[1];
    ASSERT(!re_init(&reg, "a"));
    ASSERT_EQ(
        re_match(
            &reg, 
            RE_MATCH_ANCHOR_BOTH, 
            RE_MATCH_GROUPS_NONE, 
            "a", 1, 
            spans
        ),
        RE_MATCH
    );
    ASSERT_EQ(
        re_match(
            &reg, 
            RE_MATCH_ANCHOR_BOTH, 
            RE_MATCH_GROUPS_NONE, 
            "b", 1, 
            spans
        ),
        RE_NOMATCH
    );
    ASSERT_EQ(
        re_match(
            &reg, 
            RE_MATCH_ANCHOR_BOTH, 
            RE_MATCH_GROUPS_NONE, 
            "bc", 1, 
            spans
        ),
        RE_NOMATCH
    );
    re_destroy(&reg);
    PASS();
}

TEST(t_match_one_character_anchored_start_groups_none) {
    re reg;
    re_span spans[1];
    ASSERT(!re_init(&reg, "a"));
    ASSERT_EQ(
        re_match(
            &reg, 
            RE_MATCH_ANCHOR_START, 
            RE_MATCH_GROUPS_NONE, 
            "a", 1, 
            spans
        ),
        RE_MATCH
    );
    ASSERT_EQ(
        re_match(
            &reg, 
            RE_MATCH_ANCHOR_START, 
            RE_MATCH_GROUPS_NONE, 
            "abc", 1, 
            spans
        ),
        RE_MATCH
    );
    ASSERT_EQ(
        re_match(
            &reg, 
            RE_MATCH_ANCHOR_START, 
            RE_MATCH_GROUPS_NONE, 
            "ba", 1, 
            spans
        ),
        RE_NOMATCH
    );
    re_destroy(&reg);
    PASS();
}

SUITE(s_match) {
    RUN_TEST(t_match_one_character_anchored_both_groups_none);
    RUN_TEST(t_match_one_character_anchored_start_groups_none);
}

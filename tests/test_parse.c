#include "test_parse.h"

#include "test_ast.h"

TEST(t_parse_empty) {
    re re;
    ASSERT(!re_init(&re, ""));
    ASSERT_SYMEQm(
        re__ast_root,
        re.data->parse.ast_root,
        "(ast)",
        "empty regex should return an empty ast"
    );
    re_destroy(&re);
    PASS();
}

TEST(t_parse_text_end) {
    re re;
    ASSERT(!re_init(&re, "$"));
    ASSERT_SYMEQ(
        re__ast_root, 
        re.data->parse.ast_root, 
        "(ast"
        "   (assert (text_end)))"
    );
    re_destroy(&re);
    PASS();
}

TEST(t_parse_group) {
    re re;
    ASSERT(!re_init(&re, "(a)"));
    ASSERT_SYMEQ(
        re__ast_root,
        re.data->parse.ast_root,
        "(ast"
        "   (group () (rune 'a')))"
    );
    re_destroy(&re);
    PASS();
}

TEST(t_parse_group_unfinished) {
    re re;
    ASSERT_EQ(re_init(&re, "("), RE_ERROR_PARSE);
    re_destroy(&re);
    ASSERT_EQ(re_init(&re, "(a"), RE_ERROR_PARSE);
    re_destroy(&re);
    ASSERT_EQ(re_init(&re, "(a("), RE_ERROR_PARSE);
    re_destroy(&re);
    ASSERT_EQ(re_init(&re, "(a(b(())("), RE_ERROR_PARSE);
    re_destroy(&re);
    PASS();
}

TEST(t_parse_group_unmatched) {
    re re;
    ASSERT_EQ(re_init(&re, ")"), RE_ERROR_PARSE);
    re_destroy(&re);
    ASSERT_EQ(re_init(&re, "a)"), RE_ERROR_PARSE);
    re_destroy(&re);
    ASSERT_EQ(re_init(&re, "())"), RE_ERROR_PARSE);
    re_destroy(&re);
    ASSERT_EQ(re_init(&re, "(aa((()a))))"), RE_ERROR_PARSE);
    re_destroy(&re);
    PASS();
}


TEST(t_parse_group_balance) {
    re__str reg;
    int balance = 0;
    int stack = 0;
    int iters = 0;
    re__str_init(&reg);
    while (iters < 100) {
        int choice = RAND_PARAM(16);
        if (choice < 5) {
            re__str_cat_n(&reg, "(", 1);
            stack++;
        } else if (choice < 10) {
            re__str_cat_n(&reg, ")", 1);
            if (stack == 0) {
                balance = -1;
            } else {
                stack--;
            }
        } else if (choice < 15) {
            re_char ch = RAND_PARAM(26) + 'A';
            re__str_cat_n(&reg, &ch, 1);    
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
        re re;
        int res = re_init(&re, re__str_get_data(&reg));
        if (balance != 0) {
            ASSERT_EQ(res, RE_ERROR_PARSE);
        } else {
            ASSERT_EQ(res, 0);
        }
        re_destroy(&re);
    }
    re__str_destroy(&reg);
    PASS();
}

TEST(t_parse_star) {
    re re;
    ASSERT(!re_init(&re, "a*"));
    ASSERT_SYMEQ(
        re__ast_root,
        re.data->parse.ast_root,
        "(ast"
        "   (quantifier 0 inf"
        "       (rune 'a')))"
    );
    re_destroy(&re);
    PASS();
}

SUITE(s_parse) {
    RUN_TEST(t_parse_empty);
    RUN_TEST(t_parse_text_end);
    RUN_TEST(t_parse_group);
    RUN_TEST(t_parse_group_unfinished);
    RUN_TEST(t_parse_group_unmatched);
    FUZZ_TEST(t_parse_group_balance);
    RUN_TEST(t_parse_star);
}

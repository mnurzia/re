#include "test_parse.h"

#include "test_ast.h"

TEST(t_parse_empty) {
    re re;
    ASSERTm(!re_init(&re, ""),
        "empty regex should compile");
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
    ASSERT_SYMEQm(
        re__ast_root, 
        re.data->parse.ast_root, 
        "(ast"
        "   (assert (text_end)))",
        "$ should create a text_end ast"
    );
    re_destroy(&re);
    PASS();
}

TEST(t_parse_group) {
    re re;
    ASSERT(!re_init(&re, "(a)"));
    ASSERT_SYMEQm(
        re__ast_root,
        re.data->parse.ast_root,
        "(ast"
        "   (group () (rune 'a')))",
        "group should create a group"
    );
    re_destroy(&re);
    PASS();
}

TEST(t_parse_group_unfinished) {
    re re;
    ASSERT_EQm(re_init(&re, "("), RE_ERROR_PARSE,
        "error for unfinished group");
    re_destroy(&re);
    ASSERT_EQm(re_init(&re, "(a"), RE_ERROR_PARSE,
        "error for unfinished group with contents");
    re_destroy(&re);
    ASSERT_EQm(re_init(&re, "(a("), RE_ERROR_PARSE,
        "error for unfinished nested group");
    re_destroy(&re);
    ASSERT_EQm(re_init(&re, "(a(b(())("), RE_ERROR_PARSE,
        "error for unfinished complex nested group");
    re_destroy(&re);
    PASS();
}

TEST(t_parse_group_unmatched) {
    re re;
    ASSERT_EQm(re_init(&re, ")"), RE_ERROR_PARSE,
        "error for unmatched )");
    re_destroy(&re);
    ASSERT_EQm(re_init(&re, "a)"), RE_ERROR_PARSE,
        "error for unmatched ) after contents");
    re_destroy(&re);
    ASSERT_EQm(re_init(&re, "())"), RE_ERROR_PARSE,
        "error for unmatched ) after group");
    re_destroy(&re);
    ASSERT_EQm(re_init(&re, "(aa((()a))))"), RE_ERROR_PARSE,
        "error for unmatched ) after complex nested groups");
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
            ASSERT_EQm(res, RE_ERROR_PARSE,
                "error for arbitrary unbalanced group");
        } else {
            ASSERT_EQm(res, 0,
                "error for arbitrary balanced group");
        }
        re_destroy(&re);
    }
    re__str_destroy(&reg);
    PASS();
}

TEST(t_parse_star) {
    re re;
    ASSERT(!re_init(&re, "a*"));
    ASSERT_SYMEQm(
        re__ast_root,
        re.data->parse.ast_root,
        "(ast"
        "   (quantifier 0 inf greedy"
        "       (rune 'a')))",
        "* operator should wrap previous nodes with quantifier"
    );
    re_destroy(&re);
    ASSERT(!re_init(&re, "a*?"));
    ASSERT_SYMEQm(
        re__ast_root,
        re.data->parse.ast_root,
        "(ast"
        "   (quantifier 0 inf nongreedy"
        "       (rune 'a')))",
        "*? operator should non-greedily wrap previous nodes with quantifier"
    );
    re_destroy(&re);
    PASS();
}

TEST(t_parse_question) {
    re re;
    ASSERT(!re_init(&re, "a?"));
    ASSERT_SYMEQ(
        re__ast_root,
        re.data->parse.ast_root,
        "(ast"
        "   (quantifier 0 2 greedy"
        "       (rune 'a')))"
    );
    re_destroy(&re);
    ASSERT(!re_init(&re, "a??"));
    ASSERT_SYMEQ(
        re__ast_root,
        re.data->parse.ast_root,
        "(ast"
        "   (quantifier 0 2 nongreedy"
        "       (rune 'a')))"
    );
    re_destroy(&re);
    PASS();
}

TEST(t_parse_plus) {
    re re;
    ASSERT(!re_init(&re, "a+"));
    ASSERT_SYMEQ(
        re__ast_root,
        re.data->parse.ast_root,
        "(ast"
        "   (quantifier 1 inf greedy"
        "       (rune 'a')))"
    );
    re_destroy(&re);
    ASSERT(!re_init(&re, "a+?"));
    ASSERT_SYMEQ(
        re__ast_root,
        re.data->parse.ast_root,
        "(ast"
        "   (quantifier 1 inf nongreedy"
        "       (rune 'a')))"
    );
    re_destroy(&re);
    PASS();
}

TEST(t_parse_any_char) {
    re re;
    ASSERT(!re_init(&re, "."));
    ASSERT_SYMEQ(
        re__ast_root,
        re.data->parse.ast_root,
        "(ast"
        "   (any_char))"
    );
    re_destroy(&re);
    PASS();
}

TEST(t_parse_charclass_one) {
    re re;
    ASSERT(!re_init(&re, "[a]"));
    ASSERT_SYMEQ(
        re__ast_root,
        re.data->parse.ast_root,
        "(ast"
        "   (charclass ((rune_range 'a' 'a'))))"
    );
    re_destroy(&re);
    PASS();
}

TEST(t_parse_charclass_lbracket) {
    re re;
    ASSERT(!re_init(&re, "[[]"));
    ASSERT_SYMEQ(
        re__ast_root,
        re.data->parse.ast_root,
        "(ast"
        "   (charclass ((rune_range '[' '['))))"
    );
    re_destroy(&re);
    PASS();
}

TEST(t_parse_charclass_rbracket) {
    re re;
    ASSERT(!re_init(&re, "[]]"));
    ASSERT_SYMEQ(
        re__ast_root,
        re.data->parse.ast_root,
        "(ast"
        "   (charclass ((rune_range ']' ']'))))"
    );
    re_destroy(&re);
    PASS();
}

TEST(t_parse_charclass_hyphen) {
    re re;
    ASSERT(!re_init(&re, "[-]"));
    ASSERT_SYMEQ(
        re__ast_root,
        re.data->parse.ast_root,
        "(ast"
        "   (charclass ((rune_range '-' '-'))))"
    );
    re_destroy(&re);
    ASSERT(!re_init(&re, "[a-]"));
    ASSERT_SYMEQ(
        re__ast_root,
        re.data->parse.ast_root,
        "(ast"
        "   (charclass ("
        "       (rune_range '-' '-')"
        "       (rune_range 'a' 'a'))))"
    );
    re_destroy(&re);
    PASS();
}

TEST(t_parse_charclass_unfinished) {
    re re;
    ASSERT_EQm(re_init(&re, "["), RE_ERROR_PARSE,
        "error if charclass is missing ]");
    re_destroy(&re);
    ASSERT_EQm(re_init(&re, "[["), RE_ERROR_PARSE,
        "error if charclass with open bracket is missing ]");
    re_destroy(&re);
    ASSERT_EQm(re_init(&re, "[]"), RE_ERROR_PARSE,
        "error if charclass with close bracket is missing ]");
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
    RUN_TEST(t_parse_question);
    RUN_TEST(t_parse_plus);
    RUN_TEST(t_parse_any_char);
    RUN_TEST(t_parse_charclass_one);
    RUN_TEST(t_parse_charclass_rbracket);
    RUN_TEST(t_parse_charclass_hyphen);
    RUN_TEST(t_parse_charclass_unfinished);
}
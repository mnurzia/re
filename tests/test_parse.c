#include "test_parse.h"

#include "test_ast.h"
#include "test_helpers.h"

TEST(t_parse_empty) {
    re reg;
    ASSERTm(!re_init(&reg, ""),
        "empty regex should compile");
    ASSERT_SYMEQm(
        re__ast_root,
        reg.data->ast_root,
        "(ast)",
        "empty regex should return an empty ast"
    );
    re_destroy(&reg);
    PASS();
}

TEST(t_parse_text_end) {
    re reg;
    ASSERT(!re_init(&reg, "$"));
    ASSERT_SYMEQm(
        re__ast_root, 
        reg.data->ast_root, 
        "(ast"
        "   (assert (text_end)))",
        "$ should create a text_end ast"
    );
    re_destroy(&reg);
    PASS();
}

TEST(t_parse_group) {
    re reg;
    ASSERT(!re_init(&reg, "(a)"));
    ASSERT_SYMEQm(
        re__ast_root,
        reg.data->ast_root,
        "(ast"
        "   (group () 0 (rune 'a')))",
        "group should create a group"
    );
    re_destroy(&reg);
    PASS();
}

TEST(t_parse_group_unfinished) {
    re reg;
    ASSERT_EQm(re_init(&reg, "("), RE_ERROR_PARSE,
        "error for unfinished group");
    re_destroy(&reg);
    ASSERT_EQm(re_init(&reg, "(a"), RE_ERROR_PARSE,
        "error for unfinished group with contents");
    re_destroy(&reg);
    ASSERT_EQm(re_init(&reg, "(a("), RE_ERROR_PARSE,
        "error for unfinished nested group");
    re_destroy(&reg);
    ASSERT_EQm(re_init(&reg, "(a(b(())("), RE_ERROR_PARSE,
        "error for unfinished complex nested group");
    re_destroy(&reg);
    PASS();
}

TEST(t_parse_group_unmatched) {
    re reg;
    ASSERT_EQm(re_init(&reg, ")"), RE_ERROR_PARSE,
        "error for unmatched )");
    re_destroy(&reg);
    ASSERT_EQm(re_init(&reg, "a)"), RE_ERROR_PARSE,
        "error for unmatched ) after contents");
    re_destroy(&reg);
    ASSERT_EQm(re_init(&reg, "())"), RE_ERROR_PARSE,
        "error for unmatched ) after group");
    re_destroy(&reg);
    ASSERT_EQm(re_init(&reg, "(aa((()a))))"), RE_ERROR_PARSE,
        "error for unmatched ) after complex nested groups");
    re_destroy(&reg);
    PASS();
}

TEST(t_parse_groups) {
    re reg;
    ASSERT(!re_init(&reg, "(a)(b)"));
    ASSERT_SYMEQm(
        re__ast_root,
        reg.data->ast_root,
        "(ast"
        "    (concat"
        "        ("
        "            (group () 0 (rune 'a'))"
        "            (group () 1 (rune 'b'))))",
        "two adjacent groups should create a concat with two groups"
    );
    re_destroy(&reg);
    ASSERT(!re_init(&reg, "(a)(b)(c)"));
    ASSERT_SYMEQm(
        re__ast_root,
        reg.data->ast_root,
        "(ast"
        "    (concat"
        "        ("
        "            (group () 0 (rune 'a'))"
        "            (group () 1 (rune 'b'))"
        "            (group () 2 (rune 'c'))))",
        "three adjacent groups should create a concat with three groups"
    );
    re_destroy(&reg);
    PASS();
}

TEST(t_parse_group_balance) {
    re__str regs;
    int balance = 0;
    int stack = 0;
    int iters = 0;
    re__str_init(&regs);
    while (iters < 100) {
        int choice = RAND_PARAM(16);
        if (choice < 5) {
            re__str_cat_n(&regs, "(", 1);
            stack++;
        } else if (choice < 10) {
            re__str_cat_n(&regs, ")", 1);
            if (stack == 0) {
                balance = -1;
            } else {
                stack--;
            }
        } else if (choice < 15) {
            re_char ch = (re_char)RAND_PARAM(26) + 'A';
            re__str_cat_n(&regs, &ch, 1);    
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
        int res = re_init(&reg, re__str_get_data(&regs));
        if (balance != 0) {
            ASSERT_EQm(res, RE_ERROR_PARSE,
                "error for arbitrary unbalanced group");
        } else {
            ASSERT_EQm(res, 0,
                "error for arbitrary balanced group");
        }
        re_destroy(&reg);
    }
    re__str_destroy(&regs);
    PASS();
}

TEST(t_parse_group_named) {
    re reg;
    ASSERT(!re_init(&reg, "(?<name>a)"));
    ASSERT_SYMEQ(
        re__ast_root,
        reg.data->ast_root,
        "(ast"
        "  (group (named) 0 name"
        "    (rune 'a')))"
    );
    re_destroy(&reg);
    PASS();
}

TEST(t_parse_star) {
    re reg;
    ASSERT(!re_init(&reg, "a*"));
    ASSERT_SYMEQm(
        re__ast_root,
        reg.data->ast_root,
        "(ast"
        "   (quantifier 0 inf greedy"
        "       (rune 'a')))",
        "* operator should wrap previous nodes with quantifier"
    );
    re_destroy(&reg);
    ASSERT(!re_init(&reg, "a*?"));
    ASSERT_SYMEQm(
        re__ast_root,
        reg.data->ast_root,
        "(ast"
        "   (quantifier 0 inf nongreedy"
        "       (rune 'a')))",
        "*? operator should non-greedily wrap previous nodes with quantifier"
    );
    re_destroy(&reg);
    PASS();
}

TEST(t_parse_question) {
    re reg;
    ASSERT(!re_init(&reg, "a?"));
    ASSERT_SYMEQ(
        re__ast_root,
        reg.data->ast_root,
        "(ast"
        "   (quantifier 0 2 greedy"
        "       (rune 'a')))"
    );
    re_destroy(&reg);
    ASSERT(!re_init(&reg, "a??"));
    ASSERT_SYMEQ(
        re__ast_root,
        reg.data->ast_root,
        "(ast"
        "   (quantifier 0 2 nongreedy"
        "       (rune 'a')))"
    );
    re_destroy(&reg);
    PASS();
}

TEST(t_parse_plus) {
    re reg;
    ASSERT(!re_init(&reg, "a+"));
    ASSERT_SYMEQ(
        re__ast_root,
        reg.data->ast_root,
        "(ast"
        "   (quantifier 1 inf greedy"
        "       (rune 'a')))"
    );
    re_destroy(&reg);
    ASSERT(!re_init(&reg, "a+?"));
    ASSERT_SYMEQ(
        re__ast_root,
        reg.data->ast_root,
        "(ast"
        "   (quantifier 1 inf nongreedy"
        "       (rune 'a')))"
    );
    re_destroy(&reg);
    PASS();
}

TEST(t_parse_any_char) {
    re reg;
    ASSERT(!re_init(&reg, "."));
    ASSERT_SYMEQ(
        re__ast_root,
        reg.data->ast_root,
        "(ast"
        "   (any_char))"
    );
    re_destroy(&reg);
    PASS();
}

TEST(t_parse_charclass_one) {
    re reg;
    ASSERT(!re_init(&reg, "[a]"));
    ASSERT_SYMEQ(
        re__ast_root,
        reg.data->ast_root,
        "(ast"
        "   (charclass ((rune_range 'a' 'a'))))"
    );
    re_destroy(&reg);
    PASS();
}

TEST(t_parse_charclass_lbracket) {
    re reg;
    ASSERT(!re_init(&reg, "[[]"));
    ASSERT_SYMEQ(
        re__ast_root,
        reg.data->ast_root,
        "(ast"
        "   (charclass ((rune_range '[' '['))))"
    );
    re_destroy(&reg);
    PASS();
}

TEST(t_parse_charclass_rbracket) {
    re reg;
    ASSERT(!re_init(&reg, "[]]"));
    ASSERT_SYMEQ(
        re__ast_root,
        reg.data->ast_root,
        "(ast"
        "   (charclass ((rune_range ']' ']'))))"
    );
    re_destroy(&reg);
    PASS();
}

TEST(t_parse_charclass_hyphen) {
    re reg;
    ASSERT(!re_init(&reg, "[-]"));
    ASSERT_SYMEQm(
        re__ast_root,
        reg.data->ast_root,
        "(ast"
        "   (charclass ((rune_range '-' '-'))))",
        "charclass with only hyphen should just include a hyphen rune"
    );
    re_destroy(&reg);
    ASSERT(!re_init(&reg, "[a-]"));
    ASSERT_SYMEQm(
        re__ast_root,
        reg.data->ast_root,
        "(ast"
        "   (charclass ("
        "       (rune_range '-' '-')"
        "       (rune_range 'a' 'a'))))",
        "charclass ending with hyphen should include a hyphen rune"
    );
    re_destroy(&reg);
    ASSERT(!re_init(&reg, "[a-z-]"));
    ASSERT_SYMEQm(
        re__ast_root,
        reg.data->ast_root,
        "(ast"
        "   (charclass ("
        "       (rune_range '-' '-')"
        "       (rune_range 'a' 'z'))))",
        "charclass ending with hyphen should include a hyphen rune"
    );
    re_destroy(&reg);
    ASSERT(!re_init(&reg, "[a-z]"));
    ASSERT_SYMEQm(
        re__ast_root,
        reg.data->ast_root,
        "(ast"
        "   (charclass ("
        "       (rune_range 'a' 'z')))",
        "charclass with middle hyphen should include a rune range"
    );
    re_destroy(&reg);
    PASS();
}

TEST(t_parse_charclass_unfinished) {
    re reg;
    /* RE__PARSE_STATE_CHARCLASS_INITIAL */
    ASSERT_EQm(re_init(&reg, "["), RE_ERROR_PARSE,
        "error if charclass is missing ]");
    re_destroy(&reg);
    /* RE__PARSE_STATE_CHARCLASS_AFTER_BRACKET */
    ASSERT_EQm(re_init(&reg, "[["), RE_ERROR_PARSE,
        "error if charclass with open bracket is missing ]");
    re_destroy(&reg);
    /* RE__PARSE_STATE_CHARCLASS_AFTER_LO */
    ASSERT_EQm(re_init(&reg, "[]"), RE_ERROR_PARSE,
        "error if charclass with close bracket is missing ]");
    re_destroy(&reg);
    /* RE__PARSE_STATE_CHARCLASS_AFTER_CARET */
    ASSERT_EQm(re_init(&reg, "[^"), RE_ERROR_PARSE,
        "error if charclass with caret is missing ]");
    re_destroy(&reg);
    /* RE__PARSE_STATE_CHARCLASS_AFTER_ESCAPE */
    ASSERT_EQm(re_init(&reg, "[\\"), RE_ERROR_PARSE,
        "error if charclass with escape is missing ]");
    re_destroy(&reg);
    /* RE__PARSE_STATE_CHARCLASS_AFTER_ESCAPE */
    ASSERT_EQm(re_init(&reg, "[^\\"), RE_ERROR_PARSE,
        "error if charclass with caret and escape is missing ]");
    re_destroy(&reg);
    /* RE__PARSE_STATE_CHARCLASS_AFTER_LO */
    ASSERT_EQm(re_init(&reg, "[\\a"), RE_ERROR_PARSE,
        "error if charclass with escape char is missing ]");
    re_destroy(&reg);
    /* RE__PARSE_STATE_CHARCLASS_AFTER_LO */
    ASSERT_EQm(re_init(&reg, "[^\\a"), RE_ERROR_PARSE,
        "error if charclass with caret and escape is missing ]");
    re_destroy(&reg);
    /* RE__PARSE_STATE_CHARCLASS_AFTER_LO */
    ASSERT_EQm(re_init(&reg, "[a"), RE_ERROR_PARSE,
        "error if charclass with letter is missing ]");
    re_destroy(&reg);
    /* RE__PARSE_STATE_CHARCLASS_HI */
    ASSERT_EQm(re_init(&reg, "[a-"), RE_ERROR_PARSE,
        "error if charclass with unfinished range is missing ]");
    re_destroy(&reg);
    /* RE__PARSE_STATE_CHARCLASS_LO */
    ASSERT_EQm(re_init(&reg, "[a-z"), RE_ERROR_PARSE,
        "error if charclass with range is missing ]");
    re_destroy(&reg);
    /* RE__PARSE_STATE_CHARCLASS_LO */
    ASSERT_EQm(re_init(&reg, "[ab-z"), RE_ERROR_PARSE,
        "error if charclass with letter and range is missing ]");
    re_destroy(&reg);
    /* RE__PARSE_STATE_CHARCLASS_AFTER_LO */
    ASSERT_EQm(re_init(&reg, "[a-yz"), RE_ERROR_PARSE,
        "error if charclass with range and letter is missing ]");
    re_destroy(&reg);
    /* RE__PARSE_STATE_CHARCLASS_LO */
    ASSERT_EQm(re_init(&reg, "[^ab-z"), RE_ERROR_PARSE,
        "error if negated charclass with letter and range is missing ]");
    re_destroy(&reg);
    /* RE__PARSE_STATE_CHARCLASS_LO */
    ASSERT_EQm(re_init(&reg, "[^a-yz"), RE_ERROR_PARSE,
        "error if negated charclass with range and letter is missing ]");
    re_destroy(&reg);
    /* RE__PARSE_STATE_CHARCLASS_LO */
    ASSERT_EQm(re_init(&reg, "[[:alnum:]"), RE_ERROR_PARSE,
        "error if charclass with named charclass is missing ]");
    re_destroy(&reg);
    /* RE__PARSE_STATE_CHARCLASS_NAMED_INITIAL */
    ASSERT_EQm(re_init(&reg, "[[:"), RE_ERROR_PARSE,
        "error if empty named charclass is missing :]");
    re_destroy(&reg);
    /* RE__PARSE_STATE_CHARCLASS_NAMED_INVERTED */
    ASSERT_EQm(re_init(&reg, "[[:^"), RE_ERROR_PARSE,
        "error if named inverted charclass is missing :]");
    re_destroy(&reg);
    /* RE__PARSE_STATE_CHARCLASS_NAMED */
    ASSERT_EQm(re_init(&reg, "[[:aa"), RE_ERROR_PARSE,
        "error if named charclass is missing :]");
    re_destroy(&reg);
    /* RE__PARSE_STATE_CHARCLASS_NAMED */
    ASSERT_EQm(re_init(&reg, "[[:aa:"), RE_ERROR_PARSE,
        "error if named charclass is missing ]");
    re_destroy(&reg);
    /* RE__PARSE_STATE_CHARCLASS_NAMED */
    ASSERT_EQm(re_init(&reg, "[[::"), RE_ERROR_PARSE,
        "error if named charclass is missing ]");
    re_destroy(&reg);
    /* RE__PARSE_STATE_CHARCLASS_NAMED */
    ASSERT_EQm(re_init(&reg, "[[::]"), RE_ERROR_PARSE,
        "error if named charclass is missing ]");
    re_destroy(&reg);
    /* RE__PARSE_STATE_CHARCLASS_NAMED_INVERTED */
    ASSERT_EQm(re_init(&reg, "[[:^aa:"), RE_ERROR_PARSE,
        "error if named charclass is missing ]");
    re_destroy(&reg);
    /* RE__PARSE_STATE_CHARCLASS_NAMED_INVERTED */
    ASSERT_EQm(re_init(&reg, "[[:^aa"), RE_ERROR_PARSE,
        "error if named inverted charclass is missing ]");
    re_destroy(&reg);
    PASS();
}

TEST(t_parse_charclass_inverted) {
    re reg;
    ASSERT(!re_init(&reg, "[^a]"));
    ASSERT_SYMEQ(
        re__ast_root,
        reg.data->ast_root,
        "(ast "
        "    (charclass ((rune_range 0 96) (rune_range 98 0x10FFFF))))"
    );
    re_destroy(&reg);
    ASSERT(!re_init(&reg, "[^a-z]"));
    ASSERT_SYMEQ(
        re__ast_root,
        reg.data->ast_root,
        "(ast "
        "    (charclass ((rune_range 0 96) (rune_range 123 0x10FFFF))))"
    );
    re_destroy(&reg);
    ASSERT(!re_init(&reg, "[^a-zA-Z]"));
    ASSERT_SYMEQ(
        re__ast_root,
        reg.data->ast_root,
        "(ast "
        "    (charclass ((rune_range 0 64)"
        "                (rune_range 91 96)"
        "                (rune_range 123 0x10FFFF))))"
    );
    re_destroy(&reg);
    PASS();
}

TEST(t_parse_charclass_named) {
    re reg;
    ASSERT(!re_init(&reg, "[[:alnum:]]"));
    ASSERT_SYMEQ(
        re__ast_root,
        reg.data->ast_root,
        "(ast "
        "  (charclass "
        "    ("
        "      (rune_range 48 57)"
        "      (rune_range 65 90)"
        "      (rune_range 97 122)))"
    );
    re_destroy(&reg);
    PASS();
}

TEST(t_parse_charclass_named_inverted) {
    re reg;
    ASSERT(!re_init(&reg, "[[:^alnum:]]"));
    ASSERT_SYMEQ(
        re__ast_root,
        reg.data->ast_root,
        "(ast "
        "  (charclass "
        "    ("
        "      (rune_range 0 47)"
        "      (rune_range 58 64)"
        "      (rune_range 91 96)"
        "      (rune_range 123 0x10FFFF))"
    );
    re_destroy(&reg);
    PASS();
}

TEST(t_parse_opt_fuse_rune_rune) {
    re reg;
    re_rune first = re_rune_rand();
    re_rune second = re_rune_rand();
    re__str in_str;
    re_uint8 utf8_bytes[32];
    int utf8_bytes_ptr = 0;
    utf8_bytes_ptr += re__compile_gen_utf8(first, utf8_bytes + utf8_bytes_ptr);
    utf8_bytes_ptr += re__compile_gen_utf8(second, utf8_bytes + utf8_bytes_ptr);
    utf8_bytes[utf8_bytes_ptr] = '\0';
    re__str_init_n(&in_str, (re_char*)utf8_bytes, (re_size)utf8_bytes_ptr);
    ASSERT(!re_init(&reg, re__str_get_data(&in_str)));
    {
        re__ast* ast = re__ast_root_get(
            &reg.data->ast_root,
            reg.data->ast_root.root_ref
        );
        re__str_view a, b;
        ASSERT(ast->type == RE__AST_TYPE_STR);
        a = re__ast_root_get_str_view(&reg.data->ast_root, ast->_data.str_ref);
        re__str_view_init(&b, &in_str);
        ASSERT(re__str_view_cmp(&a, &b) == 0);
    }
    re_destroy(&reg);
    re__str_destroy(&in_str);
    PASS();
}

TEST(t_parse_opt_fuse_str_rune) {
    re reg;
    int n = (int)RAND_PARAM(25) + 2;
    int i;
    re__str in_str;
    re__str_init(&in_str);
    for (i = 0; i < n; i++) {
        re_uint8 utf8_bytes[32];
        int utf8_bytes_ptr = 0;
        re_rune r = re_rune_rand();
        utf8_bytes_ptr += re__compile_gen_utf8(r, utf8_bytes + utf8_bytes_ptr);
        re__str_cat_n(&in_str, (re_char*)utf8_bytes, (re_size)utf8_bytes_ptr);
    }
    ASSERT(!re_init(&reg, re__str_get_data(&in_str)));
    {
        re__ast* ast = re__ast_root_get(
            &reg.data->ast_root,
            reg.data->ast_root.root_ref
        );
        re__str_view a, b;
        ASSERT(ast->type == RE__AST_TYPE_STR);
        a = re__ast_root_get_str_view(&reg.data->ast_root, ast->_data.str_ref);
        re__str_view_init(&b, &in_str);
        ASSERT(re__str_view_cmp(&a, &b) == 0);
    }
    re_destroy(&reg);
    re__str_destroy(&in_str);
    PASS();
}

SUITE(s_parse) {
    RUN_TEST(t_parse_empty);
    RUN_TEST(t_parse_text_end);
    RUN_TEST(t_parse_group);
    RUN_TEST(t_parse_group_unfinished);
    RUN_TEST(t_parse_group_unmatched);
    FUZZ_TEST(t_parse_group_balance);
    RUN_TEST(t_parse_group_named);
    RUN_TEST(t_parse_groups);
    RUN_TEST(t_parse_star);
    RUN_TEST(t_parse_question);
    RUN_TEST(t_parse_plus);
    RUN_TEST(t_parse_any_char);
    RUN_TEST(t_parse_charclass_one);
    RUN_TEST(t_parse_charclass_rbracket);
    RUN_TEST(t_parse_charclass_hyphen);
    RUN_TEST(t_parse_charclass_unfinished);
    RUN_TEST(t_parse_charclass_inverted);
    RUN_TEST(t_parse_charclass_named);
    RUN_TEST(t_parse_charclass_named_inverted);
    FUZZ_TEST(t_parse_opt_fuse_rune_rune);
    FUZZ_TEST(t_parse_opt_fuse_str_rune);
}

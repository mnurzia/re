#include "test_charclass.h"
#include "test_range.h"

int re__charclass_to_sym(sym_build* parent, const re__charclass charclass) {
    sym_build build;
    SYM_PUT_EXPR(parent, &build);
    SYM_PUT_TYPE(&build, "charclass");
    {
        sym_build range_list;
        mptest_size i;
        SYM_PUT_EXPR(&build, &range_list);
        for (i = 0; i < re__charclass_get_num_ranges(&charclass); i++) {
            re__rune_range rr = re__charclass_get_ranges(&charclass)[i];
            SYM_PUT_SUB(&range_list, re__rune_range, rr);
        }
    }
    return SYM_OK;
}

int re__charclass_from_sym(sym_walk* parent, re__charclass* charclass) {
    sym_walk walk;
    SYM_GET_EXPR(parent, &walk);
    SYM_CHECK_TYPE(&walk, "charclass");
    {
        sym_walk range_list;
        SYM_GET_EXPR(&walk, &range_list);
        while (SYM_MORE(&range_list)) {
            re__rune_range rr;
            SYM_GET_SUB(&range_list, re__rune_range, &rr);
            re__charclass_push(charclass, rr);
        }
    }
    return SYM_OK;
}

TEST(t_charclass_init) {
    re__charclass charclass;
    re__charclass_init(&charclass);
    ASSERT_SYMEQ(re__charclass, charclass, "(charclass ())");
    re__charclass_destroy(&charclass);
    PASS();
}

TEST(t_charclass_init_from_class_alnum) {
    re__charclass charclass;
    if (re__charclass_init_from_class(&charclass, RE__CHARCLASS_ASCII_TYPE_ALNUM, 0) == RE_ERROR_NOMEM) { PASS(); }
    ASSERT_SYMEQ(re__charclass, charclass,
        "(charclass "
        "   ((rune_range 48 57)"
        "    (rune_range 65 90)"
        "    (rune_range 97 122))");
    re__charclass_destroy(&charclass);
    if (re__charclass_init_from_class(&charclass, RE__CHARCLASS_ASCII_TYPE_ALNUM, 1) == RE_ERROR_NOMEM) { PASS(); }
    ASSERT_SYMEQ(re__charclass, charclass,
        "(charclass "
        "   ((rune_range 0 47)"
        "    (rune_range 58 64)"
        "    (rune_range 91 96)"
        "    (rune_range 123 0x10FFFF))");
    re__charclass_destroy(&charclass);
    PASS();
}

TEST(t_charclass_init_from_class_alpha) {
    re__charclass charclass;
    if (re__charclass_init_from_class(&charclass, RE__CHARCLASS_ASCII_TYPE_ALPHA, 0) == RE_ERROR_NOMEM) { PASS(); }
    ASSERT_SYMEQ(re__charclass, charclass,
        "(charclass "
        "   ((rune_range 65 90)"
        "    (rune_range 97 122))");
    re__charclass_destroy(&charclass);
    if (re__charclass_init_from_class(&charclass, RE__CHARCLASS_ASCII_TYPE_ALPHA, 1) == RE_ERROR_NOMEM) { PASS(); }
    ASSERT_SYMEQ(re__charclass, charclass,
        "(charclass "
        "   ((rune_range 0 64)"
        "    (rune_range 91 96)"
        "    (rune_range 123 0x10FFFF))");
    re__charclass_destroy(&charclass);
    PASS();
}

TEST(t_charclass_init_from_class_ascii) {
    re__charclass charclass;
    if (re__charclass_init_from_class(&charclass, RE__CHARCLASS_ASCII_TYPE_ASCII, 0) == RE_ERROR_NOMEM) { PASS(); }
    ASSERT_SYMEQ(re__charclass, charclass,
        "(charclass "
        "   ((rune_range 0 127))");
    re__charclass_destroy(&charclass);
    if (re__charclass_init_from_class(&charclass, RE__CHARCLASS_ASCII_TYPE_ASCII, 1) == RE_ERROR_NOMEM) { PASS(); }
    ASSERT_SYMEQ(re__charclass, charclass,
        "(charclass "
        "   ((rune_range 128 0x10FFFF))");
    re__charclass_destroy(&charclass);
    PASS();
}

TEST(t_charclass_init_from_class_blank) {
    re__charclass charclass;
    if (re__charclass_init_from_class(&charclass, RE__CHARCLASS_ASCII_TYPE_BLANK, 0) == RE_ERROR_NOMEM) { PASS(); }
    ASSERT_SYMEQ(re__charclass, charclass,
        "(charclass "
        "   ((rune_range '\t' '\t')"
        "    (rune_range ' ' ' '))");
    re__charclass_destroy(&charclass);
    if (re__charclass_init_from_class(&charclass, RE__CHARCLASS_ASCII_TYPE_BLANK, 1) == RE_ERROR_NOMEM) { PASS(); }
    ASSERT_SYMEQ(re__charclass, charclass,
        "(charclass "
        "   ((rune_range 0 8)"
        "    (rune_range 10 31)"
        "    (rune_range 33 0x10FFFF))");
    re__charclass_destroy(&charclass);
    PASS();
}

TEST(t_charclass_init_from_class_cntrl) {
    re__charclass charclass;
    if (re__charclass_init_from_class(&charclass, RE__CHARCLASS_ASCII_TYPE_CNTRL, 0) == RE_ERROR_NOMEM) { PASS(); }
    ASSERT_SYMEQ(re__charclass, charclass,
        "(charclass "
        "   ((rune_range 0 31)"
        "    (rune_range 127 127))");
    re__charclass_destroy(&charclass);
    if (re__charclass_init_from_class(&charclass, RE__CHARCLASS_ASCII_TYPE_CNTRL, 1) == RE_ERROR_NOMEM) { PASS(); }
    ASSERT_SYMEQ(re__charclass, charclass,
        "(charclass "
        "   ((rune_range 32 126)"
        "    (rune_range 128 0x10FFFF))");
    re__charclass_destroy(&charclass);
    PASS();
}

TEST(t_charclass_init_from_class_digit) {
    re__charclass charclass;
    if (re__charclass_init_from_class(&charclass, RE__CHARCLASS_ASCII_TYPE_DIGIT, 0) == RE_ERROR_NOMEM) { PASS(); }
    ASSERT_SYMEQ(re__charclass, charclass,
        "(charclass "
        "   ((rune_range '0' '9'))");
    re__charclass_destroy(&charclass);
    if (re__charclass_init_from_class(&charclass, RE__CHARCLASS_ASCII_TYPE_DIGIT, 1) == RE_ERROR_NOMEM) { PASS(); }
    ASSERT_SYMEQ(re__charclass, charclass,
        "(charclass "
        "   ((rune_range 0 47)"
        "    (rune_range 58 0x10FFFF))");
    re__charclass_destroy(&charclass);
    PASS();
}

TEST(t_charclass_init_from_class_graph) {
    re__charclass charclass;
    if (re__charclass_init_from_class(&charclass, RE__CHARCLASS_ASCII_TYPE_GRAPH, 0) == RE_ERROR_NOMEM) { PASS(); }
    ASSERT_SYMEQ(re__charclass, charclass,
        "(charclass "
        "   ((rune_range 33 126))");
    re__charclass_destroy(&charclass);
    if (re__charclass_init_from_class(&charclass, RE__CHARCLASS_ASCII_TYPE_GRAPH, 1) == RE_ERROR_NOMEM) { PASS(); }
    ASSERT_SYMEQ(re__charclass, charclass,
        "(charclass "
        "   ((rune_range 0 32)"
        "    (rune_range 127 0x10FFFF))");
    re__charclass_destroy(&charclass);
    PASS();
}

TEST(t_charclass_init_from_class_lower) {
    re__charclass charclass;
    if (re__charclass_init_from_class(&charclass, RE__CHARCLASS_ASCII_TYPE_LOWER, 0) == RE_ERROR_NOMEM) { PASS(); }
    ASSERT_SYMEQ(re__charclass, charclass,
        "(charclass "
        "   ((rune_range 'a' 'z'))");
    re__charclass_destroy(&charclass);
    if (re__charclass_init_from_class(&charclass, RE__CHARCLASS_ASCII_TYPE_LOWER, 1) == RE_ERROR_NOMEM) { PASS(); }
    ASSERT_SYMEQ(re__charclass, charclass,
        "(charclass "
        "   ((rune_range 0 96)"
        "    (rune_range 123 0x10FFFF))");
    re__charclass_destroy(&charclass);
    PASS();
}

TEST(t_charclass_init_from_class_perl_space) {
    re__charclass charclass;
    if (re__charclass_init_from_class(&charclass, RE__CHARCLASS_ASCII_TYPE_PERL_SPACE, 0) == RE_ERROR_NOMEM) { PASS(); }
    ASSERT_SYMEQ(re__charclass, charclass,
        "(charclass "
        "   ((rune_range 9 10)"
        "    (rune_range 12 13)"
        "    (rune_range 32 32))");
    re__charclass_destroy(&charclass);
    if (re__charclass_init_from_class(&charclass, RE__CHARCLASS_ASCII_TYPE_PERL_SPACE, 1) == RE_ERROR_NOMEM) { PASS(); }
    ASSERT_SYMEQ(re__charclass, charclass,
        "(charclass "
        "   ((rune_range 0 8)"
        "    (rune_range 11 11)"
        "    (rune_range 14 31)"
        "    (rune_range 33 0x10FFFF))");
    re__charclass_destroy(&charclass);
    PASS();
}

TEST(t_charclass_init_from_class_print) {
    re__charclass charclass;
    if (re__charclass_init_from_class(&charclass, RE__CHARCLASS_ASCII_TYPE_PRINT, 0) == RE_ERROR_NOMEM) { PASS(); }
    ASSERT_SYMEQ(re__charclass, charclass,
        "(charclass "
        "   ((rune_range ' ' 126))");
    re__charclass_destroy(&charclass);
    if (re__charclass_init_from_class(&charclass, RE__CHARCLASS_ASCII_TYPE_PRINT, 1) == RE_ERROR_NOMEM) { PASS(); }
    ASSERT_SYMEQ(re__charclass, charclass,
        "(charclass "
        "   ((rune_range 0 31)"
        "    (rune_range 127 0x10FFFF))");
    re__charclass_destroy(&charclass);
    PASS();
}

TEST(t_charclass_init_from_class_punct) {
    re__charclass charclass;
    if (re__charclass_init_from_class(&charclass, RE__CHARCLASS_ASCII_TYPE_PUNCT, 0) == RE_ERROR_NOMEM) { PASS(); }
    ASSERT_SYMEQ(re__charclass, charclass,
        "(charclass "
        "   ((rune_range 0x21 0x2F)"
        "    (rune_range 0x3A 0x40)"
        "    (rune_range 0x5B 0x60)"
        "    (rune_range 0x7B 0x7E))");
    re__charclass_destroy(&charclass);
    if (re__charclass_init_from_class(&charclass, RE__CHARCLASS_ASCII_TYPE_PUNCT, 1) == RE_ERROR_NOMEM) { PASS(); }
    ASSERT_SYMEQ(re__charclass, charclass,
        "(charclass "
        "   ((rune_range 0 0x20)"
        "    (rune_range 0x30 0x39)"
        "    (rune_range 0x41 0x5A)"
        "    (rune_range 0x61 0x7A)"
        "    (rune_range 0x7F 0x10FFFF))");
    re__charclass_destroy(&charclass);
    PASS();
}

TEST(t_charclass_init_from_class_space) {
    re__charclass charclass;
    if (re__charclass_init_from_class(&charclass, RE__CHARCLASS_ASCII_TYPE_SPACE, 0) == RE_ERROR_NOMEM) { PASS(); }
    ASSERT_SYMEQ(re__charclass, charclass,
        "(charclass "
        "   ((rune_range 0x09 0x0D)"
        "    (rune_range ' ' ' '))");
    re__charclass_destroy(&charclass);
    if (re__charclass_init_from_class(&charclass, RE__CHARCLASS_ASCII_TYPE_SPACE, 1) == RE_ERROR_NOMEM) { PASS(); }
    ASSERT_SYMEQ(re__charclass, charclass,
        "(charclass "
        "   ((rune_range 0 8)"
        "    (rune_range 0xE 31)"
        "    (rune_range 33 0x10FFFF))");
    re__charclass_destroy(&charclass);
    PASS();
}

TEST(t_charclass_init_from_class_upper) {
    re__charclass charclass;
    if (re__charclass_init_from_class(&charclass, RE__CHARCLASS_ASCII_TYPE_UPPER, 0) == RE_ERROR_NOMEM) { PASS(); }
    ASSERT_SYMEQ(re__charclass, charclass,
        "(charclass "
        "   ((rune_range 'A' 'Z'))");
    re__charclass_destroy(&charclass);
    if (re__charclass_init_from_class(&charclass, RE__CHARCLASS_ASCII_TYPE_UPPER, 1) == RE_ERROR_NOMEM) { PASS(); }
    ASSERT_SYMEQ(re__charclass, charclass,
        "(charclass "
        "   ((rune_range 0 64)"
        "    (rune_range 91 0x10FFFF))");
    re__charclass_destroy(&charclass);
    PASS();
}

TEST(t_charclass_init_from_class_word) {
    re__charclass charclass;
    if (re__charclass_init_from_class(&charclass, RE__CHARCLASS_ASCII_TYPE_WORD, 0) == RE_ERROR_NOMEM) { PASS(); }
    ASSERT_SYMEQ(re__charclass, charclass,
        "(charclass "
        "   ((rune_range '0' '9')"
        "    (rune_range 'A' 'Z')"
        "    (rune_range '_' '_')"
        "    (rune_range 'a' 'z'))");
    re__charclass_destroy(&charclass);
    if (re__charclass_init_from_class(&charclass, RE__CHARCLASS_ASCII_TYPE_WORD, 1) == RE_ERROR_NOMEM) { PASS(); }
    ASSERT_SYMEQ(re__charclass, charclass,
        "(charclass "
        "   ((rune_range 0 47)"
        "    (rune_range 58 64)"
        "    (rune_range 91 94)"
        "    (rune_range 96 96)"
        "    (rune_range 123 0x10FFFF))");
    re__charclass_destroy(&charclass);
    PASS();
}

TEST(t_charclass_init_from_class_xdigit) {
    re__charclass charclass;
    if (re__charclass_init_from_class(&charclass, RE__CHARCLASS_ASCII_TYPE_XDIGIT, 0) == RE_ERROR_NOMEM) { PASS(); }
    ASSERT_SYMEQ(re__charclass, charclass,
        "(charclass "
        "   ((rune_range '0' '9')"
        "    (rune_range 'A' 'F')"
        "    (rune_range 'a' 'f'))");
    re__charclass_destroy(&charclass);
    if (re__charclass_init_from_class(&charclass, RE__CHARCLASS_ASCII_TYPE_XDIGIT, 1) == RE_ERROR_NOMEM) { PASS(); }
    ASSERT_SYMEQ(re__charclass, charclass,
        "(charclass "
        "   ((rune_range 0 47)"
        "    (rune_range 58 64)"
        "    (rune_range 71 96)"
        "    (rune_range 103 0x10FFFF)");
    re__charclass_destroy(&charclass);
    PASS();
}

SUITE(s_charclass_init_from_class) {
    RUN_TEST(t_charclass_init_from_class_alnum);
    RUN_TEST(t_charclass_init_from_class_alpha);
    RUN_TEST(t_charclass_init_from_class_ascii);
    RUN_TEST(t_charclass_init_from_class_blank);
    RUN_TEST(t_charclass_init_from_class_cntrl);
    RUN_TEST(t_charclass_init_from_class_digit);
    RUN_TEST(t_charclass_init_from_class_graph);
    RUN_TEST(t_charclass_init_from_class_lower);
    RUN_TEST(t_charclass_init_from_class_perl_space);
    RUN_TEST(t_charclass_init_from_class_print);
    RUN_TEST(t_charclass_init_from_class_punct);
    RUN_TEST(t_charclass_init_from_class_space);
    RUN_TEST(t_charclass_init_from_class_upper);
    RUN_TEST(t_charclass_init_from_class_word);
    RUN_TEST(t_charclass_init_from_class_xdigit);
}

TEST(t_charclass_init_from_str_alnum) {
    re__charclass out, test;
    mn__str_view name;
    re_error err = RE_ERROR_NONE;
    mn__str_view_init_s(&name, (const mn_char*)"alnum");
    err = re__charclass_init_from_str(&out, name, 0);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    err = re__charclass_init_from_class(&test, RE__CHARCLASS_ASCII_TYPE_ALNUM, 0);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    ASSERT(re__charclass_equals(&out, &test));
    re__charclass_destroy(&out);
    re__charclass_destroy(&test);
    err = re__charclass_init_from_str(&out, name, 1);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    err = re__charclass_init_from_class(&test, RE__CHARCLASS_ASCII_TYPE_ALNUM, 1);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    ASSERT(re__charclass_equals(&out, &test));
    re__charclass_destroy(&out);
    re__charclass_destroy(&test);

    PASS();
}

TEST(t_charclass_init_from_str_alpha) {
    re__charclass out, test;
    mn__str_view name;
    re_error err = RE_ERROR_NONE;
    mn__str_view_init_s(&name, (const mn_char*)"alpha");
    err = re__charclass_init_from_str(&out, name, 0);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    err = re__charclass_init_from_class(&test, RE__CHARCLASS_ASCII_TYPE_ALPHA, 0);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    ASSERT(re__charclass_equals(&out, &test));
    re__charclass_destroy(&out);
    re__charclass_destroy(&test);
    err = re__charclass_init_from_str(&out, name, 1);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    err = re__charclass_init_from_class(&test, RE__CHARCLASS_ASCII_TYPE_ALPHA, 1);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    ASSERT(re__charclass_equals(&out, &test));
    re__charclass_destroy(&out);
    re__charclass_destroy(&test);
    PASS();
}

TEST(t_charclass_init_from_str_ascii) {
    re__charclass out, test;
    mn__str_view name;
    re_error err = RE_ERROR_NONE;
    mn__str_view_init_s(&name, (const mn_char*)"ascii");
    err = re__charclass_init_from_str(&out, name, 0);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    err = re__charclass_init_from_class(&test, RE__CHARCLASS_ASCII_TYPE_ASCII, 0);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    ASSERT(re__charclass_equals(&out, &test));
    re__charclass_destroy(&out);
    re__charclass_destroy(&test);
    err = re__charclass_init_from_str(&out, name, 1);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    err = re__charclass_init_from_class(&test, RE__CHARCLASS_ASCII_TYPE_ASCII, 1);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    ASSERT(re__charclass_equals(&out, &test));
    re__charclass_destroy(&out);
    re__charclass_destroy(&test);
    PASS();
}

TEST(t_charclass_init_from_str_blank) {
    re__charclass out, test;
    mn__str_view name;
    re_error err = RE_ERROR_NONE;
    mn__str_view_init_s(&name, (const mn_char*)"blank");
    err = re__charclass_init_from_str(&out, name, 0);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    err = re__charclass_init_from_class(&test, RE__CHARCLASS_ASCII_TYPE_BLANK, 0);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    ASSERT(re__charclass_equals(&out, &test));
    re__charclass_destroy(&out);
    re__charclass_destroy(&test);
    err = re__charclass_init_from_str(&out, name, 1);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    err = re__charclass_init_from_class(&test, RE__CHARCLASS_ASCII_TYPE_BLANK, 1);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    ASSERT(re__charclass_equals(&out, &test));
    re__charclass_destroy(&out);
    re__charclass_destroy(&test);
    PASS();
}

TEST(t_charclass_init_from_str_cntrl) {
    re__charclass out, test;
    mn__str_view name;
    re_error err = RE_ERROR_NONE;
    mn__str_view_init_s(&name, (const mn_char*)"cntrl");
    err = re__charclass_init_from_str(&out, name, 0);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    err = re__charclass_init_from_class(&test, RE__CHARCLASS_ASCII_TYPE_CNTRL, 0);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    ASSERT(re__charclass_equals(&out, &test));
    re__charclass_destroy(&out);
    re__charclass_destroy(&test);
    err = re__charclass_init_from_str(&out, name, 1);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    err = re__charclass_init_from_class(&test, RE__CHARCLASS_ASCII_TYPE_CNTRL, 1);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    ASSERT(re__charclass_equals(&out, &test));
    re__charclass_destroy(&out);
    re__charclass_destroy(&test);
    PASS();
}

TEST(t_charclass_init_from_str_digit) {
    re__charclass out, test;
    mn__str_view name;
    re_error err = RE_ERROR_NONE;
    mn__str_view_init_s(&name, (const mn_char*)"digit");
    err = re__charclass_init_from_str(&out, name, 0);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    err = re__charclass_init_from_class(&test, RE__CHARCLASS_ASCII_TYPE_DIGIT, 0);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    ASSERT(re__charclass_equals(&out, &test));
    re__charclass_destroy(&out);
    re__charclass_destroy(&test);
    err = re__charclass_init_from_str(&out, name, 1);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    err = re__charclass_init_from_class(&test, RE__CHARCLASS_ASCII_TYPE_DIGIT, 1);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    ASSERT(re__charclass_equals(&out, &test));
    re__charclass_destroy(&out);
    re__charclass_destroy(&test);
    PASS();
}

TEST(t_charclass_init_from_str_graph) {
    re__charclass out, test;
    mn__str_view name;
    re_error err = RE_ERROR_NONE;
    mn__str_view_init_s(&name, (const mn_char*)"graph");
    err = re__charclass_init_from_str(&out, name, 0);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    err = re__charclass_init_from_class(&test, RE__CHARCLASS_ASCII_TYPE_GRAPH, 0);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    ASSERT(re__charclass_equals(&out, &test));
    re__charclass_destroy(&out);
    re__charclass_destroy(&test);
    err = re__charclass_init_from_str(&out, name, 1);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    err = re__charclass_init_from_class(&test, RE__CHARCLASS_ASCII_TYPE_GRAPH, 1);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    ASSERT(re__charclass_equals(&out, &test));
    re__charclass_destroy(&out);
    re__charclass_destroy(&test);
    PASS();
}

TEST(t_charclass_init_from_str_lower) {
    re__charclass out, test;
    mn__str_view name;
    re_error err = RE_ERROR_NONE;
    mn__str_view_init_s(&name, (const mn_char*)"lower");
    err = re__charclass_init_from_str(&out, name, 0);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    err = re__charclass_init_from_class(&test, RE__CHARCLASS_ASCII_TYPE_LOWER, 0);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    ASSERT(re__charclass_equals(&out, &test));
    re__charclass_destroy(&out);
    re__charclass_destroy(&test);
    err = re__charclass_init_from_str(&out, name, 1);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    err = re__charclass_init_from_class(&test, RE__CHARCLASS_ASCII_TYPE_LOWER, 1);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    ASSERT(re__charclass_equals(&out, &test));
    re__charclass_destroy(&out);
    re__charclass_destroy(&test);
    PASS();
}

TEST(t_charclass_init_from_str_perl_space) {
    re__charclass out, test;
    mn__str_view name;
    re_error err = RE_ERROR_NONE;
    mn__str_view_init_s(&name, (const mn_char*)"perl_space");
    err = re__charclass_init_from_str(&out, name, 0);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    err = re__charclass_init_from_class(&test, RE__CHARCLASS_ASCII_TYPE_PERL_SPACE, 0);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    ASSERT(re__charclass_equals(&out, &test));
    re__charclass_destroy(&out);
    re__charclass_destroy(&test);
    err = re__charclass_init_from_str(&out, name, 1);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    err = re__charclass_init_from_class(&test, RE__CHARCLASS_ASCII_TYPE_PERL_SPACE, 1);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    ASSERT(re__charclass_equals(&out, &test));
    re__charclass_destroy(&out);
    re__charclass_destroy(&test);
    PASS();
}

TEST(t_charclass_init_from_str_print) {
    re__charclass out, test;
    mn__str_view name;
    re_error err = RE_ERROR_NONE;
    mn__str_view_init_s(&name, (const mn_char*)"print");
    err = re__charclass_init_from_str(&out, name, 0);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    err = re__charclass_init_from_class(&test, RE__CHARCLASS_ASCII_TYPE_PRINT, 0);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    ASSERT(re__charclass_equals(&out, &test));
    re__charclass_destroy(&out);
    re__charclass_destroy(&test);
    err = re__charclass_init_from_str(&out, name, 1);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    err = re__charclass_init_from_class(&test, RE__CHARCLASS_ASCII_TYPE_PRINT, 1);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    ASSERT(re__charclass_equals(&out, &test));
    re__charclass_destroy(&out);
    re__charclass_destroy(&test);
    PASS();
}

TEST(t_charclass_init_from_str_punct) {
    re__charclass out, test;
    mn__str_view name;
    re_error err = RE_ERROR_NONE;
    mn__str_view_init_s(&name, (const mn_char*)"punct");
    err = re__charclass_init_from_str(&out, name, 0);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    err = re__charclass_init_from_class(&test, RE__CHARCLASS_ASCII_TYPE_PUNCT, 0);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    ASSERT(re__charclass_equals(&out, &test));
    re__charclass_destroy(&out);
    re__charclass_destroy(&test);
    err = re__charclass_init_from_str(&out, name, 1);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    err = re__charclass_init_from_class(&test, RE__CHARCLASS_ASCII_TYPE_PUNCT, 1);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    ASSERT(re__charclass_equals(&out, &test));
    re__charclass_destroy(&out);
    re__charclass_destroy(&test);
    PASS();
}

TEST(t_charclass_init_from_str_space) {
    re__charclass out, test;
    mn__str_view name;
    re_error err = RE_ERROR_NONE;
    mn__str_view_init_s(&name, (const mn_char*)"space");
    err = re__charclass_init_from_str(&out, name, 0);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    err = re__charclass_init_from_class(&test, RE__CHARCLASS_ASCII_TYPE_SPACE, 0);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    ASSERT(re__charclass_equals(&out, &test));
    re__charclass_destroy(&out);
    re__charclass_destroy(&test);
    err = re__charclass_init_from_str(&out, name, 1);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    err = re__charclass_init_from_class(&test, RE__CHARCLASS_ASCII_TYPE_SPACE, 1);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    ASSERT(re__charclass_equals(&out, &test));
    re__charclass_destroy(&out);
    re__charclass_destroy(&test);
    PASS();
}

TEST(t_charclass_init_from_str_upper) {
    re__charclass out, test;
    mn__str_view name;
    re_error err = RE_ERROR_NONE;
    mn__str_view_init_s(&name, (const mn_char*)"upper");
    err = re__charclass_init_from_str(&out, name, 0);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    err = re__charclass_init_from_class(&test, RE__CHARCLASS_ASCII_TYPE_UPPER, 0);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    ASSERT(re__charclass_equals(&out, &test));
    re__charclass_destroy(&out);
    re__charclass_destroy(&test);
    err = re__charclass_init_from_str(&out, name, 1);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    err = re__charclass_init_from_class(&test, RE__CHARCLASS_ASCII_TYPE_UPPER, 1);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    ASSERT(re__charclass_equals(&out, &test));
    re__charclass_destroy(&out);
    re__charclass_destroy(&test);
    PASS();
}

TEST(t_charclass_init_from_str_word) {
    re__charclass out, test;
    mn__str_view name;
    re_error err = RE_ERROR_NONE;
    mn__str_view_init_s(&name, (const mn_char*)"word");
    err = re__charclass_init_from_str(&out, name, 0);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    err = re__charclass_init_from_class(&test, RE__CHARCLASS_ASCII_TYPE_WORD, 0);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    ASSERT(re__charclass_equals(&out, &test));
    re__charclass_destroy(&out);
    re__charclass_destroy(&test);
    err = re__charclass_init_from_str(&out, name, 1);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    err = re__charclass_init_from_class(&test, RE__CHARCLASS_ASCII_TYPE_WORD, 1);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    ASSERT(re__charclass_equals(&out, &test));
    re__charclass_destroy(&out);
    re__charclass_destroy(&test);
    PASS();
}

TEST(t_charclass_init_from_str_xdigit) {
    re__charclass out, test;
    mn__str_view name;
    re_error err = RE_ERROR_NONE;
    mn__str_view_init_s(&name, (const mn_char*)"xdigit");
    err = re__charclass_init_from_str(&out, name, 0);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    err = re__charclass_init_from_class(&test, RE__CHARCLASS_ASCII_TYPE_XDIGIT, 0);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    ASSERT(re__charclass_equals(&out, &test));
    re__charclass_destroy(&out);
    re__charclass_destroy(&test);
    err = re__charclass_init_from_str(&out, name, 1);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    err = re__charclass_init_from_class(&test, RE__CHARCLASS_ASCII_TYPE_XDIGIT, 1);
    if (err == RE_ERROR_NOMEM) { PASS(); }
    ASSERT(err == RE_ERROR_NONE);
    ASSERT(re__charclass_equals(&out, &test));
    re__charclass_destroy(&out);
    re__charclass_destroy(&test);
    PASS();
}

SUITE(s_charclass_init_from_str) {
    RUN_TEST(t_charclass_init_from_str_alnum);
    RUN_TEST(t_charclass_init_from_str_alpha);
    RUN_TEST(t_charclass_init_from_str_ascii);
    RUN_TEST(t_charclass_init_from_str_blank);
    RUN_TEST(t_charclass_init_from_str_cntrl);
    RUN_TEST(t_charclass_init_from_str_digit);
    RUN_TEST(t_charclass_init_from_str_graph);
    RUN_TEST(t_charclass_init_from_str_lower);
    RUN_TEST(t_charclass_init_from_str_perl_space);
    RUN_TEST(t_charclass_init_from_str_print);
    RUN_TEST(t_charclass_init_from_str_punct);
    RUN_TEST(t_charclass_init_from_str_space);
    RUN_TEST(t_charclass_init_from_str_upper);
    RUN_TEST(t_charclass_init_from_str_word);
    RUN_TEST(t_charclass_init_from_str_xdigit);
}

TEST(t_charclass_destroy) {
    mn_size l = RAND_PARAM(600);
    mn_size i;
    re__charclass cc;
    re_error err = RE_ERROR_NONE;
    re__charclass_init(&cc);
    for (i = 0; i < l; i++) {
        re__rune_range rr;
        rr.min = (re_rune)RAND_PARAM(0x10FFFF);
        rr.max = ((mn_int32)RAND_PARAM((mptest_rand)(0x10FFFF - rr.min))) + rr.min;
        if ((err = re__charclass_push(&cc, rr)) == RE_ERROR_NOMEM) {
            goto error;
        }
    }
error:
    re__charclass_destroy(&cc);
    PASS();
}

TEST(t_charclass_push) {
    mn_size l = RAND_PARAM(600);
    mn_size i;
    re__charclass cc;
    re_error err = RE_ERROR_NONE;
    re__charclass_init(&cc);
    for (i = 0; i < l; i++) {
        re__rune_range rr;
        rr.min = (re_rune)RAND_PARAM(0x10FFFF);
        rr.max = ((mn_int32)RAND_PARAM((mptest_rand)(0x10FFFF - rr.min))) + rr.min;
        if ((err = re__charclass_push(&cc, rr)) == RE_ERROR_NOMEM) {
            goto error;
        }
        ASSERT(re__rune_range_equals(re__charclass_get_ranges(&cc)[i], rr));
    }
error:
    re__charclass_destroy(&cc);
    PASS();
}

TEST(t_charclass_get_num_ranges) {
    mn_size l = RAND_PARAM(600);
    mn_size i;
    re__charclass cc;
    re_error err = RE_ERROR_NONE;
    re__charclass_init(&cc);
    ASSERT_EQ(re__charclass_get_num_ranges(&cc), 0);
    for (i = 0; i < l; i++) {
        re__rune_range rr;
        rr.min = (re_rune)RAND_PARAM(0x10FFFF);
        rr.max = ((mn_int32)RAND_PARAM((mptest_rand)(0x10FFFF - rr.min))) + rr.min;
        if ((err = re__charclass_push(&cc, rr)) == RE_ERROR_NOMEM) {
            goto error;
        }
    }
    ASSERT_EQ(re__charclass_get_num_ranges(&cc), l);
error:
    re__charclass_destroy(&cc);
    PASS();
}

TEST(t_charclass_equals) {
    mn_size l = RAND_PARAM(600) + 1;
    mn_size i;
    re__charclass cc, ot, mt;
    re_error err = RE_ERROR_NONE;
    re__charclass_init(&cc);
    re__charclass_init(&ot);
    re__charclass_init(&mt);
    for (i = 0; i < l; i++) {
        re__rune_range rr;
        rr.min = (re_rune)RAND_PARAM(0x10FFFF);
        rr.max = ((mn_int32)RAND_PARAM((mptest_rand)(0x10FFFF - rr.min))) + rr.min;
        if ((err = re__charclass_push(&cc, rr)) == RE_ERROR_NOMEM) {
            goto error;
        }
        if ((err = re__charclass_push(&ot, rr)) == RE_ERROR_NOMEM) {
            goto error;
        }
    }
    ASSERT(re__charclass_equals(&cc, &ot));
    ASSERT(!re__charclass_equals(&cc, &mt));
error:
    re__charclass_destroy(&cc);
    re__charclass_destroy(&ot);
    re__charclass_destroy(&mt);
    PASS();
}

TEST(t_charclass_builder_init) {
    re__charclass_builder builder;
    re__charclass_builder_init(&builder);
    ASSERT(builder.should_invert == 0);
    ASSERT(builder.highest == -1);
    re__charclass_builder_destroy(&builder);
    PASS();
}

TEST(t_charclass_builder_begin) {
    re__charclass_builder builder;
    mn_size l = RAND_PARAM(600);
    mn_size i;
    re__charclass_builder_init(&builder);
    re__charclass_builder_begin(&builder);
    ASSERT(builder.should_invert == 0);
    ASSERT(builder.highest == -1);
    for (i = 0; i < l; i++) {
        re__rune_range rr;
        rr.min = (re_rune)RAND_PARAM(0x10FFFF);
        rr.max = ((mn_int32)RAND_PARAM((mptest_rand)(0x10FFFF - rr.min))) + rr.min;
        re__charclass_builder_insert_range(&builder, rr);
    }
    re__charclass_builder_begin(&builder);
    ASSERT(builder.should_invert == 0);
    ASSERT(builder.highest == -1);
    re__charclass_builder_destroy(&builder);
    PASS();
}

TEST(t_charclass_builder_invert) {
    re__charclass_builder builder;
    re__charclass_builder_init(&builder);
    ASSERT(builder.should_invert == 0);
    re__charclass_builder_invert(&builder);
    ASSERT(builder.should_invert == 1);
    re__charclass_builder_destroy(&builder);
    PASS();
}

TEST(t_charclass_builder_insert_range) {
    mn_size l = RAND_PARAM(60);
    mn_size i;
    re__charclass_builder builder;
    re__charclass_builder_init(&builder);
    for (i = 0; i < l; i++) {
        re__rune_range rr;
        rr.min = (re_rune)RAND_PARAM(0x10FFFF);
        rr.max = ((mn_int32)RAND_PARAM((mptest_rand)(0x10FFFF - rr.min))) + rr.min;
        re__charclass_builder_insert_range(&builder, rr);
    }
    re__charclass_builder_destroy(&builder);
    PASS();
}

SUITE(s_charclass_builder) {
    RUN_TEST(t_charclass_builder_init);
    FUZZ_TEST(t_charclass_builder_begin);
    RUN_TEST(t_charclass_builder_invert);
    FUZZ_TEST(t_charclass_builder_insert_range);
}

SUITE(s_charclass) {
    RUN_TEST(t_charclass_init);
    RUN_SUITE(s_charclass_init_from_class);
    RUN_SUITE(s_charclass_init_from_str);
    FUZZ_TEST(t_charclass_destroy);
    FUZZ_TEST(t_charclass_push);
    FUZZ_TEST(t_charclass_get_num_ranges);
    FUZZ_TEST(t_charclass_equals);
}

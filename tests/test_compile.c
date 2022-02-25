#include "test_compile.h"

#include "test_ast.h"
#include "test_helpers.h"
#include "test_prog.h"
#include "test_range.h"

TEST(t_compile_rune_ascii) {
    re__ast_root ast_root;
    re__prog prog;
    re__compile compile;
    SYM(
        re__ast_root,
        "(ast"
        "  (rune 'a'))",
        &ast_root
    );
    re__prog_init(&prog);
    re__compile_init(&compile);
    ASSERT(!re__compile_regex(&compile, &ast_root, &prog));
    ASSERT_SYMEQ(
        re__prog,
        prog,
        "(prog"
        "  (fail)"
        "  (byte 'a' 2)"
        "  (match 0))"
    );
    re__compile_destroy(&compile);
    re__prog_destroy(&prog);
    re__ast_root_destroy(&ast_root);
    PASS();
}

TEST(t_compile_rune_unicode) {
    re__ast_root ast_root;
    re__prog prog, prog_actual;
    re__compile compile;
    re__ast ast;
    re_int32 ast_ref;
    re_rune rune = re_rune_rand();
    re_uint8 chars[16];
    re_uint32 chars_len = (re_uint32)re__compile_gen_utf8(rune, chars);
    re__prog_loc i;
    re__prog_inst inst;
    re__ast_init_rune(&ast, rune);
    re__ast_root_init(&ast_root);
    re__ast_root_add_child(&ast_root, RE__AST_NONE, ast, &ast_ref);
    ast_root.depth_max = 1;
    re__prog_init(&prog);
    re__prog_init(&prog_actual);
    re__compile_init(&compile);
    re__prog_inst_init_fail(&inst);
    re__prog_add(&prog, inst);
    for (i = 0; i < chars_len; i++) {
        re__prog_inst_init_byte(&inst, chars[i]);
        re__prog_inst_set_primary(&inst, 2 + i);
        re__prog_add(&prog, inst);
    }
    re__prog_inst_init_match(&inst, 0);
    re__prog_add(&prog, inst);
    ASSERT(!re__compile_regex(&compile, &ast_root, &prog_actual));
    ASSERT(re__prog_equals(&prog, &prog_actual));
    re__prog_destroy(&prog);
    re__prog_destroy(&prog_actual);
    re__compile_destroy(&compile);
    re__ast_root_destroy(&ast_root);
    PASS();
}

TEST(t_compile_str) {
    re__ast_root ast_root;
    re__prog prog;
    re__compile compile;
    SYM(
        re__ast_root,
        "(ast"
        "  (str test))",
        &ast_root
    );
    re__prog_init(&prog);
    re__compile_init(&compile);
    ASSERT(!re__compile_regex(&compile, &ast_root, &prog));
    ASSERT_SYMEQ(
        re__prog,
        prog,
        "(prog"
        "  (fail)"
        "  (byte 't' 2)"
        "  (byte 'e' 3)"
        "  (byte 's' 4)"
        "  (byte 't' 5)"
        "  (match 0))"
    );
    re__compile_destroy(&compile);
    re__prog_destroy(&prog);
    re__ast_root_destroy(&ast_root);
    PASS();
}

TEST(t_compile_str_thrash) {
    re__ast_root ast_root;
    re__prog prog, prog_actual;
    re__compile compile;
    re__ast ast;
    re_int32 ast_ref;
    re_int32 str_ref;
    re__str in_str, copy_str;
    re__prog_loc i;
    re__prog_inst inst;
    re_size j, num_runes = RAND_PARAM(600);
    re__str_init(&in_str);
    for (j = 0; j < num_runes; j++) {
        re_uint8 chars[16];
        re_rune rune = re_rune_rand();
        re_uint32 chars_len = (re_uint32)re__compile_gen_utf8(rune, chars);
        re__str_cat_n(&in_str, (re_char*)chars, chars_len);
    }
    re__str_init_copy(&copy_str, &in_str);
    re__ast_root_init(&ast_root);
    re__ast_root_add_str(&ast_root, copy_str, &str_ref);
    re__ast_init_str(&ast, str_ref);
    re__ast_root_add_child(&ast_root, RE__AST_NONE, ast, &ast_ref);
    ast_root.depth_max = 1;
    re__prog_init(&prog);
    re__prog_init(&prog_actual);
    re__compile_init(&compile);
    re__prog_inst_init_fail(&inst);
    re__prog_add(&prog, inst);
    for (i = 0; i < (re__prog_loc)re__str_size(&in_str); i++) {
        re__prog_inst_init_byte(&inst, (re_uint8)re__str_get_data(&in_str)[i]);
        re__prog_inst_set_primary(&inst, 2 + i);
        re__prog_add(&prog, inst);
    }
    re__prog_inst_init_match(&inst, 0);
    re__prog_add(&prog, inst);
    ASSERT(!re__compile_regex(&compile, &ast_root, &prog_actual));
    ASSERT(re__prog_equals(&prog, &prog_actual));
    re__prog_destroy(&prog);
    re__prog_destroy(&prog_actual);
    re__compile_destroy(&compile);
    re__ast_root_destroy(&ast_root);
    re__str_destroy(&in_str);
    PASS();
}

TEST(t_compile_concat) {
    re__ast_root ast_root;
    re__prog prog, prog_actual;
    re__compile compile;
    SYM(re__ast_root,
        "(ast"
        "    (concat"
        "        ("
        "            (rune 'a')"
        "            (rune 'b'))))", &ast_root);
    SYM(re__prog,
        "(prog"
        "    (fail)"
        "    (byte 'a' 2)"
        "    (byte 'b' 3)"
        "    (match 0)", &prog);
    re__compile_init(&compile);
    re__prog_init(&prog_actual);
    ASSERT(!re__compile_regex(&compile, &ast_root, &prog_actual));
    ASSERT(re__prog_equals(&prog, &prog_actual));
    re__compile_destroy(&compile);
    re__prog_destroy(&prog);
    re__prog_destroy(&prog_actual);
    re__ast_root_destroy(&ast_root);
    PASS();
}

TEST(t_compile_alt) {
    re__ast_root ast_root;
    re__prog prog, prog_actual;
    re__compile compile;
    SYM(re__ast_root,
        "(ast"
        "    (alt"
        "        ("
        "            (rune 'a')"
        "            (rune 'b'))))", &ast_root);
    SYM(re__prog,
        "(prog"
        "    (fail)"
        "    (split 2 3)"
        "    (byte 'a' 4)"
        "    (byte 'b' 4)"
        "    (match 0)", &prog);
    re__compile_init(&compile);
    re__prog_init(&prog_actual);
    ASSERT(!re__compile_regex(&compile, &ast_root, &prog_actual));
    ASSERT(re__prog_equals(&prog, &prog_actual));
    re__compile_destroy(&compile);
    re__prog_destroy(&prog);
    re__prog_destroy(&prog_actual);
    re__ast_root_destroy(&ast_root);
    PASS();
}

TEST(t_compile_star) {
    re__ast_root ast_root;
    re__prog prog, prog_actual;
    re__compile compile;
    SYM(re__ast_root,
        "(ast"
        "    (quantifier 0 inf greedy"
        "        (rune 'a')))", &ast_root);
    SYM(re__prog,
        "(prog"
        "    (fail)"
        "    (split 2 3)"
        "    (byte 'a' 1)"
        "    (match 0)", &prog);
    re__compile_init(&compile);
    re__prog_init(&prog_actual);
    ASSERT(!re__compile_regex(&compile, &ast_root, &prog_actual));
    ASSERT(re__prog_equals(&prog, &prog_actual));
    re__compile_destroy(&compile);
    re__prog_destroy(&prog);
    re__prog_destroy(&prog_actual);
    re__ast_root_destroy(&ast_root);
    PASS();
}

TEST(t_compile_star_nongreedy) {
    re__ast_root ast_root;
    re__prog prog, prog_actual;
    re__compile compile;
    SYM(re__ast_root,
        "(ast"
        "    (quantifier 0 inf nongreedy"
        "        (rune 'a')))", &ast_root);
    SYM(re__prog,
        "(prog"
        "    (fail)"
        "    (split 3 2)"
        "    (byte 'a' 1)"
        "    (match 0)", &prog);
    re__compile_init(&compile);
    re__prog_init(&prog_actual);
    ASSERT(!re__compile_regex(&compile, &ast_root, &prog_actual));
    ASSERT(re__prog_equals(&prog, &prog_actual));
    re__compile_destroy(&compile);
    re__prog_destroy(&prog);
    re__prog_destroy(&prog_actual);
    re__ast_root_destroy(&ast_root);
    PASS();
}

TEST(t_compile_question) {
    re__ast_root ast_root;
    re__prog prog, prog_actual;
    re__compile compile;
    SYM(re__ast_root,
        "(ast"
        "    (quantifier 0 1 greedy"
        "        (rune 'a')))", &ast_root);
    SYM(re__prog,
        "(prog"
        "    (fail)"
        "    (split 2 3)"
        "    (byte 'a' 3)"
        "    (match 0)", &prog);
    re__compile_init(&compile);
    re__prog_init(&prog_actual);
    ASSERT(!re__compile_regex(&compile, &ast_root, &prog_actual));
    ASSERT(re__prog_equals(&prog, &prog_actual));
    re__compile_destroy(&compile);
    re__prog_destroy(&prog);
    re__prog_destroy(&prog_actual);
    re__ast_root_destroy(&ast_root);
    PASS();
}

TEST(t_compile_question_nongreedy) {
    re__ast_root ast_root;
    re__prog prog, prog_actual;
    re__compile compile;
    SYM(re__ast_root,
        "(ast"
        "    (quantifier 0 1 nongreedy"
        "        (rune 'a')))", &ast_root);
    SYM(re__prog,
        "(prog"
        "    (fail)"
        "    (split 3 2)"
        "    (byte 'a' 3)"
        "    (match 0)", &prog);
    re__compile_init(&compile);
    re__prog_init(&prog_actual);
    ASSERT(!re__compile_regex(&compile, &ast_root, &prog_actual));
    ASSERT(re__prog_equals(&prog, &prog_actual));
    re__compile_destroy(&compile);
    re__prog_destroy(&prog);
    re__prog_destroy(&prog_actual);
    re__ast_root_destroy(&ast_root);
    PASS();
}

TEST(t_compile_plus) {
    re__ast_root ast_root;
    re__prog prog, prog_actual;
    re__compile compile;
    SYM(re__ast_root,
        "(ast"
        "    (quantifier 1 inf greedy"
        "        (rune 'a')))", &ast_root);
    SYM(re__prog,
        "(prog"
        "    (fail)"
        "    (byte 'a' 2)"
        "    (split 1 3)"
        "    (match 0)", &prog);
    re__compile_init(&compile);
    re__prog_init(&prog_actual);
    ASSERT(!re__compile_regex(&compile, &ast_root, &prog_actual));
    ASSERT(re__prog_equals(&prog, &prog_actual));
    re__compile_destroy(&compile);
    re__prog_destroy(&prog);
    re__prog_destroy(&prog_actual);
    re__ast_root_destroy(&ast_root);
    PASS();
}

TEST(t_compile_plus_nongreedy) {
    re__ast_root ast_root;
    re__prog prog, prog_actual;
    re__compile compile;
    SYM(re__ast_root,
        "(ast"
        "    (quantifier 1 inf nongreedy"
        "        (rune 'a')))", &ast_root);
    SYM(re__prog,
        "(prog"
        "    (fail)"
        "    (byte 'a' 2)"
        "    (split 3 1)"
        "    (match 0)", &prog);
    re__compile_init(&compile);
    re__prog_init(&prog_actual);
    ASSERT(!re__compile_regex(&compile, &ast_root, &prog_actual));
    ASSERT(re__prog_equals(&prog, &prog_actual));
    re__compile_destroy(&compile);
    re__prog_destroy(&prog);
    re__prog_destroy(&prog_actual);
    re__ast_root_destroy(&ast_root);
    PASS();
}

TEST(t_compile_quantifier_nomax) {
    re__ast_root ast_root;
    re__prog prog, prog_actual;
    re__compile compile;
    SYM(re__ast_root,
        "(ast"
        "    (quantifier 3 inf greedy"
        "        (rune 'a')))", &ast_root);
    SYM(re__prog,
        "(prog"
        "    (fail)"
        "    (byte 'a' 2)"
        "    (byte 'a' 3)"
        "    (byte 'a' 4)"
        "    (split 3 5)"
        "    (match 0))", &prog);
    re__compile_init(&compile);
    re__prog_init(&prog_actual);
    ASSERT(!re__compile_regex(&compile, &ast_root, &prog_actual));
    ASSERT(re__prog_equals(&prog, &prog_actual));
    re__compile_destroy(&compile);
    re__prog_destroy(&prog);
    re__prog_destroy(&prog_actual);
    re__ast_root_destroy(&ast_root);
    PASS();
}

TEST(t_compile_quantifier_nomax_nongreedy) {
    re__ast_root ast_root;
    re__prog prog, prog_actual;
    re__compile compile;
    SYM(re__ast_root,
        "(ast"
        "    (quantifier 3 inf nongreedy"
        "        (rune 'a')))", &ast_root);
    SYM(re__prog,
        "(prog"
        "    (fail)"
        "    (byte 'a' 2)"
        "    (byte 'a' 3)"
        "    (byte 'a' 4)"
        "    (split 5 3)"
        "    (match 0))", &prog);
    re__compile_init(&compile);
    re__prog_init(&prog_actual);
    ASSERT(!re__compile_regex(&compile, &ast_root, &prog_actual));
    ASSERT(re__prog_equals(&prog, &prog_actual));
    re__compile_destroy(&compile);
    re__prog_destroy(&prog);
    re__prog_destroy(&prog_actual);
    re__ast_root_destroy(&ast_root);
    PASS();
}

TEST(t_compile_quantifier_minmax) {
    re__ast_root ast_root;
    re__prog prog, prog_actual;
    re__compile compile;
    SYM(re__ast_root,
        "(ast"
        "    (quantifier 3 5 greedy"
        "        (rune 'a')))", &ast_root);
    SYM(re__prog,
        "(prog"
        "    (fail)"
        "    (byte 'a' 2)"
        "    (byte 'a' 3)"
        "    (byte 'a' 4)"
        "    (split 5 8)"
        "    (byte 'a' 6)"
        "    (split 7 8)"
        "    (byte 'a' 8)"
        "    (match 0))", &prog);
    re__compile_init(&compile);
    re__prog_init(&prog_actual);
    ASSERT(!re__compile_regex(&compile, &ast_root, &prog_actual));
    ASSERT(re__prog_equals(&prog, &prog_actual));
    re__compile_destroy(&compile);
    re__prog_destroy(&prog);
    re__prog_destroy(&prog_actual);
    re__ast_root_destroy(&ast_root);
    PASS();
}

TEST(t_compile_quantifier_minmax_nongreedy) {
    re__ast_root ast_root;
    re__prog prog, prog_actual;
    re__compile compile;
    SYM(re__ast_root,
        "(ast"
        "    (quantifier 3 5 nongreedy"
        "        (rune 'a')))", &ast_root);
    SYM(re__prog,
        "(prog"
        "    (fail)"
        "    (byte 'a' 2)"
        "    (byte 'a' 3)"
        "    (byte 'a' 4)"
        "    (split 8 5)"
        "    (byte 'a' 6)"
        "    (split 8 7)"
        "    (byte 'a' 8)"
        "    (match 0))", &prog);
    re__compile_init(&compile);
    re__prog_init(&prog_actual);
    ASSERT(!re__compile_regex(&compile, &ast_root, &prog_actual));
    ASSERT(re__prog_equals(&prog, &prog_actual));
    re__compile_destroy(&compile);
    re__prog_destroy(&prog);
    re__prog_destroy(&prog_actual);
    re__ast_root_destroy(&ast_root);
    PASS();
}

TEST(t_compile_group) {
    re__ast_root ast_root;
    re__prog prog, prog_actual;
    re__compile compile;
    SYM(re__ast_root,
        "(ast"
        "    (group () 0"
        "        (rune 'a')))", &ast_root);
    SYM(re__prog,
        "(prog"
        "    (fail)"
        "    (save 0 2)"
        "    (byte 'a' 3)"
        "    (save 1 4)"
        "    (match 0))", &prog);
    re__compile_init(&compile);
    re__prog_init(&prog_actual);
    ASSERT(!re__compile_regex(&compile, &ast_root, &prog_actual));
    ASSERT(re__prog_equals(&prog, &prog_actual));
    re__compile_destroy(&compile);
    re__prog_destroy(&prog);
    re__prog_destroy(&prog_actual);
    re__ast_root_destroy(&ast_root);
    PASS();
}

TEST(t_compile_group_nonmatching) {
    re__ast_root ast_root;
    re__prog prog, prog_actual;
    re__compile compile;
    SYM(re__ast_root,
        "(ast"
        "    (group (nonmatching)"
        "        (rune 'a')))", &ast_root);
    SYM(re__prog,
        "(prog"
        "    (fail)"
        "    (byte 'a' 2)"
        "    (match 0))", &prog);
    re__compile_init(&compile);
    re__prog_init(&prog_actual);
    ASSERT(!re__compile_regex(&compile, &ast_root, &prog_actual));
    ASSERT(re__prog_equals(&prog, &prog_actual));
    re__compile_destroy(&compile);
    re__prog_destroy(&prog);
    re__prog_destroy(&prog_actual);
    re__ast_root_destroy(&ast_root);
    PASS();
}

SUITE(s_compile) {
    RUN_TEST(t_compile_rune_ascii);
    FUZZ_TEST(t_compile_rune_unicode);
    RUN_TEST(t_compile_str);
    FUZZ_TEST(t_compile_str_thrash);
    RUN_TEST(t_compile_concat);
    RUN_TEST(t_compile_alt);
    RUN_TEST(t_compile_star);
    RUN_TEST(t_compile_star_nongreedy);
    RUN_TEST(t_compile_question);
    RUN_TEST(t_compile_question_nongreedy);
    RUN_TEST(t_compile_plus);
    RUN_TEST(t_compile_plus_nongreedy);
    RUN_TEST(t_compile_quantifier_nomax);
    RUN_TEST(t_compile_quantifier_nomax_nongreedy);
    RUN_TEST(t_compile_quantifier_minmax);
    RUN_TEST(t_compile_quantifier_minmax_nongreedy);
    RUN_TEST(t_compile_group);
    RUN_TEST(t_compile_group_nonmatching);
}

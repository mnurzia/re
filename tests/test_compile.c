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

TEST(t_compile_str_basic) {
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

TEST(t_compile_concat_basic) {
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

SUITE(s_compile) {
    RUN_TEST(t_compile_rune_ascii);
    FUZZ_TEST(t_compile_rune_unicode);
    RUN_TEST(t_compile_str_basic);
    FUZZ_TEST(t_compile_str_thrash);
    RUN_TEST(t_compile_concat_basic);
}

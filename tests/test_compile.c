#include "test_compile.h"

#include "test_ast.h"
#include "test_prog.h"

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

SUITE(s_compile) {
    RUN_TEST(t_compile_rune);
}

#ifndef TEST_AST_H
#define TEST_AST_H

#include "test_harness.h"

SUITE(s_ast);

int re__ast_root_to_sym(sym_build* parent, re__ast_root ast_root);
int re__ast_root_from_sym(sym_walk* parent, re__ast_root* ast_root);

int re__ast_assert_type_to_sym(sym_build* parent, re__ast_assert_type assert_type);
int re__ast_assert_type_from_sym(sym_walk* parent, re__ast_assert_type* assert_type);

SUITE(s_ast_root);

#endif

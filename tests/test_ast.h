#include "test_harness.h"

SUITE(s_ast);

int re__ast_root_to_sym(sym_build* parent, re__ast_root ast_root);
int re__ast_root_from_sym(sym_walk* parent, re__ast_root* ast_root);

SUITE(s_ast_root);

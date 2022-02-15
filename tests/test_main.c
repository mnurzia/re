#define MPTEST_IMPLEMENTATION
#include "test_harness.h"

#include "test_ast.h"
#include "test_charclass.h"
#include "test_parse.h"
#include "test_range.h"

int main() {
    printf("\x1b[3J");
    MPTEST_MAIN_BEGIN();
    MPTEST_ENABLE_LEAK_CHECKING();
    RUN_SUITE(s_ast);
    RUN_SUITE(s_ast_root);
    RUN_SUITE(s_charclass);
    RUN_SUITE(s_charclass_builder);
    RUN_SUITE(s_range);
    RUN_SUITE(s_parse);
    MPTEST_MAIN_END();
}

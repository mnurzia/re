#define MPTEST_IMPLEMENTATION
#include "test_harness.h"

#include "test_ast.h"
#include "test_charclass.h"
#include "test_compile.h"
#include "test_compile_charclass.h"
#include "test_exec.h"
#include "test_match.h"
#include "test_parse.h"
#include "test_prog.h"
#include "test_range.h"
#include "test_regr.h"
#include "test_rune_data.h"

int main(int argc, char** argv)
{
  printf("\x1b[3J");
  MPTEST_MAIN_BEGIN_ARGS(argc, argv);
  RUN_SUITE(s_regr);
  RUN_SUITE(s_ast);
  RUN_SUITE(s_charclass);
  RUN_SUITE(s_compile);
  RUN_SUITE(s_compile_charclass);
  RUN_SUITE(s_match);
  /*RUN_SUITE(s_exec);*/
  RUN_SUITE(s_range);
  RUN_SUITE(s_rune_data);
  RUN_SUITE(s_parse);
  RUN_SUITE(s_prog);
  MPTEST_MAIN_END();
  return 0;
}

#include "test_regr.h"

/* 2022-06-06: libfuzzer */
TEST(t_regr_parse_fuzz_0000)
{
  re reg;
  re_init_sz_flags(&reg, "\"*[\x0a", 4, 0);
  re_destroy(&reg);
  PASS();
}

/* 2022-05-24: libfuzzer */
TEST(t_regr_parse_fuzz_0001)
{
  re reg;
  re_init_sz_flags(&reg, "\x28\x00\x00\x3f", 4, 0);
  re_destroy(&reg);
  PASS();
}

TEST(t_regr_parse_fuzz_0002)
{
  re reg;
  re_init_sz_flags(&reg, "|\xf8$\xe5", 4, 0);
  re_destroy(&reg);
  PASS();
}

TEST(t_regr_parse_fuzz_0003)
{
  re reg;
  re_init_sz_flags(&reg, "||[\x01~-||", 8, 0);
  re_destroy(&reg);
  PASS();
}

TEST(t_regr_parse_fuzz_0004)
{
  re reg;
  re_init_sz_flags(&reg, "[^?\0E\0\\D]", 8, 0);
  re_destroy(&reg);
  PASS();
}

TEST(t_regr_parse_fuzz_0005)
{
  re reg;
  re_init_sz_flags(&reg, ".\\QN|Q", 6, 0);
  re_destroy(&reg);
  PASS();
}

TEST(t_regr_parse_fuzz_0006)
{
  re reg;
  re_init_sz_flags(
      &reg, "[^\0\0\x08\0=============[\\W..[\\\0([\\..5=M,]", 37, 0);
  re_destroy(&reg);
  PASS();
}

SUITE(s_regr)
{
  RUN_TEST(t_regr_parse_fuzz_0000);
  RUN_TEST(t_regr_parse_fuzz_0001);
  RUN_TEST(t_regr_parse_fuzz_0002);
  RUN_TEST(t_regr_parse_fuzz_0003);
  RUN_TEST(t_regr_parse_fuzz_0004);
  RUN_TEST(t_regr_parse_fuzz_0005);
  RUN_TEST(t_regr_parse_fuzz_0006);
}

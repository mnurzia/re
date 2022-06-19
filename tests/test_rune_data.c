#include "test_rune_data.h"

TEST(t_rune_data_init_destroy_none)
{
  re__rune_data rune_data;
  re__rune_data_init(&rune_data);
  re__rune_data_destroy(&rune_data);
  PASS();
}

SUITE(s_rune_data) { RUN_TEST(t_rune_data_init_destroy_none); }

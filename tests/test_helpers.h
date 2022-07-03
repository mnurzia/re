#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include "test_harness.h"

re_rune re_rune_rand(void);

re_rune re__rune_rand_above(re_rune minimum);

re_error re_test_merge_sym(const char** l, char** out);

#endif

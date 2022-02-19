#ifndef TEST_RANGE_H
#define TEST_RANGE_H

#include "test_harness.h"

int re__byte_range_to_sym(mptest_sym_build* build, re__byte_range br);
int re__byte_range_from_sym(mptest_sym_walk* walk, re__byte_range* br);

int re__rune_range_to_sym(mptest_sym_build* build, re__rune_range rr);
int re__rune_range_from_sym(mptest_sym_walk* walk, re__rune_range* rr);

re__byte_range re__byte_range_random(void);
re__rune_range re__rune_range_random(void);

SUITE(s_range);

#endif

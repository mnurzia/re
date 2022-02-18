#include "test_harness.h"

int re__charclass_to_sym(sym_build* parent, const re__charclass charclass);
int re__charclass_from_sym(sym_walk* parent, re__charclass* charclass);

SUITE(s_charclass);
SUITE(s_charclass_builder);

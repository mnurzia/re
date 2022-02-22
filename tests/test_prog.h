#ifndef TEST_PROG_H
#define TEST_PROG_H

#include "test_harness.h"

int re__prog_inst_to_sym(sym_build* parent, re__prog_inst prog_inst);
int re__prog_inst_from_sym(sym_walk* parent, re__prog_inst* prog_inst);

int re__prog_to_sym(sym_build* parent, re__prog prog);
int re__prog_from_sym(sym_walk* parent, re__prog* prog);

SUITE(s_prog_inst);

#endif

#ifndef TEST_HARNESS_H
#define TEST_HARNESS_H

#include "mptest_build.h"

#include "../re_internal.h"

#define ASSERT_ERR_NOMEMm(expr, msg, lab)                                      \
  do {                                                                         \
    re_error _err = RE_ERROR_NONE;                                             \
    if ((_err = expr) == RE_ERROR_NOMEM) {                                     \
      goto lab;                                                                \
    } else if (_err) {                                                         \
      FAIL();                                                                  \
    }                                                                          \
  } while (0)

#define ASSERT_ERR_NOMEM(expr, lab) ASSERT_ERR_NOMEMm(expr, #expr, lab)

#define ASSERT_PARSE_ERR_NOMEMm(expr, msg, lab)                                \
  do {                                                                         \
    re_error _err = RE_ERROR_NONE;                                             \
    if ((_err = expr) == RE_ERROR_NOMEM) {                                     \
      goto lab;                                                                \
    } else if (_err != RE_ERROR_PARSE) {                                       \
      FAIL();                                                                  \
    }                                                                          \
  } while (0)

#define ASSERT_PARSE_ERR_NOMEM(expr, lab)                                      \
  ASSERT_PARSE_ERR_NOMEMm(expr, #expr, lab)

#endif

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

/* https://en.wikipedia.org/wiki/UTF-8 */
#define EX_UTF8_VALID_1 "$"
#define EX_UTF8_VALID_2 "\xc2\xa3"
#define EX_UTF8_VALID_3 "\xe0\xa4\xb9"
#define EX_UTF8_VALID_4 "\xf0\x90\x8d\x88"
#define EX_UTF8_INVALID_OVERLONG "\xc0\x81"
#define EX_UTF8_INVALID_SURROGATE "\xed\xa0\xb0"
#define EX_UTF8_INVALID_TOOBIG "\xf4\x90\xb0\xb0"
#define EX_UTF8_INVALID_UNFINISHED "\xc2"
#define EX_UTF8_INVALID_UNFINISHED_OVERLONG "\xc0"
#define EX_UTF8_INVALID_UNDEFINED "\xff"

#endif

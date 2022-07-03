#include "test_helpers.h"

re_rune re_rune_rand()
{
  int cls = (int)RAND_PARAM(4);
  re_rune ch = 0;
  if (cls == 0) {
    int sub = (int)RAND_PARAM(3);
    if (sub == 0) {
      ch = (int)RAND_PARAM(10) + '0';
    } else if (sub == 1) {
      ch = (int)RAND_PARAM(26) + 'a';
    } else if (sub == 2) {
      ch = (int)RAND_PARAM(26) + 'A';
    }
  } else if (cls == 1) {
    ch = (int)RAND_PARAM(0x800 - 0x80) + 0x80;
  } else if (cls == 2) {
    int sub = (int)RAND_PARAM(2);
    if (sub == 0) {
      ch = (int)RAND_PARAM(0xD800 - 0x800) + 0x800;
    } else if (sub == 1) {
      ch = (int)RAND_PARAM(0x10000 - 0xE000) + 0xE000;
    }
  } else if (cls == 3) {
    ch = (int)RAND_PARAM(0x10FFFF - 0x10000) + 0x10000;
  }
  return ch;
}

re_error re_test_merge_sym(const char** l, char** out)
{
  mn__str str_out;
  mn__str_init(&str_out);
  while (*l) {
    ASSERT_ERR_NOMEM(mn__str_cat_s(&str_out, *l), error);
    l++;
  }
  *out = MN_MALLOC(mn__str_size(&str_out) + 1);
  if (!*out) {
    goto error;
  }
  {
    mn_size i = 0;
    while (i < mn__str_size(&str_out) + 1) {
      (*out)[i] = mn__str_get_data(&str_out)[i];
      i++;
    }
  }
  mn__str_destroy(&str_out);
  return RE_ERROR_NONE;
error:
  mn__str_destroy(&str_out);
  return RE_ERROR_NOMEM;
}

re_rune re__rune_rand_above(re_rune minimum)
{
  int class = RAND_PARAM(4);
  re_rune min;
  re_rune max = -1;
  MN_ASSERT(minimum <= 0x10FFFF);
  while (max < minimum) {
    if (class == 0) {
      min = 0;
      max = 0x7F;
    } else if (class == 1) {
      min = 0x80;
      max = 0x7FF;
    } else if (class == 2) {
      min = 0x800;
      max = 0xFFFF;
    } else {
      min = 0x10000;
      max = 0x10FFFF;
    }
    class ++;
  }
  min = MN__MAX(min, minimum);
  if (max == min) {
    return max;
  }
  return (mn_int32)RAND_PARAM((mptest_rand)max - (mptest_rand)min) + min;
}

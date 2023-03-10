typedef unsigned char uint8_t;

#include <stddef.h>

#define MPTEST_IMPLEMENTATION

#include "../re_api.h"

int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size)
{
  re reg;
  re_init_sz_flags(&reg, (const char*)Data, Size, 0);
  re_is_match(&reg, "", 0, 'U');
  re_destroy(&reg);
  return 0;
}

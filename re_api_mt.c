#include "re_internal.h"

#if RE_USE_THREAD

MN_API re_error re_mt_init(re_mt* reg_mt, re* reg)
{
  reg_mt->data = (re_mt_data*)MN_MALLOC(sizeof(re_mt_data));
  if (!reg_mt->data) {
    return RE_ERROR_NOMEM;
  }
  reg_mt->data->reg = reg;
  reg_mt->data->compile_status = 0;
  mn__str_init(&reg_mt->data->error_string);
  mn__str_view_init_null(&reg_mt->data->error_string_view);
  return RE_ERROR_NONE;
}

MN_API void re_mt_destroy(re_mt* reg_mt)
{
  if (reg_mt->data) {
    MN_FREE(reg_mt->data);
  }
  mn__str_destroy(&reg_mt->data->error_string);
}

#endif

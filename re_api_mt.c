#include "re_internal.h"

#if RE_USE_THREAD

MN_API re_error re_mt_init(re_mt* reg_mt, re* reg)
{
  re_error err = RE_ERROR_NONE;
  reg_mt->data = (re_mt_data*)MN_MALLOC(sizeof(re_mt_data));
  if (!reg_mt->data) {
    err = RE_ERROR_NOMEM;
    goto error;
  }
  reg_mt->data->reg = reg;
  reg_mt->data->compile_status = 0;
  mn__str_init(&reg_mt->data->error_string);
  mn__str_view_init_null(&reg_mt->data->error_string_view);
error:
  return err;
}

MN_API void re_mt_destroy(re_mt* reg_mt)
{
  mn__str_destroy(&reg_mt->data->error_string);
  if (reg_mt->data) {
    MN_FREE(reg_mt->data);
  }
}

MN_INTERNAL re_error re__mt_match_prepare_progs(re_mt* reg_mt, int fwd, int rev)
{
  re_error err = RE_ERROR_NONE;
  if (fwd && !(reg_mt->data->compile_status & 1)) {
    re__mutex_lock(&reg_mt->data->reg->data->program_mutex);
    if ((err = re__match_prepare_progs(reg_mt->data->reg, 1, 0, 1, 0, 1))) {
      goto error;
    }
    re__mutex_unlock(&reg_mt->data->reg->data->program_mutex);
    reg_mt->data->compile_status |= 1;
  }
  if (rev && !(reg_mt->data->compile_status & 2)) {
    re__mutex_lock(&reg_mt->data->reg->data->program_reverse_mutex);
    if ((err = re__match_prepare_progs(reg_mt->data->reg, 0, 1, 0, 1, 1))) {
      goto error;
    }
    re__mutex_unlock(&reg_mt->data->reg->data->program_reverse_mutex);
    reg_mt->data->compile_status |= 2;
  }
error:
  return err;
}

#if 0

MN_API re_error re_mt_is_match(
    re_mt* reg_mt, const char* text, mn_size text_size,
    re_anchor_type anchor_type)
{
  re_error err = RE_ERROR_NONE;
  if (anchor_type == RE_ANCHOR_BOTH) {
    if ((err = re__mt_match_prepare_progs(reg_mt, 1, 0))) {
      goto error;
    }
  } else if (anchor_type == RE_ANCHOR_START) {
    if ((err = re__mt_match_prepare_progs(reg_mt, 1, 0))) {
      goto error;
    }
  } else if (anchor_type == RE_ANCHOR_END) {
    if ((err = re__mt_match_prepare_progs(reg_mt, 0, 1))) {
      goto error;
    }
  } else if (anchor_type == RE_UNANCHORED) {
    if ((err = re__mt_match_prepare_progs(reg_mt, 1, 0))) {
      goto error;
    }
  } else {
    return RE_ERROR_INVALID;
  }
error:
  return err;
}

#endif

#endif

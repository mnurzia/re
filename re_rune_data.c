#include "re_internal.h"

MN_INTERNAL void re__rune_data_init(re__rune_data* rune_data)
{
  int i;
  rune_data->range_storage = MN_NULL;
  rune_data->range_storage_size = 0;
  rune_data->range_storage_alloc = 0;
  for (i = 0; i < RE__RUNE_DATA_IMMEDIATE_STORAGE_SIZE; i++) {
    rune_data->immediate_range_storage[i] = 0;
  }
}

MN_INTERNAL void re__rune_data_destroy(re__rune_data* rune_data)
{
  if (rune_data->range_storage) {
    MN_FREE(rune_data->range_storage);
  }
}

MN_INTERNAL re_error re__rune_data_add(re__rune_data* rune_data, re_rune ch)
{
  if (rune_data->range_storage == MN_NULL) {
    if (rune_data->range_storage_size < RE__RUNE_DATA_IMMEDIATE_STORAGE_SIZE) {
      rune_data->immediate_range_storage[rune_data->range_storage_size++] = ch;
    } else {
      int i;
      mn_size new_alloc = rune_data->range_storage_size + 1;
      rune_data->range_storage = MN_MALLOC(sizeof(re_rune) * new_alloc);
      if (rune_data->range_storage == MN_NULL) {
        return RE_ERROR_NOMEM;
      }
      for (i = 0; i < RE__RUNE_DATA_IMMEDIATE_STORAGE_SIZE; i++) {
        rune_data->range_storage[i] = rune_data->immediate_range_storage[i];
      }
      rune_data->range_storage[RE__RUNE_DATA_IMMEDIATE_STORAGE_SIZE] = ch;
      rune_data->range_storage_size = new_alloc;
      rune_data->range_storage_alloc = new_alloc;
    }
  } else {
    if (rune_data->range_storage_size < rune_data->range_storage_alloc) {
      rune_data->range_storage[rune_data->range_storage_size++] = ch;
    } else {
      mn_size new_alloc = rune_data->range_storage_alloc + 1;
      rune_data->range_storage =
          MN_REALLOC(rune_data->range_storage, sizeof(re_rune) * new_alloc);
      if (rune_data->range_storage == MN_NULL) {
        return RE_ERROR_NOMEM;
      }
      rune_data->range_storage[rune_data->range_storage_size++] = ch;
      rune_data->range_storage_alloc = new_alloc;
    }
  }
  return RE_ERROR_NONE;
}

MN_INTERNAL re_error re__rune_data_casefold(
    re__rune_data* rune_data, re_rune ch, re_rune** runes, mn_size* runes_size)
{
  int needs_fold = 0;
  re_rune norm_char = 0;
  re_error err = RE_ERROR_NONE;
  rune_data->range_storage_size = 0;
  if (ch >= 'A' && ch <= 'Z') {
    needs_fold = 1;
    norm_char = ch;
  } else if (ch >= 'a' && ch <= 'z') {
    needs_fold = 1;
    /* ascii symmetry */
    norm_char = ch - 32;
  }
  if (!needs_fold) {
    if ((err = re__rune_data_add(rune_data, ch))) {
      return err;
    }
  } else {
    if ((err = re__rune_data_add(rune_data, norm_char))) {
      return err;
    }
    if ((err = re__rune_data_add(rune_data, norm_char + 32))) {
      return err;
    }
  }
  if (rune_data->range_storage != MN_NULL) {
    *runes = rune_data->range_storage;
    *runes_size = rune_data->range_storage_size;
  } else {
    *runes = rune_data->immediate_range_storage;
    *runes_size = rune_data->range_storage_size;
  }
  return RE_ERROR_NONE;
}

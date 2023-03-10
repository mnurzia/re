#include "re_internal.h"

MN__VEC_IMPL_FUNC(re__rune_range, init)
MN__VEC_IMPL_FUNC(re__rune_range, destroy)
MN__VEC_IMPL_FUNC(re__rune_range, push)
MN__VEC_IMPL_FUNC(re__rune_range, get)
MN__VEC_IMPL_FUNC(re__rune_range, set)
MN__VEC_IMPL_FUNC(re__rune_range, insert)
MN__VEC_IMPL_FUNC(re__rune_range, size)
MN__VEC_IMPL_FUNC(re__rune_range, clear)
MN__VEC_IMPL_FUNC(re__rune_range, get_data)

/* Holds a single ASCII character class. */
typedef struct re__charclass_ascii {
  /* Length of name */
  unsigned char name_size;
  const char* name;
  unsigned char num_classes;
  const unsigned char* classes;
} re__charclass_ascii;

/* Table of ASCII character classes */
/* Since ascii characters fit into one byte, we can fit them into uchars. */
static const re__charclass_ascii re__charclass_ascii_defaults[] = {
    {5, "alnum", 3, (unsigned char*)"\x30\x39\x41\x5A\x61\x7A"},
    {5, "alpha", 2, (unsigned char*)"\x41\x5A\x61\x7A"},
    {5, "ascii", 1, (unsigned char*)"\x00\x7F"},
    {5, "blank", 2, (unsigned char*)"\x09\x09\x20\x20"},
    {5, "cntrl", 2, (unsigned char*)"\x00\x1F\x7F\x7F"},
    {5, "digit", 1, (unsigned char*)"\x30\x39"},
    {5, "graph", 1, (unsigned char*)"\x21\x7E"},
    {5, "lower", 1, (unsigned char*)"\x61\x7A"},
    {10, "perl_space", 3, (unsigned char*)"\x09\x0A\x0C\x0D\x20\x20"},
    {5, "print", 1, (unsigned char*)"\x20\x7E"},
    {5, "punct", 4, (unsigned char*)"\x21\x2F\x3A\x40\x5B\x60\x7B\x7E"},
    {5, "space", 2, (unsigned char*)"\x09\x0D\x20\x20"},
    {5, "upper", 1, (unsigned char*)"\x41\x5A"},
    {4, "word", 4, (unsigned char*)"\x30\x39\x41\x5A\x5F\x5F\x61\x7A"},
    {6, "xdigit", 3, (unsigned char*)"\x30\x39\x41\x46\x61\x66"}};

#define RE__CHARCLASS_ASCII_DEFAULTS_SIZE                                      \
  (sizeof(re__charclass_ascii_defaults) / sizeof(re__charclass_ascii))

MN_INTERNAL void re__charclass_destroy(re__charclass* charclass)
{
  if (charclass->ranges) {
    MN_FREE(charclass->ranges);
  }
}

MN_INTERNAL const re__rune_range*
re__charclass_get_ranges(const re__charclass* charclass)
{
  return charclass->ranges;
}

MN_INTERNAL mn_size re__charclass_size(const re__charclass* charclass)
{
  return charclass->ranges_size;
}

MN_INTERNAL const re__charclass_ascii*
re__charclass_ascii_find(mn__str_view name_view)
{
  mn_size i;
  const re__charclass_ascii* found = MN_NULL;
  /* Search table for the matching named character class */
  for (i = 0; i < RE__CHARCLASS_ASCII_DEFAULTS_SIZE; i++) {
    const re__charclass_ascii* cur = &re__charclass_ascii_defaults[i];
    mn__str_view temp_view;
    mn__str_view_init_n(
        &temp_view, (mn_char*)cur->name, (mn_size)cur->name_size);
    if (mn__str_view_cmp(&temp_view, &name_view) == 0) {
      found = cur;
      break;
    }
  }
  return found;
}

typedef struct re__charclass_inverter {
  int should_invert;
  re__rune_range temp;
  re_rune last_max;
} re__charclass_inverter;

void re__charclass_inverter_init(
    re__charclass_inverter* inverter, int should_invert)
{
  inverter->should_invert = should_invert;
  inverter->temp.min = 0;
  inverter->temp.max = 0;
  inverter->last_max = -1;
}
void re__charclass_inverter_push(
    re__charclass_inverter* inverter, re__rune_range range)
{
  if (inverter->should_invert) {
    if (inverter->last_max == -1) {
      inverter->temp.min = 0;
    } else {
      inverter->temp.min = inverter->last_max;
    }
    inverter->temp.max = range.min - 1;
    inverter->last_max = range.max + 1;
  } else {
    inverter->temp.min = range.min;
    inverter->temp.max = range.max;
  }
}
void re__charclass_inverter_end(re__charclass_inverter* inverter)
{
  if (inverter->should_invert) {
    if (inverter->last_max == -1) {
      inverter->temp.min = 0;
      inverter->temp.max = RE_RUNE_MAX;
    } else {
      inverter->temp.min = inverter->last_max;
      inverter->temp.max = RE_RUNE_MAX;
    }
  } else {
    inverter->last_max = 0;
  }
}

int re__charclass_inverter_hasnext(re__charclass_inverter* inverter)
{
  if (inverter->should_invert) {
    return inverter->temp.max >= inverter->temp.min;
  } else {
    return inverter->last_max == -1;
  }
}

re__rune_range re__charclass_inverter_next(re__charclass_inverter* inverter)
{
  return inverter->temp;
}

MN_INTERNAL void re__charclass_builder_init(
    re__charclass_builder* builder, re__rune_data* rune_data)
{
  re__rune_range_vec_init(&builder->ranges);
  builder->should_invert = 0;
  builder->should_fold = 0;
  builder->highest = -1;
  builder->rune_data = rune_data;
}

MN_INTERNAL void re__charclass_builder_begin(re__charclass_builder* builder)
{
  re__rune_range_vec_clear(&builder->ranges);
  builder->should_invert = 0;
  builder->should_fold = 0;
  builder->highest = -1;
}

MN_INTERNAL void re__charclass_builder_destroy(re__charclass_builder* builder)
{
  re__rune_range_vec_destroy(&builder->ranges);
}

MN_INTERNAL void re__charclass_builder_invert(re__charclass_builder* builder)
{
  builder->should_invert = 1;
}

MN_INTERNAL void re__charclass_builder_fold(re__charclass_builder* builder)
{
  builder->should_fold = 1;
}

/* Insert a range into the builder. */
/* Inserts in sorted order, that is, all ranges are ordered by their low bound.
 */
MN_INTERNAL re_error re__charclass_builder_insert_range_internal(
    re__charclass_builder* builder, re__rune_range range)
{
  re_error err = RE_ERROR_NONE;
  mn_size i;
  mn_size max = re__rune_range_vec_size(&builder->ranges);
  /* The range should be normalized. */
  MN_ASSERT(range.min <= range.max);
  /* We don't want to penalize people who sort their character classes already,
   * so we check if the lower bound is higher than the absolute highest bound,
   * and just push it on the end of the list of ranges */
  /* Whether this optimization is actually good is unknown */
  if (range.min > builder->highest) {
    goto just_push;
  }
  for (i = 0; i < max; i++) {
    re__rune_range cmp = re__rune_range_vec_get(&builder->ranges, i);
    if (range.min < cmp.min) {
      /* Keep track of the highest range */
      if (range.max > builder->highest) {
        builder->highest = range.max;
      }
      if ((err = re__rune_range_vec_insert(&builder->ranges, i, range))) {
        return err;
      }
      /* Exit early */
      return err;
    }
  }
just_push:
  /* Range is higher than everything else */
  builder->highest = range.max;
  return re__rune_range_vec_push(&builder->ranges, range);
}

/* Insert a range into the builder. */
/* Inserts in sorted order, that is, all ranges are ordered by their low bound.
 */
MN_INTERNAL re_error re__charclass_builder_insert_range(
    re__charclass_builder* builder, re__rune_range range)
{
  MN_ASSERT(range.min >= 0);
  MN_ASSERT(range.max <= RE_RUNE_MAX);
  if (builder->should_fold) {
    re_rune i;
    for (i = range.min; i <= range.max; i++) {
      re_rune* folded;
      int folded_size = re__rune_data_casefold(builder->rune_data, i, &folded);
      int j;
      for (j = 0; j < folded_size; j++) {
        re_error err = RE_ERROR_NONE;
        re__rune_range temp;
        temp.min = folded[j];
        temp.max = folded[j];
        if ((err =
                 re__charclass_builder_insert_range_internal(builder, temp))) {
          return err;
        }
      }
    }
    return RE_ERROR_NONE;
  } else {
    return re__charclass_builder_insert_range_internal(builder, range);
  }
}

/* Insert a set of ranges into a builder. */
MN_INTERNAL re_error re__charclass_builder_insert_ranges(
    re__charclass_builder* builder, const re__rune_range* ranges,
    mn_size ranges_size, int inverted)
{
  mn_size i;
  re_error err = RE_ERROR_NONE;
  re__charclass_inverter inverter;
  re__charclass_inverter_init(&inverter, inverted);
  for (i = 0; i < ranges_size; i++) {
    /* Iterate through ranges, adding them all */
    /* Perhaps could be optimized more */
    re__charclass_inverter_push(&inverter, ranges[i]);
    if (re__charclass_inverter_hasnext(&inverter)) {
      if ((err = re__charclass_builder_insert_range(
               builder, re__charclass_inverter_next(&inverter)))) {
        return err;
      }
    }
  }
  re__charclass_inverter_end(&inverter);
  if (re__charclass_inverter_hasnext(&inverter)) {
    if ((err = re__charclass_builder_insert_range(
             builder, re__charclass_inverter_next(&inverter)))) {
      return err;
    }
  }
  return err;
}

MN_INTERNAL re_error re__charclass_builder_insert_property(
    re__charclass_builder* builder, mn__str_view str, int inverted)
{
  re_error err = RE_ERROR_NONE;
  re__rune_range* ranges;
  mn_size ranges_size;
  if ((err = re__rune_data_get_property(
           builder->rune_data, mn__str_view_get_data(&str),
           mn__str_view_size(&str), &ranges, &ranges_size))) {
    return err;
  }
  return re__charclass_builder_insert_ranges(
      builder, ranges, ranges_size, inverted);
}

MN_INTERNAL re_error re__charclass_builder_insert_ascii_internal(
    re__charclass_builder* builder, const re__charclass_ascii* ascii_cc,
    int inverted)
{
  re_error err = RE_ERROR_NONE;
  mn_size i;
  re__rune_range temp;
  re__charclass_inverter inverter;
  re__charclass_inverter_init(&inverter, inverted);
  for (i = 0; i < ascii_cc->num_classes; i++) {
    temp.min = ascii_cc->classes[i * 2];
    temp.max = ascii_cc->classes[i * 2 + 1];
    re__charclass_inverter_push(&inverter, temp);
    if (re__charclass_inverter_hasnext(&inverter)) {
      if ((err = re__charclass_builder_insert_range(
               builder, re__charclass_inverter_next(&inverter)))) {
        return err;
      }
    }
  }
  re__charclass_inverter_end(&inverter);
  if (re__charclass_inverter_hasnext(&inverter)) {
    if ((err = re__charclass_builder_insert_range(
             builder, re__charclass_inverter_next(&inverter)))) {
      return err;
    }
  }
  return err;
}

MN_INTERNAL re_error re__charclass_builder_insert_ascii_class(
    re__charclass_builder* builder, re__charclass_ascii_type type, int inverted)
{
  return re__charclass_builder_insert_ascii_internal(
      builder, &re__charclass_ascii_defaults[type], inverted);
}

/* Returns RE_ERROR_INVALID if not found */
MN_INTERNAL re_error re__charclass_builder_insert_ascii_class_by_str(
    re__charclass_builder* builder, mn__str_view name, int inverted)
{
  const re__charclass_ascii* found = re__charclass_ascii_find(name);
  /* Found is NULL if we didn't find anything during the loop */
  if (found == MN_NULL) {
    return RE_ERROR_INVALID;
  }
  return re__charclass_builder_insert_ascii_internal(builder, found, inverted);
}

/* I'm proud of this function. */
MN_INTERNAL re_error re__charclass_builder_finish(
    re__charclass_builder* builder, re__charclass* charclass)
{
  re_error err = RE_ERROR_NONE;
  /* Temporary range */
  re__rune_range temp;
  mn_size read_ptr;
  mn_size write_ptr = 0;
  mn_size ranges_size = re__rune_range_vec_size(&builder->ranges);
  re__charclass_inverter inverter;
  temp.min = -1;
  temp.max = -1;
  /* We write back to the range buffer used for building, so we reserve an extra
   * element for the possibility of the ending [x, RUNE_MAX] range */
  if ((err = re__rune_range_vec_push(&builder->ranges, temp))) {
    return err;
  }
  re__charclass_inverter_init(&inverter, builder->should_invert);
  for (read_ptr = 0; read_ptr < ranges_size; read_ptr++) {
    re__rune_range cur = re__rune_range_vec_get(&builder->ranges, read_ptr);
    MN_ASSERT(cur.min <= cur.max);
    if (temp.min == -1) {
      /* First range */
      temp.min = cur.min;
      temp.max = cur.max;
      /* Result */
      /* temp_min      temp_max
       * ***************         */
    } else if (cur.min <= temp.max) {
      /* Some kind of intersection */
      if (cur.max > temp.max) {
        /* Current range intersects with temp.min/temp.max range */
        /* temp.min      temp.max
         * ***************
         *             cur.min       cur.max
         *             ***************           */
        /* Extend range */
        temp.max = cur.max;
        /* Result: */
        /* temp.min                  temp.max
         * ***************************         */
      } else {
        /* Current range is contained within temp.min/temp.max range */
        /* temp.min      temp.max
         * ***************
         *  cur.min cur.max
         *  *********         */
        /* Nothing needs to be done */
        /* Result: */
        /* temp.min      temp.max
         * ***************         */
      }
    } else if (cur.min == temp.max + 1) {
      /* Current range is adjacent to temp.min/temp.max range */
      /* temp.min      temp.max
       * ***************
       *                cur.min    cur.max
       *                ************       */
      /* Concatenate range */
      temp.max = cur.max;
      /* Result: */
      /* temp.min                  temp.max
       * ***************************         */
    } else { /* cur.min > temp.max */
      /* Current range is outside of temp.min/temp.max range */
      /* temp.min      temp.max
       * ***************
       *                      cur.min    cur.max
       *                      ************       */
      /* Create a new range, and push the temp.min/temp.max range */
      re__charclass_inverter_push(&inverter, temp);
      if (re__charclass_inverter_hasnext(&inverter)) {
        re__rune_range_vec_set(
            &builder->ranges, write_ptr++,
            re__charclass_inverter_next(&inverter));
      }
      temp.min = cur.min;
      temp.max = cur.max;
      /* Result */
      /* old_temp.min  old_temp.max
       * ***************
       *                      temp.min   temp.max
       *                      ************         */
    }
  }
  if (temp.min != -1) {
    re__charclass_inverter_push(&inverter, temp);
    if (re__charclass_inverter_hasnext(&inverter)) {
      re__rune_range_vec_set(
          &builder->ranges, write_ptr++,
          re__charclass_inverter_next(&inverter));
    }
  }
  re__charclass_inverter_end(&inverter);
  if (re__charclass_inverter_hasnext(&inverter)) {
    re__rune_range_vec_set(
        &builder->ranges, write_ptr++, re__charclass_inverter_next(&inverter));
  }
  if (write_ptr) {
    charclass->ranges =
        (re__rune_range*)MN_MALLOC(sizeof(re__rune_range) * write_ptr);
    if (charclass->ranges == MN_NULL) {
      return RE_ERROR_NOMEM;
    }
    charclass->ranges_size = write_ptr;
    for (read_ptr = 0; read_ptr < write_ptr; read_ptr++) {
      charclass->ranges[read_ptr] =
          re__rune_range_vec_get_data(&builder->ranges)[read_ptr];
    }
  } else {
    /* don't bother calling malloc(0) */
    charclass->ranges = MN_NULL;
    charclass->ranges_size = 0;
  }
  return err;
}

int re__charclass_equals(
    const re__charclass* charclass, const re__charclass* other)
{
  mn_size cs = re__charclass_size(charclass);
  mn_size os = re__charclass_size(other);
  mn_size i;
  if (cs != os) {
    return 0;
  }
  for (i = 0; i < cs; i++) {
    re__rune_range cr = re__charclass_get_ranges(charclass)[i];
    re__rune_range or = re__charclass_get_ranges(other)[i];
    if (!re__rune_range_equals(cr, or)) {
      return 0;
    }
  }
  return 1;
}

#if MN_DEBUG

#include <stdio.h>

void re__charclass_dump(const re__charclass* charclass, mn_size lvl)
{
  mn_size i;
  mn_size sz = charclass->ranges_size;
  printf("Charclass %p\n", (void*)charclass);
  for (i = 0; i < sz; i++) {
    re__rune_range cur = charclass->ranges[i];
    mn_size j;
    for (j = 0; j < lvl; j++) {
      printf("  ");
    }
    printf("%X - %X\n", (re_rune)cur.min, (re_rune)cur.max);
  }
}

MN_INTERNAL int re__charclass_verify(const re__charclass* charclass)
{
  mn_size i;
  re__rune_range last;
  last.min = -1;
  last.max = -1;
  for (i = 0; i < re__charclass_size(charclass); i++) {
    re__rune_range rr = re__charclass_get_ranges(charclass)[i];
    if (rr.min <= last.max) {
      return 0;
    }
    last = rr;
  }
  return 1;
}

void re__charclass_builder_dump(const re__charclass_builder* builder)
{
  mn_size i;
  printf("Charclass Builder %p:\n", (void*)builder);
  printf("  Highest Rune: %i\n", builder->highest);
  printf("  Should Invert: %i\n", builder->should_invert);
  for (i = 0; i < re__rune_range_vec_size(&builder->ranges); i++) {
    re__rune_range r = re__rune_range_vec_get(&builder->ranges, i);
    printf("  [%u]: ", (unsigned int)i);
    re__rune_range_debug_dump(r);
    printf("\n");
  }
}

#endif

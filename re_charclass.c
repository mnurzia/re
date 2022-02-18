#include "re_internal.h"

RE_VEC_IMPL_FUNC(re__rune_range, init)
RE_VEC_IMPL_FUNC(re__rune_range, destroy)
RE_VEC_IMPL_FUNC(re__rune_range, push)
RE_VEC_IMPL_FUNC(re__rune_range, get)
RE_VEC_IMPL_FUNC(re__rune_range, insert)
RE_VEC_IMPL_FUNC(re__rune_range, size)
RE_VEC_IMPL_FUNC(re__rune_range, clear)
RE_VEC_IMPL_FUNC(re__rune_range, get_data)

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
    {6, "xdigit", 3, (unsigned char*)"\x30\x39\x41\x46\x61\x66"}
};

#define RE__CHARCLASS_ASCII_DEFAULTS_SIZE (sizeof(re__charclass_ascii_defaults) / sizeof(re__charclass_ascii))

RE_INTERNAL void re__charclass_init(re__charclass* charclass) {
    re__rune_range_vec_init(&charclass->ranges);
}

RE_INTERNAL void re__charclass_destroy(re__charclass* charclass) {
    re__rune_range_vec_destroy(&charclass->ranges);
}

RE_INTERNAL re_error re__charclass_push(re__charclass* charclass, re__rune_range range) {
    re__rune_range temp;
    RE_ASSERT(range.min <= range.max);
    re__zero_mem(sizeof(temp), (void*)&temp);
    temp.min = range.min;
    temp.max = range.max;
    return re__rune_range_vec_push(&charclass->ranges, range);
}

RE_INTERNAL const re__rune_range* re__charclass_get_ranges(const re__charclass* charclass) {
    const re__rune_range* out = re__rune_range_vec_get_data(&charclass->ranges);
    RE_ASSERT(out != NULL);
    return out;
}

RE_INTERNAL re_size re__charclass_get_num_ranges(const re__charclass* charclass) {
    return re__rune_range_vec_size(&charclass->ranges);
}

RE_INTERNAL const re__charclass_ascii* re__charclass_ascii_find(re__str* name) {
    re_size i;
    const re__charclass_ascii* found = RE_NULL;
    /* Search table for the matching named character class */
    for (i = 0; i < RE__CHARCLASS_ASCII_DEFAULTS_SIZE; i++) {
        const re__charclass_ascii* cur = &re__charclass_ascii_defaults[i];
        re__str_view temp_view, name_view;
        re__str_view_init_n(&temp_view, (re_char*)cur->name, (re_size)cur->name_size);
        re__str_view_init(&name_view, name);
        if (re__str_view_cmp(&temp_view, &name_view) == 0) {
            found = cur;
            break;
        }
    }
    return found;
}

RE_INTERNAL re_error re__charclass_init_from_ascii(re__charclass* charclass, const re__charclass_ascii* ascii_cc, int inverted) {
    re_error err = RE_ERROR_NONE;
    re_size i;
    re__rune_range temp;
    re_rune last_max = -1;
    /* Dump found ranges into a char class */
    re__charclass_init(charclass);
    for (i = 0; i < ascii_cc->num_classes; i++) {
        if (inverted) {
            if (last_max == -1) {
                temp.min = 0;
                temp.max = ascii_cc->classes[i*2] - 1;
            } else {
                temp.min = last_max;
                temp.max = ascii_cc->classes[i*2] - 1;
            }
            last_max = ascii_cc->classes[i*2 + 1] + 1;
        } else {
            temp.min = ascii_cc->classes[i*2];
            temp.max = ascii_cc->classes[i*2 + 1];
        }
        if (temp.max >= temp.min) {
            if ((err = re__charclass_push(charclass, temp))) {
                re__charclass_destroy(charclass);
                return err;
            }
        }
    }
    if (inverted) {
        if (last_max == -1) {
            temp.min = 0;
            temp.max = RE_RUNE_MAX;
        } else {
            temp.min = last_max;
            temp.max = RE_RUNE_MAX;
        }
        if (temp.min != temp.max) {
            if ((err = re__charclass_push(charclass, temp))) {
                re__charclass_destroy(charclass);
                return err;
            }
        }
    }
    return err;
}

RE_INTERNAL re_error re__charclass_init_from_class(re__charclass* charclass, re__charclass_ascii_type type, int inverted) {
    return re__charclass_init_from_ascii(charclass, &re__charclass_ascii_defaults[type], inverted);
}

/* Returns RE_ERROR_INVALID if not found */
RE_INTERNAL re_error re__charclass_init_from_string(re__charclass* charclass, re__str* name, int inverted) {
    const re__charclass_ascii* found = re__charclass_ascii_find(name);
    /* Found is NULL if we didn't find anything during the loop */
    if (found == RE_NULL) {
        return RE_ERROR_INVALID;
    }
    return re__charclass_init_from_ascii(charclass, found, inverted);
}

RE_INTERNAL void re__charclass_builder_init(re__charclass_builder* builder) {
    re__rune_range_vec_init(&builder->ranges);
    builder->should_invert = 0;
    builder->highest = -1;
}

RE_INTERNAL void re__charclass_builder_begin(re__charclass_builder* builder) {
    re__rune_range_vec_clear(&builder->ranges);
    builder->should_invert = 0;
    builder->highest = -1;
}

RE_INTERNAL void re__charclass_builder_destroy(re__charclass_builder* builder) {
    re__rune_range_vec_destroy(&builder->ranges);
}

RE_INTERNAL void re__charclass_builder_invert(re__charclass_builder* builder) {
    builder->should_invert = 1;
}

/* Insert a range into the builder. */
/* Inserts in sorted order, that is, all ranges are ordered by their low bound. */
RE_INTERNAL re_error re__charclass_builder_insert_range(re__charclass_builder* builder, re__rune_range range) {
    re_error err = RE_ERROR_NONE;
    re_size i;
    re_size max = re__rune_range_vec_size(&builder->ranges);
    /* The range should be normalized. */
    RE_ASSERT(range.min <= range.max);
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

/* Insert a character class into a builder. */
/* Used when putting a named character class inside of an unnamed one. */
RE_INTERNAL re_error re__charclass_builder_insert_class(re__charclass_builder* builder, re__charclass* charclass) {
    re_size i;
    re_size sz = re__rune_range_vec_size(&charclass->ranges);
    re_error err = RE_ERROR_NONE;
    for (i = 0; i < sz; i++) {
        /* Iterate through ranges, adding them all */
        /* Perhaps could be optimized more */
        re__rune_range cur_range = re__rune_range_vec_get(&charclass->ranges, i);
        if ((err = re__charclass_builder_insert_range(builder, cur_range))) {
            return err;
        }
    }
    return err;
}

RE_INTERNAL re_error re__charclass_builder_finish(re__charclass_builder* builder, re__charclass* charclass) {
    re_error err = RE_ERROR_NONE;
    /* Temporary range */
    re_rune temp_min = -1;
    re_rune temp_max = -1;
    re_rune last_temp_max = -1;
    re_size i;
    re_size ranges_size = re__rune_range_vec_size(&builder->ranges);
    re__rune_range new_range;
    re__charclass_init(charclass);
    for (i = 0; i < ranges_size; i++) {
        re__rune_range cur = re__rune_range_vec_get(&builder->ranges, i);
        RE_ASSERT(cur.min <= cur.max);
        if (temp_min == -1) {
            /* First range */
            temp_min = cur.min;
            temp_max = cur.max;
            /* Result */
            /* temp_min      temp_max
             * ***************         */
        } else if (cur.min <= temp_max) {
            if (cur.max > temp_max) {
                /* Current range intersects with temp_min/temp_max range */
                /* temp_min      temp_max
                 * ***************   
                 *             cur.min       cur.max
                 *             ***************           */
                /* Extend range */
                temp_max = cur.max;
                /* Result: */
                /* temp_min                  temp_max
                 * ***************************         */
            } else {
                /* Current range is contained within temp_min/temp_max range */
                /* temp_min      temp_max
                 * ***************   
                 *  cur.min cur.max
                 *  *********         */
                /* Nothing needs to be done */
                /* Result: */
                /* temp_min      temp_max
                 * ***************         */
            }
        } else if (cur.min == temp_max + 1) {
            /* Current range is adjacent to temp_min/temp_max range */
            /* temp_min      temp_max
             * ***************   
             *                cur.min    cur.max
             *                ************       */
            /* Concatenate range */
            temp_max = cur.max;
            /* Result: */
            /* temp_min                  temp_max
             * ***************************         */
        } else if (cur.min > temp_max) {
            /* Current range is outside of temp_min/temp_max range */
            /* temp_min      temp_max
             * ***************   
             *                      cur.min    cur.max
             *                      ************       */
            /* Create a new range, and push the temp_min/temp_max range */
            if (!builder->should_invert) {
                new_range.min = temp_min;
                new_range.max = temp_max;
                if ((err = re__charclass_push(charclass, new_range))) {
                    goto destroy_out;
                }
            } else {
                /* For inverted classes, we need to keep track of the previous
                 * temp_maximum bound so we can "fill in the gaps" */
                if (last_temp_max == -1) {
                    new_range.min = 0;
                    new_range.max = temp_min - 1;
                } else {
                    new_range.min = last_temp_max + 1;
                    new_range.max = temp_min - 1;
                }
                RE_ASSERT(new_range.min <= new_range.max + 1);
                if (new_range.min <= new_range.max) {
                    if ((err = re__charclass_push(charclass, new_range))) {
                        goto destroy_out;
                    }
                    last_temp_max = temp_max;
                }
            }
            temp_min = cur.min;
            temp_max = cur.max;
            /* Result */
            /* old_temp_min  old_temp_max
             * ***************   
             *                      temp_min   temp_max
             *                      ************         */
        }
    }
    /* Finished: add the temporary range */
    if (!builder->should_invert) {
        if (temp_min == -1) {
            /* empty class */
            /* no ranges to add */
        } else {
            new_range.min = temp_min;
            new_range.max = temp_max;
            if ((err = re__charclass_push(charclass, new_range))) {
                goto destroy_out;
            }
        }
    } else {
        /* If we are inverted, add the range *up to* the temporary range */
        if (last_temp_max == -1) {
            /* Initial inversion */
            new_range.min = 0;
            new_range.max = temp_min - 1;
        } else {
            /* Intermediate inversions */
            new_range.min = last_temp_max + 1;
            new_range.max = temp_min - 1;
        }
        RE_ASSERT(new_range.min <= new_range.max + 1);
        if (new_range.min <= new_range.max) {
            /* If the lo/hi are equal, then don't bother */
            if ((err = re__charclass_push(charclass, new_range))) {
                goto destroy_out;
            }
        }
        /* Add the final inverted range, the one that goes from the maximum non-
         * inverted range to infinity (RE_RUNE_MAX) */
        if (new_range.max < RE_RUNE_MAX) {
            if (temp_max == -1) {
                /* empty class */
                new_range.min = 0;
            } else {
                new_range.min = temp_max + 1;
            }
            new_range.max = RE_RUNE_MAX;
            if ((err = re__charclass_push(charclass, new_range))) {
                goto destroy_out;
            }
        }
    }
    return err;
destroy_out:
    re__charclass_destroy(charclass);
    return err;
}

int re__charclass_equals(const re__charclass* charclass, const re__charclass* other) {
    re_size cs = re__charclass_get_num_ranges(charclass);
    re_size os = re__charclass_get_num_ranges(other);
    re_size i;
    if (cs != os) {
        return 0;
    }
    for (i = 0; i < cs; i++) {
        re__rune_range cr = re__charclass_get_ranges(charclass)[i];
        re__rune_range or = re__charclass_get_ranges(charclass)[i];
        if (!re__rune_range_equals(cr, or)) {
            return 0;
        }
    }
    return 1;
}

#if RE_DEBUG

void re__charclass_dump(const re__charclass* charclass, re_size lvl) {
    re_size i;
    re_size sz = re__rune_range_vec_size(&charclass->ranges);
    for (i = 0; i < sz; i++) {
        re__rune_range cur = re__rune_range_vec_get(&charclass->ranges, i);
        re_size j;
        for (j = 0; j < lvl; j++) {
            printf("  ");
        }
        printf("%X - %X\n", (re_rune)cur.min, (re_rune)cur.max);
    }
}

RE_INTERNAL int re__charclass_verify(const re__charclass* charclass) {
    re_size i;
    re__rune_range last;
    last.min = -1;
    last.max = -1;
    for (i = 0; i < re__charclass_get_num_ranges(charclass); i++) {
        re__rune_range rr = re__charclass_get_ranges(charclass)[i];
        if (rr.min <= last.max) {
            return 0;
        }
        last = rr;
    }
    return 1;
}

#endif

#include "re_internal.h"

int re__byte_range_equals(re__byte_range range, re__byte_range other) {
    return range.min == other.min && range.max == other.max;
}

int re__byte_range_adjacent(re__byte_range range, re__byte_range other) {
    return ((re_uint32)other.min) == ((re_uint32)range.max) + 1;
}

re__byte_range re__byte_range_merge(re__byte_range range, re__byte_range other) {
    re__byte_range out;
    RE_ASSERT(re__byte_range_adjacent(range, other));
    out.min = range.min;
    out.max = other.max;
    return out;
}

int re__rune_range_equals(re__rune_range range, re__rune_range other) {
    return range.min == other.min && range.max == other.max;
}

int re__rune_range_intersects(re__rune_range range, re__rune_range clip) {
    return range.min <= clip.max && clip.min <= range.max;
}

re__rune_range re__rune_range_clamp(re__rune_range range, re__rune_range bounds) {
    re__rune_range out = range;
    RE_ASSERT(re__rune_range_intersects(range, bounds));
    if (range.min < bounds.min) {
        out.min = bounds.min;
    }
    if (range.max > bounds.max) {
        out.max = bounds.max;
    }
    return out;
}

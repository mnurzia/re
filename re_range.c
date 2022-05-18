#include "re_internal.h"

int re__byte_range_equals(re__byte_range range, re__byte_range other) {
    return range.min == other.min && range.max == other.max;
}

int re__byte_range_adjacent(re__byte_range range, re__byte_range other) {
    return ((mn_uint32)other.min) == ((mn_uint32)range.max) + 1;
}

int re__byte_range_intersects(re__byte_range range, re__byte_range clip) {
    return range.min <= clip.max && clip.min <= range.max;
}

re__byte_range re__byte_range_intersection(re__byte_range range, re__byte_range clip) {
    re__byte_range out;
    MN_ASSERT(re__byte_range_intersects(range, clip));
    out.min = MN__MAX(range.min, clip.min);
    out.max = MN__MIN(range.max, clip.max);
    return out;
}

re__byte_range re__byte_range_merge(re__byte_range range, re__byte_range other) {
    re__byte_range out;
    MN_ASSERT(re__byte_range_adjacent(range, other));
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
    MN_ASSERT(re__rune_range_intersects(range, bounds));
    if (range.min < bounds.min) {
        out.min = bounds.min;
    }
    if (range.max > bounds.max) {
        out.max = bounds.max;
    }
    return out;
}

#if MN_DEBUG

#include <stdio.h>

void re__byte_debug_dump(mn_uint8 byte) {
    if ((byte >= ' ') && (byte < 0x7F)) {
        printf("0x%02X ('%c')", byte, byte);
    } else {
        printf("0x%02X", byte);
    }
}

void re__rune_debug_dump(re_rune rune) {
    mn_uint8 utf_buf[5];
    int nchar;
    if ((rune >= ' ') && (rune < 0x7F)) {
        printf("0x%02X ('%c')", rune, rune);
    } else {
        printf("0x%02X", rune);
    }
    nchar = re__compile_gen_utf8(rune, utf_buf);
    if (nchar != 0) {
        int i;
        printf(" {");
        for (i = 0; i < nchar; i++) {
            printf("%02X", utf_buf[i]);
        }
        printf("}");
    }
}

void re__byte_range_debug_dump(re__byte_range br) {
    printf("[");
    re__byte_debug_dump(br.min);
    printf(" - ");
    re__byte_debug_dump(br.max);
    printf("]");
}


void re__rune_range_debug_dump(re__rune_range rr) {
    printf("[");
    re__rune_debug_dump(rr.min);
    printf(" - ");
    re__rune_debug_dump(rr.max);
    printf("]");
}

#endif

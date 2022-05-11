#include "re_internal.h"

/* python brute_force_utf8_compiler.py fwd 0-10FFFF */
MN_INTERNAL_DATA mn_uint8 re__prog_data_dot_fwd_accsurr[] = {
    /* 0001 */ 0x02, 0x02, 0x03,       /* SPLIT           -> 0x02, 0x03 */
    /* 0002 */ 0x01, 0x00, 0x7F, 0x00, /* RANGE 0x00-0x7F -> out */
    /* 0003 */ 0x02, 0x04, 0x06,       /* SPLIT           -> 0x04, 0x06 */
    /* 0004 */ 0x01, 0xC2, 0xDF, 0x05, /* RANGE 0xC2-0xDF -> 0x05 */
    /* 0005 */ 0x01, 0x80, 0xBF, 0x00, /* RANGE 0x80-0xBF -> out */
    /* 0006 */ 0x02, 0x07, 0x09,       /* SPLIT           -> 0x07, 0x09 */
    /* 0007 */ 0x00, 0xE0, 0x08,       /* BYTE  0xE0      -> 0x08 */
    /* 0008 */ 0x01, 0xA0, 0xBF, 0x05, /* RANGE 0xA0-0xBF -> 0x05 */
    /* 0009 */ 0x02, 0x0A, 0x0C,       /* SPLIT           -> 0x0A, 0x0C */
    /* 000A */ 0x01, 0xE1, 0xEF, 0x0B, /* RANGE 0xE1-0xEF -> 0x0B */
    /* 000B */ 0x01, 0x80, 0xBF, 0x05, /* RANGE 0x80-0xBF -> 0x05 */
    /* 000C */ 0x02, 0x0D, 0x0F,       /* SPLIT           -> 0x0D, 0x0F */
    /* 000D */ 0x00, 0xF0, 0x0E,       /* BYTE  0xF0      -> 0x0E */
    /* 000E */ 0x01, 0x90, 0xBF, 0x0B, /* RANGE 0x90-0xBF -> 0x0B */
    /* 000F */ 0x02, 0x10, 0x12,       /* SPLIT           -> 0x10, 0x12 */
    /* 0010 */ 0x01, 0xF1, 0xF3, 0x11, /* RANGE 0xF1-0xF3 -> 0x11 */
    /* 0011 */ 0x01, 0x80, 0xBF, 0x0B, /* RANGE 0x80-0xBF -> 0x0B */
    /* 0012 */ 0x00, 0xF4, 0x13,       /* BYTE  0xF4      -> 0x13 */
    /* 0013 */ 0x01, 0x80, 0x8F, 0x0B, /* RANGE 0x80-0x8F -> 0x0B */
};

/* python brute_force_utf8_compiler.py rev 0-10FFFF */
MN_INTERNAL_DATA mn_uint8 re__prog_data_dot_rev_accsurr[] = {
    /* 0001 */ 0x02, 0x02, 0x03,       /* SPLIT           -> 0x02, 0x03 */
    /* 0002 */ 0x01, 0x00, 0x7F, 0x00, /* RANGE 0x00-0x7F -> out */
    /* 0003 */ 0x01, 0x80, 0xBF, 0x04, /* RANGE 0x80-0xBF -> 0x04 */
    /* 0004 */ 0x02, 0x05, 0x0D,       /* SPLIT           -> 0x05, 0x0D */
    /* 0005 */ 0x01, 0x80, 0x9F, 0x06, /* RANGE 0x80-0x9F -> 0x06 */
    /* 0006 */ 0x02, 0x07, 0x09,       /* SPLIT           -> 0x07, 0x09 */
    /* 0007 */ 0x01, 0x80, 0x8F, 0x08, /* RANGE 0x80-0x8F -> 0x08 */
    /* 0008 */ 0x01, 0xF1, 0xF4, 0x00, /* RANGE 0xF1-0xF4 -> out */
    /* 0009 */ 0x02, 0x0A, 0x0C,       /* SPLIT           -> 0x0A, 0x0C */
    /* 000A */ 0x01, 0x90, 0xBF, 0x0B, /* RANGE 0x90-0xBF -> 0x0B */
    /* 000B */ 0x01, 0xF0, 0xF3, 0x00, /* RANGE 0xF0-0xF3 -> out */
    /* 000C */ 0x01, 0xE1, 0xEF, 0x00, /* RANGE 0xE1-0xEF -> out */
    /* 000D */ 0x02, 0x0E, 0x14,       /* SPLIT           -> 0x0E, 0x14 */
    /* 000E */ 0x01, 0xA0, 0xBF, 0x0F, /* RANGE 0xA0-0xBF -> 0x0F */
    /* 000F */ 0x02, 0x10, 0x11,       /* SPLIT           -> 0x10, 0x11 */
    /* 0010 */ 0x01, 0x80, 0x8F, 0x08, /* RANGE 0x80-0x8F -> 0x08 */
    /* 0011 */ 0x02, 0x12, 0x13,       /* SPLIT           -> 0x12, 0x13 */
    /* 0012 */ 0x01, 0x90, 0xBF, 0x0B, /* RANGE 0x90-0xBF -> 0x0B */
    /* 0013 */ 0x01, 0xE0, 0xEF, 0x00, /* RANGE 0xE0-0xEF -> out */
    /* 0014 */ 0x01, 0xC2, 0xDF, 0x00, /* RANGE 0xC2-0xDF -> out */
};

/* python brute_force_utf8_compiler.py fwd 0-D7FF C000-10FFFF */
MN_INTERNAL_DATA mn_uint8 re__prog_data_dot_fwd_rejsurr[] = {
    /* 0001 */ 0x02, 0x02, 0x03,       /* SPLIT           -> 0x02, 0x03 */
    /* 0002 */ 0x01, 0x00, 0x7F, 0x00, /* RANGE 0x00-0x7F -> out */
    /* 0003 */ 0x02, 0x04, 0x06,       /* SPLIT           -> 0x04, 0x06 */
    /* 0004 */ 0x01, 0xC2, 0xDF, 0x05, /* RANGE 0xC2-0xDF -> 0x05 */
    /* 0005 */ 0x01, 0x80, 0xBF, 0x00, /* RANGE 0x80-0xBF -> out */
    /* 0006 */ 0x02, 0x07, 0x09,       /* SPLIT           -> 0x07, 0x09 */
    /* 0007 */ 0x00, 0xE0, 0x08,       /* BYTE  0xE0      -> 0x08 */
    /* 0008 */ 0x01, 0xA0, 0xBF, 0x05, /* RANGE 0xA0-0xBF -> 0x05 */
    /* 0009 */ 0x02, 0x0A, 0x0C,       /* SPLIT           -> 0x0A, 0x0C */
    /* 000A */ 0x01, 0xE1, 0xEF, 0x0B, /* RANGE 0xE1-0xEF -> 0x0B */
    /* 000B */ 0x01, 0x80, 0xBF, 0x05, /* RANGE 0x80-0xBF -> 0x05 */
    /* 000C */ 0x02, 0x0D, 0x0F,       /* SPLIT           -> 0x0D, 0x0F */
    /* 000D */ 0x00, 0xF0, 0x0E,       /* BYTE  0xF0      -> 0x0E */
    /* 000E */ 0x01, 0x90, 0xBF, 0x0B, /* RANGE 0x90-0xBF -> 0x0B */
    /* 000F */ 0x02, 0x10, 0x12,       /* SPLIT           -> 0x10, 0x12 */
    /* 0010 */ 0x01, 0xF1, 0xF3, 0x11, /* RANGE 0xF1-0xF3 -> 0x11 */
    /* 0011 */ 0x01, 0x80, 0xBF, 0x0B, /* RANGE 0x80-0xBF -> 0x0B */
    /* 0012 */ 0x00, 0xF4, 0x13,       /* BYTE  0xF4      -> 0x13 */
    /* 0013 */ 0x01, 0x80, 0x8F, 0x0B, /* RANGE 0x80-0x8F -> 0x0B */
};

/* python brute_force_utf8_compiler.py rev 0-D7FF C000-10FFFF */
MN_INTERNAL_DATA mn_uint8 re__prog_data_dot_rev_rejsurr[] = {
    /* 0001 */ 0x02, 0x02, 0x03,       /* SPLIT           -> 0x02, 0x03 */
    /* 0002 */ 0x01, 0x00, 0x7F, 0x00, /* RANGE 0x00-0x7F -> out */
    /* 0003 */ 0x01, 0x80, 0xBF, 0x04, /* RANGE 0x80-0xBF -> 0x04 */
    /* 0004 */ 0x02, 0x05, 0x0D,       /* SPLIT           -> 0x05, 0x0D */
    /* 0005 */ 0x01, 0x80, 0x9F, 0x06, /* RANGE 0x80-0x9F -> 0x06 */
    /* 0006 */ 0x02, 0x07, 0x09,       /* SPLIT           -> 0x07, 0x09 */
    /* 0007 */ 0x01, 0x80, 0x8F, 0x08, /* RANGE 0x80-0x8F -> 0x08 */
    /* 0008 */ 0x01, 0xF1, 0xF4, 0x00, /* RANGE 0xF1-0xF4 -> out */
    /* 0009 */ 0x02, 0x0A, 0x0C,       /* SPLIT           -> 0x0A, 0x0C */
    /* 000A */ 0x01, 0x90, 0xBF, 0x0B, /* RANGE 0x90-0xBF -> 0x0B */
    /* 000B */ 0x01, 0xF0, 0xF3, 0x00, /* RANGE 0xF0-0xF3 -> out */
    /* 000C */ 0x01, 0xE1, 0xEF, 0x00, /* RANGE 0xE1-0xEF -> out */
    /* 000D */ 0x02, 0x0E, 0x14,       /* SPLIT           -> 0x0E, 0x14 */
    /* 000E */ 0x01, 0xA0, 0xBF, 0x0F, /* RANGE 0xA0-0xBF -> 0x0F */
    /* 000F */ 0x02, 0x10, 0x11,       /* SPLIT           -> 0x10, 0x11 */
    /* 0010 */ 0x01, 0x80, 0x8F, 0x08, /* RANGE 0x80-0x8F -> 0x08 */
    /* 0011 */ 0x02, 0x12, 0x13,       /* SPLIT           -> 0x12, 0x13 */
    /* 0012 */ 0x01, 0x90, 0xBF, 0x0B, /* RANGE 0x90-0xBF -> 0x0B */
    /* 0013 */ 0x01, 0xE0, 0xEF, 0x00, /* RANGE 0xE0-0xEF -> out */
    /* 0014 */ 0x01, 0xC2, 0xDF, 0x00, /* RANGE 0xC2-0xDF -> out */
};

mn_uint8* re__prog_data[RE__PROG_DATA_ID_MAX] = {
    re__prog_data_dot_fwd_accsurr,
    re__prog_data_dot_rev_accsurr,
    re__prog_data_dot_fwd_rejsurr,
    re__prog_data_dot_rev_rejsurr
};

mn_size re__prog_data_size[RE__PROG_DATA_ID_MAX] = {
    sizeof(re__prog_data_dot_fwd_accsurr),
    sizeof(re__prog_data_dot_rev_accsurr),
    sizeof(re__prog_data_dot_fwd_rejsurr),
    sizeof(re__prog_data_dot_rev_rejsurr)
};

MN_INTERNAL re_error re__prog_data_decompress_read_loc(mn_uint8* compressed, mn_size compressed_size, mn_size* ptr, re__prog_loc* out_loc) {
    re__prog_loc loc = 0;
    mn_uint8 byte;
    int len = 0;
    if (compressed_size == *ptr) {
        return RE__ERROR_COMPRESSION_FORMAT;
    }
    byte = compressed[*ptr];
    (*ptr)++;
    loc = byte & 0x7F;
    len++;
    while ((byte & 0x80) == 0x80) {
        if (compressed_size == *ptr) {
            return RE__ERROR_COMPRESSION_FORMAT;
        }
        byte = compressed[*ptr];
        (*ptr)++;
        loc <<= 7;
        loc |= byte & 0x7F;
        len++;
        if (len == 3) {
            return RE__ERROR_COMPRESSION_FORMAT;
        }
    }
    *out_loc = loc;
    return RE_ERROR_NONE;
}

MN_INTERNAL re_error re__prog_data_decompress(re__prog* prog, mn_uint8* compressed, mn_size compressed_size, re__compile_patches* patches) {
    mn_size ptr = 0;
    re_error err;
    re__prog_inst inst;
    re__prog_loc offset = re__prog_size(prog) - 1;
    while (1) {
        mn_uint8 inst_type;
        if (ptr == compressed_size) {
            break;
        }
        inst_type = compressed[ptr++];
        if (inst_type == 0) { /* BYTE */
            mn_uint8 byte_val;
            re__prog_loc primary;
            if (ptr == compressed_size) {
                return RE__ERROR_COMPRESSION_FORMAT;
            }
            byte_val = compressed[ptr++];
            if ((err = re__prog_data_decompress_read_loc(compressed, compressed_size, &ptr, &primary))) {
                return err;
            }
            re__prog_inst_init_byte(&inst, byte_val);
            if (primary == 0) {
                re__compile_patches_append(patches, prog, re__prog_size(prog), 0);
            } else {
                re__prog_inst_set_primary(&inst, primary + offset);
            }
        } else if (inst_type == 1) { /* RANGE */
            re__byte_range range;
            re__prog_loc primary;
            if (ptr == compressed_size) {
                return RE__ERROR_COMPRESSION_FORMAT;
            }
            range.min = compressed[ptr++];
            if (ptr == compressed_size) {
                return RE__ERROR_COMPRESSION_FORMAT;
            }
            range.max = compressed[ptr++];
            if ((err = re__prog_data_decompress_read_loc(compressed, compressed_size, &ptr, &primary))) {
                return err;
            }
            re__prog_inst_init_byte_range(&inst, range);
            if (primary == 0) {
                re__compile_patches_append(patches, prog, re__prog_size(prog), 0);
            } else {
                re__prog_inst_set_primary(&inst, primary + offset);
            }
        } else if (inst_type == 2) { /* SPLIT */
            re__prog_loc primary;
            re__prog_loc secondary;
            if ((err = re__prog_data_decompress_read_loc(compressed, compressed_size, &ptr, &primary))) {
                return err;
            }
            if ((err = re__prog_data_decompress_read_loc(compressed, compressed_size, &ptr, &secondary))) {
                return err;
            }
            re__prog_inst_init_split(&inst, 0, 0);
            if (primary == 0) {
                re__compile_patches_append(patches, prog, re__prog_size(prog), 0);
            } else {
                re__prog_inst_set_primary(&inst, primary + offset);
            }
            if (secondary == 0) {
                re__compile_patches_append(patches, prog, re__prog_size(prog), 1);
            } else {
                re__prog_inst_set_split_secondary(&inst, secondary + offset);
            }
        } else {
            return RE__ERROR_COMPRESSION_FORMAT;
        }
        if ((err = re__prog_add(prog, inst))) {
            return err;
        }
    }
    return RE_ERROR_NONE;
}

#include "re_internal.h"

MN_INTERNAL void re__prog_inst_init(re__prog_inst* inst, re__prog_inst_type inst_type) {
    inst->_inst_type = inst_type;
    inst->_primary = RE__PROG_LOC_INVALID;
}

MN_INTERNAL void re__prog_inst_init_byte(re__prog_inst* inst, mn_uint8 byte) {
    re__prog_inst_init(inst, RE__PROG_INST_TYPE_BYTE);
    inst->_inst_data._byte = byte;
}

MN_INTERNAL void re__prog_inst_init_byte_range(re__prog_inst* inst, re__byte_range br) {
    re__prog_inst_init(inst, RE__PROG_INST_TYPE_BYTE_RANGE);
    inst->_inst_data._range.min = br.min;
    inst->_inst_data._range.max = br.max;
}

MN_INTERNAL void re__prog_inst_init_split(re__prog_inst* inst, re__prog_loc primary, re__prog_loc secondary) {
    re__prog_inst_init(inst, RE__PROG_INST_TYPE_SPLIT);
    inst->_primary = primary;
    inst->_inst_data._secondary = secondary;
}

MN_INTERNAL void re__prog_inst_init_match(re__prog_inst* inst, mn_uint32 match_idx) {
    re__prog_inst_init(inst, RE__PROG_INST_TYPE_MATCH);
    inst->_inst_data._match_idx = match_idx;
}

MN_INTERNAL void re__prog_inst_init_fail(re__prog_inst* inst) {
    re__prog_inst_init(inst, RE__PROG_INST_TYPE_FAIL);
}

MN_INTERNAL void re__prog_inst_init_assert(re__prog_inst* inst, mn_uint32 assert_context) {
    re__prog_inst_init(inst, RE__PROG_INST_TYPE_ASSERT);
    inst->_inst_data._assert_context = assert_context;
}

MN_INTERNAL void re__prog_inst_init_save(re__prog_inst* inst, mn_uint32 save_idx) {
    re__prog_inst_init(inst, RE__PROG_INST_TYPE_SAVE);
    inst->_inst_data._save_idx = save_idx;
}

MN_INTERNAL re__prog_loc re__prog_inst_get_primary(const re__prog_inst* inst) {
    return inst->_primary;
}

MN_INTERNAL void re__prog_inst_set_primary(re__prog_inst* inst, re__prog_loc loc) {
    inst->_primary = loc;
}

MN_INTERNAL mn_uint8 re__prog_inst_get_byte(const re__prog_inst* inst) {
    MN_ASSERT(inst->_inst_type == RE__PROG_INST_TYPE_BYTE);
    return inst->_inst_data._byte;
}

MN_INTERNAL mn_uint8 re__prog_inst_get_byte_min(const re__prog_inst* inst) {
    MN_ASSERT(inst->_inst_type == RE__PROG_INST_TYPE_BYTE_RANGE);
    return inst->_inst_data._range.min;
}

MN_INTERNAL mn_uint8 re__prog_inst_get_byte_max(const re__prog_inst* inst) {
    MN_ASSERT(inst->_inst_type == RE__PROG_INST_TYPE_BYTE_RANGE);
    return inst->_inst_data._range.max;
}

MN_INTERNAL re__prog_loc re__prog_inst_get_split_secondary(const re__prog_inst* inst) {
    MN_ASSERT(inst->_inst_type == RE__PROG_INST_TYPE_SPLIT);
    return inst->_inst_data._secondary;
}

MN_INTERNAL void re__prog_inst_set_split_secondary(re__prog_inst* inst, re__prog_loc loc) {
    MN_ASSERT(inst->_inst_type == RE__PROG_INST_TYPE_SPLIT);
    inst->_inst_data._secondary = loc;
}

MN_INTERNAL re__prog_inst_type re__prog_inst_get_type(const re__prog_inst* inst) {
    return inst->_inst_type;
}

MN_INTERNAL mn_uint32 re__prog_inst_get_match_idx(const re__prog_inst* inst) {
    MN_ASSERT(inst->_inst_type == RE__PROG_INST_TYPE_MATCH);
    return inst->_inst_data._match_idx;
}

MN_INTERNAL mn_uint32 re__prog_inst_get_save_idx(const re__prog_inst* inst) {
    MN_ASSERT(inst->_inst_type == RE__PROG_INST_TYPE_SAVE);
    return inst->_inst_data._save_idx;
}

MN_INTERNAL mn_uint32 re__prog_inst_get_assert_ctx(const re__prog_inst* inst) {
    MN_ASSERT(inst->_inst_type == RE__PROG_INST_TYPE_ASSERT);
    return inst->_inst_data._assert_context;
}

MN_INTERNAL int re__prog_inst_equals(re__prog_inst* a, re__prog_inst* b) {
    int eq = 1;
    if (re__prog_inst_get_type(a) != re__prog_inst_get_type(b)) {
        return 0;
    }
    if (re__prog_inst_get_type(a) == RE__PROG_INST_TYPE_BYTE) {
        eq &= (re__prog_inst_get_byte(a) == re__prog_inst_get_byte(b));
        eq &= (re__prog_inst_get_primary(a) == re__prog_inst_get_primary(b));
    } else if (re__prog_inst_get_type(a) == RE__PROG_INST_TYPE_BYTE_RANGE) {
        eq &= (re__prog_inst_get_byte_min(a) == re__prog_inst_get_byte_min(b));
        eq &= (re__prog_inst_get_byte_max(a) == re__prog_inst_get_byte_max(b));
        eq &= (re__prog_inst_get_primary(a) == re__prog_inst_get_primary(b));
    } else if (re__prog_inst_get_type(a) == RE__PROG_INST_TYPE_SPLIT) {
        eq &= (re__prog_inst_get_primary(a) == re__prog_inst_get_primary(b));
        eq &= (re__prog_inst_get_split_secondary(a) == re__prog_inst_get_split_secondary(b));  
    } else if (re__prog_inst_get_type(a) == RE__PROG_INST_TYPE_MATCH) {
        eq &= (re__prog_inst_get_match_idx(a) == re__prog_inst_get_match_idx(b));
    } else if (re__prog_inst_get_type(a) == RE__PROG_INST_TYPE_SAVE) {
        eq &= (re__prog_inst_get_save_idx(a) == re__prog_inst_get_save_idx(b));
        eq &= (re__prog_inst_get_primary(a) == re__prog_inst_get_primary(b));
    } else if (re__prog_inst_get_type(a) == RE__PROG_INST_TYPE_ASSERT) {
        eq &= (re__prog_inst_get_assert_ctx(a) == re__prog_inst_get_assert_ctx(b));
        eq &= (re__prog_inst_get_primary(a) == re__prog_inst_get_primary(b));
    }
    return eq;
}

MN__VEC_IMPL_FUNC(re__prog_inst, init)
MN__VEC_IMPL_FUNC(re__prog_inst, destroy)
MN__VEC_IMPL_FUNC(re__prog_inst, push)
MN__VEC_IMPL_FUNC(re__prog_inst, get)
MN__VEC_IMPL_FUNC(re__prog_inst, getref)
MN__VEC_IMPL_FUNC(re__prog_inst, getcref)
MN__VEC_IMPL_FUNC(re__prog_inst, set)
MN__VEC_IMPL_FUNC(re__prog_inst, size)

MN_INTERNAL re_error re__prog_init(re__prog* prog) {
    re__prog_inst_vec_init(&prog->_instructions);
    {
        int i;
        for (i = 0; i < RE__PROG_ENTRY_MAX; i++) {
            prog->_entrypoints[i] = RE__PROG_LOC_INVALID;
        }
    }
    return RE_ERROR_NONE;
}

MN_INTERNAL void re__prog_destroy(re__prog* prog) {
    re__prog_inst_vec_destroy(&prog->_instructions);
}

MN_INTERNAL re__prog_loc re__prog_size(const re__prog* prog) {
    return (re__prog_loc)re__prog_inst_vec_size(&prog->_instructions);
}

MN_INTERNAL re__prog_inst* re__prog_get(re__prog* prog, re__prog_loc loc) {
    return re__prog_inst_vec_getref(&prog->_instructions, loc);
}

MN_INTERNAL const re__prog_inst* re__prog_cget(const re__prog* prog, re__prog_loc loc) {
    return re__prog_inst_vec_getcref(&prog->_instructions, loc);
}

MN_INTERNAL void re__prog_set(re__prog* prog, re__prog_loc loc, re__prog_inst inst) {
    re__prog_inst_vec_set(&prog->_instructions, loc, inst);
}

MN_INTERNAL re_error re__prog_add(re__prog* prog, re__prog_inst inst) {
    if (re__prog_size(prog) == RE__PROG_SIZE_MAX) {
        return RE__ERROR_PROGMAX;
    } else {
        return re__prog_inst_vec_push(&prog->_instructions, inst);
    }
}

MN_INTERNAL int re__prog_equals(re__prog* a, re__prog* b) {
    re__prog_loc i;
    if (re__prog_size(a) != re__prog_size(b)) {
        return 0;
    }
    for (i = 0; i < re__prog_size(a); i++) {
        if (!re__prog_inst_equals(re__prog_get(a, i), re__prog_get(b, i))) {
            return 0;
        }
    }
    return 1;
}

MN_INTERNAL re_error re__prog_decompress_read_loc(mn_uint8* compressed, mn_size compressed_size, mn_size* ptr, re__prog_loc* out_loc) {
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

MN_INTERNAL re_error re__prog_decompress(re__prog* prog, mn_uint8* compressed, mn_size compressed_size, re__compile_patches* patches) {
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
            if ((err = re__prog_decompress_read_loc(compressed, compressed_size, &ptr, &primary))) {
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
            if ((err = re__prog_decompress_read_loc(compressed, compressed_size, &ptr, &primary))) {
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
            if ((err = re__prog_decompress_read_loc(compressed, compressed_size, &ptr, &primary))) {
                return err;
            }
            if ((err = re__prog_decompress_read_loc(compressed, compressed_size, &ptr, &secondary))) {
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

MN_INTERNAL void re__prog_set_entry(re__prog* prog, re__prog_entry idx, re__prog_loc loc) {
    MN_ASSERT(idx < RE__PROG_ENTRY_MAX);
    prog->_entrypoints[idx] = loc;
}

MN_INTERNAL re__prog_loc re__prog_get_entry(const re__prog* prog, re__prog_entry idx) {
    MN_ASSERT(idx < RE__PROG_ENTRY_MAX);
    return prog->_entrypoints[idx];
}

MN_INTERNAL void re__prog_debug_dump(re__prog* prog) {
    re__prog_loc i;
    for (i = 0; i < re__prog_size(prog); i++) {
        re__prog_inst* inst = re__prog_get(prog, i);
        printf("%04X | ", i);
        switch (re__prog_inst_get_type(inst)) {
            case RE__PROG_INST_TYPE_BYTE:
                printf("BYTE v=%0X ('%c')", re__prog_inst_get_byte(inst), re__prog_inst_get_byte(inst));
                break;
            case RE__PROG_INST_TYPE_BYTE_RANGE:
                printf("BYTE_RANGE min=%X ('%c') max=%X ('%c')",
                    re__prog_inst_get_byte_min(inst),
                    re__prog_inst_get_byte_min(inst),
                    re__prog_inst_get_byte_max(inst),
                    re__prog_inst_get_byte_max(inst));
                break;
            case RE__PROG_INST_TYPE_SPLIT:
                printf("SPLIT");
                break;
            case RE__PROG_INST_TYPE_MATCH:
                printf("MATCH idx=%u", re__prog_inst_get_match_idx(inst));
                break;
            case RE__PROG_INST_TYPE_FAIL:
                printf("FAIL");
                break;
            case RE__PROG_INST_TYPE_SAVE:
                printf("SAVE idx=%u", re__prog_inst_get_save_idx(inst));
                break;
            case RE__PROG_INST_TYPE_ASSERT:
                printf("ASSERT ctx=%u", re__prog_inst_get_assert_ctx(inst));
                break;
            default:
                RE__ASSERT_UNREACHED();
                break;
        }
        printf(" -> %04X", re__prog_inst_get_primary(inst));
        if (re__prog_inst_get_type(inst) == RE__PROG_INST_TYPE_SPLIT) {
            printf(", %04X", re__prog_inst_get_split_secondary(inst));
        }
        printf("\n");
    }
}

#endif

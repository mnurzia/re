#include "re_internal.h"

RE_INTERNAL void re__prog_inst_init(re__prog_inst* inst, re__prog_inst_type inst_type) {
    inst->_inst_type = inst_type;
    inst->_primary = RE__PROG_LOC_INVALID;
}

RE_INTERNAL void re__prog_inst_init_byte(re__prog_inst* inst, re_uint8 byte) {
    re__prog_inst_init(inst, RE__PROG_INST_TYPE_BYTE);
    inst->_inst_data._byte = byte;
}

RE_INTERNAL void re__prog_inst_init_byte_range(re__prog_inst* inst, re__byte_range br) {
    re__prog_inst_init(inst, RE__PROG_INST_TYPE_BYTE_RANGE);
    inst->_inst_data._range.min = br.min;
    inst->_inst_data._range.max = br.max;
}

RE_INTERNAL void re__prog_inst_init_split(re__prog_inst* inst, re__prog_loc primary, re__prog_loc secondary) {
    re__prog_inst_init(inst, RE__PROG_INST_TYPE_SPLIT);
    inst->_primary = primary;
    inst->_inst_data._secondary = secondary;
}

RE_INTERNAL void re__prog_inst_init_match(re__prog_inst* inst, re_uint32 match_idx) {
    re__prog_inst_init(inst, RE__PROG_INST_TYPE_MATCH);
    inst->_inst_data._match_idx = match_idx;
}

RE_INTERNAL void re__prog_inst_init_fail(re__prog_inst* inst) {
    re__prog_inst_init(inst, RE__PROG_INST_TYPE_FAIL);
}

RE_INTERNAL void re__prog_inst_init_assert(re__prog_inst* inst, re_uint32 assert_context) {
    re__prog_inst_init(inst, RE__PROG_INST_TYPE_ASSERT);
    inst->_inst_data._assert_context = assert_context;
}

RE_INTERNAL void re__prog_inst_init_save(re__prog_inst* inst, re_uint32 save_idx) {
    re__prog_inst_init(inst, RE__PROG_INST_TYPE_SAVE);
    inst->_inst_data._save_idx = save_idx;
}

RE_INTERNAL re__prog_loc re__prog_inst_get_primary(re__prog_inst* inst) {
    return inst->_primary;
}

RE_INTERNAL void re__prog_inst_set_primary(re__prog_inst* inst, re__prog_loc loc) {
    inst->_primary = loc;
}

RE_INTERNAL re_uint8 re__prog_inst_get_byte(re__prog_inst* inst) {
    RE_ASSERT(inst->_inst_type == RE__PROG_INST_TYPE_BYTE);
    return inst->_inst_data._byte;
}

RE_INTERNAL re_uint8 re__prog_inst_get_byte_min(re__prog_inst* inst) {
    RE_ASSERT(inst->_inst_type == RE__PROG_INST_TYPE_BYTE_RANGE);
    return inst->_inst_data._range.min;
}

RE_INTERNAL re_uint8 re__prog_inst_get_byte_max(re__prog_inst* inst) {
    RE_ASSERT(inst->_inst_type == RE__PROG_INST_TYPE_BYTE_RANGE);
    return inst->_inst_data._range.max;
}

RE_INTERNAL re__prog_loc re__prog_inst_get_split_secondary(re__prog_inst* inst) {
    RE_ASSERT(inst->_inst_type == RE__PROG_INST_TYPE_SPLIT);
    return inst->_inst_data._secondary;
}

RE_INTERNAL void re__prog_inst_set_split_secondary(re__prog_inst* inst, re__prog_loc loc) {
    RE_ASSERT(inst->_inst_type == RE__PROG_INST_TYPE_SPLIT);
    inst->_inst_data._secondary = loc;
}

RE_INTERNAL re__prog_inst_type re__prog_inst_get_type(re__prog_inst* inst) {
    return inst->_inst_type;
}

RE_INTERNAL re_uint32 re__prog_inst_get_match_idx(re__prog_inst* inst) {
    RE_ASSERT(inst->_inst_type == RE__PROG_INST_TYPE_MATCH);
    return inst->_inst_data._match_idx;
}

RE_INTERNAL re_uint32 re__prog_inst_get_save_idx(re__prog_inst* inst) {
    RE_ASSERT(inst->_inst_type == RE__PROG_INST_TYPE_SAVE);
    return inst->_inst_data._save_idx;
}

RE_INTERNAL re_uint32 re__prog_inst_get_assert_ctx(re__prog_inst* inst) {
    RE_ASSERT(inst->_inst_type == RE__PROG_INST_TYPE_ASSERT);
    return inst->_inst_data._assert_context;
}

RE_VEC_IMPL_FUNC(re__prog_inst, init)
RE_VEC_IMPL_FUNC(re__prog_inst, destroy)
RE_VEC_IMPL_FUNC(re__prog_inst, push)
RE_VEC_IMPL_FUNC(re__prog_inst, get)
RE_VEC_IMPL_FUNC(re__prog_inst, getref)
RE_VEC_IMPL_FUNC(re__prog_inst, set)
RE_VEC_IMPL_FUNC(re__prog_inst, size)

RE_INTERNAL re_error re__prog_init(re__prog* prog) {
    re__prog_inst_vec_init(&prog->_instructions);
    return RE_ERROR_NONE;
}

RE_INTERNAL void re__prog_destroy(re__prog* prog) {
    re__prog_inst_vec_destroy(&prog->_instructions);
}

RE_INTERNAL re__prog_loc re__prog_size(re__prog* prog) {
    return (re__prog_loc)re__prog_inst_vec_size(&prog->_instructions);
}

RE_INTERNAL re__prog_inst* re__prog_get(re__prog* prog, re__prog_loc loc) {
    return re__prog_inst_vec_getref(&prog->_instructions, loc);
}

RE_INTERNAL void re__prog_set(re__prog* prog, re__prog_loc loc, re__prog_inst inst) {
    re__prog_inst_vec_set(&prog->_instructions, loc, inst);
}

RE_INTERNAL re_error re__prog_add(re__prog* prog, re__prog_inst inst) {
    if (re__prog_size(prog) == RE__PROG_SIZE_MAX) {
        return RE__ERROR_PROGMAX;
    } else {
        return re__prog_inst_vec_push(&prog->_instructions, inst);
    }
}

#if RE_DEBUG

RE_INTERNAL void re__prog_debug_dump(re__prog* prog) {
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

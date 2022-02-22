#include "re_internal.h"

#define RE__EXEC_SAVE_REF_NONE -1

RE_VEC_IMPL_FUNC(re_size, init)
RE_VEC_IMPL_FUNC(re_size, destroy)
RE_VEC_IMPL_FUNC(re_size, size)
RE_VEC_IMPL_FUNC(re_size, reserve)
RE_VEC_IMPL_FUNC(re_size, push)
RE_VEC_IMPL_FUNC(re_size, getref)

RE_INTERNAL void re__exec_save_init(re__exec_save* save, re_uint32 max_slots_per_thrd) {
    re_size_vec_init(&save->slots);
    save->last_empty_ref = RE__EXEC_SAVE_REF_NONE;
    save->slots_per_thrd = max_slots_per_thrd + 1; /* +1 for ref count */
}

RE_INTERNAL void re__exec_save_destroy(re__exec_save* save) {
    re_size_vec_destroy(&save->slots);
}

RE_INTERNAL re_size* re__exec_save_get_slots(re__exec_save* save, re_int32 slots_ref) {
    RE_ASSERT(slots_ref != RE__EXEC_SAVE_REF_NONE);
    return re_size_vec_getref(&save->slots, (re_size)slots_ref);
}

RE_INTERNAL void re__exec_save_inc_refs(re__exec_save* save, re_int32 slots_ref) {
    re_size* slots = re__exec_save_get_slots(save, slots_ref);
    slots[save->slots_per_thrd - 1] += 1;
}

RE_INTERNAL void re__exec_save_dec_refs(re__exec_save* save, re_int32 slots_ref) {
    re_size* slots = re__exec_save_get_slots(save, slots_ref);
    if (--slots[save->slots_per_thrd - 1] == 0) {
        re_int32* next_empty_save = ((re_int32*)(slots + (save->slots_per_thrd - 1)));
        save->last_empty_ref = slots_ref;
        /* hacky */
        *next_empty_save = save->last_empty_ref;
    }
}

RE_INTERNAL re_size re__exec_save_get_refs(re__exec_save* save, re_int32 slots_ref) {
    re_size* slots = re__exec_save_get_slots(save, slots_ref);
    return slots[save->slots_per_thrd - 1] += 1;
}

RE_INTERNAL re_error re__exec_save_get_new(re__exec_save* save, re_int32* slots_out_ref) {
    re_size* slots;
    re_error err = RE_ERROR_NONE;
    if (save->last_empty_ref != RE__EXEC_SAVE_REF_NONE) {
        *slots_out_ref = save->last_empty_ref;
        slots = re__exec_save_get_slots(save, save->last_empty_ref);
        save->last_empty_ref = *((re_int32*)(slots + (save->slots_per_thrd - 1)));
        re__exec_save_inc_refs(save, *slots_out_ref);
        return err;
    } else {
        /* need realloc */
        re_uint32 i;
        *slots_out_ref = (re_int32)re_size_vec_size(&save->slots);
        if ((err = re_size_vec_reserve(&save->slots, re_size_vec_size(&save->slots) + save->slots_per_thrd))) {
            return err;
        }
        for (i = 0; i < save->slots_per_thrd - 1; i++) {
            if ((err = re_size_vec_push(&save->slots, 0))) {
                return err;
            }
        }
        re__exec_save_inc_refs(save, *slots_out_ref);
        return err;
    }
}

RE_INTERNAL re_error re__exec_save_do_save(re__exec_save* save, re_int32* slots_inout_ref, re_uint32 slot_number, re_size data) {
    re_error err = RE_ERROR_NONE;
    re_size* slots;
    if (*slots_inout_ref == RE__EXEC_SAVE_REF_NONE) {
        if ((err = re__exec_save_get_new(save, slots_inout_ref))) {
            return err;
        }
        slots = re__exec_save_get_slots(save, *slots_inout_ref);
    } else if (re__exec_save_get_refs(save, *slots_inout_ref) > 1) {
        re_int32 old_slots_ref = *slots_inout_ref;
        re_uint32 i;
        re_size* old_slots;
        re__exec_save_dec_refs(save, old_slots_ref);
        if ((err = re__exec_save_get_new(save, slots_inout_ref))) {
            return err;
        }
        slots = re__exec_save_get_slots(save, *slots_inout_ref);
        old_slots = re__exec_save_get_slots(save, old_slots_ref);
        /* copy over */
        for (i = 0; i < save->slots_per_thrd - 1; i++) {
            slots[i] = old_slots[i];
        }
    }
    RE_ASSERT(slot_number < save->slots_per_thrd - 1);
    slots[slot_number] = data;
    return err;
}

RE_INTERNAL void re__exec_thrd_init(re__exec_thrd* thrd, re__prog_loc loc, re_int32 save_slot) {
    thrd->loc = loc;
    thrd->save_slot = save_slot;
}

RE_INTERNAL void re__exec_thrd_set_init(re__exec_thrd_set* set) {
    set->size = 0;
    set->n = 0;
    set->dense = RE_NULL;
    set->sparse = RE_NULL;
}

RE_INTERNAL void re__exec_thrd_set_destroy(re__exec_thrd_set* set) {
    if (set->dense) {
        RE_FREE(set->dense);
        set->dense = RE_NULL;
    }
    if (set->sparse) {
        RE_FREE(set->sparse);
        set->sparse = RE_NULL;
    }
}

RE_INTERNAL re_error re__exec_thrd_set_alloc(re__exec_thrd_set* set, re__prog_loc size) {
    re_error err = RE_ERROR_NONE;
    set->size = size;
    set->dense = (re__exec_thrd*)RE_MALLOC(sizeof(re__prog_loc) * size);
    if (set->dense == RE_NULL) {
        return RE_ERROR_NOMEM;
    }
    set->sparse = (re__prog_loc*)RE_MALLOC(sizeof(re__exec_thrd) * size);
    if (set->sparse == RE_NULL) {
        return RE_ERROR_NOMEM;
    }
    return err;
}

RE_INTERNAL void re__exec_thrd_set_add(re__exec_thrd_set* set, re__exec_thrd thrd) {
    RE_ASSERT(thrd.loc < set->size);
    set->dense[set->n] = thrd;
    set->sparse[thrd.loc] = set->n;
    set->n++;
}

RE_INTERNAL void re__exec_thrd_set_clear(re__exec_thrd_set* set) {
    set->n = 0;
}

RE_INTERNAL int re__exec_thrd_set_ismemb(re__exec_thrd_set* set, re__exec_thrd thrd) {
    RE_ASSERT(thrd.loc < set->size);
    return set->sparse[thrd.loc] < set->n && 
           set->dense[set->sparse[thrd.loc]].loc == thrd.loc;
}

#if RE_DEBUG

RE_INTERNAL void re__exec_thrd_set_dump(re__exec_thrd_set* set, re__exec* exec, int with_save) {
    printf("n: %u\n", set->n);
    printf("s: %u\n", set->size);
    printf("memb:\n");
    {
        re_uint32 i;
        for (i = 0; i < set->n; i++) {
            printf("  %04X, %i\n", set->dense[i].loc, set->dense[i].save_slot);
        }
    }
    if (with_save) {
        printf("slots:\n");
        {
            re_uint32 i;
            for (i = 0; i < set->n; i++) {
                re_int32 slot_ref = set->dense[i].save_slot;
                re_size* slots;
                re_uint32 j;
                if (slot_ref == RE__EXEC_SAVE_REF_NONE) {
                    continue;
                }
                slots = re__exec_save_get_slots(&exec->save_slots, slot_ref);
                printf("%i: %u\n", slot_ref, (re_uint32)re__exec_save_get_refs(&exec->save_slots, slot_ref));
                for (j = 0; j < exec->save_slots.slots_per_thrd - 1; j++) {
                    printf("  %i: %u\n", j, (re_uint32)slots[j]);
                }
            }
        }
    }
}

#endif

RE_VEC_IMPL_FUNC(re__exec_thrd, init)
RE_VEC_IMPL_FUNC(re__exec_thrd, destroy)
RE_VEC_IMPL_FUNC(re__exec_thrd, push)
RE_VEC_IMPL_FUNC(re__exec_thrd, pop)
RE_VEC_IMPL_FUNC(re__exec_thrd, clear)
RE_VEC_IMPL_FUNC(re__exec_thrd, size)

RE_INTERNAL void re__exec_init(re__exec* exec, re* re) {
    exec->re = re;
    re__exec_thrd_set_init(&exec->set_a);
    re__exec_thrd_set_init(&exec->set_b);
    re__exec_thrd_set_init(&exec->set_c);
    re__exec_thrd_vec_init(&exec->thrd_stk);
    re__exec_save_init(&exec->save_slots, re__ast_root_get_num_groups(&re->data->ast_root));
}

RE_INTERNAL void re__exec_destroy(re__exec* exec) {
    re__exec_save_destroy(&exec->save_slots);
    re__exec_thrd_vec_destroy(&exec->thrd_stk);
    re__exec_thrd_set_destroy(&exec->set_c);
    re__exec_thrd_set_destroy(&exec->set_b);
    re__exec_thrd_set_destroy(&exec->set_a);
}

RE_INTERNAL re_error re__exec_follow(re__exec* exec, re__exec_thrd thrd, re__ast_assert_type assert_context, re_size pos) {
    re__prog* prog = &exec->re->data->program;
    re_error err = RE_ERROR_NONE;
    re__exec_thrd_vec_clear(&exec->thrd_stk);
    re__exec_thrd_set_clear(&exec->set_c);
    if ((err = re__exec_thrd_vec_push(&exec->thrd_stk, thrd))) {
        return err;
    }
    while (re__exec_thrd_vec_size(&exec->thrd_stk)) {
        re__exec_thrd top = re__exec_thrd_vec_pop(&exec->thrd_stk);
        re__prog_inst* inst = re__prog_get(prog, top.loc);
        re__prog_inst_type inst_type;
        RE_ASSERT(top.loc != 0);
        if (re__exec_thrd_set_ismemb(&exec->set_c, top)) {
            /* target is already a member */
            continue;
        } else {
            re__exec_thrd_set_add(&exec->set_c, top);
        }
        inst_type = re__prog_inst_get_type(inst);
        if (inst_type == RE__PROG_INST_TYPE_BYTE) {
            re__exec_thrd_set_add(&exec->set_b, top);
        } else if (inst_type == RE__PROG_INST_TYPE_BYTE_RANGE) {
            re__exec_thrd_set_add(&exec->set_b, top);
        } else if (inst_type == RE__PROG_INST_TYPE_SPLIT) {
            /* push secondary first to maintain priority */
            re__exec_thrd secondary_thrd, primary_thrd;
            re__exec_thrd_init(
                &secondary_thrd, 
                re__prog_inst_get_split_secondary(inst),
                top.save_slot
            );
            re__exec_thrd_init(
                &primary_thrd,
                re__prog_inst_get_primary(inst),
                top.save_slot
            );
            if (top.save_slot != RE__EXEC_SAVE_REF_NONE) {
                re__exec_save_inc_refs(&exec->save_slots, top.save_slot);
            }
            if ((err = re__exec_thrd_vec_push(&exec->thrd_stk, secondary_thrd))) {
                return err;
            }
            if ((err = re__exec_thrd_vec_push(&exec->thrd_stk, primary_thrd))) {
                return err;
            }
        } else if (inst_type == RE__PROG_INST_TYPE_MATCH) {
            re__exec_thrd_set_add(&exec->set_b, top);
        } else if (inst_type == RE__PROG_INST_TYPE_SAVE) {
            re__exec_thrd primary_thrd;
            re__exec_thrd_init(&primary_thrd, re__prog_inst_get_primary(inst), top.save_slot);
            if ((err = re__exec_save_do_save(&exec->save_slots, &primary_thrd.save_slot, re__prog_inst_get_save_idx(inst), pos))) {
                return err;
            }
            if ((err = re__exec_thrd_vec_push(&exec->thrd_stk, primary_thrd))) {
                return err;
            }
        } else if (inst_type == RE__PROG_INST_TYPE_ASSERT) {
            re__exec_thrd primary_thrd;
            re__exec_thrd_init(&primary_thrd, re__prog_inst_get_primary(inst), top.save_slot);
            if ((assert_context & re__prog_inst_get_assert_ctx(inst)) == assert_context) {
                if ((err = re__exec_thrd_vec_push(&exec->thrd_stk, primary_thrd))) {
                    return err;
                }
            } else {
                re__exec_save_dec_refs(&exec->save_slots, primary_thrd.save_slot);
            }
        } else {
            RE__ASSERT_UNREACHED();
        }
    }
    return err;
}

RE_INTERNAL void re__exec_swap(re__exec* exec) {
    re__exec_thrd_set temp = exec->set_a;
    exec->set_a = exec->set_b;
    exec->set_b = temp;
}

RE_INTERNAL re_error re__exec_nfa(re__exec* exec, re__str_view str_view) {
    re_error err = RE_ERROR_NONE;
    re__prog* prog = &exec->re->data->program;
    re__prog_loc set_size = re__prog_size(prog);
    re__ast_assert_type assert_ctx;
    re__exec_thrd thrd;
    re_size pos, j;
    const re_char* str = re__str_view_get_data(&str_view);
    if ((err = re__exec_thrd_set_alloc(&exec->set_a, set_size))) {
        return err;
    }
    if ((err = re__exec_thrd_set_alloc(&exec->set_b, set_size))) {
        return err;
    }
    if ((err = re__exec_thrd_set_alloc(&exec->set_c, set_size))) {
        return err;
    }
    assert_ctx = RE__AST_ASSERT_TYPE_TEXT_START_ABSOLUTE | RE__AST_ASSERT_TYPE_TEXT_START;
    re__exec_thrd_init(&thrd, 1, RE__EXEC_SAVE_REF_NONE);
    if ((err = re__exec_follow(exec, thrd, assert_ctx, 0))) {
        return err;
    }
    re__exec_thrd_set_clear(&exec->set_a);
    re__exec_swap(exec);
    for (pos = 0; pos < re__str_view_size(&str_view); pos++) {
        re_char ch = str[pos];
        for (j = 0; j < exec->set_a.n; j++) {
            re__exec_thrd cur_thrd = exec->set_a.dense[j];
            re__prog_inst* cur_inst = re__prog_get(prog, cur_thrd.loc);
            re__prog_inst_type cur_inst_type = re__prog_inst_get_type(cur_inst);
            if (cur_inst_type == RE__PROG_INST_TYPE_BYTE) {
                if (ch == re__prog_inst_get_byte(cur_inst)) {
                    re__exec_thrd primary_thrd;
                    re__exec_thrd_init(&primary_thrd, re__prog_inst_get_primary(cur_inst), cur_thrd.save_slot);
                    if ((err = re__exec_follow(exec, cur_thrd, assert_ctx, pos))) {
                        return err;
                    }
                } else {
                    re__exec_save_dec_refs(&exec->save_slots, cur_thrd.save_slot);
                }
            } else if (cur_inst_type == RE__PROG_INST_TYPE_BYTE_RANGE) {
                if (ch >= re__prog_inst_get_byte_min(cur_inst) && ch <= re__prog_inst_get_byte_max(cur_inst)) {
                    re__exec_thrd primary_thrd;
                    re__exec_thrd_init(&primary_thrd, re__prog_inst_get_primary(cur_inst), cur_thrd.save_slot);
                    if ((err = re__exec_follow(exec, cur_thrd, assert_ctx, pos))) {
                        return err;
                    }
                } else {
                    re__exec_save_dec_refs(&exec->save_slots, cur_thrd.save_slot);
                }
            } else if (cur_inst_type == RE__PROG_INST_TYPE_MATCH) {
                /* nothin */
            } else {
                RE__ASSERT_UNREACHED();
            }
        }
        re__exec_thrd_set_clear(&exec->set_a);
        re__exec_swap(exec);
    }
    re__exec_thrd_set_dump(&exec->set_a, exec, 1);
    return err;
}

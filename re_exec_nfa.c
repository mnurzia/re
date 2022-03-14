#include "re_internal.h"

#define RE__EXEC_SAVE_REF_NONE -1

MN__VEC_IMPL_FUNC(mn_size, init)
MN__VEC_IMPL_FUNC(mn_size, destroy)
MN__VEC_IMPL_FUNC(mn_size, clear)
MN__VEC_IMPL_FUNC(mn_size, size)
MN__VEC_IMPL_FUNC(mn_size, reserve)
MN__VEC_IMPL_FUNC(mn_size, push)
MN__VEC_IMPL_FUNC(mn_size, getref)
MN__VEC_IMPL_FUNC(mn_size, getcref)

MN_INTERNAL void re__exec_save_init(re__exec_save* save) {
    mn_size_vec_init(&save->slots);
    save->last_empty_ref = RE__EXEC_SAVE_REF_NONE;
    save->slots_per_thrd = 1; /* +1 for ref count */
}

MN_INTERNAL void re__exec_save_start(re__exec_save* save) {
    mn_size_vec_clear(&save->slots);
}

MN_INTERNAL mn_uint32 re__exec_save_get_slots_per_thrd(re__exec_save* save) {
    return save->slots_per_thrd - 1;
}

MN_INTERNAL void re__exec_save_set_slots_per_thrd(re__exec_save* save, mn_uint32 slots_per_thrd) {
    save->slots_per_thrd = slots_per_thrd + 1;
}

MN_INTERNAL void re__exec_save_destroy(re__exec_save* save) {
    mn_size_vec_destroy(&save->slots);
}

MN_INTERNAL const mn_size* re__exec_save_get_slots_const(const re__exec_save* save, mn_int32 slots_ref) {
    MN_ASSERT(slots_ref != RE__EXEC_SAVE_REF_NONE);
    return mn_size_vec_getcref(&save->slots, (mn_size)slots_ref);
}

MN_INTERNAL mn_size* re__exec_save_get_slots(re__exec_save* save, mn_int32 slots_ref) {
    MN_ASSERT(slots_ref != RE__EXEC_SAVE_REF_NONE);
    return mn_size_vec_getref(&save->slots, (mn_size)slots_ref);
}

MN_INTERNAL void re__exec_save_inc_refs(re__exec_save* save, mn_int32 slots_ref) {
    mn_size* slots = re__exec_save_get_slots(save, slots_ref);
    slots[save->slots_per_thrd - 1] += 1;
}

MN_INTERNAL void re__exec_save_dec_refs(re__exec_save* save, mn_int32 slots_ref) {
    mn_size* slots;
    if (slots_ref == -1) {
        return;
    }
    slots = re__exec_save_get_slots(save, slots_ref);
    if (--slots[save->slots_per_thrd - 1] == 0) {
        mn_int32* next_empty_save = ((mn_int32*)(slots + (save->slots_per_thrd - 1)));
        save->last_empty_ref = slots_ref;
        /* hacky */
        *next_empty_save = save->last_empty_ref;
    }
}

MN_INTERNAL mn_size re__exec_save_get_refs(const re__exec_save* save, mn_int32 slots_ref) {
    const mn_size* slots = re__exec_save_get_slots_const(save, slots_ref);
    return slots[save->slots_per_thrd - 1];
}

MN_INTERNAL re_error re__exec_save_get_new(re__exec_save* save, mn_int32* slots_out_ref) {
    mn_size* slots;
    re_error err = RE_ERROR_NONE;
    if (save->last_empty_ref != RE__EXEC_SAVE_REF_NONE) {
        *slots_out_ref = save->last_empty_ref;
        slots = re__exec_save_get_slots(save, save->last_empty_ref);
        save->last_empty_ref = *((mn_int32*)(slots + (save->slots_per_thrd - 1)));
        re__exec_save_inc_refs(save, *slots_out_ref);
        return err;
    } else {
        /* need realloc */
        mn_uint32 i;
        *slots_out_ref = (mn_int32)mn_size_vec_size(&save->slots);
        if ((err = mn_size_vec_reserve(&save->slots, mn_size_vec_size(&save->slots) + save->slots_per_thrd))) {
            return err;
        }
        for (i = 0; i < save->slots_per_thrd; i++) {
            if ((err = mn_size_vec_push(&save->slots, 0))) {
                return err;
            }
        }
        re__exec_save_inc_refs(save, *slots_out_ref);
        return err;
    }
}

MN_INTERNAL re_error re__exec_save_do_save(re__exec_save* save, mn_int32* slots_inout_ref, mn_uint32 slot_number, mn_size data) {
    re_error err = RE_ERROR_NONE;
    mn_size* slots;
    if (*slots_inout_ref == RE__EXEC_SAVE_REF_NONE) {
        if ((err = re__exec_save_get_new(save, slots_inout_ref))) {
            return err;
        }
        slots = re__exec_save_get_slots(save, *slots_inout_ref);
    } else if (re__exec_save_get_refs(save, *slots_inout_ref) > 1) {
        mn_int32 old_slots_ref = *slots_inout_ref;
        mn_uint32 i;
        mn_size* old_slots;
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
    } else {
        slots = re__exec_save_get_slots(save, *slots_inout_ref);
    }
    MN_ASSERT(slot_number < save->slots_per_thrd - 1);
    slots[slot_number] = data;
    return err;
}

MN_INTERNAL void re__exec_thrd_init(re__exec_thrd* thrd, re__prog_loc loc, mn_int32 save_slot) {
    thrd->loc = loc;
    thrd->save_slot = save_slot;
}

MN_INTERNAL void re__exec_thrd_set_init(re__exec_thrd_set* set) {
    set->size = 0;
    set->n = 0;
    set->dense = MN_NULL;
    set->sparse = MN_NULL;
    set->match = 0;
}

MN_INTERNAL void re__exec_thrd_set_destroy(re__exec_thrd_set* set) {
    if (set->dense) {
        MN_FREE(set->dense);
        set->dense = MN_NULL;
    }
    if (set->sparse) {
        MN_FREE(set->sparse);
        set->sparse = MN_NULL;
    }
}

MN_INTERNAL re_error re__exec_thrd_set_alloc(re__exec_thrd_set* set, re__prog_loc size) {
    re_error err = RE_ERROR_NONE;
    set->size = size;
    if (!set->dense) {
        set->dense = (re__exec_thrd*)MN_MALLOC(sizeof(re__prog_loc) * size);
    } else {
        set->dense = (re__exec_thrd*)MN_REALLOC(set->dense, sizeof(re__prog_loc) * size);
    }
    if (set->dense == MN_NULL) {
        return RE_ERROR_NOMEM;
    }
    if (!set->sparse) {
        set->sparse = (re__prog_loc*)MN_MALLOC(sizeof(re__exec_thrd) * size);
    } else {
        set->sparse = (re__prog_loc*)MN_REALLOC(set->sparse, sizeof(re__exec_thrd) * size);   
    }
    if (set->sparse == MN_NULL) {
        return RE_ERROR_NOMEM;
    }
    return err;
}

MN_INTERNAL void re__exec_thrd_set_add(re__exec_thrd_set* set, re__exec_thrd thrd) {
    MN_ASSERT(thrd.loc < set->size);
    set->dense[set->n] = thrd;
    set->sparse[thrd.loc] = set->n;
    set->n++;
}

MN_INTERNAL void re__exec_thrd_set_clear(re__exec_thrd_set* set) {
    set->n = 0;
    set->match = 0;
}

MN_INTERNAL int re__exec_thrd_set_ismemb(re__exec_thrd_set* set, re__exec_thrd thrd) {
    MN_ASSERT(thrd.loc < set->size);
    return set->sparse[thrd.loc] < set->n && 
           set->dense[set->sparse[thrd.loc]].loc == thrd.loc;
}

#if RE_DEBUG

MN_INTERNAL void re__exec_thrd_set_dump(const re__exec_thrd_set* set, const re__exec_nfa* exec, int with_save) {
    printf("n: %u\n", set->n);
    printf("s: %u\n", set->size);
    printf("memb:\n");
    {
        mn_uint32 i;
        for (i = 0; i < set->n; i++) {
            printf("  %04X, %i\n", set->dense[i].loc, set->dense[i].save_slot);
        }
    }
    if (with_save) {
        printf("slots:\n");
        {
            mn_uint32 i;
            for (i = 0; i < set->n; i++) {
                mn_int32 slot_ref = set->dense[i].save_slot;
                const mn_size* slots;
                mn_uint32 j;
                if (slot_ref == RE__EXEC_SAVE_REF_NONE) {
                    continue;
                }
                slots = re__exec_save_get_slots_const(&exec->save_slots, slot_ref);
                printf("%i: %u\n", slot_ref, (mn_uint32)re__exec_save_get_refs(&exec->save_slots, slot_ref));
                for (j = 0; j < exec->save_slots.slots_per_thrd - 1; j++) {
                    printf("  %i: %u\n", j, (mn_uint32)slots[j]);
                }
            }
        }
    }
}

#endif

MN__VEC_IMPL_FUNC(re__exec_thrd, init)
MN__VEC_IMPL_FUNC(re__exec_thrd, destroy)
MN__VEC_IMPL_FUNC(re__exec_thrd, push)
MN__VEC_IMPL_FUNC(re__exec_thrd, pop)
MN__VEC_IMPL_FUNC(re__exec_thrd, clear)
MN__VEC_IMPL_FUNC(re__exec_thrd, size)

MN_INTERNAL void re__exec_nfa_init(re__exec_nfa* exec, const re__prog* prog, re_match_anchor_type anchor_type, mn_uint32 num_groups) {
    exec->prog = prog;
    exec->anchor_type = anchor_type;
    exec->num_groups = num_groups;
    re__exec_thrd_set_init(&exec->set_a);
    re__exec_thrd_set_init(&exec->set_b);
    re__exec_thrd_set_init(&exec->set_c);
    re__exec_thrd_vec_init(&exec->thrd_stk);
    re__exec_save_init(&exec->save_slots);
}

MN_INTERNAL void re__exec_nfa_destroy(re__exec_nfa* exec) {
    re__exec_save_destroy(&exec->save_slots);
    re__exec_thrd_vec_destroy(&exec->thrd_stk);
    re__exec_thrd_set_destroy(&exec->set_c);
    re__exec_thrd_set_destroy(&exec->set_b);
    re__exec_thrd_set_destroy(&exec->set_a);
}

MN_INTERNAL const re__exec_thrd* re__exec_nfa_get_thrds(re__exec_nfa* exec, re__prog_loc* out_thrds_size) {
    *out_thrds_size = exec->set_a.n;
    return exec->set_a.dense;
}

MN_INTERNAL void re__exec_nfa_set_thrds(re__exec_nfa* exec, const re__prog_loc* in_thrds, re__prog_loc in_thrds_size) {
    re__prog_loc i;
    for (i = 0; i < in_thrds_size; i++) {
        re__exec_thrd new_thrd;
        re__exec_thrd_init(&new_thrd, in_thrds[i], RE__EXEC_SAVE_REF_NONE);
        re__exec_thrd_set_add(&exec->set_a, new_thrd);
    }
}

MN_INTERNAL void re__exec_nfa_swap(re__exec_nfa* exec) {
    re__exec_thrd_set temp = exec->set_a;
    exec->set_a = exec->set_b;
    exec->set_b = temp;
}

MN_INTERNAL re_error re__exec_follow(re__exec_nfa* exec, const re__prog* prog, re__exec_thrd thrd, re__ast_assert_type assert_context, mn_size pos) {
    re_error err = RE_ERROR_NONE;
    re__exec_thrd_vec_clear(&exec->thrd_stk);
    re__exec_thrd_set_clear(&exec->set_c);
    if ((err = re__exec_thrd_vec_push(&exec->thrd_stk, thrd))) {
        return err;
    }
    while (re__exec_thrd_vec_size(&exec->thrd_stk)) {
        re__exec_thrd top = re__exec_thrd_vec_pop(&exec->thrd_stk);
        const re__prog_inst* inst = re__prog_cget(prog, top.loc);
        re__prog_inst_type inst_type;
        MN_ASSERT(top.loc != 0);
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
            mn_uint32 match_idx = re__prog_inst_get_match_idx(inst);
            exec->set_b.match = match_idx;
            re__exec_thrd_set_add(&exec->set_b, top);
        } else if (inst_type == RE__PROG_INST_TYPE_SAVE) {
            re__exec_thrd primary_thrd;
            mn_uint32 save_idx = re__prog_inst_get_save_idx(inst);
            re__exec_thrd_init(&primary_thrd, re__prog_inst_get_primary(inst), top.save_slot);
            if (save_idx < re__exec_save_get_slots_per_thrd(&exec->save_slots)) {
                if ((err = re__exec_save_do_save(&exec->save_slots, &primary_thrd.save_slot, re__prog_inst_get_save_idx(inst), pos))) {
                    return err;
                }
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
            MN__ASSERT_UNREACHED();
        }
    }
    return err;
}

MN_INTERNAL re_error re__exec_nfa_start(re__exec_nfa* exec, re__ast_assert_type assert_ctx, re__prog_loc start_loc) {
    re_error err = RE_ERROR_NONE;
    re__prog_loc set_size = re__prog_size(exec->prog);
    re__exec_thrd initial;
    re__exec_save_start(&exec->save_slots);
    re__exec_save_set_slots_per_thrd(&exec->save_slots, exec->num_groups * 2);
    if ((err = re__exec_thrd_set_alloc(&exec->set_a, set_size))) {
        return err;
    }
    if ((err = re__exec_thrd_set_alloc(&exec->set_b, set_size))) {
        return err;
    }
    if ((err = re__exec_thrd_set_alloc(&exec->set_c, set_size))) {
        return err;
    }
    re__exec_thrd_init(&initial, start_loc, -1);
    if ((err = re__exec_follow(exec, exec->prog, initial, assert_ctx, 0))) {
        return err;
    }
    re__exec_thrd_set_clear(&exec->set_a);
    re__exec_nfa_swap(exec);
    return err;
}

MN_INTERNAL re__ast_assert_type re__exec_nfa_next_assert_ctx(mn_size pos, mn_size len) {
    re__ast_assert_type out = 0;
    if (pos == 0) {
        out |= RE__AST_ASSERT_TYPE_TEXT_START_ABSOLUTE | RE__AST_ASSERT_TYPE_TEXT_START;
    }
    if (pos == len) {
        out |= RE__AST_ASSERT_TYPE_TEXT_END_ABSOLUTE | RE__AST_ASSERT_TYPE_TEXT_END;
    }
    return out;
}

MN_INTERNAL re_error re__exec_nfa_do(re__exec_nfa* exec, re__prog* prog, re_match_anchor_type anchor_type, mn_uint32 num_groups, mn__str_view str_view, re_span* out) {
    re_error err = RE_ERROR_NONE;
    re__prog_loc set_size = re__prog_size(prog);
    re__ast_assert_type assert_ctx;
    re__exec_thrd thrd;
    mn_size pos, j, len = mn__str_view_size(&str_view);
    const mn_char* str = mn__str_view_get_data(&str_view);
    re__exec_save_set_slots_per_thrd(&exec->save_slots, num_groups * 2);
    if ((err = re__exec_thrd_set_alloc(&exec->set_a, set_size))) {
        return err;
    }
    if ((err = re__exec_thrd_set_alloc(&exec->set_b, set_size))) {
        return err;
    }
    if ((err = re__exec_thrd_set_alloc(&exec->set_c, set_size))) {
        return err;
    }
    pos = 0;
    assert_ctx = re__exec_nfa_next_assert_ctx(pos, len);
    re__exec_thrd_init(&thrd, 1, RE__EXEC_SAVE_REF_NONE);
    if ((err = re__exec_follow(exec, prog, thrd, assert_ctx, 0))) {
        return err;
    }
    re__exec_thrd_set_clear(&exec->set_a);
    re__exec_nfa_swap(exec);
    for (; pos < len; pos++) {
        mn_char ch = str[pos];
        assert_ctx = re__exec_nfa_next_assert_ctx(pos + 1, len);
        if (exec->set_a.n == 0) {
            /* no more threads */
            break;
        }
        for (j = 0; j < exec->set_a.n; j++) {
            re__exec_thrd cur_thrd = exec->set_a.dense[j];
            re__prog_inst* cur_inst = re__prog_get(prog, cur_thrd.loc);
            re__prog_inst_type cur_inst_type = re__prog_inst_get_type(cur_inst);
            if (cur_inst_type == RE__PROG_INST_TYPE_BYTE) {
                if (ch == re__prog_inst_get_byte(cur_inst)) {
                    re__exec_thrd primary_thrd;
                    re__exec_thrd_init(&primary_thrd, re__prog_inst_get_primary(cur_inst), cur_thrd.save_slot);
                    if ((err = re__exec_follow(exec, prog, primary_thrd, assert_ctx, pos + 1))) {
                        return err;
                    }
                } else {
                    re__exec_save_dec_refs(&exec->save_slots, cur_thrd.save_slot);
                }
            } else if (cur_inst_type == RE__PROG_INST_TYPE_BYTE_RANGE) {
                if (ch >= re__prog_inst_get_byte_min(cur_inst) && ch <= re__prog_inst_get_byte_max(cur_inst)) {
                    re__exec_thrd primary_thrd;
                    re__exec_thrd_init(&primary_thrd, re__prog_inst_get_primary(cur_inst), cur_thrd.save_slot);
                    if ((err = re__exec_follow(exec, prog, primary_thrd, assert_ctx, pos + 1))) {
                        return err;
                    }
                } else {
                    re__exec_save_dec_refs(&exec->save_slots, cur_thrd.save_slot);
                }
            } else if (cur_inst_type == RE__PROG_INST_TYPE_MATCH) {
                /* do nothing */
            } else {
                MN__ASSERT_UNREACHED();
            }
        }
        if (exec->set_b.match) {
            if (anchor_type == RE_MATCH_ANCHOR_BOTH || anchor_type == RE_MATCH_ANCHOR_END) {
                /* found a match */
                break;
            }
        }
        re__exec_thrd_set_clear(&exec->set_a);
        re__exec_nfa_swap(exec);
    }
    /* check if there are any threads left */
    if (exec->set_a.n) {
        /* extract matches from the top thread */
        re__exec_thrd top_thrd = exec->set_a.dense[0];
        /* check if the top thread has save slots */
        if (top_thrd.save_slot != RE__EXEC_SAVE_REF_NONE) {
            mn_size* slots = re__exec_save_get_slots(&exec->save_slots, top_thrd.save_slot);
            re_span out_span;
            mn_size i;
            /* Set first span (match boundaries) */
            out_span.begin = 0;
            out_span.end = pos;
            *(out++) = out_span;
            /* Write all other groups */
            for (i = 0; i < num_groups; i++) {
                out_span.begin = slots[i*2];
                out_span.end = slots[i*2 + 1];
                *(out++) = out_span;
            }
            return RE_MATCH;
        } else {
            MN_ASSERT(num_groups == 0);
            return RE_MATCH;
        }
    } else {
        return RE_NOMATCH;
    }
}


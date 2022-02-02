#include "re_internal.h"

RE_INTERNAL void re__exec_thrdmin_set_init(re__exec_thrdmin_set* set) {
    set->size = 0;
    set->n = 0;
    set->dense = RE_NULL;
    set->sparse = RE_NULL;
}

RE_INTERNAL re_error re__exec_thrdmin_set_alloc(re__exec_thrdmin_set* set, re__prog_loc size) {
    re_error err = RE_ERROR_NONE;
    set->size = size;
    set->dense = (re__exec_thrdmin*)RE_MALLOC(sizeof(re__exec_thrdmin) * size);
    if (set->dense == RE_NULL) {
        return RE_ERROR_NOMEM;
    }
    set->sparse = (re__exec_thrdmin*)RE_MALLOC(sizeof(re__exec_thrdmin) * size);
    if (set->sparse == RE_NULL) {
        return RE_ERROR_NOMEM;
    }
    return err;
}

RE_INTERNAL void re__exec_thrdmin_set_free(re__exec_thrdmin_set* set) {
    if (set->dense) {
        RE_FREE(set->dense);
        set->dense = RE_NULL;
    }
    if (set->sparse) {
        RE_FREE(set->sparse);
        set->sparse = RE_NULL;
    }
}

RE_INTERNAL void re__exec_thrdmin_set_destroy(re__exec_thrdmin_set* set) {
    RE__UNUSED(set);
}

RE_INTERNAL void re__exec_thrdmin_set_add(re__exec_thrdmin_set* set, re__exec_thrdmin loc) {
    RE_ASSERT(loc < set->size);
    set->dense[set->n] = loc;
    set->sparse[loc] = set->n;
    set->n++;
}

RE_INTERNAL void re__exec_thrdmin_set_clear(re__exec_thrdmin_set* set) {
    set->n = 0;
}

RE_INTERNAL int re__exec_thrdmin_set_ismemb(re__exec_thrdmin_set* set, re__exec_thrdmin loc) {
    RE_ASSERT(loc < set->size);
    return set->sparse[loc] < set->n && 
           set->dense[set->sparse[loc]] == loc;
}

#if RE_DEBUG

RE_INTERNAL void re__exec_thrdmin_set_dump(re__exec_thrdmin_set* set) {
    printf("n: %u\n", set->n);
    printf("s: %u\n", set->size);
    printf("memb:\n");
    {
        re_uint32 i;
        for (i = 0; i < set->n; i++) {
            printf("  %04X\n", set->dense[i]);
        }
    }
}

#endif

RE_VEC_IMPL_FUNC(re__exec_thrdmin, init)
RE_VEC_IMPL_FUNC(re__exec_thrdmin, destroy)
RE_VEC_IMPL_FUNC(re__exec_thrdmin, push)
RE_VEC_IMPL_FUNC(re__exec_thrdmin, pop)
RE_VEC_IMPL_FUNC(re__exec_thrdmin, clear)
RE_VEC_IMPL_FUNC(re__exec_thrdmin, size)

RE_INTERNAL void re__exec_init(re__exec* exec, re* re) {
    exec->re = re;
    re__exec_thrdmin_set_init(&exec->set_a);
    re__exec_thrdmin_set_init(&exec->set_b);
    re__exec_thrdmin_set_init(&exec->set_c);
    re__exec_thrdmin_vec_init(&exec->thrd_stk);
}

RE_INTERNAL void re__exec_destroy(re__exec* exec) {
    re__exec_thrdmin_vec_destroy(&exec->thrd_stk);
    re__exec_thrdmin_set_destroy(&exec->set_c);
    re__exec_thrdmin_set_destroy(&exec->set_b);
    re__exec_thrdmin_set_destroy(&exec->set_a);
}

RE_INTERNAL re_error re__exec_nfa_follow_threads(re__exec* exec, re__exec_thrdmin thrd, re__exec_thrdmin_set* target, re__ast_assert_type assert_context) {
    re__prog* prog = &exec->re->data->program;
    re_error err = RE_ERROR_NONE;
    re__exec_thrdmin_vec_clear(&exec->thrd_stk);
    re__exec_thrdmin_set_clear(&exec->set_c);
    if ((err = re__exec_thrdmin_vec_push(&exec->thrd_stk, thrd))) {
        return err;
    }
    while (re__exec_thrdmin_vec_size(&exec->thrd_stk)) {
        re__exec_thrdmin top = re__exec_thrdmin_vec_pop(&exec->thrd_stk);
        re__prog_inst* inst = re__prog_get(prog, top);
        re__prog_inst_type inst_type;
        RE_ASSERT(top != 0);
        if (re__exec_thrdmin_set_ismemb(&exec->set_c, top)) {
            /* target is already a member */
            continue;
        } else {
            re__exec_thrdmin_set_add(&exec->set_c, top);
        }
        inst_type = re__prog_inst_get_type(inst);
        if (inst_type == RE__PROG_INST_TYPE_BYTE) {
            re__exec_thrdmin_set_add(target, top);
        } else if (inst_type == RE__PROG_INST_TYPE_BYTE_RANGE) {
            re__exec_thrdmin_set_add(target, top);
        } else if (inst_type == RE__PROG_INST_TYPE_SPLIT) {
            /* push secondary first to maintain priority */
            re__prog_loc secondary = re__prog_inst_get_split_secondary(inst);
            re__prog_loc primary = re__prog_inst_get_primary(inst);
            if ((err = re__exec_thrdmin_vec_push(&exec->thrd_stk, secondary))) {
                return err;
            }
            if ((err = re__exec_thrdmin_vec_push(&exec->thrd_stk, primary))) {
                return err;
            }
        } else if (inst_type == RE__PROG_INST_TYPE_MATCH) {
            re__exec_thrdmin_set_add(target, top);
        } else if (inst_type == RE__PROG_INST_TYPE_ASSERT) {
            if (assert_context & re__prog_inst_get_assert_ctx(inst)) {
                if ((err = re__exec_thrdmin_vec_push(&exec->thrd_stk, re__prog_inst_get_primary(inst)))) {
                    return err;
                }
            }
        } else {
            RE__ASSERT_UNREACHED();
        }
    }
    return 0;
}

RE_INTERNAL void re__exec_swap(re__exec* exec) {
    re__exec_thrdmin_set temp = exec->set_a;
    exec->set_a = exec->set_b;
    exec->set_b = temp;
}

RE_INTERNAL re_error re__exec_nfa(re__exec* exec, const re_char* str, re_size n) {
    re_error err = RE_ERROR_NONE;
    re__prog* prog = &exec->re->data->program;
    re__prog_loc set_size = re__prog_size(prog);
    re_uint32 i;
    const re_char* str_ptr = str;
    const re_char* str_end = str + n;
    re__ast_assert_type assert_ctx;
    /* Initialize sparse sets. */
    if ((err = re__exec_thrdmin_set_alloc(&exec->set_a, set_size))) {
        goto error;
    }
    if ((err = re__exec_thrdmin_set_alloc(&exec->set_b, set_size))) {
        goto error;
    }
    if ((err = re__exec_thrdmin_set_alloc(&exec->set_c, set_size))) {
        goto error;
    }
    assert_ctx = RE__AST_ASSERT_TYPE_TEXT_START_ABSOLUTE | RE__AST_ASSERT_TYPE_TEXT_START;
    if ((err = re__exec_nfa_follow_threads(exec, 1, &exec->set_a, assert_ctx))) {
        goto error;
    }
    while (str_ptr != str_end) {
        if (exec->set_a.n == 0) {
            RE_ASSERT(0);
        }
        assert_ctx = 0;
        for (i = 0; i < exec->set_a.n; i++) {
            re__prog_inst* inst = re__prog_get(prog, exec->set_a.dense[i]);
            re__prog_inst_type inst_type = re__prog_inst_get_type(inst);
            if (inst_type == RE__PROG_INST_TYPE_BYTE) {
                if (*str_ptr == re__prog_inst_get_byte(inst)) {
                    if ((err = re__exec_nfa_follow_threads(exec, re__prog_inst_get_primary(inst), &exec->set_b, assert_ctx))) {
                        goto error;
                    }
                }
            } else if (inst_type == RE__PROG_INST_TYPE_BYTE_RANGE) {
                if (*str_ptr >= re__prog_inst_get_byte_min(inst) && *str_ptr <= re__prog_inst_get_byte_max(inst)) {
                    if ((err = re__exec_nfa_follow_threads(exec, re__prog_inst_get_primary(inst), &exec->set_b, assert_ctx))) {
                        goto error;
                    }
                }
            } else if (inst_type == RE__PROG_INST_TYPE_MATCH) {
                /* do nothing */
            }
        }
        re__exec_swap(exec);
        re__exec_thrdmin_set_clear(&exec->set_b);
        str_ptr++;
    }
    re__exec_thrdmin_set_dump(&exec->set_a);
    return err;
error:
    re__exec_thrdmin_set_free(&exec->set_c);
    re__exec_thrdmin_set_free(&exec->set_b);
    re__exec_thrdmin_set_free(&exec->set_a);
    re__set_error_generic(exec->re, err);
    return err;
}

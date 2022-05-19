#include "re_internal.h"

MN_INTERNAL void re__compile_patches_init(re__compile_patches* patches) {
    patches->first_inst = RE__PROG_LOC_INVALID;
    patches->last_inst = RE__PROG_LOC_INVALID;
}

MN_INTERNAL void re__compile_patches_prepend(re__compile_patches* patches, re__prog* prog, re__prog_loc to, int secondary) {
    re__prog_loc out_val = to << 1;
    if (secondary) {
        out_val |= 1;
    }
    if (patches->first_inst == RE__PROG_LOC_INVALID) {
        patches->first_inst = out_val;
        patches->last_inst = out_val;
    } else {
        re__prog_inst* inst = re__prog_get(prog, to >> 1);
        if (!secondary) {
            re__prog_inst_set_primary(inst, patches->first_inst);
        } else {
            re__prog_inst_set_split_secondary(inst, patches->first_inst);
        }
        patches->first_inst = out_val;
    }
}

MN_INTERNAL void re__compile_patches_append(re__compile_patches* patches, re__prog* prog, re__prog_loc to, int secondary) {
    re__prog_loc out_val = to << 1;
    if (secondary) {
        out_val |= 1;
    }
    if (patches->first_inst == RE__PROG_LOC_INVALID) {
        patches->first_inst = out_val;
        patches->last_inst = out_val;
    } else {
        re__prog_inst* inst = re__prog_get(prog, patches->last_inst >> 1);
        if (!(patches->last_inst & 1)) {
            re__prog_inst_set_primary(inst, out_val);
        } else {
            re__prog_inst_set_split_secondary(inst, out_val);
        }
        patches->last_inst = out_val;
    }
}

MN_INTERNAL void re__compile_patches_merge(re__compile_patches* patches, re__prog* prog, re__compile_patches* merge_from) {
    re__prog_loc first_loc;
    re__prog_inst* inst;
    MN_ASSERT(merge_from->first_inst != RE__PROG_LOC_INVALID);
    if (patches->first_inst == RE__PROG_LOC_INVALID) {
        *patches = *merge_from;
        return;
    }
    first_loc = merge_from->first_inst;
    inst = re__prog_get(prog, patches->last_inst >> 1);
    if (!(patches->last_inst & 1)) {
        re__prog_inst_set_primary(inst, first_loc);
    } else {
        re__prog_inst_set_split_secondary(inst, first_loc);
    }
    patches->last_inst = merge_from->last_inst;
}

MN_INTERNAL void re__compile_patches_patch(re__compile_patches* patches, re__prog* prog, re__prog_loc to) {
    re__prog_loc current_loc = patches->first_inst;
    re__prog_inst* inst = re__prog_get(prog, current_loc >> 1);
    while (current_loc != patches->last_inst) {
        re__prog_loc next_loc;
        if (!(current_loc & 1)) {
            next_loc = re__prog_inst_get_primary(inst);
            re__prog_inst_set_primary(inst, to);
        } else {
            next_loc = re__prog_inst_get_split_secondary(inst);
            re__prog_inst_set_split_secondary(inst, to);
        }
        current_loc = next_loc;
        inst = re__prog_get(prog, current_loc >> 1);
    }
    if (!(current_loc & 1)) {
        re__prog_inst_set_primary(inst, to);
    } else {
        re__prog_inst_set_split_secondary(inst, to);
    }
}

#if RE_DEBUG

RE_VEC_IMPL_FUNC(re__prog_loc, init)
RE_VEC_IMPL_FUNC(re__prog_loc, destroy)
RE_VEC_IMPL_FUNC(re__prog_loc, push)
RE_VEC_IMPL_FUNC(re__prog_loc, size)
RE_VEC_IMPL_FUNC(re__prog_loc, get)

/* check for cycles in compile_patches */
MN_INTERNAL int re__compile_patches_verify(re__compile_patches* patches, re__prog* prog) {
    re__prog_loc current_loc = patches->first_inst;
    re__prog_loc_vec found_list;
    if (patches->first_inst == RE__PROG_LOC_INVALID) {
        return patches->last_inst == RE__PROG_LOC_INVALID;
    }
    re__prog_loc_vec_init(&found_list);
    /* O(n^2) so use with care! */
    while (current_loc != patches->last_inst) {
        mn_size i;
        re__prog_inst* inst = re__prog_get(prog, current_loc >> 1);
        for (i = 0; i < re__prog_loc_vec_size(&found_list); i++) {
            re__prog_loc found = re__prog_loc_vec_get(&found_list, i);
            if (found == current_loc) {
                return 0; /* cycle detected */
            }
        }
        re__prog_loc_vec_push(&found_list, current_loc);
        if (!(current_loc & 1)) {
            current_loc = re__prog_inst_get_primary(inst);
        } else {
            current_loc = re__prog_inst_get_split_secondary(inst);
        }
    }
    re__prog_loc_vec_destroy(&found_list);
    return 1;
}

MN_INTERNAL void re__compile_patches_dump(re__compile_patches* patches, re__prog* prog) {
    re__prog_loc current_loc = patches->first_inst;
    if (current_loc == RE__PROG_LOC_INVALID) {
        printf("<no patches>\n");
        return;
    }
    while (current_loc != patches->last_inst) {
        re__prog_loc next_loc;
        re__prog_inst* inst = re__prog_get(prog, current_loc >> 1);
        if (!(current_loc & 1)) {
            next_loc = re__prog_inst_get_primary(inst);
        } else {
            next_loc = re__prog_inst_get_split_secondary(inst);
        }
        printf("%04X | %s\n", current_loc >> 1, (current_loc & 1) ? "secondary" : "primary");
        current_loc = next_loc;
    }
    printf("%04X | %s\n", current_loc >> 1, (current_loc & 1) ? "secondary" : "primary");
}

#endif

MN_INTERNAL void re__compile_frame_init(re__compile_frame* frame, mn_int32 ast_base_ref, mn_int32 ast_child_ref, re__compile_patches patches, re__prog_loc start, re__prog_loc end) {
    frame->ast_base_ref = ast_base_ref;
    frame->ast_child_ref = ast_child_ref;
    frame->patches = patches;
    frame->start = start;
    frame->end = end;
    frame->rep_idx = 0;
}

MN_INTERNAL void re__compile_init(re__compile* compile) {
    compile->frames = NULL;
    compile->frames_size = 0;
    compile->frame_ptr = 0;
    re__compile_charclass_init(&compile->char_comp);
    /* set to NULL, re__compile_regex() sets ast_root */
    compile->ast_root = NULL;
    compile->should_push_child = 0;
    compile->should_push_child_ref = RE__AST_NONE;
    /* purposefully don't initialize returned_frame */
    /* compile->returned_frame; */
    compile->reversed = 0;
}

MN_INTERNAL void re__compile_destroy(re__compile* compile) {
    re__compile_charclass_destroy(&compile->char_comp);
}

MN_INTERNAL void re__compile_frame_push(re__compile* compile, re__compile_frame frame) {
    MN_ASSERT(compile->frame_ptr != compile->frames_size);
    compile->frames[compile->frame_ptr++] = frame;
}

MN_INTERNAL re__compile_frame re__compile_frame_pop(re__compile* compile) {
    MN_ASSERT(compile->frame_ptr != 0);
    return compile->frames[--compile->frame_ptr];
}

MN_INTERNAL int re__compile_gen_utf8(re_rune codep, mn_uint8* out_buf) {
    if (codep <= 0x7F) {
		out_buf[0] = codep & 0x7F;
        return 1;
	} else if (codep <= 0x07FF) {
		out_buf[0] = (mn_uint8)(((codep >>  6) & 0x1F) | 0xC0);
		out_buf[1] = (mn_uint8)(((codep >>  0) & 0x3F) | 0x80);
        return 2;
	} else if (codep <= 0xFFFF) {
		out_buf[0] = (mn_uint8)(((codep >> 12) & 0x0F) | 0xE0);
		out_buf[1] = (mn_uint8)(((codep >>  6) & 0x3F) | 0x80);
		out_buf[2] = (mn_uint8)(((codep >>  0) & 0x3F) | 0x80);
        return 3;
	} else if (codep <= 0x10FFFF) {
		out_buf[0] = (mn_uint8)(((codep >> 18) & 0x07) | 0xF0);
		out_buf[1] = (mn_uint8)(((codep >> 12) & 0x3F) | 0x80);
		out_buf[2] = (mn_uint8)(((codep >>  6) & 0x3F) | 0x80);
		out_buf[3] = (mn_uint8)(((codep >>  0) & 0x3F) | 0x80);
        return 4;
	} else {
        MN__ASSERT_UNREACHED();
        return 0;
	}
}

MN_INTERNAL re_error re__compile_do_rune(re__compile* compile, re__compile_frame* frame, const re__ast* ast, re__prog* prog) {
    /* Generates a single Byte or series of Byte instructions for a
     * UTF-8 codepoint. */
    /*    +0
     * ~~~+-------+~~~
     * .. | Byte  | ..
     * .. | Instr | ..
     * ~~~+---+---+~~~
     *        |
     *        +-----1--> ... */
    re_error err = RE_ERROR_NONE;
    re__prog_inst new_inst;
    mn_uint8 utf8_bytes[4];
    int num_bytes = re__compile_gen_utf8(re__ast_get_rune(ast), utf8_bytes);
    int i;
    MN__UNUSED(compile);
    for (i = 0; i < num_bytes; i++) {
        if (!compile->reversed) {
            re__prog_inst_init_byte(
                &new_inst, 
                utf8_bytes[i]
            );
        } else {
            re__prog_inst_init_byte(
                &new_inst, 
                utf8_bytes[(num_bytes - 1) - i]
            );
        }
        if (i == num_bytes - 1) {
            /* Add an outgoing patch (1) */
            re__compile_patches_append(&frame->patches, prog, re__prog_size(prog), 0);
        } else {
            re__prog_inst_set_primary(&new_inst, re__prog_size(prog) + 1);
        }
        if ((err = re__prog_add(prog, new_inst))) {
            return err;
        }
    }
    return err;
}

MN_INTERNAL re_error re__compile_do_str(re__compile* compile, re__compile_frame* frame, const re__ast* ast, re__prog* prog) {
    /* Generates a single Byte or series of Byte instructions for a
     * string. */
    /*    +0
     * ~~~+-------+~~~
     * .. | Byte  | ..
     * .. | Instr | ..
     * ~~~+---+---+~~~
     *        |
     *        +-----1--> ... */
    re_error err = RE_ERROR_NONE;
    re__prog_inst new_inst;
    mn__str_view str_view = re__ast_root_get_str_view(compile->ast_root, re__ast_get_str_ref(ast));
    mn_size i;
    mn_size sz = mn__str_view_size(&str_view);
    for (i = 0; i < sz; i++) {
        if (!compile->reversed) {
            re__prog_inst_init_byte(
                &new_inst, 
                (mn_uint8)mn__str_view_get_data(&str_view)[i]
            );
        } else {
            re__prog_inst_init_byte(
                &new_inst, 
                (mn_uint8)mn__str_view_get_data(&str_view)[(sz - 1) - i]
            );
        }
        if (i == sz - 1) {
            /* Add an outgoing patch (1) */
            re__compile_patches_append(&frame->patches, prog, re__prog_size(prog), 0);
        } else {
            re__prog_inst_set_primary(&new_inst, re__prog_size(prog) + 1);
        }
        if ((err = re__prog_add(prog, new_inst))) {
            return err;
        }
    }
    return err;
}

MN_INTERNAL re_error re__compile_do_charclass(re__compile* compile, re__compile_frame* frame, const re__ast* ast, re__prog* prog) {
    /* Generates a character class, which is a complex series of Byte
     * and Split instructions. */
    const re__charclass* charclass = re__ast_root_get_charclass(compile->ast_root,
        ast->_data.charclass_ref);
    return re__compile_charclass_gen(
        &compile->char_comp, 
        charclass,
        prog, &frame->patches, compile->reversed);
}

MN_INTERNAL re_error re__compile_do_concat(re__compile* compile, re__compile_frame* frame, const re__ast* ast, re__prog* prog) {
    /* Generates each child node, and patches them all together,
     * leaving the final child's outgoing branch targets unpatched. */
    /*    +0        +L(C0)          ...
     * ~~~+--.....--+--.....--+.....+--.....--+~~~
     * .. | Child 0 | Child 1 |.....| Child n | ..
     * .. | Instrs  | Instrs  |.....| Instrs  | ..
     * ~~~+--..+..--+--..+..+-+.....+--..+..+-+~~~
     *         |      ^  |  |         ^  |  |
     *         1      |  2  3         |  +--|----4-> ...
     *         |      |  |  |         |     |
     *         |      |  +--|---...---+     +----5-> ...
     *         +------+     |         |
     *                      +---...---+                 */
    /* Assert that there are children to compile.
     * Currently, we disallow concats with zero children. */ 
    MN_ASSERT(ast->first_child_ref != RE__AST_NONE);
    if (!compile->reversed) {
        if (frame->ast_child_ref == RE__AST_NONE) {
            /* Before first child */
            compile->should_push_child = 1;
            compile->should_push_child_ref = ast->first_child_ref;
            frame->ast_child_ref = ast->first_child_ref;
        } else {
            const re__ast* prev_child = re__ast_root_get_const(compile->ast_root, frame->ast_child_ref);
            if (prev_child->next_sibling_ref != RE__AST_NONE) {
                /* Patch outgoing branches (1, 2, 3) */
                re__compile_patches_patch(&compile->returned_frame.patches, prog, re__prog_size(prog));
                compile->should_push_child = 1;
                compile->should_push_child_ref = prev_child->next_sibling_ref;
                frame->ast_child_ref = prev_child->next_sibling_ref;
            } else {
                /* We are done */
                re__compile_patches_merge(&frame->patches, prog, &compile->returned_frame.patches);
            }
        }
    } else {
        if (frame->ast_child_ref == RE__AST_NONE) {
            /* Before *last* child */
            compile->should_push_child = 1;
            compile->should_push_child_ref = ast->last_child_ref;
            frame->ast_child_ref = ast->last_child_ref;
        } else {
            const re__ast* next_child = re__ast_root_get_const(compile->ast_root, frame->ast_child_ref);
            if (next_child->prev_sibling_ref != RE__AST_NONE) {
                re__compile_patches_patch(&compile->returned_frame.patches, prog, re__prog_size(prog));
                compile->should_push_child = 1;
                compile->should_push_child_ref = next_child->prev_sibling_ref;
                frame->ast_child_ref = next_child->prev_sibling_ref;
            } else {
                re__compile_patches_merge(&frame->patches, prog, &compile->returned_frame.patches);
            }
        }
    }
    return RE_ERROR_NONE; /* <- cool! */
}

MN_INTERNAL re_error re__compile_do_alt(re__compile* compile, re__compile_frame* frame, const re__ast* ast, re__prog* prog) {
    /* For each child node except for the last one, generates a SPLIT 
     * instruction, and then the instructions of the child.
     * Each split instruction's primary branch target is patched to the
     * next child, and the secondary branch targets are patched to the 
     * next split instruction. Leaves each child's outgoing branch
     * targets unpatched. */
    /*    +0      +1        +L(C0)+1                ...
     * ~~~+-------+--.....--+-------+--.....--+.....+--.....--+~~~
     * .. | Split | Child 0 | Split | Child 1 |.....| Child n | ..
     * .. | Instr | Instrs  | Instr | Instrs  |.....| Instrs  | ..
     * ~~~+---+-+-+--..+..+-+---+-+-+--..+..+-+.....+--..+..+-+~~~
     *        | |   ^  |  |   ^ | |   ^  |  |         ^  |  |
     *        | |   |  |  |   | | |   |  |  |         |  |  |
     *        +---1-+  |  |   | +---3-+  |  |         |  |  |
     *          |      |  |   |   |      |  |         |  |  |
     *          +-----------2-+   +-------------...-4-+  |  |
     *                 |  |              |  |            |  |
     *                 +------------------------...--------------5-> ...
     *                    |              |  |            |  |
     *                    +---------------------...--------------6-> ...
     *                                   |  |            |  |
     *                                   +------...--------------7-> ...
     *                                      |            |  |
     *                                      +---...--------------8-> ...
     *                                                   |  |
     *                                                   +-------9-> ...
     *                                                      |
     *                                                      +---10-> ...
     */
    re_error err = RE_ERROR_NONE;
    MN_ASSERT(ast->first_child_ref != RE__AST_NONE);
    /* Unlike concats, we still go through alterations in the correct order. */
    /* if (!compile->reversed || compile->reversed) { */
    if (frame->ast_child_ref == RE__AST_NONE) {
        /* Before *first* child */
        /* Initialize split instruction */
        re__prog_inst new_inst;
        re__prog_inst_init_split(
            &new_inst, 
            re__prog_size(prog) + 1, /* Outgoing branch (1) */
            RE__PROG_LOC_INVALID /* Will become outgoing branch (2) */
        );
        /* Add the Split instruction */
        if ((err = re__prog_add(prog, new_inst))) {
            return err;
        }
        compile->should_push_child = 1;
        compile->should_push_child_ref = ast->first_child_ref;
        frame->ast_child_ref = ast->first_child_ref;
    } else {
        const re__ast* prev_child = re__ast_root_get_const(compile->ast_root, frame->ast_child_ref);
        if (prev_child->next_sibling_ref != RE__AST_NONE) {
            /* Before intermediate children */
            /* Patch the secondary branch target of the old SPLIT
                * instruction. Corresponds to outgoing branch (2). */
            /* top.seg.end points to the instruction after the old split 
                * instruction, since we didn't set the endpoint before the 
                * first child. */
            re__prog_loc old_split_loc = frame->end - 1;
            re__prog_inst* old_inst = re__prog_get(prog, old_split_loc);
            const re__ast* child;
            /* Patch outgoing branch (2). */
            re__prog_inst_set_split_secondary(old_inst, re__prog_size(prog));
            /* Collect outgoing branches (5, 6, 7, 8). */
            re__compile_patches_merge(&frame->patches, prog, &compile->returned_frame.patches);
            child = re__ast_root_get_const(compile->ast_root, prev_child->next_sibling_ref);
            if (child->next_sibling_ref != RE__AST_NONE) {
                /* Before intermediate children and NOT last child */
                re__prog_inst new_inst;
                /* Create the intermediary SPLIT instruction, if there
                    * are more than two child nodes in the alternation. */
                re__prog_inst_init_split(
                    &new_inst, 
                    re__prog_size(prog) + 1, /* Outgoing branch (3) */
                    RE__PROG_LOC_INVALID /* Outgoing branch (4) */
                );
                /* Add it to the program. */
                if ((err = re__prog_add(prog, new_inst))) {
                    return err;
                }
            }
            compile->should_push_child = 1;
            compile->should_push_child_ref = prev_child->next_sibling_ref;
            frame->ast_child_ref = prev_child->next_sibling_ref;
        } else {
            /* After last child */
            /* Collect outgoing branches (9, 10). */
            re__compile_patches_merge(&frame->patches, prog, &compile->returned_frame.patches);
        }
    }
    return err;
}

MN_INTERNAL re_error re__compile_do_quantifier(re__compile* compile, re__compile_frame* frame, const re__ast* ast, re__prog* prog) {
    /*   *   min=0 max=INF */ /* Spl E -> 1 to 2 and out, 2 to 1 */
    /*   +   min=1 max=INF */ /* E Spl -> 1 to 2, 2 to 1 and out */
    /*   ?   min=0 max=1   */ /* Spl E -> 1 to 2 and out, 2 to out */
    /*  {n}  min=n max=n+1 */ /* E repeated -> out */
    /* {n, } min=n max=INF */ /* E repeated, Spl -> spl to last and out */
    /* {n,m} min=n max=m+1 */ /* E repeated, E Spl E Spl E*/
    /*   *   min=0 max=INF */
    /*   +   min=1 max=INF */ /* E Spl -> 1 to 2, 2 to 1 and out */
    /*   ?   min=0 max=1   */ /* Spl E -> 1 to 2 and out, 2 to out */
    /*  {n}  min=n max=n+1 */ /* E repeated -> out */
    /* {n, } min=n max=INF */ /* E repeated, Spl -> spl to last and out */
    /* {n,m} min=n max=m+1 */ /* E repeated, E Spl E Spl E*/
    re_error err = RE_ERROR_NONE;
    mn_int32 min = re__ast_get_quantifier_min(ast);
    mn_int32 max = re__ast_get_quantifier_max(ast);
    mn_int32 int_idx = frame->rep_idx;
    re__prog_loc this_start_pc = re__prog_size(prog);
    frame->rep_idx++;
    MN_ASSERT(ast->first_child_ref != RE__AST_NONE);
    MN_ASSERT(ast->first_child_ref == ast->last_child_ref);
    /* Quantifiers are the same when reversed */
    if (int_idx < min) {
        /* Generate child min times */
        if (int_idx > 0) {
            /* Patch previous */
            re__compile_patches_patch(&compile->returned_frame.patches, prog, this_start_pc);
        }
        compile->should_push_child = 1;
        compile->should_push_child_ref = ast->first_child_ref;
        frame->ast_child_ref = ast->first_child_ref;
    } else { /* int_idx >= min */
        if (max == RE__AST_QUANTIFIER_INFINITY) {
            re__prog_inst new_spl;
            if (min == 0) {
                if (int_idx == 0) {
                    if (re__ast_get_quantifier_greediness(ast)) {
                        re__prog_inst_init_split(&new_spl, this_start_pc + 1, RE__PROG_LOC_INVALID);
                        if ((err = re__prog_add(prog, new_spl))) {
                            return err;
                        }
                        re__compile_patches_append(&frame->patches, prog, this_start_pc, 1);
                    } else {
                        re__prog_inst_init_split(&new_spl, RE__PROG_LOC_INVALID, this_start_pc + 1);
                        if ((err = re__prog_add(prog, new_spl))) {
                            return err;
                        }
                        re__compile_patches_append(&frame->patches, prog, this_start_pc, 0);
                    }
                    compile->should_push_child = 1;
                    compile->should_push_child_ref = ast->first_child_ref;
                    frame->ast_child_ref = ast->first_child_ref;
                } else if (int_idx == 1) {
                    re__compile_patches_patch(&compile->returned_frame.patches, prog, frame->end - 1);
                }
            } else {
                re__compile_patches_patch(&compile->returned_frame.patches, prog, this_start_pc);
                if (re__ast_get_quantifier_greediness(ast)) {
                    re__prog_inst_init_split(&new_spl, compile->returned_frame.start, RE__PROG_LOC_INVALID);
                    if ((err = re__prog_add(prog, new_spl))) {
                        return err;
                    }
                    re__compile_patches_append(&frame->patches, prog, this_start_pc, 1);
                } else {
                    re__prog_inst_init_split(&new_spl, RE__PROG_LOC_INVALID, compile->returned_frame.start);
                    if ((err = re__prog_add(prog, new_spl))) {
                        return err;
                    }
                    re__compile_patches_append(&frame->patches, prog, this_start_pc, 0);
                }
            }
        } else {
            if (int_idx <= max - 1) {
                re__prog_inst new_spl;
                if (int_idx > 0) {
                    re__compile_patches_patch(&compile->returned_frame.patches, prog, this_start_pc);
                }
                if (re__ast_get_quantifier_greediness(ast)) {
                    re__prog_inst_init_split(&new_spl, this_start_pc + 1, RE__PROG_LOC_INVALID);
                    if ((err = re__prog_add(prog, new_spl))) {
                        return err;
                    }
                    re__compile_patches_append(&frame->patches, prog, this_start_pc, 1);
                } else {
                    re__prog_inst_init_split(&new_spl, RE__PROG_LOC_INVALID, this_start_pc + 1);
                    if ((err = re__prog_add(prog, new_spl))) {
                        return err;
                    }
                    re__compile_patches_append(&frame->patches, prog, this_start_pc, 0);
                }
                compile->should_push_child = 1;
                compile->should_push_child_ref = ast->first_child_ref;
                frame->ast_child_ref = ast->first_child_ref;
            } else {
                re__compile_patches_merge(&frame->patches, prog, &compile->returned_frame.patches);
            }
        }
    }
    return err;
}

MN_INTERNAL re_error re__compile_do_group(re__compile* compile, re__compile_frame* frame, const re__ast* ast, re__prog* prog) {
    re_error err = RE_ERROR_NONE;
    re__prog_inst new_inst;
    mn_uint32 group_idx;
    re__ast_group_flags group_flags = re__ast_get_group_flags(ast);
    MN_ASSERT(ast->first_child_ref != RE__AST_NONE);
    MN_ASSERT(ast->first_child_ref == ast->last_child_ref);
    if (frame->ast_child_ref == RE__AST_NONE) {
        /* Before child */
        if (!(group_flags & RE__AST_GROUP_FLAG_NONMATCHING)) {
            group_idx = re__ast_get_group_idx(ast);
            if (!compile->reversed) {
                re__prog_inst_init_save(&new_inst, group_idx * 2);
            } else {
                re__prog_inst_init_save(&new_inst, group_idx * 2 + 1);
            }
            re__prog_inst_set_primary(&new_inst, re__prog_size(prog) + 1);
            if ((err = re__prog_add(prog, new_inst))) {
                return err;
            }
        }
        compile->should_push_child = 1;
        compile->should_push_child_ref = ast->first_child_ref;
        frame->ast_child_ref = ast->first_child_ref;
    } else {
        /* After child */
        if (!(group_flags & RE__AST_GROUP_FLAG_NONMATCHING)) {
            re__prog_loc save_pc = re__prog_size(prog);
            group_idx = re__ast_get_group_idx(ast);
            if (!compile->reversed) {
                re__prog_inst_init_save(&new_inst, group_idx * 2 + 1);
            } else {
                re__prog_inst_init_save(&new_inst, group_idx * 2);
            }
            if ((err = re__prog_add(prog, new_inst))) {
                return err;
            }
            re__compile_patches_patch(&compile->returned_frame.patches, prog, save_pc);
            re__compile_patches_append(&frame->patches, prog, save_pc, 0);
        } else {
            re__compile_patches_merge(&frame->patches, prog, &compile->returned_frame.patches);
        }
    }
    return err;
}

MN_INTERNAL re_error re__compile_do_assert(re__compile* compile, re__compile_frame* frame, const re__ast* ast, re__prog* prog) {
    /* Generates a single Assert instruction. */
    /*    +0
     * ~~~+-------+~~~
     * .. |  Ass  | ..
     * .. | Instr | ..
     * ~~~+---+---+~~~
     *        |
     *        +-----1--> ... */
    re_error err = RE_ERROR_NONE;
    re__prog_inst new_inst;
    re__prog_loc assert_pc = re__prog_size(prog);
    re__assert_type assert_type = re__ast_get_assert_type(ast);
    MN__UNUSED(compile);
    if (compile->reversed) {
        re__assert_type new_assert_type = 0;
        if (assert_type & RE__ASSERT_TYPE_TEXT_START) {
            new_assert_type |= RE__ASSERT_TYPE_TEXT_END;
        }
        if (assert_type & RE__ASSERT_TYPE_TEXT_END) {
            new_assert_type |= RE__ASSERT_TYPE_TEXT_START;
        }
        if (assert_type & RE__ASSERT_TYPE_TEXT_START_ABSOLUTE) {
            new_assert_type |= RE__ASSERT_TYPE_TEXT_END_ABSOLUTE;
        }
        if (assert_type & RE__ASSERT_TYPE_TEXT_END_ABSOLUTE) {
            new_assert_type |= RE__ASSERT_TYPE_TEXT_START_ABSOLUTE;
        }
        /* word boundaries are symmetric */
        if (assert_type & RE__ASSERT_TYPE_WORD) {
            new_assert_type |= RE__ASSERT_TYPE_WORD;
        }
        if (assert_type & RE__ASSERT_TYPE_WORD_NOT) {
            new_assert_type |= RE__ASSERT_TYPE_WORD_NOT;
        }
        assert_type = new_assert_type;
    }
    re__prog_inst_init_assert(
        &new_inst,
        (mn_uint32)assert_type
    ); /* Creates unpatched branch (1) */
    if ((err = re__prog_add(prog, new_inst))) {
        return err;
    }
    /* Add an outgoing patch (1) */
    re__compile_patches_append(&frame->patches, prog, assert_pc, 0);
    return err;
}

MN_INTERNAL re_error re__compile_do_any_char(re__compile* compile, re__compile_frame* frame, const re__ast* ast, re__prog* prog) {
    /* Generates a sequence of instructions corresponding to a single
     * UTF-8 codepoint. */
    re__prog_data_id id = RE__PROG_DATA_ID_DOT_FWD_REJSURR;
    MN__UNUSED(ast);
    if (compile->reversed) {
        id = RE__PROG_DATA_ID_DOT_REV_REJSURR;
    }
return re__prog_data_decompress(prog, re__prog_data[id], re__prog_data_size[id], &frame->patches);
}

MN_INTERNAL re_error re__compile_do_any_byte(re__compile* compile, re__compile_frame* frame, const re__ast* ast, re__prog* prog) {
    /* Generates a single Byte Range instruction. */
    /*    +0
     * ~~~+-------+~~~
     * .. | ByteR | ..
     * .. | Instr | ..
     * ~~~+---+---+~~~
     *        |
     *        +-----1--> ... */
    re_error err = RE_ERROR_NONE;
    re__prog_inst new_inst;
    re__prog_loc byte_range_pc = re__prog_size(prog);
    re__byte_range br;
    MN__UNUSED(compile);
    MN__UNUSED(ast);
    br.min = 0;
    br.max = 255;
    re__prog_inst_init_byte_range(
        &new_inst,
        br
    ); /* Creates unpatched branch (1) */
    if ((err = re__prog_add(prog, new_inst))) {
        return err;
    }
    /* Add an outgoing patch (1) */
    re__compile_patches_append(&frame->patches, prog, byte_range_pc, 0);
    return err;
}

MN_INTERNAL re_error re__compile_regex(re__compile* compile, const re__ast_root* ast_root, re__prog* prog, int reversed) {
    re_error err = RE_ERROR_NONE;
    re__compile_frame initial_frame;
    re__compile_patches initial_patches;
    re__prog_inst fail_inst;
    re__prog_loc entry;
    /* Set ast_root */
    compile->ast_root = ast_root;
    /* Set reversed flag, more economical to set here */
    compile->reversed = reversed;
    /* Allocate memory for frames */
    /* depth_max + 1 because we include an extra frame for terminals within the
     * deepest multi-child node */
    compile->frames_size = ast_root->depth_max;
    compile->frames = (re__compile_frame*)MN_MALLOC((sizeof(re__compile_frame)*((mn_size)compile->frames_size)));
    if (compile->frames == MN_NULL) {
        err = RE_ERROR_NOMEM;
        goto error;
    }
    MN_ASSERT(re__prog_get_entry(prog, RE__PROG_ENTRY_DEFAULT) == RE__PROG_LOC_INVALID);
    /* Add the FAIL instruction to the program */
    re__prog_inst_init_fail(&fail_inst);
    if ((err = re__prog_add(prog, fail_inst))) {
        goto error;
    }
    entry = re__prog_size(prog);
    re__compile_patches_init(&initial_patches);
    /* Start first frame */
    re__compile_frame_init(&initial_frame, ast_root->root_ref, RE__AST_NONE, initial_patches, 0, 0);
    /* Push it */
    re__compile_frame_push(compile, initial_frame);
    /* While there are nodes left to compile... */
    while (compile->frame_ptr != 0) {
        re__compile_frame top_frame = re__compile_frame_pop(compile);
        const re__ast* top_node = re__ast_root_get_const(ast_root, top_frame.ast_base_ref);
        re__ast_type top_node_type = top_node->type;
        compile->should_push_child = 0;
        compile->should_push_child_ref = top_frame.ast_child_ref;
        switch (top_node_type) {
            case RE__AST_TYPE_RUNE:
                err = re__compile_do_rune(compile, &top_frame, top_node, prog);
                break;
            case RE__AST_TYPE_STR:
                err = re__compile_do_str(compile, &top_frame, top_node, prog);
                break;
            case RE__AST_TYPE_CHARCLASS:
                err = re__compile_do_charclass(compile, &top_frame, top_node, prog);
                break;
            case RE__AST_TYPE_CONCAT:
                err = re__compile_do_concat(compile, &top_frame, top_node, prog);
                break;
            case RE__AST_TYPE_ALT:
                err = re__compile_do_alt(compile, &top_frame, top_node, prog);
                break;
            case RE__AST_TYPE_QUANTIFIER:
                err = re__compile_do_quantifier(compile, &top_frame, top_node, prog);
                break;
            case RE__AST_TYPE_GROUP:
                err = re__compile_do_group(compile, &top_frame, top_node, prog);
                break;
            case RE__AST_TYPE_ASSERT:
                err = re__compile_do_assert(compile, &top_frame, top_node, prog);
                break;
            case RE__AST_TYPE_ANY_CHAR:
                err = re__compile_do_any_char(compile, &top_frame, top_node, prog);
                break;
            case RE__AST_TYPE_ANY_BYTE:
                err = re__compile_do_any_byte(compile, &top_frame, top_node, prog);
                break;
            default:
                MN__ASSERT_UNREACHED();
        }
        if (err) {
            goto error;
        }
        /* Set the end of the segment to the next instruction */
        top_frame.end = re__prog_size(prog);
        if (compile->should_push_child) {
            re__compile_frame up_frame;
            re__compile_patches up_patches;

            MN_ASSERT(compile->should_push_child_ref != RE__AST_NONE);
            re__compile_frame_push(compile, top_frame);
            /* Prepare the child's patches */
            re__compile_patches_init(&up_patches);
            /* Prepare the child's stack frame */
            re__compile_frame_init(
                &up_frame,
                compile->should_push_child_ref,
                RE__AST_NONE,
                up_patches,
                top_frame.end,
                top_frame.end
            );
            re__compile_frame_push(compile, up_frame);
        }
        compile->returned_frame = top_frame;
    }
    /* There should be no more frames. */
    MN_ASSERT(compile->frame_ptr == 0);
    /* Link the returned patches to a final MATCH instruction. */
    re__compile_patches_patch(&compile->returned_frame.patches, prog, re__prog_size(prog));
    {
        re__prog_inst match_inst;
        re__prog_inst_init_match(&match_inst, 1);
        if ((err = re__prog_add(prog, match_inst))) {
            goto error;
        }
    }
    MN_FREE(compile->frames);
    compile->frames = MN_NULL;
    re__prog_set_entry(prog, RE__PROG_ENTRY_DEFAULT, entry);
    return err;
error:
    if (compile->frames != MN_NULL) {
        MN_FREE(compile->frames);
    }
    compile->frames = MN_NULL;
    /*
    if (err == RE__ERROR_PROGMAX) {
        re__str err_str;
        if ((err = re__str_init_s(&err_str, "compiled program length exceeds maximum of " RE__STRINGIFY(RE__PROG_SIZE_MAX) " instructions"))) {
            re__set_error_generic(compile->re, err);
            return err;
        }
        re__set_error_str(compile->re, &err_str);
        err = RE_ERROR_COMPILE;
        re__str_destroy(&err_str);
    }
    if (err == RE_ERROR_COMPILE) {
        MN_ASSERT(re__str_size(&compile->re->data->error_string));
    } else {
        re__set_error_generic(compile->re, err);
    }*/
    return err;
}

MN_INTERNAL re_error re__compile_dotstar(re__prog* prog, int reversed) {
    re__compile_patches patches;
    re__prog_loc entry = re__prog_size(prog);
    re__prog_data_id id = RE__PROG_DATA_ID_DOT_FWD_REJSURR;
    re__prog_inst inst;
    re_error err = RE_ERROR_NONE;
    MN_ASSERT(re__prog_get_entry(prog, RE__PROG_ENTRY_DOTSTAR) == RE__PROG_LOC_INVALID);
    if (reversed) {
        id = RE__PROG_DATA_ID_DOT_REV_REJSURR;
    }
    re__prog_inst_init_split(&inst, re__prog_get_entry(prog, RE__PROG_ENTRY_DEFAULT), entry + 1);
    if ((err = re__prog_add(prog, inst))) {
        return err;
    }
    re__compile_patches_init(&patches);
    if ((err = re__prog_data_decompress(prog, re__prog_data[id], re__prog_data_size[id], &patches))) {
        return err;
    }
    re__compile_patches_patch(&patches, prog, entry);
    re__prog_set_entry(prog, RE__PROG_ENTRY_DOTSTAR, entry);
    return err;
}

#if RE_DEBUG

void re__compile_debug_dump(re__compile* compile) {
    mn_int32 i;
    printf("%u frames / %u frames:\n", (mn_uint32)compile->frame_ptr, (mn_uint32)compile->frames_size);
    for (i = 0; i < compile->frames_size; i++) {
        re__compile_frame* cur_frame = &compile->frames[i];
        printf("  Frame %u:\n", i);
        printf("    AST root reference: %i\n", cur_frame->ast_base_ref);
        printf("    AST child reference: %i\n", cur_frame->ast_child_ref);
        printf("    Start loc: %u\n", cur_frame->start);
        printf("    End loc: %u\n", cur_frame->end);
    }
}

#endif

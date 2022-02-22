#include "re_internal.h"

RE_INTERNAL void re__compile_patches_init(re__compile_patches* patches) {
    patches->first_inst = RE__PROG_LOC_INVALID;
    patches->last_inst = RE__PROG_LOC_INVALID;
}

RE_INTERNAL void re__compile_patches_prepend(re__compile_patches* patches, re__prog* prog, re__prog_loc to, int secondary) {
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

RE_INTERNAL void re__compile_patches_append(re__compile_patches* patches, re__prog* prog, re__prog_loc to, int secondary) {
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

RE_INTERNAL void re__compile_patches_merge(re__compile_patches* patches, re__prog* prog, re__compile_patches* merge_from) {
    re__prog_loc first_loc;
    re__prog_inst* inst;
    RE_ASSERT(merge_from->first_inst != RE__PROG_LOC_INVALID);
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

RE_INTERNAL void re__compile_patches_patch(re__compile_patches* patches, re__prog* prog, re__prog_loc to) {
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

RE_INTERNAL void re__compile_patches_dump(re__compile_patches* patches, re__prog* prog) {
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

RE_INTERNAL void re__compile_frame_init(re__compile_frame* frame, re_int32 ast_root_ref, re_int32 ast_child_ref, re__compile_patches patches, re__prog_loc start, re__prog_loc end) {
    frame->ast_root_ref = ast_root_ref;
    frame->ast_child_ref = ast_child_ref;
    frame->patches = patches;
    frame->start = start;
    frame->end = end;
    frame->rep_idx = 0;
}

RE_INTERNAL void re__compile_init(re__compile* compile, re* re) {
    compile->re = re;
    compile->frames = NULL;
    compile->frames_size = 0;
    compile->frame_ptr = 0;
    compile->ast_ref = 0;
    re__compile_charclass_init(&compile->char_comp);
}

RE_INTERNAL void re__compile_destroy(re__compile* compile) {
    re__compile_charclass_destroy(&compile->char_comp);
}

RE_INTERNAL void re__compile_frame_push(re__compile* compile, re__compile_frame frame) {
    RE_ASSERT(compile->frame_ptr != compile->frames_size);
    compile->frames[compile->frame_ptr++] = frame;
}

RE_INTERNAL re__compile_frame re__compile_frame_pop(re__compile* compile) {
    RE_ASSERT(compile->frame_ptr != 0);
    return compile->frames[--compile->frame_ptr];
}

RE_INTERNAL int re__compile_gen_utf8(re_rune codep, re_uint8* out_buf) {
    if (codep <= 0x7F) {
		out_buf[0] = codep & 0x7F;
        return 1;
	} else if (codep <= 0x07FF) {
		out_buf[0] = ((codep >>  6) & 0x1F) | 0xC0;
		out_buf[1] = ((codep >>  0) & 0x3F) | 0x80;
        return 2;
	} else if (codep <= 0xFFFF) {
		out_buf[0] = ((codep >> 12) & 0x0F) | 0xE0;
		out_buf[1] = ((codep >>  6) & 0x3F) | 0x80;
		out_buf[2] = ((codep >>  0) & 0x3F) | 0x80;
        return 3;
	} else if (codep <= 0x10FFFF) {
		out_buf[0] = ((codep >> 18) & 0x07) | 0xF0;
		out_buf[1] = ((codep >> 12) & 0x3F) | 0x80;
		out_buf[2] = ((codep >>  6) & 0x3F) | 0x80;
		out_buf[3] = ((codep >>  0) & 0x3F) | 0x80;
        return 4;
	} else {
        RE__ASSERT_UNREACHED();
        return 0;
	}
}

RE_INTERNAL re_error re__compile_regex(re__compile* compile) {
    re_error err = RE_ERROR_NONE;
    re__compile_frame initial_frame;
    re__compile_patches initial_patches;
    re__ast_root* ast_root = &compile->re->data->ast_root;
    re__prog* prog = &compile->re->data->program;
    re__compile_frame returned_frame;
    re__prog_inst fail_inst;
    /* Allocate memory for frames */
    /* depth_max + 1 because we include an extra frame for terminals within the
     * deepest multi-child node */
    compile->frames_size = compile->re->data->parse.depth_max + 1;
    compile->frames = (re__compile_frame*)RE_MALLOC((sizeof(re__compile_frame)*((re_size)compile->frames_size)));
    if (compile->frames == RE_NULL) {
        err = RE_ERROR_NOMEM;
        goto error;
    }
    /* Add the FAIL instruction to the program */
    re__prog_inst_init_fail(&fail_inst);
    if ((err = re__prog_add(prog, fail_inst))) {
        goto error;
    }
    re__compile_patches_init(&initial_patches);
    /* Start first frame */
    re__compile_frame_init(&initial_frame, 0, 0, initial_patches, 0, 0);
    /* Push it */
    re__compile_frame_push(compile, initial_frame);
    /* While there are nodes left to compile... */
    while (compile->frame_ptr != 0) {
        re__compile_frame top_frame = re__compile_frame_pop(compile);
        re__ast* top_node = re__ast_root_get(ast_root, top_frame.ast_root_ref);
        re__ast_type top_node_type = top_node->type;
        const re__prog_loc this_start_pc = re__prog_size(prog);
        int push_child = 0;
        int next_child = 0;
        if (top_node_type == RE__AST_TYPE_RUNE) {
            /* Generates a single Byte or series of Byte instructions for a
             * UTF-8 codepoint. */
            /*    +0
             * ~~~+-------+~~~
             * .. | Byte  | ..
             * .. | Instr | ..
             * ~~~+---+---+~~~
             *        |
             *        +-----1--> ... */
            re__prog_inst new_inst;
            re_uint8 utf8_bytes[4];
            int num_bytes = re__compile_gen_utf8(re__ast_get_rune(top_node), utf8_bytes);
            int i;
            for (i = 0; i < num_bytes; i++) {
                if (i == num_bytes - 1) {
                    /* Add an outgoing patch (1) */
                    re__compile_patches_append(&top_frame.patches, prog, re__prog_size(prog), 0);
                }
                re__prog_inst_init_byte(
                    &new_inst, 
                    utf8_bytes[i]
                );
                if ((err = re__prog_add(prog, new_inst))) {
                    goto error;
                }
            }
            next_child = 1;
        } else if (top_node_type == RE__AST_TYPE_CHARCLASS) {
            /* Generates a character class, which is a complex series of Byte
             * and Split instructions. */
            const re__charclass* charclass = re__ast_root_get_charclass(ast_root,
                top_node->_data.charclass_ref);
            if ((err = re__compile_charclass_gen(
                &compile->char_comp, 
                charclass,
                prog, &top_frame.patches))) {
                goto error;
            }
            next_child = 1;
        } else if (top_node_type == RE__AST_TYPE_CONCAT) {
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
            RE_ASSERT(top_node->first_child_ref != RE__AST_NONE);
            if (top_frame.ast_child_ref != RE__AST_NONE) {
                re__ast* child = re__ast_root_get(ast_root, top_frame.ast_child_ref);
                if (child->prev_sibling_ref == RE__AST_NONE) {
                    /* Before first child */
                    push_child = 1;
                    compile->ast_ref = child->next_sibling_ref;
                } else {
                    /* Patch outgoing branches (1, 2, 3) */
                    re__compile_patches_patch(&returned_frame.patches, prog, this_start_pc);
                    /* There are children left to check */
                    push_child = 1;
                    compile->ast_ref = child->next_sibling_ref;
                }
            } else {
                /* After last child */
                /* Collect outgoing branches (4, 5) */
                re__compile_patches_merge(&top_frame.patches, prog, &returned_frame.patches);
                next_child = 1;
            }
        } else if (top_node_type == RE__AST_TYPE_ALT) {
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
            RE_ASSERT(top_node->first_child_ref != RE__AST_NONE);
            if (top_frame.ast_child_ref != RE__AST_NONE) {
                re__ast* child = re__ast_root_get(ast_root, top_frame.ast_child_ref);
                if (child->prev_sibling_ref == RE__AST_NONE) {
                    /* Before first child */
                    /* Initialize split instruction */
                    re__prog_inst new_inst;
                    re__prog_inst_init_split(
                        &new_inst, 
                        this_start_pc + 1, /* Outgoing branch (1) */
                        RE__PROG_LOC_INVALID /* Will become outgoing branch (2) */
                    );
                    /* Add the Split instruction */
                    if ((err = re__prog_add(prog, new_inst))) {
                        goto error;
                    }
                    compile->ast_ref = top_node->next_sibling_ref;
                } else {
                    /* Before intermediate children and last child */
                    /* Patch the secondary branch target of the old SPLIT
                     * instruction. Corresponds to outgoing branch (2). */
                    /* top.seg.end points to the instruction after the old split 
                     * instruction, since we didn't set the endpoint before the 
                     * first child. */
                    re__prog_loc old_split_loc = top_frame.end - 1;
                    re__prog_inst* old_inst = re__prog_get(prog, old_split_loc);
                    /* Patch outgoing branch (2). */
                    re__prog_inst_set_split_secondary(old_inst, this_start_pc);
                    /* Collect outgoing branches (5, 6, 7, 8). */
                    re__compile_patches_merge(&top_frame.patches, prog, &returned_frame.patches);
                    if (child->next_sibling_ref != RE__AST_NONE) {
                        /* Before intermediate children and NOT last child */
                        re__prog_inst new_inst;
                        /* Create the intermediary SPLIT instruction, if there
                         * are more than two child nodes in the alternation. */
                        re__prog_inst_init_split(
                            &new_inst, 
                            this_start_pc + 1, /* Outgoing branch (3) */
                            RE__PROG_LOC_INVALID /* Outgoing branch (4) */
                        );
                        /* Add it to the program. */
                        if ((err = re__prog_add(prog, new_inst))) {
                            goto error;
                        }
                    }
                }
                /* Before every child, including the first and last one */
                push_child = 1;
            } else {
                /* After last child */
                /* Collect outgoing branches (9, 10). */
                re__compile_patches_merge(&top_frame.patches, prog, &returned_frame.patches);
            }
        } else if (top_node_type == RE__AST_TYPE_QUANTIFIER) {
            /*   *   min=0 max=INF */ /* Spl E -> 1 to 2 and out, 2 to 1 */
            /*   +   min=1 max=INF */ /* E Spl -> 1 to 2, 2 to 1 and out */
            /*   ?   min=0 max=1   */ /* Spl E -> 1 to 2 and out, 2 to out */
            /*  {n}  min=n max=n+1 */ /* E repeated -> out */
            /* {n, } min=n max=INF */ /* E repeated, Spl -> spl to last and out */
            /* {n,m} min=n max=m+1 */ /* E repeated, E Spl E Spl E*/
            re_int32 min = re__ast_get_quantifier_min(top_node);
            re_int32 max = re__ast_get_quantifier_max(top_node);
            re_int32 int_idx = top_frame.rep_idx;
            top_frame.rep_idx++;
            if (int_idx < min) {
                /* Generate child min times */
                if (int_idx > 0) {
                    /* Patch previous */
                    re__compile_patches_patch(&returned_frame.patches, prog, this_start_pc);
                }
                push_child = 1;
                compile->ast_ref = top_node->first_child_ref;
            } else { /* int_idx >= min */
                if (max == RE__AST_QUANTIFIER_INFINITY) {
                    re__prog_inst new_spl;
                    if (min == 0) {
                        if (int_idx == 0) {
                            re__prog_inst_init_split(&new_spl, this_start_pc + 1, RE__PROG_LOC_INVALID);
                            if ((err = re__prog_add(prog, new_spl))) {
                                goto error;
                            }
                            re__compile_patches_append(&top_frame.patches, prog, this_start_pc, 1);
                            push_child = 1;
                            compile->ast_ref = top_node->first_child_ref;
                        } else if (int_idx == 1) {
                            re__compile_patches_patch(&returned_frame.patches, prog, top_frame.end - 1);
                        }
                    } else {
                        re__compile_patches_patch(&returned_frame.patches, prog, this_start_pc);
                        re__prog_inst_init_split(&new_spl, returned_frame.start, RE__PROG_LOC_INVALID);
                        if ((err = re__prog_add(prog, new_spl))) {
                            goto error;
                        }
                        re__compile_patches_append(&top_frame.patches, prog, this_start_pc, 1);
                    }
                } else {
                    if (int_idx > 0) {
                        re__compile_patches_patch(&returned_frame.patches, prog, this_start_pc);
                    }
                    if (int_idx <= max - 2) {
                        re__prog_inst new_spl;
                        re__prog_inst_init_split(&new_spl, this_start_pc + 1, RE__PROG_LOC_INVALID);
                        if ((err = re__prog_add(prog, new_spl))) {
                            goto error;
                        }
                        re__compile_patches_append(&top_frame.patches, prog, this_start_pc, 1);
                        push_child = 1;
                        compile->ast_ref = top_node->first_child_ref;
                    } else {
                        re__compile_patches_merge(&top_frame.patches, prog, &returned_frame.patches);
                    }
                }
            }
        } else if (top_node_type == RE__AST_TYPE_GROUP) {
            RE_ASSERT(top_node->first_child_ref != RE__AST_NONE);
            if (top_frame.ast_child_ref != RE__AST_NONE) {
                /* Before child */
                push_child = 1;
                next_child = 1;
                compile->ast_ref = top_node->first_child_ref;
            } else {
                /* After child */
                re__compile_patches_merge(&top_frame.patches, prog, &returned_frame.patches);
            }
        } else if (top_node_type == RE__AST_TYPE_ASSERT) {
            /* Generates a single Assert instruction. */
            /*    +0
             * ~~~+-------+~~~
             * .. |  Ass  | ..
             * .. | Instr | ..
             * ~~~+---+---+~~~
             *        |
             *        +-----1--> ... */
            re__prog_inst new_inst;
            re__prog_inst_init_assert(
                &new_inst,
                (re_uint32)re__ast_get_assert_type(top_node)
            ); /* Creates unpatched branch (1) */
            if ((err = re__prog_add(prog, new_inst))) {
                goto error;
            }
            /* Add an outgoing patch (1) */
            re__compile_patches_append(&top_frame.patches, prog, this_start_pc, 0);
            next_child = 1;
        } else if (top_node_type == RE__AST_TYPE_ANY_CHAR) {
            /* Generates a single Byte Range instruction. */
            /*    +0
             * ~~~+-------+~~~
             * .. | ByteR | ..
             * .. | Instr | ..
             * ~~~+---+---+~~~
             *        |
             *        +-----1--> ... */
            re__prog_inst new_inst;
            re__byte_range br;
            br.min = 0;
            br.max = 255;
            re__prog_inst_init_byte_range(
                &new_inst,
                br
            ); /* Creates unpatched branch (1) */
            if ((err = re__prog_add(prog, new_inst))) {
                goto error;
            }
            /* Add an outgoing patch (1) */
            re__compile_patches_append(&top_frame.patches, prog, this_start_pc, 0);
            next_child = 1;
        } else {
            RE__ASSERT_UNREACHED();
        }
        /* Set the end of the segment to the next instruction */
        top_frame.end = re__prog_size(prog);
        if (next_child) {
            re__ast* child = re__ast_root_get(ast_root, top_frame.ast_child_ref);
            /* Increment the child index on the top frame before we push
             * it again */
            top_frame.ast_child_ref = child->next_sibling_ref;
        }
        if (push_child) {
            re__compile_frame up_frame;
            re__compile_patches up_patches;
            RE_ASSERT(compile->ast_ref != RE__AST_NONE);
            re__compile_frame_push(compile, top_frame);
            /* Prepare the child's patches */
            re__compile_patches_init(&up_patches);
            /* Prepare the child's stack frame */
            re__compile_frame_init(
                &up_frame,
                compile->ast_ref,
                0, /* Start at first child *of* child */
                up_patches,
                top_frame.end,
                top_frame.end
            );
            re__compile_frame_push(compile, up_frame);
        }
        returned_frame = top_frame;
    }
    /* There should be no more frames. */
    RE_ASSERT(compile->frame_ptr == 0);
    /* Link the returned patches to a final MATCH instruction. */
    re__compile_patches_patch(&returned_frame.patches, prog, re__prog_size(prog));
    {
        re__prog_inst match_inst;
        re__prog_inst_init_match(&match_inst, 0);
        if ((err = re__prog_add(prog, match_inst))) {
            goto error;
        }
    }
    RE_FREE(compile->frames);
    compile->frames = NULL;
    re__prog_debug_dump(&compile->re->data->program);
    return err;
error:
    if (compile->frames != RE_NULL) {
        RE_FREE(compile->frames);
    }
    compile->frames = NULL;
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
        RE_ASSERT(re__str_size(&compile->re->data->error_string));
    } else {
        re__set_error_generic(compile->re, err);
    }
    return err;
}

#if RE_DEBUG

void re__compile_debug_dump(re__compile* compile) {
    re_int32 i;
    printf("%u frames / %u frames:\n", (re_uint32)compile->frame_ptr, (re_uint32)compile->frames_size);
    for (i = 0; i < compile->frames_size; i++) {
        re__compile_frame* cur_frame = &compile->frames[i];
        printf("  Frame %u:\n", i);
        printf("    AST root reference: %i\n", cur_frame->ast_root_ref);
        printf("    AST child reference: %i\n", cur_frame->ast_child_ref);
        printf("    Start loc: %u\n", cur_frame->start);
        printf("    End loc: %u\n", cur_frame->end);
    }
}

#endif

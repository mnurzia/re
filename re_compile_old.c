#include "re_internal.h"

/* Patch structure. */
typedef struct re__compile_patch {
    /* Instruction in program to patch */
    re__prog_loc _loc;
    /* Whether or not to patch the secondary jump in the instruction */
    int _secondary;
} re__compile_patch;

RE_INTERNAL void re__compile_patch_init(re__compile_patch* patch, re__prog_loc loc, int secondary) {
    patch->_loc = loc;
    patch->_secondary = secondary;
}

RE_INTERNAL re__prog_loc re__compile_patch_get_loc(re__compile_patch* patch) {
    return patch->_loc;
}

RE_INTERNAL int re__compile_patch_get_secondary(re__compile_patch* patch) {
    return patch->_secondary;
}

RE_VEC_DECL(re__compile_patch);
RE_VEC_IMPL_FUNC(re__compile_patch, init)
RE_VEC_IMPL_FUNC(re__compile_patch, destroy)
RE_VEC_IMPL_FUNC(re__compile_patch, push)
RE_VEC_IMPL_FUNC(re__compile_patch, pop)
RE_VEC_IMPL_FUNC(re__compile_patch, cat)
RE_VEC_IMPL_FUNC(re__compile_patch, get)
RE_VEC_IMPL_FUNC(re__compile_patch, size)

/* Patch memoizer structure -- points to a range of instructions within the
 * program and maintains a list of which of those instructions have outgoing 
 * "arrows" that will point to the next constructed instruction. */
typedef struct re__compile_seg {
    /* Beginning and end of segment (segment contains [start, end) ) */
    re__prog_loc start;
    re__prog_loc end;
    /* List of pending patches */
    re__compile_patch_vec _patches;
} re__compile_seg;

/* Think of this structure as a "window" over the program that remembers which
 * instructions don't have a jump destination. */

/* seg.start               seg.end
 *    |                       |
 *    V       +1      +2      V
 * ~~~+-------+-------+-------+~~~~~~~~~~~~~~~~~~~~~~~~~~
 * .. | Char  | Char  | Split | .. unfinished program ..
 * .. | Instr | Instr | Instr | ........................
 * ~~~+-------+-------+---+-+-+~~~~~~~~~~~~~~~~~~~~~~~~~~
 *                        | |
 *                        | |     +-------------------------+
 *                        +-----> | primary split branch    |
 *                          |     +-------------------------+
 *                          +---> | secondary split branch  |
 *                                +-------------------------+
 *                                | other branch targets... |
 *                                +-------------------------+
 *                                            ^
 *                                            |
 *                                       seg._patches        */

/* Later, the compiler "patches" these jump destinations when the next
 * instructions are filled in. */

/*    +0      +1      +2      +3      +4      +5
 * ~~~+-------+-------+-------+-------+-------+-------+~~~
 * .. | Char  | Char  | Split | Char  | Match | Split | ..
 * .. | Instr | Instr | Instr | Instr | Instr | Instr | ..
 * ~~~+-------+-------+---+-+-+-------+-------+---+-+-+~~~
 *                        | |   ^               ^ | |
 *                        | |   |               | +-|------> ...
 *                        +-----+               |   |
 *                          |                   |   +------> ...
 *                          +-------------------+                */

/* Note that this is a relatively simple example. */

RE_INTERNAL void re__compile_seg_init(re__compile_seg* seg, re__prog_loc start) {
    seg->start = start;
    seg->end = start;
    re__compile_patch_vec_init(&seg->_patches);
}

RE_INTERNAL void re__compile_seg_destroy(re__compile_seg* seg) {
    re__compile_patch_vec_destroy(&seg->_patches);
}

RE_INTERNAL void re__compile_seg_set_start(re__compile_seg* seg, re__prog_loc start) {
    seg->start = start;
}

RE_INTERNAL re__prog_loc re__compile_seg_get_start(re__compile_seg* seg) {
    return seg->start;
}

RE_INTERNAL void re__compile_seg_set_end(re__compile_seg* seg, re__prog_loc end) {
    seg->end = end;
}

RE_INTERNAL re__prog_loc re__compile_seg_get_end(re__compile_seg* seg) {
    return seg->end;
}

RE_INTERNAL re_error re__compile_seg_add_patch(re__compile_seg* seg, re__prog_loc loc, int secondary) {
    re__compile_patch patch;
    re__compile_patch_init(&patch, loc, secondary);
    return re__compile_patch_vec_push(&seg->_patches, patch); 
}

/* Execute a patch -- fill in all outgoing arrows from the program segment. */
RE_INTERNAL void re__compile_seg_patch(re__compile_seg* seg, re__prog* prog, re__prog_loc target) {
    re_size patch_idx;
    for (patch_idx = 0; patch_idx < re__compile_patch_vec_size(&seg->_patches); patch_idx++) {
        /* Get the patch in question */
        re__compile_patch current_patch = re__compile_patch_vec_get(&seg->_patches, patch_idx);
        re__prog_inst current_inst;
        /* Get the instruction in question */
        current_inst = re__prog_get(prog, re__compile_patch_get_loc(&current_patch));
        /* Write either the primary or secondary jump target */
        if (!re__compile_patch_get_secondary(&current_patch)) {
            re__prog_inst_set_primary(&current_inst, target);
        } else {
            re__prog_inst_set_split_secondary(&current_inst, target);
        }
        /* Write the instruction back into the program */
        re__prog_set(prog, re__compile_patch_get_loc(&current_patch), current_inst);
    }
}

/* Merge patches from another segment */
RE_INTERNAL re_error re__compile_seg_merge(re__compile_seg* seg, re__compile_seg* other) {
    return re__compile_patch_vec_cat(&seg->_patches, &other->_patches);
}

/* Internal stack frame structure used by the compiler. */
/* This is necessary because we 'walk' the AST tree when we compile, as opposed
 * to recursively iterating on one node at a time. */
/* We use this to guarantee constant stack usage. */
typedef struct re__compile_frame {
    /* Root AST node */
    re__ast* ast_node;
    /* Index of the current child being compiled */
    re_size child_index;
    /* Patches that need to be addressed, as well as the range within the
     * program that this frame compiled to */
    re__compile_seg seg;
} re__compile_frame;

RE_INTERNAL void re__compile_frame_init(re__compile_frame* frame, re__ast* ast_node, re_size child_index, re__compile_seg seg) {
    frame->ast_node = ast_node;
    frame->child_index = child_index;
    frame->seg = seg;
}

RE_VEC_DECL(re__compile_frame);
RE_VEC_IMPL_FUNC(re__compile_frame, init)
RE_VEC_IMPL_FUNC(re__compile_frame, destroy)
RE_VEC_IMPL_FUNC(re__compile_frame, push)
RE_VEC_IMPL_FUNC(re__compile_frame, pop)
RE_VEC_IMPL_FUNC(re__compile_frame, size)

RE_INTERNAL re_error re__compile(re* re) {
    re_error err = RE_ERROR_NONE;
    /* Stack frames used for AST walking */
    re__compile_frame_vec stk;
    /* Initial stack frame and segment */
    re__compile_frame initial_frame;
    re__compile_seg initial_seg;
    /* Returned segment from a child node */
    re__compile_seg returned_seg;
    /* Convenience pointer to the program */
    re__prog* prog = &re->data->program;
    re__compile_frame_vec_init(&stk);
    /* Note that the program might have things in it -- depending on how the
     * regex is being used, there might be a several-instruction-long prelude
     * before the actual program */
    /* For this reason we start the segment at the end of the program */
    re__compile_seg_init(&initial_seg, re__prog_size(prog));
    /* The initial frame points to the root AST node, suggests we are on the
     * zero'th child, and starts at the program's end. */
    re__compile_frame_init(&initial_frame, &re->data->ast_root, 0, initial_seg);
    re__compile_frame_vec_push(&stk, initial_frame);
    /* While there are nodes left to compile... */
    while (re__compile_frame_vec_size(&stk)) {
        /* Definitions: */
        /* Parent -+- Child 1 ---- First Child
         *         |
         *         +- Child 2 -\
         *         |            \
         *         +- Child 3 ---- Intermediate Children
         *         |            /
         *         +- Child 4 -/
         *         |
         *         +- Child 5 ---- Last Child */
        re__compile_frame top = re__compile_frame_vec_pop(&stk);
        const re__prog_loc next_pc = re__prog_size(prog);
        re__ast_type type = top.ast_node->type;
        /* At the end of this iteration, should we push the next child AST node
         * to the stack? */
        int should_push_next_child = 0;
        /* Switch on different types of AST nodes */
        if (type == RE__AST_TYPE_CHAR) {
            /* Generates a single CHAR instruction. */
            /*    +0
             * ~~~+-------+~~~
             * .. | Char  | ..
             * .. | Instr | ..
             * ~~~+---+---+~~~
             *        |
             *        +-----1--> ... */
            /* Char nodes are simple -- they compile directly to a CHAR op. */
            re__prog_inst new_inst;
            re__prog_inst_init_char(
                &new_inst, 
                re__ast_get_char(top.ast_node)
            ); /* Creates unpatched branch (1) */
            if ((err = re__prog_add(prog, new_inst))) {
                return err;
            }
            /* Add an outgoing patch */
            if ((err = re__compile_seg_add_patch(&top.seg, next_pc, 0))) {
                return err;
            }
        } else if (type == RE__AST_TYPE_CHAR_RANGE) {
            /* Generates a single CHAR_RANGE instruction. */
            /*    +0
             * ~~~+-------+~~~
             * .. | CharR | ..
             * .. | Instr | ..
             * ~~~+---+---+~~~
             *        |
             *        +-----1--> ... */
            /* Likewise for char range nodes */
            re__prog_inst new_inst;
            re__prog_inst_init_char_range(
                &new_inst, 
                re__ast_get_char_lo(top.ast_node),
                re__ast_get_char_hi(top.ast_node)
            ); /* Creates unpatched branch (1) */
            if ((err = re__prog_add(prog, new_inst))) {
                return err;
            }
            /* Add an outgoing patch */
            if ((err = re__compile_seg_add_patch(&top.seg, next_pc, 0))) {
                return err;
            }
        } else if (type == RE__AST_TYPE_CHAR_STRING) {
            /* Generates multiple CHAR instructions. */
            /*    +0      +1      +2        +L(S)-1
             * ~~~+-------+-------+--.....--+-------+~~~
             * .. | Char  | Char  |  Char   | Char  | ..
             * .. | Instr | Instr |  Instrs | Instr | ..
             * ~~~+---+---+---+---+--..+..--+---+---+~~~
             *        |     ^ |     ^  |      ^ |
             *        |     | |     |  |      | +------2-> ...
             *        +--1--+ +-1a--+  +--1b--+               */
            re__str str = re__ast_get_str(top.ast_node);
            re_size str_length = re__str_size(&str);
            re_char* data = re__str_get_data(&str);
            re_size i;
            re__prog_loc cur_pc = next_pc;
            for (i = 0; i < str_length; i++) {
                re_char ch = data[i];
                re__prog_inst new_inst;
                re__prog_inst_init_char(
                    &new_inst, 
                    ch
                ); /* Creates unpatched branch (2) */
                cur_pc++;
                if (i != str_length - 1) {
                    /* Creates branches (1, 1a, 1b) */
                    re__prog_inst_set_primary(&new_inst, cur_pc);
                }
                if ((err = re__prog_add(prog, new_inst))) {
                    return err;
                }
            }
            /* Add an outgoing patch */
            if ((err = re__compile_seg_add_patch(&top.seg, cur_pc - 1, 0))) {
                return err;
            }
        } else if (type == RE__AST_TYPE_CONCAT) {
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
            re_size length = re__ast_get_num_children(top.ast_node);
            if (top.child_index < length) {
                if (top.child_index == 0) {
                    /* Before first child */
                    /* Don't need to do anything */
                } else {
                    /* Patch outgoing branches (1, 2, 3) */
                    re__compile_seg_patch(&returned_seg, prog, next_pc);
                    /* Discard returned segment, we don't need it anymore */
                    re__compile_seg_destroy(&returned_seg);
                }
                /* Before first child, intermediate children, and last child */
                should_push_next_child = 1;
            } else {
                /* After last child */
                /* Collect outgoing branches (4, 5) */
                if ((err = re__compile_seg_merge(&top.seg, &returned_seg))) {
                    return err;
                }
                re__compile_seg_destroy(&returned_seg);
            }
        } else if (type == RE__AST_TYPE_ALT) {
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
            re_size length = re__ast_get_num_children(top.ast_node);
            if (top.child_index < length) {
                if (top.child_index == 0) {
                    /* Before first child */
                    /* Initialize split instruction */
                    re__prog_inst new_inst;
                    re__prog_inst_init_split(
                        &new_inst, 
                        next_pc + 1, /* Outgoing branch (1) */
                        RE__PROG_LOC_INVALID /* Will become outgoing branch (2) */
                    );
                    /* Add the Split instruction */
                    if ((err = re__prog_add(prog, new_inst))) {
                        return err;
                    }
                } else if (top.child_index <= length - 1) {
                    /* Before intermediate children and last child */
                    /* Patch the secondary branch target of the old SPLIT
                     * instruction. Corresponds to outgoing branch (2). */
                    /* top.seg.end points to the instruction after the old split 
                     * instruction, since we didn't set the endpoint before the 
                     * first child. */
                    re__prog_loc old_split_loc = re__compile_seg_get_end(&top.seg) - 1;
                    re__prog_inst old_inst = re__prog_get(prog, old_split_loc);
                    /* Patch outgoing branch (2). */
                    re__prog_inst_set_split_secondary(&old_inst, next_pc);
                    re__prog_set(prog, old_split_loc, old_inst);
                    /* Collect outgoing branches (5, 6, 7, 8). */
                    if ((err = re__compile_seg_merge(&top.seg, &returned_seg))) {
                        return err;
                    }
                    /* Discard returned segment */
                    re__compile_seg_destroy(&returned_seg);
                    if (top.child_index < length - 1) {
                        /* Before intermediate children and NOT last child */
                        re__prog_inst new_inst;
                        /* Create the intermediary SPLIT instruction, if there
                         * are more than two child nodes in the alternation. */
                        re__prog_inst_init_split(
                            &new_inst, 
                            next_pc + 1, /* Outgoing branch (3) */
                            RE__PROG_LOC_INVALID /* Outgoing branch (4) */
                        );
                        /* Add it to the program. */
                        if ((err = re__prog_add(prog, new_inst))) {
                            return err;
                        }
                    }
                }
                /* Before every child, including the first and last one */
                should_push_next_child = 1;
            } else {
                /* Collect outgoing branches (9, 10). */
                if ((err = re__compile_seg_merge(&top.seg, &returned_seg))) {
                    return err;
                }
                /* Discard the returned segment. */
                re__compile_seg_destroy(&returned_seg);
            }
        } else if (type == RE__AST_TYPE_STAR) {
            /* Generates a single SPLIT instruction and the instructions of its
             * child node, patching the secondary branch from the split
             * instruction to the child node. Patches the child's outgoing
             * branch targets back to the split instruction. Leaves the
             * primary branch from the split instruction unpatched. */
            /* For the non-greedy version, the secondary and primary branch 
             * targets are swapped. */
            /* Greedy version */
            /*    +0      +1        +L(C)
             * ~~~+-------+--.....--+~~~
             * .. | Split |  Child  | ..
             * .. | Instr |  Instrs | ..
             * ~~~+---+-+-+--..+..+-+~~~
             *      ^ | |   ^  |  |
             *      | | |   |  |  |
             *      | +---1-+  +<-+
             *      |   |      | 
             *      +-------3--+
             *          |
             *          +--------------2-> ... */
            /* Non-greedy version: */
            /*    +0      +1        +L(C)
             * ~~~+-------+--.....--+~~~
             * .. | Split |  Child  | ..
             * .. | Instr |  Instrs | ..
             * ~~~+---+-+-+--..+..+-+~~~
             *      ^ | |   ^  |  |
             *      | | |   |  |  |
             *      | | +-4-+  +<-+
             *      | |        | 
             *      +-------6--+
             *        |  
             *        +----------------5-> ... */
            if (top.child_index == 0) {
                /* Before child */
                re__prog_inst new_split;
                if (re__ast_get_quantifier_greedy(top.ast_node)) {
                    re__prog_inst_init_split(
                        &new_split, 
                        next_pc + 1,         /* Outgoing branch (1) */
                        RE__PROG_LOC_INVALID /* Outgoing branch (2) */
                    );
                } else {
                    /* Prioritize continuing on for non-greedy matching */
                    re__prog_inst_init_split(
                        &new_split, 
                        RE__PROG_LOC_INVALID, /* Outgoing branch (5) */
                        next_pc + 1           /* Outgoing branch (4) */
                    );
                }
                /* Add the split instruction */
                if ((err = re__prog_add(prog, new_split))) {
                    return err;
                }
                should_push_next_child = 1;
            } else if (top.child_index == 1) {
                /* After child */
                /* Patch outgoing branches (3, 6) */
                /* Redirect child's branches back to split instruction */
                re__prog_loc split_start = re__compile_seg_get_start(&top.seg);
                re__compile_seg_patch(&returned_seg, prog, split_start);
                re__compile_seg_destroy(&returned_seg);
                if (re__ast_get_quantifier_greedy(top.ast_node)) {
                    /* Collect arrow (2) */
                    re__compile_seg_add_patch(&top.seg, split_start, 1);
                } else {
                    /* Collect arrow (5) */
                    re__compile_seg_add_patch(&top.seg, split_start, 0);
                }
            } else {
                RE__ASSERT_UNREACHED();
            }
        } else if (type == RE__AST_TYPE_PLUS) {
            /* Generates the instructions of the child node, and then a single
             * SPLIT instruction. Patches the child's outgoing branch targets
             * to the split instruction. Patches the primary branch target of
             * the split instruction back to the child. Leaves the secondary
             * branch from the split instruction unpatched. */
            /* For the non-greedy version, the secondary and primary branch
             * targets are swapped. */
            /* Greedy version: */
            /*    +0        +L(C)   +L(C)+1
             * ~~~+--.....--+-------+~~~
             * .. |  Child  | Split | ..
             * .. |  Instrs | Instr | ..
             * ~~~+--..+..+-+---+-+-+~~~
             *      ^  |  |   ^ | |
             *      |  2  |   | | +----4-> ...
             *      |  +->+-1-+ | 
             *      |           |
             *      +-----3-----+              */
            /* Non-greedy version: */
            /*    +0        +L(C)   +L(C)+1
             * ~~~+--.....--+-------+~~~
             * .. |  Child  | Split | ..
             * .. |  Instrs | Instr | ..
             * ~~~+--..+..+-+---+-+-+~~~
             *      ^  |  |   ^ | |
             *      |  6  |   | +-----8-> ...
             *      |  +->+-5-+   |
             *      |             |
             *      +-----7-------+           */
            if (top.child_index == 0) {
                /* Before child */
                should_push_next_child = 1;
            } else if (top.child_index == 1) {
                /* After child */
                re__prog_inst new_split;
                /* Patch branches (1, 2, 5, 6) */
                re__compile_seg_patch(&returned_seg, prog, next_pc);
                re__compile_seg_destroy(&returned_seg);
                if (re__ast_get_quantifier_greedy(top.ast_node)) {
                    re__prog_inst_init_split(
                        &new_split, 
                        top.seg.start,       /* Create branch (3) */
                        RE__PROG_LOC_INVALID /* Create unpatched branch (4) */
                    );
                    /* Collect unpatched branch (4) */
                    if ((err = re__compile_seg_add_patch(&top.seg, next_pc, 1))) {
                        return err;
                    }
                } else {
                    re__prog_inst_init_split(
                        &new_split, 
                        RE__PROG_LOC_INVALID, /* Create unpatched branch (8) */
                        top.seg.start         /* Create branch (7) */
                    );
                    /* Collect unpatched branch (8) */
                    if ((err = re__compile_seg_add_patch(&top.seg, next_pc, 0))) {
                        return err;
                    }
                }
                if ((err = re__prog_add(prog, new_split))) {
                    return err;
                }
            } else {
                RE__ASSERT_UNREACHED();
            }
        } else if (type == RE__AST_TYPE_QUESTION) {
            /* Generates a single SPLIT instruction and its child node, linking
             * the primary branch from the split instruction to the child node.
             * Leaves the child's outgoing branch targets unpatched, as well as
             * the secondary branch from the split instruction. */
            /* For the non-greedy version, the secondary and primary branch
             * targets are swapped. */
            /* Greedy version: */
            /*    +0      +1        +L(C)
             * ~~~+-------+--.....--+~~~
             * .. | Split |  Child  | ..
             * .. | Instr |  Instrs | ..
             * ~~~+---+-+-+--..+..+-+~~~
             *        | |   ^  |  |
             *        | |   |  +-------3-> ...
             *        +--1--+     |
             *          |         +----4-> ...
             *          |
             *          +--------------2-> ... */
            /* Non-greedy version: */
            /*    +0      +1        +L(C)
             * ~~~+-------+--.....--+~~~
             * .. | Split |  Child  | ..
             * .. | Instr |  Instrs | ..
             * ~~~+---+-+-+--..+..+-+~~~
             *        | |   ^  |  |
             *        | |   |  +-------7-> ...
             *        | +-6-+     |
             *        |           +----8-> ...
             *        |  
             *        +----------------5-> ... */
            if (top.child_index == 0) {
                /* Before child */
                re__prog_inst new_split;
                if (re__ast_get_quantifier_greedy(top.ast_node)) {
                    re__prog_inst_init_split(
                        &new_split, 
                        next_pc + 1, /* Create branch (1) */
                        0            /* Create unpatched branch (2) */
                    );
                } else {
                    re__prog_inst_init_split(
                        &new_split, 
                        0,           /* Create unpatched branch (5) */
                        next_pc + 1 /* Create branch (6) */
                    );
                }
                if ((err = re__prog_add(prog, new_split))) {
                    return err;
                }
                should_push_next_child = 1;
            } else if (top.child_index == 1) {
                /* After child */
                /* Collect branches (3, 4, 7, 8) */
                if ((err = re__compile_seg_merge(&top.seg, &returned_seg))) {
                    return err;
                }
                /* Discard return value */
                re__compile_seg_destroy(&returned_seg);
                if (re__ast_get_quantifier_greedy(top.ast_node)) {
                    re__compile_seg_add_patch(
                        &top.seg, 
                        re__compile_seg_get_start(&top.seg),
                        1 /* Collect branch (2) */
                    );
                } else {
                    re__compile_seg_add_patch(
                        &top.seg, 
                        re__compile_seg_get_start(&top.seg),
                        0 /* Collect branch (5) */
                    );
                }
            } else {
                RE__ASSERT_UNREACHED();
            }
        } else {
            RE__ASSERT_UNREACHED();
        }
        /* Set the end of the segment to the next instruction */
        re__compile_seg_set_end(&top.seg, next_pc + 1);
        if (should_push_next_child) {
            re__compile_frame up_frame;
            re__compile_seg up_seg;
            re_size old_child_index = top.child_index;
            /* Increment the child index on the top frame before we push
             * it again */
            top.child_index += 1;
            if ((err = re__compile_frame_vec_push(&stk, top))) {
                return err;
            }
            /* Prepare the child's segment */
            re__compile_seg_init(
                &up_seg, 
                re__prog_size(prog)
            );
            /* Prepare the child's stack frame */
            re__compile_frame_init(
                &up_frame, 
                re__ast_get_child(top.ast_node, old_child_index),
                0, /* Start at first child *of* child */
                up_seg
            );
            if ((err = re__compile_frame_vec_push(&stk, up_frame))) {
                return err;
            }
        }
        returned_seg = top.seg;
    }
    return err;
}

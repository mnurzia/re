#include "re_internal.h"

MN__VEC_IMPL_FUNC(re__parse_frame, init)
MN__VEC_IMPL_FUNC(re__parse_frame, destroy)
MN__VEC_IMPL_FUNC(re__parse_frame, push)
MN__VEC_IMPL_FUNC(re__parse_frame, pop)
MN__VEC_IMPL_FUNC(re__parse_frame, peek)
MN__VEC_IMPL_FUNC(re__parse_frame, size)

MN_INTERNAL void re__parse_init(re__parse* parse, re* reg) {
    parse->re = reg;
    re__parse_frame_vec_init(&parse->frames);
    re__charclass_builder_init(&parse->charclass_builder);
    parse->ast_prev_child_ref = RE__AST_NONE;
    parse->ast_frame_root_ref = RE__AST_NONE;
    parse->state = RE__PARSE_STATE_GND;
    parse->radix_num = 0;
    parse->radix_digits = 0;
    parse->defer = 0;
    parse->group_flags = 0;
    parse->group_flags_new = 0;
    parse->str_begin = NULL;
    parse->str_end = NULL;
    parse->charclass_lo_rune = -1;
    parse->depth_max = 0;
    parse->depth_max_prev = 0;
    parse->depth = 0;
    parse->ast_root = &reg->data->ast_root;
}

MN_INTERNAL void re__parse_destroy(re__parse* parse) {
    re__charclass_builder_destroy(&parse->charclass_builder);
    re__parse_frame_vec_destroy(&parse->frames);
}

/* Convenience function to set the error to a literal string. */
MN_INTERNAL re_error re__parse_error(re__parse* parse, const char* err_chars) {
    mn__str err_str;
    re_error err = mn__str_init_s(&err_str, (const mn_char*)err_chars);
    if (err) {
        mn__str_destroy(&err_str);
        return err;
    }
    if ((err = re__set_error_str(parse->re, &err_str))) {
        mn__str_destroy(&err_str);
        return err;
    }
    return RE_ERROR_PARSE;
}

/* Formats a "invalid escape sequence" message */
MN_INTERNAL re_error re__parse_error_invalid_escape(re__parse* parse, re_rune esc) {
    /* Build error message */
    re_error err = RE_ERROR_NONE;
    mn__str err_str;
    mn_char esc_ch[2];
    esc_ch[0] = (mn_char)esc;
    esc_ch[1] = '\'';
    if ((err = mn__str_init_s(&err_str, (const mn_char*)"invalid escape sequence '\\"))) {
        goto destroy_err_str;
    }
    if ((err = mn__str_cat_n(&err_str, esc_ch, 2))) {
        goto destroy_err_str;
    }

    re__set_error_str(parse->re, &err_str);
destroy_err_str:
    mn__str_destroy(&err_str);
    return RE_ERROR_PARSE;
}

#define RE__PARSE_UTF8_ACCEPT 0
#define RE__PARSE_UTF8_REJECT 12

static const mn_uint8 re__parse_utf8_tt[] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
   1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
   7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,  7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
   8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
  10,3,3,3,3,3,3,3,3,3,3,3,3,4,3,3, 11,6,6,6,5,8,8,8,8,8,8,8,8,8,8,8,

   0,12,24,36,60,96,84,12,12,12,48,72, 12,12,12,12,12,12,12,12,12,12,12,12,
  12, 0,12,12,12,12,12, 0,12, 0,12,12, 12,24,12,12,12,12,12,24,12,24,12,12,
  12,12,12,12,12,12,12,24,12,12,12,12, 12,24,12,12,12,12,12,12,12,24,12,12,
  12,12,12,12,12,12,12,36,12,36,12,12, 12,36,12,12,12,12,12,36,12,36,12,12,
  12,36,12,12,12,12,12,12,12,12,12,12, 
};

MN_INTERNAL mn_uint32 re__parse_utf8_decode(mn_uint32* state, mn_uint32* codep, mn_uint32 byte) {
  mn_uint32 type = re__parse_utf8_tt[byte];

  *codep = (*state != 0) ?
    (byte & 0x3fu) | (*codep << 6) :
    (0xff >> type) & (byte);

  *state = re__parse_utf8_tt[256 + *state + type];
  return *state;
}

MN_INTERNAL re_error re__parse_next_char(re__parse* parse, const mn_char** current_loc, const mn_char* end_loc, re_rune* ch) {
    mn_uint32 codep = 0;
    mn_uint32 state = 0;
    while (1) {
        if (*current_loc == end_loc) {
            if (state == RE__PARSE_UTF8_REJECT) {
                (*current_loc)++;
                return re__parse_error(parse, "invalid UTF-8 byte");
            } else {
                (*current_loc)++;
                *ch = -1;
                return RE_ERROR_NONE;
            }
        } else {
            mn_uint8 in_byte = (mn_uint8)**current_loc;
            if (!re__parse_utf8_decode(&state, &codep, in_byte)) {
                (*current_loc)++;
                *ch = (re_rune)codep;
                return RE_ERROR_NONE;
            } else if (state == RE__PARSE_UTF8_REJECT) {
                (*current_loc)++;
                return re__parse_error(parse, "invalid UTF-8 byte");
            } else {
                (*current_loc)++;
            }
        }
    }
    return RE_ERROR_NONE;
}

MN_INTERNAL int re__parse_frame_is_empty(re__parse* parse) {
    return parse->ast_prev_child_ref == RE__AST_NONE;
}

MN_INTERNAL re__ast* re__parse_get_frame(re__parse* parse) {
    return re__ast_root_get(parse->ast_root, parse->ast_frame_root_ref);
}

MN_INTERNAL re_error re__parse_push_node(re__parse* parse, re__ast ast, mn_int32* new_ast_ref) {
    re_error err = RE_ERROR_NONE;
    if ((err = re__ast_root_add_child(parse->ast_root, parse->ast_frame_root_ref, ast, new_ast_ref))) {
        return err;
    }
    parse->ast_prev_child_ref = *new_ast_ref;
    parse->depth_max_prev = parse->depth;
    return err;
}

/* Insert a node right before the previous child, making the previous child the
 * new node's parent. */
MN_INTERNAL re_error re__parse_link_wrap_node(re__parse* parse, re__ast outer, mn_int32* new_outer) {
    re_error err = RE_ERROR_NONE;
    if ((err = re__ast_root_add_wrap(parse->ast_root, parse->ast_frame_root_ref, parse->ast_prev_child_ref, outer, new_outer))) {
        return err;
    }
    parse->depth_max_prev += 1;
    parse->depth_max = MN__MAX(parse->depth_max_prev, parse->depth_max);
    parse->ast_prev_child_ref = *new_outer;
    return err;
}

MN_INTERNAL re_error re__parse_wrap_node(re__parse* parse, re__ast outer) {
    mn_int32 dummy;
    return re__parse_link_wrap_node(parse, outer, &dummy);
}

MN_INTERNAL re_error re__parse_frame_push(re__parse* parse) {
    re__parse_frame op;
    op.ast_frame_root_ref = parse->ast_frame_root_ref;
    op.ast_prev_child_ref = parse->ast_prev_child_ref;
    op.ret_state = parse->state;
    op.group_flags = parse->group_flags;
    op.depth = parse->depth;
    op.depth_max = parse->depth_max;
    return re__parse_frame_vec_push(&parse->frames, op);
}

MN_INTERNAL void re__parse_frame_pop(re__parse* parse) {
    re__parse_frame op;
    MN_ASSERT(re__parse_frame_vec_size(&parse->frames) > 0);
    op = re__parse_frame_vec_pop(&parse->frames);
    parse->ast_frame_root_ref = op.ast_frame_root_ref;
    parse->ast_prev_child_ref = op.ast_prev_child_ref;
    parse->state = op.ret_state;
    parse->group_flags = op.group_flags;
    parse->depth_max_prev = parse->depth_max;
    parse->depth = op.depth;
    parse->depth_max = MN__MAX(op.depth_max, parse->depth_max);
}

MN_INTERNAL re_error re__parse_opt_fuse_concat(re__parse* parse, re__ast* next, int* did_fuse) {
    re__ast* prev;
    re__ast_type t_prev, t_next;
    re_error err = RE_ERROR_NONE;
    MN_ASSERT(parse->ast_prev_child_ref != RE__AST_NONE);
    prev = re__ast_root_get(parse->ast_root, parse->ast_prev_child_ref);
    t_prev = prev->type;
    t_next = next->type;
    *did_fuse = 0;
    if (t_prev == RE__AST_TYPE_RUNE) {
        if (t_next == RE__AST_TYPE_RUNE) {
            /* Opportunity to fuse two runes into a string */
            mn__str new_str;
            re__ast new_ast;
            mn_char rune_bytes[16]; /* 16 oughta be good */
            mn_int32 new_str_ref;
            int rune_bytes_ptr = 0;
            rune_bytes_ptr += re__compile_gen_utf8(re__ast_get_rune(prev), (mn_uint8*)rune_bytes + rune_bytes_ptr);
            rune_bytes_ptr += re__compile_gen_utf8(re__ast_get_rune(next), (mn_uint8*)rune_bytes + rune_bytes_ptr);
            if ((err = mn__str_init_n(&new_str, rune_bytes, (mn_size)rune_bytes_ptr))) {
                return err;
            }
            if ((err = re__ast_root_add_str(parse->ast_root, new_str, &new_str_ref))) {
                mn__str_destroy(&new_str);
                return err;
            }
            re__ast_init_str(&new_ast, new_str_ref);
            re__ast_root_replace(parse->ast_root, parse->ast_prev_child_ref, new_ast);
            re__ast_destroy(next);
            *did_fuse = 1;
        }
    } else if (t_prev == RE__AST_TYPE_STR) {
        if (t_next == RE__AST_TYPE_RUNE) {
            /* Opportunity to add a rune to a string */
            mn__str* old_str;
            mn_char rune_bytes[16];
            mn_int32 old_str_ref;
            int rune_bytes_ptr = 0;
            rune_bytes_ptr += re__compile_gen_utf8(re__ast_get_rune(next), (mn_uint8*)rune_bytes + rune_bytes_ptr);
            old_str_ref = re__ast_get_str_ref(prev);
            old_str = re__ast_root_get_str(parse->ast_root, old_str_ref);
            if ((err = mn__str_cat_n(old_str, rune_bytes, (mn_size)rune_bytes_ptr))) {
                return err;
            }
            re__ast_destroy(next);
            *did_fuse = 1;
        }
    }
    return err;
}

/* Add a new node to the end of the stack, while maintaining these invariants:
 * - Group nodes can only hold one immediate node.
 * - Alt nodes can only hold one immediate node per branch.
 * - Concat nodes can hold an infinite number of nodes.
 * 
 * To maintain these, when we have to add a second child to an alt/group node, 
 * we convert it into a concatenation of the first and second children. */
MN_INTERNAL re_error re__parse_link_new_node(re__parse* parse, re__ast new_ast, mn_int32* new_ast_ref) {
    re__ast_type frame_type = RE__AST_TYPE_NONE;
    re_error err = RE_ERROR_NONE;
    /* Firstly, attempt an optimization by fusing the nodes, if possible. */
    if (!re__parse_frame_is_empty(parse)) {
        int did_fuse;
        if ((err = re__parse_opt_fuse_concat(parse, &new_ast, &did_fuse))) {
            return err;
        }
        if (did_fuse) {
            /* We successfully fused the node, so there is no need to create
            * a new concatenation. */
            return err;
        }
    }
    /* If we are here, then the node couldn't be optimized away and we have to
     * push it. */
    if (parse->ast_frame_root_ref != RE__AST_NONE) {
        re__ast* frame = re__parse_get_frame(parse);
        frame_type = frame->type;
    }
    /* Weird control flow -- it's the only way I figured out how to do the
     * assertion below. */
    if (frame_type == RE__AST_TYPE_GROUP || frame_type == RE__AST_TYPE_ALT || frame_type == RE__AST_TYPE_NONE) {
        if (re__parse_frame_is_empty(parse)) {
            /* Push node, fallthrough */
        } else {
            re__ast new_concat;
            mn_int32 old_inner;
            mn_int32 new_outer;
            /* Push the current frame */
            if ((err = re__parse_frame_push(parse))) {
                return err;
            }
            old_inner = parse->ast_prev_child_ref;
            /* Wrap the last child(ren) in a concatenation */
            re__ast_init_concat(&new_concat);
            if ((err = re__parse_link_wrap_node(parse, new_concat, &new_outer))) {
                return err;
            }
            /* Set frame_ref to the new outer node */
            parse->ast_frame_root_ref = new_outer;
            parse->depth += 1;
            /* Set prev_child_ref to the last child */
            parse->ast_prev_child_ref = old_inner;
            /* frame is now invalid */
            /* new_concat is moved */
        }
    } else if (frame_type == RE__AST_TYPE_CONCAT) {
        /* Push node, fallthrough */
    } else {
        /* Due to operator precedence, we should never arrive here. */
        MN__ASSERT_UNREACHED();
    }
    /* Add the new node to the frame. */
    if ((err = re__parse_push_node(parse, new_ast, new_ast_ref))) {
        return err;
    }
    return err;
}

MN_INTERNAL re_error re__parse_add_new_node(re__parse* parse, re__ast new_ast) {
    mn_int32 dummy;
    return re__parse_link_new_node(parse, new_ast, &dummy);
}


MN_INTERNAL re_error re__parse_finish_alt(re__parse* parse) {
    MN_ASSERT(re__parse_get_frame(parse)->type == RE__AST_TYPE_ALT);
    if (re__parse_frame_is_empty(parse)) {
        re__ast new_concat;
        re__ast_init_concat(&new_concat);
        return re__parse_add_new_node(parse, new_concat);
    }
    return RE_ERROR_NONE;
}

MN_INTERNAL re_error re__parse_finish(re__parse* parse) {
    re_error err = RE_ERROR_NONE;
    /* Pop frames until frame_ptr == 0. */
    while (1) {
        re__ast* frame;
        re__ast_type peek_type;
        if (!re__parse_frame_vec_size(&parse->frames)) {
            break;
        } else {
            frame = re__parse_get_frame(parse);
            peek_type = frame->type;
        }
        if (parse->ast_frame_root_ref == -1) {
            /* We have hit the base frame successfully. */
            /* Since the base frame is a group, if we continue the loop we will
             * run into an error. */
            break;
        }
        if (peek_type == RE__AST_TYPE_CONCAT) {
            /* Just pop concatenations */
            re__parse_frame_pop(parse);
        } else if (peek_type == RE__AST_TYPE_ALT) {
            /* Finish alternations and then pop */
            if ((err = re__parse_finish_alt(parse))) {
                return err;
            }
            re__parse_frame_pop(parse);
        } else if (peek_type == RE__AST_TYPE_GROUP) {
            /* If we find a group, that means it has not been closed. */
            return re__parse_error(parse, "unmatched '('");
        }
    }
    return err;
}

/* Begin a new group. Push a group node and set stack/previous/base pointers
 * appropriately. Also push the current frame. */
MN_INTERNAL re_error re__parse_group_begin(re__parse* parse) {
    re__ast new_group;
    re_error err = RE_ERROR_NONE;
    mn_int32 new_group_ref;
    mn__str_view group_name;
    mn_uint32 new_group_idx = 0;
    if (parse->group_flags & RE__AST_GROUP_FLAG_NAMED) {
        if (parse->group_flags & RE__AST_GROUP_FLAG_NONMATCHING) {
            return re__parse_error(parse, "cannot have non-matching group with a name");
        } else {
            mn__str_view_init_n(&group_name, parse->str_begin, (mn_size)(parse->str_end - parse->str_begin));
            new_group_idx = re__ast_root_get_num_groups(parse->ast_root);
            if ((err = re__ast_root_add_group(parse->ast_root, group_name))) {
                return err;
            }
        }
    } else if (!(parse->group_flags & RE__AST_GROUP_FLAG_NONMATCHING)) {
        mn__str_view_init_null(&group_name);
        new_group_idx = re__ast_root_get_num_groups(parse->ast_root);
        if ((err = re__ast_root_add_group(parse->ast_root, group_name))) {
            return err;
        }
    }
    re__ast_init_group(&new_group, new_group_idx, parse->group_flags);
    if ((err = re__parse_link_new_node(parse, new_group, &new_group_ref))) {
        return err;
    }
    /* Also pushes old group flags so they can be restored later */
    if ((err = re__parse_frame_push(parse))) {
        return err;
    }
    /* Increment depth */
    parse->depth += 1;
    /* No previous node to keep track of depth for */
    parse->depth_max_prev = parse->depth;
    /* Maximum depth within this group is equal to base depth since there are
     * no child nodes */
    parse->depth_max = parse->depth;
    /* Set running group flags to the new group flags */
    parse->group_flags = parse->group_flags_new;
    /* The frame is now the newly pushed Group node. */
    parse->ast_frame_root_ref = new_group_ref;
    /* There is no previous child */
    parse->ast_prev_child_ref = RE__AST_NONE;
    return err;
}

/* End a group. Pop operators until we get a group node. */
MN_INTERNAL re_error re__parse_group_end(re__parse* parse) {
    while (1) {
        re__ast_type peek_type;
        /* If we are at the absolute bottom of the stack, there was no opening
         * parentheses to begin with. */
        if (!re__parse_frame_vec_size(&parse->frames)) {
            return re__parse_error(parse, "unmatched ')'");
        }
        /* Check the type of the current frame */
        peek_type = re__parse_get_frame(parse)->type;
        /* Now pop the current frame */
        re__parse_frame_pop(parse);
        /* If we just popped a group, finish */
        if (peek_type == RE__AST_TYPE_GROUP) {
            break;
        }
    }
    return RE_ERROR_NONE;
}

/* Act on a '|' character. If this is the first alteration, we wrap the current
 * group node in an ALT. If not, we simply add to the previous ALT. */
MN_INTERNAL re_error re__parse_alt(re__parse* parse) {
    re__ast_type peek_type;
    re_error err = RE_ERROR_NONE;
    while (1) {
        peek_type = RE__AST_TYPE_NONE;
        if (parse->ast_frame_root_ref != RE__AST_NONE) {
            peek_type = re__parse_get_frame(parse)->type;
        }
        if (peek_type == RE__AST_TYPE_CONCAT) {
            /* Pop all concatenations, alt takes priority */
            re__parse_frame_pop(parse);
        } else if (peek_type == RE__AST_TYPE_GROUP || peek_type == RE__AST_TYPE_NONE) {
            /* This is the initial alteration: "a|" or "(a|" */
            /* Note: the group in question could be the base frame. */
            /* In any case, we shim an ALT node in before the previous child. */
            /* Since GROUP nodes are defined to have a maximum of one child, we
             * don't need to mess around with the amount of children for either
             * node. */
            re__ast new_alt;
            mn_int32 new_alt_ref;
            re__ast_init_alt(&new_alt);
            if (re__parse_frame_is_empty(parse)) {
                /* Empty frame -- null alteration. */
                re__ast new_concat;
                re__ast_init_concat(&new_concat);
                if ((err = re__parse_add_new_node(parse, new_concat))) {
                    return err;
                }
            }
            if ((err = re__parse_link_wrap_node(parse, new_alt, &new_alt_ref))) {
                return err;
            }
            /* frame is now invalid */
            /* Push a new ALT frame */
            if ((err = re__parse_frame_push(parse))) {
                return err;
            }
            /* Set depth correctly, see group_begin() */
            parse->depth += 1;
            parse->depth_max_prev = parse->depth;
            /* Perform a MAX here because we need to keep track of the previous
             * node that started the ALT */
            parse->depth_max = MN__MAX(parse->depth_max, parse->depth);
            /* Set base_ptr to the ALT node */
            parse->ast_frame_root_ref = new_alt_ref;
            /* Indicate that there are no new children. */
            parse->ast_prev_child_ref = RE__AST_NONE;
            return err;
        } else if (peek_type == RE__AST_TYPE_ALT) {
            /* Third+ part of the alteration: "a|b|" or "(a|b|" */
            /* Indicate that there are no new children (this is the beginning
             * of the second+ part of an alteration) */
            parse->ast_prev_child_ref = RE__AST_NONE;
            parse->depth_max = MN__MAX(parse->depth_max, parse->depth_max_prev);
            return err;
        }
    }
}

/* Ingest a single rune. */
MN_INTERNAL re_error re__parse_rune(re__parse* parse, re_rune ch) {
    re__ast new_rune;
    re__ast_init_rune(&new_rune, ch);
    return re__parse_add_new_node(parse, new_rune);
}

/* Clear number parsing state. */
MN_INTERNAL void re__parse_radix_clear(re__parse* parse) {
    parse->radix_num = 0;
    parse->radix_digits = 0;
}

/* Maximum octal value that can be held in three digits */
#define RE__PARSE_OCT_NUM_MAX (8 * 8 * 8)
#define RE__PARSE_OCT_DIGITS_MAX 3
#define RE__PARSE_IS_OCT_DIGIT(ch) \
    ((ch) == '0' || (ch) == '1' || (ch) == '2' || (ch) == '3' || (ch) == '4' || (ch) == '5' || (ch) == '6' || (ch) == '7')

/* Parse in a single octal digit. */
MN_INTERNAL re_error re__parse_radix_consume_oct(re__parse* parse, re_rune ch) {
    if (parse->radix_digits == RE__PARSE_OCT_DIGITS_MAX) {
        return re__parse_error(parse, "octal literal exceeds maximum of three digits");
    }
    MN_ASSERT(RE__PARSE_IS_OCT_DIGIT(ch));
    parse->radix_num *= 8;
    parse->radix_num += ch - '0';
    parse->radix_digits++;
    if (parse->radix_num >= RE__PARSE_OCT_NUM_MAX) {
        return re__parse_error(parse, "octal literal exceeds maximum value of \\777");
    }
    return RE_ERROR_NONE;
}

/* Ensure that the radix doesn't exceed octal limits. */
MN_INTERNAL re_error re__parse_radix_check_oct(re__parse* parse) {
    if (parse->radix_num >= RE__PARSE_OCT_NUM_MAX) {
        return re__parse_error(parse, "octal literal exceeds maximum value of \\777");
    }
    if (parse->radix_digits > RE__PARSE_OCT_DIGITS_MAX) {
        return re__parse_error(parse, "octal literal exceeds maximum of three digits");
    }
    return RE_ERROR_NONE;
}

#define RE__PARSE_HEX_SHORT_NUM_MAX (16 * 16)
#define RE__PARSE_HEX_SHORT_DIGITS_MAX  2
#define RE__PARSE_IS_HEX_DIGIT(ch) \
    ((ch) == '0' || (ch) == '1' || (ch) == '2' || (ch) == '3' || (ch) == '4' || (ch) == '5' || (ch) == '6' || (ch) == '7' || \
     (ch) == '8' || (ch) == '9' || (ch) == 'A' || (ch) == 'B' || (ch) == 'C' || (ch) == 'D' || (ch) == 'E' || (ch) == 'F' || \
     (ch) == 'a' || (ch) == 'b' || (ch) == 'c' || (ch) == 'd' || (ch) == 'e' || (ch) == 'f')

MN_INTERNAL re_rune re__parse_radix_hex_digit(re_rune dig) {
    if (dig >= '0' && dig <= '9') {
        return dig - '0';
    } else if (dig >= 'A' && dig <= 'F') {
        return (dig - 'A') + 10;
    } else if (dig >= 'a' && dig <= 'f') {
        return (dig - 'a') + 10;
    } else {
        MN__ASSERT_UNREACHED();
        return 0;
    }
}

/* Parse in a single hex digit in short format (\x..) */
MN_INTERNAL re_error re__parse_radix_consume_hex_short(re__parse* parse, re_rune ch) {
    if (parse->radix_digits == RE__PARSE_HEX_SHORT_DIGITS_MAX) {
        return re__parse_error(parse, "short hex literal exceeds maximum of two digits");
    }
    MN_ASSERT(RE__PARSE_IS_HEX_DIGIT(ch));
    parse->radix_num *= 16;
    parse->radix_num += re__parse_radix_hex_digit(ch);
    parse->radix_digits++;
    if (parse->radix_num >= RE__PARSE_HEX_SHORT_NUM_MAX) {
        return re__parse_error(parse, "short hex literal exceeds maximum value of \\xFF");
    }
    return RE_ERROR_NONE;
}

/* Ensure that the radix doesn't exceed hex limits. */
MN_INTERNAL re_error re__parse_radix_check_hex_short(re__parse* parse) {
    if (parse->radix_num >= RE__PARSE_HEX_SHORT_NUM_MAX) {
        return re__parse_error(parse, "short hex literal exceeds maximum value of \\xFF");
    }
    if (parse->radix_digits > RE__PARSE_HEX_SHORT_DIGITS_MAX) {
        return re__parse_error(parse, "short hex literal exceeds maximum of two digits");
    }
    return RE_ERROR_NONE;
}

#define RE__PARSE_HEX_LONG_NUM_MAX RE_RUNE_MAX+1
#define RE__PARSE_HEX_LONG_DIGITS_MAX 6

/* Parse in a single hex digit in long format (\x{....}) */
MN_INTERNAL re_error re__parse_radix_consume_hex_long(re__parse* parse, re_rune ch) {
    if (parse->radix_digits == RE__PARSE_HEX_LONG_DIGITS_MAX) {
        return re__parse_error(parse, "long hex literal exceeds maximum of six digits");
    }
    MN_ASSERT(RE__PARSE_IS_HEX_DIGIT(ch));
    parse->radix_num *= 16;
    parse->radix_num += re__parse_radix_hex_digit(ch);
    parse->radix_digits++;
    if (parse->radix_num >= RE__PARSE_HEX_LONG_NUM_MAX) {
        return re__parse_error(parse, "long hex literal exceeds maximum value of \\x{10FFFF}");
    }
    return RE_ERROR_NONE;
}

/* Ensure that the radix doesn't exceed hex limits. */
MN_INTERNAL re_error re__parse_radix_check_hex_long(re__parse* parse) {
    if (parse->radix_num > RE__PARSE_HEX_LONG_NUM_MAX) {
        return re__parse_error(parse, "long hex literal exceeds maximum value of \\x{10FFFF}");
    }
    if (parse->radix_digits > RE__PARSE_HEX_LONG_DIGITS_MAX) {
        return re__parse_error(parse, "long hex literal exceeds maximum of six digits");
    }
    return RE_ERROR_NONE;
}

#define RE__PARSE_COUNTING_DIGITS_MAX 6
#define RE__PARSE_IS_DEC_DIGIT(ch) \
    ((ch) == '0' || (ch) == '1' || (ch) == '2' || (ch) == '3' || (ch) == '4' || (ch) == '5' || (ch) == '6' || (ch) == '7' || \
     (ch) == '8' || (ch) == '9')

/* Parse in a single decimal digit */
MN_INTERNAL re_error re__parse_radix_consume_counting(re__parse* parse, re_rune ch) {
    if (parse->radix_digits == RE__PARSE_COUNTING_DIGITS_MAX) {
        return re__parse_error(parse, "counting literal exceeds maximum of four digits");
    }
    MN_ASSERT(RE__PARSE_IS_DEC_DIGIT(ch));
    parse->radix_num *= 10;
    parse->radix_num += ch - '0';
    parse->radix_digits++;
    if (parse->radix_num >= RE__AST_QUANTIFIER_MAX) {
        return re__parse_error(parse, "counting literal exceeds maximum value of " MN__STRINGIFY(RE__AST_QUANTIFIER_MAX) );
    }
    return RE_ERROR_NONE;
}

/* Ensure that the radix doesn't exceed decimal counting limits. */
MN_INTERNAL re_error re__parse_radix_check_counting(re__parse* parse) {
    if (parse->radix_num > RE__AST_QUANTIFIER_MAX) {
        return re__parse_error(parse, "counting literal exceeds maximum value of " MN__STRINGIFY(RE__AST_QUANTIFIER_MAX) );
    }
    if (parse->radix_digits > RE__PARSE_COUNTING_DIGITS_MAX) {
        return re__parse_error(parse, "counting literal exceeds maximum of four digits");
    }
    return RE_ERROR_NONE;
}

/* Disallow an escape character from occurring in a character class. */
/* This function succeeds when the calling state is GND, but does not when the
 * calling state is anything else, in which case it returns an error. */
/* Ensure that this is only called with printable characters. */
MN_INTERNAL re_error re__parse_disallow_escape_in_charclass(re__parse* parse, re_rune esc) {
    re_error err = RE_ERROR_NONE;
    re__parse_frame top;
    mn__str err_str;
    MN_ASSERT(re__parse_frame_vec_size(&parse->frames) > 0);
    top = re__parse_frame_vec_peek(&parse->frames);
    if (top.ret_state != RE__PARSE_STATE_GND) {
        /* Build error message */
        mn_char esc_ch = (mn_char)(esc);
        if ((err = mn__str_init_s(&err_str, (const mn_char*)"cannot use escape sequence '\\"))) {
            goto destroy_err_str;
        }
        if ((err = mn__str_cat_n(&err_str, &esc_ch, 1))) {
            goto destroy_err_str;
        }
        if ((err = mn__str_cat_s(&err_str, (const mn_char*)"' from within character class (\"[]\")"))) {
            goto destroy_err_str;
        }
        re__set_error_str(parse->re, &err_str);
        return RE_ERROR_PARSE;
    }
    return err;
destroy_err_str:
    mn__str_destroy(&err_str);
    return err;
}

/* Create a new assert */
MN_INTERNAL re_error re__parse_create_assert(re__parse* parse, re__assert_type assert_type) {
    re__ast new_node;
    re__ast_init_assert(&new_node, assert_type);
    return re__parse_add_new_node(parse, new_node);
}

/* Create a new "any byte" (\C) */
MN_INTERNAL re_error re__parse_create_any_byte(re__parse* parse) {
    re__ast new_node;
    re__ast_init_any_byte(&new_node);
    return re__parse_add_new_node(parse, new_node);
}

/* Create a new "any char" (.) */
MN_INTERNAL re_error re__parse_create_any_char(re__parse* parse) {
    re__ast new_node;
    re__ast_init_any_char(&new_node);
    return re__parse_add_new_node(parse, new_node);
}

MN_INTERNAL void re__parse_charclass_begin(re__parse* parse) {
    re__charclass_builder_begin(&parse->charclass_builder);
    parse->charclass_lo_rune = -1;
}

MN_INTERNAL void re__parse_charclass_setlo(re__parse* parse, re_rune ch) {
    parse->charclass_lo_rune = ch;
}

MN_INTERNAL re_error re__parse_charclass_addlo(re__parse* parse) {
    re_error err = RE_ERROR_NONE;
    re__rune_range new_range;
    MN_ASSERT(parse->charclass_lo_rune != -1);
    new_range.min = parse->charclass_lo_rune;
    new_range.max = parse->charclass_lo_rune;
    parse->charclass_lo_rune = -1;
    if ((err = re__charclass_builder_insert_range(&parse->charclass_builder, new_range))) {
        return err;
    }
    return err;
}

MN_INTERNAL re_error re__parse_charclass_addhi(re__parse* parse, re_rune ch) {
    re_error err = RE_ERROR_NONE;
    re__rune_range new_range;
    MN_ASSERT(parse->charclass_lo_rune != -1);
    new_range.min = parse->charclass_lo_rune;
    new_range.max = ch;
    parse->charclass_lo_rune = -1;
    /* Account for swapped ranges, they are legal */
    if (new_range.min > new_range.max) {
        MN__SWAP(new_range.min, new_range.max, re_rune);
    }
    if ((err = re__charclass_builder_insert_range(&parse->charclass_builder, new_range))) {
        return err;
    }
    return err;
}


MN_INTERNAL re_error re__parse_charclass_finish(re__parse* parse) {
    re__ast new_node;
    re__charclass new_charclass;
    re_error err = RE_ERROR_NONE;
    mn_int32 new_charclass_ref;
    if ((err = re__charclass_builder_finish(&parse->charclass_builder, &new_charclass))) {
        return err;
    }
    if ((err = re__ast_root_add_charclass(parse->ast_root, new_charclass, &new_charclass_ref))) {
        re__charclass_destroy(&new_charclass);
        return err;
    }
    re__ast_init_charclass(&new_node, new_charclass_ref);
    if ((err = re__parse_add_new_node(parse, new_node))) {
        return err;
    }
    return err;
}

/* Depending on the current state, push a new character AST node, or add the
 * given character to the current character class. */
MN_INTERNAL re_error re__parse_finish_escape_char(re__parse* parse, re_rune ch) {
    re__parse_state top_state;
    re_error err = RE_ERROR_NONE;
    /* This can only be called on GND or a charclass builder state. */
    MN_ASSERT(re__parse_frame_vec_size(&parse->frames) > 0);
    top_state = re__parse_frame_vec_peek(&parse->frames).ret_state;
    re__parse_frame_pop(parse);
    if (top_state == RE__PARSE_STATE_GND) {
        /* Wrap it in an AST node */
        re__ast new_node;
        re__ast_init_rune(&new_node, ch);
        if ((err = re__parse_add_new_node(parse, new_node))) {
            return err;
        }
    } else if (top_state == RE__PARSE_STATE_CHARCLASS_AFTER_LO) {
        MN_ASSERT(parse->charclass_lo_rune == -1);
        re__parse_charclass_setlo(parse, ch);
    } else if (top_state == RE__PARSE_STATE_CHARCLASS_LO) {
        MN_ASSERT(parse->charclass_lo_rune != -1);
        if ((err = re__parse_charclass_addhi(parse, ch))) {
            return err;
        }
    } else {
        MN__ASSERT_UNREACHED();
    }
    return err;
}

/* Depending on the current state, add a character class *into* a character
 * class, *OR* add a character class AST node. */
MN_INTERNAL re_error re__parse_finish_escape_class(re__parse* parse, re__charclass_ascii_type ascii_cc, int inverted) {
    re__parse_state top_state;
    re__charclass new_class;
    re_error err = RE_ERROR_NONE;
    top_state = re__parse_frame_vec_peek(&parse->frames).ret_state;
    re__parse_frame_pop(parse);
    /* Create the character class */
    if ((err = re__charclass_init_from_class(&new_class, ascii_cc, inverted))) {
        return err;
    }
    if (top_state == RE__PARSE_STATE_GND) {
        /* Wrap it in an AST node */
        re__ast new_node;
        mn_int32 new_class_ref;
        if ((err = re__ast_root_add_charclass(parse->ast_root, new_class, &new_class_ref))) {
            re__charclass_destroy(&new_class);
            return err;
        }
        re__ast_init_charclass(&new_node, new_class_ref);
        /* new_node now owns new_class */
        if ((err = re__parse_add_new_node(parse, new_node))) {
            re__charclass_destroy(&new_class);
            return err;
        }
    } else if (top_state == RE__PARSE_STATE_CHARCLASS_AFTER_LO) {
        MN_ASSERT(parse->charclass_lo_rune == -1);
        /* Add it to the charclass builder */
        if ((err = re__charclass_builder_insert_class(&parse->charclass_builder, &new_class))) {
            /* destroy charclass */
            re__charclass_destroy(&new_class);
            return err;
        }
        re__charclass_destroy(&new_class);
        /* This is possibly the most hacky bit within the entire parser. Since
         * a char class cannot have a "range end", we set state to 
         * RE__PARSE_STATE_CHARCLASS_LO. This is, to my knowledge, the only time
         * we explicity set state outside of the main parse function. */
        parse->state = RE__PARSE_STATE_CHARCLASS_LO;
    } else if (top_state == RE__PARSE_STATE_CHARCLASS_LO) {
        if (parse->charclass_lo_rune != -1) {
            re__charclass_destroy(&new_class);
            return re__parse_error(parse, "cannot have character class as upper bound of character range");
        }
    }
    return err;
}

MN_INTERNAL void re__parse_defer(re__parse* parse) {
    parse->defer = 1;
}

MN_INTERNAL re_error re__parse_create_star(re__parse* parse) {
    re__ast new_star;
    if (re__parse_frame_is_empty(parse)) {
        return re__parse_error(parse, "cannot use '*' operator with nothing");
    }
    re__ast_init_quantifier(&new_star, 0, RE__AST_QUANTIFIER_INFINITY);
    re__ast_set_quantifier_greediness(&new_star, !!!(parse->group_flags & RE__AST_GROUP_FLAG_UNGREEDY));
    return re__parse_wrap_node(parse, new_star);
}

MN_INTERNAL re_error re__parse_create_plus(re__parse* parse) {
    re__ast new_plus;
    if (re__parse_frame_is_empty(parse)) {
        return re__parse_error(parse, "cannot use '+' operator with nothing");
    }
    re__ast_init_quantifier(&new_plus, 1, RE__AST_QUANTIFIER_INFINITY);
    re__ast_set_quantifier_greediness(&new_plus, !!!(parse->group_flags & RE__AST_GROUP_FLAG_UNGREEDY));
    return re__parse_wrap_node(parse, new_plus);
}

MN_INTERNAL re_error re__parse_create_question(re__parse* parse) {
    re__ast new_question;
    if (re__parse_frame_is_empty(parse)) {
        return re__parse_error(parse, "cannot use '?' operator with nothing");
    }
    re__ast_init_quantifier(&new_question, 0, 2);
    re__ast_set_quantifier_greediness(&new_question, !!!(parse->group_flags & RE__AST_GROUP_FLAG_UNGREEDY));
    return re__parse_wrap_node(parse, new_question);
}

MN_INTERNAL re_error re__parse_create_repeat(re__parse* parse, mn_int32 min, mn_int32 max) {
    re__ast new_question;
    if (re__parse_frame_is_empty(parse)) {
        return re__parse_error(parse, "cannot use '{' operator with nothing");
    }
    if (min == 0 && max == 1) {
        return re__parse_error(parse, "repetition count must not be zero for '{' operator");
    }
    if (max <= min) {
        return re__parse_error(parse, "minimum repetition count must be less than maximum repetition count for '{' operator");
    }
    re__ast_init_quantifier(&new_question, min, max);
    re__ast_set_quantifier_greediness(&new_question, !!!(parse->group_flags & RE__AST_GROUP_FLAG_UNGREEDY));
    return re__parse_wrap_node(parse, new_question);
}

MN_INTERNAL void re__parse_swap_greedy(re__parse* parse) {
    re__ast* quant;
    /* Cannot make nothing ungreedy */
    MN_ASSERT(!re__parse_frame_is_empty(parse));
    quant = re__ast_root_get(parse->ast_root, parse->ast_prev_child_ref);
    /* Must be a quantifier */
    MN_ASSERT(quant->type == RE__AST_TYPE_QUANTIFIER);
    re__ast_set_quantifier_greediness(quant, !re__ast_get_quantifier_greediness(quant));
}

MN_INTERNAL re_error re__parse_finish_named_class(re__parse* parse, int should_invert) {
    re__charclass new_class;
    re_error err = RE_ERROR_NONE;
    mn__str_view name_view;
    mn__str_view_init_n(&name_view, parse->str_begin, (mn_size)(parse->str_end - parse->str_begin));
    if ((err = re__charclass_init_from_str(&new_class, name_view, should_invert))) {
        if (err == RE_ERROR_INVALID) {
            return re__parse_error(parse, "invalid charclass name");
        }
        return err;
    }
    if ((err = re__charclass_builder_insert_class(&parse->charclass_builder, &new_class))) {
        /* destroy charclass */
        re__charclass_destroy(&new_class);
        return err;
    }
    re__charclass_destroy(&new_class);
    return err;
}

MN_INTERNAL void re__parse_str_clear(re__parse* parse, const mn_char* begin) {
    parse->str_begin = begin;
    parse->str_end = begin;
}

MN_INTERNAL void re__parse_str_setend(re__parse* parse, const mn_char* end) {
    parse->str_end = end;
}

#define RE__IS_LAST() (ch == -1)
/* This macro is only used within re__parse_str. */
/* Try-except usually encourages forgetting to clean stuff up, but the 
 * constraints on code within this function allow us to always use this macro 
 * safely. */
/* I think it's a good design choice. */
#define RE__TRY(expr) \
    if ((err = expr)) { \
        goto error; \
    }

MN_INTERNAL re_error re__parse_str(re__parse* parse, const mn__str_view* regex) {
    /*const mn_char* start = regex;*/
    const mn_char* current = mn__str_view_get_data(regex);
    const mn_char* end = current + mn__str_view_size(regex);
    re_error err = RE_ERROR_NONE;
    /* ch will only be -1 if the if this is the last character, a.k.a.
     * "epsilon" as all the cool kids call it */
    re_rune ch;
    /* Set stack/previous pointers accordingly. */
    parse->ast_prev_child_ref = RE__AST_NONE;
    /* Set initial depth. */
    parse->depth = 0; /* 1 because of initial group */
    parse->depth_max = 0;
    parse->depth_max_prev = 0; /* same as parse->depth */
    while (current <= end || parse->defer) {
        if (parse->defer) {
            parse->defer -= 1;
        } else {
            if ((err = re__parse_next_char(parse, &current, end, &ch))) {
                goto error;
            }
        }
        if (parse->state == RE__PARSE_STATE_GND) {
            /* Within the main body of the state machine, everything is kept 
             * purposefully as simple as possible, in order to avoid errors and 
             * keep things easy to follow.
             * Notably:
             *  - No temporaries.
             *  - No control flow.
             *  - Everything is either a function call or an assignment to some
             *    parser variable.
             * Specicfically, disallowing temporaries (i.e., objects that need
             * to be cleaned up) allows much more simplified error handling at
             * the cost of more function calls and functions used during
             * parsing. In this case, it allows the use of RE__TRY macros, which
             * just check the error code returned by each function and goto a
             * single central error handling location. */
            if (RE__IS_LAST()) {
                /* Finish parsing. */
                RE__TRY(re__parse_finish(parse));
            } else if (ch == '$') {
                /* $: Text end assert. */
                RE__TRY(re__parse_create_assert(parse, RE__ASSERT_TYPE_TEXT_END));
            } else if (ch == '(') {
                /* (: Begin a group. */
                re__parse_str_clear(parse, current);
                parse->group_flags_new = parse->group_flags;
                parse->state = RE__PARSE_STATE_PARENS_INITIAL;
            } else if (ch == ')') {
                /* ): End a group. */
                RE__TRY(re__parse_group_end(parse));
            } else if (ch == '*') {
                /* *: Create a Kleene star. */
                RE__TRY(re__parse_create_star(parse));
                parse->state = RE__PARSE_STATE_MAYBE_QUESTION;
            } else if (ch == '+') {
                /* +: Create a Kleene plus. */
                RE__TRY(re__parse_create_plus(parse));
                parse->state = RE__PARSE_STATE_MAYBE_QUESTION;
            } else if (ch == '.') {
                /* .: Create an "any character." */
                RE__TRY(re__parse_create_any_char(parse));
            } else if (ch == '?') {
                /* ?: Create a question. */
                RE__TRY(re__parse_create_question(parse));
                parse->state = RE__PARSE_STATE_MAYBE_QUESTION;
            } else if (ch == '[') {
                /* [: Start of a character class. */
                re__parse_charclass_begin(parse);
                parse->state = RE__PARSE_STATE_CHARCLASS_INITIAL;
            } else if (ch == '\\') {
                /* Switch to ESCAPE, and push a frame. */
                RE__TRY(re__parse_frame_push(parse));
                parse->state = RE__PARSE_STATE_ESCAPE;  
            } else if (ch == '^') {
                /* ^: Text start assert. */
                RE__TRY(re__parse_create_assert(parse, RE__ASSERT_TYPE_TEXT_START));
            } else if (ch == '{') {
                /* {: Start of counting form. */
                re__parse_radix_clear(parse);
                parse->counting_first_num = 0;
                parse->state = RE__PARSE_STATE_COUNTING_FIRST_NUM_INITIAL;
            } else if (ch == '|') {
                /* Begin or continue an alternation. */
                RE__TRY(re__parse_alt(parse));
            } else {
                /* Push a character. */
                RE__TRY(re__parse_rune(parse, ch));
            }
        } else if (parse->state == RE__PARSE_STATE_MAYBE_QUESTION) {
            if (RE__IS_LAST()) {
                /* Defer finishing parsing to GND. */
                re__parse_defer(parse);
                parse->state = RE__PARSE_STATE_GND;
            } else if (ch == '?') {
                /* <quant>?: Swap <quant>'s non-greediness. */
                re__parse_swap_greedy(parse);
                parse->state = RE__PARSE_STATE_GND;
            } else {
                re__parse_defer(parse);
                parse->state = RE__PARSE_STATE_GND;
            }
        } else if (parse->state == RE__PARSE_STATE_ESCAPE) {
            /* Escape character: \ */
            if (RE__IS_LAST()) {
                /* \ followed by end of string */
                RE__TRY(re__parse_error(parse, "unfinished escape sequence"));
            } else if (RE__PARSE_IS_OCT_DIGIT(ch)) {
                /* \0 - \7: Octal digits */
                re__parse_radix_clear(parse);
                RE__TRY(re__parse_radix_consume_oct(parse, ch));
                parse->state = RE__PARSE_STATE_OCTAL_SECOND_DIGIT;
            } else if (ch == 'A') {
                /* \A: Absolute text start */
                /* This cannot be used from within a char class */
                RE__TRY(re__parse_disallow_escape_in_charclass(parse, ch));
                /* Return to GND or calling state */
                re__parse_frame_pop(parse);
                RE__TRY(re__parse_create_assert(parse, RE__ASSERT_TYPE_TEXT_START_ABSOLUTE));
            } else if (ch == 'B') {
                /* \B: Not a word boundary */
                RE__TRY(re__parse_disallow_escape_in_charclass(parse, ch));
                re__parse_frame_pop(parse);
                RE__TRY(re__parse_create_assert(parse, RE__ASSERT_TYPE_WORD_NOT));
            } else if (ch == 'C') {
                /* \C: Any *byte* (NOT any char) */
                RE__TRY(re__parse_disallow_escape_in_charclass(parse, ch));
                re__parse_frame_pop(parse);
                RE__TRY(re__parse_create_any_byte(parse));
            } else if (ch == 'D') {
                /* \D: Non-digit characters */
                RE__TRY(re__parse_finish_escape_class(parse, RE__CHARCLASS_ASCII_TYPE_DIGIT, 1));
            } else if (ch == 'E') {
                /* \E: Invalid here */
                RE__TRY(re__parse_error(parse, "\\E can only be used from within \\Q"));
            } else if (ch == 'F' || ch == 'G' || ch == 'H' || ch == 'I' || ch == 'J' || ch == 'K' || ch == 'L' || ch == 'M' || ch == 'N' || ch == 'O') {
                /* \F - \O: Invalid */
                RE__TRY(re__parse_error_invalid_escape(parse, ch));
            } else if (ch == 'P') {
                /* \P: Inverted Unicode character class */
                RE__TRY(re__parse_error(parse, "unimplemented"));
                /*re__parser_unicode_charclass_invert(parser);
                parser.state = RE__PARSE_STATE_UNICODE_CHARCLASS_BEGIN;*/
            } else if (ch == 'Q') {
                /* \Q: Quote begin */
                RE__TRY(re__parse_disallow_escape_in_charclass(parse, ch));
                re__parse_frame_pop(parse);
                parse->state = RE__PARSE_STATE_QUOTE;
            } else if (ch == 'R') {
                /* \R: Invalid */
                RE__TRY(re__parse_error_invalid_escape(parse, ch));
            } else if (ch == 'S') {
                /* \S: Not whitespace (Perl) */
                RE__TRY(re__parse_finish_escape_class(parse, RE__CHARCLASS_ASCII_TYPE_PERL_SPACE, 1));
            } else if (ch == 'T' || ch == 'U' || ch == 'V') {
                /* \T - \V: Invalid */
                RE__TRY(re__parse_error_invalid_escape(parse, ch));
            } else if (ch == 'W') {
                /* \W: Not a word character */
                RE__TRY(re__parse_finish_escape_class(parse, RE__CHARCLASS_ASCII_TYPE_WORD, 1));
            } else if (ch == 'X' || ch == 'Y' || ch == 'Z') {
                /* \X - \Z: Invalid */
                RE__TRY(re__parse_error_invalid_escape(parse, ch));
            } else if (ch == 'a') {
                /* \a: Bell character */
                RE__TRY(re__parse_finish_escape_char(parse, '\a'));
            } else if (ch == 'b') {
                /* \b: Word boundary */
                RE__TRY(re__parse_disallow_escape_in_charclass(parse, ch));
                re__parse_frame_pop(parse);
                RE__TRY(re__parse_create_assert(parse, RE__ASSERT_TYPE_WORD));
            } else if (ch == 'c') {
                /* \c: Invalid */
                RE__TRY(re__parse_error_invalid_escape(parse, ch));
            } else if (ch == 'd') {
                /* \d: Digit characters */
                RE__TRY(re__parse_disallow_escape_in_charclass(parse, ch));
                RE__TRY(re__parse_finish_escape_class(parse, RE__CHARCLASS_ASCII_TYPE_DIGIT, 0));
            } else if (ch == 'e') {
                /* \e: Invalid */
                RE__TRY(re__parse_error_invalid_escape(parse, ch));
            } else if (ch == 'f') {
                /* \f: Form feed character */
                RE__TRY(re__parse_finish_escape_char(parse, '\f'));
            } else if (ch == 'g' || ch == 'h' || ch == 'i' || ch == 'j' || ch == 'k' || ch == 'l' || ch == 'm') {
                /* \g - \m: Invalid */
                RE__TRY(re__parse_error_invalid_escape(parse, ch));
            } else if (ch == 'n') {
                /* \n: Newline */
                RE__TRY(re__parse_finish_escape_char(parse, '\n'));
            } else if (ch == 'o') {
                /* \o: Invalid */
                RE__TRY(re__parse_error_invalid_escape(parse, ch));
            } else if (ch == 'p') {
                /* \p: Unicode character class */
                RE__TRY(re__parse_error(parse, "unimplemented"));
            } else if (ch == 'r') {
                /* \r: Carriage return */
                RE__TRY(re__parse_finish_escape_char(parse, '\r'));
            } else if (ch == 's') {
                /* \s: Whitespace (Perl) */
                RE__TRY(re__parse_finish_escape_class(parse, RE__CHARCLASS_ASCII_TYPE_PERL_SPACE, 0));
            } else if (ch == 't') {
                /* \t: Horizontal tab */
                RE__TRY(re__parse_finish_escape_char(parse, '\t'));
            } else if (ch == 'u') {
                /* \u: Invalid */
                RE__TRY(re__parse_error_invalid_escape(parse, ch));
            } else if (ch == 'v') {
                /* \v: Vertical tab */
                RE__TRY(re__parse_finish_escape_char(parse, '\v'));
            } else if (ch == 'w') {
                /* \w: Word character */
                RE__TRY(re__parse_finish_escape_class(parse, RE__CHARCLASS_ASCII_TYPE_WORD, 0));
            } else if (ch == 'x') {
                /* \x: Two digit hex literal or one to six digit hex literal */
                re__parse_radix_clear(parse);
                parse->state = RE__PARSE_STATE_HEX_INITIAL;
            } else if (ch == 'y') {
                /* \y: Invalid */
                RE__TRY(re__parse_error_invalid_escape(parse, ch));
            } else if (ch == 'z') {
                /* \z: Absolute text end */
                RE__TRY(re__parse_disallow_escape_in_charclass(parse, ch));
                re__parse_frame_pop(parse);
                RE__TRY(re__parse_create_assert(parse, RE__ASSERT_TYPE_TEXT_END_ABSOLUTE));
            } else {
                /* All other characters */
                RE__TRY(re__parse_finish_escape_char(parse, ch));
            }
        } else if (parse->state == RE__PARSE_STATE_PARENS_INITIAL) {
            /* Start of group: ( */
            if (RE__IS_LAST()) {
                RE__TRY(re__parse_error(parse, "unmatched '('"));
            } else if (ch == '?') {
                /* (?: Start of group flags/name */
                parse->state = RE__PARSE_STATE_PARENS_FLAG_INITIAL;
            } else {
                /* Everything else: begin group */
                re__parse_defer(parse);
                parse->state = RE__PARSE_STATE_GND;
                RE__TRY(re__parse_group_begin(parse));
            }
        } else if (parse->state == RE__PARSE_STATE_PARENS_FLAG_INITIAL) {
            /* Start of group flags: (? */
            if (RE__IS_LAST()) {
                RE__TRY(re__parse_error(parse, "expected one of '-', ':', '<', 'P', 'U', 'i', 'm', 's' for group flags or name"));
            } else if (ch == ')') {
                /* (?): Go back to GND without creating a group, retain flags */
                parse->group_flags = parse->group_flags_new;
                parse->state = RE__PARSE_STATE_GND; 
            } else if (ch == '-') {
                /* (?-: Negate remaining flags */
                parse->state = RE__PARSE_STATE_PARENS_FLAG_NEGATE;
            } else if (ch == ':') {
                /* (?:: Non-matching group, also signals end of flags */
                parse->group_flags_new |= RE__AST_GROUP_FLAG_NONMATCHING;
                parse->state = RE__PARSE_STATE_PARENS_AFTER_COLON;
            } else if (ch == '<') {
                /* (?<: start of group name */
                re__parse_str_clear(parse, current);
                parse->state = RE__PARSE_STATE_PARENS_NAME_INITIAL;
            } else if (ch == 'P') {
                /* (?P: Start of group name (after <) */
                parse->state = RE__PARSE_STATE_PARENS_AFTER_P;
            } else if (ch == 'U') {
                /* (?U: Ungreedy mode: *+? operators have priority swapped */
                parse->group_flags_new |= RE__AST_GROUP_FLAG_UNGREEDY;
            } else if (ch == 'i') {
                /* (?i: Case insensitive matching */
                parse->group_flags_new |= RE__AST_GROUP_FLAG_CASE_INSENSITIVE;
            } else if (ch == 'm') {
                /* (?m: Multi-line mode: ^$ match line boundaries */
                parse->group_flags_new |= RE__AST_GROUP_FLAG_MULTILINE;
            } else if (ch == 's') {
                /* (?s: Stream (?) mode: . matches \n */
                parse->group_flags_new |= RE__AST_GROUP_FLAG_DOT_NEWLINE;
            } else {
                RE__TRY(re__parse_error(parse, "expected one of '-', ':', 'P', 'U', 'i', 'm', 's' for group flags or name"));
            }
        } else if (parse->state == RE__PARSE_STATE_PARENS_FLAG_NEGATE) {
            /* Start of negated group flags: (?:..- */
            if (RE__IS_LAST()) {
                RE__TRY(re__parse_error(parse, "expected one of ':', '<', 'P', 'U', 'i', 'm', 's' for negated group flags or name"));
            } else if (ch == ')') {
                /* (?): Go back to GND without creating a group, retain flags */
                parse->group_flags = parse->group_flags_new;
                parse->state = RE__PARSE_STATE_GND; 
            } else if (ch == ':') {
                /* (?:: Non-matching group, also signals end of flags */
                parse->group_flags_new &= ~((unsigned int)RE__AST_GROUP_FLAG_NONMATCHING);
                parse->state = RE__PARSE_STATE_PARENS_AFTER_COLON;
            } else if (ch == '<') {
                /* (?<: Start of group name */
                re__parse_str_clear(parse, current);
                parse->state = RE__PARSE_STATE_PARENS_NAME_INITIAL;
            } else if (ch == 'P') {
                /* (?P: Start of group name */
                parse->state = RE__PARSE_STATE_PARENS_AFTER_P;
            } else if (ch == 'U') {
                /* (?U: Ungreedy mode: *+? operators have priority swapped */
                parse->group_flags_new &= ~((unsigned int)RE__AST_GROUP_FLAG_UNGREEDY);
            } else if (ch == 'i') {
                /* (?i: Case insensitive matching */
                parse->group_flags_new &= ~((unsigned int)RE__AST_GROUP_FLAG_CASE_INSENSITIVE);
            } else if (ch == 'm') {
                /* (?m: Multi-line mode: ^$ match line boundaries */
                parse->group_flags_new &= ~((unsigned int)RE__AST_GROUP_FLAG_MULTILINE);
            } else if (ch == 's') {
                /* (?s: Stream (?) mode: . matches \n */
                parse->group_flags_new &= ~((unsigned int)RE__AST_GROUP_FLAG_DOT_NEWLINE);
            } else {
                RE__TRY(re__parse_error(parse, "expected one of ':', 'P', 'U', 'i', 'm', 's' for negated group flags or name"));
            }
        } else if (parse->state == RE__PARSE_STATE_PARENS_AFTER_COLON) {
            /* Group flags, after colon: (?: */
            if (RE__IS_LAST()) {
                RE__TRY(re__parse_error(parse, "expected expression or ')' to close group"));
            } else if (ch == ')') {
                /* (?:): Group ended. This is a zero-length group. Wacky! */
                parse->state = RE__PARSE_STATE_GND;
                RE__TRY(re__parse_group_begin(parse));
                RE__TRY(re__parse_group_end(parse));
            } else {
                /* (?:...: Group has an expression. */
                re__parse_defer(parse);
                parse->state = RE__PARSE_STATE_GND;
                RE__TRY(re__parse_group_begin(parse));
            }
        } else if (parse->state == RE__PARSE_STATE_PARENS_AFTER_P) {
            /* Before <, after P for named group: (?P */
            if (RE__IS_LAST()) {
                RE__TRY(re__parse_error(parse, "expected '<' to begin group name"));
            } else if (ch == '<') {
                /* (?P<: Begin group name */
                re__parse_str_clear(parse, current);
                parse->state = RE__PARSE_STATE_PARENS_NAME_INITIAL;
            } else {
                RE__TRY(re__parse_error(parse, "expected '<' to begin group name"));
            }
        } else if (parse->state == RE__PARSE_STATE_PARENS_NAME_INITIAL) {
            /* First character between <> in named group: (?P< */
            if (RE__IS_LAST()) {
                RE__TRY(re__parse_error(parse, "expected '>' to close group name"));
            } else if (ch == '>') {
                /* (?P<>: Empty group name, currently not allowed */
                RE__TRY(re__parse_error(parse, "cannot create empty group name"));
            } else {
                /* Otherwise, add characters to the group name */
                parse->state = RE__PARSE_STATE_PARENS_NAME;
            }
        } else if (parse->state == RE__PARSE_STATE_PARENS_NAME) {
            /* Second+ character between <> in named group: (?P<> */
            if (RE__IS_LAST()) {
                RE__TRY(re__parse_error(parse, "expected '>' to close group name"));
            } else if (ch == '>') {
                /* (?P<...>: End of group name, begin group, go to GND */
                parse->state = RE__PARSE_STATE_GND;
                parse->group_flags |= RE__AST_GROUP_FLAG_NAMED;
                RE__TRY(re__parse_group_begin(parse));
            } else {
                /* (?P<...: Name character, append to name */
                re__parse_str_setend(parse, current);
            }
        } else if (parse->state == RE__PARSE_STATE_OCTAL_SECOND_DIGIT) {
            /* Second digit in an octal literal: \0 */
            if (RE__IS_LAST()) {
                /* Last: push character and defer */
                RE__TRY(re__parse_radix_check_oct(parse));
                re__parse_defer(parse);
                RE__TRY(re__parse_finish_escape_char(parse, parse->radix_num));
            } else if (RE__PARSE_IS_OCT_DIGIT(ch)) {
                /* \.x: add to the radix accumulator */
                RE__TRY(re__parse_radix_consume_oct(parse, ch));
                RE__TRY(re__parse_radix_check_oct(parse));
                parse->state = RE__PARSE_STATE_OCTAL_THIRD_DIGIT;
            } else {
                /* Other characters: push character and defer */
                RE__TRY(re__parse_radix_check_oct(parse));
                re__parse_defer(parse);
                RE__TRY(re__parse_finish_escape_char(parse, parse->radix_num));
            }
        } else if (parse->state == RE__PARSE_STATE_OCTAL_THIRD_DIGIT) {
            /* Third digit in an octal literal: \00 */
            if (RE__IS_LAST()) {
                /* Last: push character and defer */
                RE__TRY(re__parse_radix_check_oct(parse));
                re__parse_defer(parse);
                RE__TRY(re__parse_finish_escape_char(parse, parse->radix_num));
            } else if (RE__PARSE_IS_OCT_DIGIT(ch)) {
                /* \.x: add to the radix accumulator and finish */
                RE__TRY(re__parse_radix_consume_oct(parse, ch));
                RE__TRY(re__parse_radix_check_oct(parse));
                RE__TRY(re__parse_finish_escape_char(parse, parse->radix_num));
            } else {
                /* Other characters: push character and defer */
                RE__TRY(re__parse_radix_check_oct(parse));
                re__parse_defer(parse);
                RE__TRY(re__parse_finish_escape_char(parse, parse->radix_num));
            }
        } else if (parse->state == RE__PARSE_STATE_HEX_INITIAL) {
            /* Hex literal: \x */
            if (RE__IS_LAST()) {
                /* Last: error */
                RE__TRY(re__parse_error(parse, "expected two hex characters or a bracketed hex literal for hex escape \"\\x\""));
            } else if (RE__PARSE_IS_HEX_DIGIT(ch)) {
                /* \x.: add to the radix accumulator */
                RE__TRY(re__parse_radix_consume_hex_short(parse, ch));
                RE__TRY(re__parse_radix_check_hex_short(parse));
                parse->state = RE__PARSE_STATE_HEX_SECOND_DIGIT;
            } else if (ch == '{') {
                /* \x{: Begin bracketed hex literal */
                parse->state = RE__PARSE_STATE_HEX_BRACKETED;
            } else {
                /* Invalid character: error */
                RE__TRY(re__parse_error(parse, "expected two hex characters or a bracketed hex literal for hex escape \"\\x\""));
            }
        } else if (parse->state == RE__PARSE_STATE_HEX_SECOND_DIGIT) {
            /* Second digit of hex literal: \x. */
            if (RE__IS_LAST()) {
                /* Last: error */
                RE__TRY(re__parse_error(parse, "expected a second hex character for hex escape \"\\x\""));
            } else if (RE__PARSE_IS_HEX_DIGIT(ch)) {
                /* \x..: add to the radix accumulator and finish */
                RE__TRY(re__parse_radix_consume_hex_short(parse, ch));
                RE__TRY(re__parse_radix_check_hex_short(parse));
                RE__TRY(re__parse_finish_escape_char(parse, parse->radix_num));
            } else {
                /* Invalid character: error */
                RE__TRY(re__parse_error(parse, "expected a second hex character for hex escape \"\\x\""));
            }
        } else if (parse->state == RE__PARSE_STATE_HEX_BRACKETED) {
            /* Beginning of one to six digit hex literal: \x{ */  
            if (RE__IS_LAST()) {
                /* Last: error */
                RE__TRY(re__parse_error(parse, "expected one to six hex characters for bracketed hex escape \"\\x{\""))
            } else if (RE__PARSE_IS_HEX_DIGIT(ch)) {
                /* \x{.: add to the radix accumulator */
                RE__TRY(re__parse_radix_consume_hex_long(parse, ch));
                RE__TRY(re__parse_radix_check_hex_long(parse));
            } else if (ch == '}') {
                /* \x{...}: Finish */
                RE__TRY(re__parse_radix_check_hex_long(parse));
                RE__TRY(re__parse_finish_escape_char(parse, parse->radix_num));
            } else {
                /* Invalid character: error */
                RE__TRY(re__parse_error(parse, "expected one to six hex characters for bracketed hex escape \"\\x{\""))
            }
        } else if (parse->state == RE__PARSE_STATE_QUOTE) {
            /* Quote string: \Q */
            if (RE__IS_LAST()) {
                /* Last: defer to GND */
                re__parse_defer(parse);
                parse->state = RE__PARSE_STATE_GND;
            } else if (ch == '\\') {
                /* \Q..\: check for 'E' to end quote */
                parse->state = RE__PARSE_STATE_QUOTE_ESCAPE;
            } else {
                /* Otherwise, add char */
                /* hack to work around ret_state, because of a prickly bit of
                 * the parser */
                parse->state = RE__PARSE_STATE_GND;
                RE__TRY(re__parse_rune(parse, ch));
                parse->state = RE__PARSE_STATE_QUOTE;
            }
        } else if (parse->state == RE__PARSE_STATE_QUOTE_ESCAPE) {
            /* Quote escape: \Q...\ */
            if (RE__IS_LAST()) {
                /* Last: ambiguous, error */
                RE__TRY(re__parse_error(parse, "expected 'E' or a character after '\\' within \"\\Q\""));
            } else if (ch == 'E') {
                /* \Q...\E: finish quote string */
                parse->state = RE__PARSE_STATE_GND;
            } else {
                /* Otherwise, add escaped char */
                parse->state = RE__PARSE_STATE_GND;
                RE__TRY(re__parse_rune(parse, ch));
                parse->state = RE__PARSE_STATE_QUOTE;
            }
        } else if (parse->state == RE__PARSE_STATE_COUNTING_FIRST_NUM_INITIAL) {
            /* First number in a counting expression */
            if (RE__IS_LAST()) {
                /* Last: not allowed */
                RE__TRY(re__parse_error(parse, "expected decimal digits for repetition expression '{'"));
            } else if (ch == ',') {
                /* {,: Not allowed yet */
                RE__TRY(re__parse_error(parse, "expected a decimal number before ',' in repetition expression '{'"))
            } else if (RE__PARSE_IS_DEC_DIGIT(ch)) {
                /* {.: Accumulate */
                RE__TRY(re__parse_radix_check_counting(parse));
                RE__TRY(re__parse_radix_consume_counting(parse, ch));
                parse->state = RE__PARSE_STATE_COUNTING_FIRST_NUM;
            } else {
                RE__TRY(re__parse_error(parse, "expected decimal digits for repetition expression '{'"));
            }
        } else if (parse->state == RE__PARSE_STATE_COUNTING_FIRST_NUM) {
            /* First number in a counting expression, after first digit: {. */
            if (RE__IS_LAST()) {
                /* Last: not allowed */
                RE__TRY(re__parse_error(parse, "expected decimal digits for repetition expression '{'"));
            } else if (ch == ',') {
                /* {.,: Move to next number */
                RE__TRY(re__parse_radix_check_counting(parse));
                parse->counting_first_num = parse->radix_num;
                re__parse_radix_clear(parse);
                parse->state = RE__PARSE_STATE_COUNTING_SECOND_NUM_INITIAL;
            } else if (RE__PARSE_IS_DEC_DIGIT(ch)) {
                /* {..: Accumulate */
                RE__TRY(re__parse_radix_check_counting(parse));
                RE__TRY(re__parse_radix_consume_counting(parse, ch));
            } else if (ch == '}') {
                /* {..}: Finish */
                RE__TRY(re__parse_radix_check_counting(parse));
                parse->state = RE__PARSE_STATE_MAYBE_QUESTION;
                RE__TRY(re__parse_create_repeat(parse, parse->radix_num, parse->radix_num+1));
            } else {
                RE__TRY(re__parse_error(parse, "expected decimal digits for repetition expression '{'"));
            }
        } else if (parse->state == RE__PARSE_STATE_COUNTING_SECOND_NUM_INITIAL) {
            /* Second number in a counting expression */
            if (RE__IS_LAST()) {
                /* Last: not allowed */
                RE__TRY(re__parse_error(parse, "expected decimal digits for repetition expression '{'"));
            } else if (RE__PARSE_IS_DEC_DIGIT(ch)) {
                /* {..: Accumulate */
                RE__TRY(re__parse_radix_check_counting(parse));
                RE__TRY(re__parse_radix_consume_counting(parse, ch));
                parse->state = RE__PARSE_STATE_COUNTING_SECOND_NUM;
            } else if (ch == '}') {
                /* {.,}: Finish */
                parse->state = RE__PARSE_STATE_MAYBE_QUESTION;
                RE__TRY(re__parse_create_repeat(parse, parse->counting_first_num, RE__AST_QUANTIFIER_INFINITY));
            } else {
                RE__TRY(re__parse_error(parse, "expected decimal digits for repetition expression '{'"))
            }
        } else if (parse->state == RE__PARSE_STATE_COUNTING_SECOND_NUM) {
            /* Second number in a counting expression, after first char */
            if (RE__IS_LAST()) {
                /* Last: not allowed */
                RE__TRY(re__parse_error(parse, "expected decimal digits for repetition expression '{'"));
            } else if (RE__PARSE_IS_DEC_DIGIT(ch)) {
                /* {..: Accumulate */
                RE__TRY(re__parse_radix_check_counting(parse));
                RE__TRY(re__parse_radix_consume_counting(parse, ch));
            } else if (ch == '}') {
                /* {..}: Finish */
                RE__TRY(re__parse_radix_check_counting(parse));
                parse->state = RE__PARSE_STATE_MAYBE_QUESTION;
                RE__TRY(re__parse_create_repeat(parse, parse->counting_first_num, parse->radix_num+1));
            } else {
                RE__TRY(re__parse_error(parse, "expected decimal digits for repetition expression '{'"));
            }
        } else if (parse->state == RE__PARSE_STATE_CHARCLASS_INITIAL) {
            /* After first charclass bracket: [ */
            if (RE__IS_LAST()) {
                RE__TRY(re__parse_error(parse, "expected '^', characters, character classes, or character ranges for character class expression '['"));
            } else if (ch == '[') {
                /* [[: Literal [ or char class */
                parse->state = RE__PARSE_STATE_CHARCLASS_AFTER_BRACKET;
            } else if (ch == '\\') {
                /* [\: Escape */
                /* Set return state to AFTERLO */
                parse->state = RE__PARSE_STATE_CHARCLASS_AFTER_LO;
                RE__TRY(re__parse_frame_push(parse));
                parse->state = RE__PARSE_STATE_ESCAPE;
            } else if (ch == '^') {
                /* [^: Invert */
                re__charclass_builder_invert(&parse->charclass_builder);
                parse->state = RE__PARSE_STATE_CHARCLASS_AFTER_CARET;
            } else {
                /* Otherwise, add the character */
                re__parse_charclass_setlo(parse, ch);
                parse->state = RE__PARSE_STATE_CHARCLASS_AFTER_LO;
            }
        } else if (parse->state == RE__PARSE_STATE_CHARCLASS_AFTER_CARET) {
            /* After caret in charclass: [^ */
            if (RE__IS_LAST()) {
                RE__TRY(re__parse_error(parse, "expected characters, character classes, or character ranges for character class expression '['"));
            } else if (ch == '[') {
                /* [^[: Literal [ or charclass */
                parse->state = RE__PARSE_STATE_CHARCLASS_AFTER_BRACKET;
            } else if (ch == '\\') {
                /* [^\: Escape */
                parse->state = RE__PARSE_STATE_CHARCLASS_AFTER_LO;
                RE__TRY(re__parse_frame_push(parse));
                parse->state = RE__PARSE_STATE_ESCAPE;
            } else {
                /* Otherwise, add the character */
                re__parse_charclass_setlo(parse, ch);
                parse->state = RE__PARSE_STATE_CHARCLASS_AFTER_LO;
            }
        } else if (parse->state == RE__PARSE_STATE_CHARCLASS_AFTER_BRACKET) {
            /* After initial bracket in charclass: [[ or [^[ */
            if (RE__IS_LAST()) {
                RE__TRY(re__parse_error(parse, "expected characters, character classes, or character ranges for character class expression '['"));
            } else if (ch == '\\') {
                /* [\: Escape. */
                re__parse_charclass_setlo(parse, '[');
                RE__TRY(re__parse_charclass_addlo(parse));
                parse->state = RE__PARSE_STATE_CHARCLASS_AFTER_LO;
                RE__TRY(re__parse_frame_push(parse));
                parse->state = RE__PARSE_STATE_ESCAPE;
            } else if (ch == ']') {
                /* []: A single [. */
                parse->state = RE__PARSE_STATE_GND;
                re__parse_charclass_setlo(parse, '[');
                RE__TRY(re__parse_charclass_addlo(parse));
                RE__TRY(re__parse_charclass_finish(parse));
            } else if (ch == ':') {
                /* [:: Start of ASCII charclass */
                re__parse_str_clear(parse, current);
                parse->state = RE__PARSE_STATE_CHARCLASS_NAMED_INITIAL;
            } else {
                /* Otherwise, add the bracket and the character */
                re__parse_charclass_setlo(parse, '[');
                RE__TRY(re__parse_charclass_addlo(parse));
                re__parse_charclass_setlo(parse, ch);
                parse->state = RE__PARSE_STATE_CHARCLASS_AFTER_LO;
            }
        } else if (parse->state == RE__PARSE_STATE_CHARCLASS_LO) {
            /* Before lo character in a charclass range: [.-. */
            if (RE__IS_LAST()) {
                RE__TRY(re__parse_error(parse, "expected characters, character classes, or character ranges for character class expression '['"));
            } else if (ch == '[') {
                /* [.[: Beginning of ASCII charclass or bracket. */
                parse->state = RE__PARSE_STATE_CHARCLASS_AFTER_BRACKET;
            } else if (ch == '\\') {
                /* [.\: Escape. */
                parse->state = RE__PARSE_STATE_CHARCLASS_AFTER_LO;
                RE__TRY(re__parse_frame_push(parse));
                parse->state = RE__PARSE_STATE_ESCAPE;
            } else if (ch == ']') {
                /* [.]: Finish. */
                parse->state = RE__PARSE_STATE_GND;
                RE__TRY(re__parse_charclass_finish(parse));
            } else {
                /* Otherwise set the low bound. */
                re__parse_charclass_setlo(parse, ch);
                parse->state = RE__PARSE_STATE_CHARCLASS_AFTER_LO;
            }
        } else if (parse->state == RE__PARSE_STATE_CHARCLASS_AFTER_LO) {
            /* After lo character in a charclass range: [. */
            if (RE__IS_LAST()) {
                RE__TRY(re__parse_error(parse, "expected characters, character classes, or character ranges for character class expression '['"));
            } else if (ch == '[') {
                /* [.[: Beginning of ASCII charclass or bracket. */
                RE__TRY(re__parse_charclass_addlo(parse));
                parse->state = RE__PARSE_STATE_CHARCLASS_AFTER_BRACKET;
            } else if (ch == '\\') {
                /* [.\: Escape. */
                RE__TRY(re__parse_charclass_addlo(parse));
                parse->state = RE__PARSE_STATE_CHARCLASS_AFTER_LO;
                RE__TRY(re__parse_frame_push(parse));
                parse->state = RE__PARSE_STATE_ESCAPE;
            } else if (ch == ']') {
                /* [.]: Finished. */
                parse->state = RE__PARSE_STATE_GND;
                RE__TRY(re__parse_charclass_addlo(parse));
                RE__TRY(re__parse_charclass_finish(parse));
            } else if (ch == '-') {
                /* [.-: Set to second range. */
                parse->state = RE__PARSE_STATE_CHARCLASS_HI;  
            } else {
                /* Otherwise, stay here and keep adding chars. */
                RE__TRY(re__parse_charclass_addlo(parse));
                re__parse_charclass_setlo(parse, ch);
            }
        } else if (parse->state == RE__PARSE_STATE_CHARCLASS_HI) {
            /* After low value for range and hyphen : [.- */
            if (RE__IS_LAST()) {
                RE__TRY(re__parse_error(parse, "expected characters, character classes, or character ranges for character class expression '['"));
            } else if (ch == '\\') {
                /* [.-\: Escape. */
                parse->state = RE__PARSE_STATE_CHARCLASS_LO;
                RE__TRY(re__parse_frame_push(parse));
                parse->state = RE__PARSE_STATE_ESCAPE;
            } else if (ch == ']') {
                /* [.-]: Finished. Add a literal hyphen. */
                parse->state = RE__PARSE_STATE_GND;
                RE__TRY(re__parse_charclass_addlo(parse));
                re__parse_charclass_setlo(parse, '-');
                RE__TRY(re__parse_charclass_addlo(parse));
                RE__TRY(re__parse_charclass_finish(parse));
            } else {
                /* Otherwise, add a char and go to range start. */
                RE__TRY(re__parse_charclass_addhi(parse, ch));
                parse->state = RE__PARSE_STATE_CHARCLASS_LO;
            }
        } else if (parse->state == RE__PARSE_STATE_CHARCLASS_NAMED_INITIAL) {
            /* After colon starting named char class: [: */
            if (RE__IS_LAST() || ch == ':' || ch == ']') {
                RE__TRY(re__parse_error(parse, "expected a valid character class name or '^' for named character class expression '[:'"));
            } else if (ch == '^') {
                re__parse_str_clear(parse, current);
                parse->state = RE__PARSE_STATE_CHARCLASS_NAMED_INVERTED;
            } else {
                re__parse_str_setend(parse, current);
                parse->state = RE__PARSE_STATE_CHARCLASS_NAMED;
            }
        } else if (parse->state == RE__PARSE_STATE_CHARCLASS_NAMED) {
            /* After first character in named char class name: [:. */
            if (RE__IS_LAST()) {
                RE__TRY(re__parse_error(parse, "expected a valid character class name for named character class expression '[:'"));
            } else if (ch == ':') {
                /* [:..:: Finish and move to AFTER_COLON */
                RE__TRY(re__parse_finish_named_class(parse, 0));
                parse->state = RE__PARSE_STATE_CHARCLASS_NAMED_AFTER_COLON;
            } else {
                re__parse_str_setend(parse, current);
                parse->state = RE__PARSE_STATE_CHARCLASS_NAMED;
            }
        } else if (parse->state == RE__PARSE_STATE_CHARCLASS_NAMED_INVERTED) {
            /* After first character in named inverted char class name: [:^. */
            if (RE__IS_LAST()) {
                RE__TRY(re__parse_error(parse, "expected a valid character class name for named character class expression '[:'"));
            } else if (ch == ':') {
                /* [:..:: Finish and move to AFTER_COLON */
                RE__TRY(re__parse_finish_named_class(parse, 1));
                parse->state = RE__PARSE_STATE_CHARCLASS_NAMED_AFTER_COLON;
            } else {
                re__parse_str_setend(parse, current);
                parse->state = RE__PARSE_STATE_CHARCLASS_NAMED_INVERTED;
            }
        } else if (parse->state == RE__PARSE_STATE_CHARCLASS_NAMED_AFTER_COLON) {
            /* After final colon in named char class name: [:...: */
            if (RE__IS_LAST()) {
                RE__TRY(re__parse_error(parse, "expected a ']' to end named character class expression"));
            } else if (ch == ']') {
                /* [:...:]: Finish char class */
                parse->state = RE__PARSE_STATE_CHARCLASS_LO;
            } else {
                RE__TRY(re__parse_error(parse, "expected a ']' to end named character class expression"));
            }
        } else {
            MN__ASSERT_UNREACHED();
        }
    }
    /* Parse state must equal ground when done. Other states must either defer
     * to ground or create errors. */
    MN_ASSERT(parse->state == RE__PARSE_STATE_GND);

    MN_ASSERT(re__parse_frame_vec_size(&parse->frames) == 0);
    parse->ast_root->depth_max = parse->depth_max + 1;
#if RE_DEBUG
    MN_ASSERT(re__ast_root_verify(parse->ast_root));
#endif
    return RE_ERROR_NONE;
error:
    if (err == RE_ERROR_PARSE) {
        MN_ASSERT(mn__str_size(&parse->re->data->error_string));
    } else {
        re__set_error_generic(parse->re, err);
    }
    return err;
}

#undef RE__IS_LAST
#undef RE__TRY

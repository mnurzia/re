#include "re_internal.h"

/* Check if the current frame has any children. */
RE_INTERNAL int re__parse_is_frame_empty(re__parse* parse) {
    /* The frame is empty when frame_ptr == prev_child_ptr. */
    return parse->ast_stk_ptr == parse->ast_prev_child_ptr;
}

/* Get the parent AST node, that is, the node at frame_ptr. */
RE_INTERNAL re__ast* re__parse_get_frame(re__parse* parse) {
    /* Assure that frame_ptr is within bounds. */
    RE_ASSERT(parse->ast_frame_ptr < re__ast_vec_size(&parse->ast_stk));
    return re__ast_vec_getref(&parse->ast_stk, parse->ast_frame_ptr);
}

/* Push a node to the end of the AST stack. */
RE_INTERNAL re_error re__parse_push_node(re__parse* parse, re__ast node) {
    re_error err = RE_ERROR_NONE;
    if ((err = re__ast_vec_push(&parse->ast_stk, node))) {
        return err;
    }
    if (parse->ast_stk_ptr == parse->ast_prev_child_ptr) {
        /* Empty frame: increment stk_ptr, leaving prev_child_ptr untouched */
        /* Since we just pushed the first node, prev_child_ptr should now
         * point to it. */
        parse->ast_stk_ptr += 1;
    } else {
        /* Non-empty frame: increment stk_ptr, and set prev_child_ptr to 
         * stk_ptr - 1, so that it points to the just-pushed node. */
        parse->ast_stk_ptr += 1;
        parse->ast_prev_child_ptr = parse->ast_stk_ptr - 1;
    }
    return err;
}

/* Insert a node right before the previous child, making the previous child the
 * new node's parent. */
RE_INTERNAL re_error re__parse_wrap_node(re__parse* parse, re__ast node) {
    re_error err = RE_ERROR_NONE;
    /* Increment children count */
    re__ast_set_children_count(
        &node,
        re__ast_get_children_count(&node) + 1
    );
    if ((err = re__ast_vec_insert(&parse->ast_stk, parse->ast_prev_child_ptr, node))) {
        return err;
    }
    /* Increment stk_ptr, because we inserted into the stack */
    parse->ast_stk_ptr += 1;
    return err;
}

RE_INTERNAL re_error re__parse_push_frame(re__parse* parse) {
    re__parse_op op;
    op.ast_frame_ptr = parse->ast_frame_ptr;
    op.ast_prev_child_ptr = parse->ast_prev_child_ptr;
    op.ret_state = parse->state;
    op.group_flags = parse->group_flags;
    return re__parse_op_vec_push(&parse->op_stk, op);
}

RE_INTERNAL void re__parse_pop_frame(re__parse* parse) {
    re__parse_op op;
    RE_ASSERT(re__parse_op_vec_size(&parse->op_stk) > 0);
    op = re__parse_op_vec_pop(&parse->op_stk);
    parse->ast_frame_ptr = op.ast_frame_ptr;
    parse->ast_prev_child_ptr = op.ast_prev_child_ptr;
    parse->state = op.ret_state;
    parse->group_flags = op.group_flags;
}

/* Add a new node to the end of the stack, while maintaining these invariants:
 * - Group nodes can only hold one immediate node.
 * - Alt nodes can only hold one immediate node per branch.
 * - Concat nodes can hold an infinite number of nodes.
 * 
 * To maintain these, when we have to add a second child to an alt/group node, 
 * we convert it into a concatenation of the first and second children. */
RE_INTERNAL re_error re__parse_add_new_node(re__parse* parse, re__ast node) {
    re__ast* frame = re__parse_get_frame(parse);
    re__ast_type frame_type = frame->type;
    re_error err = RE_ERROR_NONE;
    /* Weird control flow -- it's the only way I figured out how to do the
     * assertion below. */
    if (frame_type == RE__AST_TYPE_GROUP || frame_type == RE__AST_TYPE_ALT) {
        if (re__parse_is_frame_empty(parse)) {
            /* Push node */
        } else {
            re__ast new_concat;
            /* Push the current frame */
            if ((err = re__parse_push_frame(parse))) {
                return err;
            }
            /* Set frame_ptr to right before the last child, as it is where the 
             * concatenation wrap will reside */
            parse->ast_frame_ptr = parse->ast_prev_child_ptr;
            /* Wrap the last child(ren) in a concatenation */
            re__ast_init_concat(&new_concat);
            re__parse_wrap_node(parse, new_concat);
            /* frame is now invalid */
            /* new_concat is moved */
        }
    } else if (frame_type == RE__AST_TYPE_CONCAT) {
        /* Push node */
    } else {
        /* Due to operator precedence, we should never arrive here. */
        RE__ASSERT_UNREACHED();
    }
    /* Now, add the current node to the frame. */
    if ((err = re__parse_push_node(parse, node))) {
        return err;
    }
    /* frame is now invalidated */
    /* Get new frame -- it may have changed */
    frame = re__parse_get_frame(parse);
    /* Increment frame's children. */
    re__ast_set_children_count(frame, re__ast_get_children_count(frame) + 1);
    return err;
}

RE_INTERNAL re_error re__parse_finish(re__parse* parse) {
    re_error err = RE_ERROR_NONE;
    /* Pop frames until frame_ptr == 0. */
    while (1) {
        re__ast* frame = re__parse_get_frame(parse);
        re__ast_type peek_type = frame->type;
        if (parse->ast_frame_ptr == 0) {
            /* We have hit the base frame successfully. */
            /* Since the base frame is a group, if we continue the loop we will
             * run into an error. */
            break;
        }
        if (peek_type == RE__AST_TYPE_CONCAT || peek_type == RE__AST_TYPE_ALT) {
            /* These operators are binary and can be popped, but only if they
             * have more than one node. */
            /* Currently, we disallow unary alternations and concatenations. */
            if (re__ast_get_children_count(frame) == 1) {
                if (peek_type == RE__AST_TYPE_ALT) {
                    return re__parse_error(parse, "cannot use '|' operator with only one value");
                } else if (peek_type == RE__AST_TYPE_CONCAT) {
                    return re__parse_error(parse, "cannot concatenate only one value");
                }
            }
            re__parse_pop_frame(parse);
        } else if (peek_type == RE__AST_TYPE_GROUP) {
            /* If we find a group, that means it has not been closed. */
            return re__parse_error(parse, "unmatched '('");
        }
    }
    return err;
}

/* Begin a new group. Push a group node and set stack/previous/base pointers
 * appropriately. Also push the current frame. */
RE_INTERNAL re_error re__parse_group_begin(re__parse* parse) {
    re__ast new_group;
    re_error err = RE_ERROR_NONE;
    re__ast_init_group(&new_group);
    /* Set group's flags */
    re__ast_set_group_flags(&new_group, parse->group_flags_new);
    if ((err = re__parse_add_new_node(parse, new_group))) {
        return err;
    }
    /* Also pushes old group flags so they can be restored later */
    if ((err = re__parse_push_frame(parse))) {
        return err;
    }
    /* Set running group flags to the new group flags */
    parse->group_flags = parse->group_flags_new;
    /* The frame is now the location right behind the newly pushed Group node. */
    parse->ast_frame_ptr = parse->ast_stk_ptr - 1;
    /* There is no previous child, so indicate that by setting prev_child_ptr to
     * stk_ptr. */
    parse->ast_prev_child_ptr = parse->ast_stk_ptr;
    return err;   
}

/* End a group. Pop operators until we get a group node. */
RE_INTERNAL re_error re__parse_group_end(re__parse* parse) {
    while (1) {
        /* Check the type of the current frame */
        re__ast_type peek_type = re__parse_get_frame(parse)->type;
        /* If we are at the absolute bottom of the stack, there was no opening
         * parentheses to begin with. */
        if (parse->ast_frame_ptr == 0) {
            return re__parse_error(parse, "unmatched ')'");
        }
        /* Now pop the current frame */
        re__parse_pop_frame(parse);
        /* If we just popped a group, finish */
        if (peek_type == RE__AST_TYPE_GROUP) {
            break;
        }
    }
    return RE_ERROR_NONE;
}

/* Act on a '|' character. If this is the first alteration, we wrap the current
 * group node in an ALT. If not, we simply add to the previous ALT. */
RE_INTERNAL re_error re__parse_alt(re__parse* parse) {
    re__ast_type peek_type;
    re_error err = RE_ERROR_NONE;
    while (1) {
        peek_type = re__parse_get_frame(parse)->type;
        if (peek_type == RE__AST_TYPE_CONCAT) {
            /* Pop all concatenations, alt takes priority */
            re__parse_pop_frame(parse);
        } else if (peek_type == RE__AST_TYPE_GROUP) {
            /* This is the initial alteration: "a|" or "(a|" */
            /* Note: the group in question could be the base frame. */
            /* In any case, we shim an ALT node in before the previous child. */
            /* Since GROUP nodes are defined to have a maximum of one child, we
             * don't need to mess around with the amount of children for either
             * node. */
            re__ast new_alt;
            re__ast_init_alt(&new_alt);
            if ((err = re__parse_wrap_node(parse, new_alt))) {
                return err;
            }
            /* frame is now invalid */
            /* Push a new ALT frame */
            if ((err = re__parse_push_frame(parse))) {
                return err;
            }
            /* Set base_ptr to right before the ALT node. */
            parse->ast_frame_ptr = parse->ast_prev_child_ptr;
            /* Indicate that there are no new children. */
            parse->ast_prev_child_ptr = parse->ast_stk_ptr;
            return err;
        } else if (peek_type == RE__AST_TYPE_ALT) {
            /* Third+ part of the alteration: "a|b|" or "(a|b|" */
            /* Indicate that there are no new children (this is the beginning
             * of the second+ part of an alteration) */
            parse->ast_prev_child_ptr = parse->ast_stk_ptr;
            return err;
        }
    }
}

/* Ingest a single character literal. */
RE_INTERNAL re_error re__parse_char(re__parse* parse, re_char ch) {
    re__ast new_char;
    re__ast_init_rune(&new_char, ch);
    return re__parse_add_new_node(parse, new_char);
}

/* Clear number parsing state. */
RE_INTERNAL void re__parse_radix_clear(re__parse* parse) {
    parse->radix_num = 0;
    parse->radix_digits = 0;
}

/* Maximum octal value that can be held in three digits */
#define RE__PARSE_OCT_NUM_MAX (8 * 8 * 8)
#define RE__PARSE_OCT_DIGITS_MAX 3
#define RE__PARSE_IS_OCT_DIGIT(ch) \
    ((ch) == '0' || (ch) == '1' || (ch) == '2' || (ch) == '3' || (ch) == '4' || (ch) == '5' || (ch) == '6' || (ch) == '7')

/* Parse in a single octal digit. */
RE_INTERNAL re_error re__parse_radix_consume_oct(re__parse* parse, re_rune ch) {
    if (parse->radix_digits == RE__PARSE_OCT_DIGITS_MAX) {
        return re__parse_error(parse, "octal literal exceeds maximum of three digits");
    }
    RE_ASSERT(RE__PARSE_IS_OCT_DIGIT(ch));
    parse->radix_num *= 8;
    parse->radix_num += ((re_uint32)ch) - '0';
    parse->radix_digits++;
    if (parse->radix_num >= RE__PARSE_OCT_NUM_MAX) {
        return re__parse_error(parse, "octal literal exceeds maximum value of \\777");
    }
    return RE_ERROR_NONE;
}

/* Ensure that the radix doesn't exceed octal limits. */
RE_INTERNAL re_error re__parse_radix_check_oct(re__parse* parse) {
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

RE_INTERNAL re_rune re__parse_radix_hex_digit(re_rune dig) {
    if (dig >= '0' && dig <= '9') {
        return dig - '0';
    } else if (dig >= 'A' && dig <= 'F') {
        return (dig - 'A') + 10;
    } else if (dig >= 'a' && dig <= 'f') {
        return (dig - 'a') + 10;
    } else {
        RE__ASSERT_UNREACHED();
        return 0;
    }
}

/* Parse in a single hex digit in short format (\x..) */
RE_INTERNAL re_error re__parse_radix_consume_hex_short(re__parse* parse, re_rune ch) {
    if (parse->radix_digits == RE__PARSE_HEX_SHORT_DIGITS_MAX) {
        return re__parse_error(parse, "short hex literal exceeds maximum of two digits");
    }
    RE_ASSERT(RE__PARSE_IS_HEX_DIGIT(ch));
    parse->radix_num *= 16;
    parse->radix_num += re__parse_radix_hex_digit(ch);
    parse->radix_digits++;
    if (parse->radix_num >= RE__PARSE_HEX_SHORT_NUM_MAX) {
        return re__parse_error(parse, "short hex literal exceeds maximum value of \\xFF");
    }
    return RE_ERROR_NONE;
}

/* Ensure that the radix doesn't exceed hex limits. */
RE_INTERNAL re_error re__parse_radix_check_hex_short(re__parse* parse) {
    if (parse->radix_num >= RE__PARSE_HEX_SHORT_NUM_MAX) {
        return re__parse_error(parse, "short hex literal exceeds maximum value of \\xFF");
    }
    if (parse->radix_digits > RE__PARSE_HEX_SHORT_DIGITS_MAX) {
        return re__parse_error(parse, "short hex literal exceeds maximum of two digits");
    }
    return RE_ERROR_NONE;
}

#define RE__PARSE_HEX_LONG_NUM_MAX RE_RUNE_MAX
#define RE__PARSE_HEX_LONG_DIGITS_MAX 6

/* Parse in a single hex digit in long format (\x{....}) */
RE_INTERNAL re_error re__parse_radix_consume_hex_long(re__parse* parse, re_rune ch) {
    if (parse->radix_digits == RE__PARSE_HEX_LONG_DIGITS_MAX) {
        return re__parse_error(parse, "long hex literal exceeds maximum of six digits");
    }
    RE_ASSERT(RE__PARSE_IS_HEX_DIGIT(ch));
    parse->radix_num *= 16;
    parse->radix_num += re__parse_radix_hex_digit(ch);
    parse->radix_digits++;
    if (parse->radix_num >= RE__PARSE_HEX_LONG_NUM_MAX) {
        return re__parse_error(parse, "long hex literal exceeds maximum value of \\x{10FFFF}");
    }
    return RE_ERROR_NONE;
}

/* Ensure that the radix doesn't exceed hex limits. */
RE_INTERNAL re_error re__parse_radix_check_hex_long(re__parse* parse) {
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
RE_INTERNAL re_error re__parse_radix_consume_counting(re__parse* parse, re_rune ch) {
    if (parse->radix_digits == RE__PARSE_COUNTING_DIGITS_MAX) {
        return re__parse_error(parse, "counting literal exceeds maximum of four digits");
    }
    RE_ASSERT(RE__PARSE_IS_DEC_DIGIT(ch));
    parse->radix_num *= 10;
    parse->radix_num += ch - '0';
    parse->radix_digits++;
    if (parse->radix_num >= RE__AST_QUANTIFIER_MAX) {
        return re__parse_error(parse, "counting literal exceeds maximum value of " RE__STRINGIFY(RE__AST_QUANTIFIER_MAX) );
    }
    return RE_ERROR_NONE;
}

/* Ensure that the radix doesn't exceed decimal counting limits. */
RE_INTERNAL re_error re__parse_radix_check_counting(re__parse* parse) {
    if (parse->radix_num > RE__AST_QUANTIFIER_MAX) {
        return re__parse_error(parse, "counting literal exceeds maximum value of " RE__STRINGIFY(RE__AST_QUANTIFIER_MAX) );
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
RE_INTERNAL re_error re__parse_disallow_escape_in_charclass(re__parse* parse, re_rune esc) {
    re_error err = RE_ERROR_NONE;
    re__parse_op top;
    re__str err_str;
    RE_ASSERT(re__parse_op_vec_size(&parse->op_stk) > 0);
    top = re__parse_op_vec_peek(&parse->op_stk);
    if (top.ret_state != RE__PARSE_STATE_GND) {
        /* Build error message */
        re_char esc_ch = (re_char)(esc);
        if ((err = re__str_init_s(&err_str, (const re_char*)"cannot use escape sequence '\\"))) {
            goto destroy_err_str;
        }
        if ((err = re__str_cat_n(&err_str, 1, &esc_ch))) {
            goto destroy_err_str;
        }
        if ((err = re__str_cat_s(&err_str, (const re_char*)"' from within character class (\"[]\")"))) {
            goto destroy_err_str;
        }
        re__set_error_str(parse->re, &err_str);
    }
destroy_err_str:
    re__str_destroy(&err_str);
    return err;
}

/* Create a new assert */
RE_INTERNAL re_error re__parse_create_assert(re__parse* parse, re__ast_assert_type assert_type) {
    re__ast new_node;
    re__ast_init_assert(&new_node, assert_type);
    return re__parse_add_new_node(parse, new_node);
}

/* Create a new "any byte" (\C) */
RE_INTERNAL re_error re__parse_create_any_byte(re__parse* parse) {
    re__ast new_node;
    re__ast_init_any_byte(&new_node);
    return re__parse_add_new_node(parse, new_node);
}

/* Create a new "any char" (.) */
RE_INTERNAL re_error re__parse_create_any_char(re__parse* parse) {
    re__ast new_node;
    re__ast_init_any_char(&new_node);
    return re__parse_add_new_node(parse, new_node);
}

/* Depending on the current state, add a character class *into* a character
 * class, *OR* add a character class AST node. */
RE_INTERNAL re_error re__parse_ast_or_charclass_create_default(re__parse* parse, re__charclass_ascii_type ascii_cc, int inverted) {
    re__parse_state top_state;
    re__charclass new_class;
    re_error err = RE_ERROR_NONE;
    top_state = re__parse_op_vec_peek(&parse->op_stk).ret_state;
    /* Create the character class */
    if ((err = re__charclass_init_from_class(&new_class, ascii_cc, inverted))) {
        return err;
    }
    if (top_state == RE__PARSE_STATE_GND) {
        /* Wrap it in an AST node */
        re__ast new_node;
        re__ast_init_class(&new_node, new_class);
        /* new_node now owns new_class */
        if ((err = re__parse_add_new_node(parse, new_node))) {
            return err;
        }
    } else {
        if (parse->charclass_lo_rune != -1) {
            return re__parse_error(parse, "cannot have character class as upper bound of character range");
        }
        /* Add it to the charclass builder */
        if ((err = re__charclass_builder_insert_class(&parse->charclass_builder, &new_class))) {
            /* destroy charclass */
            re__charclass_destroy(&new_class);
            return err;
        }
        re__charclass_destroy(&new_class);
    }
    return err;
}

RE_INTERNAL void re__parse_charclass_begin(re__parse* parse) {
    re__charclass_builder_begin(&parse->charclass_builder);
    parse->charclass_lo_rune = -1;
}

RE_INTERNAL void re__parse_charclass_setlo(re__parse* parse, re_rune ch) {
    parse->charclass_lo_rune = -1;
}

RE_INTERNAL re_error re__parse_charclass_addlo(re__parse* parse) {
    re_error err = RE_ERROR_NONE;
    re__runerange new_range;
    RE_ASSERT(parse->charclass_lo_rune != -1);
    new_range.lo = parse->charclass_lo_rune;
    new_range.hi = parse->charclass_lo_rune + 1;
    parse->charclass_lo_rune = -1;
    if ((err = re__charclass_builder_insert_range(&parse->charclass_builder, new_range))) {
        return err;
    }
    return err;
}

RE_INTERNAL re_error re__parse_charclass_addhi(re__parse* parse, re_rune ch) {
    re_error err = RE_ERROR_NONE;
    re__runerange new_range;
    RE_ASSERT(parse->charclass_lo_rune != -1);
    new_range.lo = parse->charclass_lo_rune;
    new_range.hi = ch + 1;
    parse->charclass_lo_rune = -1;
    if ((err = re__charclass_builder_insert_range(&parse->charclass_builder, new_range))) {
        return err;
    }
    return err;
}


RE_INTERNAL re_error re__parse_charclass_finish(re__parse* parse) {
    re__ast new_node;
    re__charclass new_charclass;
    re_error err = RE_ERROR_NONE;
    if ((err = re__charclass_builder_finish(&parse->charclass_builder, &new_charclass))) {
        return err;
    }
    re__ast_init_class(&new_node, new_charclass);
    if ((err = re__parse_add_new_node(parse, new_node))) {
        return err;
    }
}

/* Depending on the current state, push a new character AST node, or add the
 * given character to the current character class. */
RE_INTERNAL re_error re__parse_ast_or_charclass_create_char(re__parse* parse, re_rune ch) {
    re__parse_state top_state;
    re_error err = RE_ERROR_NONE;
    /* This can only be called on GND or a charclass builder state. */
    RE_ASSERT(parse->state == RE__PARSE_STATE_GND);
    RE_ASSERT(re__parse_op_vec_size(&parse->op_stk) > 0);
    top_state = re__parse_op_vec_peek(&parse->op_stk).ret_state;
    if (top_state == RE__PARSE_STATE_GND) {
        /* Wrap it in an AST node */
        re__ast new_node;
        re__ast_init_rune(&new_node, ch);
        if ((err = re__parse_add_new_node(parse, new_node))) {
            return err;
        }
    } else {
        if ((err = re__parse_charclass_feed_char(parse, ch))) {
            return err;
        }
    }
    return err;
}

/* Depending on the current state, push a new character AST node, or add the
 * given character to the current character class. */
RE_INTERNAL re_error re__parse_finish_escape_char(re__parse* parse, re_rune ch) {
    re__parse_state top_state;
    re_error err = RE_ERROR_NONE;
    /* This can only be called on GND or a charclass builder state. */
    RE_ASSERT(parse->state == RE__PARSE_STATE_GND);
    RE_ASSERT(re__parse_op_vec_size(&parse->op_stk) > 0);
    top_state = re__parse_op_vec_peek(&parse->op_stk).ret_state;
}

RE_INTERNAL void re__parse_defer(re__parse* parse) {
    parse->defer = 1;
}

RE_INTERNAL re_error re__parse_create_star(re__parse* parse) {
    re__ast new_star;
    if (re__parse_is_frame_empty(parse)) {
        return re__parse_error(parse, "cannot use '*' operator with nothing");
    }
    re__ast_init_quantifier(&new_star, 0, RE__AST_QUANTIFIER_INFINITY);
    re__ast_set_quantifier_greediness(&new_star, !!(parse->group_flags & RE__AST_GROUP_FLAG_UNGREEDY));
    return re__parse_wrap_node(parse, new_star);
}

RE_INTERNAL re_error re__parse_create_plus(re__parse* parse) {
    re__ast new_plus;
    if (re__parse_is_frame_empty(parse)) {
        return re__parse_error(parse, "cannot use '+' operator with nothing");
    }
    re__ast_init_quantifier(&new_plus, 1, RE__AST_QUANTIFIER_INFINITY);
    re__ast_set_quantifier_greediness(&new_plus, !!!(parse->group_flags & RE__AST_GROUP_FLAG_UNGREEDY));
    return re__parse_wrap_node(parse, new_plus);
}

RE_INTERNAL re_error re__parse_create_question(re__parse* parse) {
    re__ast new_question;
    if (re__parse_is_frame_empty(parse)) {
        return re__parse_error(parse, "cannot use '?' operator with nothing");
    }
    re__ast_init_quantifier(&new_question, 0, 2);
    re__ast_set_quantifier_greediness(&new_question, !!!(parse->group_flags & RE__AST_GROUP_FLAG_UNGREEDY));
    return re__parse_wrap_node(parse, new_question);
}

RE_INTERNAL re_error re__parse_create_repeat(re__parse* parse, re_int32 min, re_int32 max) {
    re__ast new_question;
    if (re__parse_is_frame_empty(parse)) {
        return re__parse_error(parse, "cannot use '{' operator with nothing");
    }
    if (max <= min) {
        return re__parse_error(parse, "minimum repetition count must be less than maximum repetition count for '{' operator");
    }
    re__ast_init_quantifier(&new_question, min, max);
    re__ast_set_quantifier_greediness(&new_question, !!!(parse->group_flags & RE__AST_GROUP_FLAG_UNGREEDY));
    return re__parse_wrap_node(parse, new_question);
}

RE_INTERNAL void re__parse_swap_greedy(re__parse* parse) {
    re__ast* quant;
    /* Cannot make nothing ungreedy */
    RE_ASSERT(!re__parse_is_frame_empty(parse));
    quant = re__ast_vec_getref(&parse->ast_stk, parse->ast_prev_child_ptr);
    /* Must be a quantifier */
    RE_ASSERT(quant->type == RE__AST_TYPE_QUANTIFIER);
    re__ast_set_quantifier_greediness(quant, !re__ast_get_quantifier_greediness(quant));
}

#define RE__IS_LAST() (ch == -1)
/* This macro is only used within re__parse_regex. */
/* Try-except encourages forgetting to clean stuff up, but the constraints on
 * code within this function allow us to always use this macro safely. */
/* I think it's a good design choice. */
#define RE__TRY(expr) \
    if ((err = expr)) { \
        goto error; \
    }

RE_INTERNAL re_error re__parse_regex(re__parse* parse, re_size regex_size, const re_char* regex) {
    /*const re_char* start = regex;*/
    const re_char* end = regex + regex_size;
    re_error err = RE_ERROR_NONE;
    re__ast new_group;
    /* Start by pushing the initial GROUP node. */
    re__ast_init_group(&new_group);
    if ((err = re__ast_vec_push(&parse->ast_stk, new_group))) {
        return err;
    }
    /* Set stack/previous pointers accordingly. */
    parse->ast_stk_ptr = 1;
    parse->ast_prev_child_ptr = 1;
    /* Push the base op (group) */
    if ((err = re__parse_push_frame(parse))) {
        return err;
    }
    /* Set the frame pointer to the group node. */
    parse->ast_frame_ptr = 0;
    while (regex <= end) {
        /* ch will only be -1 if the if this is the last character, a.k.a.
         * "epsilon" as all the cool kids call it */
        re_rune ch = -1;
        /* Otherwise ch is the character in question */
        if (regex < end) {
            ch = *regex;
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
                RE__TRY(re__parse_create_assert(parse, RE__AST_ASSERT_TYPE_TEXT_END));
            } else if (ch == '(') {
                /* (: Begin a group. */
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
                RE__TRY(re__parse_push_frame(parse));
                parse->state = RE__PARSE_STATE_ESCAPE;  
            } else if (ch == '^') {
                /* ^: Text start assert. */
                RE__TRY(re__parse_create_assert(parse, RE__AST_ASSERT_TYPE_TEXT_START));
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
                RE__TRY(re__parse_char(parse, (re_char)ch));
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
                re__parse_pop_frame(parse);
                RE__TRY(re__parse_create_assert(parse, RE__AST_ASSERT_TYPE_TEXT_START_ABSOLUTE));
            } else if (ch == 'B') {
                /* \B: Not a word boundary */
                RE__TRY(re__parse_disallow_escape_in_charclass(parse, ch));
                re__parse_pop_frame(parse);
                RE__TRY(re__parse_create_assert(parse, RE__AST_ASSERT_TYPE_WORD_NOT));
            } else if (ch == 'C') {
                /* \C: Any *byte* (NOT any char) */
                RE__TRY(re__parse_disallow_escape_in_charclass(parse, ch));
                re__parse_pop_frame(parse);
                RE__TRY(re__parse_create_any_byte(parse));
            } else if (ch == 'D') {
                /* \D: Non-digit characters */
                re__parse_pop_frame(parse);
                RE__TRY(re__parse_ast_or_charclass_create_default(parse, RE__CHARCLASS_ASCII_TYPE_DIGIT, 1));
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
                re__parse_pop_frame(parse);
                parse->state = RE__PARSE_STATE_QUOTE;
            } else if (ch == 'R') {
                /* \R: Invalid */
                RE__TRY(re__parse_error_invalid_escape(parse, ch));
            } else if (ch == 'S') {
                /* \S: Not whitespace (Perl) */
                re__parse_pop_frame(parse);
                RE__TRY(re__parse_ast_or_charclass_create_default(parse, RE__CHARCLASS_ASCII_TYPE_PERL_SPACE, 1));
            } else if (ch == 'T' || ch == 'U' || ch == 'V') {
                /* \T - \V: Invalid */
                RE__TRY(re__parse_error_invalid_escape(parse, ch));
            } else if (ch == 'W') {
                /* \W: Not a word character */
                re__parse_pop_frame(parse);
                RE__TRY(re__parse_ast_or_charclass_create_default(parse, RE__CHARCLASS_ASCII_TYPE_WORD, 1));
            } else if (ch == 'X' || ch == 'Y' || ch == 'Z') {
                /* \X - \Z: Invalid */
                RE__TRY(re__parse_error_invalid_escape(parse, ch));
            } else if (ch == 'a') {
                /* \a: Bell character */
                re__parse_pop_frame(parse);
                RE__TRY(re__parse_ast_or_charclass_create_char(parse, '\a'));
            } else if (ch == 'b') {
                /* \b: Word boundary */
                RE__TRY(re__parse_disallow_escape_in_charclass(parse, ch));
                re__parse_pop_frame(parse);
                RE__TRY(re__parse_ast_or_charclass_create_default(parse, RE__CHARCLASS_ASCII_TYPE_WORD, 0));
            } else if (ch == 'c') {
                /* \c: Invalid */
                RE__TRY(re__parse_error_invalid_escape(parse, ch));
            } else if (ch == 'd') {
                /* \d: Digit characters */
                RE__TRY(re__parse_disallow_escape_in_charclass(parse, ch));
                re__parse_pop_frame(parse);
                RE__TRY(re__parse_ast_or_charclass_create_default(parse, RE__CHARCLASS_ASCII_TYPE_DIGIT, 0));
            } else if (ch == 'e') {
                /* \e: Invalid */
                RE__TRY(re__parse_error_invalid_escape(parse, ch));
            } else if (ch == 'f') {
                /* \f: Form feed character */
                re__parse_pop_frame(parse);
                RE__TRY(re__parse_ast_or_charclass_create_char(parse, '\f'));
            } else if (ch == 'g' || ch == 'h' || ch == 'i' || ch == 'j' || ch == 'k' || ch == 'l' || ch == 'm') {
                /* \g - \m: Invalid */
                RE__TRY(re__parse_error_invalid_escape(parse, ch));
            } else if (ch == 'n') {
                /* \n: Newline */
                re__parse_pop_frame(parse);
                RE__TRY(re__parse_ast_or_charclass_create_char(parse, '\n'));
            } else if (ch == 'o') {
                /* \o: Invalid */
                RE__TRY(re__parse_error_invalid_escape(parse, ch));
            } else if (ch == 'p') {
                /* \p: Unicode character class */
                RE__TRY(re__parse_error(parse, "unimplemented"));
            } else if (ch == 'r') {
                /* \r: Carriage return */
                re__parse_pop_frame(parse);
                RE__TRY(re__parse_ast_or_charclass_create_char(parse, '\r'));
            } else if (ch == 's') {
                /* \s: Whitespace (Perl) */
                re__parse_pop_frame(parse);
                RE__TRY(re__parse_ast_or_charclass_create_default(parse, RE__CHARCLASS_ASCII_TYPE_PERL_SPACE, 0));
            } else if (ch == 't') {
                /* \t: Horizontal tab */
                re__parse_pop_frame(parse);
                RE__TRY(re__parse_ast_or_charclass_create_char(parse, '\t'));
            } else if (ch == 'u') {
                /* \u: Invalid */
                RE__TRY(re__parse_error_invalid_escape(parse, ch));
            } else if (ch == 'v') {
                /* \v: Vertical tab */
                re__parse_pop_frame(parse);
                RE__TRY(re__parse_ast_or_charclass_create_char(parse, '\v'));
            } else if (ch == 'w') {
                /* \w: Word character */
                re__parse_pop_frame(parse);
                RE__TRY(re__parse_ast_or_charclass_create_default(parse, RE__CHARCLASS_ASCII_TYPE_WORD, 0));
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
                re__parse_pop_frame(parse);
                RE__TRY(re__parse_create_assert(parse, RE__AST_ASSERT_TYPE_TEXT_END_ABSOLUTE));
            } else {
                /* All other characters */
                re__parse_pop_frame(parse);
                RE__TRY(re__parse_ast_or_charclass_create_char(parse, ch));
            }
        } else if (parse->state == RE__PARSE_STATE_PARENS_INITIAL) {
            /* Start of group: ( */
            if (RE__IS_LAST()) {
                re__parse_defer(parse);
                parse->state = RE__PARSE_STATE_GND;
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
                RE__TRY(re__parse_error(parse, "expected one of '-', ':', 'P', 'U', 'i', 'm', 's' for group flags or name"));
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
            } else if (ch == 'P') {
                /* (?P: Start of group name */
                parse->str_begin = regex;
                parse->str_end = regex;
                parse->state = RE__PARSE_STATE_PARENS_NAME_INITIAL;
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
                RE__TRY(re__parse_error(parse, "expected one of ':', 'P', 'U', 'i', 'm', 's' for negated group flags or name"));
            } else if (ch == ')') {
                /* (?): Go back to GND without creating a group, retain flags */
                parse->group_flags = parse->group_flags_new;
                parse->state = RE__PARSE_STATE_GND; 
            } else if (ch == ':') {
                /* (?:: Non-matching group, also signals end of flags */
                parse->group_flags_new &= ~((unsigned int)RE__AST_GROUP_FLAG_NONMATCHING);
                parse->state = RE__PARSE_STATE_PARENS_AFTER_COLON;
            } else if (ch == 'P') {
                /* (?P: Start of group name */
                parse->str_begin = regex;
                parse->str_end = regex;
                parse->state = RE__PARSE_STATE_PARENS_NAME_INITIAL;
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
                parse->str_begin = regex+1;
                parse->str_end = parse->str_begin;
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
                parse->str_end++;
                parse->state = RE__PARSE_STATE_PARENS_NAME;
            }
        } else if (parse->state == RE__PARSE_STATE_PARENS_NAME) {
            /* Second+ character between <> in named group: (?P<> */
            if (RE__IS_LAST()) {
                RE__TRY(re__parse_error(parse, "expected '>' to close group name"));
            } else if (ch == '>') {
                /* (?P<...>: End of group name, begin group and defer to GND */
                re__parse_defer(parse);
                parse->state = RE__PARSE_STATE_GND;
                RE__TRY(re__parse_group_begin(parse));
            } else {
                /* (?P<...: Name character, append to name */
                parse->str_end++;
            }
        } else if (parse->state == RE__PARSE_STATE_OCTAL_SECOND_DIGIT) {
            /* Second digit in an octal literal: \0 */
            if (RE__IS_LAST()) {
                /* Last: push character and defer */
                RE__TRY(re__parse_radix_check_oct(parse));
                re__parse_pop_frame(parse);
                re__parse_defer(parse);
                RE__TRY(re__parse_ast_or_charclass_create_char(parse, parse->radix_num));
            } else if (RE__PARSE_IS_OCT_DIGIT(ch)) {
                /* \.x: add to the radix accumulator */
                RE__TRY(re__parse_radix_consume_oct(parse, ch));
                RE__TRY(re__parse_radix_check_oct(parse));
                parse->state = RE__PARSE_STATE_OCTAL_THIRD_DIGIT;
            } else {
                /* Other characters: push character and defer */
                RE__TRY(re__parse_radix_check_oct(parse));
                re__parse_pop_frame(parse);
                re__parse_defer(parse);
                RE__TRY(re__parse_ast_or_charclass_create_char(parse, parse->radix_num));
            }
        } else if (parse->state == RE__PARSE_STATE_OCTAL_THIRD_DIGIT) {
            /* Third digit in an octal literal: \00 */
            if (RE__IS_LAST()) {
                /* Last: push character and defer */
                RE__TRY(re__parse_radix_check_oct(parse));
                re__parse_pop_frame(parse);
                re__parse_defer(parse);
                RE__TRY(re__parse_ast_or_charclass_create_char(parse, parse->radix_num));
            } else if (RE__PARSE_IS_OCT_DIGIT(ch)) {
                /* \.x: add to the radix accumulator and finish */
                RE__TRY(re__parse_radix_consume_oct(parse, ch));
                RE__TRY(re__parse_radix_check_oct(parse));
                re__parse_pop_frame(parse);
                RE__TRY(re__parse_ast_or_charclass_create_char(parse, parse->radix_num));
            } else {
                /* Other characters: push character and defer */
                RE__TRY(re__parse_radix_check_oct(parse));
                re__parse_pop_frame(parse);
                re__parse_defer(parse);
                RE__TRY(re__parse_ast_or_charclass_create_char(parse, parse->radix_num));
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
                re__parse_pop_frame(parse);
                RE__TRY(re__parse_ast_or_charclass_create_char(parse, parse->radix_num));
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
                re__parse_pop_frame(parse);
                RE__TRY(re__parse_ast_or_charclass_create_char(parse, parse->radix_num));
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
                re__parse_char(parse, (re_char)ch);
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
                re__parse_char(parse, (re_char)ch);
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
                parse->counting_first_num = parse->radix_num;
                re__parse_radix_clear(parse);
                parse->state = RE__PARSE_STATE_COUNTING_SECOND_NUM;
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
                /* [[: Literal [ or char class*/
                parse->state = RE__PARSE_STATE_CHARCLASS_AFTER_BRACKET;
            } else if (ch == '\\') {
                /* [\: Escape */
                /* Set return state to AFTERLO */
                parse->state = RE__PARSE_STATE_CHARCLASS_AFTER_LO;
                RE__TRY(re__parse_push_frame(parse));
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
                RE__TRY(re__parse_push_frame(parse));
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
                parse->state = RE__PARSE_STATE_CHARCLASS_AFTER_LO;
                RE__TRY(re__parse_push_frame(parse));
                parse->state = RE__PARSE_STATE_ESCAPE;
            } else if (ch == ']') {
                /* []: A single [. */
                parse->state = RE__PARSE_STATE_GND;
                re__parse_charclass_setlo(parse, ']');
                RE__TRY(re__parse_charclass_addlo(parse));
                RE__TRY(re__parse_charclass_finish(parse));
            } else if (ch == ':') {
                /* [:: Start of ASCII charclass */
                parse->state = RE__PARSE_STATE_CHARCLASS_NAMED;
            } else {
                /* Otherwise, add the bracket and the character */
                re__parse_charclass_setlo(parse, ']');
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
                RE__TRY(re__parse_push_frame(parse));
                parse->state = RE__PARSE_STATE_ESCAPE;
            } else if (ch == ']') {
                /* [.]: Finish. */
                parse->state = RE__PARSE_STATE_GND;
                RE__TRY(re__parse_charclass_addlo(parse));
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
                parse->state = RE__PARSE_STATE_CHARCLASS_AFTER_LO;
                RE__TRY(re__parse_push_frame(parse));
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
                RE__TRY(re__parse_push_frame(parse));
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
        } else {
            RE__ASSERT_UNREACHED();
        }
        if (parse->defer) {
            parse->defer -= 1;
        } else {
            regex++;
        }
    }
    re__ast_debug_dump(re__ast_vec_get_data(&parse->ast_stk), 0);
    return RE_ERROR_NONE;
error:
    if (err == RE_ERROR_PARSE) {
        RE_ASSERT(re__str_size(&parse->re->data->error_string));
    } else {
        re__set_error_generic(parse->re, err);
    }
    return err;
}

#undef RE__IS_LAST
#undef RE__TRY

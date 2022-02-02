#include "re_internal.h"

typedef enum re__parse_state {
    RE__PARSE_STATE_GND,
    RE__PARSE_STATE_MAYBE_QUESTION,
    RE__PARSE_STATE_ESCAPE,
    RE__PARSE_STATE_PARENS_INITIAL,
    RE__PARSE_STATE_PARENS_FLAG_INITIAL,
    RE__PARSE_STATE_PARENS_FLAG,
    RE__PARSE_STATE_PARENS_FLAG_NAMED_INITIAL,
    RE__PARSE_STATE_PARENS_FLAG_NAMED,
    RE__PARSE_STATE_QUOTE,
    RE__PARSE_STATE_QUOTE_ESCAPE,
    RE__PARSE_STATE_HEX_INITIAL,
    RE__PARSE_STATE_HEX_SECOND_DIGIT,
    RE__PARSE_STATE_HEX_EXTENDED,
    RE__PARSE_STATE_OCTAL_SECOND_DIGIT,
    RE__PARSE_STATE_OCTAL_THIRD_DIGIT,
    RE__PARSE_STATE_CHARCLASS_INITIAL,
    RE__PARSE_STATE_CHARCLASS_AFTER_CARET,
    RE__PARSE_STATE_CHARCLASS_AFTER_RANGE_BEGIN,
    RE__PARSE_STATE_CHARCLASS_AFTER_HYPHEN,
    RE__PARSE_STATE_CHARCLASS_NAMED_INITIAL,
    RE__PARSE_STATE_CHARCLASS_NAMED_NAME_INITIAL,
    RE__PARSE_STATE_CHARCLASS_NAMED_NAME,
    RE__PARSE_STATE_CHARCLASS_NAMED_FINAL,
    RE__PARSE_STATE_UNICODE_CHARCLASS_BEGIN,
    RE__PARSE_STATE_UNICODE_CHARCLASS_NAMED_NAME,
    RE__PARSE_STATE_COUNTING_FIRST_NUM_INITIAL,
    RE__PARSE_STATE_COUNTING_FIRST_NUM,
    RE__PARSE_STATE_COUNTING_SECOND_NUM_INITIAL,
    RE__PARSE_STATE_COUNTING_SECOND_NUM
} re__parse_state;

typedef enum re__parse_op_type {
    /* Group / parentheses */
    RE__PARSE_OP_TYPE_PARENS,
    /* Alternation */
    RE__PARSE_OP_TYPE_ALT,
    /* Escape character */
    RE__PARSE_OP_TYPE_ESCAPE
} re__parse_op_type;

/* A regex operator.
 * We keep a list of these operators as if they are functions on a virtual
 * stack, with the base_ptr field pointing to the first sub-expression or
 * argument to the operator. */
typedef struct re__parse_op {
    /* Type of operation */
    re__parse_op_type op_type;
    /* Base of stack (the first argument to this operation) */
    re_size base_ptr;
    /* Group flags -- when parsing a group we have to set these flags */
    re__ast_group_flags group_flags;
    /* Calling state (where to return to after this is popped) */
    re__parse_state ret_state;
    /* Literal string (for group - RE__PARSE_OP_TYPE_PARENS) */
    re__str literal;
} re__parse_op;

RE_VEC_DECL(re__parse_op);
RE_VEC_IMPL_FUNC(re__parse_op, init)
RE_VEC_IMPL_FUNC(re__parse_op, destroy)
RE_VEC_IMPL_FUNC(re__parse_op, push)
RE_VEC_IMPL_FUNC(re__parse_op, pop)
RE_VEC_IMPL_FUNC(re__parse_op, peek)
RE_VEC_IMPL_FUNC(re__parse_op, size)

RE_VEC_IMPL_FUNC(re__ast, cat)

typedef struct re__parser {
    /* Parent regex */
    re* re;
    /* Path to the current ast node being built */
    re__ast_vec ast_stk;
    /* Stack of pending operations to perform on previous nodes */
    re__parse_op_vec op_stk;
    /* Pointer to current stack base */
    re_size base_ptr;
    /* Current parser state */
    enum re__parse_state state;
    /* Defers parsing of current character to the next loop iteration */
    int defer;
    /* Number used when parsing an integer of a given radix */
    re_uint32 radix_num;
    /* Amount of digits currently used by the number */
    int radix_digits;
    /* For counting forms: the lower and upper bounds */
    re_int32 counting_lo;
    re_int32 counting_hi;
    /* For character classes: current building state */
    re__charclass_builder charclass_builder;
    /* Current group flags */
    re__ast_group_flags group_flags;
    /* For \Q...\E, group names, named unicode char classes */
    re__str literal;
} re__parser;

RE_INTERNAL void re__parser_init(re__parser* parser, re* re) {
    parser->re = re;

    /* Initialize stacks */
    re__ast_vec_init(&parser->ast_stk);
    re__parse_op_vec_init(&parser->op_stk);

    parser->base_ptr = 0;
    parser->state = RE__PARSE_STATE_GND;
    parser->defer = 0;
    parser->radix_num = 0;
    parser->radix_digits = 0;
    parser->counting_lo = -1;
    parser->counting_hi = -1;
    parser->group_flags = 0;

    re__charclass_builder_init(&parser->charclass_builder);
    re__str_init(&parser->literal);
}

RE_INTERNAL re_error re__parser_destroy(re__parser* parser) {
    re_error err = RE_ERROR_NONE;
    re__str_destroy(&parser->literal);
    re__charclass_builder_destroy(&parser->charclass_builder);
    /* Clean up */
    while (re__ast_vec_size(&parser->ast_stk)) {
        re__ast popped_node = re__ast_vec_pop(&parser->ast_stk);
        /* We attempt to destroy every node. But only return error from the last
         * one. */
        err = re__ast_destroy(&popped_node);
    }
    re__ast_vec_destroy(&parser->ast_stk);
    /* Clean up literals inside of operator stack */
    /* Most of these have SSO enabled, so doesn't incur too much of a
     * performance penalty */
    while (re__parse_op_vec_size(&parser->op_stk)) {
        re__parse_op popped_op = re__parse_op_vec_pop(&parser->op_stk);
        re__str_destroy(&popped_op.literal);
    }
    re__parse_op_vec_destroy(&parser->op_stk);
    return err;
}

RE_INTERNAL re_error re__parser_error(re__parser* parser, const char* err_chars) {
    re__str err_str;
    re_error err = re__str_init_s(&err_str, (const re_char*)err_chars);
    if (err) {
        return err;
    }
    re__set_error_str(parser->re, &err_str);
    re__str_destroy(&err_str);
    return RE_ERROR_PARSE;
}

RE_INTERNAL re_error re__parser_op_push(re__parser* parser, re__parse_op_type op_type) {
    re_error err = RE_ERROR_NONE;
    re__parse_op op;
    op.op_type = op_type;
    op.base_ptr = parser->base_ptr;
    op.group_flags = parser->group_flags;
    op.ret_state = parser->state;
    /* We only use the `literal` field when op_type is PARENS to
     * store the group name. */
    /* Otherwise, parser->literal never needs to be pushed -- the
     * other states that handle it automatically free it when
     * necessary. */
    /* Copy-move parser->literal to the op */
    re__str_move(&op.literal, &parser->literal);
    return re__parse_op_vec_push(&parser->op_stk, op);
}

RE_INTERNAL re__parse_op re__parser_op_pop(re__parser* parser) {
    re__parse_op out;
    RE_ASSERT(re__parse_op_vec_size(&parser->op_stk) > 0);
    out = re__parse_op_vec_pop(&parser->op_stk);
    parser->base_ptr = out.base_ptr;
    parser->group_flags = out.group_flags;
    parser->state = out.ret_state;
    re__str_destroy(&parser->literal);
    parser->literal = out.literal;
    return out;
}

RE_INTERNAL void re__parser_op_pop_expect(re__parser* parser, re__parse_op_type op_type) {
    re__parse_op out = re__parser_op_pop(parser);
    RE_ASSERT(out.op_type == op_type);
}

/* Combine two RE__AST_TYPE_CHARs, two RE__AST_TYPE_STRINGs, or a mixture of the
 * two into a single RE__AST_TYPE_STRING. */
RE_INTERNAL re_error re__parse_fuse_char_str(re__ast e1, re__ast e2, re__ast* res) {
    /* In this function, we have ownership of e1 and e2. */
    re_error err = RE_ERROR_NONE;
    if (e1.type == RE__AST_TYPE_CHAR) {
        if (e2.type == RE__AST_TYPE_CHAR) {
            /* e1 == char, e2 == char */
            /* new string with two characters */
            re__str* str;
            re__ast_init_char_string(res);
            /* Note: str is a REFERENCE (do not free it!) */
            str = re__ast_get_str(res);
            if ((err = re__str_push(str, re__ast_get_char(&e1)))) {
                goto destroy_e1_e2_res;
            }
            if ((err = re__str_push(str, re__ast_get_char(&e2)))) {
                goto destroy_e1_e2_res;
            }
            /* e1, e2 are both consumed */
            if ((err = re__ast_destroy(&e1))) {
                goto destroy_e2_res;
            }
            if ((err = re__ast_destroy(&e2))) {
                goto destroy_res;
            }
        } else if (e2.type == RE__AST_TYPE_CHAR_STRING) {
            /* e1 == char, e2 == char str */
            /* insert e1 before e2 */
            re__str* str = re__ast_get_str(&e2);
            if ((err = re__str_insert(str, 0, re__ast_get_char(&e1)))) {
                goto destroy_e1_e2;
            }
            /* set result to newly inserted e2 */
            *res = e2;
            /* e1 is consumed */
            if ((err = re__ast_destroy(&e1))) {
                goto destroy_e2;
            }
        }
    } else if (e1.type == RE__AST_TYPE_CHAR_STRING) {
        if (e2.type == RE__AST_TYPE_CHAR) {
            /* e1 == char_str, e2 == char */
            /* append e2 to e1 */
            re__str* str = re__ast_get_str(&e1);
            if ((err = re__str_push(str, re__ast_get_char(&e2)))) {
                goto destroy_e1_e2;
            }
            /* set result to newly pushed e1 */
            *res = e1;
            /* e2 is consumed */
            if ((err = re__ast_destroy(&e2))) {
                goto destroy_e1;
            }
        } else if (e2.type == RE__AST_TYPE_CHAR_STRING) {
            /* e1 == char_str, e2 == char_str */
            /* concatenate e1, e2 */
            re__str* a = re__ast_get_str(&e1);
            re__str* b = re__ast_get_str(&e2);
            if ((err = re__str_cat(a, b))) {
                return err;
            }
            /* set result to concatenated e1 */
            *res = e1;
            /* e2 is consumed */
            if ((err = re__ast_destroy(&e2))) {
                goto destroy_e1;
            }
        }
    }
    return err;
destroy_e1:
    {
        re_error err1 = re__ast_destroy(&e1);
        re_error err2;
        if (err1) {
            if ((err2 = re__ast_destroy(&e2))) {
                return err2;
            }
            return err1;
        }
        return err;
    }
destroy_e2:
    {
        re_error err1 = re__ast_destroy(&e2);
        re_error err2;
        if (err1) {
            if ((err2 = re__ast_destroy(&e1))) {
                return err2;
            }
            return err1;
        }
        return err;
    }
    /* Destroys e1 and e2, and if errors occur during destruction of those,
     * return the destruction error (an error during destruction is far more
     * significant) otherwise return the error in err */
destroy_e1_e2:
    {
        re_error err1 = re__ast_destroy(&e1);
        re_error err2 = re__ast_destroy(&e2);
        if (err2) {
            return err2;
        }
        if (err1) {
            return err1;
        }
        return err;
    }
destroy_e1_e2_res:
    {
        re_error err1 = re__ast_destroy(&e1);
        re_error err2 = re__ast_destroy(&e2);
        re_error err3 = re__ast_destroy(res);
        if (err3) {
            return err3;
        }
        if (err2) {
            return err2;
        }
        if (err1) {
            return err1;
        }
        return err;
    }
destroy_e2_res:
    {
        re_error err1 = re__ast_destroy(&e2);
        re_error err2 = re__ast_destroy(res);
        if (err2) {
            return err2;
        }
        if (err1) {
            return err1;
        }
        return err;
    }
destroy_res:
    {
        re_error err1 = re__ast_destroy(res);
        if (err1) {
            return err1;
        }
        return err;
    }
}

/* Apply concatenation rules to the stack, reducing it to a single value ahead
 * of base_ptr. */
RE_INTERNAL re_error re__parser_ast_stk_concat(re__parser* parser) {
    re__ast_vec* ast_stk = &parser->ast_stk;
    re_size base_ptr = parser->base_ptr;
    re_error err = RE_ERROR_NONE;
    re__ast e1, e2, res;
    while (re__ast_vec_size(ast_stk) > base_ptr + 1) {
        /* Loop invariant ensures that there are at least two elements left */
        /* We pull e2 first because stack order is backwards */
        e2 = re__ast_vec_pop(ast_stk);
        e1 = re__ast_vec_pop(ast_stk);
        /* We now own e1 and e2. */
        if ((e1.type == RE__AST_TYPE_CHAR ||
            e1.type == RE__AST_TYPE_CHAR_STRING) &&
            (e2.type == RE__AST_TYPE_CHAR ||
            e2.type == RE__AST_TYPE_CHAR_STRING)) {
            /* Fuse characters and strings */
            /* After this call e1 and e2 cannot be used */
            if ((err = re__parse_fuse_char_str(e1, e2, &res))) {
                /* re__parse_fuse_char_str handles destruction of e1 and e2 in
                 * the event of an error */
                return err; 
            }
        } else if (e1.type == RE__AST_TYPE_CONCAT) {
            /* If e1 is already a concatenation just add e2 as a child */
            res = e1;
            if ((err = re__ast_add_child(&res, e2))) {
                goto destroy_e1_e2;
            }
        } else {
            /* Create a new concatenation */
            re__ast_init_concat(&res);
            /* Add the previous two children to the concatenation */
            if ((err = re__ast_add_child(&res, e1))) {
                goto destroy_e1_e2_res;
            }
            if ((err = re__ast_add_child(&res, e2))) {
                goto destroy_e1_e2_res;
            }
        }
        /* Push the concatenation back onto the stack */
        if ((err = re__ast_vec_push(ast_stk, res))) {
            goto destroy_e1_e2_res;
        }
    }
    return err;
destroy_e1_e2:
    {
        re_error err1 = re__ast_destroy(&e1);
        re_error err2 = re__ast_destroy(&e2);
        if (err2) {
            return err2;
        }
        if (err1) {
            return err1;
        }
        return err;
    }
destroy_e1_e2_res:
    {
        re_error err1 = re__ast_destroy(&e1);
        re_error err2 = re__ast_destroy(&e2);
        re_error err3 = re__ast_destroy(&res);
        if (err3) {
            return err3;
        }
        if (err2) {
            return err2;
        }
        if (err1) {
            return err1;
        }
        return err;
    }
}

/* Apply alternation rules to the stack, reducing it to a single value ahead of 
 * base_ptr. */
RE_INTERNAL re_error re__parser_ast_stk_alt(re__parser* parser) {
    re_error err = RE_ERROR_NONE;
    re__ast e1, e2, res;
    while (re__ast_vec_size(&parser->ast_stk) > parser->base_ptr + 1) {
        /* Loop invariant ensures that there are at least two elements left */
        /* We pull e2 first because stack order is backwards */
        e2 = re__ast_vec_pop(&parser->ast_stk);
        e1 = re__ast_vec_pop(&parser->ast_stk);
        if (e2.type == RE__AST_TYPE_ALT) {
            /* If e2 is already an alternation, then we can just prepend e1. */
            res = e2;
            if ((err = re__ast_insert_child(&res, 0, e1))) {
                goto destroy_e1_e2;
            }
        } else {
            /* Create a new alternation */
            re__ast_init_alt(&res);
            /* Add the previous two children to the alternation */
            if ((err = re__ast_add_child(&res, e1))) {
                goto destroy_e1_e2_res;
            }
            if ((err = re__ast_add_child(&res, e2))) {
                goto destroy_e1_e2_res;
            }
        }
        /* Push the alternation back onto the stack */
        if ((err = re__ast_vec_push(&parser->ast_stk, res))) {
            goto destroy_e1_e2_res;
        }
    }
    return err;
destroy_e1_e2:
    {
        re_error err1 = re__ast_destroy(&e1);
        re_error err2 = re__ast_destroy(&e2);
        if (err2) {
            return err2;
        }
        if (err1) {
            return err1;
        }
        return err;
    }
destroy_e1_e2_res:
    {
        re_error err1 = re__ast_destroy(&e1);
        re_error err2 = re__ast_destroy(&e2);
        re_error err3 = re__ast_destroy(&res);
        if (err3) {
            return err3;
        }
        if (err2) {
            return err2;
        }
        if (err1) {
            return err1;
        }
        return err;
    }
}

RE_INTERNAL re_error re__parser_group_finish(re__parser* parser) {
    re_error err = RE_ERROR_NONE;
    /* Pop operators off the stack until we get the matching paren */
    while (1) {
        re__parse_op old_op;
        /* If we've exhausted the operator stack without finding a left parenthesis, there was none in the first place */
        if (re__parse_op_vec_size(&parser->op_stk) == 0) {
            return re__parser_error(parser, "'(' without matching ')'");
        }
        /* Pop the operator in question */
        old_op = re__parser_op_pop(parser);
        if (old_op.op_type == RE__PARSE_OP_TYPE_PARENS) {
            /* We found the left parenthesis. base_ptr is rewound. */
            break;
        } else if (old_op.op_type == RE__PARSE_OP_TYPE_ALT) {
            /* Require alternation to have at least 2 parameters */
            if (re__ast_vec_size(&parser->ast_stk) < parser->base_ptr + 2) {
                return re__parser_error(parser, "cannot use '|' operator with an empty expression");
            }
            /* Execute alternation operator */
            if ((err = re__parse_alt_stk(&parser->ast_stk, parser->base_ptr))) {
                return err;
            }
        } else {
            /* We should never encounter any other op types here. */
            RE__ASSERT_UNREACHED();
        }
    }
    /* Now, base_ptr points to the one AST node that will comprise the parenthesis group. */
    /* base_ptr is guaranteed to point to at least one node because we've ensured that all alternations and concatenations beforehand have resulted in outputting an AST node. */
    {
        re__ast new_group;
        re__ast old_node = re__ast_vec_pop(&parser->ast_stk);
        /* Convert it to a capture group. */
        re__ast_init_group(&new_group);
        if ((err = re__ast_add_child(&new_group, old_node))) {
            goto destroy_new_old;
        }
        /* Push it back onto the stack. */
        if ((err = re__ast_vec_push(&parser->ast_stk, new_group))) {
            re_error err1 = re__ast_destroy(&new_group);
            if (err1) {
                return err1;
            }
            return err;
        }
        return err;
destroy_new_old:
        {
            re_error err1 = re__ast_destroy(&old_node);
            re_error err2 = re__ast_destroy(&new_group);
            if (err2) {
                return err2;
            }
            if (err1) {
                return err1;
            }
            return err;
        }
    }
}

/* Wrap the top of the stack in a quantifier. */
RE_INTERNAL re_error re__parser_build_quantifier(re__parser* parser, re_int32 min, re_int32 max) {
    re_error err = RE_ERROR_NONE;
    /* Ensure there is an argument available */
    if (re__ast_vec_size(&parser->ast_stk) < parser->base_ptr + 1) {
        return re__parser_error(parser, "cannot use quantifier with an empty expression");
    }
    {
        re__ast new_quant;
        re__ast old_node = re__ast_vec_pop(&parser->ast_stk);
        re__ast_init_quantifier(&new_quant, min, max);
        if ((err = re__ast_add_child(&new_quant, old_node))) {
            re_error err1 = re__ast_destroy(&old_node);
            re_error err2 = re__ast_destroy(&new_quant);
            if (err2) {
                return err2;
            }
            if (err1) {
                return err1;
            }
            return err;
        }
        if ((err = re__ast_vec_push(&parser->ast_stk, new_quant))) {
            re_error err1 = re__ast_destroy(&new_quant);
            if (err1) {
                return err1;
            }
            return err;
        }
    }
    return err;
}

/* Create a new anchor */
RE_INTERNAL re_error re__parser_create_anchor(re__parser* parser, re__ast_anchor_type anchor_type) {
    re_error err = RE_ERROR_NONE;
    re__ast new_node;
    re__ast_init_anchor(&new_node, anchor_type);
    if ((err = re__ast_vec_push(&parser->ast_stk, new_node))) {
        re_error err1 = re__ast_destroy(&new_node);
        if (err1) {
            return err1;
        }
        return err;
    }
    return err;
}

/* Create a new Any Character (.) */
RE_INTERNAL re_error re__parser_create_any_char(re__parser* parser) {
    re_error err = RE_ERROR_NONE;
    re__ast new_node;
    re__ast_init_any_char(&new_node);
    if ((err = re__ast_vec_push(&parser->ast_stk, new_node))) {
        re_error err1 = re__ast_destroy(&new_node);
        if (err1) {
            return err1;
        }
        return err;
    }
    return err;
}

/* Create a new Any Byte (\C) */
/* This can be used inside or outside of a character class */
RE_INTERNAL re_error re__parser_create_any_byte(re__parser* parser) {
    re_error err = RE_ERROR_NONE;
    re__ast new_node;
    re__ast_init_any_byte(&new_node);
    if ((err = re__ast_vec_push(&parser->ast_stk, new_node))) {
        re_error err1 = re__ast_destroy(&new_node);
        if (err1) {
            return err1;
        }
        return err;
    }
    return err;
}

/* Create and push a new character */
RE_INTERNAL re_error re__parser_create_char(re__parser* parser, re_char ch) {
    re_error err = RE_ERROR_NONE;
    re__ast new_node;
    re__ast_init_char(&new_node, ch);
    if ((err = re__ast_vec_push(&parser->ast_stk, new_node))) {
        re_error err1 = re__ast_destroy(&new_node);
        if (err1) {
            return err1;
        }
        return err;
    }
    return err;
}

/* Clear number parsing state. */
RE_INTERNAL void re__parser_radix_clear(re__parser* parser) {
    parser->radix_num = 0;
    parser->radix_digits = 0;
}

/* Maximum octal value that can be held in three digits */
#define RE__PARSER_OCT_NUM_MAX ((8 * 8 * 8) - 1)
#define RE__PARSER_OCT_DIGITS_MAX 3

/* Parse in a single octal digit. */
RE_INTERNAL re_error re__parser_radix_consume_oct(re__parser* parser, re_char ch) {
    if (parser->radix_digits == RE__PARSER_OCT_DIGITS_MAX) {
        return re__parser_error(parser, "octal literal exceeds maximum of three digits");
    }
    parser->radix_num *= 8;
    parser->radix_num += ch - '0';
    parser->radix_digits++;
    if (parser->radix_num >= RE__PARSER_OCT_NUM_MAX) {
        return re__parser_error(parser, "octal literal exceeds maximum value of \\777");
    }
    return RE_ERROR_NONE;
}

/* Clear counting form state. */
RE_INTERNAL void re__parser_counting_clear(re__parser* parser) {
    parser->counting_lo = -1;
    parser->counting_hi = -1;
}

/* Make the top operand on the stack non-greedy. */
RE_INTERNAL re_error re__parser_make_non_greedy(re__parser* parser) {
    re_error err = RE_ERROR_NONE;
    re__ast old_node;
    /* Ensure there is an argument available */
    /* We really don't have to do this, but in the spirit of defensive
     * programming it is a good idea */
    if (re__ast_vec_size(&parser->ast_stk) < parser->base_ptr + 1) {
        return re__parser_error(parser, "cannot use '?' operator with an empty expression");
    }
}

/* Disallow an escape character from occurring in a character class. */
/* This function succeeds when the calling state is GND, but does not when the
 * calling state is anything else, in which case it returns an error. */
/* Ensure that this is only called with printable characters. */
RE_INTERNAL re_error re__parser_disallow_escape_in_charclass(re__parser* parser, re_char esc_ch) {
    re_error err = RE_ERROR_NONE;
    re__parse_op top;
    re__str err_str;
    RE_ASSERT(re__parse_op_vec_size(&parser->op_stk) > 0);
    top = re__parse_op_vec_peek(&parser->op_stk);
    if (top.ret_state != RE__PARSE_STATE_GND) {
        /* Build error message */
        re__str err_str;
        if ((err = re__str_init_s(&err_str, "cannot use escape sequence '\\"))) {
            goto destroy_err_str;
        }
        if ((err = re__str_cat_n(&err_str, 1, &esc_ch))) {
            goto destroy_err_str;
        }
        if ((err = re__str_cat_s(&err_str, "' from within character class (\"[]\")"))) {
            goto destroy_err_str;
        }
        re__set_error_str(parser->re, &err_str);
    }
destroy_err_str:
    re__str_destroy(&err_str);
    return err;
}

/* Depending on the current state, push a character class *into* a character
 * class, *OR* push a character class AST node. */
RE_INTERNAL re_error re__parser_ast_or_charclass_push_default(re__parser* parser, re__charclass_ascii_type ascii_cc, int inverted) {
    re__parse_state top_state;
    re__charclass new_class;
    re_error err = RE_ERROR_NONE;
    /* This cannot be called outside of escape. */
    RE_ASSERT(parser->state == RE__PARSE_STATE_ESCAPE);
    RE_ASSERT(re__parse_op_vec_size(&parser->op_stk) > 0);
    top_state = re__parse_op_vec_peek(&parser->op_stk).ret_state;
    /* Create the character class */
    if ((err = re__charclass_init_from_class(&new_class, ascii_cc, inverted))) {
        return err;
    }
    if (top_state == RE__PARSE_STATE_GND) {
        /* Wrap it in an AST node */
        re__ast new_node;
        re__ast_init_class(&new_node, new_class);
        /* new_node now owns new_class */
        if ((err = re__ast_vec_push(&parser->ast_stk, new_node))) {
            /* error adding to stack, destroy new node (and charclass ! ) */
            re_error err1 = re__ast_destroy(&new_node);
            if (err1) {
                return err1;
            }
            return err;
        }
    } else {
        /* Add it to the charclass builder */
        if ((err = re__charclass_builder_insert_class(&parser->charclass_builder, &new_class))) {
            /* destroy charclass */
            re__charclass_destroy(&new_class);
            return err;
        }
        re__charclass_destroy(&new_class);
    }
    return err;
}

/* Depending on the current state, push a new character AST node, or add the
 * given character to the current character class. */
RE_INTERNAL re_error re__parser_ast_or_charclass_push_char(re__parser* parser, re_char ch) {
    re__parse_state top_state;
    re__charclass new_class;
    re_error err = RE_ERROR_NONE;
    /* This cannot be called outside of escape. */
    RE_ASSERT(parser->state == RE__PARSE_STATE_ESCAPE);
    RE_ASSERT(re__parse_op_vec_size(&parser->op_stk) > 0);
    top_state = re__parse_op_vec_peek(&parser->op_stk).ret_state;
    if (top_state == RE__PARSE_STATE_GND) {
        /* Wrap it in an AST node */
        re__ast new_node;
        re__ast_init_char(&new_node, ch);
        if ((err = re__ast_vec_push(&parser->ast_stk, new_node))) {
            re_error err1 = re__ast_destroy(&new_node);
            if (err1) {
                return err1;
            }
            return err;
        }
    } else {
        /* Add it to the charclass builder */
        re__runerange new_range;
        new_range.lo = ch;
        new_range.hi = ch + 1;
        if ((err = re__charclass_builder_insert_range(&parser->charclass_builder, new_range))) {
            return err;
        }
    }
    return err;
}

RE_INTERNAL re_error re__parser_error_invalid_escape(re__parser* parser, re_char esc_ch) {
    /* Build error message */
    re_error err = RE_ERROR_NONE;
    re__str err_str;
    if ((err = re__str_init_s(&err_str, "invalid escape sequence '\\"))) {
        goto destroy_err_str;
    }
    if ((err = re__str_cat_n(&err_str, 1, &esc_ch))) {
        goto destroy_err_str;
    }
    re__set_error_str(parser->re, &err_str);
destroy_err_str:
    re__str_destroy(&err_str);
    return RE_ERROR_PARSE;
}

#define RE__TRY(expr) \
        if ((err = expr)) { \
            goto error; \
        } 

RE_INTERNAL re_error re__parse_new(re* re, re_size input_regex_size, const re_char* input_regex) { {
    re_error err = RE_ERROR_NONE;
    re__parser parser;
    re__parser_init(&parser, re);

    const re_char* reg_ptr = input_regex;
    const re_char* end_ptr = input_regex + input_regex_size;

    /* Are we on the last iteration? */
    int last = 0;

    /* Loop through reg expr */
    while (reg_ptr <= end_ptr) {
        re_char ch = '\0';
        if (reg_ptr == end_ptr) {
            last = 1;
        } else {
            ch = *reg_ptr;
        }
        if (parser.state == RE__PARSE_STATE_GND) {
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
            if (last) {
                /* Last iteration: reduce stack to one value. */
                RE__TRY(re__parser_ast_stk_concat(&parser));
            } else if (ch == '\\') {
                /* Escape: Push an Escape frame. */
                RE__TRY(re__parser_ast_stk_concat(&parser));
                RE__TRY(re__parser_op_push(&parser, RE__PARSE_OP_TYPE_ESCAPE));
                parser.state = RE__PARSE_STATE_ESCAPE;
            } else if (ch == '(') {
                /* Group: reduce stack, push a Parens frame. */
                RE__TRY(re__parser_ast_stk_concat(&parser));
                parser.state = RE__PARSE_STATE_PARENS_INITIAL;
            } else if (ch == ')') {
                /* End of group reduce stack, pop the group frame */
                RE__TRY(re__parser_ast_stk_concat(&parser));
                /* re__parser_group_finish pops the group frame */
                RE__TRY(re__parser_group_finish(&parser));
            } else if (ch == '|') {
                /* Start of alternation: reduce stack, push an alt frame */
                RE__TRY(re__parser_ast_stk_concat(&parser));
                RE__TRY(re__parser_op_push(&parser, RE__PARSE_OP_TYPE_ALT));
            } else if (ch == '*') {
                /* Kleene star: *don't* reduce stack, star the last item */
                RE__TRY(re__parser_build_quantifier(&parser, 0, RE__AST_QUANTIFIER_INFINITY));
                /* Set to MAYBE_QUESTION in order to detect non-greedy's */
                parser.state = RE__PARSE_STATE_MAYBE_QUESTION;
            } else if (ch == '+') {
                /* Kleene plus: *don't* reduce stack, plus the last item */
                RE__TRY(re__parser_build_plus(&parser, 1, RE__AST_QUANTIFIER_INFINITY));
                parser.state = RE__PARSE_STATE_MAYBE_QUESTION;
            } else if (ch == '?') {
                /* Question: *don't* reduce stack, question the last item */
                RE__TRY(re__parser_build_question(&parser, 0, 1));
                parser.state = RE__PARSE_STATE_MAYBE_QUESTION;
            } else if (ch == '^') {
                /* Text start: push a text_start anchor */
                RE__TRY(re__parser_create_anchor(&parser, RE__AST_ANCHOR_TYPE_TEXT_START));
            } else if (ch == '$') {
                /* Text end: push a text_end anchor */
                RE__TRY(re__parser_create_anchor(&parser, RE__AST_ANCHOR_TYPE_TEXT_END));
            } else if (ch == '.') {
                /* Any character: push an any node */
                RE__TRY(re__parser_create_any_char(&parser));
            } else if (ch == '[') {
                /* Start of charclass -- jump over there */
                parser.state = RE__PARSE_STATE_CHARCLASS_INITIAL;
            } else if (ch == '{') {
                /* Start of a counting form, clear state */
                re__parser_radix_clear(&parser);
                re__parser_counting_clear(&parser);
                parser.state = RE__PARSE_STATE_COUNTING_FIRST_NUM_INITIAL;
            } else {
                /* Any other character, create it */
                RE__TRY(re__parser_create_char(&parser, ch));
            }
        } else if (parser.state == RE__PARSE_STATE_MAYBE_QUESTION) {
            /* After *,+,?, or {n,m}: optionally makes a non-greedy quantifier */
            if (last) {
                /* Defer finishing to ground */
                parser.defer = 1;
                parser.state = RE__PARSE_STATE_GND;
            } else if (ch == '?') {
                /* Another question -- "non-greedy-ize" the top operand */
                RE__TRY(re__parser_make_non_greedy(&parser));
                parser.state = RE__PARSE_STATE_GND;
            } else {
                /* Defer parsing of next char to ground */
                re__parser_defer(parser);
                parser.state = RE__PARSE_STATE_GND;
            }
        } else if (parser.state == RE__PARSE_STATE_ESCAPE) {
            /* Escape character: \ */
            if (last) {
                /* \ followed by end of string */
                RE__TRY(re__parser_error(&parser, "unfinished escape sequence"));
            } else if (ch == '0' || ch == '1' || ch == '2' || ch == '3' || ch == '4' || ch == '5' || ch == '6' || ch == '7') {
                /* Octal digits */
                re__parser_radix_clear(&parser);
                RE__TRY(re__parser_radix_consume_oct(&parser, ch));
                parser.state = RE__PARSE_STATE_OCTAL_SECOND_DIGIT;
            } else if (ch == 'A') {
                /* Absolute text start */
                /* This cannot be used from within a charclass */
                RE__TRY(re__parser_disallow_escape_in_charclass(&parser, ch));
                RE__TRY(re__parser_create_anchor(&parser, RE__AST_ANCHOR_TYPE_TEXT_START_ABSOLUTE));
                /* Return to GND or calling state */
                re__parser_op_pop_expect(&parser, RE__PARSE_OP_TYPE_ESCAPE);
            } else if (ch == 'B') {
                /* Not a word boundary */
                RE__TRY(re__parser_disallow_escape_in_charclass(&parser, ch));
                RE__TRY(re__parser_create_anchor(&parser, RE__AST_ANCHOR_TYPE_WORD_NOT));
                re__parser_op_pop_expect(&parser, RE__PARSE_OP_TYPE_ESCAPE);
            } else if (ch == 'C') {
                /* Any *byte* (NOT any char) */
                RE__TRY(re__parser_disallow_escape_in_charclass(&parser, ch));
                RE__TRY(re__parser_create_any_byte(&parser));
                re__parser_op_pop_expect(&parser, RE__PARSE_OP_TYPE_ESCAPE);
            } else if (ch == 'D') {
                /* Not a digit */
                RE__TRY(re__parser_ast_or_charclass_push_default(&parser, RE__CHARCLASS_ASCII_TYPE_DIGIT, 1));
                re__parser_op_pop_expect(&parser, RE__PARSE_OP_TYPE_ESCAPE);
            } else if (ch == 'E') {
                /* Quote end -- invalid here */
                RE__TRY(re__parser_error(&parser, "\\E can only be used from within \\Q"));
            } else if (ch == 'F' || ch == 'G' || ch == 'H' || ch == 'I' || ch == 'J' || ch == 'K' || ch == 'L' || ch == 'M' || ch == 'N' || ch == 'O') {
                RE__TRY(re__parser_error_invalid_escape(&parser, ch));
            } else if (ch == 'P') {
                /* Unicode character class */
                RE__TRY(re__parser_error(&parser, "unimplemented"));
                /*re__parser_unicode_charclass_invert(parser);
                parser.state = RE__PARSE_STATE_UNICODE_CHARCLASS_BEGIN;*/
            } else if (ch == 'Q') {
                /* Quote begin */
                RE__TRY(re__parser_disallow_escape_in_charclass(&parser, ch));
                re__parser_op_pop_expect(&parser, RE__PARSE_OP_TYPE_ESCAPE);
                parser.state = RE__PARSE_STATE_QUOTE;
            } else if (ch == 'R') {
                RE__TRY(re__parser_error_invalid_escape(&parser, ch));
            } else if (ch == 'S') {
                /* Not whitespace (Perl) */
                RE__TRY(re__parser_ast_or_charclass_push_default(&parser, RE__CHARCLASS_ASCII_TYPE_PERL_SPACE, 1));
                re__parser_op_pop_expect(&parser, RE__PARSE_OP_TYPE_ESCAPE);
            } else if (ch == 'T' || ch == 'U' || ch == 'V') {
                RE__TRY(re__parser_disallow_escape_in_charclass(&parser, ch));
            } else if (ch == 'W') {
                /* Not a word character */
                RE__TRY(re__parser_ast_or_charclass_push_default(&parser, RE__CHARCLASS_ASCII_TYPE_WORD, 1));
                re__parser_op_pop_expect(&parser, RE__PARSE_OP_TYPE_ESCAPE);
            } else if (ch == 'X' || ch == 'Y' || ch == 'Z') {
                RE__TRY(re__parser_disallow_escape_in_charclass(&parser, ch));
            } else if (ch == 'a') {
                /* Bell character */
                RE__TRY(re__parser_ast_or_charclass_push_char(&parser, '\a'));
                re__parser_op_pop_expect(&parser, RE__PARSE_OP_TYPE_ESCAPE);
            } else if (ch == 'b') {
                /* Word boundary */
                RE__TRY(re__parser_disallow_escape_in_charclass(&parser, ch));
                RE__TRY(re__parser_create_anchor(&parser, RE__AST_ANCHOR_TYPE_WORD_NOT));
                re__parser_op_pop_expect(&parser, RE__PARSE_OP_TYPE_ESCAPE);
            } else if (ch == 'c') {
                /* Case/control character : not supported */
                RE__TRY(re__parser_disallow_escape_in_charclass(&parser, ch));
            } else if (ch == 'd') {
                /* Digit character */
                RE__TRY(re__parser_ast_or_charclass_push_default(&parser, RE__CHARCLASS_ASCII_TYPE_DIGIT, 0));
                re__parser_op_pop_expect(&parser, RE__PARSE_OP_TYPE_ESCAPE);
            } else if (ch == 'e') {
                RE__TRY(re__parser_disallow_escape_in_charclass(&parser, ch));
            } else if (ch == 'f') {
                /* Form feed */
                RE__TRY(re__parser_ast_or_charclass_push_char(&parser, '\f'));
                re__parser_op_pop_expect(&parser, RE__PARSE_OP_TYPE_ESCAPE);
            } else if (ch == 'g' || ch == 'h' || ch == 'i' || ch == 'j' || ch == 'k' || ch == 'l' || ch == 'm') {
                RE__TRY(re__parser_error_invalid_escape(&parser, ch));
            } else if (ch == 'n') {
                /* Newline */
                RE__TRY(re__parser_ast_or_charclass_push_char(&parser, '\n'));
                re__parser_op_pop_expect(&parser, RE__PARSE_OP_TYPE_ESCAPE);
            } else if (ch == 'o') {
                RE__TRY(re__parser_error_invalid_escape(&parser, ch));
            } else if (ch == 'p') {
                /* Beginning of named Unicode character class */
                RE__TRY(re__parser_error(&parser, "unimplemented"));
                /*parser.state = RE__PARSE_STATE_UNICODE_CHARCLASS_BEGIN;*/
            } else if (ch == 'q') {
                RE__TRY(re__parser_error_invalid_escape(&parser, ch));
            } else if (ch == 'r') {
                /* Carriage return */
                RE__TRY(re__parser_ast_or_charclass_push_char(&parser, '\r'));
                re__parser_op_pop_expect(&parser, RE__PARSE_OP_TYPE_ESCAPE);
            } else if (ch == 's') {
                /* Whitespace (Perl) */
                RE__TRY(re__parser_ast_or_charclass_push_default(&parser, RE__CHARCLASS_ASCII_TYPE_PERL_SPACE, 0));
                re__parser_op_pop_expect(&parser, RE__PARSE_OP_TYPE_ESCAPE);
            } else if (ch == 't') {
                /* Horizontal tab */
                RE__TRY(re__parser_ast_or_charclass_push_char(&parser, '\t'));
                re__parser_op_pop_expect(&parser, RE__PARSE_OP_TYPE_ESCAPE);
            } else if (ch == 'u') {
                RE__TRY(re__parser_error_invalid_escape(&parser, ch));
            } else if (ch == 'v') {
                /* Vertical tab */
                RE__TRY(re__parser_ast_or_charclass_push_char(&parser, '\v'));
                re__parser_op_pop_expect(&parser, RE__PARSE_OP_TYPE_ESCAPE);
            } else if (ch == 'w') {
                /* Word character */
                RE__TRY(re__parser_ast_or_charclass_push_default(&parser, RE__CHARCLASS_ASCII_TYPE_WORD, 0));
                re__parser_op_pop_expect(&parser, RE__PARSE_OP_TYPE_ESCAPE);
            } else if (ch == 'x') {
                /* Two digit / zero to six digit hex literal */
                re__parser_radix_clear(&parser);
                parser.state = RE__PARSE_STATE_HEX_INITIAL;
            } else if (ch == 'y') {
                RE__TRY(re__parser_error_invalid_escape(&parser, ch));
            } else if (ch == 'z') {
                /* Absolute text end */
                RE__TRY(re__parser_disallow_escape_in_charclass(&parser, ch));
                RE__TRY(re__parser_create_anchor(&parser, RE__AST_ANCHOR_TYPE_TEXT_START_ABSOLUTE));
                re__parser_op_pop_expect(&parser, RE__PARSE_OP_TYPE_ESCAPE);
            } else {
                /* All other characters */
                RE__TRY(re__parser_ast_or_charclass_push_char(&parser, ch));
                re__parser_op_pop_expect(&parser, RE__PARSE_OP_TYPE_ESCAPE);
            }
        } else if (parser.state == RE__PARSE_STATE_PARENS_INITIAL) {
            /* After the first '(' */
            if (last) {
                RE__TRY(re__parser_error(&parser, "unfinished '('"));
            } else if (ch == ')') {
                /* Empty group, captures */
                /* Defer to GND state handling of ) */
                re__parser_defer(&parser);
                RE__TRY(re__parser_op_push(&parser, RE__PARSE_OP_TYPE_PARENS));
                parser.state = RE__PARSE_STATE_GND;
            } else if (ch == '?') {
                parser.state = RE__PARSE_STATE_PARENS_FLAG_INITIAL;
            } else {
                re__parser_defer(&parser);
                RE__TRY(re__parser_op_push(&parser, RE__PARSE_OP_TYPE_PARENS));
                parser.state = RE__PARSE_STATE_GND;
            }
        } else if (parser.state == RE__PARSE_STATE_PARENS_FLAG) {
            /* After "(?" */
            if (last) {
                RE__TRY(re__parser_error(&parser, "unset parenthetical group flag(s)"));
            } else if (ch == 'P') {
                /* (?P -- Perl named group */
                parser.state = RE__PARSE_STATE_PARENS_FLAG_NAMED_INITIAL;
            } else if (ch == ':') {
                /* (?: -- Non-matching group */
                parser.group_flags |= RE__AST_GROUP_FLAG_NONMATCHING;
                parser.state = RE__PARSE_STATE_GND;
            } else if (ch == 'i') {
                /* (?i -- Case-insensitive matching */
                parser.group_flags |= RE__AST_GROUP_FLAG_CASE_INSENSITIVE;
            } else if (ch == 'm') {
                /* (?m -- ^/$ matches line start/end */
                parser.group_flags |= RE__AST_GROUP_FLAG_MULTILINE;
            } else if (ch == 's') {
                /* (?s -- . matches newline */
                parser.group_flags |= RE__AST_GROUP_FLAG_DOT_NEWLINE;
            } else if (ch == 'U') {
                /* (?U -- un-greedy matches apply within group */
                parser.group_flags |= RE__AST_GROUP_FLAG_UNGREEDY;
            } else if (ch == ')') {
                
            } else {
                RE__TRY(re__parser_error(&parser, "invalid group flag character"));
            }
        } else if (parser.state == RE__PARSE_STATE_PARENS_FLAG_NAMED_INITIAL) {
            /* After (?P */
            if (last) {
                RE__TRY(re__parser_error(&parser, "expected a '<' to begin capturing group name"));
            } else if (ch == '<') {
                parser.state = RE__PARSE_STATE_PARENS_FLAG_NAMED;
            } else {
                RE__TRY(re__parser_error(&parser, "expected a '<' to begin capturing group name"));
            }
        } else if (parser.state == RE__PARSE_STATE_PARENS_FLAG_NAMED) {
            /* After (?P< */
            if (last) {
                RE__TRY(re__parser_error(&parser, "expected a '>' to close capturing group name"));
            } else if (ch == '>') {
                /* Get out of name, we are done */
                RE__TRY(re__parser_op_push(&parser, RE__PARSE_OP_TYPE_PARENS));
                parser.state = RE__PARSE_STATE_GND;
            } else {
                /* Add character to name */
                RE__TRY(re__str_push(&parser.literal, ch));
            }
        } else if (parser.state == RE__PARSE_STATE_QUOTE) {
            if (last) {
                re__parser_ast_push(parser);
                re__parser_defer(parser);
                parser.state = RE__PARSE_STATE_GND;
            } else if (ch == '\\') {
                parser.state = RE__PARSE_STATE_QUOTE_ESCAPE;
            } else {
                re__parser_quote_string_add(parser);
            }
        } else if (parser.state == RE__PARSE_STATE_QUOTE_ESCAPE) {
            if (last) {
                re__parser_quote_string_add(parser);
                re__parser_ast_push(parser);
                re__parser_defer(parser);
                parser.state = RE__PARSE_STATE_GND;
            } else if (ch == 'E') {
                re__parser_ast_push(parser);
                parser.state = RE__PARSE_STATE_GND;
            } else if (ch == '\\') {
                parser.state = RE__PARSE_STATE_QUOTE;
            } else {
                re__parser_quote_string_add(parser);
                re__parser_quote_string_add(parser);
                parser.state = RE__PARSE_STATE_QUOTE;
            }
        } else if (parser.state == RE__PARSE_STATE_HEX_INITIAL) {
            if (last) {
                re__parser_error(parser, "Unfinished hex code");
            } else if (ch == '0' || ch == '1' || ch == '2' || ch == '3' || ch == '4' || ch == '5' || ch == '6' || ch == '7' || ch == '8' || ch == '9' || ch == 'A' || ch == 'B' || ch == 'C' || ch == 'D' || ch == 'E' || ch == 'F' || ch == 'a' || ch == 'b' || ch == 'c' || ch == 'd' || ch == 'e' || ch == 'f') {
                re__parser_radix_consume_hex(parser);
                re__parser_radix_check_hex(parser);
                parser.state = RE__PARSE_STATE_HEX_SECOND_DIGIT;
            } else if (ch == '{') {
                parser.state = RE__PARSE_STATE_HEX_EXTENDED;
            } else {
                re__parser_error(parser, "Invalid hexadecimal character, expected 0-9A-Fa-F or {");
            }
        } else if (parser.state == RE__PARSE_STATE_HEX_SECOND_DIGIT) {
            if (last) {
                re__parser_error(parser, "Unfinished two-digit hex code");
            } else if (ch == '0' || ch == '1' || ch == '2' || ch == '3' || ch == '4' || ch == '5' || ch == '6' || ch == '7' || ch == '8' || ch == '9' || ch == 'A' || ch == 'B' || ch == 'C' || ch == 'D' || ch == 'E' || ch == 'F' || ch == 'a' || ch == 'b' || ch == 'c' || ch == 'd' || ch == 'e' || ch == 'f') {
                re__parser_radix_consume_hex(parser);
                re__parser_radix_check_hex(parser);
                re__parser_ast_or_charclass_push(parser);
                re__parser_op_pop(parser);
            } else {
                re__parser_error(parser, "Invalid hexadecimal character");
            }
        } else if (parser.state == RE__PARSE_STATE_HEX_EXTENDED) {
            if (last) {
                re__parser_error(parser, "Unfinished extended hex code");
            } else if (ch == '0' || ch == '1' || ch == '2' || ch == '3' || ch == '4' || ch == '5' || ch == '6' || ch == '7' || ch == '8' || ch == '9' || ch == 'A' || ch == 'B' || ch == 'C' || ch == 'D' || ch == 'E' || ch == 'F' || ch == 'a' || ch == 'b' || ch == 'c' || ch == 'd' || ch == 'e' || ch == 'f') {
                re__parser_radix_consume_hex(parser);
                re__parser_radix_check_hex(parser);
            } else if (ch == '}') {
                re__parser_radix_check_hex(parser);
                re__parser_ast_or_charclass_push(parser);
                re__parser_op_pop(parser);
            } else {
                re__parser_error(parser, "Invalid hexadecimal character");
            }
        } else if (parser.state == RE__PARSE_STATE_OCTAL_SECOND_DIGIT) {
            if (last) {
                re__parser_radix_check_oct(parser);
                re__parser_ast_or_charclass_push(parser);
                re__parser_op_pop(parser);
                re__parser_defer(parser);
            } else if (ch == '0' || ch == '1' || ch == '2' || ch == '3' || ch == '4' || ch == '5' || ch == '6' || ch == '7') {
                re__parser_radix_consume_oct(parser);
                re__parser_radix_check_oct(parser);
                parser.state = RE__PARSE_STATE_OCTAL_THIRD_DIGIT;
            } else {
                re__parser_radix_check_oct(parser);
                re__parser_ast_or_charclass_push(parser);
                re__parser_op_pop(parser);
                re__parser_defer(parser);
            }
        } else if (parser.state == RE__PARSE_STATE_OCTAL_THIRD_DIGIT) {
            if (last) {
                re__parser_radix_check_oct(parser);
                re__parser_ast_or_charclass_push(parser);
                re__parser_op_pop(parser);
                re__parser_defer(parser);
            } else if (ch == '0' || ch == '1' || ch == '2' || ch == '3' || ch == '4' || ch == '5' || ch == '6' || ch == '7') {
                re__parser_radix_consume_oct(parser);
                re__parser_radix_check_oct(parser);
                parser.state = RE__PARSE_STATE_GND;
            } else {
                re__parser_radix_check_oct(parser);
                re__parser_ast_or_charclass_push(parser);
                re__parser_op_pop(parser);
                re__parser_defer(parser);
            }
        } else if (parser.state == RE__PARSE_STATE_CHARCLASS_INITIAL) {
            if (last) {
                re__parser_error(parser, "Unfinished char class");
            } else if (ch == '[') {
                parser.state = RE__PARSE_STATE_CHARCLASS_AFTER_CARET;
                re__parser_op_push(parser);
                parser.state = RE__PARSE_STATE_CHARCLASS_NAMED_INITIAL;
            } else if (ch == ']') {
                re__parser_error(parser, "Empty character class");
            } else if (ch == ':') {
                parser.state = RE__PARSE_STATE_GND;
                re__parser_op_push(parser);
                parser.state = RE__PARSE_STATE_CHARCLASS_NAMED_NAME_INITIAL;
            } else if (ch == '^') {
                re__parser_charclass_invert(parser);
                parser.state = RE__PARSE_STATE_CHARCLASS_AFTER_CARET;
            } else if (ch == '\\') {
                re__parser_op_push(parser);
                parser.state = RE__PARSE_STATE_ESCAPE;
            } else {
                re__parser_charclass_range_add(parser);
                parser.state = RE__PARSE_STATE_CHARCLASS_AFTER_RANGE_BEGIN;
            }
        } else if (parser.state == RE__PARSE_STATE_CHARCLASS_AFTER_CARET) {
            if (last) {
                re__parser_error(parser, "Unfinished inverted char class");
            } else if (ch == '[') {
                parser.state = RE__PARSE_STATE_CHARCLASS_AFTER_CARET;
                re__parser_op_push(parser);
                parser.state = RE__PARSE_STATE_CHARCLASS_NAMED_INITIAL;
            } else if (ch == ']') {
                re__parser_error(parser, "Unfinished inverted char class");
            } else if (ch == '\\') {
                re__parser_op_push(parser);
                parser.state = RE__PARSE_STATE_ESCAPE;
            } else {
                re__parser_charclass_range_add(parser);
                parser.state = RE__PARSE_STATE_CHARCLASS_AFTER_RANGE_BEGIN;
            }
        } else if (parser.state == RE__PARSE_STATE_CHARCLASS_AFTER_RANGE_BEGIN) {
            if (last) {
                re__parser_error(parser, "Unterminated char class");
            } else if (ch == '[') {
                parser.state = RE__PARSE_STATE_CHARCLASS_AFTER_CARET;
                re__parser_op_push(parser);
                parser.state = RE__PARSE_STATE_CHARCLASS_NAMED_INITIAL;
            } else if (ch == ']') {
                re__parser_charclass_finish(parser);
                parser.state = RE__PARSE_STATE_GND;
            } else if (ch == '\\') {
                parser.state = RE__PARSE_STATE_CHARCLASS_AFTER_HYPHEN;
                re__parser_op_push(parser);
                parser.state = RE__PARSE_STATE_ESCAPE;
            } else if (ch == '-') {
                parser.state = RE__PARSE_STATE_CHARCLASS_AFTER_HYPHEN;
            } else {
                re__parser_charclass_push(parser);
                re__parser_charclass_range_add(parser);
                parser.state = RE__PARSE_STATE_CHARCLASS_AFTER_RANGE_BEGIN;
            }
        } else if (parser.state == RE__PARSE_STATE_CHARCLASS_AFTER_HYPHEN) {
            if (last) {
                re__parser_error(parser, "Unfinished char class");
            } else if (ch == '[') {
                parser.state = RE__PARSE_STATE_CHARCLASS_AFTER_CARET;
                re__parser_op_push(parser);
                parser.state = RE__PARSE_STATE_CHARCLASS_NAMED_INITIAL;
            } else if (ch == ']') {
                re__parser_charclass_range_add(parser);
                re__parser_charclass_finish(parser);
                parser.state = RE__PARSE_STATE_GND;
            } else if (ch == '\\') {
                parser.state = RE__PARSE_STATE_CHARCLASS_AFTER_CARET;
                re__parser_op_push(parser);
                parser.state = RE__PARSE_STATE_ESCAPE;
            } else {
                re__parser_charclass_range_add(parser);
                re__parser_charclass_finish(parser);
                parser.state = RE__PARSE_STATE_CHARCLASS_AFTER_CARET;
            }
        } else if (parser.state == RE__PARSE_STATE_CHARCLASS_NAMED_INITIAL) {
            if (last) {
                re__parser_error(parser, "Expected a : for named char class");
            } else if (ch == ':') {
                parser.state = RE__PARSE_STATE_CHARCLASS_NAMED_NAME_INITIAL;
            } else {
                re__parser_error(parser, "Expected a : for named char class");
            }
        } else if (parser.state == RE__PARSE_STATE_CHARCLASS_NAMED_NAME_INITIAL) {
            if (last) {
                re__parser_error(parser, "Expected either a ^ or char class name for named char class");
            } else if (ch == '^') {
                re__parser_charclass_named_invert(parser);
                parser.state = RE__PARSE_STATE_CHARCLASS_NAMED_NAME;
            } else {
                re__parser_charclass_named_push(parser);
                parser.state = RE__PARSE_STATE_CHARCLASS_NAMED_NAME;
            }
        } else if (parser.state == RE__PARSE_STATE_CHARCLASS_NAMED_NAME) {
            if (last) {
                re__parser_error(parser, "Unfinished named char class");
            } else if (ch == ':') {
                re__parser_charclass_named_finish(parser);
                parser.state = RE__PARSE_STATE_CHARCLASS_NAMED_FINAL;
            } else {
                re__parser_charclass_named_push(parser);
            }
        } else if (parser.state == RE__PARSE_STATE_CHARCLASS_NAMED_FINAL) {
            if (last) {
                re__parser_error(parser, "Expected ] to finish char class name");
            } else if (ch == ']') {
                re__parser_charclass_class_add(parser);
                re__parser_op_pop(parser);
            } else {
                re__parser_error(parser, "Expected ] to finish char class name");
            }
        } else if (parser.state == RE__PARSE_STATE_UNICODE_CHARCLASS_BEGIN) {
            if (last) {
                re__parser_error(parser, "Expected a character or { for unicode char class");
            } else if (ch == '{') {
                parser.state = RE__PARSE_STATE_UNICODE_CHARCLASS_NAMED_NAME;
            } else {
                re__parser_unicode_charclass_name_add(parser);
                re__parser_unicode_charclass_finish(parser);
                re__parser_op_pop(parser);
            }
        } else if (parser.state == RE__PARSE_STATE_UNICODE_CHARCLASS_NAMED_NAME) {
            if (last) {
                re__parser_error(parser, "Expected a } to end named Unicode char class");
            } else if (ch == '}') {
                re__parser_unicode_charclass_finish(parser);
                re__parser_op_pop(parser);
            } else {
                re__parser_unicode_charclass_name_add(parser);
            }
        } else if (parser.state == RE__PARSE_STATE_COUNTING_FIRST_NUM_INITIAL) {
            if (last) {
                re__parser_error(parser, "Unfinished counting form");
            } else if (ch == '0' || ch == '1' || ch == '2' || ch == '3' || ch == '4' || ch == '5' || ch == '6' || ch == '7' || ch == '8' || ch == '9') {
                re__parser_radix_consume_dec(parser);
                parser.state = RE__PARSE_STATE_COUNTING_FIRST_NUM;
            } else if (ch == '}') {
                re__parser_error(parser, "Expected a number");
            } else {
                re__parser_error(parser, "Invalid decimal character");
            }
        } else if (parser.state == RE__PARSE_STATE_COUNTING_FIRST_NUM) {
            if (last) {
                re__parser_error(parser, "Unfinished counting form");
            } else if (ch == '0' || ch == '1' || ch == '2' || ch == '3' || ch == '4' || ch == '5' || ch == '6' || ch == '7' || ch == '8' || ch == '9') {
                re__parser_radix_consume_dec(parser);
                re__parser_radix_check_dec(parser);
            } else if (ch == '}') {
                re__parser_radix_check_dec(parser);
                re__parser_counting_set_upper(parser);
                re__parser_counting_finish(parser);
                parser.state = RE__PARSE_STATE_MAYBE_QUESTION;
            } else if (ch == ',') {
                re__parser_radix_check_dec(parser);
                re__parser_counting_set_lower(parser);
                parser.state = RE__PARSE_STATE_COUNTING_SECOND_NUM;
            } else {
                re__parser_error(parser, "Invalid decimal character or ,");
            }
        } else if (parser.state == RE__PARSE_STATE_COUNTING_SECOND_NUM_INITIAL) {
            if (last) {
                re__parser_error(parser, "Unfinished counting form");
            } else if (ch == '0' || ch == '1' || ch == '2' || ch == '3' || ch == '4' || ch == '5' || ch == '6' || ch == '7' || ch == '8' || ch == '9') {
                re__parser_radix_consume_dec(parser);
                re__parser_radix_check_dec(parser);
                parser.state = RE__PARSE_STATE_COUNTING_SECOND_NUM;
            } else if (ch == '}') {
                re__parser_error(parser, "Expected number for upper counting bound");
            } else {
                re__parser_error(parser, "Invalid decimal character");
            }
        } else if (parser.state == RE__PARSE_STATE_COUNTING_SECOND_NUM) {
            if (last) {
                re__parser_error(parser, "Unfinished counting form");
            } else if (ch == '0' || ch == '1' || ch == '2' || ch == '3' || ch == '4' || ch == '5' || ch == '6' || ch == '7' || ch == '8' || ch == '9') {
                re__parser_radix_consume_dec(parser);
                re__parser_radix_check_dec(parser);
            } else if (ch == '}') {
                re__parser_radix_check_dec(parser);
                re__parser_counting_set_upper(parser);
                re__parser_counting_finish(parser);
                parser.state = RE__PARSE_STATE_MAYBE_QUESTION;
            } else {
                re__parser_error(parser, "Invalid decimal character");
            }
        } else {
            RE__ASSERT_UNREACHABLE();
        }
    }
    RE_ASSERT(re__ast_vec_size(&parser.ast_stk) == 1);
    re->data->ast_root = re__ast_vec_pop(&parser.ast_stk);
    if ((err = re__parser_destroy(&parser))) {
        return err;
    }
    return RE_ERROR_NONE;
error:
    if (err == RE_ERROR_PARSE) {
        RE_ASSERT(re__str_size(&re->data->error_string));
    } else {
        re__set_error_generic(re, err);
    }
    {
        re_error err1 = re__parser_destroy(&parser);
        if (err1) {
            return err1;
        }
    }
    return err;
}

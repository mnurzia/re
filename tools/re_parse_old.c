#include "re_internal.h"

enum re__parse_state {
    RE__PARSE_STATE_GND,
    RE__PARSE_STATE_ESCAPE
};

enum re__parse_op_type {
    RE__PARSE_OP_TYPE_PARENS,
    RE__PARSE_OP_TYPE_ALT
};

/* A regex operator. Currently the only operators in use are parentheses (for
 * grouping) and vertical bars (for alternating.)
 * We keep a list of these operators as if they are functions on a virtual
 * stack, with the base_ptr field pointing to the first sub-expression or
 * argument to the operator. */
typedef struct re__parse_op {
    /* Type of operation */
    enum re__parse_op_type op_type;
    /* Base of stack (the first argument to this operation) */
    re_size base_ptr;
} re__parse_op;

RE_VEC_DECL(re__parse_op);
RE_VEC_IMPL_FUNC(re__parse_op, init)
RE_VEC_IMPL_FUNC(re__parse_op, destroy)
RE_VEC_IMPL_FUNC(re__parse_op, push)
RE_VEC_IMPL_FUNC(re__parse_op, pop)
RE_VEC_IMPL_FUNC(re__parse_op, size)

RE_VEC_IMPL_FUNC(re__ast, cat)

/* Combine two chars, two char strs, or a mixture of the two into a single char
 * str node. */
RE_INTERNAL re_error re__parse_fuse_char_str(re__ast e1, re__ast e2, re__ast* res) {
    re_error err = RE_ERROR_NONE;
    if (e1.type == RE__AST_TYPE_CHAR) {
        if (e2.type == RE__AST_TYPE_CHAR) {
            /* e1 == char, e2 == char */
            /* new string with two characters */
            re__str str;
            re__ast_init_char_string(res);
            str = re__ast_get_str(res);
            if ((err = re__str_push(&str, re__ast_get_char(&e1)))) {
                return err;
            }
            if ((err = re__str_push(&str, re__ast_get_char(&e2)))) {
                return err;
            }
            re__ast_set_str(res, str);
            /* e1, e2 are both consumed */
            if ((err = re__ast_destroy(&e1))) {
                return err;
            }
            if ((err = re__ast_destroy(&e2))) {
                return err;
            }
        } else if (e2.type == RE__AST_TYPE_CHAR_STRING) {
            /* e1 == char, e2 == char str */
            /* insert e1 before e2 */
            re__str str = re__ast_get_str(&e2);
            if ((err = re__str_insert(&str, 0, re__ast_get_char(&e1)))) {
                return err;
            }
            re__ast_set_str(&e2, str);
            /* set result to newly inserted e2 */
            *res = e2;
            /* e1 is consumed */
            if ((err = re__ast_destroy(&e1))) {
                return err;
            }
        }
    } else if (e1.type == RE__AST_TYPE_CHAR_STRING) {
        if (e2.type == RE__AST_TYPE_CHAR) {
            /* e1 == char_str, e2 == char */
            /* append e2 to e1 */
            re__str str = re__ast_get_str(&e1);
            if ((err = re__str_push(&str, re__ast_get_char(&e2)))) {
                return err;
            }
            re__ast_set_str(&e1, str);
            /* set result to newly pushed e1 */
            *res = e1;
            /* e2 is consumed */
            if ((err = re__ast_destroy(&e2))) {
                return err;
            }
        } else if (e2.type == RE__AST_TYPE_CHAR_STRING) {
            /* e1 == char_str, e2 == char_str */
            /* concatenate e1, e2 */
            re__str a = re__ast_get_str(&e1);
            re__str b = re__ast_get_str(&e2);
            if ((err = re__str_cat(&a, &b))) {
                return err;
            }
            re__ast_set_str(&e1, a);
            /* set result to concatenated e1 */
            *res = e1;
            /* e2 is consumed */
            if ((err = re__ast_destroy(&e2))) {
                return err;
            }
        }
    }
    return err;
}

/* Apply concatenation rules to the stack, reducing it to a single value ahead
 * of base_ptr. */
RE_INTERNAL re_error re__parse_concat_stk(re__ast_vec* ast_stk, re_size base_ptr) {
    re_error err = RE_ERROR_NONE;
    while (re__ast_vec_size(ast_stk) > base_ptr + 1) {
        /* Loop invariant ensures that there are at least two elements left */
        /* We pull e2 first because stack order is backwards */
        re__ast e2 = re__ast_vec_pop(ast_stk);
        re__ast e1 = re__ast_vec_pop(ast_stk);
        re__ast new_concat;
        if ((e1.type == RE__AST_TYPE_CHAR ||
            e1.type == RE__AST_TYPE_CHAR_STRING) &&
            (e2.type == RE__AST_TYPE_CHAR ||
            e2.type == RE__AST_TYPE_CHAR_STRING)) {
            /* Fuse characters and strings */
            /* After this call e1 and e2 cannot be used */
            if ((err = re__parse_fuse_char_str(e1, e2, &new_concat))) {
                return err; 
            }
        } else if (e1.type == RE__AST_TYPE_CONCAT) {
            /* If e1 is already a concatenation just add e2 as a child */
            new_concat = e1;
            if ((err = re__ast_add_child(&new_concat, e2))) {
                return err;
            }
        } else {
            /* Create a new concatenation */
            re__ast_init_concat(&new_concat);
            /* Add the previous two children to the concatenation */
            if ((err = re__ast_add_child(&new_concat, e1))) {
                return err;
            }
            if ((err = re__ast_add_child(&new_concat, e2))) {
                return err;
            }
        }
        /* Push the concatenation back onto the stack */
        if ((err = re__ast_vec_push(ast_stk, new_concat))) {
            return err;
        }
    }
    return err;
}

/* Apply alternation rules to the stack, reducing it to a single value ahead of 
 * base_ptr. */
RE_INTERNAL re_error re__parse_alt_stk(re__ast_vec* ast_stk, re_size base_ptr) {
    re_error err = RE_ERROR_NONE;
    while (re__ast_vec_size(ast_stk) > base_ptr + 1) {
        /* Loop invariant ensures that there are at least two elements left */
        /* We pull e2 first because stack order is backwards */
        re__ast e2 = re__ast_vec_pop(ast_stk);
        re__ast e1 = re__ast_vec_pop(ast_stk);
        re__ast new_alt;
        if (e2.type == RE__AST_TYPE_ALT) {
            /* If e2 is already an alternation, then we can just prepend e1. */
            new_alt = e2;
            if ((err = re__ast_insert_child(&new_alt, 0, e1))) {
                return err;
            }
        } else {
            /* Create a new alternation */
            re__ast_init_alt(&new_alt);
            /* Add the previous two children to the alternation */
            if ((err = re__ast_add_child(&new_alt, e1))) {
                return err;
            }
            if ((err = re__ast_add_child(&new_alt, e2))) {
                return err;
            }
        }
        /* Push the alternation back onto the stack */
        if ((err = re__ast_vec_push(ast_stk, new_alt))) {
            return err;
        }
    }
    return err;
}

/* Implements a simple operator-precedence regex parser. */
/* Some notes:
 *   - Maintains a stack of AST nodes and a stack of operators with their
 *     locations on the AST stack.
 *   - Whenever an alteration or concatenation is found, an operator is pushed
 *     onto the operator stack.
 *   - Between all non-quantifier and non-operator tokens, there is an implicit
 *     concatenation operator. */
RE_INTERNAL re_error re__parse(re* re, re_size input_regex_size, const re_char* input_regex) {
    re_error err = RE_ERROR_NONE;

    /* Path to the current ast node being built */
    re__ast_vec ast_stk;
    /* Stack of pending operations to perform on previous nodes */
    re__parse_op_vec op_stk;
    /* Pointer to current stack base */
    re_size base_ptr = 0;
    /* Current parser state */
    enum re__parse_state parse_state = RE__PARSE_STATE_GND;
    /* Defers parsing of current character to the next loop iteration */
    int defer = 0;
    const re_char* reg_ptr = input_regex;
    const re_char* end_ptr = input_regex + input_regex_size;

    /* Initialize stacks */
    re__ast_vec_init(&ast_stk);
    re__parse_op_vec_init(&op_stk);

    /* Loop through reg expr */
    while (reg_ptr < end_ptr) {
        re_char ch = *reg_ptr;
        if (ch == '(') {
            re__parse_op new_op;
            /* Compress the stack (concatenate everything) */
            if ((err = re__parse_concat_stk(&ast_stk, base_ptr))) {
                return err;
            }
            /* Push a PARENS op onto the operator stack */
            new_op.op_type = RE__PARSE_OP_TYPE_PARENS;
            new_op.base_ptr = base_ptr;
            if ((err = re__parse_op_vec_push(&op_stk, new_op))) {
                return err;
            }
            /* Set the base pointer to the new stack length */
            base_ptr = re__ast_vec_size(&ast_stk);
        } else if (ch == ')') {
            /* Compress the stack */
            if ((err = re__parse_concat_stk(&ast_stk, base_ptr))) {
                return err;
            }
            /* Pop operators off the stack until we get the matching paren */
            while (1) {
                re__parse_op old_op;
                /* If we've exhausted the operator stack without finding a 
                 * left parenthesis, there was none in the first place */
                if (re__parse_op_vec_size(&op_stk) == 0) {
                    re->parse_error = RE__PARSE_ERR_UNMATCHED_RIGHT_PARENTHESES;
                    goto parse_error;
                }
                /* Pop the operator in question */
                old_op = re__parse_op_vec_pop(&op_stk);
                if (old_op.op_type == RE__PARSE_OP_TYPE_PARENS) {
                    /* We found the left parenthesis. Rewind base_ptr. */
                    base_ptr = old_op.base_ptr;
                    break;
                } else if (old_op.op_type == RE__PARSE_OP_TYPE_ALT) {
                    /* Rewind base_ptr. */
                    base_ptr = old_op.base_ptr;
                    /* Require alternation to have at least 2 parameters */
                    if (re__ast_vec_size(&ast_stk) < base_ptr + 2) {
                        re->parse_error = RE__PARSE_ERR_ALTERNATE_WITH_EMPTY_VALUE;
                        goto parse_error;
                    }
                    /* Execute alternation operator */
                    if ((err = re__parse_alt_stk(&ast_stk, base_ptr))) {
                        return err;
                    }
                }
            }
            /* Now, base_ptr points to the one AST node that will comprise the
             * parenthesis group. */
            /* base_ptr is guaranteed to point to at least one node because
             * we've ensured that all alternations and concatenations beforehand
             * have resulted in outputting an AST node. */
            {
                re__ast new_group;
                re__ast old_node = re__ast_vec_pop(&ast_stk);
                /* Convert it to a capture group. */
                re__ast_init_group(&new_group);
                if ((err = re__ast_add_child(&new_group, old_node))) {
                    return err;
                }
                /* Push it back onto the stack. */
                if ((err = re__ast_vec_push(&ast_stk, new_group))) {
                    return err;
                }
            }
        } else if (ch == '|') {
            /* Compress the stack */
            if ((err = re__parse_concat_stk(&ast_stk, base_ptr))) {
                return err;
            }
            /* There must be at least one argument on the stack to alt with */
            if (re__ast_vec_size(&ast_stk) < base_ptr + 1) {
                re->parse_error = RE__PARSE_ERR_ALTERNATE_WITH_EMPTY_VALUE;
                goto parse_error;
            }
            /* Push an ALT operator to the stack */
            {
                re__parse_op new_op;
                new_op.op_type = RE__PARSE_OP_TYPE_ALT;
                new_op.base_ptr = base_ptr;
                if ((err = re__parse_op_vec_push(&op_stk, new_op))) {
                    return err;
                }
            }
            /* Set base_ptr to the beginning of the alternation's arguments */
            base_ptr = re__ast_vec_size(&ast_stk);
        } else if (ch == '*') {
            /* Ensure there is an argument available */
            if (re__ast_vec_size(&ast_stk) < base_ptr + 1) {
                re->parse_error = RE__PARSE_ERR_STAR_NOTHING;
                goto parse_error;
            }
            /* Wrap the current stack top in a Kleene star. */
            {
                re__ast new_star;
                re__ast old_node = re__ast_vec_pop(&ast_stk);
                re__ast_init_star(&new_star);
                if ((err = re__ast_add_child(&new_star, old_node))) {
                    return err;
                }
                if ((err = re__ast_vec_push(&ast_stk, new_star))) {
                    return err;
                }
            }
        } else if (ch == '+') {
            /* Ensure there is an argument available */
            if (re__ast_vec_size(&ast_stk) < base_ptr + 1) {
                re->parse_error = RE__PARSE_ERR_PLUS_NOTHING;
                goto parse_error;
            }
            /* Wrap the stack top in a Kleene plus. */
            {
                re__ast new_plus;
                re__ast old_node = re__ast_vec_pop(&ast_stk);
                re__ast_init_plus(&new_plus);
                if ((err = re__ast_add_child(&new_plus, old_node))) {
                    return err;
                }
                if ((err = re__ast_vec_push(&ast_stk, new_plus))) {
                    return err;
                }
            }
        } else if (ch == '?') {
            /* Ensure there is an argument available */
            if (re__ast_vec_size(&ast_stk) < base_ptr + 1) {
                re->parse_error = RE__PARSE_ERR_QUESTION_NOTHING;
                goto parse_error;
            }
            {
                re__ast new_question;
                re__ast old_node = re__ast_vec_pop(&ast_stk);
                /* To enable non-greedy quantifiers, we check if the immediate
                 * node we are wrapping is another quantifier. */
                /* This enables the "+?" or "*?" or "??" syntax. */
                if (old_node.type == RE__AST_TYPE_STAR ||
                    old_node.type == RE__AST_TYPE_PLUS ||
                    old_node.type == RE__AST_TYPE_QUESTION) {
                    re__ast_set_quantifier_greedy(&old_node, 1);
                    new_question = old_node;
                } else {
                    /* Wrap the stack top in a question. */
                    re__ast_init_question(&new_question);
                    if ((err = re__ast_add_child(&new_question, old_node))) {
                        return err;
                    }
                }
                if ((err = re__ast_vec_push(&ast_stk, new_question))) {
                    return err;
                }
            }
        } else if (ch == '^') {
            re__ast new_text_start;
            if ((err = re__parse_concat_stk(&ast_stk, base_ptr))) {
                return err;
            }
            re__ast_init_text_start(&new_text_start);
            if ((err = re__ast_vec_push(&ast_stk, new_text_start))) {
                return err;
            }
        } else if (ch == '$') {
            re__ast new_text_end;
            if ((err = re__parse_concat_stk(&ast_stk, base_ptr))) {
                return err;
            }
            re__ast_init_text_end(&new_text_end);
            if ((err = re__ast_vec_push(&ast_stk, new_text_end))) {
                return err;
            }
        } else {
            re__ast new_char;
            if ((err = re__parse_concat_stk(&ast_stk, base_ptr))) {
                return err;
            }
            re__ast_init_char(&new_char, ch);
            if ((err = re__ast_vec_push(&ast_stk, new_char))) {
                return err;
            }
        }
        reg_ptr++;
    }
    /* Concatenate the remaining things on the stack */
    if ((err = re__parse_concat_stk(&ast_stk, base_ptr))) {
        return err;
    }
    /* Execute the remaining operators on the stack */
    while (1) {
        re__parse_op old_op;
        /* If there are no elements left, break */
        if (re__parse_op_vec_size(&op_stk) == 0) {
            break;
        }
        /* Pop the operator */
        old_op = re__parse_op_vec_pop(&op_stk);
        if (old_op.op_type == RE__PARSE_OP_TYPE_PARENS) {
            /* There shouldn't be any parentheses on the stack once the entire
             * regex has been parsed. */
            re->parse_error = RE__PARSE_ERR_UNMATCHED_LEFT_PARENTHESES;
            goto parse_error;
        } else if (old_op.op_type == RE__PARSE_OP_TYPE_ALT) {
            /* Rewind base_ptr. */
            base_ptr = old_op.base_ptr;
            /* Require alternation to have at least 2 parameters */
            if (re__ast_vec_size(&ast_stk) < base_ptr + 2) {
                re->parse_error = RE__PARSE_ERR_ALTERNATE_WITH_EMPTY_VALUE;
                goto parse_error;
            }
            /* Execute alternation operator */
            if ((err = re__parse_alt_stk(&ast_stk, base_ptr))) {
                return err;
            }
        }
    }
    /* The stack now contains the final AST node (the root) */
    re->data->ast_root = re__ast_vec_pop(&ast_stk);
    re__ast_vec_destroy(&ast_stk);
    re__parse_op_vec_destroy(&op_stk);
    return RE_ERROR_NONE;
parse_error:
    re->parse_error_location = (re_size)(reg_ptr - input_regex);
    /* Clean up */
    while (re__ast_vec_size(&ast_stk)) {
        re__ast popped_node = re__ast_vec_pop(&ast_stk);
        if ((err = re__ast_destroy(&popped_node))) {
            return err;
        }
    }
    re__ast_vec_destroy(&ast_stk);
    re__parse_op_vec_destroy(&op_stk);
    return RE_ERROR_PARSE;
}

typedef struct re__parser {
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
} re__parser;

RE_INTERNAL void re__parser_init(re__parser* parser) {
    /* Initialize stacks */
    re__ast_vec_init(&parser->ast_stk);
    re__parse_op_vec_init(&parser->op_stk);

    parser->base_ptr = 0;
    parser->state = RE__PARSE_STATE_GND;
    parser->defer = 0;
}

RE_INTERNAL re_error re__parser_destroy(re__parser* parser) {
    re_error err = RE_ERROR_NONE;
    /* Clean up */
    while (re__ast_vec_size(&parser->ast_stk)) {
        re__ast popped_node = re__ast_vec_pop(&parser->ast_stk);
        if ((err = re__ast_destroy(&popped_node))) {
            return err;
        }
    }
    re__ast_vec_destroy(&parser->ast_stk);
    re__parse_op_vec_destroy(&parser->op_stk);
}

RE_INTERNAL re_error re__parse_new(re* re, re_size input_regex_size, const re_char* input_regex) { {
    re_error err = RE_ERROR_NONE;
    re__parser parser;
    re__parser_init(&parser);

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
        
    }
}

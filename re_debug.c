#include "re_internal.h"

RE_INTERNAL re_error re__debug_sexpr_tree_init(re__debug_sexpr_tree* sexpr, const re__str* atom, int is_atom) {
    sexpr->first_child_ref = RE__DEBUG_SEXPR_NONE;
    sexpr->next_sibling_ref = RE__DEBUG_SEXPR_NONE;
    sexpr->is_atom = is_atom;
    return re__str_init_copy(&sexpr->atom, atom);
}

RE_INTERNAL void re__debug_sexpr_tree_destroy(re__debug_sexpr_tree* sexpr) {
    re__str_destroy(&sexpr->atom);
}

RE_VEC_IMPL_FUNC(re__debug_sexpr_tree, init)
RE_VEC_IMPL_FUNC(re__debug_sexpr_tree, destroy)
RE_VEC_IMPL_FUNC(re__debug_sexpr_tree, push)
RE_VEC_IMPL_FUNC(re__debug_sexpr_tree, pop)
RE_VEC_IMPL_FUNC(re__debug_sexpr_tree, size)
RE_VEC_IMPL_FUNC(re__debug_sexpr_tree, getref)

RE_INTERNAL void re__debug_sexpr_init(re__debug_sexpr* sexpr) {
    re__debug_sexpr_tree_vec_init(&sexpr->tree);
}

RE_INTERNAL re__debug_sexpr_tree* re__debug_sexpr_get(re__debug_sexpr* sexpr, re_int32 ref) {
    RE_ASSERT(ref != RE__DEBUG_SEXPR_NONE);
    return re__debug_sexpr_tree_vec_getref(&sexpr->tree, (re_size)ref);
}

RE_INTERNAL void re__debug_sexpr_destroy(re__debug_sexpr* sexpr) {
    re_int32 i;
    for (i = 0; i < (re_int32)re__debug_sexpr_tree_vec_size(&sexpr->tree); i++) {
        re__str_destroy(&re__debug_sexpr_get(sexpr, i)->atom);
    }
    re__debug_sexpr_tree_vec_destroy(&sexpr->tree);
}

RE_INTERNAL re_error re__debug_sexpr_new(re__debug_sexpr* sexpr, re__debug_sexpr_tree tree, re_int32* new_ref) {
    re_error err = RE_ERROR_NONE;
    re_int32 next_ref = (re_int32)re__debug_sexpr_tree_vec_size(&sexpr->tree);
    if ((err = re__debug_sexpr_tree_vec_push(&sexpr->tree, tree))) {
        return err;
    }
    *new_ref = next_ref;
    return err;
}

RE_INTERNAL int re__debug_sexpr_isblank(re_char ch) {
    return (ch == '\n') || (ch == '\t') || (ch == '\r') || (ch == ' ');
}

enum re__debug_sexpr_parse_state {
    RE__DEBUG_SEXPR_PARSE_STATE_NODE,
    RE__DEBUG_SEXPR_PARSE_STATE_ATOM
};

typedef struct re__debug_sexpr_parse_frame {
    re_int32 parent_ref;
    re_int32 child_ref;
} re__debug_sexpr_parse_frame;

RE_VEC_DECL(re__debug_sexpr_parse_frame);
RE_VEC_IMPL_FUNC(re__debug_sexpr_parse_frame, init)
RE_VEC_IMPL_FUNC(re__debug_sexpr_parse_frame, destroy)
RE_VEC_IMPL_FUNC(re__debug_sexpr_parse_frame, push)
RE_VEC_IMPL_FUNC(re__debug_sexpr_parse_frame, pop)
RE_VEC_IMPL_FUNC(re__debug_sexpr_parse_frame, getref)

typedef struct re__debug_sexpr_parse {
    re__debug_sexpr_parse_frame_vec sexpr_stack;
    re_size sexpr_stack_ptr;
    re__str atom_str;
} re__debug_sexpr_parse;

RE_INTERNAL void re__debug_sexpr_parse_push_tree(re__debug_sexpr* sexpr, re__debug_sexpr_parse* parse, re_int32 new_ref) {
    re__debug_sexpr_parse_frame* frame;
    re__debug_sexpr_tree* last_child;
    re__debug_sexpr_tree* parent;
    RE_ASSERT(parse->sexpr_stack_ptr);
    frame = re__debug_sexpr_parse_frame_vec_getref(&parse->sexpr_stack, parse->sexpr_stack_ptr - 1);
    if (frame->child_ref == RE__DEBUG_SEXPR_NONE) {
        parent = re__debug_sexpr_get(sexpr, frame->parent_ref);
        parent->first_child_ref = new_ref;
    } else {
        last_child = re__debug_sexpr_get(sexpr, frame->child_ref);
        last_child->next_sibling_ref = new_ref;
    }
    frame->child_ref = new_ref;
}

RE_INTERNAL re_error re__debug_sexpr_parse_new_node(re__debug_sexpr* sexpr, re__debug_sexpr_parse* parse) {
    re_error err = RE_ERROR_NONE;
    re__debug_sexpr_parse_frame new_frame;
    re__debug_sexpr_tree new_tree;
    re_int32 new_ref;
    re__debug_sexpr_tree_init(&new_tree, &parse->atom_str, 0);
    if ((err = re__debug_sexpr_new(sexpr, new_tree, &new_ref))) {
        return err;
    }
    new_frame.child_ref = RE__DEBUG_SEXPR_NONE;
    new_frame.parent_ref = new_ref;
    if (parse->sexpr_stack_ptr != 0) {
        re__debug_sexpr_parse_push_tree(sexpr, parse, new_ref);
    }
    if ((err = re__debug_sexpr_parse_frame_vec_push(&parse->sexpr_stack, new_frame))) {
        return err;
    }
    parse->sexpr_stack_ptr++;
    return err;
}

RE_INTERNAL re_error re__debug_sexpr_parse_new_atom(re__debug_sexpr* sexpr, re__debug_sexpr_parse* parse) {
    re_error err = RE_ERROR_NONE;
    re__debug_sexpr_tree new_tree;
    re_int32 new_ref;
    re__debug_sexpr_tree_init(&new_tree, &parse->atom_str, 1);
    re__str_destroy(&parse->atom_str);
    re__str_init(&parse->atom_str);
    if ((err = re__debug_sexpr_new(sexpr, new_tree, &new_ref))) {
        return err;
    }
    if (parse->sexpr_stack_ptr != 0) {
        re__debug_sexpr_parse_push_tree(sexpr, parse, new_ref);
    }
    return err;
}

RE_INTERNAL re_error re__debug_sexpr_parse_finish_node(re__debug_sexpr* sexpr, re__debug_sexpr_parse* parse) {
    re_error err = RE_ERROR_NONE;
    RE__UNUSED(sexpr);
    RE_ASSERT(parse->sexpr_stack_ptr);
    re__debug_sexpr_parse_frame_vec_pop(&parse->sexpr_stack);
    return err;
}

RE_INTERNAL re_error re__debug_sexpr_do_parse(re__debug_sexpr* sexpr, const re__str* in) {
    re__debug_sexpr_parse parse;
    re_size str_ptr = 0;
    re_error err;
    int state = RE__DEBUG_SEXPR_PARSE_STATE_NODE;
    re__debug_sexpr_parse_frame_vec_init(&parse.sexpr_stack);
    parse.sexpr_stack_ptr = 0;
    re__str_init(&parse.atom_str);
    while (str_ptr != re__str_size(in)) {
        re_char ch = re__str_get_data(in)[str_ptr];
        if (state == RE__DEBUG_SEXPR_PARSE_STATE_NODE) {
            if (re__debug_sexpr_isblank(ch)) {
                state = RE__DEBUG_SEXPR_PARSE_STATE_NODE;
            } else if (ch == '(') {
                if ((err = re__debug_sexpr_parse_new_node(sexpr, &parse))) {
                    goto error;
                }
                state = RE__DEBUG_SEXPR_PARSE_STATE_NODE;
            } else if (ch == ')') {
                if ((err = re__debug_sexpr_parse_finish_node(sexpr, &parse))) {
                    goto error;
                }
                state = RE__DEBUG_SEXPR_PARSE_STATE_NODE;
            } else {
                if ((err = re__str_cat_n(&parse.atom_str, 1, &ch))) {
                    goto error;
                }
                state = RE__DEBUG_SEXPR_PARSE_STATE_ATOM;
            }
        } else if (state == RE__DEBUG_SEXPR_PARSE_STATE_ATOM) {
            if (re__debug_sexpr_isblank(ch)) {
                if ((err = re__debug_sexpr_parse_new_atom(sexpr, &parse))) {
                    goto error;
                }
                state = RE__DEBUG_SEXPR_PARSE_STATE_NODE;
            } else if (ch == '(') {
                if ((err = re__debug_sexpr_parse_new_atom(sexpr, &parse))) {
                    goto error;
                }
                if ((err = re__debug_sexpr_parse_new_node(sexpr, &parse))) {
                    goto error;
                }
                state = RE__DEBUG_SEXPR_PARSE_STATE_NODE;
            } else if (ch == ')') {
                if ((err = re__debug_sexpr_parse_new_atom(sexpr, &parse))) {
                    goto error;
                }
                if ((err = re__debug_sexpr_parse_finish_node(sexpr, &parse))) {
                    goto error;
                }
                state = RE__DEBUG_SEXPR_PARSE_STATE_NODE;
            } else {
                if ((err = re__str_cat_n(&parse.atom_str, 1, &ch))) {
                    goto error;
                }
                state = RE__DEBUG_SEXPR_PARSE_STATE_ATOM;
            }
        }
        str_ptr++;
    }
error:
    re__str_destroy(&parse.atom_str);
    re__debug_sexpr_parse_frame_vec_destroy(&parse.sexpr_stack);
    return err;
}

RE_INTERNAL void re__debug_sexpr_dump(re__debug_sexpr* sexpr, re_int32 parent_ref, re_int32 indent) {
    re__debug_sexpr_tree* tree;
    re_int32 child_ref;
    re_int32 i;
    if (parent_ref == RE__DEBUG_SEXPR_NONE) {
        return;
    }
    tree = re__debug_sexpr_get(sexpr, parent_ref);
    if (tree->first_child_ref == RE__DEBUG_SEXPR_NONE) {
        if (tree->is_atom) {
            printf("%s", re__str_get_data(&tree->atom));
        } else {
            printf("()");
        }
    } else {
        printf("\n");
        for (i = 0; i < indent; i++) {
            printf("  ");
        }
        printf("(");
        child_ref = tree->first_child_ref;
        while (child_ref != RE__DEBUG_SEXPR_NONE) {
            re__debug_sexpr_tree* child = re__debug_sexpr_get(sexpr, child_ref);
            re__debug_sexpr_dump(sexpr, child_ref, indent+1);
            child_ref = child->next_sibling_ref;
            if (child_ref != RE__DEBUG_SEXPR_NONE) {
                printf(" ");
            }
        }
        printf(")");
    }
}

RE_INTERNAL int re__debug_sexpr_equals(re__debug_sexpr* sexpr, re__debug_sexpr* other, re_int32 sexpr_ref, re_int32 other_ref) {
    re__debug_sexpr_tree* parent_tree;
    re__debug_sexpr_tree* other_tree;
    if ((sexpr_ref == other_ref) && sexpr_ref == RE__DEBUG_SEXPR_NONE) {
        return 1;
    } else if (sexpr_ref == RE__DEBUG_SEXPR_NONE || other_ref == RE__DEBUG_SEXPR_NONE) {
        return 0;
    }
    parent_tree = re__debug_sexpr_get(sexpr, sexpr_ref);
    other_tree = re__debug_sexpr_get(other, other_ref);
    if (re__str_cmp(&parent_tree->atom, &other_tree->atom) != 0) {
        return 1;
    }
    {
        re_int32 parent_child_ref = parent_tree->first_child_ref;
        re_int32 other_child_ref = other_tree->first_child_ref;
        while (parent_child_ref != RE__DEBUG_SEXPR_NONE &&
            other_child_ref != RE__DEBUG_SEXPR_NONE) {
            if (!re__debug_sexpr_equals(sexpr, other, parent_child_ref, other_child_ref)) {
                return 0;
            }
        }
        return parent_child_ref != other_child_ref;
    }
}

RE_INTERNAL re_int32 re__debug_sexpr_new_tree(re__debug_sexpr* sexpr, re_int32 parent_ref, re__debug_sexpr_tree new_tree) {
    re_int32 new_ref;
    re__debug_sexpr_new(sexpr, new_tree, &new_ref);
    if (parent_ref == RE__DEBUG_SEXPR_NONE) {
        return new_ref;
    } else {
        re__debug_sexpr_tree* parent = re__debug_sexpr_get(sexpr, parent_ref);
        if (parent->first_child_ref == RE__DEBUG_SEXPR_NONE) {
            parent->first_child_ref = new_ref;
            return new_ref;
        } else {
            re_int32 child_ref = parent->first_child_ref;
            re__debug_sexpr_tree* child_tree = re__debug_sexpr_get(sexpr, child_ref);
            do {
                child_tree = re__debug_sexpr_get(sexpr, child_ref);
                child_ref = child_tree->next_sibling_ref;
            } while (child_ref != RE__DEBUG_SEXPR_NONE);
            child_tree->next_sibling_ref = new_ref;
            return new_ref;
        }
    }
}

RE_INTERNAL re_int32 re__debug_sexpr_new_node(re__debug_sexpr* sexpr, re_int32 parent_ref) {
    re__debug_sexpr_tree new_tree;
    re__str mt_str;
    re__str_init(&mt_str);
    re__debug_sexpr_tree_init(&new_tree, &mt_str, 0);
    re__str_destroy(&mt_str);
    return re__debug_sexpr_new_tree(sexpr, parent_ref, new_tree);
}

RE_INTERNAL void re__debug_sexpr_new_atom(re__debug_sexpr* sexpr, re_int32 parent_ref, const char* name) {
    re__debug_sexpr_tree new_tree;
    re__str str;
    re__str_init_s(&str, (const re_char*)name);
    re__debug_sexpr_tree_init(&new_tree, &str, 1);
    re__str_destroy(&str);
    re__debug_sexpr_new_tree(sexpr, parent_ref, new_tree);
}

RE_INTERNAL void re__debug_sexpr_new_int(re__debug_sexpr* sexpr, re_int32 parent_ref, re_int32 num) {
    re__debug_sexpr_tree new_tree;
    re_char sbuf[64];
    re__str str;
    re__str_init_n(&str, (re_size)sprintf((char*)sbuf, "%i", num), sbuf);
    re__debug_sexpr_tree_init(&new_tree, &str, 1);
    re__str_destroy(&str);
    re__debug_sexpr_new_tree(sexpr, parent_ref, new_tree);
}

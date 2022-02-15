#include "re_internal.h"

RE_INTERNAL void re__ast_init(re__ast* ast, re__ast_type type) {
    ast->type = type;
    ast->first_child_ref = -1;
    ast->prev_sibling_ref = -1;
    ast->next_sibling_ref = -1;
}

RE_INTERNAL void re__ast_init_rune(re__ast* ast, re_rune rune) {
    re__ast_init(ast, RE__AST_TYPE_RUNE);
    ast->_data.rune = rune;
}

/* Transfers ownership of charclass to the AST node. */
RE_INTERNAL void re__ast_init_class(re__ast* ast, re__charclass charclass) {
    re__ast_init(ast, RE__AST_TYPE_CLASS);
    ast->_data.charclass = charclass;
}

RE_INTERNAL void re__ast_init_concat(re__ast* ast) {
    re__ast_init(ast, RE__AST_TYPE_CONCAT);
}

RE_INTERNAL void re__ast_init_alt(re__ast* ast) {
    re__ast_init(ast, RE__AST_TYPE_ALT);
}

RE_INTERNAL void re__ast_init_quantifier(re__ast* ast, re_int32 min, re_int32 max) {
    re__ast_init(ast, RE__AST_TYPE_QUANTIFIER);
    RE_ASSERT(min != max);
    RE_ASSERT(min < max);
    ast->_data.quantifier_info.min = min;
    ast->_data.quantifier_info.max = max;
    ast->_data.quantifier_info.greediness = 1;
}

RE_INTERNAL void re__ast_init_group(re__ast* ast) {
    re__ast_init(ast, RE__AST_TYPE_GROUP);
    ast->_data.group_info.flags = 0;
    ast->_data.group_info.match_number = 0;
}

RE_INTERNAL void re__ast_init_assert(re__ast* ast, re__ast_assert_type assert_type) {
    re__ast_init(ast, RE__AST_TYPE_ASSERT);
    ast->_data.assert_type = assert_type;
}

RE_INTERNAL void re__ast_init_any_char(re__ast* ast) {
    re__ast_init(ast, RE__AST_TYPE_ANY_CHAR);
}

RE_INTERNAL void re__ast_init_any_byte(re__ast* ast) {
    re__ast_init(ast, RE__AST_TYPE_ANY_BYTE);
}

RE_INTERNAL void re__ast_destroy(re__ast* ast) {
    if (ast->type == RE__AST_TYPE_CLASS) {
        re__charclass_destroy(&ast->_data.charclass);
    }
}

RE_INTERNAL void re__ast_set_quantifier_greediness(re__ast* ast, int is_greedy) {
    RE_ASSERT(ast->type == RE__AST_TYPE_QUANTIFIER);
    ast->_data.quantifier_info.greediness = is_greedy;
}

RE_INTERNAL int re__ast_get_quantifier_greediness(re__ast* ast) {
    RE_ASSERT(ast->type == RE__AST_TYPE_QUANTIFIER);
    return ast->_data.quantifier_info.greediness;
}

RE_INTERNAL re_int32 re__ast_get_quantifier_min(re__ast* ast) {
    RE_ASSERT(ast->type == RE__AST_TYPE_QUANTIFIER);
    return ast->_data.quantifier_info.min;
}

RE_INTERNAL re_int32 re__ast_get_quantifier_max(re__ast* ast) {
    RE_ASSERT(ast->type == RE__AST_TYPE_QUANTIFIER);
    return ast->_data.quantifier_info.max;
}

RE_INTERNAL re_rune re__ast_get_rune(re__ast* ast) {
    RE_ASSERT(ast->type == RE__AST_TYPE_RUNE);
    return ast->_data.rune;
}

RE_INTERNAL re__ast_group_flags re__ast_get_group_flags(re__ast* ast) {
    RE_ASSERT(ast->type == RE__AST_TYPE_GROUP);
    return ast->_data.group_info.flags;
}

RE_INTERNAL void re__ast_set_group_flags(re__ast* ast, re__ast_group_flags flags) {
    RE_ASSERT(ast->type == RE__AST_TYPE_GROUP);
    ast->_data.group_info.flags = flags;
}

RE_INTERNAL re__ast_assert_type re__ast_get_assert_type(re__ast* ast) {
    RE_ASSERT(ast->type == RE__AST_TYPE_ASSERT);
    return ast->_data.assert_type;
}

RE_INTERNAL void re__ast_root_init(re__ast_root* ast_root) {
    re__ast_vec_init(&ast_root->ast_vec);
    ast_root->last_empty_ref = RE__AST_NONE;
    ast_root->root_ref = RE__AST_NONE;
}

RE_INTERNAL void re__ast_root_destroy(re__ast_root* ast_root) {
    re_size i;
    for (i = 0; i < re__ast_vec_size(&ast_root->ast_vec); i++) {
        re__ast_destroy(re__ast_vec_getref(&ast_root->ast_vec, i));
    }
    re__ast_vec_destroy(&ast_root->ast_vec);
}

RE_INTERNAL re__ast* re__ast_root_get(re__ast_root* ast_root, re_int32 ast_ref) {
    RE_ASSERT(ast_ref != RE__AST_NONE);
    RE_ASSERT(ast_ref < (re_int32)re__ast_vec_size(&ast_root->ast_vec));
    return re__ast_vec_getref(&ast_root->ast_vec, (re_size)ast_ref);
}

RE_INTERNAL void re__ast_root_remove(re__ast_root* ast_root, re_int32 ast_ref) {
    re__ast* empty = re__ast_root_get(ast_root, ast_ref);
    empty->type = RE__AST_TYPE_NONE;
    empty->next_sibling_ref = ast_root->last_empty_ref;
    ast_root->last_empty_ref = ast_ref;
}

RE_INTERNAL re_int32 re__ast_root_size(re__ast_root* ast_root) {
    return (re_int32)re__ast_vec_size(&ast_root->ast_vec);
}

RE_INTERNAL re_error re__ast_root_new(re__ast_root* ast_root, re__ast ast, re_int32* out_ref) {
    re_error err = RE_ERROR_NONE;
    re_int32 empty_ref = ast_root->last_empty_ref;
    if (empty_ref != RE__AST_NONE) {
        re__ast* empty = re__ast_root_get(ast_root,empty_ref);
        ast_root->last_empty_ref = empty->next_sibling_ref;
        *empty = ast;
        *out_ref = empty_ref;
    } else {
        re_int32 next_ref = (re_int32)re__ast_vec_size(&ast_root->ast_vec);
        if ((err = re__ast_vec_push(&ast_root->ast_vec, ast))) {
            return err;
        }
        *out_ref = next_ref;
    }
    return err;
}

RE_INTERNAL re_error re__ast_root_add_child(re__ast_root* ast_root, re_int32 parent_ref, re__ast ast, re_int32* out_ref) {
    re_error err = RE_ERROR_NONE;
    if ((err = re__ast_root_new(ast_root, ast, out_ref))) {
        return err;
    }
    if (parent_ref == RE__AST_NONE) {
        /* We can only add one child to the root. */
        RE_ASSERT(ast_root->root_ref == RE__AST_NONE);
        /* If this is the first child, set root to it. */
        ast_root->root_ref = *out_ref;
    } else {
        re__ast* parent = re__ast_root_get(ast_root, parent_ref);
        parent->first_child_ref = *out_ref;
    }
    return err;
}

RE_INTERNAL re_error re__ast_root_add_sibling(re__ast_root* ast_root, re_int32 prev_sibling_ref, re__ast ast, re_int32* out_ref) {
    re_error err = RE_ERROR_NONE;
    if ((err = re__ast_root_new(ast_root, ast, out_ref))) {
        return err;
    }
    RE_ASSERT(prev_sibling_ref != RE__AST_NONE);
    {
        re__ast* prev = re__ast_root_get(ast_root, prev_sibling_ref);
        re__ast* out = re__ast_root_get(ast_root, *out_ref);
        prev->next_sibling_ref = *out_ref;
        out->prev_sibling_ref = prev_sibling_ref;
    }
    return err;
}

RE_INTERNAL re_error re__ast_root_add_wrap(re__ast_root* ast_root, re_int32 parent_ref, re_int32 inner_ref, re__ast ast_outer, re_int32* out_ref){
    re_error err = RE_ERROR_NONE;
    re__ast *inner;
    re__ast *outer;
    if ((err = re__ast_root_new(ast_root, ast_outer, out_ref))) {
        return err;
    }
    /* If parent is root, then we *must* be wrapping root. */
    if (parent_ref == RE__AST_NONE) {
        RE_ASSERT(inner_ref == ast_root->root_ref);
        ast_root->root_ref = *out_ref;
    }
    inner = re__ast_root_get(ast_root, inner_ref);
    outer = re__ast_root_get(ast_root, *out_ref);
    if (inner->prev_sibling_ref != RE__AST_NONE) {
        re__ast* inner_prev_sibling = re__ast_root_get(ast_root, inner->prev_sibling_ref);
        inner_prev_sibling->next_sibling_ref = *out_ref;
        outer->prev_sibling_ref = inner->prev_sibling_ref;
    } else {
        if (parent_ref != RE__AST_NONE) {
            re__ast* parent = re__ast_root_get(ast_root, parent_ref);
            if (parent->first_child_ref == inner_ref) {
                /* Parent used to point to inner as child */
                parent->first_child_ref = *out_ref;
            }
        }
    }
    inner->prev_sibling_ref = RE__AST_NONE;
    outer->first_child_ref = inner_ref;
    return err;
}

#if RE_DEBUG

#if 0

RE_INTERNAL void re__ast_root_debug_dump_sexpr_rec(re__ast_root* ast_root, re__debug_sexpr* sexpr, re_int32 ast_root_ref, re_int32 sexpr_root_ref) {
    while (ast_root_ref != RE__AST_NONE) {
        const re__ast* ast = re__ast_root_get(ast_root, ast_root_ref);
        re_int32 ast_sexpr_ref = re__debug_sexpr_new_node(sexpr, sexpr_root_ref);
        switch (ast->type) {
            case RE__AST_TYPE_NONE:
                re__debug_sexpr_new_atom(sexpr, ast_sexpr_ref, "none");
                break;
            case RE__AST_TYPE_RUNE:
                re__debug_sexpr_new_atom(sexpr, ast_sexpr_ref, "char");
                re__debug_sexpr_new_int(sexpr, ast_sexpr_ref, ast->_data.rune);
                break;
            case RE__AST_TYPE_CLASS:
                /*re__charclass_dump(&ast->_data.charclass, (re_size)(lvl+1));*/
                break;
            case RE__AST_TYPE_CONCAT:
                re__debug_sexpr_new_atom(sexpr, ast_sexpr_ref, "concat");
                break;
            case RE__AST_TYPE_ALT:
                re__debug_sexpr_new_atom(sexpr, ast_sexpr_ref, "alt");
                break;
            case RE__AST_TYPE_QUANTIFIER:
                re__debug_sexpr_new_atom(sexpr, ast_sexpr_ref, "quantifier");
                re__debug_sexpr_new_int(sexpr, ast_sexpr_ref, ast->_data.quantifier_info.min);
                re__debug_sexpr_new_int(sexpr, ast_sexpr_ref, ast->_data.quantifier_info.max);
                if (ast->_data.quantifier_info.greediness) {
                    re__debug_sexpr_new_atom(sexpr, ast_sexpr_ref, "greedy");
                } else {
                    re__debug_sexpr_new_atom(sexpr, ast_sexpr_ref, "non-greedy");
                }
                break;
            case RE__AST_TYPE_GROUP: {
                re_int32 flags_node;
                re__debug_sexpr_new_atom(sexpr, ast_sexpr_ref, "group");
                flags_node = re__debug_sexpr_new_node(sexpr, ast_sexpr_ref);
                if (ast->_data.group_info.flags & RE__AST_GROUP_FLAG_CASE_INSENSITIVE) {
                    re__debug_sexpr_new_atom(sexpr, flags_node, "i");
                } else if (ast->_data.group_info.flags & RE__AST_GROUP_FLAG_MULTILINE) {
                    re__debug_sexpr_new_atom(sexpr, flags_node, "m");
                } else if (ast->_data.group_info.flags & RE__AST_GROUP_FLAG_DOT_NEWLINE) {
                    re__debug_sexpr_new_atom(sexpr, flags_node, "s");
                } else if (ast->_data.group_info.flags & RE__AST_GROUP_FLAG_UNGREEDY) {
                    re__debug_sexpr_new_atom(sexpr, flags_node, "U");
                } else if (ast->_data.group_info.flags & RE__AST_GROUP_FLAG_NONMATCHING) {
                    re__debug_sexpr_new_atom(sexpr, flags_node, ":");
                }
                break;
            }
            case RE__AST_TYPE_ASSERT:
                re__debug_sexpr_new_atom(sexpr, ast_sexpr_ref, "assert");
                break;
            case RE__AST_TYPE_ANY_CHAR:
                re__debug_sexpr_new_atom(sexpr, ast_sexpr_ref, "any-char");
                break;
            case RE__AST_TYPE_ANY_BYTE:
                re__debug_sexpr_new_atom(sexpr, ast_sexpr_ref, "any-byte");
                break;
            default:
                RE__ASSERT_UNREACHED();
                break;
        }
        re__ast_root_debug_dump_sexpr_rec(ast_root, sexpr, ast->first_child_ref, ast_sexpr_ref);
        ast_root_ref = ast->next_sibling_ref;
    }
}

RE_INTERNAL void re__ast_root_debug_dump_sexpr(re__ast_root* ast_root, re__debug_sexpr* sexpr, re_int32 sexpr_root_ref) {
    re_int32 root_node;
    root_node = re__debug_sexpr_new_node(sexpr, sexpr_root_ref);
    re__debug_sexpr_new_atom(sexpr, root_node, "ast");
    re__ast_root_debug_dump_sexpr_rec(ast_root, sexpr, 0, root_node);
}

#endif

RE_INTERNAL void re__ast_root_debug_dump(re__ast_root* ast_root, re_int32 root_ref, re_int32 lvl) {
    re_int32 i;
    while (root_ref != RE__AST_NONE) {
        const re__ast* ast = re__ast_root_get(ast_root, root_ref);
        for (i = 0; i < lvl; i++) {
            printf("  ");
        }
        printf("%04i | ", root_ref);
        if (ast->prev_sibling_ref == RE__AST_NONE) {
            printf("p=None ");
        } else {
            printf("p=%04i ", ast->next_sibling_ref);
        }
        if (ast->next_sibling_ref == RE__AST_NONE) {
            printf("n=None ");
        } else {
            printf("n=%04i ", ast->next_sibling_ref);
        }
        if (ast->first_child_ref == RE__AST_NONE) {
            printf("c=None ");
        } else {
            printf("c=%04i ", ast->first_child_ref);
        }
        switch (ast->type) {
            case RE__AST_TYPE_NONE:
                printf("NONE");
                break;
            case RE__AST_TYPE_RUNE:
                printf("CHAR: ord=%X ('%c')", ast->_data.rune, ast->_data.rune);
                break;
            case RE__AST_TYPE_CLASS:
                printf("CLASS:\n");
                re__charclass_dump(&ast->_data.charclass, (re_size)(lvl+1));
                break;
            case RE__AST_TYPE_CONCAT:
                printf("CONCAT");
                break;
            case RE__AST_TYPE_ALT:
                printf("ALT");
                break;
            case RE__AST_TYPE_QUANTIFIER:
                printf("QUANTIFIER: %i - %i; %s", 
                    ast->_data.quantifier_info.min,
                    ast->_data.quantifier_info.max,
                    ast->_data.quantifier_info.greediness ? "greedy" : "non-greedy"
                );
                break;
            case RE__AST_TYPE_GROUP:
                printf("GROUP: %c%c%c%c%c",
                    ((ast->_data.group_info.flags & RE__AST_GROUP_FLAG_CASE_INSENSITIVE) ? 'i' : ' '), 
                    ((ast->_data.group_info.flags & RE__AST_GROUP_FLAG_MULTILINE) ? 'm' : ' '), 
                    ((ast->_data.group_info.flags & RE__AST_GROUP_FLAG_DOT_NEWLINE) ? 's' : ' '), 
                    ((ast->_data.group_info.flags & RE__AST_GROUP_FLAG_UNGREEDY) ? 'U' : ' '), 
                    ((ast->_data.group_info.flags & RE__AST_GROUP_FLAG_NONMATCHING) ? ':' : ' ')
                );
                break;
            case RE__AST_TYPE_ASSERT:
                printf("ASSERT: %i", ast->_data.assert_type);
                break;
            case RE__AST_TYPE_ANY_CHAR:
                printf("ANY_CHAR");
                break;
            case RE__AST_TYPE_ANY_BYTE:
                printf("ANY_BYTE");
                break;
            default:
                RE__ASSERT_UNREACHED();
                break;
        }
        printf("\n");
        re__ast_root_debug_dump(ast_root, ast->first_child_ref, lvl + 1);
        root_ref = ast->next_sibling_ref;
    }
}

RE_VEC_DECL(re_int32);
RE_VEC_IMPL_FUNC(re_int32, init)
RE_VEC_IMPL_FUNC(re_int32, destroy)
RE_VEC_IMPL_FUNC(re_int32, push)
RE_VEC_IMPL_FUNC(re_int32, size)
RE_VEC_IMPL_FUNC(re_int32, get)

RE_INTERNAL int re__ast_root_verify(re__ast_root* ast_root) {
    re_int32_vec removed_list;
    re_int32_vec_init(&removed_list);
    if (ast_root->last_empty_ref != RE__AST_NONE) {
        re_int32 empty_ref = ast_root->last_empty_ref;
        while (1) {
            if (empty_ref == RE__AST_NONE) {
                break;
            }
            if (empty_ref >= re__ast_root_size(ast_root)) {
                /* empty refs can't exceed size */
                return 0;
            }
            {
                re_size i;
                /* no cycles in empty list */
                for (i = 0; i < re_int32_vec_size(&removed_list); i++) {
                    if (re_int32_vec_get(&removed_list, i) == empty_ref) {
                        return 0;
                    }
                }
            }
            {
                re__ast* empty = re__ast_root_get(ast_root,empty_ref);
                re_int32_vec_push(&removed_list, empty_ref);
                empty_ref = empty->next_sibling_ref;
            }
        }
    }
    {
        re_int32 i = 0;
        for (i = 0; i < re__ast_root_size(ast_root); i++) {
            re_size j;
            /* don't loop over empty nodes */
            for (j = 0; j < re_int32_vec_size(&removed_list); j++) {
                if (re_int32_vec_get(&removed_list, j) == i) {
                    goto cont;
                }
            }
            {
                re__ast* ast = re__ast_root_get(ast_root, i);
                re_int32 to_check[3];
                re_int32 k;
                to_check[0] = ast->prev_sibling_ref;
                to_check[1] = ast->next_sibling_ref;
                to_check[2] = ast->first_child_ref;
                for (k = 0; k < 3; k++) {
                    re_int32 candidate = to_check[k];
                    for (j = 0; j < re_int32_vec_size(&removed_list); j++) {
                        if (re_int32_vec_get(&removed_list, j) == candidate) {
                            /* node points to a removed node */
                            return 0;
                        }
                    }
                }
            }
            if (0) {
cont:
                continue;
            }
        }
    }
    re_int32_vec_destroy(&removed_list);
    return 1;
}

#endif

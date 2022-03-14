#include "re_internal.h"

MN_INTERNAL void re__ast_init(re__ast* ast, re__ast_type type) {
    ast->type = type;
    ast->first_child_ref = RE__AST_NONE;
    ast->last_child_ref = RE__AST_NONE;
    ast->prev_sibling_ref = RE__AST_NONE;
    ast->next_sibling_ref = RE__AST_NONE;
}

MN_INTERNAL void re__ast_init_rune(re__ast* ast, re_rune rune) {
    re__ast_init(ast, RE__AST_TYPE_RUNE);
    ast->_data.rune = rune;
}

MN_INTERNAL void re__ast_init_str(re__ast* ast, mn_int32 str_ref) {
    re__ast_init(ast, RE__AST_TYPE_STR);
    ast->_data.str_ref = str_ref;
}

MN_INTERNAL void re__ast_init_charclass(re__ast* ast, mn_int32 charclass_ref) {
    re__ast_init(ast, RE__AST_TYPE_CHARCLASS);
    ast->_data.charclass_ref = charclass_ref;
}

MN_INTERNAL void re__ast_init_concat(re__ast* ast) {
    re__ast_init(ast, RE__AST_TYPE_CONCAT);
}

MN_INTERNAL void re__ast_init_alt(re__ast* ast) {
    re__ast_init(ast, RE__AST_TYPE_ALT);
}

MN_INTERNAL void re__ast_init_quantifier(re__ast* ast, mn_int32 min, mn_int32 max) {
    re__ast_init(ast, RE__AST_TYPE_QUANTIFIER);
    MN_ASSERT(min != max);
    MN_ASSERT(min < max);
    ast->_data.quantifier_info.min = min;
    ast->_data.quantifier_info.max = max;
    ast->_data.quantifier_info.greediness = 1;
}

MN_INTERNAL void re__ast_init_group(re__ast* ast, mn_uint32 group_idx, re__ast_group_flags group_flags) {
    re__ast_init(ast, RE__AST_TYPE_GROUP);
    if (group_flags & RE__AST_GROUP_FLAG_NONMATCHING) {
        MN_ASSERT(!(group_flags & RE__AST_GROUP_FLAG_NAMED));
        MN_ASSERT(group_idx == 0);
    }
    ast->_data.group_info.flags = group_flags;
    ast->_data.group_info.group_idx = group_idx;
}

MN_INTERNAL void re__ast_init_assert(re__ast* ast, re__ast_assert_type assert_type) {
    re__ast_init(ast, RE__AST_TYPE_ASSERT);
    ast->_data.assert_type = assert_type;
}

MN_INTERNAL void re__ast_init_any_char(re__ast* ast) {
    re__ast_init(ast, RE__AST_TYPE_ANY_CHAR);
}

MN_INTERNAL void re__ast_init_any_byte(re__ast* ast) {
    re__ast_init(ast, RE__AST_TYPE_ANY_BYTE);
}

MN_INTERNAL void re__ast_destroy(re__ast* ast) {
    MN__UNUSED(ast);
}

MN_INTERNAL void re__ast_set_quantifier_greediness(re__ast* ast, int is_greedy) {
    MN_ASSERT(ast->type == RE__AST_TYPE_QUANTIFIER);
    ast->_data.quantifier_info.greediness = is_greedy;
}

MN_INTERNAL int re__ast_get_quantifier_greediness(const re__ast* ast) {
    MN_ASSERT(ast->type == RE__AST_TYPE_QUANTIFIER);
    return ast->_data.quantifier_info.greediness;
}

MN_INTERNAL mn_int32 re__ast_get_quantifier_min(const re__ast* ast) {
    MN_ASSERT(ast->type == RE__AST_TYPE_QUANTIFIER);
    return ast->_data.quantifier_info.min;
}

MN_INTERNAL mn_int32 re__ast_get_quantifier_max(const re__ast* ast) {
    MN_ASSERT(ast->type == RE__AST_TYPE_QUANTIFIER);
    return ast->_data.quantifier_info.max;
}

MN_INTERNAL re_rune re__ast_get_rune(const re__ast* ast) {
    MN_ASSERT(ast->type == RE__AST_TYPE_RUNE);
    return ast->_data.rune;
}

MN_INTERNAL re__ast_group_flags re__ast_get_group_flags(const re__ast* ast) {
    MN_ASSERT(ast->type == RE__AST_TYPE_GROUP);
    return ast->_data.group_info.flags;
}

MN_INTERNAL mn_uint32 re__ast_get_group_idx(const re__ast* ast) {
    MN_ASSERT(ast->type == RE__AST_TYPE_GROUP);
    return ast->_data.group_info.group_idx;
}

MN_INTERNAL re__ast_assert_type re__ast_get_assert_type(const re__ast* ast) {
    MN_ASSERT(ast->type == RE__AST_TYPE_ASSERT);
    return ast->_data.assert_type;
}

MN_INTERNAL mn_int32 re__ast_get_str_ref(const re__ast* ast) {
    MN_ASSERT(ast->type == RE__AST_TYPE_STR);
    return ast->_data.str_ref;
}

MN__ARENA_IMPL_FUNC(re__charclass, init)
MN__ARENA_IMPL_FUNC(re__charclass, destroy)
MN__ARENA_IMPL_FUNC(re__charclass, getref)
MN__ARENA_IMPL_FUNC(re__charclass, getcref)
MN__ARENA_IMPL_FUNC(re__charclass, begin)
MN__ARENA_IMPL_FUNC(re__charclass, next)
MN__ARENA_IMPL_FUNC(re__charclass, add)

MN__ARENA_IMPL_FUNC(mn__str, init)
MN__ARENA_IMPL_FUNC(mn__str, destroy)
MN__ARENA_IMPL_FUNC(mn__str, getref)
MN__ARENA_IMPL_FUNC(mn__str, getcref)
MN__ARENA_IMPL_FUNC(mn__str, begin)
MN__ARENA_IMPL_FUNC(mn__str, next)
MN__ARENA_IMPL_FUNC(mn__str, add)

MN__VEC_IMPL_FUNC(mn__str, init)
MN__VEC_IMPL_FUNC(mn__str, destroy)
MN__VEC_IMPL_FUNC(mn__str, push)
MN__VEC_IMPL_FUNC(mn__str, getref)
MN__VEC_IMPL_FUNC(mn__str, size)

MN__VEC_IMPL_FUNC(re__ast, init)
MN__VEC_IMPL_FUNC(re__ast, destroy)
MN__VEC_IMPL_FUNC(re__ast, get)
MN__VEC_IMPL_FUNC(re__ast, get_data)
MN__VEC_IMPL_FUNC(re__ast, getref)
MN__VEC_IMPL_FUNC(re__ast, getcref)
MN__VEC_IMPL_FUNC(re__ast, insert)
MN__VEC_IMPL_FUNC(re__ast, peek)
MN__VEC_IMPL_FUNC(re__ast, pop)
MN__VEC_IMPL_FUNC(re__ast, push)
MN__VEC_IMPL_FUNC(re__ast, size)

MN_INTERNAL void re__ast_root_init(re__ast_root* ast_root) {
    re__ast_vec_init(&ast_root->ast_vec);
    ast_root->last_empty_ref = RE__AST_NONE;
    ast_root->root_ref = RE__AST_NONE;
    ast_root->root_last_child_ref = RE__AST_NONE;
    ast_root->depth_max = 0;
    re__charclass_arena_init(&ast_root->charclasses);
    mn__str_arena_init(&ast_root->strings);
    mn__str_vec_init(&ast_root->group_names);
}

MN_INTERNAL void re__ast_root_destroy(re__ast_root* ast_root) {
    mn_size i;
    mn_int32 cur_ref;
    for (i = 0; i < mn__str_vec_size(&ast_root->group_names); i++) {
        mn__str_destroy(mn__str_vec_getref(&ast_root->group_names, i));
    }
    mn__str_vec_destroy(&ast_root->group_names);
    cur_ref = re__charclass_arena_begin(&ast_root->charclasses);
    while (cur_ref != MN__ARENA_REF_NONE) {
        re__charclass* cur = re__charclass_arena_getref(&ast_root->charclasses, cur_ref);
        re__charclass_destroy(cur);
        cur_ref = re__charclass_arena_next(&ast_root->charclasses, cur_ref);
    }
    re__charclass_arena_destroy(&ast_root->charclasses);
    cur_ref = mn__str_arena_begin(&ast_root->strings);
    while (cur_ref != MN__ARENA_REF_NONE) {
        mn__str* cur = mn__str_arena_getref(&ast_root->strings, cur_ref);
        mn__str_destroy(cur);
        cur_ref = mn__str_arena_next(&ast_root->strings, cur_ref);
    }
    mn__str_arena_destroy(&ast_root->strings);
    for (i = 0; i < re__ast_vec_size(&ast_root->ast_vec); i++) {
        re__ast_destroy(re__ast_vec_getref(&ast_root->ast_vec, i));
    }
    re__ast_vec_destroy(&ast_root->ast_vec);
}

MN_INTERNAL re__ast* re__ast_root_get(re__ast_root* ast_root, mn_int32 ast_ref) {
    MN_ASSERT(ast_ref != RE__AST_NONE);
    MN_ASSERT(ast_ref < (mn_int32)re__ast_vec_size(&ast_root->ast_vec));
    return re__ast_vec_getref(&ast_root->ast_vec, (mn_size)ast_ref);
}

MN_INTERNAL const re__ast* re__ast_root_get_const(const re__ast_root* ast_root, mn_int32 ast_ref) {
    MN_ASSERT(ast_ref != RE__AST_NONE);
    MN_ASSERT(ast_ref < (mn_int32)re__ast_vec_size(&ast_root->ast_vec));
    return re__ast_vec_getcref(&ast_root->ast_vec, (mn_size)ast_ref);
}

MN_INTERNAL void re__ast_root_remove(re__ast_root* ast_root, mn_int32 ast_ref) {
    re__ast* empty = re__ast_root_get(ast_root, ast_ref);
    if (ast_root->root_ref == ast_ref) {
        ast_root->root_ref = RE__AST_NONE;
    }
    empty->type = RE__AST_TYPE_NONE;
    empty->next_sibling_ref = ast_root->last_empty_ref;
    ast_root->last_empty_ref = ast_ref;
}

MN_INTERNAL void re__ast_root_replace(re__ast_root* ast_root, mn_int32 ast_ref, re__ast replacement) {
    re__ast* loc = re__ast_root_get(ast_root, ast_ref);
    MN_ASSERT(loc->next_sibling_ref == RE__AST_NONE);
    MN_ASSERT(loc->first_child_ref == RE__AST_NONE);
    MN_ASSERT(loc->last_child_ref == RE__AST_NONE);
    replacement.next_sibling_ref = loc->next_sibling_ref;
    replacement.first_child_ref = loc->first_child_ref;
    replacement.prev_sibling_ref = loc->prev_sibling_ref;
    replacement.last_child_ref = loc->last_child_ref;
    *loc = replacement;
}

MN_INTERNAL mn_int32 re__ast_root_size(re__ast_root* ast_root) {
    return (mn_int32)re__ast_vec_size(&ast_root->ast_vec);
}

MN_INTERNAL re_error re__ast_root_new(re__ast_root* ast_root, re__ast ast, mn_int32* out_ref) {
    re_error err = RE_ERROR_NONE;
    mn_int32 empty_ref = ast_root->last_empty_ref;
    if (empty_ref != RE__AST_NONE) {
        re__ast* empty = re__ast_root_get(ast_root,empty_ref);
        ast_root->last_empty_ref = empty->next_sibling_ref;
        *empty = ast;
        *out_ref = empty_ref;
    } else {
        mn_int32 next_ref = (mn_int32)re__ast_vec_size(&ast_root->ast_vec);
        if ((err = re__ast_vec_push(&ast_root->ast_vec, ast))) {
            return err;
        }
        *out_ref = next_ref;
    }
    return err;
}

MN_INTERNAL re_error re__ast_root_add_child(re__ast_root* ast_root, mn_int32 parent_ref, re__ast ast, mn_int32* out_ref) {
    re_error err = RE_ERROR_NONE;
    mn_int32 prev_sibling_ref = RE__AST_NONE;
    re__ast* prev_sibling;
    re__ast* out;
    if ((err = re__ast_root_new(ast_root, ast, out_ref))) {
        return err;
    }
    if (parent_ref == RE__AST_NONE) {
        if (ast_root->root_ref == RE__AST_NONE) {
            /* If this is the first child, set root to it. */
            ast_root->root_ref = *out_ref;
            ast_root->root_last_child_ref = *out_ref;
        } else {
            prev_sibling_ref = ast_root->root_last_child_ref;
            ast_root->root_last_child_ref = *out_ref;
        }
    } else {
        re__ast* parent = re__ast_root_get(ast_root, parent_ref);
        if (parent->first_child_ref == RE__AST_NONE) {
            parent->first_child_ref = *out_ref;
            parent->last_child_ref = *out_ref;
        } else {
            prev_sibling_ref = parent->last_child_ref;
            parent->last_child_ref = *out_ref;
        }
    }
    if (prev_sibling_ref != RE__AST_NONE) {
        prev_sibling = re__ast_root_get(ast_root, prev_sibling_ref);
        prev_sibling->next_sibling_ref = *out_ref;
        out = re__ast_root_get(ast_root, *out_ref);
        out->prev_sibling_ref = prev_sibling_ref;
    }
    return err;
}
/*
MN_INTERNAL re_error re__ast_root_add_sibling(re__ast_root* ast_root, mn_int32 prev_sibling_ref, re__ast ast, mn_int32* out_ref) {
    re_error err = RE_ERROR_NONE;
    if ((err = re__ast_root_new(ast_root, ast, out_ref))) {
        return err;
    }
    MN_ASSERT(prev_sibling_ref != RE__AST_NONE);
    {
        re__ast* prev = re__ast_root_get(ast_root, prev_sibling_ref);
        re__ast* out = re__ast_root_get(ast_root, *out_ref);
        prev->next_sibling_ref = *out_ref;
        out->prev_sibling_ref = prev_sibling_ref;
    }
    return err;
}*/

MN_INTERNAL re_error re__ast_root_add_wrap(re__ast_root* ast_root, mn_int32 parent_ref, mn_int32 inner_ref, re__ast ast_outer, mn_int32* out_ref){
    re_error err = RE_ERROR_NONE;
    re__ast* inner;
    re__ast* outer;
    re__ast* parent;
    if ((err = re__ast_root_new(ast_root, ast_outer, out_ref))) {
        return err;
    }
    /* If parent is root, then we *must* be wrapping root. */
    /*if (parent_ref == RE__AST_NONE) {
        MN_ASSERT(inner_ref == ast_root->root_ref);
        ast_root->root_ref = *out_ref;
    }*/
    inner = re__ast_root_get(ast_root, inner_ref);
    outer = re__ast_root_get(ast_root, *out_ref);
    if (parent_ref != RE__AST_NONE) {
        parent = re__ast_root_get(ast_root, parent_ref);
    }
    if (inner->prev_sibling_ref == RE__AST_NONE) {
        if (parent_ref == RE__AST_NONE) {
            ast_root->root_ref = *out_ref;
        } else {
            parent->first_child_ref = *out_ref;
        }
    } else {
        re__ast* inner_prev_sibling = re__ast_root_get(ast_root, inner->prev_sibling_ref);
        inner_prev_sibling->next_sibling_ref = *out_ref;
        outer->prev_sibling_ref = inner->prev_sibling_ref;
    }
    if (inner->next_sibling_ref == RE__AST_NONE) {
        if (parent_ref == RE__AST_NONE) {
            ast_root->root_last_child_ref = *out_ref;
        } else {
            parent->last_child_ref = *out_ref;
        }
    } else {
        re__ast* inner_next_sibling = re__ast_root_get(ast_root, inner->next_sibling_ref);
        inner_next_sibling->prev_sibling_ref = *out_ref;
        outer->next_sibling_ref = inner->next_sibling_ref;
    }
    inner->prev_sibling_ref = RE__AST_NONE;
    inner->next_sibling_ref = RE__AST_NONE;
    outer->first_child_ref = inner_ref;
    outer->last_child_ref = inner_ref;
    return err;
}

MN_INTERNAL re_error re__ast_root_add_charclass(re__ast_root* ast_root, re__charclass charclass, mn_int32* out_ref) {
    return re__charclass_arena_add(&ast_root->charclasses, charclass, out_ref);
}

MN_INTERNAL const re__charclass* re__ast_root_get_charclass(const re__ast_root* ast_root, mn_int32 charclass_ref) {
    return re__charclass_arena_getcref(&ast_root->charclasses, charclass_ref);
}

MN_INTERNAL re_error re__ast_root_add_str(re__ast_root* ast_root, mn__str str, mn_int32* out_ref) {
    return mn__str_arena_add(&ast_root->strings, str, out_ref);
}

MN_INTERNAL mn__str* re__ast_root_get_str(re__ast_root* ast_root, mn_int32 str_ref) {
    return mn__str_arena_getref(&ast_root->strings, str_ref);
}

MN_INTERNAL mn__str_view re__ast_root_get_str_view(const re__ast_root* ast_root, mn_int32 str_ref) {
    mn__str_view out;
    const mn__str* src = mn__str_arena_getcref(&ast_root->strings, str_ref);
    mn__str_view_init(&out, src);
    return out;
}

MN_INTERNAL re_error re__ast_root_add_group(re__ast_root* ast_root, mn__str_view group_name) {
    mn__str new_str;
    mn__str_init_n(&new_str, mn__str_view_get_data(&group_name), mn__str_view_size(&group_name));
    return mn__str_vec_push(&ast_root->group_names, new_str);
}

MN_INTERNAL mn__str_view re__ast_root_get_group(re__ast_root* ast_root, mn_uint32 group_number) {
    mn__str_view view;
    mn__str_view_init(&view, mn__str_vec_getref(&ast_root->group_names, (mn_size)group_number));
    return view;
}

MN_INTERNAL mn_uint32 re__ast_root_get_num_groups(re__ast_root* ast_root) {
    return (mn_uint32)mn__str_vec_size(&ast_root->group_names);
}

#if MN_DEBUG

#if 0

MN_INTERNAL void re__ast_root_debug_dump_sexpr_rec(re__ast_root* ast_root, re__debug_sexpr* sexpr, mn_int32 ast_root_ref, mn_int32 sexpr_root_ref) {
    while (ast_root_ref != RE__AST_NONE) {
        const re__ast* ast = re__ast_root_get(ast_root, ast_root_ref);
        mn_int32 ast_sexpr_ref = re__debug_sexpr_new_node(sexpr, sexpr_root_ref);
        switch (ast->type) {
            case RE__AST_TYPE_NONE:
                re__debug_sexpr_new_atom(sexpr, ast_sexpr_ref, "none");
                break;
            case RE__AST_TYPE_RUNE:
                re__debug_sexpr_new_atom(sexpr, ast_sexpr_ref, "char");
                re__debug_sexpr_new_int(sexpr, ast_sexpr_ref, ast->_data.rune);
                break;
            case RE__AST_TYPE_CLASS:
                /*re__charclass_dump(&ast->_data.charclass, (mn_size)(lvl+1));*/
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
                mn_int32 flags_node;
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
                MN__ASSERT_UNREACHED();
                break;
        }
        re__ast_root_debug_dump_sexpr_rec(ast_root, sexpr, ast->first_child_ref, ast_sexpr_ref);
        ast_root_ref = ast->next_sibling_ref;
    }
}

MN_INTERNAL void re__ast_root_debug_dump_sexpr(re__ast_root* ast_root, re__debug_sexpr* sexpr, mn_int32 sexpr_root_ref) {
    mn_int32 root_node;
    root_node = re__debug_sexpr_new_node(sexpr, sexpr_root_ref);
    re__debug_sexpr_new_atom(sexpr, root_node, "ast");
    re__ast_root_debug_dump_sexpr_rec(ast_root, sexpr, 0, root_node);
}

#endif

#include <stdio.h>

MN_INTERNAL void re__ast_root_debug_dump(re__ast_root* ast_root, mn_int32 root_ref, mn_int32 lvl) {
    mn_int32 i;
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
            case RE__AST_TYPE_STR: {
                mn__str_view str_view = re__ast_root_get_str_view(ast_root, ast->_data.str_ref);
                printf("STR: \"%s\"", mn__str_view_get_data(&str_view));
                break;
            }
            case RE__AST_TYPE_CHARCLASS: {
                const re__charclass* charclass;
                printf("CLASS:\n");
                charclass = re__charclass_arena_getcref(&ast_root->charclasses, ast->_data.charclass_ref);
                re__charclass_dump(charclass, (mn_size)(lvl+1));
                break;
            }
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
                MN__ASSERT_UNREACHED();
                break;
        }
        printf("\n");
        re__ast_root_debug_dump(ast_root, ast->first_child_ref, lvl + 1);
        root_ref = ast->next_sibling_ref;
    }
}

MN__VEC_DECL(mn_int32);
MN__VEC_IMPL_FUNC(mn_int32, init)
MN__VEC_IMPL_FUNC(mn_int32, destroy)
MN__VEC_IMPL_FUNC(mn_int32, push)
MN__VEC_IMPL_FUNC(mn_int32, size)
MN__VEC_IMPL_FUNC(mn_int32, get)

MN_INTERNAL int re__ast_root_verify_depth(re__ast_root* ast_root, mn_int32 start_ref, mn_int32 depth) {
    re__ast* cur_node;
    if (depth == 0) {
        if (start_ref == RE__AST_NONE) {
            return 1;
        } else {
            return 0;
        }
    } else {
        while (start_ref != RE__AST_NONE) {
            cur_node = re__ast_root_get(ast_root, start_ref);
            if (!re__ast_root_verify_depth(ast_root, cur_node->first_child_ref, depth - 1)) {
                return 0;
            }
            start_ref = cur_node->next_sibling_ref;
        }
    }
    return 1;
}

MN_INTERNAL int re__ast_root_verify(re__ast_root* ast_root) {
    mn_int32_vec removed_list;
    mn_int32_vec_init(&removed_list);
    if (ast_root->last_empty_ref != RE__AST_NONE) {
        mn_int32 empty_ref = ast_root->last_empty_ref;
        while (1) {
            if (empty_ref == RE__AST_NONE) {
                break;
            }
            if (empty_ref >= re__ast_root_size(ast_root)) {
                /* empty refs can't exceed size */
                return 0;
            }
            {
                mn_size i;
                /* no cycles in empty list */
                for (i = 0; i < mn_int32_vec_size(&removed_list); i++) {
                    if (mn_int32_vec_get(&removed_list, i) == empty_ref) {
                        return 0;
                    }
                }
            }
            {
                re__ast* empty = re__ast_root_get(ast_root,empty_ref);
                mn_int32_vec_push(&removed_list, empty_ref);
                empty_ref = empty->next_sibling_ref;
            }
        }
    }
    {
        mn_int32 i = 0;
        for (i = 0; i < re__ast_root_size(ast_root); i++) {
            mn_size j;
            /* don't loop over empty nodes */
            for (j = 0; j < mn_int32_vec_size(&removed_list); j++) {
                if (mn_int32_vec_get(&removed_list, j) == i) {
                    goto cont;
                }
            }
            {
                re__ast* ast = re__ast_root_get(ast_root, i);
                mn_int32 to_check[3];
                mn_int32 k;
                to_check[0] = ast->prev_sibling_ref;
                to_check[1] = ast->next_sibling_ref;
                to_check[2] = ast->first_child_ref;
                for (k = 0; k < 3; k++) {
                    mn_int32 candidate = to_check[k];
                    for (j = 0; j < mn_int32_vec_size(&removed_list); j++) {
                        if (mn_int32_vec_get(&removed_list, j) == candidate) {
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
    mn_int32_vec_destroy(&removed_list);
    return re__ast_root_verify_depth(ast_root, ast_root->root_ref, ast_root->depth_max);
}

#endif

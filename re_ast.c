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
}

RE_INTERNAL void re__ast_root_destroy(re__ast_root* ast_root) {
    re__ast_vec_destroy(&ast_root->ast_vec);
}

RE_INTERNAL re__ast* re__ast_root_get(re__ast_root* ast_root, re_int32 ast_ref) {
    RE_ASSERT(ast_ref != RE__AST_NONE);
    RE_ASSERT(ast_ref < (re_int32)re__ast_vec_size(&ast_root->ast_vec));
    return re__ast_vec_getref(&ast_root->ast_vec, (re_size)ast_ref);
}

RE_INTERNAL re_error re__ast_root_add(re__ast_root* ast_root, re__ast ast, re_int32* out_ref) {
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

RE_INTERNAL void re__ast_root_remove(re__ast_root* ast_root, re_int32 ast_ref) {
    re__ast* empty = re__ast_root_get(ast_root, ast_ref);
    empty->next_sibling_ref = ast_root->last_empty_ref;
    ast_root->last_empty_ref = ast_ref;
}

RE_INTERNAL void re__ast_root_link_siblings(re__ast_root* ast_root, re_int32 first_sibling_ref, re_int32 next_sibling_ref) {
    re__ast* first_sibling = re__ast_root_get(ast_root, first_sibling_ref);
    re__ast* next_sibling = re__ast_root_get(ast_root, next_sibling_ref);
    first_sibling->next_sibling_ref = next_sibling_ref;
    next_sibling->prev_sibling_ref = first_sibling_ref;
}

RE_INTERNAL void re__ast_root_set_child(re__ast_root* ast_root, re_int32 root_ref, re_int32 child_ref) {
    re__ast* root = re__ast_root_get(ast_root, root_ref);
    root->first_child_ref = child_ref;
}

RE_INTERNAL void re__ast_root_wrap(re__ast_root* ast_root, re_int32 parent_ref, re_int32 inner_ref, re_int32 outer_ref) {
    re__ast* inner = re__ast_root_get(ast_root, inner_ref);
    re__ast* outer = re__ast_root_get(ast_root, outer_ref);
    if (inner->prev_sibling_ref != RE__AST_NONE) {
        re__ast* inner_prev_sibling = re__ast_root_get(ast_root, inner->prev_sibling_ref);
        inner_prev_sibling->next_sibling_ref = outer_ref;
        outer->prev_sibling_ref = inner->prev_sibling_ref;
    } else {
        re__ast* parent = re__ast_root_get(ast_root, parent_ref);
        parent->first_child_ref = outer_ref;
    }
    inner->prev_sibling_ref = RE__AST_NONE;
    outer->first_child_ref = inner_ref;
}

#if RE_DEBUG

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

#endif

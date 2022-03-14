#include "test_ast.h"

#include "test_charclass.h"
#include "test_range.h"

const char* ast_sym_types[RE__AST_TYPE_MAX] = {
    "none",
    "rune",
    "str",
    "charclass",
    "concat",
    "alt",
    "quantifier",
    "group",
    "assert",
    "any_char",
    "any_byte"
};

int re__ast_group_flags_to_sym(sym_build* parent, re__ast_group_flags group_flags) {
    sym_build build;
    SYM_PUT_EXPR(parent, &build);
    if (group_flags & RE__AST_GROUP_FLAG_CASE_INSENSITIVE) {
        SYM_PUT_STR(&build, "case_insensitive");
    }
    if (group_flags & RE__AST_GROUP_FLAG_MULTILINE) {
        SYM_PUT_STR(&build, "multiline");
    }
    if (group_flags & RE__AST_GROUP_FLAG_DOT_NEWLINE) {
        SYM_PUT_STR(&build, "dot_newline");
    }
    if (group_flags & RE__AST_GROUP_FLAG_UNGREEDY) {
        SYM_PUT_STR(&build, "ungreedy");
    }
    if (group_flags & RE__AST_GROUP_FLAG_NONMATCHING) {
        SYM_PUT_STR(&build, "nonmatching");
    }
    if (group_flags & RE__AST_GROUP_FLAG_NAMED) {
        SYM_PUT_STR(&build, "named");
    }
    return SYM_OK;
}

static const char* group_flag_sym_types[] = {
    "case_insensitive",
    "multiline",
    "dot_newline",
    "ungreedy",
    "nonmatching",
    "named"
};

int re__ast_group_flags_from_sym(sym_walk* parent, re__ast_group_flags* group_flags) {
    sym_walk walk;
    SYM_GET_EXPR(parent, &walk);
    *group_flags = 0;
    while (SYM_MORE(&walk)) {
        const char* str;
        mptest_size n;
        int i = 0;
        re__ast_group_flags cur_flag = RE__AST_GROUP_FLAG_CASE_INSENSITIVE;
        mn__str_view view_a, view_b;
        SYM_GET_STR(&walk, &str, &n);
        mn__str_view_init_n(&view_a, (const mn_char*)str, n);
        while (cur_flag != RE__AST_GROUP_FLAG_MAX) {
            mn__str_view_init_s(&view_b, group_flag_sym_types[i]);
            if (mn__str_view_cmp(&view_a, &view_b) == 0) {
                *group_flags |= cur_flag;
            }
            cur_flag <<= 1;
            i++;
        }
    }
    return SYM_OK;
}

static const char* assert_type_sym_types[] = {
    "text_start",
    "text_end",
    "text_start_absolute",
    "text_end_absolute",
    "word",
    "word_not"
};

int re__ast_assert_type_to_sym(sym_build* parent, re__ast_assert_type assert_type) {
    sym_build build;
    int i = 0;
    re__ast_assert_type cur_flag = RE__AST_ASSERT_TYPE_MIN;
    SYM_PUT_EXPR(parent, &build);
    while (cur_flag != RE__AST_ASSERT_TYPE_MAX) {
        if (cur_flag & assert_type) {
            SYM_PUT_STR(&build, assert_type_sym_types[i]);
        }
        i++;
        cur_flag <<= 1;
    }
    return SYM_OK;
}

int re__ast_assert_type_from_sym(sym_walk* parent, re__ast_assert_type* assert_type) {
    sym_walk walk;
    *assert_type = 0;
    SYM_GET_EXPR(parent, &walk);
    while (SYM_MORE(&walk)) {
        const char* str;
        mptest_size str_size;
        mn__str_view view_a, view_b;
        re__ast_assert_type cur_flag = RE__AST_ASSERT_TYPE_MIN;
        int i = 0;
        SYM_GET_STR(&walk, &str, &str_size);
        mn__str_view_init_n(&view_a, str, str_size);
        while (cur_flag != RE__AST_ASSERT_TYPE_MAX) {
            mn__str_view_init_s(&view_b, assert_type_sym_types[i]);
            if (mn__str_view_cmp(&view_a, &view_b) == 0) {
                *assert_type |= cur_flag;
            }
            cur_flag <<= 1;
            i++;
        }
    }
    return SYM_OK;
}

int re__ast_root_to_sym_r(sym_build* parent, re__ast_root* ast_root, re__ast* ast) {
    sym_build build;
    re__ast_type type = ast->type;
    if (type == RE__AST_TYPE_CHARCLASS) {
        const re__charclass* charclass;
        charclass = re__ast_root_get_charclass(ast_root, ast->_data.charclass_ref);
        SYM_PUT_SUB(parent, re__charclass, *charclass);
        return SYM_OK;
    }
    SYM_PUT_EXPR(parent, &build);
    SYM_PUT_TYPE(&build, ast_sym_types[type]);
    if (type == RE__AST_TYPE_RUNE) {
        SYM_PUT_NUM(&build, re__ast_get_rune(ast));
    } else if (type == RE__AST_TYPE_STR) {
        mn__str_view view = re__ast_root_get_str_view(ast_root, ast->_data.str_ref);
        SYM_PUT_STRN(&build, mn__str_view_get_data(&view), mn__str_view_size(&view));
    } else if (type == RE__AST_TYPE_QUANTIFIER) {
        const char* greed = re__ast_get_quantifier_greediness(ast) ? "greedy" : "nongreedy";
        mn_int32 max = re__ast_get_quantifier_max(ast);
        SYM_PUT_NUM(&build, re__ast_get_quantifier_min(ast));
        if (max == RE__AST_QUANTIFIER_INFINITY) {
            SYM_PUT_STR(&build, "inf");
        } else {
            SYM_PUT_NUM(&build, re__ast_get_quantifier_max(ast));
        }
        SYM_PUT_STR(&build, greed);
    } else if (type == RE__AST_TYPE_GROUP) {
        re__ast_group_flags flags = re__ast_get_group_flags(ast);
        SYM_PUT_SUB(&build, re__ast_group_flags, flags);
        if (!(flags & RE__AST_GROUP_FLAG_NONMATCHING)) {
            SYM_PUT_NUM(&build, (mn_int32)re__ast_get_group_idx(ast));
            if (flags & RE__AST_GROUP_FLAG_NAMED) {
                mn__str_view group_name = re__ast_root_get_group(ast_root, re__ast_get_group_idx(ast));
                SYM_PUT_STRN(&build, mn__str_view_get_data(&group_name), mn__str_view_size(&group_name));
            }
        }
    } else if (type == RE__AST_TYPE_ASSERT) {
        re__ast_assert_type atype = re__ast_get_assert_type(ast);
        SYM_PUT_SUB(&build, re__ast_assert_type, atype);
    }
    if (type == RE__AST_TYPE_CONCAT || type == RE__AST_TYPE_ALT) {
        sym_build children;
        mn_int32 child_ref = ast->first_child_ref;
        SYM_PUT_EXPR(&build, &children);
        while (child_ref != RE__AST_NONE) {
            int err = 0;
            re__ast* child = re__ast_root_get(ast_root, child_ref);
            if ((err = re__ast_root_to_sym_r(&children, ast_root, child))) {
                return err;
            }
            child_ref = child->next_sibling_ref;
        }
    } else if (type == RE__AST_TYPE_QUANTIFIER || type == RE__AST_TYPE_GROUP) {
        mn_int32 child_ref = ast->first_child_ref;
        if (child_ref != RE__AST_NONE) {
            int err = 0;
            re__ast* child = re__ast_root_get(ast_root, child_ref);
            if ((err = re__ast_root_to_sym_r(&build, ast_root, child))) {
                return err;
            }
        }
    }
    return SYM_OK;
}

int re__ast_root_to_sym(sym_build* parent, re__ast_root ast_root) {
    sym_build build;
    int err;
    SYM_PUT_EXPR(parent, &build);
    SYM_PUT_TYPE(&build, "ast");
    if (ast_root.root_ref != RE__AST_NONE) {
        re__ast* ast = re__ast_root_get(&ast_root, ast_root.root_ref);
        if ((err = re__ast_root_to_sym_r(&build, &ast_root, ast))) {
            return err;
        }
    }
    return SYM_OK;
}

int re__ast_root_from_sym_r(sym_walk* parent, re__ast_root* ast_root, mn_int32 parent_ref, mn_int32* out_ast_ref, mn_int32 depth) {
    sym_walk walk;
    const char* type_str;
    mptest_size type_str_size;
    re__ast_type type = 0;
    re__ast ast;
    ast_root->depth_max = MN__MAX(ast_root->depth_max, depth);
    SYM_GET_EXPR(parent, &walk);
    SYM_GET_STR(&walk, &type_str, &type_str_size);
    {
        mn__str_view view_a, view_b;
        mn__str_view_init_n(&view_a, type_str, type_str_size);
        while (type < RE__AST_TYPE_MAX) {
            mn__str_view_init_s(&view_b, ast_sym_types[type]);
            if (mn__str_view_cmp(&view_a, &view_b) == 0) {
                break;
            }
            type++;
        }
        if (type == RE__AST_TYPE_MAX) {
            return SYM_INVALID;
        }
    }
    if (type == RE__AST_TYPE_RUNE) {
        mn_int32 rune;
        SYM_GET_NUM(&walk, &rune);
        re__ast_init_rune(&ast, rune);
    } else if (type == RE__AST_TYPE_STR) {
        mn__str new_str;
        const char* new_str_data;
        mptest_size new_str_size;
        mn_int32 new_str_ref;
        SYM_GET_STR(&walk, &new_str_data, &new_str_size);
        mn__str_init_n(&new_str, new_str_data, new_str_size);
        re__ast_root_add_str(ast_root, new_str, &new_str_ref);
        re__ast_init_str(&ast, new_str_ref);
    } else if (type == RE__AST_TYPE_CHARCLASS) {
        re__charclass cc;
        mn_int32 new_cc_ref;
        SYM_GET_SUB(&walk, re__charclass, &cc);
        re__ast_root_add_charclass(ast_root, cc, &new_cc_ref);
        re__ast_init_charclass(&ast, new_cc_ref);
    } else if (type == RE__AST_TYPE_CONCAT) {
        re__ast_init_concat(&ast);
    } else if (type == RE__AST_TYPE_ALT) {
        re__ast_init_alt(&ast);
    } else if (type == RE__AST_TYPE_QUANTIFIER) {
        mn_int32 min;
        mn_int32 max;
        const char* str;
        mptest_size str_size;
        int greedy = 0;
        mn__str_view view_a, view_b;
        SYM_GET_NUM(&walk, &min);
        if (SYM_PEEK_NUM(&walk)) {
            SYM_GET_NUM(&walk, &max);
        } else if (SYM_PEEK_STR(&walk)) {
            const char* quant_str;
            mptest_size quant_str_size;
            SYM_GET_STR(&walk, &quant_str, &quant_str_size);
            mn__str_view_init_n(&view_a, quant_str, quant_str_size);
            mn__str_view_init_s(&view_b, "inf");
            if (mn__str_view_cmp(&view_a, &view_b) == 0) {
                max = RE__AST_QUANTIFIER_INFINITY;
            } else {
                return SYM_INVALID;
            }
        } else {
            return SYM_INVALID;
        }
        SYM_GET_STR(&walk, &str, &str_size);
        mn__str_view_init_n(&view_a, str, str_size);
        mn__str_view_init_s(&view_b, "greedy");
        if (mn__str_view_cmp(&view_a, &view_b) == 0) {
            greedy = 1;
        } else {
            mn__str_view_init_s(&view_b, "nongreedy");
            if (mn__str_view_cmp(&view_a, &view_b) == 0) {
                greedy = 0;
            } else {
                return SYM_INVALID;
            }
        }
        re__ast_init_quantifier(&ast, min, max);
        re__ast_set_quantifier_greediness(&ast, greedy);
    } else if (type == RE__AST_TYPE_GROUP) {
        re__ast_group_flags group_flags;
        SYM_GET_SUB(&walk, re__ast_group_flags, &group_flags);
        if (group_flags & RE__AST_GROUP_FLAG_NONMATCHING) {
            re__ast_init_group(&ast, 0, group_flags);
        } else {
            mn__str_view str_view;
            mn_int32 group_num;
            SYM_GET_NUM(&walk, &group_num);
            re__ast_init_group(&ast, (mn_uint32)group_num, group_flags);
            if (group_flags & RE__AST_GROUP_FLAG_NAMED) {
                const char* str_data;
                mptest_size str_size;
                SYM_GET_STR(&walk, &str_data, &str_size);
                mn__str_view_init_n(&str_view, str_data, str_size);
            } else {
                mn__str_view_init_null(&str_view);
            }
            re__ast_root_add_group(ast_root, str_view);
        }
    } else if (type == RE__AST_TYPE_ASSERT) {
        re__ast_assert_type assert_type;
        SYM_GET_SUB(&walk, re__ast_assert_type, &assert_type);
        re__ast_init_assert(&ast, assert_type);
    } else if (type == RE__AST_TYPE_ANY_CHAR) {
        re__ast_init_any_char(&ast);
    } else if (type == RE__AST_TYPE_ANY_BYTE) {
        re__ast_init_any_byte(&ast);
    } else {
        MN__ASSERT_UNREACHED();
        return SYM_INVALID;
    }
    {
        re__ast_root_add_child(ast_root, parent_ref, ast, out_ast_ref);
    }
    if (type == RE__AST_TYPE_CONCAT || type == RE__AST_TYPE_ALT) {
        mn_int32 child_ref = RE__AST_NONE;
        sym_walk children;
        SYM_GET_EXPR(&walk, &children);
        while (SYM_MORE(&children)) {
            int err;
            if ((err = re__ast_root_from_sym_r(&children, ast_root, *out_ast_ref, &child_ref, depth + 1))) {
                return err;
            }
        }
    } else if (type == RE__AST_TYPE_GROUP || type == RE__AST_TYPE_QUANTIFIER) {
        int err;
        mn_int32 dummy_ref;
        if ((err = re__ast_root_from_sym_r(&walk, ast_root, *out_ast_ref, &dummy_ref, depth + 1))) {
            return err;
        }
    }
    return SYM_OK;
}

int re__ast_root_from_sym(sym_walk* parent, re__ast_root* ast_root) {
    sym_walk walk;
    int err = 0;
    SYM_GET_EXPR(parent, &walk);
    SYM_CHECK_TYPE(&walk, "ast");
    re__ast_root_init(ast_root);
    while (SYM_MORE(&walk)) {
        mn_int32 dummy_ref;
        if ((err = re__ast_root_from_sym_r(&walk, ast_root, ast_root->root_ref, &dummy_ref, 1))) {
            return err;
        }
    }
    return SYM_OK;
}

TEST(t_ast_init_rune) {
    re__ast ast;
    re_rune r = (re_rune)RAND_PARAM(RE_RUNE_MAX);
    re__ast_init_rune(&ast, r);
    ASSERT_EQ(ast.type, RE__AST_TYPE_RUNE);
    ASSERT_EQ(re__ast_get_rune(&ast), r);
    re__ast_destroy(&ast);
    PASS();
}

TEST(t_ast_init_str) {
    mn_int32 ref_n = (mn_int32)RAND_PARAM(600);
    re__ast ast;
    re__ast_init_str(&ast, ref_n);
    ASSERT_EQ(ast.type, RE__AST_TYPE_STR);
    ASSERT_EQ(ast._data.str_ref, ref_n);
    re__ast_destroy(&ast);
    PASS();
}

TEST(t_ast_init_class) {
    mn_int32 ref_n = (mn_int32)RAND_PARAM(600);
    re__ast ast;
    re__ast_init_charclass(&ast, ref_n);
    ASSERT_EQ(ast.type, RE__AST_TYPE_CHARCLASS);
    ASSERT_EQ(ast._data.charclass_ref, ref_n);
    re__ast_destroy(&ast);
    PASS();
}

TEST(t_ast_init_concat) {
    re__ast ast;
    re__ast_init_concat(&ast);
    ASSERT_EQ(ast.type, RE__AST_TYPE_CONCAT);
    re__ast_destroy(&ast);
    PASS();
}

TEST(t_ast_init_alt) {
    re__ast ast;
    re__ast_init_alt(&ast);
    ASSERT_EQ(ast.type, RE__AST_TYPE_ALT);
    re__ast_destroy(&ast);
    PASS();
}

TEST(t_ast_init_quantifier) {
    re__ast ast;
    mn_int32 min, max;
    min = (mn_int32)RAND_PARAM(1000);
    max = min + (mptest_int32)RAND_PARAM(1000 - (mptest_rand)min);
    re__ast_init_quantifier(&ast, min, max);
    ASSERT_EQ(ast.type, RE__AST_TYPE_QUANTIFIER);
    re__ast_destroy(&ast);
    PASS();
}

TEST(t_ast_init_group_nonmatching) {
    re__ast ast;
    re__ast_init_group(&ast, 0, RE__AST_GROUP_FLAG_NONMATCHING);
    ASSERT_EQ(ast.type, RE__AST_TYPE_GROUP);
    re__ast_destroy(&ast);
    PASS();
}

TEST(t_ast_init_group_matching) {
    re__ast ast;
    re__ast_init_group(&ast, 0, 0);
    ASSERT_EQ(ast.type, RE__AST_TYPE_GROUP);
    re__ast_destroy(&ast);
    PASS();
}

TEST(t_ast_init_group_named) {
    re__ast ast;
    re__ast_init_group(&ast, 0, RE__AST_GROUP_FLAG_NAMED);
    ASSERT_EQ(ast.type, RE__AST_TYPE_GROUP);
    re__ast_destroy(&ast);
    PASS();
}

TEST(t_ast_init_assert) {
    re__ast ast;
    re__ast_assert_type atype = (re__ast_assert_type)RAND_PARAM(RE__AST_ASSERT_TYPE_MAX);
    re__ast_init_assert(&ast, atype);
    ASSERT_EQ(ast.type, RE__AST_TYPE_ASSERT);
    re__ast_destroy(&ast);
    PASS();
}

TEST(t_ast_init_any_char) {
    re__ast ast;
    re__ast_init_any_char(&ast);
    ASSERT_EQ(ast.type, RE__AST_TYPE_ANY_CHAR);
    re__ast_destroy(&ast);
    PASS();
}

TEST(t_ast_init_any_byte) {
    re__ast ast;
    re__ast_init_any_byte(&ast);
    ASSERT_EQ(ast.type, RE__AST_TYPE_ANY_BYTE);
    re__ast_destroy(&ast);
    PASS();
}

SUITE(s_ast_init) {
    FUZZ_TEST(t_ast_init_rune);
    FUZZ_TEST(t_ast_init_str);
    FUZZ_TEST(t_ast_init_class);
    RUN_TEST(t_ast_init_concat);
    RUN_TEST(t_ast_init_alt);
    FUZZ_TEST(t_ast_init_quantifier);
    RUN_TEST(t_ast_init_group_nonmatching);
    RUN_TEST(t_ast_init_group_matching);
    RUN_TEST(t_ast_init_group_named);
    FUZZ_TEST(t_ast_init_assert);
    FUZZ_TEST(t_ast_init_any_char);
    FUZZ_TEST(t_ast_init_any_byte);
}

TEST(t_ast_quantifier_greediness) {
    re__ast ast;
    re__ast_init_quantifier(&ast, 0, 5);
    re__ast_set_quantifier_greediness(&ast, 0);
    ASSERT_EQ(re__ast_get_quantifier_greediness(&ast), 0);
    re__ast_destroy(&ast);
    re__ast_init_quantifier(&ast, 0, 5);
    re__ast_set_quantifier_greediness(&ast, 1);
    ASSERT_EQ(re__ast_get_quantifier_greediness(&ast), 1);
    re__ast_destroy(&ast);
    PASS();
}

TEST(t_ast_quantifier_minmax) {
    re__ast ast;
    mn_int32 min, max;
    min = (mn_int32)RAND_PARAM(1000);
    max = min + (mptest_int32)RAND_PARAM(1000 - (mptest_rand)min);
    re__ast_init_quantifier(&ast, min, max);
    ASSERT_EQ(re__ast_get_quantifier_min(&ast), min);
    ASSERT_EQ(re__ast_get_quantifier_max(&ast), max);
    re__ast_destroy(&ast);
    PASS();
}

TEST(t_ast_get_rune) {
    re__ast ast;
    re_rune r = (re_rune)RAND_PARAM(RE_RUNE_MAX);
    re__ast_init_rune(&ast, r);
    ASSERT_EQ(re__ast_get_rune(&ast), r);
    re__ast_destroy(&ast);
    PASS();
}

/*TEST(t_ast_group_flags) {
    re__ast ast;
    re__ast_group_flags flags = RAND_PARAM(RE__AST_GROUP_FLAG_MAX >> 1);
    re__ast_init_group(&ast, 0);
    re__ast_set_group_flags(&ast, flags);
    ASSERT_EQ(re__ast_get_group_flags(&ast), flags);
    re__ast_destroy(&ast);
    PASS();
}*/

TEST(t_ast_assert_type) {
    re__ast ast;
    re__ast_assert_type atype = RAND_PARAM(RE__AST_ASSERT_TYPE_MAX);
    re__ast_init_assert(&ast, atype);
    ASSERT_EQ(re__ast_get_assert_type(&ast), atype);
    re__ast_destroy(&ast);
    PASS();
}

SUITE(s_ast) {
    RUN_SUITE(s_ast_init);
    RUN_TEST(t_ast_quantifier_greediness);
    FUZZ_TEST(t_ast_quantifier_minmax);
    FUZZ_TEST(t_ast_get_rune);
    FUZZ_TEST(t_ast_assert_type);
}

TEST(t_ast_root_init) {
    re__ast_root ast_root;
    re__ast_root_init(&ast_root);
    ASSERT_EQ(ast_root.root_ref, RE__AST_NONE);
    re__ast_root_destroy(&ast_root);
    PASS();
}

TEST(t_ast_root_addget) {
    re__ast_root ast_root;
    mn_int32 l = (mn_int32)RAND_PARAM(600);
    mn_int32 i;
    mn_int32* refs = MN_MALLOC(sizeof(mn_int32) * (mn_size)l);
    mn_int32 prev_ref = RE__AST_NONE;
    re__ast_root_init(&ast_root);
    for (i = 0; i < l; i++) {
        re__ast ast;
        re__ast_init_rune(&ast, i);
        re__ast_root_add_child(&ast_root, RE__AST_NONE, ast, &prev_ref);
        refs[i] = prev_ref;
    }
    ast_root.depth_max = 1;
    for (i = 0; i < l; i++) {
        mn_int32 ref = refs[i];
        re__ast* ast = re__ast_root_get(&ast_root, ref);
        ASSERT_EQ(re__ast_get_rune(ast), i);
    }
    ASSERT(re__ast_root_verify(&ast_root));
    re__ast_root_destroy(&ast_root);
    MN_FREE(refs);
    PASS();
}

TEST(t_ast_root_remove) {
    re__ast_root ast_root;
    mn_int32 l = (mn_int32)RAND_PARAM(600);
    mn_int32 i;
    mn_int32* refs = MN_MALLOC(sizeof(mn_int32) * (mn_size)l);
    mn_int32 prev_ref = RE__AST_NONE;
    re__ast_root_init(&ast_root);
    for (i = 0; i < l; i++) {
        re__ast ast;
        re__ast_init_rune(&ast, i);
        re__ast_root_add_child(&ast_root, RE__AST_NONE, ast, &prev_ref);
        refs[i] = prev_ref;
    }
    for (i = l; i > 0; i--) {
        re__ast_root_remove(&ast_root, refs[i-1]);
    }
    ast_root.depth_max = 0;
    ASSERT(re__ast_root_verify(&ast_root));
    re__ast_root_destroy(&ast_root);
    MN_FREE(refs);
    PASS();
}

TEST(t_ast_root_thrash) {
    re__ast_root ast_root;
    mn_int32 l = (mn_int32)RAND_PARAM(600);
    mn_int32 i;
    mn_int32* refs = MN_MALLOC(sizeof(mn_int32) * (mn_size)l);
    mn_int32 prev_ref = RE__AST_NONE;
    re__ast_root_init(&ast_root);
    for (i = 0; i < l; i++) {
        re__ast ast;
        re__ast_init_rune(&ast, i);
        re__ast_root_add_child(&ast_root, RE__AST_NONE, ast, &prev_ref);
        refs[i] = prev_ref;
    }
    /* shuffle refs */
    for (i = 0; i < l*3; i++) {
        mn_int32 from = (mn_int32)RAND_PARAM((mptest_rand)l);
        mn_int32 to = (mn_int32)RAND_PARAM((mptest_rand)l);
        mn_int32 temp = refs[from];
        refs[from] = refs[to];
        refs[to] =  temp;
    }
    for (i = 0; i < l; i++) {
        re__ast_root_remove(&ast_root, refs[i]);
    }
    ASSERT(re__ast_root_verify(&ast_root));
    re__ast_root_destroy(&ast_root);
    MN_FREE(refs);
    PASS();
}

/* TODO: test root_link_siblings, root_set_child, root_wrap, root_size */

SUITE(s_ast_root) {
    RUN_TEST(t_ast_root_init);
    FUZZ_TEST(t_ast_root_addget);
    FUZZ_TEST(t_ast_root_remove);
    FUZZ_TEST(t_ast_root_thrash);
}

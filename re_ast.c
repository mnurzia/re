#include "re_internal.h"

RE_INTERNAL void re__ast_init(re__ast* ast, re__ast_type type) {
    ast->type = type;
    ast->children = 0;
    ast->prev = 0;
    ast->next = 0;
    ast->prev = 0;
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

RE_INTERNAL void re__ast_set_children_count(re__ast* ast, re_size size) {
    ast->children = size;
}

RE_INTERNAL re_size re__ast_get_children_count(re__ast* ast) {
    return ast->children;
}

RE_INTERNAL re__ast_assert_type re__ast_get_assert_type(re__ast* ast) {
    RE_ASSERT(ast->type == RE__AST_TYPE_ASSERT);
    return ast->_data.assert_type;
}

#if RE_DEBUG

/* Recursive (be careful!!!) */
RE_INTERNAL re_size re__ast_debug_dump(const re__ast* ast, re_size lvl) {
    re_size i;
    const re__ast* new_ast;
    for (i = 0; i < (re_size)lvl; i++) {
        printf("  ");
    }
    printf("-%u %u | ", (unsigned int)ast->prev, (unsigned int)ast->next);
    switch (ast->type) {
        case RE__AST_TYPE_NONE:
            printf("NONE");
            break;
        case RE__AST_TYPE_RUNE:
            printf("CHAR: ord=%X ('%c')", ast->_data.rune, ast->_data.rune);
            break;
        case RE__AST_TYPE_CLASS:
            printf("CLASS:\n");
            re__charclass_dump(&ast->_data.charclass, lvl+1);
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
    new_ast = ast;
    new_ast += 1;
    for (i = 0; i < ast->children; i++) {
        new_ast += re__ast_debug_dump(new_ast, lvl+1);
    }
    return (re_size)(new_ast - ast);
}

#endif

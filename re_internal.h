#ifndef RE_INTERNAL_H
#define RE_INTERNAL_H

#include "re_api.h"
#include "pack/re_common.h"

/* POD type */
/* Holds a byte range [min, max] */
typedef struct re__byte_range {
    re_uint8 min;
    re_uint8 max;
} re__byte_range;

int re__byte_range_equals(re__byte_range range, re__byte_range other);
int re__byte_range_adjacent(re__byte_range range, re__byte_range other);
re__byte_range re__byte_range_merge(re__byte_range range, re__byte_range other);

/* POD type */
/* Stores characters in the range [min, max] === [min, max+1) */
typedef struct re__rune_range {
    re_rune min;
    re_rune max;
} re__rune_range;

RE_VEC_DECL(re__rune_range);

int re__rune_range_equals(re__rune_range range, re__rune_range other);
int re__rune_range_intersects(re__rune_range range, re__rune_range clip);
re__rune_range re__rune_range_clamp(re__rune_range range, re__rune_range bounds);

/* Character class. */
typedef struct re__charclass {
    /* Non-overlapping set of ranges. */
    re__rune_range_vec ranges;
} re__charclass;

/* Index of ASCII char class types. */
typedef enum re__charclass_ascii_type {
    RE__CHARCLASS_ASCII_TYPE_ALNUM,
    RE__CHARCLASS_ASCII_TYPE_ALPHA,
    RE__CHARCLASS_ASCII_TYPE_ASCII,
    RE__CHARCLASS_ASCII_TYPE_BLANK,
    RE__CHARCLASS_ASCII_TYPE_CNTRL,
    RE__CHARCLASS_ASCII_TYPE_DIGIT,
    RE__CHARCLASS_ASCII_TYPE_GRAPH,
    RE__CHARCLASS_ASCII_TYPE_LOWER,
    RE__CHARCLASS_ASCII_TYPE_PERL_SPACE,
    RE__CHARCLASS_ASCII_TYPE_PRINT,
    RE__CHARCLASS_ASCII_TYPE_PUNCT,
    RE__CHARCLASS_ASCII_TYPE_SPACE,
    RE__CHARCLASS_ASCII_TYPE_UPPER,
    RE__CHARCLASS_ASCII_TYPE_WORD,
    RE__CHARCLASS_ASCII_TYPE_XDIGIT,
    RE__CHARCLASS_ASCII_TYPE_MAX
} re__charclass_ascii_type;

RE_INTERNAL void re__charclass_init(re__charclass* charclass);
RE_INTERNAL re_error re__charclass_init_from_class(re__charclass* charclass, re__charclass_ascii_type type, int inverted);
RE_INTERNAL re_error re__charclass_init_from_string(re__charclass* charclass, re__str* name, int inverted);
RE_INTERNAL void re__charclass_destroy(re__charclass* charclass);
RE_INTERNAL re_error re__charclass_push(re__charclass* charclass, re__rune_range range);
RE_INTERNAL const re__rune_range* re__charclass_get_ranges(const re__charclass* charclass);
RE_INTERNAL re_size re__charclass_get_num_ranges(const re__charclass* charclass);
RE_INTERNAL int re__charclass_equals(const re__charclass* charclass, const re__charclass* other);

#if RE_DEBUG

RE_INTERNAL void re__charclass_dump(const re__charclass* charclass, re_size lvl);
RE_INTERNAL int re__charclass_verify(const re__charclass* charclass);

#endif

/* Immediate-mode charclass builder. */
typedef struct re__charclass_builder {
    re__rune_range_vec ranges;
    int should_invert;
    re_rune highest;
} re__charclass_builder;

RE_INTERNAL void re__charclass_builder_init(re__charclass_builder* builder);
RE_INTERNAL void re__charclass_builder_destroy(re__charclass_builder* builder);
RE_INTERNAL void re__charclass_builder_begin(re__charclass_builder* builder);
RE_INTERNAL void re__charclass_builder_invert(re__charclass_builder* builder);
RE_INTERNAL re_error re__charclass_builder_insert_range(re__charclass_builder* builder, re__rune_range range);
RE_INTERNAL re_error re__charclass_builder_insert_class(re__charclass_builder* builder, re__charclass* charclass);
RE_INTERNAL re_error re__charclass_builder_finish(re__charclass_builder* builder, re__charclass* charclass);

typedef struct re__ast re__ast; 

/* Enumeration of AST node types. */
typedef enum re__ast_type {
    RE__AST_TYPE_MIN = 0,
    /* No type. Should never occur. */
    RE__AST_TYPE_NONE = 0,
    /* A single character. */
    RE__AST_TYPE_RUNE,
    /* A string of characters. */
    RE__AST_TYPE_STRING,
    /* A character class. */
    RE__AST_TYPE_CHARCLASS,
    /* A concatenation of multiple nodes. */
    RE__AST_TYPE_CONCAT,
    /* An alteration of multiple nodes. */
    RE__AST_TYPE_ALT,
    /* A quantifier (*,+,?) or a range {m,n} */
    RE__AST_TYPE_QUANTIFIER,
    /* Capturing/non-capturing group. */
    RE__AST_TYPE_GROUP,
    /* Text/word asserts */
    RE__AST_TYPE_ASSERT,
    /* Any character (.) */
    RE__AST_TYPE_ANY_CHAR,
    /* Any byte (\C) */
    RE__AST_TYPE_ANY_BYTE,
    RE__AST_TYPE_MAX
} re__ast_type;

RE_VEC_DECL(re__ast);
RE_VEC_DECL_FUNC(re__ast, init);
RE_VEC_DECL_FUNC(re__ast, destroy);
RE_VEC_DECL_FUNC(re__ast, push);
RE_VEC_DECL_FUNC(re__ast, getref);
RE_VEC_DECL_FUNC(re__ast, size);

#define RE__AST_QUANTIFIER_MAX 2000
#define RE__AST_QUANTIFIER_INFINITY RE__AST_QUANTIFIER_MAX+2

/* Quantifier info. */
/* Range: [min, max) */
typedef struct re__ast_quantifier_info {
    /* Minimum amount. */
    re_int32 min;
    /* Maximum amount. -1 for infinity. */
    re_int32 max;
    /* Whether or not to prefer fewer matches. */
    int greediness;
} re__ast_quantifier_info;

/* Assert types, as they are represented in the AST. */
typedef enum re__ast_assert_type {
    RE__AST_ASSERT_TYPE_MIN = 1,
    RE__AST_ASSERT_TYPE_TEXT_START = 1,
    RE__AST_ASSERT_TYPE_TEXT_END = 2,
    RE__AST_ASSERT_TYPE_TEXT_START_ABSOLUTE = 4,
    RE__AST_ASSERT_TYPE_TEXT_END_ABSOLUTE = 8,
    RE__AST_ASSERT_TYPE_WORD = 16,
    RE__AST_ASSERT_TYPE_WORD_NOT = 32,
    RE__AST_ASSERT_TYPE_MAX = 64
} re__ast_assert_type;

/* Group flags */
typedef enum re__ast_group_flags {
    RE__AST_GROUP_FLAG_CASE_INSENSITIVE = 1,
    RE__AST_GROUP_FLAG_MULTILINE = 2,
    RE__AST_GROUP_FLAG_DOT_NEWLINE = 4,
    RE__AST_GROUP_FLAG_UNGREEDY = 8,
    RE__AST_GROUP_FLAG_NONMATCHING = 16,
    RE__AST_GROUP_FLAG_MAX = 32
} re__ast_group_flags;

/* Group info */
typedef struct re__ast_group_info {
    re__ast_group_flags flags;
    re_uint32 match_number;
} re__ast_group_info;

/* Holds AST node data depending on the node type. */
typedef union re__ast_data {
    /* RE__AST_TYPE_RUNE: holds a single character */
    re_rune rune;
    /* RE__AST_TYPE_STRING: holds a reference to a string. */
    re_int32 string_ref;
    /* RE__AST_TYPE_CLASS: holds a reference to a character class. */
    re_int32 charclass_ref;
    /* RE__AST_TYPE_GROUP: holds the group's index and flags */
    re__ast_group_info group_info;
    /* RE__AST_TYPE_QUANTIFIER: minimum/maximum/greediness */
    re__ast_quantifier_info quantifier_info;
    /* RE__AST_TYPE_ASSERT: type(s) of assert present */
    re__ast_assert_type assert_type;
} re__ast_data;

#define RE__AST_NONE -1

struct re__ast {
    re__ast_type type;
    re_int32 first_child_ref;
    re_int32 prev_sibling_ref;
    re_int32 next_sibling_ref;
    re__ast_data _data;
};

RE_INTERNAL void re__ast_init_rune(re__ast* ast, re_rune rune);
RE_INTERNAL void re__ast_init_string(re__ast* ast, re_int32 string_ref);
RE_INTERNAL void re__ast_init_charclass(re__ast* ast, re_int32 charclass_ref);
RE_INTERNAL void re__ast_init_concat(re__ast* ast);
RE_INTERNAL void re__ast_init_alt(re__ast* ast);
RE_INTERNAL void re__ast_init_quantifier(re__ast* ast, re_int32 min, re_int32 max);
RE_INTERNAL void re__ast_init_group(re__ast* ast);
RE_INTERNAL void re__ast_init_assert(re__ast* ast, re__ast_assert_type assert_type);
RE_INTERNAL void re__ast_init_any_char(re__ast* ast);
RE_INTERNAL void re__ast_init_any_byte(re__ast* ast);
RE_INTERNAL void re__ast_destroy(re__ast* ast);
RE_INTERNAL int re__ast_get_quantifier_greediness(re__ast* ast);
RE_INTERNAL void re__ast_set_quantifier_greediness(re__ast* ast, int is_greedy);
RE_INTERNAL re_int32 re__ast_get_quantifier_min(re__ast* ast);
RE_INTERNAL re_int32 re__ast_get_quantifier_max(re__ast* ast);
RE_INTERNAL re_rune re__ast_get_rune(re__ast* ast);
RE_INTERNAL re__ast_group_flags re__ast_get_group_flags(re__ast* ast);
RE_INTERNAL void re__ast_set_group_flags(re__ast* ast, re__ast_group_flags flags);
RE_INTERNAL re__ast_assert_type re__ast_get_assert_type(re__ast* ast);

RE_REFS_DECL(re__charclass);
RE_REFS_DECL(re__str);

typedef struct re__ast_root {
    re__ast_vec ast_vec;
    re_int32 last_empty_ref;
    re_int32 root_ref;
    re__charclass_refs charclasses;
    re__str_refs strings;
} re__ast_root;

RE_INTERNAL void re__ast_root_init(re__ast_root* ast_root);
RE_INTERNAL void re__ast_root_destroy(re__ast_root* ast_root);
RE_INTERNAL re__ast* re__ast_root_get(re__ast_root* ast_root, re_int32 ast_ref);
RE_INTERNAL void re__ast_root_remove(re__ast_root* ast_root, re_int32 ast_ref);
RE_INTERNAL re_error re__ast_root_add_child(re__ast_root* ast_root, re_int32 parent_ref, re__ast ast, re_int32* out_ref);
RE_INTERNAL re_error re__ast_root_add_sibling(re__ast_root* ast_root, re_int32 prev_sibling_ref, re__ast ast, re_int32* out_ref);
RE_INTERNAL re_error re__ast_root_add_wrap(re__ast_root* ast_root, re_int32 parent_ref, re_int32 inner_ref, re__ast ast_outer, re_int32* out_ref);

RE_INTERNAL re_error re__ast_root_add_charclass(re__ast_root* ast_root, re__charclass charclass, re_int32* out_charclass_ref);
RE_INTERNAL const re__charclass* re__ast_root_get_charclass(const re__ast_root* ast_root, re_int32 charclass_ref); 

RE_INTERNAL re_error re__ast_root_add_str(re__ast_root* ast_root, re__str str, re_int32* out_ref);
RE_INTERNAL re__str_view re__ast_root_get_str(const re__ast_root* ast_root, re_int32 str_ref);

#if RE_DEBUG

RE_INTERNAL void re__ast_root_debug_dump(re__ast_root* ast_root, re_int32 root_ref, re_int32 lvl);
RE_INTERNAL int re__ast_root_verify(re__ast_root* ast_root);

#endif

typedef enum re__parse_state {
    RE__PARSE_STATE_GND,
    RE__PARSE_STATE_MAYBE_QUESTION,
    RE__PARSE_STATE_ESCAPE,
    RE__PARSE_STATE_PARENS_INITIAL,
    RE__PARSE_STATE_PARENS_FLAG_INITIAL,
    RE__PARSE_STATE_PARENS_FLAG_NEGATE,
    RE__PARSE_STATE_PARENS_AFTER_COLON,
    RE__PARSE_STATE_PARENS_AFTER_P,
    RE__PARSE_STATE_PARENS_NAME_INITIAL,
    RE__PARSE_STATE_PARENS_NAME,
    RE__PARSE_STATE_OCTAL_SECOND_DIGIT,
    RE__PARSE_STATE_OCTAL_THIRD_DIGIT,
    RE__PARSE_STATE_HEX_INITIAL,
    RE__PARSE_STATE_HEX_SECOND_DIGIT,
    RE__PARSE_STATE_HEX_BRACKETED,
    RE__PARSE_STATE_QUOTE,
    RE__PARSE_STATE_QUOTE_ESCAPE,
    RE__PARSE_STATE_COUNTING_FIRST_NUM_INITIAL,
    RE__PARSE_STATE_COUNTING_FIRST_NUM,
    RE__PARSE_STATE_COUNTING_SECOND_NUM_INITIAL,
    RE__PARSE_STATE_COUNTING_SECOND_NUM,
    RE__PARSE_STATE_CHARCLASS_INITIAL,
    RE__PARSE_STATE_CHARCLASS_AFTER_CARET,
    RE__PARSE_STATE_CHARCLASS_AFTER_BRACKET,
    RE__PARSE_STATE_CHARCLASS_LO,
    RE__PARSE_STATE_CHARCLASS_AFTER_LO,
    RE__PARSE_STATE_CHARCLASS_HI,
    RE__PARSE_STATE_CHARCLASS_NAMED
} re__parse_state;

typedef struct re__parse_frame {
    re_int32 ast_frame_root_ref;
    re_int32 ast_prev_child_ref;
    re__parse_state ret_state;
    re__ast_group_flags group_flags;
    re_int32 depth;
    re_int32 depth_max;
} re__parse_frame;

RE_VEC_DECL(re__parse_frame);

typedef struct re__parse {
    re* re;
    re__parse_frame_vec frames;
    re__ast_root ast_root;
    re_int32 ast_frame_root_ref;
    re_int32 ast_prev_child_ref;
    re__parse_state state;
    re_int32 radix_num;
    int radix_digits;
    re__charclass_builder charclass_builder;
    int defer;
    re__ast_group_flags group_flags_new;
    re__ast_group_flags group_flags;
    const re_char* str_begin;
    const re_char* str_end;
    re_int32 counting_first_num;
    re_rune charclass_lo_rune;
    re_int32 depth;
    re_int32 depth_max;
    re_int32 depth_max_prev;
} re__parse;

RE_INTERNAL void re__parse_init(re__parse* parse, re* re);
RE_INTERNAL void re__parse_destroy(re__parse* parse);
RE_INTERNAL re_error re__parse_str(re__parse* parse, const re__str_view* regex);

typedef re_uint32 re__prog_loc;

/* Invalid program location (used for debugging) */
#define RE__PROG_LOC_INVALID 0

/* Each instruction has a type, data associated with the specific type, and
 * either one or two pointers to the next instruction(s) to execute. */

/* An instruction is shown like this: */
/* +-------+
 * |  Op   |
 * | Instr |
 * +-------+ */
/* Instructions are shown as assembled into the program like this: */
/* 0       1       2       3
 * +-------+-------+-------+~~~
 * |  Op   |  Op   |  Op   | ..
 * | Instr | Instr | Instr | ..
 * +-------+-------+-------+~~~ */
/* Instructions all have a "primary branch", that is, a pointer to the next
 * instruction that will execute. */
/* +0      +1      +2
 * +-------+-------+-------+~~~
 * |  Op   |  Op   |  Op   | ..
 * | Instr | Instr | Instr | ..
 * +---+---+---+---+---+---+~~~
 *     |     ^ |     ^ |
 *     |     | |     | |
 *     +-----+ +-----+ +--------> ...
 *        ^       ^       ^
 *        |       |       |
 *        +-------+-------+
 *                |
 *             Primary
 *             Branches             */
/* Some instructions have no branches, and some have two. */
/* +0      +1      +2      +3      +4
 * +-------+-------+-------+-------+-------+~~~
 * | Byte  | Byte  | Split | Match | Byte  | ..
 * | Instr | Instr | Instr | Instr | Instr | ..
 * +---+---+---+---+---+-+-+-------+-------+~~~
 *     |     ^ |     ^ | |   ^       ^ |
 *     |     | |     | | |   |       | |
 *     |     | |     | | +-----------+ +--------> ...
 *     |     | |     | |     |   ^
 *     +-----+ +-----+ +-----+   |
 *        ^       ^       ^      |
 *        |       |       |      |
 *        +-------+-------+      |
 *                |              |
 *             Primary       Secondary
 *             Branches       Branch              */

/* Enumeration of instruction types (opcodes) */
typedef enum re__prog_inst_type {
    /* Match a single byte */
    /* +-------+
     * | Byte  |
     * | Instr |
     * +---+---+
     *     |
     *     +-----> */
    RE__PROG_INST_TYPE_BYTE,
    /* Match a range of bytes */
    /* +-------+
     * | ByteR |
     * | Instr |
     * +---+---+
     *     |
     *     +-----> */
    RE__PROG_INST_TYPE_BYTE_RANGE,
    /* Fork execution to two different locations */
    /* +-------+
     * | Split |
     * | Instr |
     * +---+-+-+
     *     | |
     *     +----->  <-- This branch is taken first (primary)
     *       |
     *       +--->  <-- This branch is taken second (secondary) */
    RE__PROG_INST_TYPE_SPLIT,
    /* Finish execution in a match state */
    /* +-------+
     * | Match |
     * | Instr |
     * +-------+ */
    RE__PROG_INST_TYPE_MATCH,
    /* Fail execution */
    /* +-------+
     * | Fail  |
     * | Instr |
     * +-------+ */
    RE__PROG_INST_TYPE_FAIL,
    /* Save current program counter (found a match group) */
    /* +-------+
     * | Save  |
     * | Instr |
     * +---+---+ 
     *     |
     *     +-----> */
    RE__PROG_INST_TYPE_SAVE,
    /* Assert that some context is occurring (begin/end of text/word, etc.) */
    /* +-------+
     * |  Ass  | <- funny
     * | Instr |
     * +---+---+
     *     |
     *     +-----> */
    RE__PROG_INST_TYPE_ASSERT
} re__prog_inst_type;

/* Opcode-specific data */
typedef union re__prog_inst_data {
    /* RE__PROG_INST_TYPE_BYTE: a single byte */
    re_uint8 _byte;
    /* RE__PROG_INST_TYPE_BYTE_RANGE: a range of bytes */
    re__byte_range _range;
    /* RE__PROG_INST_TYPE_SPLIT: secondary branch target */
    re__prog_loc _secondary;
    /* RE__PROG_INST_TYPE_MATCH: match index */
    re_uint32 _match_idx;
    /* RE__PROG_INST_TYPE_ASSERT: assert context set */
    re_uint32 _assert_context;
    /* RE__PROG_INST_TYPE_SAVE: save index */
    re_uint32 _save_idx;
} re__prog_inst_data;

/* Program instruction structure */
typedef struct re__prog_inst {
    /* Opcode */
    re__prog_inst_type _inst_type;
    /* Primary branch target */
    re__prog_loc _primary;
    /* Data about instruction */
    re__prog_inst_data _inst_data;
} re__prog_inst;


RE_VEC_DECL(re__prog_inst);

RE_INTERNAL void re__prog_inst_init_byte(re__prog_inst* inst, re_uint8 byte);
RE_INTERNAL void re__prog_inst_init_byte_range(re__prog_inst* inst, re_uint8 min, re_uint8 max);
RE_INTERNAL void re__prog_inst_init_split(re__prog_inst* inst, re__prog_loc primary, re__prog_loc secondary);
RE_INTERNAL void re__prog_inst_init_match(re__prog_inst* inst, re_uint32 match_idx);
RE_INTERNAL void re__prog_inst_init_fail(re__prog_inst* inst);
RE_INTERNAL void re__prog_inst_init_assert(re__prog_inst* inst, re_uint32 assert_context);
RE_INTERNAL void re__prog_inst_init_save(re__prog_inst* inst, re_uint32 save_idx);
RE_INTERNAL re__prog_loc re__prog_inst_get_primary(re__prog_inst* inst);
RE_INTERNAL void re__prog_inst_set_primary(re__prog_inst* inst, re__prog_loc loc);
RE_INTERNAL re_uint8 re__prog_inst_get_byte(re__prog_inst* inst);
RE_INTERNAL re_uint8 re__prog_inst_get_byte_min(re__prog_inst* inst);
RE_INTERNAL re_uint8 re__prog_inst_get_byte_max(re__prog_inst* inst);
RE_INTERNAL re__prog_loc re__prog_inst_get_split_secondary(re__prog_inst* inst);
RE_INTERNAL void re__prog_inst_set_split_secondary(re__prog_inst* inst, re__prog_loc loc);
RE_INTERNAL re__prog_inst_type re__prog_inst_get_type(re__prog_inst* inst);
RE_INTERNAL re__ast_assert_type re__prog_inst_get_assert_ctx(re__prog_inst* inst);

#define RE__ERROR_PROGMAX (RE_ERROR_COMPILE | (1 << 8))
#define RE__PROG_SIZE_MAX 100000

/* The program itself */
typedef struct re__prog {
    re__prog_inst_vec _instructions;
} re__prog;

RE_INTERNAL re_error re__prog_init(re__prog* prog);
RE_INTERNAL void re__prog_destroy(re__prog* prog);
RE_INTERNAL re__prog_loc re__prog_size(re__prog* prog);
RE_INTERNAL re__prog_inst* re__prog_get(re__prog* prog, re__prog_loc loc);
RE_INTERNAL re_error re__prog_add(re__prog* prog, re__prog_inst inst);

/* A list of program patches -- locations in the program that need to point to
 * later instructions */
/* Like RE2, we store the location of the next patch in each instruction, so it
 * turns out to be a linked-list of sorts. */
/* This representation is useful because we only really need prepend, append,
 * and iterate facilities. So in addition to not having to perform manual memory
 * allocation, we get quick operations "for free". */
typedef struct re__compile_patches {
    re__prog_loc first_inst;
    re__prog_loc last_inst;
} re__compile_patches;

RE_INTERNAL void re__compile_patches_init(re__compile_patches* patches);
RE_INTERNAL void re__compile_patches_append(re__compile_patches* patches, re__prog* prog, re__prog_loc to, int secondary);

#if RE_DEBUG

RE_INTERNAL void re__compile_patches_dump(re__compile_patches* patches, re__prog* prog);

#endif

/* Tree node, used for representing a character class. */
/* The tree representation is convenient because it allows a completely
 * optimized UTF-8 DFA with a somewhat simple construction. */
/* A tree node looks like this:
 *
 * +-------+
 * | AA-BB |----- children...
 * +-------+
 *     |
 *     |
 * siblings...
 *
 * where AA is the minimum byte for a range and BB is the maximum. */
/* Each node is a binary tree node, where the bottom link corresponds to the 
 * first sibling and the right link corresponds to the first child. */
/* Nodes reach each other via indexes in a contiguous chunk of memory. In the
 * code, these are suffixed with "_ref". So, a variable named "child_ref" is
 * just an int that corresponds to another tree at that position within the
 * character class compiler's tree vector. */
/* If a node isn't connected to anything, the reference is set to
 * RE__CHARCLASS_COMPILE_TREE_NONE (think of it as a NULL pointer), and we
 * consider it to be unconnected. */
/* Here is a diagram of some connected tree nodes:
 *
 *                           +-------+
 * root->prev_sibling_ref -> | 66-66 |
 *                           +-------+
 *                               |
 *                               |
 *                           +-------+     +-------+
 *                   root -> | 68-68 |-----| 35-3F | <- root->first_child_ref
 *                           +-------+     +-------+
 *                               |             |
 *                               |             |
 *                           +-------+     +-------+
 * root->next_sibling_ref -> | 70-70 |     | 66-67 |
 *                           +-------+     +-------+
 *                                             |
 *                                             |
 *                                         +-------+
 *                                         | 70-7F | <- root->last_child_ref
 *                                         +-------+ */
/* Remember, each line in the graph is actually an int32 describing the position
 * of the node inside the charclass compiler. */
/* Since it's hard to draw these graphs as they get more complex, from now I'll
 * just show them like this:
 *
 * ---[66-66]
 *       |
 *    [68-68]---[35-3F]
 *       |         |
 *       |      [66-67]
 *       |         |
 *       |      [70-7F]
 *       |
 *    [70-70]
 * 
 * This is equivalent to the above graph. */
#define RE__COMPILE_CHARCLASS_TREE_NONE -1
typedef struct re__compile_charclass_tree {
    /* Range of bytes to match */
    re__byte_range byte_range;
    /* Reference to next sibling */
    re_int32 next_sibling_ref;
    /* Reference to previous sibling (used for reverse-iterating) */
    re_int32 prev_sibling_ref;
    /* Reference to first child */
    re_int32 first_child_ref;
    /* Reference to last child (used for reverse-iterating) */
    re_int32 last_child_ref;
    /* Hash of this tree, used for caching */
    re_uint32 hash;
} re__compile_charclass_tree;

#define RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE -1

/* Entry used to index a tree in the char class compiler's hash table. */
/* The hash table uses a sparse/dense model, where there is a fixed-size sparse
 * array of integers that correspond to indices in the dense vector. */
/* We can quickly test if a tree is in the cache by moduloing its hash by the
 * size of the sparse array, checking if the corresponding index is less than
 * the size of the dense vector, and finally by checking if the corresponding
 * element in the dense vector points *back* to the sparse array. This is very
 * similar to a sparse set. */
/* If more than one element hashes to the same position in the dense vector, we
 * link them together using the 'next' member, forming a mini-linked list. */
typedef struct re__compile_charclass_hash_entry {
    /* Index in sparse array */
    re_int32 sparse_index;
    /* Reference to tree root */
    re_int32 root_ref;
    /* Compiled instruction location in the program */
    re__prog_loc prog_loc;
    /* Next hash_entry that hashes to the same value, 
     * RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE otherwise. */
    re_int32 next;
} re__compile_charclass_hash_entry;

RE_INTERNAL void re__compile_charclass_hash_entry_init(re__compile_charclass_hash_entry* hash_entry, re_int32 sparse_index, re_int32 tree_ref, re__prog_loc prog_loc);

RE_VEC_DECL(re__compile_charclass_hash_entry);
RE_VEC_DECL(re__compile_charclass_tree);

#define RE__COMPILE_CHARCLASS_CACHE_SPARSE_SIZE 1024

/* Character class compiler. */
typedef struct re__compile_charclass {
    /* Vector of tree nodes. Each '_ref' entry in re__compile_charclass_tree
     * represents an index in this vector. */
    re__compile_charclass_tree_vec tree;
    /* Sparse tree cache. Each element in this array points to a corresponding
     * position in 'cache_dense'. Lookup is performed by moduloing a tree's hash
     * with the sparse cache size. Since cache hits are relatively rare, this
     * allows pretty inexpensive lookup. */
    re_int32* cache_sparse;
    /* Dense tree cache. Entries with duplicate hashes are linked using their
     * 'next' member. */
    re__compile_charclass_hash_entry_vec cache_dense;
} re__compile_charclass;

void re__compile_charclass_init(re__compile_charclass* char_comp);
void re__compile_charclass_destroy(re__compile_charclass* char_comp);
re_error re__compile_charclass_gen(re__compile_charclass* char_comp, const re__charclass* charclass, re__prog* prog, re__compile_patches* patches_out);

#if RE_DEBUG
void re__compile_charclass_dump(re__compile_charclass* char_comp, re_int32 tree_idx, re_int32 indent);
#endif

typedef struct re__compile_frame {
    re_int32 ast_root_ref;
    re_int32 ast_child_ref;
    re__compile_patches patches;
    re__prog_loc start;
    re__prog_loc end;
    re_int32 rep_idx;
} re__compile_frame;

typedef struct re__compile {
    re* re;
    re__compile_frame* frames;
    re_int32 frames_size;
    re_int32 frame_ptr;
    re_int32 ast_ref;
    re__compile_charclass char_comp;
} re__compile;

RE_INTERNAL void re__compile_init(re__compile* compile, re* re);
RE_INTERNAL void re__compile_destroy(re__compile* compile);
RE_INTERNAL re_error re__compile_regex(re__compile* compile);

typedef re__prog_loc re__exec_thrdmin;

typedef struct re__exec_thrdmin_set {
    re__exec_thrdmin* dense;
    re__exec_thrdmin* sparse;
    re__prog_loc n;
    re__prog_loc size;
} re__exec_thrdmin_set;

RE_VEC_DECL(re__exec_thrdmin);

/* Execution context. */
typedef struct re__exec {
    re* re;
    re__exec_thrdmin_set set_a;
    re__exec_thrdmin_set set_b;
    re__exec_thrdmin_set set_c;
    re__exec_thrdmin_vec thrd_stk;
} re__exec;

/* Internal data structure */
struct re_data {
    re__parse parse;
    re__prog program;
    re__compile compile;
    re__exec exec;
    /* Note: error_string_view always points to either a static const char* that
     * is a compiletime constant or a dynamically-allocated const char* inside
     * of error_string. Either way, in OOM situations, we will not allocate more
     * memory to store an error string and default to a constant. */  
    re__str error_string;
    re__str_view error_string_view;
};

RE_INTERNAL void re__exec_init(re__exec* exec, re* re);
RE_INTERNAL void re__exec_destroy(re__exec* exec);

RE_INTERNAL re_error re__exec_nfa(re__exec* exec, const re_char* str, re_size n);

RE_INTERNAL void re__set_error_str(re* re, const re__str* error_str);
RE_INTERNAL void re__set_error_generic(re* re, re_error err);

/*RE_INTERNAL re_error re__compile(re* re);*/
RE_INTERNAL void re__prog_debug_dump(re__prog* prog);

#endif

#ifndef RE_INTERNAL_H
#define RE_INTERNAL_H

#include "re_api.h"
#include "_cpack/internal.h"

#define RE__ERROR_COMPRESSION_FORMAT (RE_ERROR_INTERNAL - 1)
#define RE__ERROR_PROGMAX (RE_ERROR_INTERNAL - 2)

/* ---------------------------------------------------------------------------
 * Byte ranges (re_range.c)
 * ------------------------------------------------------------------------ */
/* POD type */
/* Holds a byte range [min, max] */
typedef struct re__byte_range {
    mn_uint8 min;
    mn_uint8 max;
} re__byte_range;

/* Check if two byte ranges equal each other */
int re__byte_range_equals(re__byte_range range, re__byte_range other);

/* Check if two byte ranges are adjacent (other comes directly after range) */
int re__byte_range_adjacent(re__byte_range range, re__byte_range other);

/* Check if two byte ranges intersect */
int re__byte_range_intersects(re__byte_range range, re__byte_range clip);

/* Compute the intersection of two byte ranges (requires predicate
 * re__byte_range_intersects(range, other) == 1) */
re__byte_range re__byte_range_intersection(re__byte_range range, re__byte_range clip);

/* Compute the union of two adjacent byte ranges (requires predicate
 * re__byte_range_adjacent(range, other) == 1) */
re__byte_range re__byte_range_merge(re__byte_range range, re__byte_range other);


#if MN_DEBUG

void re__byte_debug_dump(mn_uint8 byte);
void re__byte_range_debug_dump(re__byte_range br);

#endif

/* ---------------------------------------------------------------------------
 * Rune ranges (re_range.c)
 * ------------------------------------------------------------------------ */
/* POD type */
/* Stores characters in the range [min, max] == [min, max+1) */
typedef struct re__rune_range {
    re_rune min;
    re_rune max;
} re__rune_range;

MN__VEC_DECL(re__rune_range);

/* Check if two rune ranges equal each other */
int re__rune_range_equals(re__rune_range range, re__rune_range other);

/* Check if two rune ranges intersect */
int re__rune_range_intersects(re__rune_range range, re__rune_range clip);

/* Clamp the given range to be within the given bounds (requires predicate
 * re__rune_range_intersects(range, clip) == 1) */
re__rune_range re__rune_range_clamp(re__rune_range range, re__rune_range bounds);

#if MN_DEBUG

void re__rune_debug_dump(re_rune rune);
void re__rune_range_debug_dump(re__rune_range rr);

#endif

/* ---------------------------------------------------------------------------
 * Character class (re_charclass.c)
 * ------------------------------------------------------------------------ */
typedef struct re__charclass {
    /* Non-overlapping, sorted set of ranges. */
    re__rune_range_vec ranges;
} re__charclass;
/* Implementation detail: since the list of ranges is in this normal form
 * (non-overlapping and sorted) doing certain operations is quick, especially
 * checking equality. Additionally, creating the optimized code for the forward
 * NFA is super fast, see re__compile_charclass.c for more details */

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

/* Initialize the given character class */
MN_INTERNAL void re__charclass_init(re__charclass* charclass);

/* Initialize the given character class from the ASCII charclass index, indexed
 * from enum re__charclass_ascii_type, optionally inverted */
MN_INTERNAL re_error re__charclass_init_from_class(re__charclass* charclass, re__charclass_ascii_type type, int inverted);

/* Initialize the given character class from the given string, the string should
 * be one of the default character class names (ripped from POSIX/C standard), 
 * see re__charclass_ascii_defaults[] in re__charclass.c, returns
 * RE_ERROR_INVALID if not found */
MN_INTERNAL re_error re__charclass_init_from_str(re__charclass* charclass, mn__str_view name, int inverted);

/* Destroy the given character class */
MN_INTERNAL void re__charclass_destroy(re__charclass* charclass);

/* Push a range onto the end of the given character class */
MN_INTERNAL re_error re__charclass_push(re__charclass* charclass, re__rune_range range);

/* Return the array of ranges stored in this character class, pointer is invalid
 * after calls to re__charclass_push() */
MN_INTERNAL const re__rune_range* re__charclass_get_ranges(const re__charclass* charclass);

/* Get the number of ranges in the character class */
MN_INTERNAL mn_size re__charclass_get_num_ranges(const re__charclass* charclass);

/* Check if the given character class is equal to the other */
MN_INTERNAL int re__charclass_equals(const re__charclass* charclass, const re__charclass* other);

#if MN_DEBUG

MN_INTERNAL void re__charclass_dump(const re__charclass* charclass, mn_size lvl);
MN_INTERNAL int re__charclass_verify(const re__charclass* charclass);

#endif


/* ---------------------------------------------------------------------------
 * Immediate-mode character class builder (re_charclass.c)
 * ------------------------------------------------------------------------ */
typedef struct re__charclass_builder {
    /* List of pending ranges, is built in a sorted, non-overlapping way */
    re__rune_range_vec ranges;
    /* 1 if the character class will be inverted when
     * re__charclass_builder_finish() is called */
    int should_invert;
    /* Optimization: highest rune in this charclass, if the rune-range to be
     * added is greater than this, we can avoid calling insert() on ranges and
     * just use push(), O(1) baby */
    re_rune highest;
} re__charclass_builder;

/* Initialize this character class builder */
MN_INTERNAL void re__charclass_builder_init(re__charclass_builder* builder);

/* Destroy this character class builder */
MN_INTERNAL void re__charclass_builder_destroy(re__charclass_builder* builder);

/* Begin building a character class */
MN_INTERNAL void re__charclass_builder_begin(re__charclass_builder* builder);

/* Set the invert flag -- when finish() is called the class will be inverted */
MN_INTERNAL void re__charclass_builder_invert(re__charclass_builder* builder);

/* Insert a range of characters into this character class */
MN_INTERNAL re_error re__charclass_builder_insert_range(re__charclass_builder* builder, re__rune_range range);

/* Insert another character class into this one */
MN_INTERNAL re_error re__charclass_builder_insert_class(re__charclass_builder* builder, re__charclass* charclass);

/* Finish building, and output results to the given character class */
MN_INTERNAL re_error re__charclass_builder_finish(re__charclass_builder* builder, re__charclass* charclass);


/* ---------------------------------------------------------------------------
 * AST node (re_ast.c)
 * ------------------------------------------------------------------------ */
typedef struct re__ast re__ast; 

/* Enumeration of AST node types. */
typedef enum re__ast_type {
    RE__AST_TYPE_MIN = 0,
    /* No type. Should never occur. */
    RE__AST_TYPE_NONE = 0,
    /* A single character. */
    /* TODO: fold into type_str */
    RE__AST_TYPE_RUNE,
    /* A string of characters. */
    RE__AST_TYPE_STR,
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

MN__VEC_DECL(re__ast);

/* Maximum number of repetitions a quantifier can have.
 * One of a few mechanisms in place to prevent diabolical program growth. */
#define RE__AST_QUANTIFIER_MAX 2000

/* Signifies infinite repetitions (*, +) */
#define RE__AST_QUANTIFIER_INFINITY RE__AST_QUANTIFIER_MAX+2

/* Quantifier info. */
/* Range: [min, max) */
typedef struct re__ast_quantifier_info {
    /* Minimum amount. */
    mn_int32 min;
    /* Maximum amount. Could be RE__AST_QUANTIFIER_MAX or
     * RE__AST_QUANTIFIER_INFINITY. */
    mn_int32 max;
    /* Whether or not to prefer fewer matches. */
    int greediness;
} re__ast_quantifier_info;

/* Assert types, as they are represented in the AST. */
typedef enum re__ast_assert_type {
    /* Minimum value */
    RE__AST_ASSERT_TYPE_MIN = 1,
    /* Text/Line start */
    RE__AST_ASSERT_TYPE_TEXT_START = 1,
    /* Text/Line start */
    RE__AST_ASSERT_TYPE_TEXT_END = 2,
    /* Text start */
    RE__AST_ASSERT_TYPE_TEXT_START_ABSOLUTE = 4,
    /* Text end */
    RE__AST_ASSERT_TYPE_TEXT_END_ABSOLUTE = 8,
    /* Word character (one-character lookaround) */
    RE__AST_ASSERT_TYPE_WORD = 16,
    /* Not a word character */
    RE__AST_ASSERT_TYPE_WORD_NOT = 32,
    /* Maximum value (non-inclusive) */
    RE__AST_ASSERT_TYPE_MAX = 64
} re__ast_assert_type;

/* Group flags */
typedef enum re__ast_group_flags {
    /* Group is case-insensitive */
    RE__AST_GROUP_FLAG_CASE_INSENSITIVE = 1,
    /* ^/$ match begin/end of line */
    RE__AST_GROUP_FLAG_MULTILINE = 2,
    /* . matches newline */
    RE__AST_GROUP_FLAG_DOT_NEWLINE = 4,
    /* Ungreedy matching: all quantifiers have greediness swapped */
    RE__AST_GROUP_FLAG_UNGREEDY = 8,
    /* Not a capturing group */
    RE__AST_GROUP_FLAG_NONMATCHING = 16,
    /* Group has a name */
    RE__AST_GROUP_FLAG_NAMED = 32,
    /* Maximum value (non-inclusive) */
    RE__AST_GROUP_FLAG_MAX = 64
} re__ast_group_flags;

/* Group info */
typedef struct re__ast_group_info {
    /* Group's flags */
    re__ast_group_flags flags;
    /* Index (ID) if this group */
    mn_uint32 group_idx;
} re__ast_group_info;

/* Holds AST node data depending on the node type. */
typedef union re__ast_data {
    /* RE__AST_TYPE_RUNE: holds a single character */
    re_rune rune;
    /* RE__AST_TYPE_STRING: holds a reference to a string. */
    mn_int32 str_ref;
    /* RE__AST_TYPE_CLASS: holds a reference to a character class. */
    mn_int32 charclass_ref;
    /* RE__AST_TYPE_GROUP: holds the group's index and flags */
    re__ast_group_info group_info;
    /* RE__AST_TYPE_QUANTIFIER: minimum/maximum/greediness */
    re__ast_quantifier_info quantifier_info;
    /* RE__AST_TYPE_ASSERT: type(s) of assert present */
    re__ast_assert_type assert_type;
} re__ast_data;

/* AST node */
struct re__ast {
    /* Reference to next / previous siblings */
    mn_int32 next_sibling_ref;
    mn_int32 prev_sibling_ref;
    /* Reference to first / last children */
    mn_int32 first_child_ref;
    mn_int32 last_child_ref;
    /* Type of AST node */
    re__ast_type type;
    /* Data describing this node */
    re__ast_data _data;
};
/* 32 bytes on my M1 Mac */
/* References are in arena format, that is, they are offsets in a contiguous
 * chunk of memory managed by re__ast_root. This design decision was made to
 * simplify destruction / moving of AST nodes, and it speeds up the parser
 * implementation. Null references are RE__AST_NONE. */

/* Initialize the given node as a RE__AST_TYPE_RUNE node */
MN_INTERNAL void re__ast_init_rune(re__ast* ast, re_rune rune);

/* Initialize the given node as a RE__AST_TYPE_STR node, str_ref is the index
 * of the node's string given by the ast_root object in its string arena */
MN_INTERNAL void re__ast_init_str(re__ast* ast, mn_int32 str_ref);

/* Initialize the given node as a RE__AST_TYPE_CHARCLASS node, charclass_ref
 * is the index of the node's charclass given by the ast_root object in its
 * charclass arena */
MN_INTERNAL void re__ast_init_charclass(re__ast* ast, mn_int32 charclass_ref);

/* Initialize the given node as a RE__AST_TYPE_CONCAT node */
MN_INTERNAL void re__ast_init_concat(re__ast* ast);

/* Initialize the given node as a RE__AST_TYPE_ALT node */
MN_INTERNAL void re__ast_init_alt(re__ast* ast);

/* Initialize the given node as a RE__AST_TYPE_QUANTIFIER node with the
 * specified boundaries min/max */
MN_INTERNAL void re__ast_init_quantifier(re__ast* ast, mn_int32 min, mn_int32 max);

/* Initialize the given node as a RE__AST_TYPE_GROUP with the given group index
 * and flags */
MN_INTERNAL void re__ast_init_group(re__ast* ast, mn_uint32 group_idx, re__ast_group_flags flags);

/* Initialize the given node as a RE__AST_TYPE_ASSERT with the given assert
 * type */
MN_INTERNAL void re__ast_init_assert(re__ast* ast, re__ast_assert_type assert_type);

/* Initialize the given node as a RE__AST_TYPE_ANY_CHAR (.) */
MN_INTERNAL void re__ast_init_any_char(re__ast* ast);

/* Initialize the given node as a RE__AST_TYPE_ANY_BYTE (\C) */
MN_INTERNAL void re__ast_init_any_byte(re__ast* ast);

/* Destroy the given node */
MN_INTERNAL void re__ast_destroy(re__ast* ast);

/* Get the given node's greediness (node must be RE__AST_TYPE_QUANTIFIER) */
MN_INTERNAL int re__ast_get_quantifier_greediness(const re__ast* ast);

/* Set the given node's greediness (node must be RE__AST_TYPE_QUANTIFIER) */
MN_INTERNAL void re__ast_set_quantifier_greediness(re__ast* ast, int is_greedy);

/* Get the given node's minimum repeat (node must be RE__AST_TYPE_QUANTIFIER) */
MN_INTERNAL mn_int32 re__ast_get_quantifier_min(const re__ast* ast);

/* Get the given node's maximum repeat (node must be RE__AST_TYPE_QUANTIFIER) */
MN_INTERNAL mn_int32 re__ast_get_quantifier_max(const re__ast* ast);

/* Get the given node's rune (node must be RE__AST_TYPE_RUNE) */
MN_INTERNAL re_rune re__ast_get_rune(const re__ast* ast);

/* Get the given node's group flags (node must be RE__AST_TYPE_GROUP) */
MN_INTERNAL re__ast_group_flags re__ast_get_group_flags(const re__ast* ast);

/* Get the given node's group index (node must be RE__AST_TYPE_GROUP) */
MN_INTERNAL mn_uint32 re__ast_get_group_idx(const re__ast* ast);

/* Get the given node's assert bits (node must be RE__AST_TYPE_ASSERT) */
MN_INTERNAL re__ast_assert_type re__ast_get_assert_type(const re__ast* ast);

/* Get the reference to the node's string in ast_root (node must be 
 * RE__AST_TYPE_STR) */
MN_INTERNAL mn_int32 re__ast_get_str_ref(const re__ast* ast);

MN__ARENA_DECL(re__charclass);
MN__ARENA_DECL(mn__str);

MN__VEC_DECL(mn__str);

/* Sentinel value for re__ast_root object */
#define RE__AST_NONE -1

/* ---------------------------------------------------------------------------
 * AST root manager (re_ast.c)
 * ------------------------------------------------------------------------ */
typedef struct re__ast_root {
    /* Vector of AST nodes, used as an arena */
    re__ast_vec ast_vec;
    /* Last empty location in ast_vec */
    mn_int32 last_empty_ref;
    /* Reference to root node */
    mn_int32 root_ref;
    /* Reference to last child of root node */
    mn_int32 root_last_child_ref;
    /* Character classes in use by RE__AST_TYPE_CHARCLASS nodes */
    re__charclass_arena charclasses;
    /* Strings in use by RE__AST_TYPE_STR nodes */
    mn__str_arena strings;
    /* Group names in use by RE__AST_TYPE_GROUP nodes */
    mn__str_vec group_names;
    /* Maximum depth recorded in this AST tree */
    mn_int32 depth_max;
} re__ast_root;

/* Initialize this ast root */
MN_INTERNAL void re__ast_root_init(re__ast_root* ast_root);

/* Destroy this ast root */
MN_INTERNAL void re__ast_root_destroy(re__ast_root* ast_root);

/* Given ast_ref, get a pointer to the re__ast within this ast root */
MN_INTERNAL re__ast* re__ast_root_get(re__ast_root* ast_root, mn_int32 ast_ref);

/* Given ast_ref, get a const pointer to the re__ast within this ast root */
MN_INTERNAL const re__ast* re__ast_root_get_const(const re__ast_root* ast_root, mn_int32 ast_ref);

/* Remove the given reference from the ast root. Does not clean up references 
 * from previous / parent nodes. Only used for testing. */
MN_INTERNAL void re__ast_root_remove(re__ast_root* ast_root, mn_int32 ast_ref);

/* Replace the node at the given reference with the given replacement. */
MN_INTERNAL void re__ast_root_replace(re__ast_root* ast_root, mn_int32 ast_ref, re__ast replacement);

/* Create a new node under the given parent with the given ast, storing its
 * resulting reference in out_ref. */
MN_INTERNAL re_error re__ast_root_add_child(re__ast_root* ast_root, mn_int32 parent_ref, re__ast ast, mn_int32* out_ref);

/* Create a new node under the given parent that wraps the given inner reference
 * with the given outer node, storing the resulting reference to ast_outer in
 * out_ref. */
MN_INTERNAL re_error re__ast_root_add_wrap(re__ast_root* ast_root, mn_int32 parent_ref, mn_int32 inner_ref, re__ast ast_outer, mn_int32* out_ref);

/* Register a new charclass, storing its reference in out_charclass_ref.*/
MN_INTERNAL re_error re__ast_root_add_charclass(re__ast_root* ast_root, re__charclass charclass, mn_int32* out_charclass_ref);

/* Get a const pointer to the charclass referenced by charclass_ref. */
MN_INTERNAL const re__charclass* re__ast_root_get_charclass(const re__ast_root* ast_root, mn_int32 charclass_ref); 

/* Register a new string, storing its reference in out_str_ref. */
MN_INTERNAL re_error re__ast_root_add_str(re__ast_root* ast_root, mn__str str, mn_int32* out_str_ref);

/* Get a pointer to the string referenced by str_ref. */
MN_INTERNAL mn__str* re__ast_root_get_str(re__ast_root* ast_root, mn_int32 str_ref);

/* Get the string view referenced by str_ref. */
MN_INTERNAL mn__str_view re__ast_root_get_str_view(const re__ast_root* ast_root, mn_int32 str_ref);

/* Register a new group. */
MN_INTERNAL re_error re__ast_root_add_group(re__ast_root* ast_root, mn__str_view group_name);

/* Get a group name referenced by its number. */
MN_INTERNAL mn__str_view re__ast_root_get_group(re__ast_root* ast_root, mn_uint32 group_number);

/* Get the number of groups in this ast root. */
MN_INTERNAL mn_uint32 re__ast_root_get_num_groups(re__ast_root* ast_root);

#if MN_DEBUG

MN_INTERNAL void re__ast_root_debug_dump(re__ast_root* ast_root, mn_int32 root_ref, mn_int32 lvl);
MN_INTERNAL int re__ast_root_verify(re__ast_root* ast_root);

#endif

/* ---------------------------------------------------------------------------
 * Parser (re_parse.c)
 * ------------------------------------------------------------------------ */
/* Enumeration of parse states */
/* The parser will likely be rewritten as recursive-descent with an explicit
 * stack. */
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
    RE__PARSE_STATE_CHARCLASS_NAMED_INITIAL,
    RE__PARSE_STATE_CHARCLASS_NAMED,
    RE__PARSE_STATE_CHARCLASS_NAMED_INVERTED,
    RE__PARSE_STATE_CHARCLASS_NAMED_AFTER_COLON
} re__parse_state;

/* Parse stack frame object. */
typedef struct re__parse_frame {
    /* Reference to base AST node that this frame is building */
    mn_int32 ast_frame_root_ref;
    /* Reference to previous child that was built by this frame, this is only
     * used in nodes with children, like quantifiers or alts */
    mn_int32 ast_prev_child_ref;
    /* Since ESCAPE and CHARCLASS share a lot of code, we use ret_state to
     * tell which state to return to after popping a frame in those cases, and
     * just merge a lot of the parsing code for those two cases */
    re__parse_state ret_state;
    /* Group flags, we need to explicitly maintain these on a stack so that we
     * correctly reset flags as we unwind the stack */
    re__ast_group_flags group_flags;
    /* Depth/maximum depth tracking of this frame */
    mn_int32 depth;
    mn_int32 depth_max;
} re__parse_frame;

MN__VEC_DECL(re__parse_frame);

typedef struct re__parse {
    re* re;
    re__ast_root* ast_root;
    re__parse_frame_vec frames;
    mn_int32 ast_frame_root_ref;
    mn_int32 ast_prev_child_ref;
    re__parse_state state;
    mn_int32 radix_num;
    int radix_digits;
    re__charclass_builder charclass_builder;
    int defer;
    re__ast_group_flags group_flags_new;
    re__ast_group_flags group_flags;
    const mn_char* str_begin;
    const mn_char* str_end;
    mn_int32 counting_first_num;
    re_rune charclass_lo_rune;
    mn_int32 depth;
    mn_int32 depth_max;
    mn_int32 depth_max_prev;
} re__parse;

MN_INTERNAL void re__parse_init(re__parse* parse, re* re);
MN_INTERNAL void re__parse_destroy(re__parse* parse);
MN_INTERNAL re_error re__parse_str(re__parse* parse, const mn__str_view* regex);


/* ---------------------------------------------------------------------------
 * Instruction format (re_prog.c)
 * ------------------------------------------------------------------------ */
typedef mn_uint32 re__prog_loc;

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
    RE__PROG_INST_TYPE_ASSERT,
    /* maximum value of enum */
    RE__PROG_INST_TYPE_MAX
} re__prog_inst_type;

/* Opcode-specific data */
typedef union re__prog_inst_data {
    /* RE__PROG_INST_TYPE_BYTE: a single byte */
    mn_uint8 _byte;
    /* RE__PROG_INST_TYPE_BYTE_RANGE: a range of bytes */
    re__byte_range _range;
    /* RE__PROG_INST_TYPE_SPLIT: secondary branch target */
    re__prog_loc _secondary;
    /* RE__PROG_INST_TYPE_MATCH: match index */
    mn_uint32 _match_idx;
    /* RE__PROG_INST_TYPE_ASSERT: assert context set */
    mn_uint32 _assert_context;
    /* RE__PROG_INST_TYPE_SAVE: save index */
    mn_uint32 _save_idx;
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

MN__VEC_DECL(re__prog_inst);

/* Initialize an instruction as a byte instruction, given its ordinal value in 
 * the range [0, 255] */
MN_INTERNAL void re__prog_inst_init_byte(re__prog_inst* inst, mn_uint8 byte);

/* Initialize an instruction as a byte range instruction, given a 
 * re__byte_range */
MN_INTERNAL void re__prog_inst_init_byte_range(re__prog_inst* inst, re__byte_range br);

/* Initialize an instruction as a split instruction, given its primary and
 * secondary branch targets */
MN_INTERNAL void re__prog_inst_init_split(re__prog_inst* inst, re__prog_loc primary, re__prog_loc secondary);

/* Initialize an instruction as a match instruction, given its match index */
MN_INTERNAL void re__prog_inst_init_match(re__prog_inst* inst, mn_uint32 match_idx);

/* Initialize an instruction as a fail instruction */
MN_INTERNAL void re__prog_inst_init_fail(re__prog_inst* inst);

/* Initialize an instruction as an assert instruction, given its assert context
 * flags */
MN_INTERNAL void re__prog_inst_init_assert(re__prog_inst* inst, mn_uint32 assert_context);

/* Initialize an instruction as a save instruction, given its save slot index */
MN_INTERNAL void re__prog_inst_init_save(re__prog_inst* inst, mn_uint32 save_idx);

/* Get the primary branch target of an instruction */
MN_INTERNAL re__prog_loc re__prog_inst_get_primary(const re__prog_inst* inst);

/* Set the primary branch target of an instruction */
MN_INTERNAL void re__prog_inst_set_primary(re__prog_inst* inst, re__prog_loc loc);

/* Get an instruction's type */
MN_INTERNAL re__prog_inst_type re__prog_inst_get_type(const re__prog_inst* inst);

/* Get the byte value of an instruction (instruction must be 
 * RE__PROG_INST_TYPE_BYTE) */
MN_INTERNAL mn_uint8 re__prog_inst_get_byte(const re__prog_inst* inst);

/* Get the minimum byte value of an instruction (instruction must be 
 * RE__PROG_INST_TYPE_BYTE_RANGE) */
MN_INTERNAL mn_uint8 re__prog_inst_get_byte_min(const re__prog_inst* inst);

/* Get the maximum byte value of an instruction (instruction must be 
 * RE__PROG_INST_TYPE_BYTE_RANGE) */
MN_INTERNAL mn_uint8 re__prog_inst_get_byte_max(const re__prog_inst* inst);

/* Get the secondary branch target of an instruction (instruction must be
 * RE__PROG_INST_TYPE_SPLIT) */
MN_INTERNAL re__prog_loc re__prog_inst_get_split_secondary(const re__prog_inst* inst);

/* Set the secondary branch target of an instruction (instruction must be
 * RE__PROG_INST_TYPE_SPLIT) */
MN_INTERNAL void re__prog_inst_set_split_secondary(re__prog_inst* inst, re__prog_loc loc);

/* Get an instruction's assert context (instruction must be 
 * RE__PROG_INST_TYPE_ASSERT) */
MN_INTERNAL re__ast_assert_type re__prog_inst_get_assert_ctx(const re__prog_inst* inst);

/* Get an instruction's match index (instruction must be 
 * RE__PROG_INST_TYPE_MATCH) */
MN_INTERNAL mn_uint32 re__prog_inst_get_match_idx(const re__prog_inst* inst);

/* Get an instruction's save index (instruction must be 
 * RE__PROG_INST_TYPE_SAVE) */
MN_INTERNAL mn_uint32 re__prog_inst_get_save_idx(const re__prog_inst* inst);

/* Check if two instructions are equal */
MN_INTERNAL int re__prog_inst_equals(re__prog_inst* a, re__prog_inst* b);


/* ---------------------------------------------------------------------------
 * Program (re_prog.c)
 * ------------------------------------------------------------------------ */
#define RE__PROG_SIZE_MAX 100000

/* Program entry points. */
typedef enum re__prog_entry {
    /* Default entry point - start of the program as compiled. */
    RE__PROG_ENTRY_DEFAULT,
    /* Dotstar entry - used for left-unanchored matches. Calls into 
     * RE__PROG_ENTRY_DEFAULT. */
    RE__PROG_ENTRY_DOTSTAR,
    /* Maximum value */
    RE__PROG_ENTRY_MAX
} re__prog_entry;

/* The program itself */
typedef struct re__prog {
    /* Instruction listing */
    re__prog_inst_vec _instructions;
    /* Entrypoints */
    re__prog_loc _entrypoints[RE__PROG_ENTRY_MAX];
} re__prog;

/* Initialize a program. */
MN_INTERNAL re_error re__prog_init(re__prog* prog);

/* Destroy a program. */
MN_INTERNAL void re__prog_destroy(re__prog* prog);

/* Get the size of a program. Also tells the location of the next inst. */
MN_INTERNAL re__prog_loc re__prog_size(const re__prog* prog);

/* Get a pointer to the instruction at the given location in the program. */
MN_INTERNAL re__prog_inst* re__prog_get(re__prog* prog, re__prog_loc loc);

/* Get a const pointer to the instruction at the given location in the
 * program. */
MN_INTERNAL const re__prog_inst* re__prog_get_const(const re__prog* prog, re__prog_loc loc);

/* Add an instruction to the end of the program. */
MN_INTERNAL re_error re__prog_add(re__prog* prog, re__prog_inst inst);

/* Check if a program is equal to another one. */
MN_INTERNAL int re__prog_equals(re__prog* a, re__prog* b);

/* Set the given entrypoint of a program to the given location. */
MN_INTERNAL void re__prog_set_entry(re__prog* prog, re__prog_entry idx, re__prog_loc loc);

/* Get the given entrypoint of a program. */
MN_INTERNAL re__prog_loc re__prog_get_entry(const re__prog* prog, re__prog_entry idx);

#if MN_DEBUG
MN_INTERNAL void re__prog_debug_dump(const re__prog* prog);
#endif


/* ---------------------------------------------------------------------------
 * Compilation patch list (re_compile.c)
 * ------------------------------------------------------------------------ */
/* A list of program patches -- locations in the program that need to point to
 * later instructions */
/* Like RE2, we store the location of the next patch in each instruction, so it
 * turns out to be a linked-list of sorts. */
/* This representation is useful because we only really need prepend, append,
 * and iterate facilities. So in addition to not having to perform manual memory
 * allocation, we get quick operations "for free". */
typedef struct re__compile_patches {
    /* Head of linked list */
    re__prog_loc first_inst;
    /* Tail of linked list */
    re__prog_loc last_inst;
} re__compile_patches;

/* Initialize a patch list object */
MN_INTERNAL void re__compile_patches_init(re__compile_patches* patches);

/* Append the given location to the patch list object, given a re__prog for
 * linked-list storage, and a flag indicating if the location is to have its
 * secondary branch target patched. */
MN_INTERNAL void re__compile_patches_append(re__compile_patches* patches, re__prog* prog, re__prog_loc to, int secondary);

#if RE_DEBUG

MN_INTERNAL void re__compile_patches_dump(re__compile_patches* patches, re__prog* prog);

#endif


/* ---------------------------------------------------------------------------
 * Pre-compiled programs (re_prog_data.c)
 * ------------------------------------------------------------------------ */
/* Pre-compiled programs, at this point they only exist as optimized UTF-8 .
 * representations */
typedef enum re__prog_data_id {
    /* dot, forward, accept surrogates */
    RE__PROG_DATA_ID_DOT_FWD_ACCSURR,
    /* dot, reverse, accept surrogates */
    RE__PROG_DATA_ID_DOT_REV_ACCSURR,
    /* dot, forward, reject surrogates */
    RE__PROG_DATA_ID_DOT_FWD_REJSURR,
    /* dot, reverse, accept surrogates */
    RE__PROG_DATA_ID_DOT_REV_REJSURR,
    RE__PROG_DATA_ID_MAX
} re__prog_data_id;

/* Pointers to precompiled program data. See re__prog_decompress for the
 * compressed program format. */
MN_INTERNAL mn_uint8* re__prog_data[RE__PROG_DATA_ID_MAX];

/* Corresponding sizes of precompiled program data. */
MN_INTERNAL mn_size re__prog_data_size[RE__PROG_DATA_ID_MAX];

/* Decompress a compressed program into an re__prog object, storing outward
 * patches in the given patches object. */
/* The format for compressed data is as follows:
 * <program> ::= <insts>
 * <insts> ::= <inst> | <inst> <insts>
 * <inst> ::= <inst_byte> | <inst_range> | <inst_split>
 * <inst_byte> ::= 0x00 <loc> <byte_value>
 * <inst_range> ::= 0x01 <loc> <byte_value> <byte_value>
 * <inst_split> ::= 0x02 <loc> <loc>
 * <loc> ::= [0x00-0x7F] | [0x80-0xFF] <loc>
 * <byte_value> ::= [0x00-0xFF]
 * 
 * Notes:
 *   <loc> is a variable-length integer, big-endian, where the top bit indicates
 *   more data.
 * 
 * Returns RE__ERROR_COMPRESSION_FORMAT if the format is wrong.
 */
MN_INTERNAL re_error re__prog_data_decompress(re__prog* prog, mn_uint8* compressed_data, mn_size compressed_size, re__compile_patches* patches);


/* ---------------------------------------------------------------------------
 * Character class compiler (re_compile_charclass.c)
 * ------------------------------------------------------------------------ */
typedef struct re__compile_charclass re__compile_charclass;

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
#define RE__COMPILE_CHARCLASS_TREE_NONE 0
typedef struct re__compile_charclass_tree {
    /* Range of bytes to match */
    re__byte_range byte_range;
    /* Reference to next sibling */
    mn_uint32 sibling_ref;
    /* Reference to first child */
    mn_uint32 child_ref;
    /* Either:
     * - The hash of this tree, used for caching
     * - A reference to this node's complement in the reverse tree
     * If the tree has not had its forward program generated, then aux is a
     * hash, otherwise, the reverse program is being generated, and aux
     * references the complement node. */
    /* This could be better clarified with a union, I plan on doing this 
     * sometime soon. */
    mn_uint32 aux;
} re__compile_charclass_tree;
/* 16 bytes, nominally, (16 on my M1 Max) */

re_error re__compile_charclass_new_node(re__compile_charclass* char_comp, mn_uint32 parent_ref, re__byte_range byte_range, mn_uint32* out_new_node_ref, int use_reverse_tree);

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
 * link them together using the 'next' member, forming a mini linked list. */
typedef struct re__compile_charclass_hash_entry {
    /* Index in sparse array */
    mn_int32 sparse_index;
    /* Reference to tree root */
    mn_uint32 root_ref;
    /* Compiled instruction location in the program */
    re__prog_loc prog_loc;
    /* Next hash_entry that hashes to the same value, 
     * RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE otherwise. */
    mn_int32 next;
} re__compile_charclass_hash_entry;

MN_INTERNAL void re__compile_charclass_hash_entry_init(re__compile_charclass_hash_entry* hash_entry, mn_int32 sparse_index, mn_uint32 tree_ref, re__prog_loc prog_loc);

MN__VEC_DECL(re__compile_charclass_hash_entry);
MN__VEC_DECL(re__compile_charclass_tree);

#define RE__COMPILE_CHARCLASS_CACHE_SPARSE_SIZE 1024

/* Character class compiler. */
struct re__compile_charclass {
    /* Vector of tree nodes. Each '_ref' entry in re__compile_charclass_tree
     * represents an index in this vector. */
    re__compile_charclass_tree_vec tree;
    /* Reference to root node. */
    mn_uint32 root_ref;
    /* Reference to last child of root node. */
    mn_uint32 root_last_child_ref;
    /* Reference to root node of reverse tree. */
    mn_uint32 rev_root_ref;
    /* Reference to last child of reverse root node. */
    mn_uint32 rev_root_last_child_ref;
    /* Sparse tree cache. Each element in this array points to a corresponding
     * position in 'cache_dense'. Lookup is performed by moduloing a tree's hash
     * with the sparse cache size. Since cache hits are relatively rare, this
     * allows pretty inexpensive lookup. */
    mn_int32* cache_sparse;
    /* Dense tree cache. Entries with duplicate hashes are linked using their
     * 'next' member. */
    re__compile_charclass_hash_entry_vec cache_dense;
};

void re__compile_charclass_init(re__compile_charclass* char_comp);
void re__compile_charclass_destroy(re__compile_charclass* char_comp);
re_error re__compile_charclass_gen(re__compile_charclass* char_comp, const re__charclass* charclass, re__prog* prog, re__compile_patches* patches_out, int also_make_reverse);
re_error re__compile_charclass_split_rune_range(re__compile_charclass* char_comp, re__rune_range range);
MN_INTERNAL re__compile_charclass_tree* re__compile_charclass_tree_get(re__compile_charclass* char_comp, mn_uint32 tree_ref);

#if RE_DEBUG
void re__compile_charclass_dump(re__compile_charclass* char_comp, mn_uint32 tree_idx, mn_int32 indent);
#endif


/* ---------------------------------------------------------------------------
 * Program compiler (re_compile.c)
 * ------------------------------------------------------------------------ */
/* Stack frame for the compiler. */
typedef struct re__compile_frame {
    /* Base AST node being compiled. Represents the current node that is being
     * examined at any given point. */
    mn_int32 ast_base_ref;
    /* Next child node of ast_base_ref that will be compiled. Will be AST_NONE
     * if the last child has already been processed. */
    mn_int32 ast_child_ref;
    /* Running set of patches for this AST node. */
    re__compile_patches patches;
    /* Start and end PCs of this frame */
    re__prog_loc start;
    re__prog_loc end;
    /* For repetitions: the number of times the node's child has been generated
     * already */
    mn_int32 rep_idx;
} re__compile_frame;

/* Compiler internal structure. */
typedef struct re__compile {
    /* Stack frames list. This is not a frame_vec because we already have the
     * maximum stack depth (stored in ast_root). frames is a static worst-case
     * allocation. Generally pretty speedy. I took a cue from that online regex
     * bible for this one. */
    /* "In retrospect, I think the tree form and the Walker might have been a 
     *  mistake ... if the RPN form recorded the maximum stack depth used in the 
     *  expression, a traversal would allocate a stack of exactly that size and 
     *  then zip through the representation in a single linear scan." 
     *    -rsc */
    re__compile_frame* frames;
    /* Size of frames (equal to ast_root->max_depth) */
    mn_int32 frames_size;
    /* Current location in frames (the stack pointer) */
    mn_int32 frame_ptr;
    /* Charclass compiler object to be used to compile all charclasses, it's
     * better to store it here in case there are multiple character classes in
     * the regex */
    re__compile_charclass char_comp;
    /* The ast root object, marked const so we don't modify it (after the parse
     * phase, we should ideally never touch ast_root) */
    const re__ast_root* ast_root;
    /* Flag set if the compiler should push a child frame to the stack for the
     * next iteration. */ 
    int should_push_child;
    /* Reference to the child that should get pushed. After the child is done
     * processing, the parent's frame->ast_child_ref will point to the next
     * sibling of should_push_child_ref. */
    mn_int32 should_push_child_ref;
    /* Returned (popped) frame from child compilation. Contains child boundaries
     * and, more importantly, child patches. */ 
    re__compile_frame returned_frame;
    /* Whether or not we are compiling in reverse mode. */
    int reversed;
} re__compile;

MN_INTERNAL void re__compile_init(re__compile* compile);
MN_INTERNAL void re__compile_destroy(re__compile* compile);
MN_INTERNAL re_error re__compile_regex(re__compile* compile, const re__ast_root* ast_root, re__prog* prog, int reversed);
MN_INTERNAL int re__compile_gen_utf8(re_rune codep, mn_uint8* out_buf);
MN_INTERNAL re_error re__compile_dotstar(re__prog* prog, int reversed);


/* ---------------------------------------------------------------------------
 * NFA execution context (re_exec_nfa.c)
 * ------------------------------------------------------------------------ */
/* Execution thread. */
typedef struct re__exec_thrd {
    /* PC of this thread */
    re__prog_loc loc;
    /* Slot to save match boundaries to. May be -1 if this thread hasn't found
     * anything yet. */
    mn_int32 save_slot;
} re__exec_thrd;

MN__VEC_DECL(re__exec_thrd);

/* Sparse set of threads. */
typedef struct re__exec_thrd_set {
    /* Sparse representation: when indexed with a program location, returns the
     * index within 'dense' that holds the thread data */
    re__prog_loc* sparse;
    /* Dense representation: stores each thread based on the order they were
     * added, each thread->prog_loc in this should point back to 'sparse' */
    re__exec_thrd* dense;
    /* Number of elements actually in 'dense' */
    re__prog_loc n;
    /* Allocation size of 'dense' (sparse is a single worst case alloc) */
    re__prog_loc size;
    /* 0 if this does not contain a match instruction, 1+ otherwise */
    mn_uint32 match_index;
    /* 0 if the match is the top state, 1+ otherwise */
    mn_uint32 match_priority;
    /* Note: (match_priority != 0) implies (match_index != 0) */
} re__exec_thrd_set;

MN__VEC_DECL(mn_size);

/* Save state manager for exec. */
typedef struct re__exec_save {
    mn_size_vec slots;
    mn_int32 last_empty_ref;
    mn_uint32 slots_per_thrd;
} re__exec_save;

MN_INTERNAL void re__exec_save_init(re__exec_save* save);
MN_INTERNAL void re__exec_save_set_slots_per_thrd(re__exec_save* save, mn_uint32 slots_per_thrd);
MN_INTERNAL void re__exec_save_destroy(re__exec_save* save);
MN_INTERNAL const mn_size* re__exec_save_get_slots_const(const re__exec_save* save, mn_int32 slots_ref);
MN_INTERNAL mn_size* re__exec_save_get_slots(re__exec_save* save, mn_int32 slots_ref);
MN_INTERNAL void re__exec_save_inc_refs(re__exec_save* save, mn_int32 slots_ref);
MN_INTERNAL void re__exec_save_dec_refs(re__exec_save* save, mn_int32 slots_ref);
MN_INTERNAL mn_size re__exec_save_get_refs(const re__exec_save* save, mn_int32 slots_ref);
MN_INTERNAL re_error re__exec_save_get_new(re__exec_save* save, mn_int32* slots_out_ref);
MN_INTERNAL re_error re__exec_save_do_save(re__exec_save* save, mn_int32* slots_inout_ref, mn_uint32 slot_number, mn_size data);

/* Execution context. */
typedef struct re__exec_nfa {
    const re__prog* prog;
    mn_uint32 num_groups;
    re__exec_thrd_set set_a;
    re__exec_thrd_set set_b;
    re__exec_thrd_set set_c;
    re__exec_thrd_vec thrd_stk;
    re__exec_save save_slots;
} re__exec_nfa;

MN_INTERNAL void re__exec_nfa_init(re__exec_nfa* exec, const re__prog* prog, mn_uint32 num_groups);
MN_INTERNAL re__prog_loc re__exec_nfa_get_thrds_size(re__exec_nfa* exec);
MN_INTERNAL const re__exec_thrd* re__exec_nfa_get_thrds(re__exec_nfa* exec);
MN_INTERNAL void re__exec_nfa_set_thrds(re__exec_nfa* exec, const re__prog_loc* in_thrds, re__prog_loc in_thrds_size);
MN_INTERNAL mn_uint32 re__exec_nfa_get_match_index(re__exec_nfa* exec);
MN_INTERNAL mn_uint32 re__exec_nfa_get_match_priority(re__exec_nfa* exec);
MN_INTERNAL void re__exec_nfa_set_match_index(re__exec_nfa* exec, mn_uint32 match_index);
MN_INTERNAL void re__exec_nfa_set_match_priority(re__exec_nfa* exec, mn_uint32 match_priority);
MN_INTERNAL re_error re__exec_nfa_start(re__exec_nfa* exec, re__ast_assert_type assert_ctx, re__prog_entry entry);
MN_INTERNAL re_error re__exec_nfa_run(re__exec_nfa* exec, mn_uint8 ch, mn_size pos, re__ast_assert_type assert_ctx);
MN_INTERNAL re_error re__exec_nfa_finish(re__exec_nfa* exec, re_span* out, mn_size pos);
MN_INTERNAL void re__exec_nfa_destroy(re__exec_nfa* exec);

#if MN_DEBUG

MN_INTERNAL void re__exec_nfa_debug_dump(const re__exec_nfa* exec, int with_save);

#endif


/* ---------------------------------------------------------------------------
 * DFA execution context (re_exec_dfa.c)
 * ------------------------------------------------------------------------ */
MN__VEC_DECL(re__prog_loc);

#define RE__EXEC_DFA_PAGE_SIZE 4
#define RE__EXEC_DFA_THRD_LOCS_PAGE_SIZE 1024

enum re__exec_dfa_sym {
    RE__EXEC_DFA_SYM_EOT = 256,
    RE__EXEC_DFA_SYM_MAX
};

typedef struct re__exec_dfa_state re__exec_dfa_state;

typedef re__exec_dfa_state* re__exec_dfa_state_ptr;

typedef enum re__exec_dfa_flags {
    RE__EXEC_DFA_FLAG_FROM_WORD = 1,
    RE__EXEC_DFA_FLAG_START_STATE = 2,
    RE__EXEC_DFA_FLAG_START_STATE_BEGIN_TEXT = 4,
    RE__EXEC_DFA_FLAG_START_STATE_BEGIN_LINE = 8
} re__exec_dfa_flags;

/* DFA state. */
struct re__exec_dfa_state {
    re__exec_dfa_state_ptr next[RE__EXEC_DFA_SYM_MAX];
    re__exec_dfa_flags flags;
    mn_uint32 match_index;
    mn_uint32 match_priority;
    mn_uint32* thrd_locs_begin;
    mn_uint32* thrd_locs_end;
    mn_uint32 empty;
    re__prog_entry start_entry;
};

typedef struct re__exec_dfa_cache_entry {
    re__exec_dfa_state_ptr state_ptr;
    mn_uint32 hash;
} re__exec_dfa_cache_entry;

typedef mn_uint32* mn_uint32_ptr;

MN__VEC_DECL(re__exec_dfa_state_ptr);
MN__VEC_DECL(mn_uint32_ptr);

typedef enum re__exec_dfa_start_state_flags {
    RE__EXEC_DFA_START_STATE_FLAG_AFTER_WORD = 1,
    RE__EXEC_DFA_START_STATE_FLAG_BEGIN_TEXT = 2,
    RE__EXEC_DFA_START_STATE_FLAG_BEGIN_LINE = 4,
    RE__EXEC_DFA_START_STATE_COUNT = 8
} re__exec_dfa_start_state_flags;

typedef struct re__exec_dfa {
    re__exec_dfa_state_ptr current_state;
    re__exec_dfa_state_ptr start_states[RE__EXEC_DFA_START_STATE_COUNT * RE__PROG_ENTRY_MAX];
    re__exec_dfa_state_ptr_vec state_pages;
    mn_size state_page_idx;
    mn_uint32_ptr_vec thrd_loc_pages;
    mn_size thrd_loc_page_idx;
    re__exec_nfa nfa;
    /* targets a load factor of 0.75 */
    re__exec_dfa_cache_entry* cache;
    mn_size cache_stored;
    mn_size cache_alloc;
    mn_uint32 prev_sym;
} re__exec_dfa;

MN_INTERNAL void re__exec_dfa_init(re__exec_dfa* exec, const re__prog* prog);
MN_INTERNAL void re__exec_dfa_destroy(re__exec_dfa* exec);
MN_INTERNAL re_error re__exec_dfa_start(re__exec_dfa* exec, re__prog_entry entry, re__exec_dfa_start_state_flags start_state_flags);
MN_INTERNAL re_error re__exec_dfa_run(re__exec_dfa* exec, mn_uint32 next_sym);
MN_INTERNAL mn_uint32 re__exec_dfa_get_match_index(re__exec_dfa* exec);
MN_INTERNAL mn_uint32 re__exec_dfa_get_match_priority(re__exec_dfa* exec);
MN_INTERNAL int re__exec_dfa_get_exhaustion(re__exec_dfa* exec);
MN_INTERNAL void re__exec_dfa_debug_dump(re__exec_dfa* exec);

/* ---------------------------------------------------------------------------
 * Top-level data (re_re.c)
 * ------------------------------------------------------------------------ */
/* Internal data structure */
struct re_data {
    re__parse parse;
    re__ast_root ast_root;
    re__prog program;
    re__prog program_reverse;
    re__compile compile;
    /* Note: error_string_view always points to either a static const char* that
     * is a compile-time constant or a dynamically-allocated const char* inside
     * of error_string. Either way, in OOM situations, we will not allocate more
     * memory to store an error string and default to a constant. */  
    mn__str error_string;
    mn__str_view error_string_view;
};

MN_INTERNAL void re__set_error_str(re* re, const mn__str* error_str);
MN_INTERNAL void re__set_error_generic(re* re, re_error err);

#endif

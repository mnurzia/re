#include "re_internal.h"

/* Get a pointer to a node in the tree, given its index. */
RE_INTERNAL re__compile_charclass_tree* re__compile_charclass_tree_get(re__compile_charclass* char_comp, re_int32 root_ref) {
    RE_ASSERT(root_ref != RE__COMPILE_CHARCLASS_TREE_NONE);
    RE_ASSERT(root_ref < (re_int32)re__compile_charclass_tree_vec_size(&char_comp->tree));
    return re__compile_charclass_tree_vec_getref(&char_comp->tree, (re_size)root_ref);
}

/* Initialize a tree node as a terminal node. */
void re__compile_charclass_tree_init(re__compile_charclass_tree* root, re__byte_range byte_range) {
    root->byte_range = byte_range;
    root->prev_sibling_ref = RE__COMPILE_CHARCLASS_TREE_NONE;
    root->next_sibling_ref = RE__COMPILE_CHARCLASS_TREE_NONE;
    root->first_child_ref = RE__COMPILE_CHARCLASS_TREE_NONE;
    root->last_child_ref = RE__COMPILE_CHARCLASS_TREE_NONE;
    root->hash = 0;
}

/* Create a new child if it doesn't exist, otherwise return the child with the
 * corresponding range. */
re_error re__compile_charclass_touch_child(re__compile_charclass* char_comp, re_int32 root_ref, re__byte_range byte_range, re_int32* out_child_ref) {
    re__compile_charclass_tree* root = re__compile_charclass_tree_get(char_comp, root_ref);
    /* New child node */
    re__compile_charclass_tree new_child;
    /* Reference to the next created node */
    re_int32 next_root_ref = (re_int32)re__compile_charclass_tree_vec_size(&char_comp->tree);
    re_error err = RE_ERROR_NONE;
    if (root->first_child_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
        /* Root has no children. Add this range as a child and return it. */
        /*     creates this node
         *           \/ \/
         * [root]---[AA-BB] */
        re__compile_charclass_tree_init(&new_child, byte_range);
        /* Set root's children to just the new node */
        root->first_child_ref = next_root_ref;
        root->last_child_ref = next_root_ref;
        if ((err = re__compile_charclass_tree_vec_push(&char_comp->tree, new_child))) {
            return err;
        }
        /* tree is invalid */
        *out_child_ref = next_root_ref;
    } else {
        /* Root has children, either return the matching child or create a new
         * one. */
        re_int32 current_child_ref = root->first_child_ref;
        re__compile_charclass_tree* current_child = re__compile_charclass_tree_get(char_comp, current_child_ref);
        while (1) {
            if (re__byte_range_equals(byte_range, current_child->byte_range)) {
                /* Found matching child */
                *out_child_ref = current_child_ref;
                return RE_ERROR_NONE;
            }
            if (current_child->next_sibling_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
                /* Got to end of children without finding a child */
                break;
            } else {
                /* Keep iterating, load next child */
                current_child_ref = current_child->next_sibling_ref;
                current_child = re__compile_charclass_tree_get(char_comp, current_child_ref);
            }
        }
        /* Add the new child to the end of root's children */
        re__compile_charclass_tree_init(&new_child, byte_range);
        /* Link the new child and the last child to each other */
        current_child->next_sibling_ref = next_root_ref;
        new_child.prev_sibling_ref = current_child_ref;
        /* Update root's last child to point to the new child */
        root->last_child_ref = next_root_ref;
        /* Finally, add the new child to the end of the tree vector. */
        if ((err = re__compile_charclass_tree_vec_push(&char_comp->tree, new_child))) {
            return err;
        }
        *out_child_ref = next_root_ref;
    }
    return RE_ERROR_NONE;
}

/* Recursive function to add a rune range to the tree. */
/* This function takes some explaining. It operates on UTF-8 codepoints, which
 * have the following forms:
 * 
 * Codepoint Ranges    Bits                     Encoded Bytes
 * [U+000000-U+00007F] 00000000000000yyyyyyy -> 0yyyyyyy (ASCII)
 * [U+000080-U+0007FF] 0000000000yyyyyxxxxxx -> 110yyyyy 10xxxxxx
 * [U+000800-U+00FFFF] 00000yyyyxxxxxxxxxxxx -> 1110yyyy 10xxxxxx 10xxxxxx
 * [U+010000-U+10FFFF] yyyxxxxxxxxxxxxxxxxxx -> 11110yyy 10xxxxxx 10xxxxxx 10xxxxxx
 * 
 * We notice that each 'class' of codepoints has a unique number of bits labeled
 * 'y' and a unique number of bits labeled 'x':
 * 
 *                     y-bits  x-bits
 * [U+000000-U+00007F]    7       0
 * [U+000080-U+0007FF]    5       6
 * [U+000800-U+00FFFF]    4      12
 * [U+010000-U+10FFFF]    3      18
 * 
 * Furthermore, for each length of continuation bytes, we can classify them as
 * runs of y-bits and x-bits:
 * 
 * Bits                  Encoded Bytes              y-bits  x-bits 
 * 000000000000yyyyyy -> 10yyyyyy                      6       0
 * 000000yyyyyyxxxxxx -> 10yyyyyy 10xxxxxx             6       6
 * yyyyyyxxxxxxxxxxxx -> 10yyyyyy 10xxxxxx 10xxxxxx    6      12
 * 
 * We now have seven 'classes' of byte sequences with unique numbers of 'y'
 * and 'x' bits.
 * However, the neat thing is that each class, when its y-bits are set to 0,
 * corresponds to another, smaller class or nothing if it only represents a
 * 1-byte sequence.
 * 
 *                     y-bits  x-bits   Next Class  Notes
 * [U+000000-U+00007F]    7       0       <none>    [one-byte seq.]
 * [U+000080-U+0007FF]    5       6    1-byte cont. [beginning of two-byte seq.]
 * [U+000800-U+00FFFF]    4      12    2-byte cont. [beginning of three-byte seq.]
 * [U+010000-U+10FFFF]    3      18    3-byte cont. [beginning of four-byte seq.]
 * 1-byte continuation    6       0       <none>    [one continuation byte]
 * 2-byte continuation    6       6    1-byte cont. [two continuation bytes]
 * 3-byte continuation    6      12    2-byte cont. [three continuation bytes]
 * 
 * We can transform this into a kind of state machine where each state
 * corresponds to one of these classes, and the transitions between each state
 * correspond to output bytes.
 * For example, take the character U+546. Since it is in the range [U+80-U+7FF],
 * we start in that state, with y-bits=5 and x-bits=6. Then, we output the first
 * byte in a two-byte unicode sequence (or with 11011111), and switch to the
 * 1-byte class/state.
 * 
 * State         Codepoint Bits        Y Bits X Bits Output Byte
 * 2-byte begin. 000000000010101000110  10101 000110    11010101
 * 1-byte cont.  000000000000000000110 000110    N/A    10000110
 * 
 * The nice thing about y-bits and x-bits is that it's really easy to go from
 * state to state: you're finished if the number of x-bits is 0, and if not, 
 * set the number of y-bits to 6 and subtract 6 from the number of x-bits.
 * 
 * The outputted bytes are 11010101 10000110, the UTF-8 for U+546.
 * 
 * Another example, U+10F6F7:
 * State         Codepoint Bits        Y Bits             X Bits Output Byte
 * 4-byte begin. 100001111011011110111    100 001111011011110111    11110100
 * 3-byte cont.  000001111011011110111 001111       011011110111    10001111
 * 2-byte cont.  000000000011011110111 011011             110111    10011011
 * 1-byte cont.  000000000000000110111 110111                N/A    10110111
 *
 * This generalizes to ranges of bytes, only the output is the tree structure
 * instead of a sequence of bytes.
 * Every time we change state, we "fork" the range (split it into smaller
 * ranges with the same leading byte) and generate trees for each of those
 * ranges. Here's an example with a relatively simple range, [U+80-U+E5].
 * 
 * The encoding sequences (UTF-8 bytes) for this range are, in hex:
 * [C2-C2][80-BF]
 * [C3-C3][80-A5]
 * 
 * We start by looking at the Y-bits of each range bound.
 *      Codepoint Bits Y Bits Encoded First Byte
 * U+80       10000000  00010    11000010 (0xC2)
 * U+E5       11000011  00011    11000011 (0xC3)
 * 
 * Since these differ in terms of their encoded first byte, we will need two
 * tree nodes, one for U+80 and one for U+E5, by encoding them as 0xC2 and 0xC3.
 * The tree of output byte ranges now looks like this:
 * 
 * ---[C2-C2]
 *       |
 *    [C3-C3]
 *
 * Each of these nodes has a sub-node that corresponds to the next encoded byte,
 * a.k.a. the x-bits of 0x80 and 0xE5.
 * We generate these sub-nodes by splitting up [U+80-U+E5] into two ranges per
 * their first encoded byte. In this case, the ranges are [U+80-U+BF] and
 * [U+C0-U+E5].
 * We reduce these sub-ranges by taking off their y-bits and masking them with
 * a Unicode continuation byte mask.
 *         Range, Original                Range, No Y              Range, Masked
 * [U+80-U+BF] 10000000 10111111 -> [00-3F] 000000 111111 -> [80-BF] 10000000 10111111
 * [U+C0-U+E5] 11000000 11100101 -> [80-A5] 000000 100101 -> [80-A5] 10000000 10100101
 * 
 * These final masked ranges are then inserted into the tree:
 * 
 * ---[C2-C2]---[80-BF]
 *       |
 *    [C3-C3]---[80-A5]
 * 
 * This corresponds to our original list of valid UTF-8 sequences for [U+80-U+E5]:
 * [C2-C2][80-BF]
 * [C3-C3][80-A5]
 * 
 * For a more complex example, the entire UTF-8 range [U+0-U+10FFFF], it is
 * necessary to "split" the range into sub-ranges that share common sequence
 * lengths. In particular, we need to turn [U+0-U+10FFFF] into:
 * 
 * [U+00-U+7F]
 * [U+80-U+7FF]
 * [U+800-U+FFFF]
 * [U+10000-U+10FFFF]
 * 
 * which is simple enough, and then these ranges into:
 * 
 * [00-7F]
 * [C2-DF][80-BF]
 * [E0-E0][A0-BF][80-BF]
 * [E1-EF][80-BF][80-BF]
 * [F0-F0][90-BF][80-BF][80-BF]
 * [F1-F3][80-BF][80-BF][80-BF]
 * [F4-F4][80-8F][80-BF][80-BF]
 * 
 * Finally, they are compiled into a tree:
 * 
 * ---[00-7F]
 *       |
 *    [C2-DF]---[80-BF]
 *       |
 *    [E0-E0]---[A0-BF]---[80-BF]
 *       |
 *    [E1-EF]---[80-BF]---[80-BF]
 *       |
 *    [F0-F0]---[90-BF]---[80-BF]---[80-BF]
 *       |
 *    [F1-F3]---[80-BF]---[80-BF]---[80-BF]
 *       |
 *    [F4-F4]---[80-8F]---[80-BF]---[80-BF]
 * 
 * The real magic, however, of the tree representation lies in how it is hashed
 * and linked to turn into a minimal DFA, explained later in the code. 
 */
re_error re__compile_charclass_add_rune_range(re__compile_charclass* char_comp, re_int32 root_ref, re__rune_range rune_range, re_int32 num_x_bits, re_int32 num_y_bits) {
    /* Mask of all ones for x-bits */
    re_rune x_mask = (1 << num_x_bits) - 1;
    /* 0b11111111 */
    re_rune byte_mask = 0xFF;
    /* Unicode top bits -- depending on y-bits, derive a mask to be used in
     * order to encode a byte.
     *
     * ASCII (no mask): u_mask = 0b00000000
     * 2-byte         : u_mask = 0b11000000
     * 3-byte         : u_mask = 0b11100000
     * 4-byte         : u_mask = 0b11110000
     * all cont. bytes: u_mask = 0b10000000 
     * 
     * This is pretty easy: just shift 0xFE by y_bits */
    /* 0xFE == 0b11111110 */
    re_rune u_mask = (0xFE << num_y_bits) & byte_mask;
    /* Reference to last created child */
    re_int32 child_ref = RE__COMPILE_CHARCLASS_TREE_NONE;
    /* Minimum/maximum bound's y-bits */
    re_rune y_min = rune_range.min >> num_x_bits;
    re_rune y_max = rune_range.max >> num_x_bits;
    /* Minimum/maximum upper encoded bytes */
    re_uint8 byte_min = (re_uint8)((y_min & byte_mask) | u_mask);
    re_uint8 byte_max = (re_uint8)((y_max & byte_mask) | u_mask);
    re_error err = RE_ERROR_NONE;
    /* num_y_bits can never be less than 7, as 7 is ASCII */
    RE_ASSERT(num_y_bits <= 7);
    if (num_x_bits == 0) {
        /* if x_bits is 0, then add a terminal node of just the byte range */
        /* This corresponds to the final byte in a UTF-8 sequence */
        /* Output:
         * ---[Ymin-Ymax] */
        re__byte_range br;
        br.min = byte_min;
        br.max = byte_max;
        if ((err = re__compile_charclass_touch_child(char_comp, root_ref, br, &child_ref))) {
            return err;
        }
    } else {
        /* Minimum/maximum bound's x_bits */
        re_rune x_min = rune_range.min & x_mask;
        re_rune x_max = rune_range.max & x_mask;
        /* Generate byte ranges (for this frame) and rune ranges (for child) */
        re__byte_range brs[3];
        re__rune_range rrs[3];
        /* Number of sub_trees (extents of 'brs' and 'rrs') */
        int num_sub_trees = 0;
        /* The next y_bits and x_bits for sub-ranges */
        re_int32 next_num_x_bits = num_x_bits - 6;
        re_int32 next_num_y_bits = 6;
        if (y_min == y_max || (x_min == 0 && x_max == x_mask)) {
            /* Range can be split into either a single byte followed by a range,
             * _or_ one range followed by another maximal range */
            /* Output:
             * ---[Ymin-Ymax]---{tree for [Xmin-Xmax]} */
            brs[0].min = byte_min, brs[0].max = byte_max;
            rrs[0].min = x_min,    rrs[0].max = x_max;
            num_sub_trees = 1;
        } else if (x_min == 0) {
            /* Range begins on zero, but has multiple starting bytes */
            /* Output:
             * ---[Ymin-(Ymax-1)]---{tree for [00-FF]}
             *           |       
             *      [Ymax-Ymax]----{tree for [00-Xmax]} */
            brs[0].min = byte_min, brs[0].max = byte_max - 1;
            rrs[0].min = 0,        rrs[0].max = x_mask;
            brs[1].min = byte_max, brs[1].max = byte_max;
            rrs[1].min = 0,        rrs[1].max = x_max;
            num_sub_trees = 2;
        } else if (x_max == x_mask) {
            /* Range ends on all ones, but has multiple starting bytes */
            /* Output:
             * -----[Ymin-Ymin]----{tree for [Xmin-FF]}
             *           |       
             *    [(Ymin+1)-Ymax]---{tree for [00-FF]} */
            brs[0].min = byte_min,     brs[0].max = byte_min;
            rrs[0].min = x_min,        rrs[0].max = x_mask;
            brs[1].min = byte_min + 1, brs[1].max = byte_max;
            rrs[1].min = 0,            rrs[1].max = x_mask;
            num_sub_trees = 2;
        } else {
            /* Range doesn't begin on all zeroes or all ones, and takes up more
             * than 2 different starting bytes */
            /* Output:
             * -------[Ymin-Ymin]-------{tree for [Xmin-FF]}
             *             |
             *    [(Ymin+1)-(Ymax-1)]----{tree for [00-FF]}
             *             |
             *        [Ymax-Ymax]-------{tree for [00-Xmax]} */
            brs[0].min = byte_min,     brs[0].max = byte_min;
            rrs[0].min = x_min,        rrs[0].max = x_mask;
            brs[1].min = byte_min + 1, brs[1].max = byte_max - 1;
            rrs[1].min = 0,            rrs[1].max = x_mask;
            brs[2].min = byte_max,     brs[2].max = byte_max;
            rrs[2].min = 0,            rrs[2].max = x_max;
            num_sub_trees = 3;
        }
        {
            /* Create all sub trees */
            int i;
            for (i = 0; i < num_sub_trees; i++) {
                if ((err = re__compile_charclass_touch_child(char_comp, root_ref, brs[i], &child_ref))) {
                    return err;
                }
                if ((err = re__compile_charclass_add_rune_range(char_comp, child_ref, rrs[i], next_num_x_bits, next_num_y_bits))) {
                    return err;
                } 
            }
        }
    }
    return err;
}

/* Check if two trees are equal, recursively. */
int re__compile_charclass_tree_equals(
    re__compile_charclass* char_comp,
    re__compile_charclass_tree* a,
    re__compile_charclass_tree* b) {
    re_int32 a_child_ref = a->first_child_ref;
    re_int32 b_child_ref = b->first_child_ref;
    /* While child references aren't none, check their equality. */
    while (a_child_ref != RE__COMPILE_CHARCLASS_TREE_NONE && b_child_ref != RE__COMPILE_CHARCLASS_TREE_NONE) {
        re__compile_charclass_tree* a_child = re__compile_charclass_tree_get(char_comp, a_child_ref);
        re__compile_charclass_tree* b_child = re__compile_charclass_tree_get(char_comp, a_child_ref);
        if (!re__compile_charclass_tree_equals(char_comp, a_child, b_child)) {
            return 0;
        }
        /* Go to next child refs. */
        a_child_ref = a_child->next_sibling_ref;
        b_child_ref = b_child->next_sibling_ref;
    }
    /* Child refs must become NONE at the same time. */
    if (a_child_ref != b_child_ref) { /* (a,b) != RE__COMPILE_CHARCLASS_TREE_NONE */
        return 0;
    }
    return re__byte_range_equals(a->byte_range, b->byte_range);
}

/* Merge two trees. */
void re__compile_charclass_merge_one(re__compile_charclass_tree* root, re__compile_charclass_tree* sibling) {
    root->next_sibling_ref = sibling->next_sibling_ref;
    root->byte_range = re__byte_range_merge(root->byte_range, sibling->byte_range);
}

/* Temporary structure that will be hashed, byte for byte. */
typedef struct re__compile_charclass_hash_temp {
    re__byte_range byte_range;
    re_uint32 down_hash;
    re_uint32 next_hash;
} re__compile_charclass_hash_temp;

/* Hash and merge everything within the tree, in place. */
/* This step is crucial to computing a minimized DFA.
 * In this function, we merge and then compute the hash of each node and its
 * children, in that order. The hashing is useful because the hashes of each
 * tree will be used later in memoizing the program locations for each tree, and
 * in addition adjacent trees will only be compared for merging if their hashes
 * are equal.
 * In a nutshell, the merging takes a tree that looks like this:
 * 
 * [55-66]---[77-88]
 *    |         |
 *    |      [99-AA]---[BB-CC]
 *    |
 * [67-69]---[77-88]
 *              |
 *           [99-AA]---[BB-CC]
 * 
 * And "merges" it into an equivalent tree that looks like this:
 * 
 * [55-69]---[77-88]
 *              |
 *           [99-AA]---[BB-CC]
 * 
 * This step ensures that we produce a minimal instruction encoding. */   
void re__compile_charclass_hash_tree(re__compile_charclass* char_comp, re__compile_charclass_tree* root) {
    /* Iterate through children backwards */
    re_int32 child_ref = root->last_child_ref;
    /* Child currently in process of being hashed */
    re__compile_charclass_tree* child;
    /* 1. Hash all children */
    while (child_ref != RE__COMPILE_CHARCLASS_TREE_NONE) {
        /* While there are children left, hash and merge them all */
        child = re__compile_charclass_tree_get(char_comp, child_ref);
        re__compile_charclass_hash_tree(char_comp, child);
        /* The previous call to hash_tree could have merged the last child into
         * its parent. Update the root's last_child_ref if the child is now the
         * last child. */
        if (child->next_sibling_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
            root->last_child_ref = child_ref;
        } 
        /* Iterate backwards */
        child_ref = child->prev_sibling_ref;
    }
    /* 2. Attempt to merge this root with its sibling */
    if (root->next_sibling_ref != RE__COMPILE_CHARCLASS_TREE_NONE) {
        re__compile_charclass_tree* sibling = re__compile_charclass_tree_get(char_comp, root->next_sibling_ref);
        if (re__byte_range_adjacent(root->byte_range, sibling->byte_range)) {
            /* root and sibling's byte ranges could potentially be merged */
            if (sibling->first_child_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
                if (root->first_child_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
                    /* Siblings are both terminals and can be merged */
                    re__compile_charclass_merge_one(root, sibling);
                }
            } else {
                if (root->first_child_ref != RE__COMPILE_CHARCLASS_TREE_NONE) {
                    re__compile_charclass_tree* root_child = re__compile_charclass_tree_get(char_comp, root->first_child_ref);
                    re__compile_charclass_tree* root_sibling_child = re__compile_charclass_tree_get(char_comp, sibling->first_child_ref);
                    if (root_child->hash == root_sibling_child->hash) {
                        if (re__compile_charclass_tree_equals(char_comp, root_child, root_sibling_child)) {
                            /* Siblings have identical children and can be merged */
                            re__compile_charclass_merge_one(root, sibling);
                        }
                    }
                }
            }
        }
    }
    /* 3. Compute hash of this root. */
    {
        /* The hash of this root is just the hash of the hashes of the first
         * child and the next sibling, as well as the hash of the byte range. */
        /* "Chaining" hashes in this way allows trees to be unique based on
         * their subsequent children and siblings, but also allows the trees of
         * children and siblings to be referenced by a unique hash themselves. */
        re__compile_charclass_hash_temp hash_obj;
        /* C89 does not guarantee struct zero-padding. This will throw off our
         * hash function if uninitialized properly. We explicitly zero out the
         * memory for this reason. */
        re__zero_mem(sizeof(re__compile_charclass_hash_temp), &hash_obj);
        hash_obj.byte_range = root->byte_range;
        /* Register hashes for next sibling and first child. */
        if (root->next_sibling_ref== RE__COMPILE_CHARCLASS_TREE_NONE) {
            /* I know nothing about cryptography. Whether or not this is an
             * actually good value is unknown. */
            hash_obj.next_hash = 0x0F0F0F0F;
        } else {
            re__compile_charclass_tree* next_sibling = re__compile_charclass_tree_get(char_comp, root->next_sibling_ref);
            hash_obj.next_hash = next_sibling->hash;
        }
        if (root->first_child_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
            hash_obj.down_hash = 0x0F0F0F0F;
        } else {
            re__compile_charclass_tree* first_child = re__compile_charclass_tree_get(char_comp, root->first_child_ref);
            hash_obj.down_hash = first_child->hash;
        }
        /* Murmurhash seemed good... */
        root->hash = re__murmurhash3_32((const re_uint8*)&hash_obj, sizeof(hash_obj));
    }
}

/* Split top-level rune ranges into ranges with unique UTF-8 byte lengths. */
/* This function takes a range like this:
 * [U+0-U+10FFFF]
 *
 * and produces trees for these ranges:
 * [U+00-U+7F]        -- ascii, 1 byte,  x_bits =  0, y_bits = 7
 * [U+80-U+7FF]       -- utf-8, 2 bytes, x_bits =  6, y_bits = 5
 * [U+800-U+FFFF]     -- utf-8, 3 bytes, x_bits = 12, y_bits = 4
 * [U+10000-U+10FFFF] -- utf-8, 4 bytes, x_bits = 18, y_bits = 3 */
re_error re__compile_charclass_split_rune_range(re__compile_charclass* char_comp, re__rune_range range) {
    re_error err = RE_ERROR_NONE;
    /* Starting values for y_bits and x_bits per byte length */
    static const re_int32 y_bits[4] = {7, 5, 4, 3};
    static const re_int32 x_bits[4] = {0, 6, 12, 18};
    /* Current byte length being processed */
    re_int32 byte_length;
    /* Minimum value to clamp for this byte length */
    re_rune min_value = 0;
    for (byte_length = 0; byte_length < 4; byte_length++) {
        /* Compute maximum value for this byte length, always equal to the
         * maximum codepoint that can be represented by byte_length bytes */
        re_rune max_value = (1 << (y_bits[byte_length] + x_bits[byte_length])) - 1;
        /* Clamp the current rune range to the given bounds */
        re__rune_range bounds;
        bounds.min = min_value;
        bounds.max = max_value;
        if (re__rune_range_intersects(range, bounds)) {
            /* If the current rune range contains a region within bounds, 
             * clamp it to bounds and then put it into the tree */
            re__rune_range clamped = re__rune_range_clamp(range, bounds);
            if ((err = re__compile_charclass_add_rune_range(char_comp, 0, clamped, x_bits[byte_length], y_bits[byte_length]))) {
                return err;
            }
        }
        /* Next byte length has a minimum value of the previous max + 1 */
        min_value = max_value + 1;
    }
    return err;
}


void re__compile_charclass_hash_entry_init(re__compile_charclass_hash_entry* hash_entry, re_int32 sparse_index, re_int32 root_ref, re__prog_loc prog_loc) {
    hash_entry->sparse_index = sparse_index;
    hash_entry->root_ref = root_ref;
    hash_entry->prog_loc = prog_loc;
    hash_entry->next = RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE;
}

/* Clear the program location cache. */
void re__compile_charclass_cache_clear(re__compile_charclass* char_comp) {
    /* We don't need to clear cache_sparse, because it can be used while
     * undefined. */
    re__compile_charclass_hash_entry_vec_clear(&char_comp->cache_dense);
}

/* Get a program location from the cache, if its given tree is in the cache. */
/* Returns RE__PROG_LOC_INVALID if not in the cache. */
re__prog_loc re__compile_charclass_cache_get(re__compile_charclass* char_comp, re_int32 root_ref) {
    re__compile_charclass_tree* root = re__compile_charclass_tree_get(char_comp, root_ref);
    /* Index into the sparse array based off of root's hash */
    re_int32 sparse_index = root->hash % RE__COMPILE_CHARCLASS_CACHE_SPARSE_SIZE;
    /* Final index in the dense array, if the tree is found in the cache */
    re_int32 dense_index = RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE;
    if (char_comp->cache_sparse == RE_NULL) {
        /* Cache is empty a.t.m., just return */
        return RE__PROG_LOC_INVALID;
    }
    dense_index = char_comp->cache_sparse[sparse_index];
    /* If dense_index_initial is more than the dense size, sparse_index is
     * nonsensical and the item isn't in the cache. */
    if (dense_index < (re_int32)re__compile_charclass_hash_entry_vec_size(&char_comp->cache_dense) && dense_index >= 0) {
        /* Load the hash entry at dense_index_initial */
        re__compile_charclass_hash_entry* hash_entry_prev = re__compile_charclass_hash_entry_vec_getref(&char_comp->cache_dense, (re_size)dense_index);
        /* If hash_entry_prev->sparse_index doesn't point back to sparse_index,
         * sparse_index is nonsensical. */
        if (hash_entry_prev->sparse_index == sparse_index) {
            while (1) {
                re__compile_charclass_tree* root_cache = re__compile_charclass_tree_vec_getref(&char_comp->tree, (re_size)hash_entry_prev->root_ref);
                if (root_cache->hash == root->hash) {
                    if (re__compile_charclass_tree_equals(char_comp, root_cache, root)) {
                        /* If both hashes and then their trees are equal, we
                         * have already compiled this tree and can return its
                         * program location. */
                        return hash_entry_prev->prog_loc;
                    }
                }
                if (hash_entry_prev->next == RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE) {
                    /* We've exhausted this hash bucket, and didn't find a
                     * matching tree. */
                    break;
                } else {
                    /* There are still more entries in this hash bucket to
                     * check. */
                    dense_index = hash_entry_prev->next;
                    hash_entry_prev = re__compile_charclass_hash_entry_vec_getref(&char_comp->cache_dense, (re_size)dense_index);
                }
            }
            return RE__PROG_LOC_INVALID;
        } else {
            return RE__PROG_LOC_INVALID;
        }
    } else {
        return RE__PROG_LOC_INVALID;
    }
}
/* Add a program location to the cache, after it has been compiled. */
re_error re__compile_charclass_cache_add(re__compile_charclass* char_comp, re_int32 root_ref, re__prog_loc prog_loc) {
    re_error err = RE_ERROR_NONE;
    re__compile_charclass_tree* root = re__compile_charclass_tree_get(char_comp, root_ref);
    /* These variables have the same meaning as they do in cache_get. */
    re_int32 sparse_index = root->hash % RE__COMPILE_CHARCLASS_CACHE_SPARSE_SIZE;
    re_int32 dense_index = RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE;
    re__compile_charclass_hash_entry* hash_entry_prev = RE_NULL;
    int requires_link;
    if (char_comp->cache_sparse == RE_NULL) {
        /* Sparse cache is empty, so let's allocate it on-demand. */
        char_comp->cache_sparse = (re_int32*)RE_MALLOC(sizeof(re_int32) * RE__COMPILE_CHARCLASS_CACHE_SPARSE_SIZE);
        if (char_comp->cache_sparse == RE_NULL) {
            return RE_ERROR_NOMEM;
        }
    }
    dense_index = char_comp->cache_sparse[sparse_index];
    /* Look up the element in the cache, see re__compile_charclass_cache_get */
    if (dense_index < (re_int32)re__compile_charclass_hash_entry_vec_size(&char_comp->cache_dense) && dense_index >= 0) {
        hash_entry_prev = re__compile_charclass_hash_entry_vec_getref(&char_comp->cache_dense, (re_size)dense_index);
        if (hash_entry_prev->sparse_index == sparse_index) {
            while (1) {
                re__compile_charclass_tree* root_cache = re__compile_charclass_tree_get(char_comp, hash_entry_prev->root_ref);
                if (root_cache->hash == root->hash) {
                    if (re__compile_charclass_tree_equals(char_comp, root_cache, root)) {
                        /* We found the item in the cache? This should never
                         * happen. To ensure optimality, we should only ever add
                         * items to the cache once. */
                        requires_link = 0;
                        RE__ASSERT_UNREACHED();
                        break;
                    }
                }
                if (hash_entry_prev->next == RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE) {
                    requires_link = 1;
                    dense_index = RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE;
                    break;
                }
                /* Keep iterating. */
                dense_index = hash_entry_prev->next;
                hash_entry_prev = re__compile_charclass_hash_entry_vec_getref(&char_comp->cache_dense, (re_size)dense_index);
            }
        } else {
            requires_link = 0;
            dense_index = RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE;
        }
    } else {
        requires_link = 0;
        dense_index = RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE;
    }
    if (dense_index == RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE) {
        /* Item isn't in cache. */
        /* The location for the new hash_entry. */
        re_int32 dense_index_final = (re_int32)re__compile_charclass_hash_entry_vec_size(&char_comp->cache_dense);
        re__compile_charclass_hash_entry new_entry;
        re__compile_charclass_hash_entry_init(&new_entry, sparse_index, root_ref, prog_loc);
        if (!requires_link) {
            /* No linking required, insert a new sparse entry */
            char_comp->cache_sparse[sparse_index] = dense_index_final;
            if ((err = re__compile_charclass_hash_entry_vec_push(&char_comp->cache_dense, new_entry))) {
                return err;
            }
            /* hash_entry_prev is invalid */
        } else {
            /* Linking required */
            hash_entry_prev->next = dense_index_final;
            if ((err = re__compile_charclass_hash_entry_vec_push(&char_comp->cache_dense, new_entry))) {
                return err;
            }
            /* hash_entry_prev is invalid */
        }
    } else {
        /* in cache already */
        RE__ASSERT_UNREACHED();
    }
    return err;
}

/* Generate the program for a particular tree. */
/* root should not be in the cache. */
re_error re__compile_charclass_generate_prog(re__compile_charclass* char_comp, re__prog* prog, re__compile_charclass_tree* root, re__prog_loc* out_pc, re__compile_patches* patches) {
    re_int32 child_ref = root->first_child_ref;
    /* Starting program location for this root. */
    re__prog_loc start_pc = re__prog_size(prog);
    /* Keeps track of the previous split location, if there is one. */
    re__prog_loc split_from = RE__PROG_LOC_INVALID;
    re_error err = RE_ERROR_NONE;
    while (child_ref != RE__COMPILE_CHARCLASS_TREE_NONE) {
        re__compile_charclass_tree* child;
        /* Program counter in the cache, if it was found */
        re__prog_loc cache_pc;
        /* Previous compiled instruction, gets linked to the next one */
        re__prog_inst* inst_from;
        /* New instruction template */
        re__prog_inst new_inst;
        /* Location in the program of inst_from */
        re__prog_loc link_from;
        /* Location to link inst_from to */
        re__prog_loc link_to;
        /* Current byte range that is getting compiled */
        re__byte_range byte_range;
        if ((cache_pc = re__compile_charclass_cache_get(char_comp, child_ref)) != RE__PROG_LOC_INVALID) {
            /* Tree is in cache! */
            /* if split_from is invalid, that means that root is in the cache.
             * This shouldn't be possible. */
            RE_ASSERT(split_from != RE__PROG_LOC_INVALID);
            inst_from = re__prog_get(prog, split_from);
            re__prog_inst_set_split_secondary(inst_from, cache_pc);
            break;
        }
        /* Tree is not in cache */
        child = re__compile_charclass_tree_get(char_comp, child_ref);
        if (split_from != RE__PROG_LOC_INVALID) {
            /* If we previously compiled a split instruction, link it to the
             * next compiled instruction. */
            inst_from = re__prog_get(prog, split_from);
            re__prog_inst_set_split_secondary(inst_from, re__prog_size(prog));
        }
        if (child->next_sibling_ref != RE__COMPILE_CHARCLASS_TREE_NONE) {
            /* If there is another sibling that will get compiled, add a SPLIT
             * instruction before the child. This split instruction will get
             * linked to the next child (see above lines) */
            split_from = re__prog_size(prog);
            re__prog_inst_init_split(&new_inst, re__prog_size(prog) + 1, RE__PROG_LOC_INVALID);
            if ((err = re__prog_add(prog, new_inst))) {
                return err;
            }
        }
        /* Location of byte range/byte instruction */
        link_from = re__prog_size(prog);
        byte_range = child->byte_range;
        /* Compile either a byterange or byte instruction depending on range */
        if (byte_range.min == byte_range.max) {
            re__prog_inst_init_byte(&new_inst, byte_range.min);
        } else {
            re__prog_inst_init_byte_range(&new_inst, byte_range);
        }
        if ((err = re__prog_add(prog, new_inst))) {
            return err;
        }
        if (child->first_child_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
            /* Child is a terminal node. Add an outgoing patch. */
            re__compile_patches_append(patches, prog, link_from, 0);
        } else {
            /* Check if child's children are in the cache. */
            /* link_to becomes the cached program location if child's children
             * are in the cache, otherwise it is RE__PROG_LOC_INVALID. */
            if ((link_to = re__compile_charclass_cache_get(char_comp, child->first_child_ref)) == RE__PROG_LOC_INVALID) {
                /* Child's children are not in the cache. Generate them. */
                if ((err = re__compile_charclass_generate_prog(char_comp, prog, child, &link_to, patches))) {
                    return err;
                }
            }
            /* link_to always points to cached pc or next pc */
            inst_from = re__prog_get(prog, link_from);
            re__prog_inst_set_primary(inst_from, link_to);
        }
        /* Register child in the cache. */
        if ((err = re__compile_charclass_cache_add(char_comp, child_ref, link_from))) {
            return err;
        }
        child_ref = child->next_sibling_ref;
    }
    *out_pc = start_pc;
    return RE_ERROR_NONE;
}

/* Compile a single character class. */
re_error re__compile_charclass_gen(re__compile_charclass* char_comp, const re__charclass* charclass, re__prog* prog, re__compile_patches* patches_out) {
    re_error err = RE_ERROR_NONE;
    re_size i;
    const re__rune_range* ranges = re__charclass_get_ranges(charclass);
    re__compile_charclass_tree initial_root;
    re__byte_range initial_range;
    initial_range.min = 0;
    initial_range.max = 0xFF;
    /* These are all idempotent. Cool word, right? I just learned it */
    re__compile_charclass_tree_vec_clear(&char_comp->tree);
    re__compile_charclass_cache_clear(char_comp);
    /* Add the initial root (0-FF node) */
    re__compile_charclass_tree_init(&initial_root, initial_range);
    if ((err = re__compile_charclass_tree_vec_push(&char_comp->tree, initial_root))) {
        return err;
    }
    /* Iterate through charclass' ranges and add them all to the tree. */
    for (i = 0; i < re__charclass_get_num_ranges(charclass); i++) {
        re__rune_range r = ranges[i];
        if ((err = re__compile_charclass_split_rune_range(char_comp, r))) {
            return err;
        }
    }
    {
        /* Do the actual compiling. */
        re__prog_loc out_pc;
        re__compile_charclass_tree* initial_tree = re__compile_charclass_tree_vec_getref(&char_comp->tree, 0);
        /* Hash and merge the tree */
        re__compile_charclass_hash_tree(char_comp, initial_tree);
        /* Generate the tree's program */
        if ((err = re__compile_charclass_generate_prog(char_comp, prog, initial_tree, &out_pc, patches_out))) {
            return err;
        }
        /* Done!!! all that effort for just a few instructions. */
    }
    return err;
}

void re__compile_charclass_init(re__compile_charclass* char_comp) {
    re__compile_charclass_tree_vec_init(&char_comp->tree);
    char_comp->cache_sparse = RE_NULL;
    re__compile_charclass_hash_entry_vec_init(&char_comp->cache_dense);
}

void re__compile_charclass_destroy(re__compile_charclass* char_comp) {
    re__compile_charclass_hash_entry_vec_destroy(&char_comp->cache_dense);
    if (char_comp->cache_sparse != RE_NULL) {
        RE_FREE(char_comp->cache_sparse);
    }
    re__compile_charclass_tree_vec_destroy(&char_comp->tree);
}

#if RE_DEBUG

void re__compile_charclass_dump(re__compile_charclass* char_comp, re_int32 tree_idx, re_int32 indent) {
    re_int32 i;
    re_int32 node = tree_idx;
    if (indent == 0) {
        printf("Charclass Compiler %p:\n", (void*)char_comp);
    }
    if (node == RE__COMPILE_CHARCLASS_TREE_NONE) {
        for (i = 0; i < indent + 1; i++) {
            printf("  ");
        }
        printf("<term>\n");
    } else {
        while (node != RE__COMPILE_CHARCLASS_TREE_NONE) {
            re__compile_charclass_tree* tree = re__compile_charclass_tree_vec_getref(&char_comp->tree, (re_size)node);
            for (i = 0; i < indent + 1; i++) {
                printf("  ");
            }
            printf("%04X | [%02X-%02X] hash=%08X\n", node, tree->byte_range.min, tree->byte_range.max, tree->hash);
            re__compile_charclass_dump(char_comp, tree->first_child_ref, indent+1);
            node = tree->next_sibling_ref;
        }
    }
    if (indent == 0) {
        printf("  Cache:\n");
        if (char_comp->cache_sparse == RE_NULL) {
            printf("    Empty cache!\n");
        } else {
            for (i = 0; i < RE__COMPILE_CHARCLASS_CACHE_SPARSE_SIZE; i++) {
                re_int32 j;
                re_int32 dense_loc = char_comp->cache_sparse[i];
                re__compile_charclass_hash_entry* hash_entry;
                if (dense_loc >= (re_int32)re__compile_charclass_hash_entry_vec_size(&char_comp->cache_dense)) {
                    continue;
                }
                hash_entry = re__compile_charclass_hash_entry_vec_getref(&char_comp->cache_dense, (re_size)dense_loc);
                if (hash_entry->sparse_index != i) {
                    continue;
                }
                printf("    Sparse index: %i\n", i);
                j = 0;
                printf("      [%i] root_ref=%04X prog_loc=%04X\n", j, hash_entry->root_ref, hash_entry->prog_loc);
                j++;
                while (hash_entry->next != RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE) {
                    hash_entry = re__compile_charclass_hash_entry_vec_getref(&char_comp->cache_dense, (re_size)hash_entry->next);
                    printf("      [%i] root_ref=%04X prog_loc=%04X\n", j, hash_entry->root_ref, hash_entry->prog_loc);
                    j++;
                }
            }
        }
    }
}

#endif

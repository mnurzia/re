#include "re_internal.h"

MN__VEC_IMPL_FUNC(re__compile_charclass_tree, init)
MN__VEC_IMPL_FUNC(re__compile_charclass_tree, destroy)
MN__VEC_IMPL_FUNC(re__compile_charclass_tree, clear)
MN__VEC_IMPL_FUNC(re__compile_charclass_tree, size)
MN__VEC_IMPL_FUNC(re__compile_charclass_tree, getref)
MN__VEC_IMPL_FUNC(re__compile_charclass_tree, push)

MN__VEC_IMPL_FUNC(re__compile_charclass_hash_entry, init)
MN__VEC_IMPL_FUNC(re__compile_charclass_hash_entry, destroy)
MN__VEC_IMPL_FUNC(re__compile_charclass_hash_entry, size)
MN__VEC_IMPL_FUNC(re__compile_charclass_hash_entry, getref)
MN__VEC_IMPL_FUNC(re__compile_charclass_hash_entry, push)
MN__VEC_IMPL_FUNC(re__compile_charclass_hash_entry, clear)

void re__compile_charclass_tree_init(re__compile_charclass_tree* tree, re__byte_range byte_range) {
    tree->byte_range = byte_range;
    tree->sibling_ref = RE__COMPILE_CHARCLASS_TREE_NONE;
    tree->child_ref = RE__COMPILE_CHARCLASS_TREE_NONE;
    tree->aux = 0;
}

/* Get a pointer to a node in the tree, given its index. */
MN_INTERNAL re__compile_charclass_tree* re__compile_charclass_tree_get(re__compile_charclass* char_comp, mn_uint32 tree_ref) {
    MN_ASSERT(tree_ref != RE__COMPILE_CHARCLASS_TREE_NONE);
    MN_ASSERT(tree_ref < (mn_uint32)re__compile_charclass_tree_vec_size(&char_comp->tree));
    return re__compile_charclass_tree_vec_getref(&char_comp->tree, (mn_size)tree_ref);
}

re_error re__compile_charclass_new_node(re__compile_charclass* char_comp, mn_uint32 parent_ref, re__byte_range byte_range, mn_uint32* out_new_node_ref, int use_reverse_tree) {
    re_error err = RE_ERROR_NONE;
    re__compile_charclass_tree new_node;
    mn_uint32 prev_sibling_ref;
    *out_new_node_ref = (mn_uint32)re__compile_charclass_tree_vec_size(&char_comp->tree);
    if (parent_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
        /* adding to root */
        mn_uint32* root_ref;
        mn_uint32* root_last_child_ref;
        if (!use_reverse_tree) {
            root_ref = &char_comp->root_ref;
            root_last_child_ref = &char_comp->root_last_child_ref;
        } else {
            root_ref = &char_comp->rev_root_ref;
            root_last_child_ref = &char_comp->rev_root_last_child_ref;
        }
        if (*root_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
            /* root hasn't been found yet */
            /* Insert a dummy node as node 0 */
            re__byte_range zero_range;
            zero_range.min = 0;
            zero_range.max = 0;
            /* account for the dummy node */
            *out_new_node_ref += 1;
            *root_ref = *out_new_node_ref;
            *root_last_child_ref = *out_new_node_ref;
            /* create and push later, good practice for avoiding use-after-free */
            re__compile_charclass_tree_init(&new_node, zero_range);
            if ((err = re__compile_charclass_tree_vec_push(&char_comp->tree, new_node))) {
                return err;
            }
            prev_sibling_ref = RE__COMPILE_CHARCLASS_TREE_NONE;
        } else {
            /* root has been found, append to last child */
            prev_sibling_ref = *root_last_child_ref;
            *root_last_child_ref = *out_new_node_ref;
        }
    } else {
        re__compile_charclass_tree* parent;
        parent = re__compile_charclass_tree_get(char_comp, parent_ref);
        prev_sibling_ref = parent->child_ref;
        parent->child_ref = *out_new_node_ref;
    }
    re__compile_charclass_tree_init(&new_node, byte_range);
    new_node.sibling_ref = prev_sibling_ref;
    if ((err = re__compile_charclass_tree_vec_push(&char_comp->tree, new_node))) {
        return err;
    }
    return err;
}

re_error re__compile_charclass_add_rune_range(re__compile_charclass* char_comp, mn_uint32 parent_ref, re__rune_range rune_range, mn_uint32 num_x_bits, mn_uint32 num_y_bits) {
    re_rune x_mask = (1 << num_x_bits) - 1;
    re_rune byte_mask = 0xFF;
    re_rune u_mask = (0xFE << num_y_bits) & byte_mask;
    mn_uint32 child_ref = RE__COMPILE_CHARCLASS_TREE_NONE;
    re_rune y_min = rune_range.min >> num_x_bits;
    re_rune y_max = rune_range.max >> num_x_bits;
    mn_uint8 byte_min = (mn_uint8)((y_min & byte_mask) | u_mask);
    mn_uint8 byte_max = (mn_uint8)((y_max & byte_mask) | u_mask);
    re_error err = RE_ERROR_NONE;
    MN_ASSERT(num_y_bits <= 7);
    if (num_x_bits == 0) {
        /* terminal nodes can never intersect, so we're covered here */
        re__byte_range br;
        br.min = byte_min;
        br.max = byte_max;
        if ((err = re__compile_charclass_new_node(char_comp, parent_ref, br, &child_ref, 0))) {
            return err;
        }
    } else {
        re_rune x_min = rune_range.min & x_mask;
        re_rune x_max = rune_range.max & x_mask;
        /* Generate byte ranges (for this frame) and rune ranges (for child) */
        re__byte_range brs[3];
        re__rune_range rrs[3];
        /* Number of sub_trees (extents of 'brs' and 'rrs') */
        int num_sub_trees = 0;
        /* The next y_bits and x_bits for sub-ranges */
        mn_uint32 next_num_x_bits = num_x_bits - 6;
        mn_uint32 next_num_y_bits = 6;
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
                /* First check if the last child intersects and compute the
                 * intersection. */
                mn_uint32 prev_sibling_ref;
                if (parent_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
                    prev_sibling_ref = char_comp->root_last_child_ref;
                } else {
                    re__compile_charclass_tree* parent = re__compile_charclass_tree_get(char_comp, parent_ref);
                    prev_sibling_ref = parent->child_ref;
                }
                if (prev_sibling_ref != RE__COMPILE_CHARCLASS_TREE_NONE) {
                    /* might have to split */
                    re__compile_charclass_tree* prev_sibling = re__compile_charclass_tree_get(char_comp, prev_sibling_ref);
                    if (re__byte_range_intersects(prev_sibling->byte_range, brs[i])) {
                        re__byte_range intersection = re__byte_range_intersection(prev_sibling->byte_range, brs[i]);
                        re__byte_range rest;
                        rest.min = intersection.max + 1;
                        rest.max = brs[i].max;
                        if ((err = re__compile_charclass_add_rune_range(char_comp, prev_sibling_ref, rrs[i], next_num_x_bits, next_num_y_bits))) {
                            return err;
                        } 
                        if ((err = re__compile_charclass_new_node(char_comp, parent_ref, rest, &child_ref, 0))) {
                            return err;
                        }
                        if ((err = re__compile_charclass_add_rune_range(char_comp, child_ref, rrs[i], next_num_x_bits, next_num_y_bits))) {
                            return err;
                        } 
                    } else {
                        if ((err = re__compile_charclass_new_node(char_comp, parent_ref, brs[i], &child_ref, 0))) {
                        return err;
                        }
                        if ((err = re__compile_charclass_add_rune_range(char_comp, child_ref, rrs[i], next_num_x_bits, next_num_y_bits))) {
                            return err;
                        } 
                    }
                } else {
                    if ((err = re__compile_charclass_new_node(char_comp, parent_ref, brs[i], &child_ref, 0))) {
                        return err;
                    }
                    if ((err = re__compile_charclass_add_rune_range(char_comp, child_ref, rrs[i], next_num_x_bits, next_num_y_bits))) {
                        return err;
                    } 
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
    mn_uint32 a_child_ref = a->child_ref;
    mn_uint32 b_child_ref = b->child_ref;
    /* While child references aren't none, check their equality. */
    while (a_child_ref != RE__COMPILE_CHARCLASS_TREE_NONE && b_child_ref != RE__COMPILE_CHARCLASS_TREE_NONE) {
        re__compile_charclass_tree* a_child = re__compile_charclass_tree_get(char_comp, a_child_ref);
        re__compile_charclass_tree* b_child = re__compile_charclass_tree_get(char_comp, a_child_ref);
        if (!re__compile_charclass_tree_equals(char_comp, a_child, b_child)) {
            return 0;
        }
        /* Go to next child refs. */
        a_child_ref = a_child->sibling_ref;
        b_child_ref = b_child->sibling_ref;
    }
    /* Child refs must become NONE at the same time. */
    if (a_child_ref != b_child_ref) { /* (a,b) != RE__COMPILE_CHARCLASS_TREE_NONE */
        return 0;
    }
    return re__byte_range_equals(a->byte_range, b->byte_range);
}

/* Temporary structure that will be hashed, byte for byte. */
typedef struct re__compile_charclass_hash_temp {
    re__byte_range byte_range;
    mn_uint32 down_hash;
    mn_uint32 next_hash;
} re__compile_charclass_hash_temp;

void re__compile_charclass_merge_one(re__compile_charclass_tree* child, re__compile_charclass_tree* sibling) {
    child->sibling_ref = sibling->sibling_ref;
    child->byte_range = re__byte_range_merge(child->byte_range, sibling->byte_range);
}

void re__compile_charclass_hash_tree(re__compile_charclass* char_comp, mn_uint32 parent_ref) {
    re__compile_charclass_tree* child;
    re__compile_charclass_tree* sibling;
    re__compile_charclass_tree* parent;
    mn_uint32 child_ref, sibling_ref, next_child_ref;
    if (parent_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
        /* this is root */
        child_ref = char_comp->root_last_child_ref;
    } else {
        parent = re__compile_charclass_tree_get(char_comp, parent_ref);
        child_ref = parent->child_ref;
    }
    sibling_ref = RE__COMPILE_CHARCLASS_TREE_NONE;
    while (child_ref != RE__COMPILE_CHARCLASS_TREE_NONE) {
        child = re__compile_charclass_tree_get(char_comp, child_ref);
        next_child_ref = child->sibling_ref;
        child->sibling_ref = sibling_ref;
        re__compile_charclass_hash_tree(char_comp, child_ref);
        if (sibling_ref != RE__COMPILE_CHARCLASS_TREE_NONE) {
            if (re__byte_range_adjacent(child->byte_range, sibling->byte_range)) {
                if (sibling->child_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
                    if (child->child_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
                        re__compile_charclass_merge_one(child, sibling);
                    }
                } else {
                    if (child->child_ref != RE__COMPILE_CHARCLASS_TREE_NONE) {
                        re__compile_charclass_tree* child_child = re__compile_charclass_tree_get(char_comp, child->child_ref);
                        re__compile_charclass_tree* sibling_child = re__compile_charclass_tree_get(char_comp, sibling->child_ref);
                        if (child_child->aux == sibling_child->aux) {
                            if (re__compile_charclass_tree_equals(char_comp, child_child, sibling_child)) {
                                /* Siblings have identical children and can be merged */
                                re__compile_charclass_merge_one(child, sibling);
                            }
                        }
                    }
                }
            }
        }
        {
            re__compile_charclass_hash_temp hash_obj;
            /* C89 does not guarantee struct zero-padding. This will throw off our
             * hash function if uninitialized properly. We explicitly zero out the
             * memory for this reason. */
            mn__memset((void*)&hash_obj, 0, sizeof(re__compile_charclass_hash_temp));
            hash_obj.byte_range = child->byte_range;
            /* Register hashes for next sibling and first child. */
            if (child->sibling_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
                /* I know nothing about cryptography. Whether or not this is an
                 * actually good value is unknown. */
                hash_obj.next_hash = 0x0F0F0F0F;
            } else {
                hash_obj.next_hash = sibling->aux;
            }
            if (child->child_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
                hash_obj.down_hash = 0x0F0F0F0F;
            } else {
                re__compile_charclass_tree* child_child = re__compile_charclass_tree_get(char_comp, child->child_ref);
                hash_obj.down_hash = child_child->aux;
            }
            /* Murmurhash seemed good... */
            child->aux = mn__murmurhash3_32(0, (const mn_uint8*)&hash_obj, sizeof(hash_obj));
        }
        sibling_ref = child_ref;
        sibling = child;
        child_ref = next_child_ref;
    }
    if (parent_ref != RE__COMPILE_CHARCLASS_TREE_NONE) {
        parent->child_ref = sibling_ref;
    }
}

re_error re__compile_charclass_split_rune_range(re__compile_charclass* char_comp, re__rune_range range) {
    re_error err = RE_ERROR_NONE;
    static const mn_uint32 y_bits[4] = {7, 5, 4, 3};
    static const mn_uint32 x_bits[4] = {0, 6, 12, 18};
    mn_uint32 byte_length;
    re_rune min_value = 0;
    for (byte_length = 0; byte_length < 4; byte_length++) {
        re_rune max_value = (1 << (y_bits[byte_length] + x_bits[byte_length])) - 1;
        re__rune_range bounds;
        bounds.min = min_value;
        bounds.max = max_value;
        if (re__rune_range_intersects(range, bounds)) {
            re__rune_range clamped = re__rune_range_clamp(range, bounds);
            if ((err = re__compile_charclass_add_rune_range(char_comp, RE__COMPILE_CHARCLASS_TREE_NONE, clamped, x_bits[byte_length], y_bits[byte_length]))) {
                return err;
            }
        }
        min_value = max_value + 1;
    }
    return err;
}

void re__compile_charclass_hash_entry_init(re__compile_charclass_hash_entry* hash_entry, mn_int32 sparse_index, mn_uint32 root_ref, re__prog_loc prog_loc) {
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
re__prog_loc re__compile_charclass_cache_get(re__compile_charclass* char_comp, mn_uint32 root_ref) {
    re__compile_charclass_tree* root;
    /* Index into the sparse array based off of root's hash */
    mn_int32 sparse_index;
    /* Final index in the dense array, if the tree is found in the cache */
    mn_int32 dense_index;
    if (root_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
        return RE__PROG_LOC_INVALID;
    }
    root = re__compile_charclass_tree_get(char_comp, root_ref);
    sparse_index = root->aux % RE__COMPILE_CHARCLASS_CACHE_SPARSE_SIZE;
    dense_index = RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE;
    if (char_comp->cache_sparse == MN_NULL) {
        /* Cache is empty a.t.m., just return */
        return RE__PROG_LOC_INVALID;
    }
    dense_index = char_comp->cache_sparse[sparse_index];
    /* If dense_index_initial is more than the dense size, sparse_index is
     * nonsensical and the item isn't in the cache. */
    if (dense_index < (mn_int32)re__compile_charclass_hash_entry_vec_size(&char_comp->cache_dense) && dense_index >= 0) {
        /* Load the hash entry at dense_index_initial */
        re__compile_charclass_hash_entry* hash_entry_prev = re__compile_charclass_hash_entry_vec_getref(&char_comp->cache_dense, (mn_size)dense_index);
        /* If hash_entry_prev->sparse_index doesn't point back to sparse_index,
         * sparse_index is nonsensical. */
        if (hash_entry_prev->sparse_index == sparse_index) {
            while (1) {
                re__compile_charclass_tree* root_cache = re__compile_charclass_tree_vec_getref(&char_comp->tree, (mn_size)hash_entry_prev->root_ref);
                if (root_cache->aux == root->aux) {
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
                    hash_entry_prev = re__compile_charclass_hash_entry_vec_getref(&char_comp->cache_dense, (mn_size)dense_index);
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
re_error re__compile_charclass_cache_add(re__compile_charclass* char_comp, mn_uint32 root_ref, re__prog_loc prog_loc) {
    re_error err = RE_ERROR_NONE;
    re__compile_charclass_tree* root = re__compile_charclass_tree_get(char_comp, root_ref);
    /* These variables have the same meaning as they do in cache_get. */
    mn_int32 sparse_index = root->aux % RE__COMPILE_CHARCLASS_CACHE_SPARSE_SIZE;
    mn_int32 dense_index = RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE;
    re__compile_charclass_hash_entry* hash_entry_prev = MN_NULL;
    int requires_link;
    if (char_comp->cache_sparse == MN_NULL) {
        /* Sparse cache is empty, so let's allocate it on-demand. */
        char_comp->cache_sparse = (mn_int32*)MN_MALLOC(sizeof(mn_int32) * RE__COMPILE_CHARCLASS_CACHE_SPARSE_SIZE);
        if (char_comp->cache_sparse == MN_NULL) {
            return RE_ERROR_NOMEM;
        }
    }
    dense_index = char_comp->cache_sparse[sparse_index];
    /* Look up the element in the cache, see re__compile_charclass_cache_get */
    if (dense_index < (mn_int32)re__compile_charclass_hash_entry_vec_size(&char_comp->cache_dense) && dense_index >= 0) {
        hash_entry_prev = re__compile_charclass_hash_entry_vec_getref(&char_comp->cache_dense, (mn_size)dense_index);
        if (hash_entry_prev->sparse_index == sparse_index) {
            while (1) {
                re__compile_charclass_tree* root_cache = re__compile_charclass_tree_get(char_comp, hash_entry_prev->root_ref);
                if (root_cache->aux == root->aux) {
                    if (re__compile_charclass_tree_equals(char_comp, root_cache, root)) {
                        /* We found the item in the cache? This should never
                         * happen. To ensure optimality, we should only ever add
                         * items to the cache once. */
                        requires_link = 0;
                        MN__ASSERT_UNREACHED();
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
                hash_entry_prev = re__compile_charclass_hash_entry_vec_getref(&char_comp->cache_dense, (mn_size)dense_index);
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
        mn_int32 dense_index_final = (mn_int32)re__compile_charclass_hash_entry_vec_size(&char_comp->cache_dense);
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
        MN__ASSERT_UNREACHED();
    }
    return err;
}

/* Generate the program for a particular tree. */
/* root should not be in the cache. */
re_error re__compile_charclass_generate_prog(re__compile_charclass* char_comp, re__prog* prog, mn_uint32 node_ref, re__prog_loc* out_pc, re__compile_patches* patches) {
    /* Starting program location for this root. */
    re__prog_loc start_pc = re__prog_size(prog);
    /* Keeps track of the previous split location, if there is one. */
    re__prog_loc split_from = RE__PROG_LOC_INVALID;
    re_error err = RE_ERROR_NONE;
    while (node_ref != RE__COMPILE_CHARCLASS_TREE_NONE) {
        re__compile_charclass_tree* node;
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
        if ((cache_pc = re__compile_charclass_cache_get(char_comp, node_ref)) != RE__PROG_LOC_INVALID) {
            /* Tree is in cache! */
            /* if split_from is invalid, that means that root is in the cache.
             * This shouldn't be possible. */
            MN_ASSERT(split_from != RE__PROG_LOC_INVALID);
            inst_from = re__prog_get(prog, split_from);
            re__prog_inst_set_split_secondary(inst_from, cache_pc);
            break;
        }
        /* Tree is not in cache */
        node = re__compile_charclass_tree_get(char_comp, node_ref);
        if (split_from != RE__PROG_LOC_INVALID) {
            /* If we previously compiled a split instruction, link it to the
             * next compiled instruction. */
            inst_from = re__prog_get(prog, split_from);
            re__prog_inst_set_split_secondary(inst_from, re__prog_size(prog));
        }
        if (node->sibling_ref != RE__COMPILE_CHARCLASS_TREE_NONE) {
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
        byte_range = node->byte_range;
        /* Compile either a byterange or byte instruction depending on range */
        if (byte_range.min == byte_range.max) {
            re__prog_inst_init_byte(&new_inst, byte_range.min);
        } else {
            re__prog_inst_init_byte_range(&new_inst, byte_range);
        }
        if ((err = re__prog_add(prog, new_inst))) {
            return err;
        }
        if (node->child_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
            /* Child is a terminal node. Add an outgoing patch. */
            re__compile_patches_append(patches, prog, link_from, 0);
        } else {
            /* Check if child's children are in the cache. */
            /* link_to becomes the cached program location if child's children
             * are in the cache, otherwise it is RE__PROG_LOC_INVALID. */
            if ((link_to = re__compile_charclass_cache_get(char_comp, node->child_ref)) == RE__PROG_LOC_INVALID) {
                /* Child's children are not in the cache. Generate them. */
                if ((err = re__compile_charclass_generate_prog(char_comp, prog, node->child_ref, &link_to, patches))) {
                    return err;
                }
            }
            /* link_to always points to cached pc or next pc */
            inst_from = re__prog_get(prog, link_from);
            re__prog_inst_set_primary(inst_from, link_to);
        }
        /* Register child in the cache. */
        if ((err = re__compile_charclass_cache_add(char_comp, node_ref, link_from))) {
            return err;
        }
        node_ref = node->sibling_ref;
    }
    *out_pc = start_pc;
    return RE_ERROR_NONE;
}

/* Clear the aux field on all nodes in the tree. */
void re__compile_charclass_clear_aux(re__compile_charclass* char_comp, mn_uint32 parent_ref) {
    mn_uint32 child_ref;
    if (parent_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
        child_ref = char_comp->root_ref;
    } else {
        re__compile_charclass_tree* parent = re__compile_charclass_tree_get(char_comp, parent_ref);
        child_ref = parent->child_ref;
    }
    while (child_ref != RE__COMPILE_CHARCLASS_TREE_NONE) {
        re__compile_charclass_tree* child = re__compile_charclass_tree_get(char_comp, child_ref);
        child->aux = RE__COMPILE_CHARCLASS_TREE_NONE;
        re__compile_charclass_clear_aux(char_comp, child_ref);
        child_ref = child->sibling_ref;
    }
}

re_error re__compile_charclass_transpose(re__compile_charclass* char_comp, mn_uint32 parent_ref) {
    mn_uint32 child_ref;
    re_error err = RE_ERROR_NONE;
    if (parent_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
        child_ref = char_comp->root_ref;
    } else {
        re__compile_charclass_tree* parent = re__compile_charclass_tree_get(char_comp, parent_ref);
        child_ref = parent->child_ref;
    }
    while (child_ref != RE__COMPILE_CHARCLASS_TREE_NONE) {
        re__compile_charclass_tree* child = re__compile_charclass_tree_get(char_comp, child_ref);
        mn_uint32 new_ref;
        if (child->child_ref == RE__COMPILE_CHARCLASS_TREE_NONE) {
            /* terminal, add to reverse root */
            if ((err = re__compile_charclass_new_node(char_comp, RE__COMPILE_CHARCLASS_TREE_NONE, child->byte_range, &new_ref, 1))) {
                return err;
            }
            child->aux = new_ref;
        } else {
            re__compile_charclass_tree* child_child;
            child_child = re__compile_charclass_tree_get(char_comp, child->child_ref);
            if (child_child->aux == RE__COMPILE_CHARCLASS_TREE_NONE) {
                if ((err = re__compile_charclass_transpose(char_comp, child_ref))) {
                    return err;
                }
            }
            /* ALERT!!! child may have changed!!! */
            child = re__compile_charclass_tree_get(char_comp, child_ref);
            child_child = re__compile_charclass_tree_get(char_comp, child->child_ref);
            if ((err = re__compile_charclass_new_node(char_comp, child_child->aux, child->byte_range, &new_ref, 1))) {
                return err;
            }
            child->aux = new_ref;
        }
    }
    return err;
}

/* Compile a single character class. */
re_error re__compile_charclass_gen(re__compile_charclass* char_comp, const re__charclass* charclass, re__prog* prog, re__compile_patches* patches_out, int reversed) {
    re_error err = RE_ERROR_NONE;
    mn_size i;
    const re__rune_range* ranges = re__charclass_get_ranges(charclass);
    /* These are all idempotent. Cool word, right? I just learned it */
    re__compile_charclass_tree_vec_clear(&char_comp->tree);
    re__compile_charclass_cache_clear(char_comp);
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
        /* Hash and merge the tree */
        re__compile_charclass_hash_tree(char_comp, RE__COMPILE_CHARCLASS_TREE_NONE);
        /* Generate the tree's program */
        if (!reversed) {
            if ((err = re__compile_charclass_generate_prog(char_comp, prog, char_comp->root_ref, &out_pc, patches_out))) {
                return err;
            }
        } else {
            re__compile_charclass_clear_aux(char_comp, RE__COMPILE_CHARCLASS_TREE_NONE);
            if ((err = re__compile_charclass_transpose(char_comp, RE__COMPILE_CHARCLASS_TREE_NONE))) {
                return err;
            }
            if ((err = re__compile_charclass_generate_prog(char_comp, prog, char_comp->rev_root_ref, &out_pc, patches_out))) {
                return err;
            }
        }
        /* Done!!! all that effort for just a few instructions. */
    }
    return err;
}

void re__compile_charclass_init(re__compile_charclass* char_comp) {
    re__compile_charclass_tree_vec_init(&char_comp->tree);
    char_comp->root_ref = RE__COMPILE_CHARCLASS_TREE_NONE;
    char_comp->root_last_child_ref = RE__COMPILE_CHARCLASS_TREE_NONE;
    char_comp->cache_sparse = MN_NULL;
    char_comp->rev_root_ref = RE__COMPILE_CHARCLASS_TREE_NONE;
    char_comp->rev_root_last_child_ref = RE__COMPILE_CHARCLASS_TREE_NONE;
    re__compile_charclass_hash_entry_vec_init(&char_comp->cache_dense);
}

void re__compile_charclass_destroy(re__compile_charclass* char_comp) {
    re__compile_charclass_hash_entry_vec_destroy(&char_comp->cache_dense);
    if (char_comp->cache_sparse) {
        MN_FREE(char_comp->cache_sparse);
    }
    re__compile_charclass_tree_vec_destroy(&char_comp->tree);
}


#if RE_DEBUG

void re__compile_charclass_dump(re__compile_charclass* char_comp, mn_uint32 tree_idx, mn_int32 indent) {
    mn_int32 i;
    mn_uint32 node = tree_idx;
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
            re__compile_charclass_tree* tree = re__compile_charclass_tree_vec_getref(&char_comp->tree, (mn_size)node);
            for (i = 0; i < indent + 1; i++) {
                printf("  ");
            }
            printf("%04X | [%02X-%02X] hash=%08X\n", node, tree->byte_range.min, tree->byte_range.max, tree->aux);
            re__compile_charclass_dump(char_comp, tree->child_ref, indent+1);
            node = tree->sibling_ref;
        }
    }
    if (indent == 0) {
        printf("  Cache:\n");
        if (char_comp->cache_sparse == MN_NULL) {
            printf("    Empty cache!\n");
        } else {
            for (i = 0; i < RE__COMPILE_CHARCLASS_CACHE_SPARSE_SIZE; i++) {
                mn_int32 j;
                mn_int32 dense_loc = char_comp->cache_sparse[i];
                re__compile_charclass_hash_entry* hash_entry;
                if (dense_loc >= (mn_int32)re__compile_charclass_hash_entry_vec_size(&char_comp->cache_dense)) {
                    continue;
                }
                hash_entry = re__compile_charclass_hash_entry_vec_getref(&char_comp->cache_dense, (mn_size)dense_loc);
                if (hash_entry->sparse_index != i) {
                    continue;
                }
                printf("    Sparse index: %i\n", i);
                j = 0;
                printf("      [%i] root_ref=%04X prog_loc=%04X\n", j, hash_entry->root_ref, hash_entry->prog_loc);
                j++;
                while (hash_entry->next != RE__COMPILE_CHARCLASS_HASH_ENTRY_NONE) {
                    hash_entry = re__compile_charclass_hash_entry_vec_getref(&char_comp->cache_dense, (mn_size)hash_entry->next);
                    printf("      [%i] root_ref=%04X prog_loc=%04X\n", j, hash_entry->root_ref, hash_entry->prog_loc);
                    j++;
                }
            }
        }
    }
}

#endif

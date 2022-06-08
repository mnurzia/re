#include "re_internal.h"

void re__parse_new_frame_init(
    re__parse_new_frame* frame, mn_int32 ast_root_ref,
    re__ast_group_flags group_flags)
{
  frame->ast_root_ref = ast_root_ref;
  frame->ast_prev_child_ref = RE__AST_NONE;
  frame->group_flags = group_flags;
}

MN__VEC_IMPL_FUNC(re__parse_new_frame, init)
MN__VEC_IMPL_FUNC(re__parse_new_frame, destroy)
MN__VEC_IMPL_FUNC(re__parse_new_frame, size)
MN__VEC_IMPL_FUNC(re__parse_new_frame, getref)
MN__VEC_IMPL_FUNC(re__parse_new_frame, push)
MN__VEC_IMPL_FUNC(re__parse_new_frame, pop)

void re__parse_new_init(re__parse_new* parse, re* reg)
{
  parse->reg = reg;
  mn__str_view_init_null(&parse->str);
  parse->str_pos = 0;
  re__parse_new_frame_vec_init(&parse->frames);
  re__charclass_builder_init(&parse->charclass_builder);
}

void re__parse_new_destroy(re__parse_new* parse)
{
  re__parse_new_frame_vec_destroy(&parse->frames);
  re__charclass_builder_destroy(&parse->charclass_builder);
}

re__parse_new_frame* re__parse_new_get_frame(re__parse_new* parse)
{
  MN_ASSERT(re__parse_frame_vec_size(&parse->frames));
  return re__parse_new_frame_vec_getref(
      &parse->frames, re__parse_new_frame_vec_size(&parse->frames) - 1);
}

re__ast_type re__parse_new_get_frame_type(re__parse_new* parse)
{
  re__parse_new_frame* frame = re__parse_new_get_frame(parse);
  if (frame->ast_root_ref == RE__AST_NONE) {
    return RE__AST_TYPE_NONE;
  } else {
    return re__ast_root_get(&parse->reg->data->ast_root, frame->ast_root_ref)
        ->type;
  }
}

re_error re__parse_new_push_frame(
    re__parse_new* parse, mn_int32 ast_root_ref,
    re__ast_group_flags group_flags)
{
  re__parse_new_frame new_frame;
  re__parse_new_frame_init(&new_frame, ast_root_ref, group_flags);
  return re__parse_new_frame_vec_push(&parse->frames, new_frame);
}

int re__parse_new_frame_is_empty(re__parse_new* parse)
{
  return re__parse_new_get_frame(parse)->ast_prev_child_ref != RE__AST_NONE;
}

/* Convenience function to set the error to a literal string. */
MN_INTERNAL re_error
re__parse_new_error(re__parse_new* parse, const char* err_chars)
{
  mn__str err_str;
  re_error err = mn__str_init_s(&err_str, (const mn_char*)err_chars);
  if (err) {
    mn__str_destroy(&err_str);
    return err;
  }
  if ((err = re__set_error_str(parse->reg, &err_str))) {
    mn__str_destroy(&err_str);
    return err;
  }
  return RE_ERROR_PARSE;
}

#define RE__PARSE_NEW_UTF8_ACCEPT 0
#define RE__PARSE_NEW_UTF8_REJECT 12

static const mn_uint8 re__parse_new_utf8_tt[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  9,  9,  9,  9,  9,  9,  9,  9,
    9,  9,  9,  9,  9,  9,  9,  9,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,
    7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,
    7,  7,  8,  8,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
    2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  10, 3,  3,  3,
    3,  3,  3,  3,  3,  3,  3,  3,  3,  4,  3,  3,  11, 6,  6,  6,  5,  8,  8,
    8,  8,  8,  8,  8,  8,  8,  8,  8,

    0,  12, 24, 36, 60, 96, 84, 12, 12, 12, 48, 72, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 0,  12, 12, 12, 12, 12, 0,  12, 0,  12, 12, 12, 24,
    12, 12, 12, 12, 12, 24, 12, 24, 12, 12, 12, 12, 12, 12, 12, 12, 12, 24, 12,
    12, 12, 12, 12, 24, 12, 12, 12, 12, 12, 12, 12, 24, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 36, 12, 36, 12, 12, 12, 36, 12, 12, 12, 12, 12, 36, 12, 36, 12,
    12, 12, 36, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
};

MN_INTERNAL mn_uint32
re__parse_new_utf8_decode(mn_uint32* state, mn_uint32* codep, mn_uint32 byte)
{
  mn_uint32 type = re__parse_new_utf8_tt[byte];

  *codep =
      (*state != 0) ? (byte & 0x3fu) | (*codep << 6) : (0xff >> type) & (byte);

  *state = re__parse_new_utf8_tt[256 + *state + type];
  return *state;
}

re_error re__parse_new_next_char(re__parse_new* parse, re_rune* ch)
{
  mn_uint32 codep = 0;
  mn_uint32 state = 0;
  while (1) {
    if (parse->str_pos == mn__str_view_size(&parse->str)) {
      if (state == RE__PARSE_NEW_UTF8_REJECT) {
        parse->str_pos++;
        return re__parse_new_error(parse, "invalid UTF-8 byte");
      } else {
        parse->str_pos++;
        *ch = -1;
        return RE_ERROR_NONE;
      }
    } else {
      mn_uint8 in_byte =
          (mn_uint8)(mn__str_view_get_data(&parse->str)[parse->str_pos]);
      if (!re__parse_utf8_decode(&state, &codep, in_byte)) {
        parse->str_pos++;
        *ch = (re_rune)codep;
        return RE_ERROR_NONE;
      } else if (state == RE__PARSE_NEW_UTF8_REJECT) {
        parse->str_pos++;
        return re__parse_new_error(parse, "invalid UTF-8 byte");
      } else {
        parse->str_pos++;
      }
    }
  }
  return RE_ERROR_NONE;
}

#define RE__PARSE_NEW_EOF (-1)

/* Add a node after the previous child. */
MN_INTERNAL re_error re__parse_new_push_node(re__parse_new* parse, re__ast ast)
{
  re_error err = RE_ERROR_NONE;
  re__parse_new_frame* frame = re__parse_new_get_frame(parse);
  mn_int32 new_ref = RE__AST_NONE;
  if ((err = re__ast_root_add_child(
           &parse->reg->data->ast_root, frame->ast_root_ref, ast, &new_ref))) {
    return err;
  }
  frame->ast_prev_child_ref = new_ref;
  return err;
}

/* Insert a node right before the previous child, making the previous child the
 * new node's parent. */
MN_INTERNAL re_error
re__parse_new_wrap_node(re__parse_new* parse, re__ast outer)
{
  re_error err = RE_ERROR_NONE;
  re__parse_new_frame* frame = re__parse_new_get_frame(parse);
  mn_int32 new_outer_ref = RE__AST_NONE;
  if ((err = re__ast_root_add_wrap(
           &parse->reg->data->ast_root, frame->ast_root_ref,
           frame->ast_prev_child_ref, outer, &new_outer_ref))) {
    return err;
  }
  frame->ast_prev_child_ref = new_outer_ref;
  return err;
}

MN_INTERNAL re_error re__parse_new_opt_fuse_concat(
    re__parse_new* parse, re__ast* next, int* did_fuse)
{
  re__ast* prev;
  mn_uint32 prev_child_ref;
  re__ast_type t_prev, t_next;
  re_error err = RE_ERROR_NONE;
  MN_ASSERT(!re__parse_new_frame_is_empty(parse));
  prev_child_ref = re__parse_new_get_frame(parse)->ast_prev_child_ref;
  prev = re__ast_root_get(&parse->reg->data->ast_root, prev_child_ref);
  t_prev = prev->type;
  t_next = next->type;
  *did_fuse = 0;
  if (t_prev == RE__AST_TYPE_RUNE) {
    if (t_next == RE__AST_TYPE_RUNE) {
      /* Opportunity to fuse two runes into a string */
      mn__str new_str;
      re__ast new_ast;
      mn_char rune_bytes[16]; /* 16 oughta be good */
      mn_int32 new_str_ref;
      int rune_bytes_ptr = 0;
      rune_bytes_ptr += re__compile_gen_utf8(
          re__ast_get_rune(prev), (mn_uint8*)rune_bytes + rune_bytes_ptr);
      rune_bytes_ptr += re__compile_gen_utf8(
          re__ast_get_rune(next), (mn_uint8*)rune_bytes + rune_bytes_ptr);
      if ((err =
               mn__str_init_n(&new_str, rune_bytes, (mn_size)rune_bytes_ptr))) {
        return err;
      }
      if ((err = re__ast_root_add_str(
               &parse->reg->data->ast_root, new_str, &new_str_ref))) {
        mn__str_destroy(&new_str);
        return err;
      }
      re__ast_init_str(&new_ast, new_str_ref);
      re__ast_root_replace(
          &parse->reg->data->ast_root, prev_child_ref, new_ast);
      re__ast_destroy(next);
      *did_fuse = 1;
    }
  } else if (t_prev == RE__AST_TYPE_STR) {
    if (t_next == RE__AST_TYPE_RUNE) {
      /* Opportunity to add a rune to a string */
      mn__str* old_str;
      mn_char rune_bytes[16];
      mn_int32 old_str_ref;
      int rune_bytes_ptr = 0;
      rune_bytes_ptr += re__compile_gen_utf8(
          re__ast_get_rune(next), (mn_uint8*)rune_bytes + rune_bytes_ptr);
      old_str_ref = re__ast_get_str_ref(prev);
      old_str = re__ast_root_get_str(&parse->reg->data->ast_root, old_str_ref);
      if ((err = mn__str_cat_n(old_str, rune_bytes, (mn_size)rune_bytes_ptr))) {
        return err;
      }
      re__ast_destroy(next);
      *did_fuse = 1;
    }
  }
  return err;
}

/* Add a new node to the end of the stack, while maintaining these invariants:
 * - Group nodes can only hold one immediate node.
 * - Alt nodes can only hold one immediate node per branch.
 * - Concat nodes can hold an infinite number of nodes.
 *
 * To maintain these, when we have to add a second child to an alt/group node,
 * we convert it into a concatenation of the first and second children. */
MN_INTERNAL re_error
re__parse_new_link_node(re__parse_new* parse, re__ast new_ast)
{
  re__ast_type frame_type = RE__AST_TYPE_NONE;
  re_error err = RE_ERROR_NONE;
  /* Firstly, attempt an optimization by fusing the nodes, if possible. */
  if (!re__parse_new_frame_is_empty(parse)) {
    int did_fuse;
    if ((err = re__parse_opt_fuse_concat(parse, &new_ast, &did_fuse))) {
      return err;
    }
    if (did_fuse) {
      /* We successfully fused the node, so there is no need to create
       * a new concatenation. */
      return err;
    }
  }
  /* If we are here, then the node couldn't be optimized away and we have to
   * push it. */
  frame_type = re__parse_new_get_frame_type(parse);
  /* Weird control flow -- it's the only way I figured out how to do the
   * assertion below. */
  if (frame_type == RE__AST_TYPE_GROUP || frame_type == RE__AST_TYPE_ALT ||
      frame_type == RE__AST_TYPE_NONE) {
    if (re__parse_new_frame_is_empty(parse)) {
      /* Push node, fallthrough */
    } else {
      re__ast new_concat;
      mn_int32 old_inner = re__parse_new_get_frame(parse)->ast_prev_child_ref;
      mn_int32 new_outer;
      re__parse_new_frame new_frame;
      /* Wrap the last child(ren) in a concatenation */
      re__ast_init_concat(&new_concat);
      if ((err = re__parse_new_wrap_node(parse, new_concat))) {
        return err;
      }
      /* new_concat is moved */
      /* Push a new frame */
      if ((err = re__parse_new_push_frame(
               parse, re__parse_new_get_frame(parse)->ast_prev_child_ref,
               re__parse_new_get_frame(parse)->group_flags))) {
        return err;
      }
    }
  } else if (frame_type == RE__AST_TYPE_CONCAT) {
    /* Push node, fallthrough */
  } else {
    /* Due to operator precedence, we should never arrive here. */
    MN__ASSERT_UNREACHED();
  }
  /* Add the new node to the frame. */
  if ((err = re__parse_push_node(parse, new_ast))) {
    return err;
  }
  return err;
}

/* Called after '(' */
re_error re__parse_new_group_begin(re__parse_new* parse)
{
  re_error err = RE_ERROR_NONE;
  re_rune ch;
  mn_size begin_name_pos = 0;
  mn_size end_name_pos = 0;
  if ((err = re__parse_new_next_char(parse, &ch))) {
    return err;
  }
  if (ch == RE__PARSE_NEW_EOF) {
    return re__parse_new_error(parse, "unmatched '('");
  } else if (ch == '?') {
    re__ast_group_flags group_flags =
        re__parse_new_get_frame(parse)->group_flags;
    int set_bit = 1;
    /* (? | Start of group flags/name */
    if ((err = re__parse_new_next_char(parse, &ch))) {
      return err;
    }
    while (1) {
      if (ch == ')') {
        /* (?) | Go back to ground without creating a group, retain flags */
        re__parse_new_get_frame(parse)->group_flags = group_flags;
        return;
      } else if (ch == '-') {
        /* (?- | Negate remaining flags */
        set_bit = 0;
      } else if (ch == ':') {
        /* (?: | Non-matching group, also signals end of flags */
        if (set_bit) {
          group_flags |= RE__AST_GROUP_FLAG_NONMATCHING;
        } else {
          group_flags &= ~RE__AST_GROUP_FLAG_NONMATCHING;
        }
        break;
      } else if (ch == 'U') {
        /* (?U | Ungreedy mode: *+? operators have priority swapped */
        if (set_bit) {
          group_flags |= RE__AST_GROUP_FLAG_UNGREEDY;
        } else {
          group_flags &= ~RE__AST_GROUP_FLAG_UNGREEDY;
        }
      } else if (ch == 'i') {
        /* (?i: Case insensitive matching */
        if (set_bit) {
          group_flags |= RE__AST_GROUP_FLAG_CASE_INSENSITIVE;
        } else {
          group_flags &= ~RE__AST_GROUP_FLAG_CASE_INSENSITIVE;
        }
      } else if (ch == 'm') {
        /* (?m: Multi-line mode: ^$ match line boundaries */
        if (set_bit) {
          group_flags |= RE__AST_GROUP_FLAG_MULTILINE;
        } else {
          group_flags &= ~RE__AST_GROUP_FLAG_MULTILINE;
        }
      } else if (ch == 's') {
        /* (?s: Stream (?) mode: . matches \n */
        if (set_bit) {
          group_flags |= RE__AST_GROUP_FLAG_DOT_NEWLINE;
        } else {
          group_flags &= ~RE__AST_GROUP_FLAG_DOT_NEWLINE;
        }
      } else if (ch == '<' || ch == 'P') {
        if (ch == 'P') {
          /* (?P | Alternative way to start group name */
          if ((err = re__parse_new_next_char(parse, &ch))) {
            return err;
          }
          if (ch != '<') {
            /* Handles EOF */
            return re__parse_new_error(
                parse, "expected '<' to begin group name");
          }
        }
        /* (?< | Start of group name */
        begin_name_pos = parse->str_pos;
        end_name_pos = begin_name_pos;
        if ((err = re__parse_new_next_char(parse, &ch))) {
          return err;
        }
        if (ch == '>') {
          return re__parse_new_error(parse, "cannot create empty group name");
        }
        while (1) {
          if ((err = re__parse_new_next_char(parse, &ch))) {
            return err;
          }
          if (ch == '>') {
            end_name_pos = parse->str_pos;
            group_flags |= RE__AST_GROUP_FLAG_NAMED;
            break;
          } else if (ch == RE__PARSE_NEW_EOF) {
            return re__parse_new_error(
                parse, "expected '>' to close group name");
          }
        }
        /* create group */
        break;
      } else {
        /* Handles EOF */
        return re__parse_error(
            parse, "expected one of '-', ':', '<', 'P', 'U', 'i', 'm', 's' for "
                   "group flags or name");
      }
    }
    {
      mn_uint32 new_group_idx;
      mn__str_view group_name;
      re__ast new_group;
      if (group_flags & RE__AST_GROUP_FLAG_NAMED) {
        if (group_flags & RE__AST_GROUP_FLAG_NONMATCHING) {
          return re__parse_error(
              parse, "cannot have non-matching group with a name");
        } else {
          mn__str_view_init_n(
              &group_name, mn__str_view_get_data(&parse->str) + begin_name_pos,
              begin_name_pos - end_name_pos);
          new_group_idx =
              re__ast_root_get_num_groups(&parse->reg->data->ast_root);
          if ((err = re__ast_root_add_group(
                   &parse->reg->data->ast_root, group_name))) {
            return err;
          }
        }
      } else if (!(group_flags & RE__AST_GROUP_FLAG_NONMATCHING)) {
        mn__str_view_init_null(&group_name);
        new_group_idx =
            re__ast_root_get_num_groups(&parse->reg->data->ast_root);
        if ((err = re__ast_root_add_group(
                 &parse->reg->data->ast_root, group_name))) {
          return err;
        }
      }
      re__ast_init_group(&new_group, new_group_idx, group_flags);
      if ((err = re__parse_new_link_node(&parse, new_group))) {
        return err;
      }
      /* Strip NAMED flag */
      group_flags &= ~RE__AST_GROUP_FLAG_NAMED;
      if ((err = re__parse_new_push_frame(
               parse, re__parse_new_get_frame(parse)->ast_prev_child_ref,
               group_flags))) {
        return err;
      }
    }
  }
}

/* Called after '(' */
re_error re__parse_new_group_end(re__parse_new* parse)
{
  re_error err = RE_ERROR_NONE;
  while (1) {
    re__ast_type peek_type = re__parse_new_get_frame_type(parse);
    if (peek_type == RE__AST_TYPE_NONE) {
      /* If we are at the absolute bottom of the stack, there was no opening
       * parentheses to begin with. */
      return re__parse_error(parse, "unmatched ')'");
    }
    /* Now pop the current frame */
    re__parse_new_frame_pop(parse);
    /* If we just popped a group, finish */
    if (peek_type == RE__AST_TYPE_GROUP) {
      break;
    }
  }
  return RE_ERROR_NONE;
}

/* Act on a '|' character. If this is the first alteration, we wrap the current
 * group node in an ALT. If not, we simply add to the previous ALT. */
MN_INTERNAL re_error re__parse_new_alt(re__parse_new* parse)
{
  re__ast_type peek_type;
  re_error err = RE_ERROR_NONE;
  while (1) {
    peek_type = re__parse_new_get_frame_type(parse);
    if (peek_type == RE__AST_TYPE_CONCAT) {
      /* Pop all concatenations, alt takes priority */
      re__parse_new_frame_pop(parse);
    } else if (
        peek_type == RE__AST_TYPE_GROUP || peek_type == RE__AST_TYPE_NONE) {
      /* This is the initial alteration: "a|" or "(a|" */
      /* Note: the group in question could be the base frame. */
      /* In any case, we shim an ALT node in before the previous child. */
      /* Since GROUP nodes are defined to have a maximum of one child, we
       * don't need to mess around with the amount of children for either
       * node. */
      re__ast new_alt;
      mn_int32 new_alt_ref;
      re__ast_init_alt(&new_alt);
      if (re__parse_new_frame_is_empty(parse)) {
        /* Empty frame -- null alteration. */
        re__ast new_concat;
        re__ast_init_concat(&new_concat);
        if ((err = re__parse_new_link_node(parse, new_concat))) {
          return err;
        }
      }
      if ((err = re__parse_new_wrap_node(parse, new_alt))) {
        return err;
      }
      /* Push a new ALT frame */
      if ((err = re__parse_new_push_frame(
               parse, re__parse_new_get_frame(parse)->ast_prev_child_ref,
               re__parse_new_get_frame(parse)->group_flags))) {
        return err;
      }
      return err;
    } else if (peek_type == RE__AST_TYPE_ALT) {
      /* Third+ part of the alteration: "a|b|" or "(a|b|" */
      /* Indicate that there are no new children (this is the beginning
       * of the second+ part of an alteration) */
      re__parse_new_get_frame(parse)->ast_prev_child_ref = RE__AST_NONE;
      return err;
    }
  }
}

/* Create a new assert */
MN_INTERNAL re_error
re__parse_new_create_assert(re__parse_new* parse, re__assert_type assert_type)
{
  re__ast new_node;
  re__ast_init_assert(&new_node, assert_type);
  return re__parse_new_link_node(parse, new_node);
}

MN_INTERNAL re_error re__parse_new_create_star(re__parse_new* parse)
{
  re__ast new_star;
  if (re__parse_new_frame_is_empty(parse)) {
    return re__parse_error(parse, "cannot use '*' operator with nothing");
  }
  re__ast_init_quantifier(&new_star, 0, RE__AST_QUANTIFIER_INFINITY);
  re__ast_set_quantifier_greediness(
      &new_star, !!!(re__parse_new_get_frame(parse)->group_flags &
                     RE__AST_GROUP_FLAG_UNGREEDY));
  return re__parse_new_wrap_node(parse, new_star);
}

MN_INTERNAL re_error re__parse_new_create_question(re__parse_new* parse)
{
  re__ast new_star;
  if (re__parse_new_frame_is_empty(parse)) {
    return re__parse_error(parse, "cannot use '?' operator with nothing");
  }
  re__ast_init_quantifier(&new_star, 0, RE__AST_QUANTIFIER_INFINITY);
  re__ast_set_quantifier_greediness(
      &new_star, !!!(re__parse_new_get_frame(parse)->group_flags &
                     RE__AST_GROUP_FLAG_UNGREEDY));
  return re__parse_new_wrap_node(parse, new_star);
}

MN_INTERNAL re_error re__parse_new_create_plus(re__parse_new* parse)
{
  re__ast new_star;
  if (re__parse_new_frame_is_empty(parse)) {
    return re__parse_error(parse, "cannot use '?' operator with nothing");
  }
  re__ast_init_quantifier(&new_star, 0, RE__AST_QUANTIFIER_INFINITY);
  re__ast_set_quantifier_greediness(
      &new_star, !!!(re__parse_new_get_frame(parse)->group_flags &
                     RE__AST_GROUP_FLAG_UNGREEDY));
  return re__parse_new_wrap_node(parse, new_star);
}

MN_INTERNAL void re__parse_new_swap_greedy(re__parse_new* parse)
{
  re__ast* quant;
  /* Cannot make nothing ungreedy */
  MN_ASSERT(!re__parse_frame_is_empty(parse));
  quant = re__ast_root_get(
      &parse->reg->data->ast_root,
      re__parse_new_get_frame(parse)->ast_prev_child_ref);
  /* Must be a quantifier */
  MN_ASSERT(quant->type == RE__AST_TYPE_QUANTIFIER);
  re__ast_set_quantifier_greediness(
      quant, !re__ast_get_quantifier_greediness(quant));
}

MN_INTERNAL re_error re__parse_new_create_any_char(re__parse_new* parse)
{
  re__ast new_dot;
  re__ast_init_any_char(&new_dot);
  return re__parse_new_link_node(parse, new_dot);
}

MN_INTERNAL re_error re__parse_new_create_any_byte(re__parse_new* parse)
{
  re__ast new_c;
  re__ast_init_any_byte(&new_c);
  return re__parse_new_link_node(parse, new_c);
}

/* Ingest a single rune. */
MN_INTERNAL re_error re__parse_new_create_rune(re__parse* parse, re_rune ch)
{
  re__ast new_rune;
  re__ast_init_rune(&new_rune, ch);
  return re__parse_new_link_node(parse, new_rune);
}

/* Create an ASCII charclass. */
MN_INTERNAL re_error re__parse_new_create_charclass_ascii(
    re__parse_new* parse, re__charclass_ascii_type ascii_cc, int inverted)
{
  re_error err = RE_ERROR_NONE;
  re__ast new_node;
  mn_int32 new_class_ref;
  re__charclass new_class;
  if ((err = re__charclass_init_from_class(&new_class, ascii_cc, inverted))) {
    return err;
  }
  if ((err = re__ast_root_add_charclass(
           &parse->reg->data->ast_root, new_class, &new_class_ref))) {
    re__charclass_destroy(&new_class);
    return err;
  }
  /* ast_root now owns new_class */
  re__ast_init_charclass(&new_node, new_class_ref);
  return re__parse_new_link_node(parse, new_node);
}

/* Insert an ASCII charclass into the builder. */
MN_INTERNAL re_error re__parse_new_insert_charclass_ascii(
    re__parse_new* parse, re__charclass_ascii_type ascii_cc, int inverted)
{
  re_error err = RE_ERROR_NONE;
  re__charclass new_class;
  if ((err = re__charclass_init_from_class(&new_class, ascii_cc, inverted))) {
    return err;
  }
  if ((err = re__charclass_builder_insert_class(
           &parse->charclass_builder, &new_class))) {
    return err;
  }
  re__charclass_destroy(&new_class);
  return err;
}

MN_INTERNAL int re__parse_new_oct(re_rune ch)
{
  switch (ch) {
  case '0':
    return 0;
  case '1':
    return 1;
  case '2':
    return 2;
  case '3':
    return 3;
  case '4':
    return 4;
  case '5':
    return 5;
  case '6':
    return 6;
  case '7':
    return 7;
  default:
    return -1;
  }
}

MN_INTERNAL int re__parse_new_hex(re_rune ch)
{
  switch (ch) {
  case '0':
    return 0;
  case '1':
    return 1;
  case '2':
    return 2;
  case '3':
    return 3;
  case '4':
    return 4;
  case '5':
    return 5;
  case '6':
    return 6;
  case '7':
    return 7;
  case '8':
    return 8;
  case '9':
    return 9;
  case 'A':
    return 10;
  case 'B':
    return 11;
  case 'C':
    return 12;
  case 'D':
    return 13;
  case 'E':
    return 14;
  case 'F':
    return 15;
  case 'a':
    return 10;
  case 'b':
    return 11;
  case 'c':
    return 12;
  case 'd':
    return 13;
  case 'e':
    return 14;
  case 'f':
    return 15;
  default:
    return -1;
  }
}

/* Parse an escape character */
/* Called after '\' */
MN_INTERNAL re_error re__parse_new_escape(
    re__parse_new* parse, re_rune* out_char, int accept_classes,
    int within_charclass)
{
  re_error err = RE_ERROR_NONE;
  re_rune ch;
  mn_uint32 accum = 0;
  mn_uint32 accum2 = 0;
  mn_size saved_pos;
  int i = 0;
  if ((err = re__parse_new_next_char(parse, &ch))) {
    return err;
  }
  saved_pos = parse->str_pos;
  *out_char = RE__PARSE_NEW_EOF;
  if (ch == RE__PARSE_NEW_EOF) {
    return re__parse_new_error(parse, "unfinished escape sequence");
  } else if ((accum = re__parse_new_oct(ch)) != -1) {
    /* \[0-7] | Octal digits */
    for (i = 0; i < 2; i++) {
      if ((err = re__parse_new_next_char(parse, &ch))) {
        return err;
      }
      if ((accum2 = re__parse_new_oct(ch)) != -1) {
        /* \[0-7][0-7] | Found second/third octal digit */
        accum *= 8;
        accum += accum2;
        saved_pos = parse->str_pos;
      } else {
        /* \[0-7]<*> | Did not find second/third octal digit */
        *out_char = accum;
        parse->str_pos = saved_pos;
        break;
      }
    }
  } else if (ch == 'A') {
    /* \A | Absolute text start */
    if (!within_charclass) {
      if ((err = re__parse_new_create_assert(
               parse, RE__ASSERT_TYPE_TEXT_START_ABSOLUTE))) {
        return err;
      }
    } else {
      return re__parse_new_error(
          parse, "cannot use \\A inside character class");
    }
  } else if (ch == 'B') {
    /* \B | Not a word boundary */
    if (!within_charclass) {
      if ((err =
               re__parse_new_create_assert(parse, RE__ASSERT_TYPE_WORD_NOT))) {
        return err;
      }
    } else {
      return re__parse_new_error(
          parse, "cannot use \\B inside character class");
    }
  } else if (ch == 'C') {
    /* \C: Any *byte* (NOT any char) */
    if (!within_charclass) {
      if ((err = re__parse_new_create_any_byte(parse))) {
        return err;
      }
    } else {
      return re__parse_new_error(
          parse, "cannot use \\C inside character class");
    }
  } else if (ch == 'D') {
    /* \D: Non-digit characters */
    if (accept_classes) {
      if (!within_charclass) {
        if ((err = re__parse_new_create_charclass_ascii(
                 parse, RE__CHARCLASS_ASCII_TYPE_DIGIT, 1))) {
          return err;
        }
      } else {
        if ((err = re__parse_new_insert_charclass_ascii(
                 parse, RE__CHARCLASS_ASCII_TYPE_DIGIT, 1))) {
          return err;
        }
      }
    } else {
      return re__parse_new_error(
          parse, "cannot use \\D as ending character for range");
    }
  } else if (ch == 'E') {
    /* \E | Invalid here */
    return re__parse_new_error(parse, "\\E can only be used from within \\Q");
  } else if (ch == 'P') {
    /* \P | Inverted Unicode character class */
    return re__parse_new_error(parse, "unimplemented");
  } else if (ch == 'Q') {
    /* \Q | Quote begin */
    if (!within_charclass) {
      int is_escape = 0;
      while (1) {
        saved_pos = parse->str_pos;
        if ((err = re__parse_new_next_char(parse, &ch))) {
          return err;
        }
        if (!is_escape) {
          if (ch == RE__PARSE_NEW_EOF) {
            /* \Q<*><EOF> | End, having added all previous chars */
            parse->str_pos = saved_pos;
            break;
          } else if (ch == '\\') {
            /* \Q\ | Escape within '\Q' */
            is_escape = 1;
          } else {
            /* \Q<*> | Add character */
            if ((err = re__parse_new_create_rune(parse, ch))) {
              return err;
            }
          }
        } else {
          if (ch == RE__PARSE_NEW_EOF) {
            /* \Q\<EOF> | Error */
            return re__parse_new_error(
                parse, "expected 'E' or a character after '\\' within \"\\Q\"");
          } else if (ch == 'E') {
            /* \Q<*>\E | Finish quote */
            break;
          } else {
            if ((err = re__parse_new_create_rune(parse, ch))) {
              return err;
            }
          }
        }
      }
    } else {
      return re__parse_new_error(
          parse, "cannot use \\Q inside character class");
    }
  } else if (ch == 'S') {
    /* \S | Non-whitespace characters (Perl) */
    if (accept_classes) {
      if (!within_charclass) {
        if ((err = re__parse_new_create_charclass_ascii(
                 parse, RE__CHARCLASS_ASCII_TYPE_PERL_SPACE, 1))) {
          return err;
        }
      } else {
        if ((err = re__parse_new_insert_charclass_ascii(
                 parse, RE__CHARCLASS_ASCII_TYPE_PERL_SPACE, 1))) {
          return err;
        }
      }
    } else {
      return re__parse_new_error(
          parse, "cannot use \\S as ending character for range");
    }
  } else if (ch == 'W') {
    /* \W | Not a word character */
    if (accept_classes) {
      if (!within_charclass) {
        if ((err = re__parse_new_create_charclass_ascii(
                 parse, RE__CHARCLASS_ASCII_TYPE_WORD, 0))) {
          return err;
        }
      } else {
        if ((err = re__parse_new_insert_charclass_ascii(
                 parse, RE__CHARCLASS_ASCII_TYPE_WORD, 0))) {
          return err;
        }
      }
    } else {
      return re__parse_new_error(
          parse, "cannot use \\W as ending character for range");
    }
  } else if (ch == 'a') {
    /* \a | Bell character */
    *out_char = ch;
  } else if (ch == 'b') {
    /* \b | Word boundary */
    if (!within_charclass) {
      if ((err = re__parse_new_create_assert(parse, RE__ASSERT_TYPE_WORD))) {
        return err;
      }
    } else {
      return re__parse_new_error(
          parse, "cannot use \\b inside character class");
    }
  } else if (ch == 'd') {
    /* \d | Digit characters */
    if (accept_classes) {
      if (!within_charclass) {
        if ((err = re__parse_new_create_charclass_ascii(
                 parse, RE__CHARCLASS_ASCII_TYPE_DIGIT, 0))) {
          return err;
        }
      } else {
        if ((err = re__parse_new_insert_charclass_ascii(
                 parse, RE__CHARCLASS_ASCII_TYPE_DIGIT, 0))) {
          return err;
        }
      }
    } else {
      return re__parse_new_error(
          parse, "cannot use \\d as ending character for range");
    }
  } else if (ch == 'f') {
    /* \f | Form feed */
    *out_char = ch;
  } else if (ch == 'n') {
    /* \n | Newline */
    *out_char = ch;
  } else if (ch == 'p') {
    /* \p | Unicode character class */
    return re__parse_new_error(parse, "unimplemented");
  } else if (ch == 'r') {
    /* \r | Carriage return */
    *out_char = ch;
  } else if (ch == 's') {
    /* \s | Whitespace (Perl) */
    if (accept_classes) {
      if (!within_charclass) {
        if ((err = re__parse_new_create_charclass_ascii(
                 parse, RE__CHARCLASS_ASCII_TYPE_PERL_SPACE, 0))) {
          return err;
        }
      } else {
        if ((err = re__parse_new_insert_charclass_ascii(
                 parse, RE__CHARCLASS_ASCII_TYPE_PERL_SPACE, 0))) {
          return err;
        }
      }
    } else {
      return re__parse_new_error(
          parse, "cannot use \\s as ending character for range");
    }
  } else if (ch == 't') {
    /* \t | Horizontal tab */
    *out_char = ch;
  } else if (ch == 'v') {
    /* \v | Vertical tab */
    *out_char = ch;
  } else if (ch == 'w') {
    /* \w | Word character */
    if (accept_classes) {
      if (!within_charclass) {
        if ((err = re__parse_new_create_charclass_ascii(
                 parse, RE__CHARCLASS_ASCII_TYPE_WORD, 0))) {
          return err;
        }
      } else {
        if ((err = re__parse_new_insert_charclass_ascii(
                 parse, RE__CHARCLASS_ASCII_TYPE_WORD, 0))) {
          return err;
        }
      }
    } else {
      return re__parse_new_error(
          parse, "cannot use \\w as ending character for range");
    }
  } else if (ch == 'x') {
    /* \x | Two-digit hex literal or one-to-six digit hex literal */
    if ((err = re__parse_new_next_char(parse, ch))) {
      return err;
    }
    if (ch == '{') {
      /* \x{ | Bracketed hex literal */
      int i;
      while (1) {
        if ((err = re__parse_new_next_char(parse, ch))) {
          return err;
        }
        if (ch == RE__PARSE_NEW_EOF) {
          /* \x{<EOF> | Error condition */
          return re__parse_new_error(
              parse, "expected one to six hex characters for bracketed hex "
                     "escape \"\\x{\"");
        } else if (ch == '}') {
          if (i == 0) {
            /* \x{} | Error condition */
            return re__parse_new_error(
                parse, "expected one to six hex characters for bracketed hex "
                       "escape \"\\x{\"");
          } else {
            /* \x{[0-9a-fA-F]} | Finish */
            break;
          }
        } else if ((accum2 = re__parse_new_hex(ch)) == -1) {
          /* \x{<*> | Invalid, error condition */
          return re__parse_new_error(
              parse, "expected one to six hex characters for bracketed hex "
                     "escape \"\\x{\"");
        } else {
          /* \x{[0-9a-fA-F] | Add to accumulator */
          accum *= 16;
          accum += accum2;
          if (accum > RE_RUNE_MAX) {
            return re__parse_new_error(
                parse, "bracketed hex literal out of range [0, 0x10FFFF]");
          }
        }
      }
    } else if ((accum = re__parse_new_hex(ch)) == -1) {
      /* Handles EOF */
      return re__parse_new_error(
          parse, "expected two hex characters or a bracketed hex literal for "
                 "hex escape \"\\x\"");
    } else {
      /* \x[0-9a-fA-F] | Two-digit hex sequence */
      if ((err = re__parse_new_next_char(parse, ch))) {
        return err;
      }
      if ((accum2 = re__parse_new_hex(ch)) == -1) {
        /* Handles EOF */
        return re__parse_new_error(
            parse, "expected two hex characters or a bracketed hex literal for "
                   "hex escape \"\\x\"");
      }
      accum *= 16;
      accum += accum2;
      *out_char = accum;
    }
  } else if (ch == 'z') {
    /* \z | Absolute text end */
    if (!within_charclass) {
      if ((err = re__parse_new_create_assert(
               parse, RE__ASSERT_TYPE_TEXT_START_ABSOLUTE))) {
        return err;
      }
    } else {
      return re__parse_new_error(
          parse, "cannot use \\z inside character class");
    }
  }
  return err;
}

/* Parse a character class. */
MN_INTERNAL re_error re__parse_new_charclass(re__parse_new* parse)
{
  /* After first charclass bracket: [ */
  re_error err = RE_ERROR_NONE;
  re_rune ch;
  re__rune_range range;
  re__charclass_builder_begin(&parse->charclass_builder);
  if ((err = re__parse_new_next_char(parse, &ch))) {
    return err;
  }
  if (ch == RE__PARSE_NEW_EOF) {
    return re__parse_new_error(
        parse, "expected '^', characters, character classes, or character "
               "ranges for character class expression '['");
  } else if (ch == '^') {
    /* [^ | Start of invert */
    re__charclass_builder_invert(&parse->charclass_builder);
  } else {
    /* All other characters: begin parsing loop */
    goto range_loop;
  }
  while (1) {
    range.min = -1;
    range.max = -1;
    if ((err = re__parse_new_next_char(parse, &ch))) {
      return err;
    }
  range_loop:
    if (ch == RE__PARSE_NEW_EOF) {
      return re__parse_new_error(
          parse, "expected characters, character classes, or character "
                 "ranges for character class expression '['");
    } else if (ch == '[') {
      /* [[ | Literal [ or char class */
      range.min = '[';
      range.max = ']';
      if ((err = re__parse_new_next_char(parse, &ch))) {
        return err;
      }
      if (ch == RE__PARSE_NEW_EOF) {
        return re__parse_new_error(
            parse, "expected characters, character classes, or character "
                   "ranges for character class expression '['");
      } else if (ch == '\\') {
        if ((err = re__charclass_builder_insert_range(
                 &parse->charclass_builder, range))) {
          return err;
        }
        /* Go to charclass escape */
        goto range_loop;
      } else if (ch == ']') {
        /* [[] | A single [. */
        if ((err = re__charclass_builder_insert_range(
                 &parse->charclass_builder, range))) {
          return err;
        }
        /* Stop. */
        break;
      } else if (ch == ':') {
        /* [[: | Start of ASCII charclass */

      } else if (ch == '-') {
        /* [[- | Start of range. Set low rune to bracket. Look for high
         * rune. */
        range.min = '[';
        goto before_high_rune;
      } else {
        /* [[<*> | Add bracket. Set low rune to next char. Look for -.
         */
        if ((err = re__charclass_builder_insert_range(
                 &parse->charclass_builder, range))) {
          return err;
        }
        goto before_dash;
      }
    } else if (ch == '\\') {
      /* [\ | Starting character escape. */
      if ((err = re__parse_new_escape(parse, &range.min, 1, 1))) {
        return err;
      }
      if (range.min == RE__PARSE_NEW_EOF) {
        /* We parsed a character class, go back to start */
        continue;
      }
      /* We parsed a single character, it is set as the minimum value */
    } else {
      /* [<*> | Set low character of char class */
      range.min = ch;
    }
  before_dash:
    if ((err = re__parse_new_next_char(parse, &ch))) {
      return err;
    }
    if (ch == RE__PARSE_NEW_EOF) {
      /* Cannot EOF here */
      return re__parse_new_error(
          parse, "expected characters, character classes, or character "
                 "ranges for character class expression '['");
    } else if (ch == ']') {
      /* [<*>] | Add lo_rune, finish. */
      range.max = range.min;
      if ((err = re__charclass_builder_insert_range(
               &parse->charclass_builder, range))) {
        return err;
      }
      break;
    } else if (ch != '-') {
      /* [<*><*> | Add lo_rune, continue. */
      range.max = range.min;
      if ((err = re__charclass_builder_insert_range(
               &parse->charclass_builder, range))) {
        return err;
      }
      goto range_loop;
    }
    /* [<*>- | Look for end character for range */
  before_high_rune:
    if ((err = re__parse_new_next_char(parse, &ch))) {
      return err;
    }
    if (ch == RE__PARSE_NEW_EOF) {
      /* Cannot EOF here */
      return re__parse_new_error(
          parse, "expected characters, character classes, or character "
                 "ranges for character class expression '['");
    } else if (ch == ']') {
      /* [<*>-] | End character class. Add lo_rune. Add dash. */
      range.max = range.min;
      if ((err = re__charclass_builder_insert_range(
               &parse->charclass_builder, range))) {
        return err;
      }
      range.min = '-';
      range.max = '-';
      if ((err = re__charclass_builder_insert_range(
               &parse->charclass_builder, range))) {
        return err;
      }
      break;
    } else if (ch == '\\') {
      /* [<*>-\ | Ending character escape. */
      if ((err = re__parse_new_escape(parse, &range.max, 0, 1))) {
        return err;
      }
      if ((err = re__charclass_builder_insert_range(
               &parse->charclass_builder, range))) {
        return err;
      }
    } else {
      /* [<*>-<*> | Add range. */
      range.max = ch;
      if ((err = re__charclass_builder_insert_range(
               &parse->charclass_builder, range))) {
        return err;
      }
    }
  }
  {
    re__ast new_node;
    re__charclass new_charclass;
    mn_int32 new_charclass_ref;
    if ((err = re__charclass_builder_finish(
             &parse->charclass_builder, &new_charclass))) {
      return err;
    }
    /* We own new_charclass */
    if ((err = re__ast_root_add_charclass(
             &parse->reg->data->ast_root, new_charclass, &new_charclass_ref))) {
      re__charclass_destroy(&new_charclass);
      return err;
    }
    /* ast_root owns new_charclass */
    re__ast_init_charclass(&new_node, new_charclass_ref);
    if ((err = re__parse_new_link_node(parse, new_node))) {
      return err;
    }
  }
  return err;
}

MN_INTERNAL re_error re__parse_new_str(re__parse_new* parse, mn__str_view str)
{
  re_error err = RE_ERROR_NONE;
  /* Set string */
  parse->str = str;
  while (1) {
    re_rune ch;
    if ((err = re__parse_new_next_char(parse, ch))) {
      goto error;
    }
    if (ch == '$') {
      if ((err =
               re__parse_new_create_assert(parse, RE__ASSERT_TYPE_TEXT_END))) {
        goto error;
      }
    } else if (ch == '(') {
      /* ( | Begin a group. */
      if ((err = re__parse_new_group_begin(parse))) {
        goto error;
      }
    } else if (ch == ')') {
      /* ) | End a group. */
      if ((err = re__parse_new_group_end(parse))) {
        goto error;
      }
    } else if (ch == '*' || ch == '?' || ch == '+') {
      mn_size saved_pos;
      if (ch == '*') {
        if ((err = re__parse_new_create_star(parse))) {
          goto error;
        }
      } else if (ch == '?') {
        if ((err = re__parse_new_create_question(parse))) {
          goto error;
        }
      } else if (ch == '+') {
        if ((err = re__parse_new_create_plus(parse))) {
          goto error;
        }
      }
      saved_pos = parse->str_pos;
      if ((err = re__parse_new_next_char(parse, &ch))) {
        goto error;
      }
      if (ch == '?') {
        /* [*+?]? | Make previous operator non-greedy. */
        re__parse_new_swap_greedy(parse);
      } else {
        /* Handles EOF */
        parse->str_pos = saved_pos;
      }
    } else if (ch == '.') {
      /* . | Create an "any character." */
      if ((err = re__parse_new_create_any_char(parse))) {
        goto error;
      }
    } else if (ch == '[') {
      /* [ | Start of a character class. */
      if ((err = re__parse_new_charclass(parse))) {
        goto error;
      }
    } else if (ch == '\\') {
      /* \ | Start of escape sequence. */
      re_rune esc_char;
      if ((err = re__parse_new_escape(parse, &esc_char, 1, 0))) {
        goto error;
      }
      if (esc_char != RE__PARSE_NEW_EOF) {
        if ((err = re__parse_new_create_rune(parse, esc_char))) {
          goto error;
        }
      }
    } else if (ch == '^') {
      /* ^ | Text start assert. */
      if ((err = re__parse_new_create_assert(
               parse, RE__ASSERT_TYPE_TEXT_START))) {
        goto error;
      }
    } else if (ch == '{') {
      /* { | Start of counting form. */
      if ((err = re__parse_new_count(parse))) {
        goto error;
      }
    } else if (ch == '|') {
      /* | | Alternation. */
      if ((err = re__parse_new_alt(parse))) {
        goto error;
      }
    } else {
      /* Any other character. */
      if ((err = re__parse_new_create_rune(parse, ch))) {
        goto error;
      }
    }
  }
error:
  return err;
}

#undef RE__PARSE_NEW_EOF

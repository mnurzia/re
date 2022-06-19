#include "re_internal.h"

void re__parse_frame_init(
    re__parse_frame* frame, mn_int32 ast_root_ref, re__parse_flags flags)
{
  frame->ast_root_ref = ast_root_ref;
  frame->ast_prev_child_ref = RE__AST_NONE;
  frame->flags = flags;
}

MN__VEC_IMPL_FUNC(re__parse_frame, init)
MN__VEC_IMPL_FUNC(re__parse_frame, destroy)
MN__VEC_IMPL_FUNC(re__parse_frame, size)
MN__VEC_IMPL_FUNC(re__parse_frame, getref)
MN__VEC_IMPL_FUNC(re__parse_frame, push)
MN__VEC_IMPL_FUNC(re__parse_frame, pop)

void re__parse_init(re__parse* parse, re* reg)
{
  parse->reg = reg;
  mn__str_view_init_null(&parse->str);
  parse->str_pos = 0;
  re__parse_frame_vec_init(&parse->frames);
  re__charclass_builder_init(&parse->charclass_builder, &reg->data->rune_data);
}

void re__parse_destroy(re__parse* parse)
{
  re__parse_frame_vec_destroy(&parse->frames);
  re__charclass_builder_destroy(&parse->charclass_builder);
}

re__parse_frame* re__parse_get_frame(re__parse* parse)
{
  MN_ASSERT(re__parse_frame_vec_size(&parse->frames));
  return re__parse_frame_vec_getref(
      &parse->frames, re__parse_frame_vec_size(&parse->frames) - 1);
}

re__ast_type re__parse_get_frame_type(re__parse* parse)
{
  re__parse_frame* frame = re__parse_get_frame(parse);
  re__ast_type ret = RE__AST_TYPE_NONE;
  if (frame->ast_root_ref != RE__AST_NONE) {
    ret = re__ast_root_get(&parse->reg->data->ast_root, frame->ast_root_ref)
              ->type;
  }
  MN_ASSERT(
      ret == RE__AST_TYPE_GROUP || ret == RE__AST_TYPE_CONCAT ||
      ret == RE__AST_TYPE_ALT || ret == RE__AST_TYPE_NONE);
  return ret;
}

re_error re__parse_push_frame(
    re__parse* parse, mn_int32 ast_root_ref, re__parse_flags flags)
{
  re__parse_frame new_frame;
  re__parse_frame_init(&new_frame, ast_root_ref, flags);
  return re__parse_frame_vec_push(&parse->frames, new_frame);
}

void re__parse_pop_frame(re__parse* parse)
{
  re__parse_frame_vec_pop(&parse->frames);
}

int re__parse_frame_is_empty(re__parse* parse)
{
  return re__parse_get_frame(parse)->ast_prev_child_ref == RE__AST_NONE;
}

/* Convenience function to set the error to a literal string. */
MN_INTERNAL re_error re__parse_error(re__parse* parse, const char* err_chars)
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

#define RE__PARSE_UTF8_ACCEPT 0
#define RE__PARSE_UTF8_REJECT 12

static const mn_uint8 re__parse_utf8_tt[] = {
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
re__parse_utf8_decode(mn_uint32* state, mn_uint32* codep, mn_uint32 byte)
{
  mn_uint32 type = re__parse_utf8_tt[byte];

  *codep =
      (*state != 0) ? (byte & 0x3fu) | (*codep << 6) : (0xff >> type) & (byte);

  *state = re__parse_utf8_tt[256 + *state + type];
  return *state;
}

#define RE__PARSE_EOF (-1)

re_error re__parse_next_char(re__parse* parse, re_rune* ch)
{
  mn_uint32 codep = 0;
  mn_uint32 state = 0;
  while (1) {
    MN_ASSERT(parse->str_pos <= mn__str_view_size(&parse->str));
    if (parse->str_pos == mn__str_view_size(&parse->str)) {
      if (state == RE__PARSE_UTF8_ACCEPT) {
        parse->str_pos++;
        *ch = RE__PARSE_EOF;
        return RE_ERROR_NONE;
      } else {
        parse->str_pos++;
        return re__parse_error(parse, "invalid UTF-8 byte");
      }
    } else {
      mn_uint8 in_byte =
          (mn_uint8)(mn__str_view_get_data(&parse->str)[parse->str_pos]);
      if (!re__parse_utf8_decode(&state, &codep, in_byte)) {
        parse->str_pos++;
        *ch = (re_rune)codep;
        return RE_ERROR_NONE;
      } else if (state == RE__PARSE_UTF8_REJECT) {
        parse->str_pos++;
        return re__parse_error(parse, "invalid UTF-8 byte");
      } else {
        parse->str_pos++;
      }
    }
  }
}

/* Add a node after the previous child. */
MN_INTERNAL re_error re__parse_push_node(re__parse* parse, re__ast ast)
{
  re_error err = RE_ERROR_NONE;
  re__parse_frame* frame = re__parse_get_frame(parse);
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
MN_INTERNAL re_error re__parse_wrap_node(re__parse* parse, re__ast outer)
{
  re_error err = RE_ERROR_NONE;
  re__parse_frame* frame = re__parse_get_frame(parse);
  mn_int32 new_outer_ref = RE__AST_NONE;
  if ((err = re__ast_root_add_wrap(
           &parse->reg->data->ast_root, frame->ast_root_ref,
           frame->ast_prev_child_ref, outer, &new_outer_ref))) {
    return err;
  }
  frame->ast_prev_child_ref = new_outer_ref;
  return err;
}

MN_INTERNAL re_error
re__parse_opt_fuse_concat(re__parse* parse, re__ast* next, int* did_fuse)
{
  re__ast* prev;
  mn_int32 prev_child_ref;
  re__ast_type t_prev, t_next;
  re_error err = RE_ERROR_NONE;
  MN_ASSERT(!re__parse_frame_is_empty(parse));
  prev_child_ref = re__parse_get_frame(parse)->ast_prev_child_ref;
  prev = re__ast_root_get(&parse->reg->data->ast_root, prev_child_ref);
  t_prev = prev->type;
  t_next = next->type;
  *did_fuse = 0;
  if (t_next == RE__AST_TYPE_RUNE) {
    if (t_prev == RE__AST_TYPE_STR || t_prev == RE__AST_TYPE_RUNE) {
      mn_int32 str_ref;
      mn_char rune_bytes[16]; /* 16 oughta be good */
      int rune_bytes_ptr = 0;
      mn__str* out_str;
      if (t_prev == RE__AST_TYPE_RUNE) {
        mn__str new_str;
        re__ast new_ast;
        mn__str_init(&new_str);
        if ((err = re__ast_root_add_str(
                 &parse->reg->data->ast_root, new_str, &str_ref))) {
          return err;
        }
        rune_bytes_ptr += re__compile_gen_utf8(
            re__ast_get_rune(prev), (mn_uint8*)rune_bytes + rune_bytes_ptr);
        re__ast_init_str(&new_ast, str_ref);
        re__ast_root_replace(
            &parse->reg->data->ast_root, prev_child_ref, new_ast);
      } else {
        str_ref = re__ast_get_str_ref(prev);
      }
      rune_bytes_ptr += re__compile_gen_utf8(
          re__ast_get_rune(next), (mn_uint8*)rune_bytes + rune_bytes_ptr);
      out_str = re__ast_root_get_str(&parse->reg->data->ast_root, str_ref);
      if ((err = mn__str_cat_n(out_str, rune_bytes, (mn_size)rune_bytes_ptr))) {
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
MN_INTERNAL re_error re__parse_link_node(re__parse* parse, re__ast new_ast)
{
  re__ast_type frame_type = RE__AST_TYPE_NONE;
  re_error err = RE_ERROR_NONE;
  /* Firstly, attempt an optimization by fusing the nodes, if possible. */
  if (!re__parse_frame_is_empty(parse)) {
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
  frame_type = re__parse_get_frame_type(parse);
  /* Weird control flow -- it's the only way I figured out how to do the
   * assertion below. */
  if ((frame_type == RE__AST_TYPE_GROUP || frame_type == RE__AST_TYPE_ALT ||
       frame_type == RE__AST_TYPE_NONE) &&
      !re__parse_frame_is_empty(parse)) {
    re__ast new_concat;
    /* Wrap the last child(ren) in a concatenation */
    re__ast_init_concat(&new_concat);
    if ((err = re__parse_wrap_node(parse, new_concat))) {
      return err;
    }
    /* new_concat is moved */
    /* Push a new frame */
    if ((err = re__parse_push_frame(
             parse, re__parse_get_frame(parse)->ast_prev_child_ref,
             re__parse_get_frame(parse)->flags))) {
      return err;
    }
  }
  /* Add the new node to the frame. */
  if ((err = re__parse_push_node(parse, new_ast))) {
    return err;
  }
  return err;
}

/* Called after '(' */
re_error re__parse_group_begin(re__parse* parse)
{
  re_error err = RE_ERROR_NONE;
  re_rune ch;
  mn_size begin_name_pos = 0;
  mn_size end_name_pos = 0;
  re__parse_flags flags = re__parse_get_frame(parse)->flags;
  mn_size saved_pos = parse->str_pos;
  re__ast_group_flags group_flags = 0;
  if ((err = re__parse_next_char(parse, &ch))) {
    return err;
  }
  if (ch == RE__PARSE_EOF) {
    return re__parse_error(parse, "unmatched '('");
  } else if (ch == '?') {
    int set_bit = 1;
    /* (? | Start of group flags/name */
    while (1) {
      if ((err = re__parse_next_char(parse, &ch))) {
        return err;
      }
      if (ch == ')') {
        /* (?) | Go back to ground without creating a group, retain flags */
        re__parse_get_frame(parse)->flags = flags;
        return err;
      } else if (ch == '-') {
        /* (?- | Negate remaining flags */
        set_bit = 0;
      } else if (ch == ':') {
        /* (?: | Non-matching group, also signals end of flags */
        group_flags |= RE__AST_GROUP_FLAG_NONMATCHING;
        break;
      } else if (ch == 'U') {
        /* (?U | Ungreedy mode: *+? operators have priority swapped */
        if (set_bit) {
          flags |= RE__PARSE_FLAG_UNGREEDY;
        } else {
          flags &= ~(unsigned int)RE__PARSE_FLAG_UNGREEDY;
        }
      } else if (ch == 'i') {
        /* (?i: Case insensitive matching */
        if (set_bit) {
          flags |= RE__PARSE_FLAG_CASE_INSENSITIVE;
        } else {
          flags &= ~(unsigned int)RE__PARSE_FLAG_CASE_INSENSITIVE;
        }
      } else if (ch == 'm') {
        /* (?m: Multi-line mode: ^$ match line boundaries */
        if (set_bit) {
          flags |= RE__PARSE_FLAG_MULTILINE;
        } else {
          flags &= ~(unsigned int)RE__PARSE_FLAG_MULTILINE;
        }
      } else if (ch == 's') {
        /* (?s: Stream (?) mode: . matches \n */
        if (set_bit) {
          flags |= RE__PARSE_FLAG_DOT_NEWLINE;
        } else {
          flags &= ~(unsigned int)RE__PARSE_FLAG_DOT_NEWLINE;
        }
      } else if (ch == '<' || ch == 'P') {
        mn_size saved_name_pos = 0;
        if (ch == 'P') {
          /* (?P | Alternative way to start group name */
          if ((err = re__parse_next_char(parse, &ch))) {
            return err;
          }
          if (ch != '<') {
            /* Handles EOF */
            return re__parse_error(parse, "expected '<' to begin group name");
          }
        }
        /* (?< | Start of group name */
        begin_name_pos = parse->str_pos;
        end_name_pos = begin_name_pos;
        if ((err = re__parse_next_char(parse, &ch))) {
          return err;
        }
        if (ch == '>') {
          return re__parse_error(parse, "cannot create empty group name");
        }
        while (1) {
          saved_name_pos = parse->str_pos;
          if ((err = re__parse_next_char(parse, &ch))) {
            return err;
          }
          if (ch == '>') {
            end_name_pos = saved_name_pos;
            group_flags |= RE__AST_GROUP_FLAG_NAMED;
            break;
          } else if (ch == RE__PARSE_EOF) {
            return re__parse_error(parse, "expected '>' to close group name");
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
  } else {
    /* Rewind, we consumed an extra character */
    parse->str_pos = saved_pos;
  }
  {
    mn_uint32 new_group_idx = 0;
    mn__str_view group_name;
    re__ast new_group;
    if (group_flags & RE__AST_GROUP_FLAG_NAMED) {
      MN_ASSERT(!(group_flags & RE__AST_GROUP_FLAG_NONMATCHING));
      mn__str_view_init_n(
          &group_name, mn__str_view_get_data(&parse->str) + begin_name_pos,
          end_name_pos - begin_name_pos);
      new_group_idx = re__ast_root_get_num_groups(&parse->reg->data->ast_root);
      if ((err = re__ast_root_add_group(
               &parse->reg->data->ast_root, group_name))) {
        return err;
      }
    } else if (!(group_flags & RE__AST_GROUP_FLAG_NONMATCHING)) {
      mn__str_view_init_null(&group_name);
      new_group_idx = re__ast_root_get_num_groups(&parse->reg->data->ast_root);
      if ((err = re__ast_root_add_group(
               &parse->reg->data->ast_root, group_name))) {
        return err;
      }
    }
    re__ast_init_group(&new_group, new_group_idx, group_flags);
    if ((err = re__parse_link_node(parse, new_group))) {
      return err;
    }
    if ((err = re__parse_push_frame(
             parse, re__parse_get_frame(parse)->ast_prev_child_ref, flags))) {
      return err;
    }
  }
  return err;
}

/* Act on a '|' character. If this is the first alteration, we wrap the current
 * group node in an ALT. If not, we simply add to the previous ALT. */
MN_INTERNAL re_error re__parse_alt(re__parse* parse)
{
  re__ast_type peek_type;
  re_error err = RE_ERROR_NONE;
  while (1) {
    peek_type = re__parse_get_frame_type(parse);
    if (peek_type == RE__AST_TYPE_CONCAT) {
      /* Pop all concatenations, alt takes priority */
      re__parse_pop_frame(parse);
    } else if (
        peek_type == RE__AST_TYPE_GROUP || peek_type == RE__AST_TYPE_NONE) {
      /* This is the initial alteration: "a|" or "(a|" */
      /* Note: the group in question could be the base frame. */
      /* In any case, we shim an ALT node in before the previous child. */
      /* Since GROUP nodes are defined to have a maximum of one child, we
       * don't need to mess around with the amount of children for either
       * node. */
      re__ast new_alt;
      re__ast_init_alt(&new_alt);
      if (re__parse_frame_is_empty(parse)) {
        /* Empty frame -- null alteration. */
        re__ast new_concat;
        re__ast_init_concat(&new_concat);
        if ((err = re__parse_link_node(parse, new_concat))) {
          return err;
        }
      }
      if ((err = re__parse_wrap_node(parse, new_alt))) {
        return err;
      }
      /* Push a new ALT frame */
      if ((err = re__parse_push_frame(
               parse, re__parse_get_frame(parse)->ast_prev_child_ref,
               re__parse_get_frame(parse)->flags))) {
        return err;
      }
      return err;
    } else { /* peek_type == RE__AST_TYPE_ALT */
      /* Third+ part of the alteration: "a|b|" or "(a|b|" */
      /* Indicate that there are no new children (this is the beginning
       * of the second+ part of an alteration) */
      re__parse_get_frame(parse)->ast_prev_child_ref = RE__AST_NONE;
      return err;
    }
  }
}

/* Create an empty concatenation for an ending ALT, if necessary. */
MN_INTERNAL re_error re__parse_alt_finish(re__parse* parse)
{
  if (re__parse_frame_is_empty(parse)) {
    re__ast new_concat;
    re__ast_init_concat(&new_concat);
    return re__parse_link_node(parse, new_concat);
  }
  return RE_ERROR_NONE;
}

/* Called after '(' */
re_error re__parse_group_end(re__parse* parse)
{
  re_error err = RE_ERROR_NONE;
  while (1) {
    re__ast_type peek_type = re__parse_get_frame_type(parse);
    if (peek_type == RE__AST_TYPE_NONE) {
      /* If we are at the absolute bottom of the stack, there was no opening
       * parentheses to begin with. */
      return re__parse_error(parse, "unmatched ')'");
    }
    if (peek_type == RE__AST_TYPE_ALT) {
      if ((err = re__parse_alt_finish(parse))) {
        return err;
      }
    }
    /* Now pop the current frame */
    re__parse_pop_frame(parse);
    /* If we just popped a group, finish */
    if (peek_type == RE__AST_TYPE_GROUP) {
      break;
    }
  }
  return err;
}

/* Create a new assert */
MN_INTERNAL re_error
re__parse_create_assert(re__parse* parse, re__assert_type assert_type)
{
  re__ast new_node;
  re__ast_init_assert(&new_node, assert_type);
  return re__parse_link_node(parse, new_node);
}

MN_INTERNAL re_error re__parse_create_star(re__parse* parse)
{
  re__ast new_star;
  if (re__parse_frame_is_empty(parse)) {
    return re__parse_error(parse, "cannot use '*' operator with nothing");
  }
  re__ast_init_quantifier(&new_star, 0, RE__AST_QUANTIFIER_INFINITY);
  re__ast_set_quantifier_greediness(
      &new_star,
      !!!(re__parse_get_frame(parse)->flags & RE__PARSE_FLAG_UNGREEDY));
  return re__parse_wrap_node(parse, new_star);
}

MN_INTERNAL re_error re__parse_create_question(re__parse* parse)
{
  re__ast new_star;
  if (re__parse_frame_is_empty(parse)) {
    return re__parse_error(parse, "cannot use '?' operator with nothing");
  }
  re__ast_init_quantifier(&new_star, 0, 2);
  re__ast_set_quantifier_greediness(
      &new_star,
      !!!(re__parse_get_frame(parse)->flags & RE__PARSE_FLAG_UNGREEDY));
  return re__parse_wrap_node(parse, new_star);
}

MN_INTERNAL re_error re__parse_create_plus(re__parse* parse)
{
  re__ast new_star;
  if (re__parse_frame_is_empty(parse)) {
    return re__parse_error(parse, "cannot use '?' operator with nothing");
  }
  re__ast_init_quantifier(&new_star, 1, RE__AST_QUANTIFIER_INFINITY);
  re__ast_set_quantifier_greediness(
      &new_star,
      !!!(re__parse_get_frame(parse)->flags & RE__PARSE_FLAG_UNGREEDY));
  return re__parse_wrap_node(parse, new_star);
}

MN_INTERNAL void re__parse_swap_greedy(re__parse* parse)
{
  re__ast* quant;
  /* Cannot make nothing ungreedy */
  MN_ASSERT(!re__parse_frame_is_empty(parse));
  quant = re__ast_root_get(
      &parse->reg->data->ast_root,
      re__parse_get_frame(parse)->ast_prev_child_ref);
  /* Must be a quantifier */
  MN_ASSERT(quant->type == RE__AST_TYPE_QUANTIFIER);
  re__ast_set_quantifier_greediness(
      quant, !re__ast_get_quantifier_greediness(quant));
}

MN_INTERNAL re_error re__parse_create_any_char(re__parse* parse)
{
  re__ast new_dot;
  if (!(re__parse_get_frame(parse)->flags & RE__PARSE_FLAG_DOT_NEWLINE)) {
    re__ast_init_any_char(&new_dot);
  } else {
    re__ast_init_any_char_newline(&new_dot);
  }
  return re__parse_link_node(parse, new_dot);
}

MN_INTERNAL re_error re__parse_create_any_byte(re__parse* parse)
{
  re__ast new_c;
  re__ast_init_any_byte(&new_c);
  return re__parse_link_node(parse, new_c);
}

/* Ingest a single rune, or casefold it if required. */
MN_INTERNAL re_error re__parse_create_rune(re__parse* parse, re_rune ch)
{
  re_error err = RE_ERROR_NONE;
  if (!(re__parse_get_frame(parse)->flags & RE__PARSE_FLAG_CASE_INSENSITIVE)) {
    re__ast new_rune;
    re__ast_init_rune(&new_rune, ch);
    return re__parse_link_node(parse, new_rune);
  } else {
    int num_fold_runes =
        re__rune_data_casefold(&parse->reg->data->rune_data, ch, MN_NULL);
    if (num_fold_runes == 1) {
      re__ast new_rune;
      re__ast_init_rune(&new_rune, ch);
      return re__parse_link_node(parse, new_rune);
    } else {
      re__charclass out;
      re__ast new_charclass;
      mn_int32 out_ref;
      re__rune_range new_range;
      re__charclass_builder_begin(&parse->charclass_builder);
      re__charclass_builder_fold(&parse->charclass_builder);
      new_range.min = ch;
      new_range.max = ch;
      if ((err = re__charclass_builder_insert_range(
               &parse->charclass_builder, new_range))) {
        return err;
      }
      if ((err =
               re__charclass_builder_finish(&parse->charclass_builder, &out))) {
        return err;
      }
      if ((err = re__ast_root_add_charclass(
               &parse->reg->data->ast_root, out, &out_ref))) {
        re__charclass_destroy(&out);
        return err;
      }
      re__ast_init_charclass(&new_charclass, out_ref);
      if ((err = re__parse_link_node(parse, new_charclass))) {
        return err;
      }
      return err;
    }
  }
}

/* Add characters to charclass builder, folding if necessary.
 * This function is potentially very slow. */
MN_INTERNAL re_error
re__parse_charclass_insert_range(re__parse* parse, re__rune_range range)
{
  if (range.min > range.max) {
    MN__SWAP(range.min, range.max, re_rune);
  }
  return re__charclass_builder_insert_range(&parse->charclass_builder, range);
}

/* Create an ASCII charclass. */
MN_INTERNAL re_error re__parse_create_charclass_ascii(
    re__parse* parse, re__charclass_ascii_type ascii_cc, int inverted)
{
  re_error err = RE_ERROR_NONE;
  re__ast new_node;
  mn_int32 new_class_ref;
  re__charclass new_class;
  if ((err = re__charclass_init_from_class(&new_class, ascii_cc, inverted))) {
    return err;
  }
  if (re__parse_get_frame(parse)->flags & RE__PARSE_FLAG_CASE_INSENSITIVE) {
    re__charclass folded_class;
    re__charclass_builder_begin(&parse->charclass_builder);
    re__charclass_builder_fold(&parse->charclass_builder);
    if ((err = re__charclass_builder_insert_class(
             &parse->charclass_builder, &new_class))) {
      re__charclass_destroy(&new_class);
      return err;
    }
    if ((err = re__charclass_builder_finish(
             &parse->charclass_builder, &folded_class))) {
      re__charclass_destroy(&new_class);
      return err;
    }
    re__charclass_destroy(&new_class);
    new_class = folded_class;
  }
  if ((err = re__ast_root_add_charclass(
           &parse->reg->data->ast_root, new_class, &new_class_ref))) {
    re__charclass_destroy(&new_class);
    return err;
  }
  /* ast_root now owns new_class */
  re__ast_init_charclass(&new_node, new_class_ref);
  return re__parse_link_node(parse, new_node);
}

/* Insert an ASCII charclass into the builder. */
MN_INTERNAL re_error re__parse_insert_charclass_ascii(
    re__parse* parse, re__charclass_ascii_type ascii_cc, int inverted)
{
  re_error err = RE_ERROR_NONE;
  re__charclass new_class;
  if ((err = re__charclass_init_from_class(&new_class, ascii_cc, inverted))) {
    return err;
  }
  if ((err = re__charclass_builder_insert_class(
           &parse->charclass_builder, &new_class))) {
    re__charclass_destroy(&new_class);
    return err;
  }
  re__charclass_destroy(&new_class);
  return err;
}

/* Either insert a Unicode property into the builder or create a new charclass
 * out of the property. */
MN_INTERNAL re_error re__parse_unicode_property(
    re__parse* parse, int inverted, int accept_classes, int within_charclass)
{
  re_error err = RE_ERROR_NONE;
  re_rune ch;
  mn_size start_pos;
  mn_size prev_pos;
  re__rune_range* ranges;
  mn_size ranges_size;
  if (!accept_classes) {
    return re__parse_error("cannot use Unicode property as range ending "
                           "character for character class");
  }
  if ((err = re__parse_next_char(parse, &ch))) {
    return err;
  }
  if (ch != '{') {
    return re__parse_error("expected '{' to begin Unicode property name");
  }
  name_start = parse->str_pos;
  while (1) {
    name_end = parse->str_pos;
    if ((err = re__parse_next_char(parse, &ch))) {
      return err;
    }
    if (ch == '}') {
      break;
    } else if (ch == RE__PARSE_EOF) {
      return re__parse_error("expected '}' to close Unicode property name");
    }
  }
  if ((err = re__rune_data_get_property(
           &parse->reg->data->rune_data,
           mn__str_get_data(&parse->str)[name_start], name_end - name_start,
           &ranges, &ranges_size))) {
    if (err == RE_ERROR_INVALID) {
      return re__parse_error("invalid Unicode property name");
    }
    return err;
  }
  if (within_charclass) {
    if (!inverted) {
      return re__charclass_builder_insert_ranges(
          &parse->charclass_builder, ranges, ranges_size);
    } else {
      re__charclass temp;
      if ((err = re__charclass_init_from_ranges(
               &temp, ranges, ranges_size, inverted))) {
        return err;
      }
      err =
          re__charclass_builder_insert_class(&parse->charclass_builder, &temp);
      re__charclass_destroy(&temp);
      return err;
    }
  } else {
    re__charclass out;
    re__ast new_node;
    mn_int32 out_charclass_ref;
    if (!inverted && !(re__parse_get_frame(parse)->flags &
                       RE__PARSE_FLAG_CASE_INSENSITIVE)) {
      if ((err =
               re__charclass_init_from_ranges(&out, ranges, ranges_size, 0))) {
        return err;
      }
    } else {
      re__charclass_builder_begin(&parse->charclass_builder);
      if (re__parse_get_frame(parse)->flags & RE__PARSE_FLAG_CASE_INSENSITIVE) {
        re__charclass_builder_fold(builder);
      }
      if (inverted) {
        re__charclass_builder_invert(builder);
      }
      if ((err = re__charclass_builder_insert_ranges(
               builder, ranges, ranges_size))) {
        return err;
      }
      if ((err = re__charclass_builder_finish(builder, &out))) {
        return err;
      }
    }
    if ((err = re__ast_root_add_charclass(
             &parse->reg->data->ast_root, out, &out_charclass_ref))) {
      return err;
    }
    re__ast_init_charclass(&new_node, out_charclass_ref);
    return re__parse_link_node(parse, new_node);
  }
}

MN_INTERNAL int re__parse_oct(re_rune ch)
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

MN_INTERNAL int re__parse_hex(re_rune ch)
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
MN_INTERNAL re_error re__parse_escape(
    re__parse* parse, re_rune* out_char, int accept_classes,
    int within_charclass)
{
  re_error err = RE_ERROR_NONE;
  re_rune ch;
  mn_int32 accum = 0;
  mn_int32 accum2 = 0;
  mn_size saved_pos;
  int i = 0;
  if ((err = re__parse_next_char(parse, &ch))) {
    return err;
  }
  saved_pos = parse->str_pos;
  *out_char = RE__PARSE_EOF;
  if (ch == RE__PARSE_EOF) {
    return re__parse_error(parse, "unfinished escape sequence");
  } else if ((accum = re__parse_oct(ch)) != -1) {
    /* \[0-7] | Octal digits */
    for (i = 0; i < 2; i++) {
      if ((err = re__parse_next_char(parse, &ch))) {
        return err;
      }
      if ((accum2 = re__parse_oct(ch)) != -1) {
        /* \[0-7][0-7] | Found second/third octal digit */
        accum *= 8;
        accum += accum2;
        saved_pos = parse->str_pos;
      } else {
        /* \[0-7]<*> | Did not find second/third octal digit */
        break;
      }
    }
    *out_char = accum;
    parse->str_pos = saved_pos;
  } else if (ch == 'A') {
    /* \A | Absolute text start */
    if (!within_charclass) {
      if ((err = re__parse_create_assert(
               parse, RE__ASSERT_TYPE_TEXT_START_ABSOLUTE))) {
        return err;
      }
    } else {
      return re__parse_error(parse, "cannot use \\A inside character class");
    }
  } else if (ch == 'B') {
    /* \B | Not a word boundary */
    if (!within_charclass) {
      if ((err = re__parse_create_assert(parse, RE__ASSERT_TYPE_WORD_NOT))) {
        return err;
      }
    } else {
      return re__parse_error(parse, "cannot use \\B inside character class");
    }
  } else if (ch == 'C') {
    /* \C: Any *byte* (NOT any char) */
    if (!within_charclass) {
      if ((err = re__parse_create_any_byte(parse))) {
        return err;
      }
    } else {
      return re__parse_error(parse, "cannot use \\C inside character class");
    }
  } else if (ch == 'D') {
    /* \D: Non-digit characters */
    if (accept_classes) {
      if (!within_charclass) {
        if ((err = re__parse_create_charclass_ascii(
                 parse, RE__CHARCLASS_ASCII_TYPE_DIGIT, 1))) {
          return err;
        }
      } else {
        if ((err = re__parse_insert_charclass_ascii(
                 parse, RE__CHARCLASS_ASCII_TYPE_DIGIT, 1))) {
          return err;
        }
      }
    } else {
      return re__parse_error(
          parse, "cannot use \\D as ending character for range");
    }
  } else if (ch == 'E') {
    /* \E | Invalid here */
    return re__parse_error(parse, "\\E can only be used from within \\Q");
  } else if (ch == 'P') {
    /* \P | Inverted Unicode character class */
    return re__parse_unicode_property(
        parse, 1, accept_classes, within_charclass);
  } else if (ch == 'Q') {
    /* \Q | Quote begin */
    if (!within_charclass) {
      int is_escape = 0;
      while (1) {
        saved_pos = parse->str_pos;
        if ((err = re__parse_next_char(parse, &ch))) {
          return err;
        }
        if (!is_escape) {
          if (ch == RE__PARSE_EOF) {
            /* \Q<*><EOF> | End, having added all previous chars */
            parse->str_pos = saved_pos;
            break;
          } else if (ch == '\\') {
            /* \Q\ | Escape within '\Q' */
            is_escape = 1;
          } else {
            /* \Q<*> | Add character */
            if ((err = re__parse_create_rune(parse, ch))) {
              return err;
            }
          }
        } else {
          if (ch == RE__PARSE_EOF) {
            /* \Q\<EOF> | Error */
            return re__parse_error(
                parse, "expected 'E' or a character after '\\' within \"\\Q\"");
          } else if (ch == 'E') {
            /* \Q<*>\E | Finish quote */
            break;
          } else {
            if ((err = re__parse_create_rune(parse, ch))) {
              return err;
            }
            is_escape = 0;
          }
        }
      }
    } else {
      return re__parse_error(parse, "cannot use \\Q inside character class");
    }
  } else if (ch == 'S') {
    /* \S | Non-whitespace characters (Perl) */
    if (accept_classes) {
      if (!within_charclass) {
        if ((err = re__parse_create_charclass_ascii(
                 parse, RE__CHARCLASS_ASCII_TYPE_PERL_SPACE, 1))) {
          return err;
        }
      } else {
        if ((err = re__parse_insert_charclass_ascii(
                 parse, RE__CHARCLASS_ASCII_TYPE_PERL_SPACE, 1))) {
          return err;
        }
      }
    } else {
      return re__parse_error(
          parse, "cannot use \\S as ending character for range");
    }
  } else if (ch == 'W') {
    /* \W | Not a word character */
    if (accept_classes) {
      if (!within_charclass) {
        if ((err = re__parse_create_charclass_ascii(
                 parse, RE__CHARCLASS_ASCII_TYPE_WORD, 1))) {
          return err;
        }
      } else {
        if ((err = re__parse_insert_charclass_ascii(
                 parse, RE__CHARCLASS_ASCII_TYPE_WORD, 1))) {
          return err;
        }
      }
    } else {
      return re__parse_error(
          parse, "cannot use \\W as ending character for range");
    }
  } else if (ch == 'a') {
    /* \a | Bell character */
    *out_char = 0x7;
  } else if (ch == 'b') {
    /* \b | Word boundary */
    if (!within_charclass) {
      if ((err = re__parse_create_assert(parse, RE__ASSERT_TYPE_WORD))) {
        return err;
      }
    } else {
      return re__parse_error(parse, "cannot use \\b inside character class");
    }
  } else if (ch == 'd') {
    /* \d | Digit characters */
    if (accept_classes) {
      if (!within_charclass) {
        if ((err = re__parse_create_charclass_ascii(
                 parse, RE__CHARCLASS_ASCII_TYPE_DIGIT, 0))) {
          return err;
        }
      } else {
        if ((err = re__parse_insert_charclass_ascii(
                 parse, RE__CHARCLASS_ASCII_TYPE_DIGIT, 0))) {
          return err;
        }
      }
    } else {
      return re__parse_error(
          parse, "cannot use \\d as ending character for range");
    }
  } else if (ch == 'f') {
    /* \f | Form feed */
    *out_char = 0xC;
  } else if (ch == 'n') {
    /* \n | Newline */
    *out_char = 0xA;
  } else if (ch == 'p') {
    /* \p | Unicode character class */
    return re__parse_unicode_property(
        parse, 0, accept_classes, within_charclass);
  } else if (ch == 'r') {
    /* \r | Carriage return */
    *out_char = 0xD;
  } else if (ch == 's') {
    /* \s | Whitespace (Perl) */
    if (accept_classes) {
      if (!within_charclass) {
        if ((err = re__parse_create_charclass_ascii(
                 parse, RE__CHARCLASS_ASCII_TYPE_PERL_SPACE, 0))) {
          return err;
        }
      } else {
        if ((err = re__parse_insert_charclass_ascii(
                 parse, RE__CHARCLASS_ASCII_TYPE_PERL_SPACE, 0))) {
          return err;
        }
      }
    } else {
      return re__parse_error(
          parse, "cannot use \\s as ending character for range");
    }
  } else if (ch == 't') {
    /* \t | Horizontal tab */
    *out_char = 0x9;
  } else if (ch == 'v') {
    /* \v | Vertical tab */
    *out_char = 0xB;
  } else if (ch == 'w') {
    /* \w | Word character */
    if (accept_classes) {
      if (!within_charclass) {
        if ((err = re__parse_create_charclass_ascii(
                 parse, RE__CHARCLASS_ASCII_TYPE_WORD, 0))) {
          return err;
        }
      } else {
        if ((err = re__parse_insert_charclass_ascii(
                 parse, RE__CHARCLASS_ASCII_TYPE_WORD, 0))) {
          return err;
        }
      }
    } else {
      return re__parse_error(
          parse, "cannot use \\w as ending character for range");
    }
  } else if (ch == 'x') {
    /* \x | Two-digit hex literal or one-to-six digit hex literal */
    accum = 0;
    if ((err = re__parse_next_char(parse, &ch))) {
      return err;
    }
    if (ch == '{') {
      /* \x{ | Bracketed hex literal */
      int hex_idx = 0;
      while (1) {
        if ((err = re__parse_next_char(parse, &ch))) {
          return err;
        }
        if (ch == RE__PARSE_EOF) {
          /* \x{<EOF> | Error condition */
          return re__parse_error(
              parse, "expected one to six hex characters for bracketed hex "
                     "escape \"\\x{\"");
        } else if (ch == '}') {
          if (hex_idx == 0) {
            /* \x{} | Error condition */
            return re__parse_error(
                parse, "expected one to six hex characters for bracketed hex "
                       "escape \"\\x{\"");
          } else {
            /* \x{[0-9a-fA-F]} | Finish */
            *out_char = accum;
            break;
          }
        } else if ((accum2 = re__parse_hex(ch)) == -1) {
          /* \x{<*> | Invalid, error condition */
          return re__parse_error(
              parse, "expected one to six hex characters for bracketed hex "
                     "escape \"\\x{\"");
        } else {
          /* \x{[0-9a-fA-F] | Add to accumulator */
          accum *= 16;
          accum += accum2;
          if (accum > RE_RUNE_MAX) {
            return re__parse_error(
                parse, "bracketed hex literal out of range [0, 0x10FFFF]");
          }
        }
        hex_idx++;
      }
    } else if ((accum = re__parse_hex(ch)) == -1) {
      /* Handles EOF */
      return re__parse_error(
          parse, "expected two hex characters or a bracketed hex literal for "
                 "hex escape \"\\x\"");
    } else {
      /* \x[0-9a-fA-F] | Two-digit hex sequence */
      if ((err = re__parse_next_char(parse, &ch))) {
        return err;
      }
      if ((accum2 = re__parse_hex(ch)) == -1) {
        /* Handles EOF */
        return re__parse_error(
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
      if ((err = re__parse_create_assert(
               parse, RE__ASSERT_TYPE_TEXT_END_ABSOLUTE))) {
        return err;
      }
    } else {
      return re__parse_error(parse, "cannot use \\z inside character class");
    }
  } else {
    return re__parse_error(parse, "invalid escape sequence");
  }
  return err;
}

/* Parse a character class. */
MN_INTERNAL re_error re__parse_charclass(re__parse* parse)
{
  /* After first charclass bracket: [ */
  re_error err = RE_ERROR_NONE;
  re_rune ch;
  re__rune_range range;
  re__charclass_builder_begin(&parse->charclass_builder);
  if (re__parse_get_frame(parse)->flags & RE__PARSE_FLAG_CASE_INSENSITIVE) {
    re__charclass_builder_fold(&parse->charclass_builder);
  }
  if ((err = re__parse_next_char(parse, &ch))) {
    return err;
  }
  if (ch == RE__PARSE_EOF) {
    return re__parse_error(
        parse, "expected '^', characters, character classes, or character "
               "ranges for character class expression '['");
  } else if (ch == ']') {
    /* [] | Set right bracket as low character, look for dash */
    range.min = ']';
    goto before_dash;
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
    if ((err = re__parse_next_char(parse, &ch))) {
      return err;
    }
  range_loop:
    if (ch == RE__PARSE_EOF) {
      return re__parse_error(
          parse, "expected characters, character classes, or character "
                 "ranges for character class expression '['");
    } else if (ch == '[') {
      /* [[ | Literal [ or char class */
      range.min = '[';
      range.max = '[';
      if ((err = re__parse_next_char(parse, &ch))) {
        return err;
      }
      if (ch == RE__PARSE_EOF) {
        /* [[<EOF> | Error */
        return re__parse_error(
            parse, "expected characters, character classes, or character "
                   "ranges for character class expression '['");
      } else if (ch == '\\') {
        /* [[\ | Push bracket, begin escape */
        if ((err = re__parse_charclass_insert_range(parse, range))) {
          return err;
        }
        /* Go to charclass escape */
        goto range_loop;
      } else if (ch == ']') {
        /* [[] | A single [. */
        if ((err = re__parse_charclass_insert_range(parse, range))) {
          return err;
        }
        /* Stop. */
        break;
      } else if (ch == ':') {
        /* [[: | Start of ASCII charclass */
        mn_size name_start_pos = parse->str_pos;
        mn_size name_end_pos = parse->str_pos;
        int inverted = 0;
        if ((err = re__parse_next_char(parse, &ch))) {
          return err;
        }
        if (ch == '^') {
          /* [[:^ | Invert ASCII charclass */
          inverted = 1;
          name_start_pos = parse->str_pos;
          name_end_pos = parse->str_pos;
        } else {
          goto ascii_name;
        }
        while (1) {
          if ((err = re__parse_next_char(parse, &ch))) {
            return err;
          }
        ascii_name:
          if (ch == RE__PARSE_EOF) {
            /* [[:<EOF> | Error */
            return re__parse_error(
                parse, "expected named character class for \"[[:\"");
          } else if (ch == ':') {
            /* [[:<*>: | Look for right bracket to finish */
            if ((err = re__parse_next_char(parse, &ch))) {
              return err;
            }
            if (ch == RE__PARSE_EOF || ch != ']') {
              /* [[:<*>:<EOF> | Error */
              return re__parse_error(
                  parse, "expected named character class for \"[[:\"");
            } else { /* ch == ']' */
              /* [[:<*>:] | Finish named char class */
              break;
            }
          } else {
            name_end_pos = parse->str_pos;
          }
        }
        {
          mn__str_view name_view;
          re__charclass ascii_cc;
          mn__str_view_init_n(
              &name_view, mn__str_view_get_data(&parse->str) + name_start_pos,
              name_end_pos - name_start_pos);
          if ((err = re__charclass_init_from_str(
                   &ascii_cc, name_view, inverted))) {
            if (err == RE_ERROR_INVALID) {
              /* couldn't find charclass with name */
              return re__parse_error(
                  parse, "unknown ASCII character class name");
            } else {
              return err;
            }
          }
          if ((err = re__charclass_builder_insert_class(
                   &parse->charclass_builder, &ascii_cc))) {
            re__charclass_destroy(&ascii_cc);
            return err;
          }
          re__charclass_destroy(&ascii_cc);
          continue;
        }
      } else if (ch == '-') {
        /* [[- | Start of range. Set low rune to bracket. Look for high rune. */
        range.min = '[';
        goto before_high_rune;
      } else {
        /* [[<*> | Add bracket. Set low rune to next char. Look for -. */
        if ((err = re__parse_charclass_insert_range(parse, range))) {
          return err;
        }
        range.min = ch;
        goto before_dash;
      }
    } else if (ch == '\\') {
      /* [\ | Starting character escape. */
      if ((err = re__parse_escape(parse, &range.min, 1, 1))) {
        return err;
      }
      if (range.min == RE__PARSE_EOF) {
        /* We parsed a character class, go back to start */
        continue;
      }
      /* We parsed a single character, it is set as the minimum value */
    } else if (ch == ']') {
      /* [] | End char class */
      break;
    } else {
      /* [<*> | Set low character of char class */
      range.min = ch;
    }
  before_dash:
    if ((err = re__parse_next_char(parse, &ch))) {
      return err;
    }
    if (ch == RE__PARSE_EOF) {
      /* Cannot EOF here */
      return re__parse_error(
          parse, "expected characters, character classes, or character "
                 "ranges for character class expression '['");
    } else if (ch == ']') {
      /* [<*>] | Add lo_rune, finish. */
      range.max = range.min;
      if ((err = re__parse_charclass_insert_range(parse, range))) {
        return err;
      }
      break;
    } else if (ch != '-') {
      /* [<*><*> | Add lo_rune, continue. */
      range.max = range.min;
      if ((err = re__parse_charclass_insert_range(parse, range))) {
        return err;
      }
      goto range_loop;
    }
    /* [<*>- | Look for end character for range */
  before_high_rune:
    if ((err = re__parse_next_char(parse, &ch))) {
      return err;
    }
    if (ch == RE__PARSE_EOF) {
      /* Cannot EOF here */
      return re__parse_error(
          parse, "expected characters, character classes, or character "
                 "ranges for character class expression '['");
    } else if (ch == ']') {
      /* [<*>-] | End character class. Add lo_rune. Add dash. */
      range.max = range.min;
      if ((err = re__parse_charclass_insert_range(parse, range))) {
        return err;
      }
      range.min = '-';
      range.max = '-';
      if ((err = re__parse_charclass_insert_range(parse, range))) {
        return err;
      }
      break;
    } else if (ch == '\\') {
      /* [<*>-\ | Ending character escape. */
      if ((err = re__parse_escape(parse, &range.max, 0, 1))) {
        return err;
      }
      if ((err = re__parse_charclass_insert_range(parse, range))) {
        return err;
      }
    } else {
      /* [<*>-<*> | Add range. */
      range.max = ch;
      if ((err = re__parse_charclass_insert_range(parse, range))) {
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
    if (re__charclass_get_num_ranges(&new_charclass) == 1) {
      re__rune_range first_range = re__charclass_get_ranges(&new_charclass)[0];
      if (first_range.min == first_range.max) {
        re__ast_init_rune(&new_node, first_range.min);
        if ((err = re__parse_link_node(parse, new_node))) {
          re__charclass_destroy(&new_charclass);
          return err;
        }
        re__charclass_destroy(&new_charclass);
        return err;
      }
    }
    /* We own new_charclass */
    if ((err = re__ast_root_add_charclass(
             &parse->reg->data->ast_root, new_charclass, &new_charclass_ref))) {
      re__charclass_destroy(&new_charclass);
      return err;
    }
    /* ast_root owns new_charclass */
    re__ast_init_charclass(&new_node, new_charclass_ref);
    if ((err = re__parse_link_node(parse, new_node))) {
      return err;
    }
  }
  return err;
}

MN_INTERNAL re_error re__parse_str(re__parse* parse, mn__str_view str)
{
  re_error err = RE_ERROR_NONE;
  /* Set string */
  parse->str = str;
  parse->str_pos = 0;
  if ((err = re__parse_push_frame(
           parse, parse->reg->data->ast_root.root_ref, 0))) {
    return err;
  }
  while (1) {
    re_rune ch;
    if ((err = re__parse_next_char(parse, &ch))) {
      goto error;
    }
    if (ch == '$') {
      re__assert_type assert_type = RE__ASSERT_TYPE_TEXT_END_ABSOLUTE;
      if (re__parse_get_frame(parse)->flags & RE__PARSE_FLAG_MULTILINE) {
        assert_type = RE__ASSERT_TYPE_TEXT_END;
      }
      if ((err = re__parse_create_assert(parse, assert_type))) {
        goto error;
      }
    } else if (ch == '(') {
      /* ( | Begin a group. */
      if ((err = re__parse_group_begin(parse))) {
        goto error;
      }
    } else if (ch == ')') {
      /* ) | End a group. */
      if ((err = re__parse_group_end(parse))) {
        goto error;
      }
    } else if (ch == '*' || ch == '?' || ch == '+') {
      mn_size saved_pos;
      if (ch == '*') {
        if ((err = re__parse_create_star(parse))) {
          goto error;
        }
      } else if (ch == '?') {
        if ((err = re__parse_create_question(parse))) {
          goto error;
        }
      } else { /* ch == '+' */
        if ((err = re__parse_create_plus(parse))) {
          goto error;
        }
      }
      saved_pos = parse->str_pos;
      if ((err = re__parse_next_char(parse, &ch))) {
        goto error;
      }
      if (ch == '?') {
        /* [*+?]? | Make previous operator non-greedy. */
        re__parse_swap_greedy(parse);
      } else {
        /* Handles EOF */
        parse->str_pos = saved_pos;
      }
    } else if (ch == '.') {
      /* . | Create an "any character." */
      if ((err = re__parse_create_any_char(parse))) {
        goto error;
      }
    } else if (ch == '[') {
      /* [ | Start of a character class. */
      if ((err = re__parse_charclass(parse))) {
        goto error;
      }
    } else if (ch == '\\') {
      /* \ | Start of escape sequence. */
      re_rune esc_char;
      if ((err = re__parse_escape(parse, &esc_char, 1, 0))) {
        goto error;
      }
      if (esc_char != RE__PARSE_EOF) {
        if ((err = re__parse_create_rune(parse, esc_char))) {
          goto error;
        }
      }
    } else if (ch == '^') {
      /* ^ | Text start assert. */
      re__assert_type assert_type = RE__ASSERT_TYPE_TEXT_START_ABSOLUTE;
      if (re__parse_get_frame(parse)->flags & RE__PARSE_FLAG_MULTILINE) {
        assert_type = RE__ASSERT_TYPE_TEXT_START;
      }
      if ((err = re__parse_create_assert(parse, assert_type))) {
        goto error;
      }
    } else if (ch == '{') {
      /* { | Start of counting form. */
      /*if ((err = re__parse_count(parse))) {
        goto error;
      }*/
    } else if (ch == '|') {
      /* | | Alternation. */
      if ((err = re__parse_alt(parse))) {
        goto error;
      }
    } else if (ch == RE__PARSE_EOF) {
      /* <EOF> | Finish. */
      break;
    } else {
      /* Any other character. */
      if ((err = re__parse_create_rune(parse, ch))) {
        goto error;
      }
    }
  }
  /* Pop all frames. */
  while (1) {
    re__ast_type peek_type = re__parse_get_frame_type(parse);
    if (peek_type == RE__AST_TYPE_NONE) {
      /* Successfully hit bottom frame. */
      break;
    } else if (peek_type == RE__AST_TYPE_CONCAT) {
      re__parse_pop_frame(parse);
    } else if (peek_type == RE__AST_TYPE_ALT) {
      if ((err = re__parse_alt_finish(parse))) {
        goto error;
      }
      re__parse_pop_frame(parse);
    } else { /* peek_type == RE__AST_TYPE_GROUP */
      /* If we find a group, that means it has not been closed. */
      err = re__parse_error(parse, "unmatched '('");
      goto error;
    }
  }
error:
  return err;
}

#undef RE__PARSE_EOF

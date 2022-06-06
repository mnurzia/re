#include "test_compile.h"

#include "test_ast.h"
#include "test_helpers.h"
#include "test_prog.h"
#include "test_range.h"

TEST(t_compile_empty)
{
  re__ast_root ast_root;
  re__prog prog;
  re__compile compile;
  re_error err = RE_ERROR_NONE;
  SYM(re__ast_root, "(ast)", &ast_root);
  re__prog_init(&prog);
  re__compile_init(&compile);
  if ((err = re__compile_regex(&compile, &ast_root, &prog, 0)) ==
      RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  ASSERT_SYMEQ(
      re__prog, prog,
      "(prog"
      "  (fail)"
      "  (match 1 0))");
error:
  re__compile_destroy(&compile);
  re__prog_destroy(&prog);
  re__ast_root_destroy(&ast_root);
  PASS();
}

TEST(t_compile_rune_ascii)
{
  re__ast_root ast_root;
  re__prog prog;
  re__compile compile;
  re_error err = RE_ERROR_NONE;
  SYM(re__ast_root,
      "(ast"
      "  (rune 'a'))",
      &ast_root);
  re__prog_init(&prog);
  re__compile_init(&compile);
  if ((err = re__compile_regex(&compile, &ast_root, &prog, 0)) ==
      RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  ASSERT_SYMEQ(
      re__prog, prog,
      "(prog"
      "  (fail)"
      "  (byte 'a' 2)"
      "  (match 1 0))");
error:
  re__compile_destroy(&compile);
  re__prog_destroy(&prog);
  re__ast_root_destroy(&ast_root);
  PASS();
}

TEST(t_compile_rune_ascii_reversed)
{
  re__ast_root ast_root;
  re__prog prog;
  re__compile compile;
  re_error err = RE_ERROR_NONE;
  SYM(re__ast_root,
      "(ast"
      "  (rune 'a'))",
      &ast_root);
  re__prog_init(&prog);
  re__compile_init(&compile);
  if ((err = re__compile_regex(&compile, &ast_root, &prog, 0)) ==
      RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  ASSERT_SYMEQ(
      re__prog, prog,
      "(prog"
      "  (fail)"
      "  (byte 'a' 2)"
      "  (match 1 0))");
error:
  re__compile_destroy(&compile);
  re__prog_destroy(&prog);
  re__ast_root_destroy(&ast_root);
  PASS();
}

TEST(t_compile_rune_unicode)
{
  re__ast_root ast_root;
  re__prog prog, prog_actual;
  re__compile compile;
  re__ast ast;
  mn_int32 ast_ref;
  re_rune rune = re_rune_rand();
  mn_uint8 chars[16];
  mn_uint32 chars_len = (mn_uint32)re__compile_gen_utf8(rune, chars);
  re__prog_loc i;
  re__prog_inst inst;
  re_error err = RE_ERROR_NONE;
  re__ast_init_rune(&ast, rune);
  re__ast_root_init(&ast_root);
  re__prog_init(&prog);
  re__prog_init(&prog_actual);
  re__compile_init(&compile);
  if ((err = re__ast_root_add_child(&ast_root, RE__AST_NONE, ast, &ast_ref)) ==
      RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  ast_root.depth_max = 1;
  re__prog_inst_init_fail(&inst);
  if ((err = re__prog_add(&prog, inst)) == RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  for (i = 0; i < chars_len; i++) {
    re__prog_inst_init_byte(&inst, chars[i]);
    re__prog_inst_set_primary(&inst, 2 + i);
    if ((err = re__prog_add(&prog, inst)) == RE_ERROR_NOMEM) {
      goto error;
    } else if (err) {
      FAIL();
    }
  }
  re__prog_inst_init_match(&inst, 1);
  if ((err = re__prog_add(&prog, inst)) == RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  if ((err = re__compile_regex(&compile, &ast_root, &prog_actual, 0)) ==
      RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  ASSERT(re__prog_equals(&prog, &prog_actual));
error:
  re__prog_destroy(&prog);
  re__prog_destroy(&prog_actual);
  re__compile_destroy(&compile);
  re__ast_root_destroy(&ast_root);
  PASS();
}

TEST(t_compile_str)
{
  re__ast_root ast_root;
  re__prog prog;
  re__compile compile;
  re_error err = RE_ERROR_NONE;
  SYM(re__ast_root,
      "(ast"
      "  (str test))",
      &ast_root);
  re__prog_init(&prog);
  re__compile_init(&compile);
  if ((err = re__compile_regex(&compile, &ast_root, &prog, 0)) ==
      RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  ASSERT_SYMEQ(
      re__prog, prog,
      "(prog"
      "  (fail)"
      "  (byte 't' 2)"
      "  (byte 'e' 3)"
      "  (byte 's' 4)"
      "  (byte 't' 5)"
      "  (match 1 0))");
error:
  re__compile_destroy(&compile);
  re__prog_destroy(&prog);
  re__ast_root_destroy(&ast_root);
  PASS();
}

TEST(t_compile_str_thrash)
{
  re__ast_root ast_root;
  re__prog prog, prog_actual;
  re__compile compile;
  re__ast ast;
  mn_int32 ast_ref;
  mn_int32 str_ref;
  mn__str in_str, copy_str;
  re__prog_loc i;
  re__prog_inst inst;
  mn_size j, num_runes = RAND_PARAM(600);
  re_error err = RE_ERROR_NONE;
  mn__str_init(&in_str);
  mn__str_init(&copy_str);
  re__ast_root_init(&ast_root);
  re__prog_init(&prog);
  re__prog_init(&prog_actual);
  re__compile_init(&compile);
  for (j = 0; j < num_runes; j++) {
    mn_uint8 chars[16];
    re_rune rune = re_rune_rand();
    mn_uint32 chars_len = (mn_uint32)re__compile_gen_utf8(rune, chars);
    if ((err = mn__str_cat_n(&in_str, (mn_char*)chars, chars_len)) ==
        RE_ERROR_NOMEM) {
      goto error;
    } else if (err) {
      FAIL();
    }
  }
  if ((err = mn__str_init_copy(&copy_str, &in_str)) == RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  if ((err = re__ast_root_add_str(&ast_root, copy_str, &str_ref)) ==
      RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  re__ast_init_str(&ast, str_ref);
  if ((err = re__ast_root_add_child(&ast_root, RE__AST_NONE, ast, &ast_ref)) ==
      RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  ast_root.depth_max = 1;
  re__prog_inst_init_fail(&inst);
  if ((err = re__prog_add(&prog, inst)) == RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  for (i = 0; i < (re__prog_loc)mn__str_size(&in_str); i++) {
    re__prog_inst_init_byte(&inst, (mn_uint8)mn__str_get_data(&in_str)[i]);
    re__prog_inst_set_primary(&inst, 2 + i);
    if ((err = re__prog_add(&prog, inst)) == RE_ERROR_NOMEM) {
      goto error;
    } else if (err) {
      FAIL();
    }
  }
  re__prog_inst_init_match(&inst, 1);
  if ((err = re__prog_add(&prog, inst)) == RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  if ((err = re__compile_regex(&compile, &ast_root, &prog_actual, 0)) ==
      RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  ASSERT(re__prog_equals(&prog, &prog_actual));
error:
  re__prog_destroy(&prog);
  re__prog_destroy(&prog_actual);
  re__compile_destroy(&compile);
  re__ast_root_destroy(&ast_root);
  mn__str_destroy(&in_str);
  PASS();
}

TEST(t_compile_concat)
{
  re__ast_root ast_root;
  re__prog prog, prog_actual;
  re__compile compile;
  re_error err = RE_ERROR_NONE;
  SYM(re__ast_root,
      "(ast"
      "    (concat"
      "        ("
      "            (rune 'a')"
      "            (rune 'b'))))",
      &ast_root);
  SYM(re__prog,
      "(prog"
      "    (fail)"
      "    (byte 'a' 2)"
      "    (byte 'b' 3)"
      "    (match 1 0)",
      &prog);
  re__compile_init(&compile);
  re__prog_init(&prog_actual);
  if ((err = re__compile_regex(&compile, &ast_root, &prog_actual, 0)) ==
      RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  ASSERT(re__prog_equals(&prog, &prog_actual));
error:
  re__compile_destroy(&compile);
  re__prog_destroy(&prog);
  re__prog_destroy(&prog_actual);
  re__ast_root_destroy(&ast_root);
  PASS();
}

TEST(t_compile_alt)
{
  re__ast_root ast_root;
  re__prog prog, prog_actual;
  re__compile compile;
  re_error err = RE_ERROR_NONE;
  SYM(re__ast_root,
      "(ast"
      "    (alt"
      "        ("
      "            (rune 'a')"
      "            (rune 'b'))))",
      &ast_root);
  SYM(re__prog,
      "(prog"
      "    (fail)"
      "    (split 2 3)"
      "    (byte 'a' 4)"
      "    (byte 'b' 4)"
      "    (match 1 0)",
      &prog);
  re__compile_init(&compile);
  re__prog_init(&prog_actual);
  if ((err = re__compile_regex(&compile, &ast_root, &prog_actual, 0)) ==
      RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  ASSERT(re__prog_equals(&prog, &prog_actual));
error:
  re__compile_destroy(&compile);
  re__prog_destroy(&prog);
  re__prog_destroy(&prog_actual);
  re__ast_root_destroy(&ast_root);
  PASS();
}

TEST(t_compile_alts)
{
  re__ast_root ast_root;
  re__prog prog, prog_actual;
  re__compile compile;
  re_error err = RE_ERROR_NONE;
  SYM(re__ast_root,
      "(ast"
      "    (alt"
      "        ("
      "            (rune 'a')"
      "            (rune 'b')"
      "            (rune 'c'))))",
      &ast_root);
  SYM(re__prog,
      "(prog"
      "    (fail)"
      "    (split 2 3)"
      "    (byte 'a' 6)"
      "    (split 4 5)"
      "    (byte 'b' 6)"
      "    (byte 'c' 6)"
      "    (match 1 0)",
      &prog);
  re__compile_init(&compile);
  re__prog_init(&prog_actual);
  if ((err = re__compile_regex(&compile, &ast_root, &prog_actual, 0)) ==
      RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  ASSERT(re__prog_equals(&prog, &prog_actual));
error:
  re__compile_destroy(&compile);
  re__prog_destroy(&prog);
  re__prog_destroy(&prog_actual);
  re__ast_root_destroy(&ast_root);
  PASS();
}

TEST(t_compile_star)
{
  re__ast_root ast_root;
  re__prog prog, prog_actual;
  re__compile compile;
  re_error err = RE_ERROR_NONE;
  SYM(re__ast_root,
      "(ast"
      "    (quantifier 0 inf greedy"
      "        (rune 'a')))",
      &ast_root);
  SYM(re__prog,
      "(prog"
      "    (fail)"
      "    (split 2 3)"
      "    (byte 'a' 1)"
      "    (match 1 0)",
      &prog);
  re__compile_init(&compile);
  re__prog_init(&prog_actual);
  if ((err = re__compile_regex(&compile, &ast_root, &prog_actual, 0)) ==
      RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  ASSERT(re__prog_equals(&prog, &prog_actual));
error:
  re__compile_destroy(&compile);
  re__prog_destroy(&prog);
  re__prog_destroy(&prog_actual);
  re__ast_root_destroy(&ast_root);
  PASS();
}

TEST(t_compile_star_nongreedy)
{
  re__ast_root ast_root;
  re__prog prog, prog_actual;
  re__compile compile;
  re_error err = RE_ERROR_NONE;
  SYM(re__ast_root,
      "(ast"
      "    (quantifier 0 inf nongreedy"
      "        (rune 'a')))",
      &ast_root);
  SYM(re__prog,
      "(prog"
      "    (fail)"
      "    (split 3 2)"
      "    (byte 'a' 1)"
      "    (match 1 0)",
      &prog);
  re__compile_init(&compile);
  re__prog_init(&prog_actual);
  if ((err = re__compile_regex(&compile, &ast_root, &prog_actual, 0)) ==
      RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  ASSERT(re__prog_equals(&prog, &prog_actual));
error:
  re__compile_destroy(&compile);
  re__prog_destroy(&prog);
  re__prog_destroy(&prog_actual);
  re__ast_root_destroy(&ast_root);
  PASS();
}

TEST(t_compile_question)
{
  re__ast_root ast_root;
  re__prog prog, prog_actual;
  re__compile compile;
  re_error err = RE_ERROR_NONE;
  SYM(re__ast_root,
      "(ast"
      "    (quantifier 0 2 greedy"
      "        (rune 'a')))",
      &ast_root);
  SYM(re__prog,
      "(prog"
      "    (fail)"
      "    (split 2 3)"
      "    (byte 'a' 3)"
      "    (match 1 0)",
      &prog);
  re__compile_init(&compile);
  re__prog_init(&prog_actual);
  if ((err = re__compile_regex(&compile, &ast_root, &prog_actual, 0)) ==
      RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  ASSERT(re__prog_equals(&prog, &prog_actual));
error:
  re__compile_destroy(&compile);
  re__prog_destroy(&prog);
  re__prog_destroy(&prog_actual);
  re__ast_root_destroy(&ast_root);
  PASS();
}

TEST(t_compile_question_nongreedy)
{
  re__ast_root ast_root;
  re__prog prog, prog_actual;
  re__compile compile;
  re_error err = RE_ERROR_NONE;
  SYM(re__ast_root,
      "(ast"
      "    (quantifier 0 2 nongreedy"
      "        (rune 'a')))",
      &ast_root);
  SYM(re__prog,
      "(prog"
      "    (fail)"
      "    (split 3 2)"
      "    (byte 'a' 3)"
      "    (match 1 0)",
      &prog);
  re__compile_init(&compile);
  re__prog_init(&prog_actual);
  if ((err = re__compile_regex(&compile, &ast_root, &prog_actual, 0)) ==
      RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  ASSERT(re__prog_equals(&prog, &prog_actual));
error:
  re__compile_destroy(&compile);
  re__prog_destroy(&prog);
  re__prog_destroy(&prog_actual);
  re__ast_root_destroy(&ast_root);
  PASS();
}

TEST(t_compile_plus)
{
  re__ast_root ast_root;
  re__prog prog, prog_actual;
  re__compile compile;
  re_error err = RE_ERROR_NONE;
  SYM(re__ast_root,
      "(ast"
      "    (quantifier 1 inf greedy"
      "        (rune 'a')))",
      &ast_root);
  SYM(re__prog,
      "(prog"
      "    (fail)"
      "    (byte 'a' 2)"
      "    (split 1 3)"
      "    (match 1 0)",
      &prog);
  re__compile_init(&compile);
  re__prog_init(&prog_actual);
  if ((err = re__compile_regex(&compile, &ast_root, &prog_actual, 0)) ==
      RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  ASSERT(re__prog_equals(&prog, &prog_actual));
error:
  re__compile_destroy(&compile);
  re__prog_destroy(&prog);
  re__prog_destroy(&prog_actual);
  re__ast_root_destroy(&ast_root);
  PASS();
}

TEST(t_compile_plus_nongreedy)
{
  re__ast_root ast_root;
  re__prog prog, prog_actual;
  re__compile compile;
  re_error err = RE_ERROR_NONE;
  SYM(re__ast_root,
      "(ast"
      "    (quantifier 1 inf nongreedy"
      "        (rune 'a')))",
      &ast_root);
  SYM(re__prog,
      "(prog"
      "    (fail)"
      "    (byte 'a' 2)"
      "    (split 3 1)"
      "    (match 1 0)",
      &prog);
  re__compile_init(&compile);
  re__prog_init(&prog_actual);
  if ((err = re__compile_regex(&compile, &ast_root, &prog_actual, 0)) ==
      RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  ASSERT(re__prog_equals(&prog, &prog_actual));
error:
  re__compile_destroy(&compile);
  re__prog_destroy(&prog);
  re__prog_destroy(&prog_actual);
  re__ast_root_destroy(&ast_root);
  PASS();
}

TEST(t_compile_quantifier_nomax)
{
  re__ast_root ast_root;
  re__prog prog, prog_actual;
  re__compile compile;
  re_error err = RE_ERROR_NONE;
  SYM(re__ast_root,
      "(ast"
      "    (quantifier 3 inf greedy"
      "        (rune 'a')))",
      &ast_root);
  SYM(re__prog,
      "(prog"
      "    (fail)"
      "    (byte 'a' 2)"
      "    (byte 'a' 3)"
      "    (byte 'a' 4)"
      "    (split 3 5)"
      "    (match 1 0))",
      &prog);
  re__compile_init(&compile);
  re__prog_init(&prog_actual);
  if ((err = re__compile_regex(&compile, &ast_root, &prog_actual, 0)) ==
      RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  ASSERT(re__prog_equals(&prog, &prog_actual));
error:
  re__compile_destroy(&compile);
  re__prog_destroy(&prog);
  re__prog_destroy(&prog_actual);
  re__ast_root_destroy(&ast_root);
  PASS();
}

TEST(t_compile_quantifier_nomax_nongreedy)
{
  re__ast_root ast_root;
  re__prog prog, prog_actual;
  re__compile compile;
  re_error err = RE_ERROR_NONE;
  SYM(re__ast_root,
      "(ast"
      "    (quantifier 3 inf nongreedy"
      "        (rune 'a')))",
      &ast_root);
  SYM(re__prog,
      "(prog"
      "    (fail)"
      "    (byte 'a' 2)"
      "    (byte 'a' 3)"
      "    (byte 'a' 4)"
      "    (split 5 3)"
      "    (match 1 0))",
      &prog);
  re__compile_init(&compile);
  re__prog_init(&prog_actual);
  if ((err = re__compile_regex(&compile, &ast_root, &prog_actual, 0)) ==
      RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  ASSERT(re__prog_equals(&prog, &prog_actual));
error:
  re__compile_destroy(&compile);
  re__prog_destroy(&prog);
  re__prog_destroy(&prog_actual);
  re__ast_root_destroy(&ast_root);
  PASS();
}

TEST(t_compile_quantifier_minmax)
{
  re__ast_root ast_root;
  re__prog prog, prog_actual;
  re__compile compile;
  re_error err = RE_ERROR_NONE;
  SYM(re__ast_root,
      "(ast"
      "    (quantifier 3 6 greedy"
      "        (rune 'a')))",
      &ast_root);
  SYM(re__prog,
      "(prog"
      "    (fail)"
      "    (byte 'a' 2)"
      "    (byte 'a' 3)"
      "    (byte 'a' 4)"
      "    (split 5 8)"
      "    (byte 'a' 6)"
      "    (split 7 8)"
      "    (byte 'a' 8)"
      "    (match 1 0))",
      &prog);
  re__compile_init(&compile);
  re__prog_init(&prog_actual);
  if ((err = re__compile_regex(&compile, &ast_root, &prog_actual, 0)) ==
      RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  ASSERT(re__prog_equals(&prog, &prog_actual));
error:
  re__compile_destroy(&compile);
  re__prog_destroy(&prog);
  re__prog_destroy(&prog_actual);
  re__ast_root_destroy(&ast_root);
  PASS();
}

TEST(t_compile_quantifier_minmax_nongreedy)
{
  re__ast_root ast_root;
  re__prog prog, prog_actual;
  re__compile compile;
  re_error err = RE_ERROR_NONE;
  SYM(re__ast_root,
      "(ast"
      "    (quantifier 3 6 nongreedy"
      "        (rune 'a')))",
      &ast_root);
  SYM(re__prog,
      "(prog"
      "    (fail)"
      "    (byte 'a' 2)"
      "    (byte 'a' 3)"
      "    (byte 'a' 4)"
      "    (split 8 5)"
      "    (byte 'a' 6)"
      "    (split 8 7)"
      "    (byte 'a' 8)"
      "    (match 1 0))",
      &prog);
  re__compile_init(&compile);
  re__prog_init(&prog_actual);
  if ((err = re__compile_regex(&compile, &ast_root, &prog_actual, 0)) ==
      RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  ASSERT(re__prog_equals(&prog, &prog_actual));
error:
  re__compile_destroy(&compile);
  re__prog_destroy(&prog);
  re__prog_destroy(&prog_actual);
  re__ast_root_destroy(&ast_root);
  PASS();
}

TEST(t_compile_group)
{
  re__ast_root ast_root;
  re__prog prog, prog_actual;
  re__compile compile;
  re_error err = RE_ERROR_NONE;
  SYM(re__ast_root,
      "(ast"
      "    (group () 0"
      "        (rune 'a')))",
      &ast_root);
  SYM(re__prog,
      "(prog"
      "    (fail)"
      "    (save 0 2)"
      "    (byte 'a' 3)"
      "    (save 1 4)"
      "    (match 1 0))",
      &prog);
  re__compile_init(&compile);
  re__prog_init(&prog_actual);
  if ((err = re__compile_regex(&compile, &ast_root, &prog_actual, 0)) ==
      RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  ASSERT(re__prog_equals(&prog, &prog_actual));
error:
  re__compile_destroy(&compile);
  re__prog_destroy(&prog);
  re__prog_destroy(&prog_actual);
  re__ast_root_destroy(&ast_root);
  PASS();
}

TEST(t_compile_group_nonmatching)
{
  re__ast_root ast_root;
  re__prog prog, prog_actual;
  re__compile compile;
  re_error err = RE_ERROR_NONE;
  SYM(re__ast_root,
      "(ast"
      "    (group (nonmatching)"
      "        (rune 'a')))",
      &ast_root);
  SYM(re__prog,
      "(prog"
      "    (fail)"
      "    (byte 'a' 2)"
      "    (match 1 0))",
      &prog);
  re__compile_init(&compile);
  re__prog_init(&prog_actual);
  if ((err = re__compile_regex(&compile, &ast_root, &prog_actual, 0)) ==
      RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  ASSERT(re__prog_equals(&prog, &prog_actual));
error:
  re__compile_destroy(&compile);
  re__prog_destroy(&prog);
  re__prog_destroy(&prog_actual);
  re__ast_root_destroy(&ast_root);
  PASS();
}

TEST(t_compile_assert)
{
  re__ast_root ast_root;
  re__prog prog, prog_actual;
  re__compile compile;
  re_error err = RE_ERROR_NONE;
  SYM(re__ast_root,
      "(ast"
      "    (assert (text_start text_end word word_not)))",
      &ast_root);
  SYM(re__prog,
      "(prog"
      "    (fail)"
      "    (assert (text_start text_end word word_not) 2)"
      "    (match 1 0))",
      &prog);
  re__compile_init(&compile);
  re__prog_init(&prog_actual);
  if ((err = re__compile_regex(&compile, &ast_root, &prog_actual, 0)) ==
      RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  ASSERT(re__prog_equals(&prog, &prog_actual));
error:
  re__compile_destroy(&compile);
  re__prog_destroy(&prog);
  re__prog_destroy(&prog_actual);
  re__ast_root_destroy(&ast_root);
  PASS();
}

/* 509 characters?? Really??? */
char* t_compile_any_char_text0 = "(prog "
                                 "    (fail)"
                                 "    (split 2 3)"
                                 "    (byte_range 0x0 0x7F 25)"
                                 "    (split 4 6)"
                                 "    (byte_range 0xC2 0xDF 5)"
                                 "    (byte_range 0x80 0xBF 25)"
                                 "    (split 7 9)"
                                 "    (byte 0xE0 8)"
                                 "    (byte_range 0xA0 0xBF 5)"
                                 "    (split 10 12)"
                                 "    (byte_range 0xE1 0xEC 11)"
                                 "    (byte_range 0x80 0xBF 5)"
                                 "    (split 13 15)";
char* t_compile_any_char_text1 = "    (byte 0xED 14)"
                                 "    (byte_range 0x80 0x9F 5)"
                                 "    (split 16 17)"
                                 "    (byte_range 0xEE 0xEF 11)"
                                 "    (split 18 20)"
                                 "    (byte 0xF0 19)"
                                 "    (byte_range 0x90 0xBF 11)"
                                 "    (split 21 23)"
                                 "    (byte_range 0xF1 0xF3 22)"
                                 "    (byte_range 0x80 0xBF 11)"
                                 "    (byte 0xF4 24)"
                                 "    (byte_range 0x80 0x8F 11)"
                                 "    (match 1 0))";

TEST(t_compile_any_char)
{
  re__ast_root ast_root;
  re__prog prog_actual;
  re__compile compile;
  mn__str cmpstr;
  re_error err = RE_ERROR_NONE;
  SYM(re__ast_root,
      "(ast"
      "    (any_char))",
      &ast_root);
  re__compile_init(&compile);
  re__prog_init(&prog_actual);
  mn__str_init_s(&cmpstr, t_compile_any_char_text0);
  if ((err = re__compile_regex(&compile, &ast_root, &prog_actual, 0)) ==
      RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  if ((err = mn__str_cat_s(&cmpstr, t_compile_any_char_text1)) ==
      RE_ERROR_NOMEM) {
    goto error;
  } else if (err) {
    FAIL();
  }
  ASSERT_SYMEQ(re__prog, prog_actual, mn__str_get_data(&cmpstr));
error:
  mn__str_destroy(&cmpstr);
  re__compile_destroy(&compile);
  re__prog_destroy(&prog_actual);
  re__ast_root_destroy(&ast_root);
  PASS();
}

SUITE(s_compile)
{
  RUN_TEST(t_compile_empty);
  RUN_TEST(t_compile_rune_ascii);
  RUN_TEST(t_compile_rune_ascii_reversed);
  FUZZ_TEST(t_compile_rune_unicode);
  RUN_TEST(t_compile_str);
  FUZZ_TEST(t_compile_str_thrash);
  RUN_TEST(t_compile_concat);
  RUN_TEST(t_compile_alt);
  RUN_TEST(t_compile_alts);
  RUN_TEST(t_compile_star);
  RUN_TEST(t_compile_star_nongreedy);
  RUN_TEST(t_compile_question);
  RUN_TEST(t_compile_question_nongreedy);
  RUN_TEST(t_compile_plus);
  RUN_TEST(t_compile_plus_nongreedy);
  RUN_TEST(t_compile_quantifier_nomax);
  RUN_TEST(t_compile_quantifier_nomax_nongreedy);
  RUN_TEST(t_compile_quantifier_minmax);
  RUN_TEST(t_compile_quantifier_minmax_nongreedy);
  RUN_TEST(t_compile_group);
  RUN_TEST(t_compile_group_nonmatching);
  RUN_TEST(t_compile_assert);
  RUN_TEST(t_compile_any_char);
}

#if !defined(MPTEST_H)
#define MPTEST_H

/* desc */
/* cppreference */
#if !defined(MPTEST_ASSERT)
#include <assert.h>
#define MPTEST_ASSERT assert
#endif

/* bits/hooks/longjmp */
/* Set to 1 in order to define setjmp(), longjmp(), and jmp_buf replacements. */
#if !defined(MPTEST_USE_CUSTOM_LONGJMP)
#include <setjmp.h>
#define MPTEST_SETJMP setjmp
#define MPTEST_LONGJMP longjmp
#define MPTEST_JMP_BUF jmp_buf
#endif

/* bits/hooks/malloc */
/* Set to 1 in order to define malloc(), free(), and realloc() replacements. */
#if !defined(MPTEST_USE_CUSTOM_ALLOCATOR)
#include <stdlib.h>
#define MPTEST_MALLOC malloc
#define MPTEST_REALLOC realloc
#define MPTEST_FREE free
#endif

/* bits/types/size */
/* desc */
/* cppreference */
#if !defined(MPTEST_SIZE_TYPE)
#include <stdlib.h>
#define MPTEST_SIZE_TYPE size_t
#endif

/* bits/util/debug */
/* Set to 1 in order to override the setting of the NDEBUG variable. */
#if !defined(MPTEST_DEBUG)
#define MPTEST_DEBUG 0
#endif

/* bits/util/exports */
/* Set to 1 in order to define all symbols with static linkage (local to the
 * including source file) as opposed to external linkage. */
#if !defined(MPTEST_STATIC)
#define MPTEST_STATIC 0
#endif

/* bits/types/char */
/* desc */
/* cppreference */
#if !defined(MPTEST_CHAR_TYPE)
#define MPTEST_CHAR_TYPE char
#endif

#if MPTEST_USE_SYM
/* bits/types/fixed/int32 */
/* desc */
/* cppreference */
#if !defined(MPTEST_INT32_TYPE)
#endif
#endif /* MPTEST_USE_SYM */

/* mptest */
/* Help text */
#if !defined(MPTEST_USE_DYN_ALLOC)
#define MPTEST_USE_DYN_ALLOC 1
#endif

/* mptest */
/* Help text */
#if !defined(MPTEST_USE_LONGJMP)
#define MPTEST_USE_LONGJMP 1
#endif

/* mptest */
/* Help text */
#if !defined(MPTEST_USE_SYM)
#define MPTEST_USE_SYM 1
#endif

/* mptest */
/* Help text */
#if !defined(MPTEST_USE_LEAKCHECK)
#define MPTEST_USE_LEAKCHECK 1
#endif

/* mptest */
/* Help text */
#if !defined(MPTEST_USE_COLOR)
#define MPTEST_USE_COLOR 1
#endif

/* mptest */
/* Help text */
#if !defined(MPTEST_USE_TIME)
#define MPTEST_USE_TIME 1
#endif

/* mptest */
/* Help text */
#if !defined(MPTEST_USE_APARSE)
#define MPTEST_USE_APARSE 1
#endif

/* mptest */
/* Help text */
#if !defined(MPTEST_USE_FUZZ)
#define MPTEST_USE_FUZZ 1
#endif

/* mptest */
/* Help text */
#if !defined(MPTEST_DETECT_UNCAUGHT_ASSERTS)
#define MPTEST_DETECT_UNCAUGHT_ASSERTS 1
#endif

/* bits/types/size */
typedef MPTEST_SIZE_TYPE mptest_size;

/* bits/util/cstd */
/* If __STDC__ is not defined, assume C89. */
#ifndef __STDC__
    #define MPTEST__CSTD 1989
#else
    #if defined(__STDC_VERSION__)
        #if __STDC_VERSION__ >= 201112L
            #define MPTEST__CSTD 2011
        #elif __STDC_VERSION__ >= 199901L
            #define MPTEST__CSTD 1999
        #else
            #define MPTEST__CSTD 1989
        #endif
    #endif
#endif

/* bits/util/debug */
#if !MPTEST_DEBUG
#if defined(NDEBUG)
#if NDEBUG == 0
#define MPTEST_DEBUG 1
#else
#define MPTEST_DEBUG 0
#endif
#endif
#endif

/* bits/util/exports */
#if !defined(MPTEST__SPLIT_BUILD)
#if MPTEST_STATIC
#define MPTEST_API static
#else
#define MPTEST_API extern
#endif
#else
#define MPTEST_API extern
#endif

/* bits/util/null */
#define MPTEST_NULL 0

/* bits/types/char */
#if !defined(MPTEST_CHAR_TYPE)
#define MPTEST_CHAR_TYPE char
#endif

typedef MPTEST_CHAR_TYPE mptest_char;

#if MPTEST_USE_SYM
/* bits/types/fixed/int32 */
#if !defined(MPTEST_INT32_TYPE)
#if MPTEST__CSTD >= 1999
#include <stdint.h>
#define MPTEST_INT32_TYPE int32_t
#else
#define MPTEST_INT32_TYPE signed int
#endif
#endif

typedef MPTEST_INT32_TYPE mptest_int32;
#endif /* MPTEST_USE_SYM */

#if MPTEST_USE_APARSE
/* aparse */
#include <stddef.h>
#define APARSE_ERROR_NONE 0
#define APARSE_ERROR_NOMEM -1
#define APARSE_ERROR_PARSE -2
#define APARSE_ERROR_OUT -3
#define APARSE_ERROR_INVALID -4
#define APARSE_ERROR_SHOULD_EXIT -5
#define APARSE_SHOULD_EXIT APARSE_ERROR_SHOULD_EXIT

/* Syntax validity table: */
/* Where `O` is the option in question, `s` is a subargument to `O`, `P` is
 * another option (could be the same option), and `o` is the long option form
 * of `O`. */
/* |  args       | -O  | -O=s | -OP | -Os | -O s | --o | --o=s | --o s |
 * | ----------- | --- | ---- | --- | --- | ---- | --- | ----- | ----- |
 * | 2+          |     |      |     |     | *    |     |       | *     |
 * | 1           |     | *    |     | *   | *    |     | *     | *     |
 * | 0           | *   |      | *   |     |      | *   |       |       |
 * | <0_OR_1>    | *   | *    | *   | *   | *    | *   | *     | *     |
 * | <0_OR_1_EQ> | *   | *    | *   |     |      |     | *     |       |
 * | <0_OR_MORE> | *   | *    | *   | *   | *    | *   | *     | *     |
 * | <1_OR_MORE> |     | *    |     | *   | *    |     | *     | *     | */
typedef enum aparse_nargs
{
    /* Parse either zero or 1 subarguments. */
    APARSE_NARGS_0_OR_1 = -1, /* Like regex '?' */
    /* Parse either zero or 1 subarguments, but only allow using '='. */
    APARSE_NARGS_0_OR_1_EQ = -2, /* Like regex '?' */
    /* Parse zero or more subarguments. */
    APARSE_NARGS_0_OR_MORE = -3, /* Like regex '*' */
    /* Parse one or more subarguments. */
    APARSE_NARGS_1_OR_MORE = -4 /* Like regex '+' */
} aparse_nargs;

typedef int aparse_error;
typedef struct aparse__state aparse__state;

typedef struct aparse_state {
    aparse__state* state;
} aparse_state;

typedef aparse_error (*aparse_out_cb)(void* user, const char* buf, mptest_size buf_size);
typedef aparse_error (*aparse_custom_cb)(void* user, aparse_state* state, int sub_arg_idx, const char* text, mptest_size text_size);

MPTEST_API aparse_error aparse_init(aparse_state* state);
MPTEST_API void aparse_set_out_cb(aparse_state* state, aparse_out_cb out_cb, void* user);
MPTEST_API void aparse_destroy(aparse_state* state);

MPTEST_API aparse_error aparse_add_opt(aparse_state* state, char short_opt, const char* long_opt);
MPTEST_API aparse_error aparse_add_pos(aparse_state* state, const char* name);
MPTEST_API aparse_error aparse_add_sub(aparse_state* state);

MPTEST_API void aparse_arg_help(aparse_state* state, const char* help_text);
MPTEST_API void aparse_arg_metavar(aparse_state* state, const char* metavar);

MPTEST_API void aparse_arg_type_bool(aparse_state* state, int* out);
MPTEST_API void aparse_arg_type_str(aparse_state* state, const char** out, mptest_size* out_size);
MPTEST_API void aparse_arg_type_help(aparse_state* state);
MPTEST_API void aparse_arg_type_version(aparse_state* state);
MPTEST_API void aparse_arg_type_custom(aparse_state* state, aparse_custom_cb cb, void* user, aparse_nargs nargs);

/* MPTEST_API void aparse_arg_int_multi(aparse_state* state, int** out, mptest_size* out_size) */
/* MPTEST_API void aparse_arg_str_multi(aparse_state* state, const char** out, mptest_size** size_out, mptest_size* num) */

MPTEST_API int aparse_sub_add_cmd(aparse_state* state, const char* name, aparse_state** subcmd);

MPTEST_API aparse_error aparse_parse(aparse_state* state, int argc, const char* const* argv);
#endif /* MPTEST_USE_APARSE */

/* mptest */
#ifndef MPTEST_API_H
#define MPTEST_API_H
/* Forward declaration */
struct mptest__state;

/* Global state object, used in all macros. */
extern struct mptest__state mptest__state_g;

typedef int mptest__result;

#define MPTEST__RESULT_PASS 0
#define MPTEST__RESULT_FAIL -1
/* an uncaught error that caused a `longjmp()` out of the test */
/* or a miscellaneous error like a sym syntax error */
#define MPTEST__RESULT_ERROR -2

#if MPTEST_USE_LEAKCHECK
typedef int mptest__leakcheck_mode;

#define MPTEST__LEAKCHECK_MODE_OFF 0
#define MPTEST__LEAKCHECK_MODE_ON 1
#define MPTEST__LEAKCHECK_MODE_OOM_ONE 2
#define MPTEST__LEAKCHECK_MODE_OOM_SET 3
#endif

/* Test function signature */
typedef mptest__result (*mptest__test_func)(void);
typedef void (*mptest__suite_func)(void);

/* Internal functions that API macros call */
MPTEST_API void mptest__state_init(struct mptest__state* state);
MPTEST_API void mptest__state_destroy(struct mptest__state* state);
MPTEST_API void mptest__state_report(struct mptest__state* state);
MPTEST_API void mptest__run_test(
    struct mptest__state* state, mptest__test_func test_func,
    const char* test_name);
MPTEST_API void mptest__run_suite(
    struct mptest__state* state, mptest__suite_func suite_func,
    const char* suite_name);

MPTEST_API void mptest__assert_fail(
    struct mptest__state* state, const char* msg, const char* assert_expr,
    const char* file, int line);
MPTEST_API void mptest__assert_pass(
    struct mptest__state* state, const char* msg, const char* assert_expr,
    const char* file, int line);

MPTEST_API void mptest_assert_fail_breakpoint(void);
MPTEST_API void mptest_uncaught_assert_fail_breakpoint(void);

MPTEST_API MPTEST_JMP_BUF* mptest__catch_assert_begin(struct mptest__state* state);
MPTEST_API void mptest__catch_assert_end(struct mptest__state* state);
MPTEST_API void mptest__catch_assert_fail(
    struct mptest__state* state, const char* msg, const char* assert_expr,
    const char* file, int line);

#if MPTEST_USE_LEAKCHECK
MPTEST_API void* mptest__leakcheck_hook_malloc(
    struct mptest__state* state, const char* file, int line, size_t size);
MPTEST_API void mptest__leakcheck_hook_free(
    struct mptest__state* state, const char* file, int line, void* ptr);
MPTEST_API void* mptest__leakcheck_hook_realloc(
    struct mptest__state* state, const char* file, int line, void* old_ptr,
    size_t new_size);
MPTEST_API void mptest__leakcheck_set(struct mptest__state* state, int on);
MPTEST_API void mptest_malloc_null_breakpoint(void);
#endif

#if MPTEST_USE_APARSE
/* declare argv as pointer to const pointer to const char */
/* can change argv, can't change *argv, can't change **argv */
MPTEST_API int mptest__state_init_argv(
    struct mptest__state* state, int argc, const char* const* argv);
#endif

#if MPTEST_USE_FUZZ
typedef unsigned long mptest_rand;
MPTEST_API void mptest__fuzz_next_test(struct mptest__state* state, int iterations);
MPTEST_API mptest_rand mptest__fuzz_rand(struct mptest__state* state);
#endif

#define _ASSERT_PASS_BEHAVIOR(expr, msg)                                       \
  do {                                                                         \
    mptest__assert_pass(&mptest__state_g, #msg, #expr, __FILE__, __LINE__);    \
  } while (0)

#define _ASSERT_FAIL_BEHAVIOR(expr, msg)                                       \
  do {                                                                         \
    mptest__assert_fail(&mptest__state_g, #msg, #expr, __FILE__, __LINE__);    \
    return MPTEST__RESULT_FAIL;                                                \
  } while (0)

/* Used for binary assertions (<, >, <=, >=, ==, !=) in order to format error
 * messages correctly. */
#define _ASSERT_BINOPm(lhs, rhs, op, msg)                                      \
  do {                                                                         \
    if (!((lhs)op(rhs))) {                                                     \
      _ASSERT_FAIL_BEHAVIOR(lhs op rhs, msg);                                  \
    } else {                                                                   \
      _ASSERT_PASS_BEHAVIOR(lhs op rhs, msg);                                  \
    }                                                                          \
  } while (0)

#define _ASSERT_BINOP(lhs, rhs, op)                                            \
  do {                                                                         \
    if (!((lhs)op(rhs))) {                                                     \
      _ASSERT_FAIL_BEHAVIOR(lhs op rhs, lhs op rhs);                           \
    } else {                                                                   \
      _ASSERT_PASS_BEHAVIOR(lhs op rhs, lhs op rhs);                           \
    }                                                                          \
  } while (0)

/* Define a test. */
/* Usage:
 * TEST(test_name) {
 *     ASSERT(...);
 *     PASS();
 * } */
#define TEST(name) mptest__result mptest__test_##name(void)

/* Define a suite. */
/* Usage:
 * SUITE(suite_name) {
 *     RUN_TEST(test_1_name);
 *     RUN_TEST(test_2_name);
 * } */
#define SUITE(name) void mptest__suite_##name(void)

/* `TEST()` and `SUITE()` macros are declared `static` because otherwise
 * -Wunused will not notice if a test is defined but not called. */

/* Run a test. Should only be used from within a suite. */
#define RUN_TEST(test)                                                         \
  do {                                                                         \
    mptest__run_test(&mptest__state_g, mptest__test_##test, #test);            \
  } while (0)

/* Run a suite. */
#define RUN_SUITE(suite)                                                       \
  do {                                                                         \
    mptest__run_suite(&mptest__state_g, mptest__suite_##suite, #suite);        \
  } while (0)

#if MPTEST_USE_FUZZ

#define MPTEST__FUZZ_DEFAULT_ITERATIONS 500

/* Run a test a number of times, changing the RNG state each time. */
#define FUZZ_TEST(test)                                                        \
  do {                                                                         \
    mptest__fuzz_next_test(&mptest__state_g, MPTEST__FUZZ_DEFAULT_ITERATIONS); \
    RUN_TEST(test);                                                            \
  } while (0)

#endif

/* Unconditionally pass a test. */
#define PASS()                                                                 \
  do {                                                                         \
    return MPTEST__RESULT_PASS;                                                \
  } while (0)

#define ASSERTm(expr, msg)                                                     \
  do {                                                                         \
    if (!(expr)) {                                                             \
      _ASSERT_FAIL_BEHAVIOR(expr, msg);                                        \
    } else {                                                                   \
      _ASSERT_PASS_BEHAVIOR(lhs op rhs, msg);                                  \
    }                                                                          \
  } while (0)

/* Unconditionally fail a test. */
#define FAIL()                                                                 \
  do {                                                                         \
    _ASSERT_FAIL_BEHAVIOR("0", "FAIL() called");                               \
  } while (0)

#define ASSERT(expr) ASSERTm(expr, #expr)

#define ASSERT_EQm(lhs, rhs, msg) _ASSERT_BINOPm(lhs, rhs, ==, msg)
#define ASSERT_NEQm(lhs, rhs, msg) _ASSERT_BINOPm(lhs, rhs, !=, msg)
#define ASSERT_GTm(lhs, rhs, msg) _ASSERT_BINOPm(lhs, rhs, >, msg)
#define ASSERT_LTm(lhs, rhs, msg) _ASSERT_BINOPm(lhs, rhs, <, msg)
#define ASSERT_GTEm(lhs, rhs, msg) _ASSERT_BINOPm(lhs, rhs, >=, msg)
#define ASSERT_LTEm(lhs, rhs, msg) _ASSERT_BINOPm(lhs, rhs, <=, msg)

#define ASSERT_EQ(lhs, rhs) _ASSERT_BINOP(lhs, rhs, ==)
#define ASSERT_NEQ(lhs, rhs) _ASSERT_BINOP(lhs, rhs, !=)
#define ASSERT_GT(lhs, rhs) _ASSERT_BINOP(lhs, rhs, >)
#define ASSERT_LT(lhs, rhs) _ASSERT_BINOP(lhs, rhs, <)
#define ASSERT_GTE(lhs, rhs) _ASSERT_BINOP(lhs, rhs, >=)
#define ASSERT_LTE(lhs, rhs) _ASSERT_BINOP(lhs, rhs, <=)

#if MPTEST_USE_LONGJMP

/* Assert that an assertion failure will occur within statement `stmt`. */
#define ASSERT_ASSERTm(stmt, msg)                                              \
  do {                                                                         \
    if (MPTEST_SETJMP(*mptest__catch_assert_begin(&mptest__state_g)) == 0) {       \
      stmt;                                                                    \
      mptest__catch_assert_end(&mptest__state_g);                              \
      _ASSERT_FAIL_BEHAVIOR("<runtime-assert-checked-function> " #stmt, msg);  \
    } else {                                                                   \
      mptest__catch_assert_end(&mptest__state_g);                              \
      _ASSERT_PASS_BEHAVIOR("<runtime-assert-checked-function> " #stmt, msg);  \
    }                                                                          \
  } while (0)

#define ASSERT_ASSERT(stmt) ASSERT_ASSERTm(stmt, #stmt)

#if MPTEST_DETECT_UNCAUGHT_ASSERTS

#define MPTEST_INJECT_ASSERTm(expr, msg)                                       \
  do {                                                                         \
    if (!(expr)) {                                                             \
      mptest_uncaught_assert_fail_breakpoint();                                \
      mptest__catch_assert_fail(                                               \
          &mptest__state_g, msg, #expr, __FILE__, __LINE__);                   \
    }                                                                          \
  } while (0)

#else

#define MPTEST_INJECT_ASSERTm(expr, msg)                                       \
  do {                                                                         \
    if (mptest__state_g.longjmp_checking &                                     \
        MPTEST__LONGJMP_REASON_ASSERT_FAIL) {                                  \
      if (!(expr)) {                                                           \
        mptest_uncaught_assert_fail_breakpoint();                              \
        mptest__catch_assert_fail(                                             \
            &mptest__state_g, msg, #expr, __FILE__, __LINE__);                 \
      }                                                                        \
    } else {                                                                   \
      MPTEST_ASSERT(expr);                                                         \
    }                                                                          \
  } while (0)

#endif

#else

#define MPTEST_INJECT_ASSERTm(expr, msg) MPTEST_ASSERT(expr)

#endif

#define MPTEST_INJECT_ASSERT(expr) MPTEST_INJECT_ASSERTm(expr, #expr)

#if MPTEST_USE_LEAKCHECK

#define MPTEST_INJECT_MALLOC(size)                                             \
  mptest__leakcheck_hook_malloc(&mptest__state_g, __FILE__, __LINE__, (size))
#define MPTEST_INJECT_FREE(ptr)                                                \
  mptest__leakcheck_hook_free(&mptest__state_g, __FILE__, __LINE__, (ptr))
#define MPTEST_INJECT_REALLOC(old_ptr, new_size)                               \
  mptest__leakcheck_hook_realloc(                                              \
      &mptest__state_g, __FILE__, __LINE__, (old_ptr), (new_size))

#define MPTEST_ENABLE_LEAK_CHECKING()                                          \
  mptest__leakcheck_set(&mptest__state_g, MPTEST__LEAKCHECK_MODE_ON)

#define MPTEST_ENABLE_OOM_ONE()                                                \
  mptest__leakcheck_set(&mptest__state_g, MPTEST__LEAKCHECK_MODE_OOM_ONE)

#define MPTEST_ENABLE_OOM_SET()                                                \
  mptest__leakcheck_set(&mptest__state_g, MPTEST__LEAKCHECK_MODE_OOM_SET)

#define MPTEST_DISABLE_LEAK_CHECKING()                                         \
  mptest__leakcheck_set(&mptest__state_g, MPTEST__LEAKCHECK_MODE_OFF)

#else

#define MPTEST_INJECT_MALLOC(size) MPTEST_MALLOC(size)
#define MPTEST_INJECT_FREE(ptr) MPTEST_FREE(ptr)
#define MPTEST_INJECT_REALLOC(old_ptr, new_size)                               \
  MPTEST_REALLOC(old_ptr, new_size)

#endif

#define MPTEST_MAIN_BEGIN() mptest__state_init(&mptest__state_g)

#define MPTEST_MAIN_BEGIN_ARGS(argc, argv)                                     \
  do {                                                                         \
    aparse_error res = mptest__state_init_argv(                                \
        &mptest__state_g, argc, (char const* const*)(argv));                   \
    if (res == APARSE_SHOULD_EXIT) {                                           \
      return 1;                                                                \
    } else if (res != 0) {                                                     \
      return (int)res;                                                         \
    }                                                                          \
  } while (0)

#define MPTEST_MAIN_END()                                                      \
  do {                                                                         \
    mptest__state_report(&mptest__state_g);                                    \
    mptest__state_destroy(&mptest__state_g);                                   \
  } while (0)

#if MPTEST_USE_FUZZ

#define RAND_PARAM(mod) (mptest__fuzz_rand(&mptest__state_g) % (mod))

#endif

#if MPTEST_USE_SYM

typedef struct mptest_sym mptest_sym;

typedef struct mptest_sym_build {
  mptest_sym* sym;
  mptest_int32 parent_ref;
  mptest_int32 prev_child_ref;
} mptest_sym_build;

typedef struct mptest_sym_walk {
  const mptest_sym* sym;
  mptest_int32 parent_ref;
  mptest_int32 prev_child_ref;
} mptest_sym_walk;

typedef mptest_sym_build sym_build;
typedef mptest_sym_walk sym_walk;

MPTEST_API void mptest_sym_build_init(
    mptest_sym_build* build, mptest_sym* sym, mptest_int32 parent_ref,
    mptest_int32 prev_child_ref);
MPTEST_API int
mptest_sym_build_expr(mptest_sym_build* build, mptest_sym_build* sub);
MPTEST_API int mptest_sym_build_str(
    mptest_sym_build* build, const char* str, mptest_size str_size);
MPTEST_API int mptest_sym_build_cstr(mptest_sym_build* build, const char* cstr);
MPTEST_API int mptest_sym_build_num(mptest_sym_build* build, mptest_int32 num);
MPTEST_API int mptest_sym_build_type(mptest_sym_build* build, const char* type);

MPTEST_API void mptest_sym_walk_init(
    mptest_sym_walk* walk, const mptest_sym* sym, mptest_int32 parent_ref,
    mptest_int32 prev_child_ref);
MPTEST_API int mptest_sym_walk_getexpr(mptest_sym_walk* walk, mptest_sym_walk* sub);
MPTEST_API int mptest_sym_walk_getstr(
    mptest_sym_walk* walk, const char** str, mptest_size* str_size);
MPTEST_API int mptest_sym_walk_getnum(mptest_sym_walk* walk, mptest_int32* num);
MPTEST_API int
mptest_sym_walk_checktype(mptest_sym_walk* walk, const char* expected_type);
MPTEST_API int mptest_sym_walk_hasmore(mptest_sym_walk* walk);
MPTEST_API int mptest_sym_walk_peekstr(mptest_sym_walk* walk);
MPTEST_API int mptest_sym_walk_peeknum(mptest_sym_walk* walk);
MPTEST_API int mptest_sym_walk_peekexpr(mptest_sym_walk* walk);

MPTEST_API int mptest__sym_check_init(
    mptest_sym_build* build_out, const char* str, const char* file, int line,
    const char* msg);
MPTEST_API int mptest__sym_check(const char* file, int line, const char* msg);
MPTEST_API void mptest__sym_check_destroy(void);
MPTEST_API int mptest__sym_make_init(
    mptest_sym_build* build_out, mptest_sym_walk* walk_out, const char* str,
    const char* file, int line, const char* msg);
MPTEST_API void mptest__sym_make_destroy(mptest_sym_build* build_out);

#define MPTEST__SYM_NONE (-1)

#define ASSERT_SYMEQm(type, in_var, chexpr, msg)                               \
  do {                                                                         \
    mptest_sym_build temp_build;                                               \
    if (mptest__sym_check_init(                                                \
            &temp_build, chexpr, __FILE__, __LINE__, msg)) {                   \
      return MPTEST__RESULT_ERROR;                                             \
    }                                                                          \
    if (type##_to_sym(&temp_build, in_var)) {                                  \
      return MPTEST__RESULT_ERROR;                                             \
    }                                                                          \
    if (mptest__sym_check(__FILE__, __LINE__, msg)) {                          \
      return MPTEST__RESULT_FAIL;                                              \
    }                                                                          \
    mptest__sym_check_destroy();                                               \
  } while (0);

#define ASSERT_SYMEQ(type, var, chexpr)                                        \
  ASSERT_SYMEQm(type, var, chexpr, #chexpr)

#define SYM_PUT_TYPE(build, type)                                              \
  do {                                                                         \
    int _sym_err;                                                              \
    if ((_sym_err = mptest_sym_build_cstr(build, (type)))) {                   \
      return _sym_err;                                                         \
    }                                                                          \
  } while (0)

#define SYM_PUT_NUM(build, num)                                                \
  do {                                                                         \
    int _sym_err;                                                              \
    if ((_sym_err = mptest_sym_build_num(build, (num)))) {                     \
      return _sym_err;                                                         \
    }                                                                          \
  } while (0)

#define SYM_PUT_STR(build, str)                                                \
  do {                                                                         \
    int _sym_err;                                                              \
    if ((_sym_err = mptest_sym_build_cstr(build, (str)))) {                    \
      return _sym_err;                                                         \
    }                                                                          \
  } while (0)

#define SYM_PUT_STRN(build, str, str_size)                                     \
  do {                                                                         \
    int _sym_err;                                                              \
    if ((_sym_err = mptest_sym_build_str(build, (str), (str_size)))) {         \
      return _sym_err;                                                         \
    }                                                                          \
  } while (0)

#define SYM_PUT_EXPR(build, new_build)                                         \
  do {                                                                         \
    int _sym_err;                                                              \
    if ((_sym_err = mptest_sym_build_expr(build, new_build))) {                \
      return _sym_err;                                                         \
    }                                                                          \
  } while (0)

#define SYM_PUT_SUB(build, type, in_var)                                       \
  do {                                                                         \
    int _sym_err;                                                              \
    if ((_sym_err = type##_to_sym((build), in_var))) {                         \
      return _sym_err;                                                         \
    }                                                                          \
  } while (0)

#define SYM(type, str, out_var)                                                \
  do {                                                                         \
    mptest_sym_build temp_build;                                               \
    mptest_sym_walk temp_walk;                                                 \
    if (mptest__sym_make_init(                                                 \
            &temp_build, &temp_walk, str, __FILE__, __LINE__, MPTEST_NULL)) {  \
      return MPTEST__RESULT_ERROR;                                             \
    }                                                                          \
    if (type##_from_sym(&temp_walk, out_var)) {                                \
      return MPTEST__RESULT_ERROR;                                             \
    }                                                                          \
    mptest__sym_make_destroy(&temp_build);                                     \
  } while (0)

#define SYM_CHECK_TYPE(walk, type_name)                                        \
  do {                                                                         \
    int _sym_err;                                                              \
    if ((_sym_err = mptest_sym_walk_checktype(walk, type_name))) {             \
      return _sym_err;                                                         \
    }                                                                          \
  } while (0)

#define SYM_GET_STR(walk, str_out, size_out)                                   \
  do {                                                                         \
    int _sym_err;                                                              \
    if ((_sym_err = mptest_sym_walk_getstr(walk, str_out, size_out))) {        \
      return _sym_err;                                                         \
    }                                                                          \
  } while (0)

#define SYM_GET_NUM(walk, num_out)                                             \
  do {                                                                         \
    int _sym_err;                                                              \
    if ((_sym_err = mptest_sym_walk_getnum(walk, num_out))) {                  \
      return _sym_err;                                                         \
    }                                                                          \
  } while (0)

#define SYM_GET_EXPR(walk, walk_out)                                           \
  do {                                                                         \
    int _sym_err;                                                              \
    if ((_sym_err = mptest_sym_walk_getexpr(walk, walk_out))) {                \
      return _sym_err;                                                         \
    }                                                                          \
  } while (0)

#define SYM_GET_SUB(walk, type, out_var)                                       \
  do {                                                                         \
    int _sym_err;                                                              \
    if ((_sym_err = type##_from_sym((walk), (out_var)))) {                     \
      return _sym_err;                                                         \
    }                                                                          \
  } while (0)

#define SYM_MORE(walk) (mptest_sym_walk_hasmore((walk)))

#define SYM_PEEK_STR(walk) (mptest_sym_walk_peekstr((walk)))
#define SYM_PEEK_NUM(walk) (mptest_sym_walk_peeknum((walk)))
#define SYM_PEEK_EXPR(walk) (mptest_sym_walk_peekexpr((walk)))

#define SYM_OK 0
#define SYM_EMPTY 5
#define SYM_WRONG_TYPE 6
#define SYM_NO_MORE 7
#define SYM_INVALID 8

#endif

#endif

#if defined(MPTEST_IMPLEMENTATION)
/* bits/math/implies */
#define MPTEST__IMPLIES(a, b) (!(a) || b)

/* bits/util/exports */
#if !defined(MPTEST__SPLIT_BUILD)
#define MPTEST_INTERNAL static
#else
#define MPTEST_INTERNAL extern
#endif

#define MPTEST_INTERNAL_DATA static

/* bits/util/preproc/token_paste */
#define MPTEST__PASTE_0(a, b) a ## b
#define MPTEST__PASTE(a, b) MPTEST__PASTE_0(a, b)

/* bits/util/static_assert */
#define MPTEST__STATIC_ASSERT(name, expr) char MPTEST__PASTE(mptest__, name)[(expr)==1]

/* bits/container/str */
typedef struct mptest__str {
    mptest_size _size_short; /* does not include \0 */
    mptest_size _alloc; /* does not include \0 */
    mptest_char* _data;
} mptest__str;

void mptest__str_init(mptest__str* str);
int mptest__str_init_s(mptest__str* str, const mptest_char* s);
int mptest__str_init_n(mptest__str* str, const mptest_char* chrs, mptest_size n);
int mptest__str_init_copy(mptest__str* str, const mptest__str* in);
void mptest__str_init_move(mptest__str* str, mptest__str* old);
void mptest__str_destroy(mptest__str* str);
mptest_size mptest__str_size(const mptest__str* str);
int mptest__str_cat(mptest__str* str, const mptest__str* other);
int mptest__str_cat_s(mptest__str* str, const mptest_char* s);
int mptest__str_cat_n(mptest__str* str, const mptest_char* chrs, mptest_size n);
int mptest__str_push(mptest__str* str, mptest_char chr);
int mptest__str_insert(mptest__str* str, mptest_size index, mptest_char chr);
const mptest_char* mptest__str_get_data(const mptest__str* str);
int mptest__str_cmp(const mptest__str* str_a, const mptest__str* str_b);
mptest_size mptest__str_slen(const mptest_char* chars);

#if MPTEST_USE_SYM
#if MPTEST_USE_DYN_ALLOC
/* bits/container/str_view */
typedef struct mptest__str_view {
    const mptest_char* _data;
    mptest_size _size;
} mptest__str_view;

void mptest__str_view_init(mptest__str_view* view, const mptest__str* other);
void mptest__str_view_init_s(mptest__str_view* view, const mptest_char* chars);
void mptest__str_view_init_n(mptest__str_view* view, const mptest_char* chars, mptest_size n);
void mptest__str_view_init_null(mptest__str_view* view);
mptest_size mptest__str_view_size(const mptest__str_view* view);
const mptest_char* mptest__str_view_get_data(const mptest__str_view* view);
int mptest__str_view_cmp(const mptest__str_view* a, const mptest__str_view* b);
#endif /* MPTEST_USE_SYM */
#endif /* MPTEST_USE_DYN_ALLOC */

#if MPTEST_USE_APARSE
/* bits/util/ntstr/cmp_n */
MPTEST_INTERNAL int mptest__scmp_n(const char* a, mptest_size a_size, const char* b);
#endif /* MPTEST_USE_APARSE */

/* bits/util/ntstr/len */
MPTEST_INTERNAL mptest_size mptest__slen(const mptest_char* s);

/* bits/util/unused */
#define MPTEST__UNUSED(x) ((void)(x))

#if MPTEST_USE_APARSE
/* aparse */
#include <stdio.h>
typedef struct aparse__arg aparse__arg;

typedef struct aparse__arg_opt {
    char short_opt;
    const char* long_opt;
    mptest_size long_opt_size;
} aparse__arg_opt;

typedef struct aparse__sub aparse__sub;

typedef struct aparse__arg_sub {
    aparse__sub* head;
    aparse__sub* tail;
} aparse__arg_sub;

typedef struct aparse__arg_pos {
    const char* name;
    mptest_size name_size;
} aparse__arg_pos;

typedef union aparse__arg_contents {
    aparse__arg_opt opt;
    aparse__arg_sub sub;
    aparse__arg_pos pos;
} aparse__arg_contents;

enum aparse__arg_type
{
    /* Optional argument (-o, --o) */
    APARSE__ARG_TYPE_OPTIONAL,
    /* Positional argument */
    APARSE__ARG_TYPE_POSITIONAL,
    /* Subcommand argument */
    APARSE__ARG_TYPE_SUBCOMMAND
};

typedef aparse_error (*aparse__arg_parse_cb)(aparse__arg* arg, aparse__state* state,  mptest_size sub_arg_idx, const char* text, mptest_size text_size);
typedef void (*aparse__arg_destroy_cb)(aparse__arg* arg);

typedef union aparse__arg_callback_data_2 {
    void* plain;
    aparse_custom_cb custom_cb;
} aparse__arg_callback_data_2;

struct aparse__arg {
    enum aparse__arg_type type;
    aparse__arg_contents contents;
    const char* help;
    mptest_size help_size;
    const char* metavar;
    mptest_size metavar_size;
    aparse_nargs nargs;
    int required;
    int was_specified;
    aparse__arg* next;
    aparse__arg_parse_cb callback;
    aparse__arg_destroy_cb destroy;
    void* callback_data;
    aparse__arg_callback_data_2 callback_data_2;
};

#define APARSE__STATE_OUT_BUF_SIZE 128

typedef struct aparse__state_root {
    char out_buf[APARSE__STATE_OUT_BUF_SIZE];
    mptest_size out_buf_ptr;
    const char* prog_name;
    mptest_size prog_name_size;
} aparse__state_root;

struct aparse__state {
    aparse__arg* head;
    aparse__arg* tail;
    const char* help;
    mptest_size help_size;
    aparse_out_cb out_cb;
    void* user;
    aparse__state_root* root;
    int is_root;
};

struct aparse__sub {
    const char* name;
    mptest_size name_size;
    aparse__state subparser;
    aparse__sub* next;
};

MPTEST_INTERNAL void aparse__arg_init(aparse__arg* arg);
MPTEST_INTERNAL void aparse__arg_destroy(aparse__arg* arg);
#if 0
MPTEST_INTERNAL void aparse__state_init_from(aparse__state* state, aparse__state* other);
#endif
MPTEST_INTERNAL void aparse__state_init(aparse__state* state);
MPTEST_INTERNAL void aparse__state_destroy(aparse__state* state);
MPTEST_INTERNAL void aparse__state_set_out_cb(aparse__state* state, aparse_out_cb out_cb, void* user);
MPTEST_INTERNAL void aparse__state_reset(aparse__state* state);
MPTEST_INTERNAL aparse_error aparse__state_add_opt(aparse__state* state, char short_opt, const char* long_opt);
MPTEST_INTERNAL aparse_error aparse__state_add_pos(aparse__state* state, const char* name);
MPTEST_INTERNAL aparse_error aparse__state_add_sub(aparse__state* state);

MPTEST_INTERNAL void aparse__state_check_before_add(aparse__state* state);
MPTEST_INTERNAL void aparse__state_check_before_modify(aparse__state* state);
MPTEST_INTERNAL void aparse__state_check_before_set_type(aparse__state* state);
MPTEST_INTERNAL aparse_error aparse__state_flush(aparse__state* state);
MPTEST_INTERNAL aparse_error aparse__state_out(aparse__state* state, char out);
MPTEST_INTERNAL aparse_error aparse__state_out_s(aparse__state* state, const char* s);
MPTEST_INTERNAL aparse_error aparse__state_out_n(aparse__state* state, const char* s, mptest_size n);

MPTEST_INTERNAL void aparse__arg_bool_init(aparse__arg* arg, int* out);
MPTEST_INTERNAL void aparse__arg_str_init(aparse__arg* arg, const char** out, mptest_size* out_size);
MPTEST_INTERNAL void aparse__arg_help_init(aparse__arg* arg);
MPTEST_INTERNAL void aparse__arg_version_init(aparse__arg* arg);
MPTEST_INTERNAL void aparse__arg_custom_init(aparse__arg* arg, aparse_custom_cb cb, void* user, aparse_nargs nargs);
MPTEST_INTERNAL void aparse__arg_sub_init(aparse__arg* arg);

MPTEST_API aparse_error aparse__parse_argv(aparse__state* state, int argc, const char* const* argv);

MPTEST_INTERNAL aparse_error aparse__error_begin(aparse__state* state);
MPTEST_INTERNAL aparse_error aparse__error_begin_arg(aparse__state* state, const aparse__arg* arg);
MPTEST_INTERNAL aparse_error aparse__error_unrecognized_arg(aparse__state* state, const char* arg);
MPTEST_INTERNAL aparse_error aparse__error_quote(aparse__state* state, const char* text, mptest_size text_size);
MPTEST_INTERNAL aparse_error aparse__error_usage(aparse__state* state);
MPTEST_INTERNAL aparse_error aparse__error_print_short_opt(aparse__state* state, const aparse__arg* arg);
MPTEST_INTERNAL aparse_error aparse__error_print_long_opt(aparse__state* state, const aparse__arg* arg);
MPTEST_INTERNAL aparse_error aparse__error_print_sub_args(aparse__state* state, const aparse__arg* arg);
#endif /* MPTEST_USE_APARSE */

#if MPTEST_USE_SYM
#if MPTEST_USE_DYN_ALLOC
/* bits/container/vec */
#define MPTEST__VEC_TYPE(T) \
    MPTEST__PASTE(T, _vec)

#define MPTEST__VEC_IDENT(T, name) \
    MPTEST__PASTE(T, MPTEST__PASTE(_vec_, name))

#define MPTEST__VEC_IDENT_INTERNAL(T, name) \
    MPTEST__PASTE(T, MPTEST__PASTE(_vec__, name))

#define MPTEST__VEC_DECL_FUNC(T, func) \
    MPTEST__PASTE(MPTEST__VEC_DECL_, func)(T)

#define MPTEST__VEC_IMPL_FUNC(T, func) \
    MPTEST__PASTE(MPTEST__VEC_IMPL_, func)(T)

#if MPTEST_DEBUG

#define MPTEST__VEC_CHECK(vec) \
    do { \
        /* ensure size is not greater than allocation size */ \
        MPTEST_ASSERT(vec->_size <= vec->_alloc); \
        /* ensure that data is not null if size is greater than 0 */ \
        MPTEST_ASSERT(vec->_size ? vec->_data != MPTEST_NULL : 1); \
    } while (0)

#else

#define MPTEST__VEC_CHECK(vec) MPTEST__UNUSED(vec)

#endif

#define MPTEST__VEC_DECL(T) \
    typedef struct MPTEST__VEC_TYPE(T) { \
        mptest_size _size; \
        mptest_size _alloc; \
        T* _data; \
    } MPTEST__VEC_TYPE(T)

#define MPTEST__VEC_DECL_init(T) \
    void MPTEST__VEC_IDENT(T, init)(MPTEST__VEC_TYPE(T)* vec)

#define MPTEST__VEC_IMPL_init(T) \
    void MPTEST__VEC_IDENT(T, init)(MPTEST__VEC_TYPE(T)* vec) { \
        vec->_size = 0; \
        vec->_alloc = 0; \
        vec->_data = MPTEST_NULL; \
    } 

#define MPTEST__VEC_DECL_destroy(T) \
    void MPTEST__VEC_IDENT(T, destroy)(MPTEST__VEC_TYPE(T)* vec)

#define MPTEST__VEC_IMPL_destroy(T) \
    void MPTEST__VEC_IDENT(T, destroy)(MPTEST__VEC_TYPE(T)* vec) { \
        MPTEST__VEC_CHECK(vec); \
        if (vec->_data != MPTEST_NULL) { \
            MPTEST_FREE(vec->_data); \
        } \
    }

#define MPTEST__VEC_GROW_ONE(T, vec) \
    do { \
        void* new_ptr; \
        mptest_size new_alloc; \
        if (vec->_size + 1 > vec->_alloc) { \
            if (vec->_data == MPTEST_NULL) { \
                new_alloc = 1; \
                new_ptr = (T*)MPTEST_MALLOC(sizeof(T) * new_alloc); \
            } else { \
                new_alloc = vec->_alloc * 2; \
                new_ptr = (T*)MPTEST_REALLOC(vec->_data, sizeof(T) * new_alloc); \
            } \
            if (new_ptr == MPTEST_NULL) { \
                return -1; \
            } \
            vec->_alloc = new_alloc; \
            vec->_data = new_ptr; \
        } \
        vec->_size = vec->_size + 1; \
    } while (0)

#define MPTEST__VEC_GROW(T, vec, n) \
    do { \
        void* new_ptr; \
        mptest_size new_alloc = vec->_alloc; \
        mptest_size new_size = vec->_size + n; \
        if (new_size > new_alloc) { \
            if (new_alloc == 0) { \
                new_alloc = 1; \
            } \
            while (new_alloc < new_size) { \
                new_alloc *= 2; \
            } \
            if (vec->_data == MPTEST_NULL) { \
                new_ptr = (T*)MPTEST_MALLOC(sizeof(T) * new_alloc); \
            } else { \
                new_ptr = (T*)MPTEST_REALLOC(vec->_data, sizeof(T) * new_alloc); \
            } \
            if (new_ptr == MPTEST_NULL) { \
                return -1; \
            } \
            vec->_alloc = new_alloc; \
            vec->_data = new_ptr; \
        } \
        vec->_size += n; \
    } while (0)

#define MPTEST__VEC_SETSIZE(T, vec, n) \
    do { \
        void* new_ptr; \
        if (vec->_alloc < n) { \
            if (vec->_data == MPTEST_NULL) { \
                new_ptr = (T*)MPTEST_MALLOC(sizeof(T) * n); \
            } else { \
                new_ptr = (T*)MPTEST_REALLOC(vec->_data, sizeof(T) * n); \
            } \
            if (new_ptr == MPTEST_NULL) { \
                return -1; \
            } \
            vec->_alloc = n; \
            vec->_data = new_ptr; \
        } \
    } while (0)

#define MPTEST__VEC_DECL_push(T) \
    int MPTEST__VEC_IDENT(T, push)(MPTEST__VEC_TYPE(T)* vec, T elem)

#define MPTEST__VEC_IMPL_push(T) \
    int MPTEST__VEC_IDENT(T, push)(MPTEST__VEC_TYPE(T)* vec, T elem) { \
        MPTEST__VEC_CHECK(vec); \
        MPTEST__VEC_GROW_ONE(T, vec); \
        vec->_data[vec->_size - 1] = elem; \
        MPTEST__VEC_CHECK(vec); \
        return 0; \
    }

#if MPTEST_DEBUG

#define MPTEST__VEC_CHECK_POP(vec) \
    do { \
        /* ensure that there is an element to pop */ \
        MPTEST_ASSERT(vec->_size > 0); \
    } while (0)

#else

#define MPTEST__VEC_CHECK_POP(vec) MPTEST__UNUSED(vec)

#endif

#define MPTEST__VEC_DECL_pop(T) \
    T MPTEST__VEC_IDENT(T, pop)(MPTEST__VEC_TYPE(T)* vec)

#define MPTEST__VEC_IMPL_pop(T) \
    T MPTEST__VEC_IDENT(T, pop)(MPTEST__VEC_TYPE(T)* vec) { \
        MPTEST__VEC_CHECK(vec); \
        MPTEST__VEC_CHECK_POP(vec); \
        return vec->_data[--vec->_size]; \
    }

#define MPTEST__VEC_DECL_cat(T) \
    T MPTEST__VEC_IDENT(T, cat)(MPTEST__VEC_TYPE(T)* vec, MPTEST__VEC_TYPE(T)* other)

#define MPTEST__VEC_IMPL_cat(T) \
    int MPTEST__VEC_IDENT(T, cat)(MPTEST__VEC_TYPE(T)* vec, MPTEST__VEC_TYPE(T)* other) { \
        re_size i; \
        re_size old_size = vec->_size; \
        MPTEST__VEC_CHECK(vec); \
        MPTEST__VEC_CHECK(other); \
        MPTEST__VEC_GROW(T, vec, other->_size); \
        for (i = 0; i < other->_size; i++) { \
            vec->_data[old_size + i] = other->_data[i]; \
        } \
        MPTEST__VEC_CHECK(vec); \
        return 0; \
    }

#define MPTEST__VEC_DECL_insert(T) \
    int MPTEST__VEC_IDENT(T, insert)(MPTEST__VEC_TYPE(T)* vec, mptest_size index, T elem)

#define MPTEST__VEC_IMPL_insert(T) \
    int MPTEST__VEC_IDENT(T, insert)(MPTEST__VEC_TYPE(T)* vec, mptest_size index, T elem) { \
        mptest_size i; \
        mptest_size old_size = vec->_size; \
        MPTEST__VEC_CHECK(vec); \
        MPTEST__VEC_GROW_ONE(T, vec); \
        if (old_size != 0) { \
            for (i = old_size; i >= index + 1; i--) { \
                vec->_data[i] = vec->_data[i - 1]; \
            } \
        } \
        vec->_data[index] = elem; \
        return 0; \
    }

#define MPTEST__VEC_DECL_peek(T) \
    T MPTEST__VEC_IDENT(T, peek)(const MPTEST__VEC_TYPE(T)* vec)

#define MPTEST__VEC_IMPL_peek(T) \
    T MPTEST__VEC_IDENT(T, peek)(const MPTEST__VEC_TYPE(T)* vec) { \
        MPTEST__VEC_CHECK(vec); \
        MPTEST__VEC_CHECK_POP(vec); \
        return vec->_data[vec->_size - 1]; \
    }

#define MPTEST__VEC_DECL_clear(T) \
    void MPTEST__VEC_IDENT(T, clear)(MPTEST__VEC_TYPE(T)* vec)

#define MPTEST__VEC_IMPL_clear(T) \
    void MPTEST__VEC_IDENT(T, clear)(MPTEST__VEC_TYPE(T)* vec) { \
        MPTEST__VEC_CHECK(vec); \
        vec->_size = 0; \
    }

#define MPTEST__VEC_DECL_size(T) \
    mptest_size MPTEST__VEC_IDENT(T, size)(const MPTEST__VEC_TYPE(T)* vec)

#define MPTEST__VEC_IMPL_size(T) \
    mptest_size MPTEST__VEC_IDENT(T, size)(const MPTEST__VEC_TYPE(T)* vec) { \
        return vec->_size; \
    }

#if MPTEST_DEBUG

#define MPTEST__VEC_CHECK_BOUNDS(vec, idx) \
    do { \
        /* ensure that idx is within bounds */ \
        MPTEST_ASSERT(idx < vec->_size); \
    } while (0)

#else

#define MPTEST__VEC_CHECK_BOUNDS(vec, idx) \
    do { \
        MPTEST__UNUSED(vec); \
        MPTEST__UNUSED(idx); \
    } while (0) 

#endif

#define MPTEST__VEC_DECL_get(T) \
    T MPTEST__VEC_IDENT(T, get)(const MPTEST__VEC_TYPE(T)* vec, mptest_size idx)

#define MPTEST__VEC_IMPL_get(T) \
    T MPTEST__VEC_IDENT(T, get)(const MPTEST__VEC_TYPE(T)* vec, mptest_size idx) { \
        MPTEST__VEC_CHECK(vec); \
        MPTEST__VEC_CHECK_BOUNDS(vec, idx); \
        return vec->_data[idx]; \
    }

#define MPTEST__VEC_DECL_getref(T) \
    T* MPTEST__VEC_IDENT(T, getref)(MPTEST__VEC_TYPE(T)* vec, mptest_size idx)

#define MPTEST__VEC_IMPL_getref(T) \
    T* MPTEST__VEC_IDENT(T, getref)(MPTEST__VEC_TYPE(T)* vec, mptest_size idx) { \
        MPTEST__VEC_CHECK(vec); \
        MPTEST__VEC_CHECK_BOUNDS(vec, idx); \
        return &vec->_data[idx]; \
    }

#define MPTEST__VEC_DECL_getcref(T) \
    const T* MPTEST__VEC_IDENT(T, getcref)(const MPTEST__VEC_TYPE(T)* vec, mptest_size idx)

#define MPTEST__VEC_IMPL_getcref(T) \
    const T* MPTEST__VEC_IDENT(T, getcref)(const MPTEST__VEC_TYPE(T)* vec, mptest_size idx) { \
        MPTEST__VEC_CHECK(vec); \
        MPTEST__VEC_CHECK_BOUNDS(vec, idx); \
        return &vec->_data[idx]; \
    }

#define MPTEST__VEC_DECL_set(T) \
    void MPTEST__VEC_IDENT(T, set)(MPTEST__VEC_TYPE(T)* vec, mptest_size idx, T elem)

#define MPTEST__VEC_IMPL_set(T) \
    void MPTEST__VEC_IDENT(T, set)(MPTEST__VEC_TYPE(T)* vec, mptest_size idx, T elem) { \
        MPTEST__VEC_CHECK(vec); \
        MPTEST__VEC_CHECK_BOUNDS(vec, idx); \
        vec->_data[idx] = elem; \
    }

#define MPTEST__VEC_DECL_capacity(T) \
    mptest_size MPTEST__VEC_IDENT(T, capacity)(MPTEST__VEC_TYPE(T)* vec)

#define MPTEST__VEC_IMPL_capacity(T) \
    mptest_size MPTEST__VEC_IDENT(T, capacity)(MPTEST__VEC_TYPE(T)* vec) { \
        return vec->_alloc; \
    }

#define MPTEST__VEC_DECL_get_data(T) \
    const T* MPTEST__VEC_IDENT(T, get_data)(const MPTEST__VEC_TYPE(T)* vec)

#define MPTEST__VEC_IMPL_get_data(T) \
    const T* MPTEST__VEC_IDENT(T, get_data)(const MPTEST__VEC_TYPE(T)* vec) { \
        return vec->_data; \
    }

#define MPTEST__VEC_DECL_move(T) \
    void MPTEST__VEC_IDENT(T, move)(MPTEST__VEC_TYPE(T)* vec, MPTEST__VEC_TYPE(T)* old);

#define MPTEST__VEC_IMPL_move(T) \
    void MPTEST__VEC_IDENT(T, move)(MPTEST__VEC_TYPE(T)* vec, MPTEST__VEC_TYPE(T)* old) { \
        MPTEST__VEC_CHECK(old); \
        *vec = *old; \
        MPTEST__VEC_IDENT(T, init)(old); \
    }

#define MPTEST__VEC_DECL_reserve(T) \
    int MPTEST__VEC_IDENT(T, reserve)(MPTEST__VEC_TYPE(T)* vec, mptest_size cap);

#define MPTEST__VEC_IMPL_reserve(T) \
    int MPTEST__VEC_IDENT(T, reserve)(MPTEST__VEC_TYPE(T)* vec, mptest_size cap) { \
        MPTEST__VEC_CHECK(vec); \
        MPTEST__VEC_SETSIZE(T, vec, cap); \
        return 0; \
    }
#endif /* MPTEST_USE_SYM */
#endif /* MPTEST_USE_DYN_ALLOC */

/* mptest */
#ifndef MPTEST_INTERNAL_H
#define MPTEST_INTERNAL_H
/* How assert checking works (and why we need longjmp for it):
 * 1. You use the function ASSERT_ASSERT(statement) in your test code.
 * 2. Under the hood, ASSERT_ASSERT setjmps the current test, and runs the
 *    statement until an assert within the program fails.
 * 3. The assert hook longjmps out of the code into the previous setjmp from
 *    step (2).
 * 4. mptest recognizes this jump back and passes the test.
 * 5. If the jump back doesn't happen, mptest recognizes this too and fails the
 *    test, expecting an assertion failure. */
#if MPTEST_USE_TIME
#include <time.h>
#endif

#define MPTEST__RESULT_SKIPPED -3

/* The different ways a test can fail. */
typedef enum mptest__fail_reason {
  /* No failure. */
  MPTEST__FAIL_REASON_NONE,
  /* ASSERT_XX(...) statement failed. */
  MPTEST__FAIL_REASON_ASSERT_FAILURE,
  /* FAIL() statement issued. */
  MPTEST__FAIL_REASON_FAIL_EXPR,
#if MPTEST_USE_LONGJMP
  /* Program caused an assert() failure *within its code*. */
  MPTEST__FAIL_REASON_UNCAUGHT_PROGRAM_ASSERT,
#endif
#if MPTEST_USE_DYN_ALLOC
  /* Fatal error: mptest (not the program) ran out of memory. */
  MPTEST__FAIL_REASON_NOMEM,
#endif
#if MPTEST_USE_LEAKCHECK
  /* Program tried to call realloc() on null pointer. */
  MPTEST__FAIL_REASON_REALLOC_OF_NULL,
  /* Program tried to call realloc() on invalid pointer. */
  MPTEST__FAIL_REASON_REALLOC_OF_INVALID,
  /* Program tried to call realloc() on an already freed pointer. */
  MPTEST__FAIL_REASON_REALLOC_OF_FREED,
  /* Program tried to call realloc() on an already reallocated pointer. */
  MPTEST__FAIL_REASON_REALLOC_OF_REALLOCED,
  /* Program tried to call free() on a null pointer. */
  MPTEST__FAIL_REASON_FREE_OF_NULL,
  /* Program tried to call free() on an invalid pointer. */
  MPTEST__FAIL_REASON_FREE_OF_INVALID,
  /* Program tried to call free() on an already freed pointer. */
  MPTEST__FAIL_REASON_FREE_OF_FREED,
  /* Program tried to call free() on an already reallocated pointer. */
  MPTEST__FAIL_REASON_FREE_OF_REALLOCED,
  /* End-of-test memory check found unfreed blocks. */
  MPTEST__FAIL_REASON_LEAKED,
#endif
#if MPTEST_USE_SYM
  /* Syms compared unequal. */
  MPTEST__FAIL_REASON_SYM_INEQUALITY,
  /* Syntax error occurred in a sym. */
  MPTEST__FAIL_REASON_SYM_SYNTAX,
  /* Couldn't parse a sym into an object. */
  MPTEST__FAIL_REASON_SYM_DESERIALIZE,
#endif
  MPTEST__FAIL_REASON_LAST
} mptest__fail_reason;

/* Type representing a function to be called whenever a suite is set up or torn
 * down. */
typedef void (*mptest__suite_callback)(void* data);

#if MPTEST_USE_SYM
typedef struct mptest__sym_fail_data {
  mptest_sym* sym_actual;
  mptest_sym* sym_expected;
} mptest__sym_fail_data;

typedef struct mptest__sym_syntax_error_data {
  const char* err_msg;
  mptest_size err_pos;
} mptest__sym_syntax_error_data;
#endif

/* Data describing how the test failed. */
typedef union mptest__fail_data {
  const char* string_data;
#if MPTEST_USE_LEAKCHECK
  void* memory_block;
#endif
#if MPTEST_USE_SYM
  mptest__sym_fail_data sym_fail_data;
  mptest__sym_syntax_error_data sym_syntax_error_data;
#endif
} mptest__fail_data;

#if MPTEST_USE_APARSE
typedef struct mptest__aparse_name mptest__aparse_name;

struct mptest__aparse_name {
  const char* name;
  mptest_size name_len;
  mptest__aparse_name* next;
};

typedef struct mptest__aparse_state {
  aparse_state aparse;
  /*     --leak-check : whether to enable leak checking or not */
  int opt_leak_check;
  /*     --leak-check-oom : whether to enable OOM checking or not */
  int opt_leak_check_oom;
  /* -t, --test : the test name(s) to search for and run */
  mptest__aparse_name* opt_test_name_head;
  mptest__aparse_name* opt_test_name_tail;
  /* -s, --suite : the suite name(s) to search for and run */
  mptest__aparse_name* opt_suite_name_head;
  mptest__aparse_name* opt_suite_name_tail;
  /*     --leak-check-pass : whether to enable leak check malloc passthrough */
  int opt_leak_check_pass;
} mptest__aparse_state;
#endif

#if MPTEST_USE_LONGJMP
typedef struct mptest__longjmp_state {
  /* Saved setjmp context (used for testing asserts, etc.) */
  MPTEST_JMP_BUF assert_context;
  /* Saved setjmp context (used to catch actual errors during testing) */
  MPTEST_JMP_BUF test_context;
  /* 1 if we are checking for a jump, 0 if not. Used so that if an assertion
   * *accidentally* goes off, we can catch it. */
  mptest__fail_reason checking;
  /* Reason for jumping (assertion failure, malloc/free failure, etc) */
  mptest__fail_reason reason;
} mptest__longjmp_state;
#endif

#if MPTEST_USE_LEAKCHECK
typedef struct mptest__leakcheck_state {
  /* 1 if current test should be audited for leaks, 0 otherwise. */
  mptest__leakcheck_mode test_leak_checking;
  /* First and most recent blocks allocated. */
  struct mptest__leakcheck_block* first_block;
  struct mptest__leakcheck_block* top_block;
  /* Total number of allocations in use. */
  int total_allocations;
  /* Total number of calls to malloc() or realloc(). */
  int total_calls;
  /* Whether or not the current test failed on an OOM condition */
  int oom_failed;
  /* The index of the call that the test failed on */
  int oom_fail_call;
  /* Whether or not to let allocations fall through */
  int fall_through;
} mptest__leakcheck_state;
#endif

#if MPTEST_USE_TIME
typedef struct mptest__time_state {
  /* Start times that will be compared against later */
  clock_t program_start_time;
  clock_t suite_start_time;
  clock_t test_start_time;
} mptest__time_state;
#endif

#if MPTEST_USE_FUZZ
typedef struct mptest__fuzz_state {
  /* State of the random number generator */
  mptest_rand rand_state;
  /* Whether or not the current test should be fuzzed */
  int fuzz_active;
  /* Whether or not the current test failed on a fuzz */
  int fuzz_failed;
  /* Number of iterations to run the next test for */
  int fuzz_iterations;
  /* Fuzz failure context */
  int fuzz_fail_iteration;
  mptest_rand fuzz_fail_seed;
} mptest__fuzz_state;
#endif

struct mptest__state {
  /* Total number of assertions */
  int assertions;
  /* Total number of tests */
  int total;
  /* Total number of passes, fails, and errors */
  int passes;
  int fails;
  int errors;
  /* Total number of suite passes and fails */
  int suite_passes;
  int suite_fails;
  /* 1 if the current suite failed, 0 if not */
  int suite_failed;
  /* Suite setup/teardown callbacks */
  mptest__suite_callback suite_test_setup_cb;
  mptest__suite_callback suite_test_teardown_cb;
  /* Names of the current running test/suite */
  const char* current_test;
  const char* current_suite;
  /* Reason for failing a test */
  mptest__fail_reason fail_reason;
  /* Fail diagnostics */
  const char* fail_msg;
  const char* fail_file;
  int fail_line;
  /* Stores information about the failure. */
  /* Assert expression that caused the fail, if `fail_reason` ==
   * `MPTEST__FAIL_REASON_ASSERT_FAILURE` */
  /* Pointer to offending allocation, if `longjmp_reason` is one of the
   * malloc fail reasons */
  mptest__fail_data fail_data;
  /* Indentation level (used for output) */
  int indent_lvl;

#if MPTEST_USE_LONGJMP
  mptest__longjmp_state longjmp_state;
#endif

#if MPTEST_USE_LEAKCHECK
  mptest__leakcheck_state leakcheck_state;
#endif

#if MPTEST_USE_TIME
  mptest__time_state time_state;
#endif

#if MPTEST_USE_APARSE
  mptest__aparse_state aparse_state;
#endif

#if MPTEST_USE_FUZZ
  mptest__fuzz_state fuzz_state;
#endif
};

#include <stdio.h>

MPTEST_INTERNAL mptest__result mptest__state_do_run_test(
    struct mptest__state* state, mptest__test_func test_func);
MPTEST_INTERNAL void mptest__state_print_indent(struct mptest__state* state);

#if MPTEST_USE_LONGJMP

MPTEST_INTERNAL void mptest__longjmp_init(struct mptest__state* state);
MPTEST_INTERNAL void mptest__longjmp_destroy(struct mptest__state* state);
MPTEST_INTERNAL void mptest__longjmp_exec(
    struct mptest__state* state, mptest__fail_reason reason, const char* file,
    int line, const char* msg);

#endif

#if MPTEST_USE_LEAKCHECK
/* Number of guard bytes to put at the top of each block. */
#define MPTEST__LEAKCHECK_GUARD_BYTES_COUNT 16

/* Flags kept for each block. */
enum mptest__leakcheck_block_flags {
  /* The block was allocated with malloc(). */
  MPTEST__LEAKCHECK_BLOCK_FLAG_INITIAL = 1,
  /* The block was freed with free(). */
  MPTEST__LEAKCHECK_BLOCK_FLAG_FREED = 2,
  /* The block was the *input* of a reallocation with realloc(). */
  MPTEST__LEAKCHECK_BLOCK_FLAG_REALLOC_OLD = 4,
  /* The block was the *result* of a reallocation with realloc(). */
  MPTEST__LEAKCHECK_BLOCK_FLAG_REALLOC_NEW = 8
};

/* Header kept in memory before each allocation. */
struct mptest__leakcheck_header {
  /* Guard bytes (like a magic number, signifies proper allocation) */
  unsigned char guard_bytes[MPTEST__LEAKCHECK_GUARD_BYTES_COUNT];
  /* Block reference */
  struct mptest__leakcheck_block* block;
};

/* Structure that keeps track of a header's properties. */
struct mptest__leakcheck_block {
  /* Pointer to the header that exists right before the memory. */
  struct mptest__leakcheck_header* header;
  /* Size of block as passed to malloc() or realloc() */
  size_t block_size;
  /* Previous and next block records */
  struct mptest__leakcheck_block* prev;
  struct mptest__leakcheck_block* next;
  /* Realloc chain previous and next */
  struct mptest__leakcheck_block* realloc_prev;
  struct mptest__leakcheck_block* realloc_next;
  /* Flags (see `enum mptest__leakcheck_block_flags`) */
  enum mptest__leakcheck_block_flags flags;
  /* Source location where the malloc originated */
  const char* file;
  int line;
};

#define MPTEST__LEAKCHECK_HEADER_SIZEOF                                        \
  (sizeof(struct mptest__leakcheck_header))

MPTEST_INTERNAL void mptest__leakcheck_init(struct mptest__state* state);
MPTEST_INTERNAL void mptest__leakcheck_destroy(struct mptest__state* state);
MPTEST_INTERNAL void mptest__leakcheck_reset(struct mptest__state* state);
MPTEST_INTERNAL int mptest__leakcheck_has_leaks(struct mptest__state* state);
MPTEST_INTERNAL int
mptest__leakcheck_block_has_freeable(struct mptest__leakcheck_block* block);
MPTEST_INTERNAL mptest__result mptest__leakcheck_oom_run_test(
    struct mptest__state* state, mptest__test_func test_func);
#endif

#if MPTEST_USE_COLOR
#define MPTEST__COLOR_PASS "\x1b[1;32m"       /* Pass messages */
#define MPTEST__COLOR_FAIL "\x1b[1;31m"       /* Fail messages */
#define MPTEST__COLOR_TEST_NAME "\x1b[1;36m"  /* Test names */
#define MPTEST__COLOR_SUITE_NAME "\x1b[1;35m" /* Suite names */
#define MPTEST__COLOR_EMPHASIS "\x1b[1m"      /* Important numbers */
#define MPTEST__COLOR_RESET "\x1b[0m"         /* Regular text */
#else
#define MPTEST__COLOR_PASS ""
#define MPTEST__COLOR_FAIL ""
#define MPTEST__COLOR_TEST_NAME ""
#define MPTEST__COLOR_SUITE_NAME ""
#define MPTEST__COLOR_EMPHASIS ""
#define MPTEST__COLOR_RESET ""
#endif

#if MPTEST_USE_TIME
MPTEST_INTERNAL void mptest__time_init(struct mptest__state* state);
MPTEST_INTERNAL void mptest__time_destroy(struct mptest__state* state);
#endif

#if MPTEST_USE_APARSE
MPTEST_INTERNAL int mptest__aparse_init(struct mptest__state* state);
MPTEST_INTERNAL void mptest__aparse_destroy(struct mptest__state* state);
MPTEST_INTERNAL int mptest__aparse_match_test_name(
    struct mptest__state* state, const char* test_name);
MPTEST_INTERNAL int mptest__aparse_match_suite_name(
    struct mptest__state* state, const char* suite_name);
#endif

#if MPTEST_USE_FUZZ
MPTEST_INTERNAL void mptest__fuzz_init(struct mptest__state* state);
MPTEST_INTERNAL mptest__result
mptest__fuzz_run_test(struct mptest__state* state, mptest__test_func test_func);
MPTEST_INTERNAL void mptest__fuzz_print(struct mptest__state* state);
#endif

#if MPTEST_USE_SYM
MPTEST_INTERNAL void
mptest__sym_dump(mptest_sym* sym, mptest_int32 parent_ref, mptest_int32 indent);
#endif

#endif

#endif /* MPTEST_IMPLEMENTATION */
#if defined(MPTEST_IMPLEMENTATION)
/* bits/types/char */
MPTEST__STATIC_ASSERT(mptest__char_is_one_byte, sizeof(mptest_char) == 1);

/* bits/container/str */
/* Maximum size, without null terminator */
#define MPTEST__STR_SHORT_SIZE_MAX (((sizeof(mptest__str) - sizeof(mptest_size)) / (sizeof(mptest_char)) - 1))

#define MPTEST__STR_GET_SHORT(str) !((str)->_size_short & 1)
#define MPTEST__STR_SET_SHORT(str, short) \
    do { \
        mptest_size temp = short; \
        (str)->_size_short &= ~((mptest_size)1); \
        (str)->_size_short |= !temp; \
    } while (0)
#define MPTEST__STR_GET_SIZE(str) ((str)->_size_short >> 1)
#define MPTEST__STR_SET_SIZE(str, size) \
    do { \
        mptest_size temp = size; \
        (str)->_size_short &= 1; \
        (str)->_size_short |= temp << 1; \
    } while (0)
#define MPTEST__STR_DATA(str) (MPTEST__STR_GET_SHORT(str) ? ((mptest_char*)&((str)->_alloc)) : (str)->_data)

/* Round up to multiple of 32 */
#define MPTEST__STR_ROUND_ALLOC(alloc) \
    (((alloc + 1) + 32) & (~((mptest_size)32)))

#if MPTEST_DEBUG

#define MPTEST__STR_CHECK(str) \
    do { \
        if (MPTEST__STR_GET_SHORT(str)) { \
            /* If string is short, the size must always be less than */ \
            /* MPTEST__STR_SHORT_SIZE_MAX. */ \
            MPTEST_ASSERT(MPTEST__STR_GET_SIZE(str) <= MPTEST__STR_SHORT_SIZE_MAX); \
        } else { \
            /* If string is long, the size can still be less, but the other */ \
            /* fields must be valid. */ \
            /* Ensure there is enough space */ \
            MPTEST_ASSERT((str)->_alloc >= MPTEST__STR_GET_SIZE(str)); \
            /* Ensure that the _data field isn't NULL if the size is 0 */ \
            if (MPTEST__STR_GET_SIZE(str) > 0) { \
                MPTEST_ASSERT((str)->_data != MPTEST_NULL); \
            } \
            /* Ensure that if _alloc is 0 then _data is NULL */ \
            if ((str)->_alloc == 0) { \
                MPTEST_ASSERT((str)->_data == MPTEST_NULL); \
            } \
        } \
        /* Ensure that there is a null-terminator */ \
        MPTEST_ASSERT(MPTEST__STR_DATA(str)[MPTEST__STR_GET_SIZE(str)] == '\0'); \
    } while (0)

#else

#define MPTEST__STR_CHECK(str) MPTEST__UNUSED(str)

#endif

void mptest__str_init(mptest__str* str) {
    str->_size_short = 0;
    MPTEST__STR_DATA(str)[0] = '\0';
}

void mptest__str_destroy(mptest__str* str) {
    if (!MPTEST__STR_GET_SHORT(str)) {
        if (str->_data != MPTEST_NULL) {
            MPTEST_FREE(str->_data);
        }
    }
}

mptest_size mptest__str_size(const mptest__str* str) {
    return MPTEST__STR_GET_SIZE(str);
}

MPTEST_INTERNAL int mptest__str_grow(mptest__str* str, mptest_size new_size) {
    mptest_size old_size = MPTEST__STR_GET_SIZE(str);
    MPTEST__STR_CHECK(str);
    if (MPTEST__STR_GET_SHORT(str)) {
        if (new_size <= MPTEST__STR_SHORT_SIZE_MAX) {
            /* Can still be a short str */
            MPTEST__STR_SET_SIZE(str, new_size);
        } else {
            /* Needs allocation */
            mptest_size new_alloc = 
                MPTEST__STR_ROUND_ALLOC(new_size + (new_size >> 1));
            mptest_char* new_data = (mptest_char*)MPTEST_MALLOC(sizeof(mptest_char) * (new_alloc + 1));
            mptest_size i;
            if (new_data == MPTEST_NULL) {
                return -1;
            }
            /* Copy data from old string */
            for (i = 0; i < old_size; i++) {
                new_data[i] = MPTEST__STR_DATA(str)[i];
            }
            /* Fill in the remaining fields */
            MPTEST__STR_SET_SHORT(str, 0);
            MPTEST__STR_SET_SIZE(str, new_size);
            str->_data = new_data;
            str->_alloc = new_alloc;
        }
    } else {
        if (new_size > str->_alloc) {
            /* Needs allocation */
            mptest_size new_alloc = 
                MPTEST__STR_ROUND_ALLOC(new_size + (new_size >> 1));
            mptest_char* new_data;
            if (str->_alloc == 0) {
                new_data = \
                    (mptest_char*)MPTEST_MALLOC(sizeof(mptest_char) * (new_alloc + 1));
            } else {
                new_data = \
                    (mptest_char*)MPTEST_REALLOC(
                        str->_data, sizeof(mptest_char) * (new_alloc + 1));
            }
            if (new_data == MPTEST_NULL) {
                return -1;
            }
            str->_data = new_data;
            str->_alloc = new_alloc;
        }
        MPTEST__STR_SET_SIZE(str, new_size);
    }
    /* Null terminate */
    MPTEST__STR_DATA(str)[MPTEST__STR_GET_SIZE(str)] = '\0';
    MPTEST__STR_CHECK(str);
    return 0;
}

int mptest__str_push(mptest__str* str, mptest_char chr) {
    int err = 0;
    mptest_size old_size = MPTEST__STR_GET_SIZE(str);
    if ((err = mptest__str_grow(str, old_size + 1))) {
        return err;
    }
    MPTEST__STR_DATA(str)[old_size] = chr;
    MPTEST__STR_CHECK(str);
    return err;
}

mptest_size mptest__str_slen(const mptest_char* s) {
    mptest_size out = 0;
    while (*(s++)) {
        out++;
    }
    return out;
}

int mptest__str_init_s(mptest__str* str, const mptest_char* s) {
    int err = 0;
    mptest_size i;
    mptest_size sz = mptest__str_slen(s);
    mptest__str_init(str);
    if ((err = mptest__str_grow(str, sz))) {
        return err;
    }
    for (i = 0; i < sz; i++) {
        MPTEST__STR_DATA(str)[i] = s[i];
    }
    return err;
}

int mptest__str_init_n(mptest__str* str, const mptest_char* chrs, mptest_size n) {
    int err = 0;
    mptest_size i;
    mptest__str_init(str);
    if ((err = mptest__str_grow(str, n))) {
        return err;
    }
    for (i = 0; i < n; i++) {
        MPTEST__STR_DATA(str)[i] = chrs[i];
    }
    return err;
}

int mptest__str_cat(mptest__str* str, const mptest__str* other) {
    int err = 0;
    mptest_size i;
    mptest_size n = MPTEST__STR_GET_SIZE(other);
    mptest_size old_size = MPTEST__STR_GET_SIZE(str);
    if ((err = mptest__str_grow(str, old_size + n))) {
        return err;
    }
    /* Copy data */
    for (i = 0; i < n; i++) {
        MPTEST__STR_DATA(str)[old_size + i] = MPTEST__STR_DATA(other)[i];
    }
    MPTEST__STR_CHECK(str);
    return err;
}

int mptest__str_cat_n(mptest__str* str, const mptest_char* chrs, mptest_size n) {
    int err = 0;
    mptest_size i;
    mptest_size old_size = MPTEST__STR_GET_SIZE(str);
    if ((err = mptest__str_grow(str, old_size + n))) {
        return err;
    }
    /* Copy data */
    for (i = 0; i < n; i++) {
        MPTEST__STR_DATA(str)[old_size + i] = chrs[i];
    }
    MPTEST__STR_CHECK(str);
    return err;
}

int mptest__str_cat_s(mptest__str* str, const mptest_char* chrs) {
    mptest_size chrs_size = mptest__str_slen(chrs);
    return mptest__str_cat_n(str, chrs, chrs_size);
}

int mptest__str_insert(mptest__str* str, mptest_size index, mptest_char chr) {
    int err = 0;
    mptest_size i;
    mptest_size old_size = MPTEST__STR_GET_SIZE(str);
    /* bounds check */
    MPTEST_ASSERT(index <= MPTEST__STR_GET_SIZE(str));
    if ((err = mptest__str_grow(str, old_size + 1))) {
        return err;
    }
    /* Shift data */
    if (old_size != 0) {
        for (i = old_size; i >= index + 1; i--) {
            MPTEST__STR_DATA(str)[i] = MPTEST__STR_DATA(str)[i - 1];
        }
    }
    MPTEST__STR_DATA(str)[index] = chr;
    MPTEST__STR_CHECK(str);
    return err;
}

const mptest_char* mptest__str_get_data(const mptest__str* str) {
    return MPTEST__STR_DATA(str);
}

int mptest__str_init_copy(mptest__str* str, const mptest__str* in) {
    mptest_size i;
    int err = 0;
    mptest__str_init(str);
    if ((err = mptest__str_grow(str, mptest__str_size(in)))) {
        return err;
    }
    for (i = 0; i < mptest__str_size(str); i++) {
        MPTEST__STR_DATA(str)[i] = MPTEST__STR_DATA(in)[i];
    }
    return err;
}

void mptest__str_init_move(mptest__str* str, mptest__str* old) {
    MPTEST__STR_CHECK(old);
    *str = *old;
    mptest__str_init(old);
}

int mptest__str_cmp(const mptest__str* str_a, const mptest__str* str_b) {
    mptest_size a_len = mptest__str_size(str_a);
    mptest_size b_len = mptest__str_size(str_b);
    const mptest_char* a_data = mptest__str_get_data(str_a);
    const mptest_char* b_data = mptest__str_get_data(str_b);
    mptest_size i;
    if (a_len < b_len) {
        return -1;
    } else if (a_len > b_len) {
        return 1;
    }
    for (i = 0; i < a_len; i++) {
        if (a_data[i] != b_data[i]) {
            if (a_data[i] < b_data[i]) {
                return -1;
            } else {
                return 1;
            }
        }
    }
    return 0;
}

#if MPTEST_USE_SYM
#if MPTEST_USE_DYN_ALLOC
/* bits/container/str_view */
void mptest__str_view_init(mptest__str_view* view, const mptest__str* other) {
    view->_size = mptest__str_size(other);
    view->_data = mptest__str_get_data(other);
}

void mptest__str_view_init_s(mptest__str_view* view, const mptest_char* chars) {
    view->_size = mptest__str_slen(chars);
    view->_data = chars;
}

void mptest__str_view_init_n(mptest__str_view* view, const mptest_char* chars, mptest_size n) {
    view->_size = n;
    view->_data = chars;
}

void mptest__str_view_init_null(mptest__str_view* view) {
    view->_size = 0;
    view->_data = MPTEST_NULL;
}

mptest_size mptest__str_view_size(const mptest__str_view* view) {
    return view->_size;
}

const mptest_char* mptest__str_view_get_data(const mptest__str_view* view) {
    return view->_data;
}

int mptest__str_view_cmp(const mptest__str_view* view_a, const mptest__str_view* view_b) {
    mptest_size a_len = mptest__str_view_size(view_a);
    mptest_size b_len = mptest__str_view_size(view_b);
    const mptest_char* a_data = mptest__str_view_get_data(view_a);
    const mptest_char* b_data = mptest__str_view_get_data(view_b);
    mptest_size i;
    if (a_len < b_len) {
        return -1;
    } else if (a_len > b_len) {
        return 1;
    }
    for (i = 0; i < a_len; i++) {
        if (a_data[i] != b_data[i]) {
            if (a_data[i] < b_data[i]) {
                return -1;
            } else {
                return 1;
            }
        }
    }
    return 0;
}
#endif /* MPTEST_USE_SYM */
#endif /* MPTEST_USE_DYN_ALLOC */

#if MPTEST_USE_SYM
/* bits/types/fixed/int32 */
/* If this fails, you need to define MPTEST_INT32_TYPE to a signed integer type
 * that is 32 bits wide. */
MPTEST__STATIC_ASSERT(mptest__int32_is_4_bytes, sizeof(mptest_int32) == 4);
#endif /* MPTEST_USE_SYM */

#if MPTEST_USE_APARSE
/* bits/util/ntstr/cmp_n */
MPTEST_INTERNAL int mptest__scmp_n(const char* a, mptest_size a_size, const char* b)
{
    mptest_size a_pos = 0;
    while (1) {
        if (a_pos == a_size) {
            if (*b != '\0') {
                return 0;
            } else {
                /* *b equals '\0' or '=' */
                return 1;
            }
        }
        if (*b == '\0' || a[a_pos] != *b) {
            /* b ended first or a and b do not match */
            return 0;
        }
        a_pos++;
        b++;
    }
    return 0;
}
#endif /* MPTEST_USE_APARSE */

/* bits/util/ntstr/len */
MPTEST_INTERNAL mptest_size mptest__slen(const mptest_char* s) {
    mptest_size sz = 0;
    while (*s) {
        sz++;
        s++;
    }
    return sz;
}

#if MPTEST_USE_APARSE
/* aparse */
MPTEST_API aparse_error aparse_init(aparse_state* state) {
    state->state = (aparse__state*)MPTEST_MALLOC(sizeof(aparse__state));
    if (state->state == MPTEST_NULL) {
        return APARSE_ERROR_NOMEM;
    }
    aparse__state_init(state->state);
    state->state->root = MPTEST_MALLOC(sizeof(aparse__state_root));
        if (state->state->root == MPTEST_NULL) {
        return APARSE_ERROR_NOMEM;
    }
    state->state->root->out_buf_ptr = 0;
    state->state->root->prog_name = MPTEST_NULL;
    state->state->root->prog_name_size = MPTEST_NULL;
    state->state->is_root = 1;
    return APARSE_ERROR_NONE;
}

MPTEST_API void aparse_destroy(aparse_state* state) {
    aparse__state_destroy(state->state);
    if (state->state != MPTEST_NULL) {
        MPTEST_FREE(state->state);
    }
}

MPTEST_API void aparse_set_out_cb(aparse_state* state, aparse_out_cb out_cb, void* user) {
    aparse__state_set_out_cb(state->state, out_cb, user);
}

MPTEST_API aparse_error aparse_add_opt(aparse_state* state, char short_opt, const char* long_opt) {
    aparse__state_check_before_add(state->state);
    return aparse__state_add_opt(state->state, short_opt, long_opt);
}

MPTEST_API aparse_error aparse_add_pos(aparse_state* state, const char* name) {
    aparse__state_check_before_add(state->state);
    return aparse__state_add_pos(state->state, name);
}

MPTEST_API aparse_error aparse_add_sub(aparse_state* state) {
    aparse__state_check_before_add(state->state);
    return aparse__state_add_sub(state->state);
}

MPTEST_API void aparse_arg_help(aparse_state* state, const char* help_text) {
    aparse__state_check_before_modify(state->state);
    state->state->tail->help = help_text;
    if (help_text != MPTEST_NULL) {
        state->state->tail->help_size = mptest__slen(help_text);
    }
}

MPTEST_API void aparse_arg_metavar(aparse_state* state, const char* metavar) {
    aparse__state_check_before_modify(state->state);
    state->state->tail->metavar = metavar;
    if (metavar != MPTEST_NULL) {
        state->state->tail->metavar_size = mptest__slen(metavar);
    }
}

MPTEST_API void aparse_arg_type_bool(aparse_state* state, int* out) {
    aparse__state_check_before_set_type(state->state);
    aparse__arg_bool_init(state->state->tail, out);
}

MPTEST_API void aparse_arg_type_str(aparse_state* state, const char** out, mptest_size* out_size) {
    aparse__state_check_before_set_type(state->state);
    aparse__arg_str_init(state->state->tail, out, out_size);
}

MPTEST_API void aparse_arg_type_help(aparse_state* state) {
    aparse__state_check_before_set_type(state->state);
    aparse__arg_help_init(state->state->tail);
}

MPTEST_API void aparse_arg_type_version(aparse_state* state) {
    aparse__state_check_before_set_type(state->state);
    aparse__arg_version_init(state->state->tail);
}

MPTEST_API void aparse_arg_type_custom(aparse_state* state, aparse_custom_cb cb, void* user, aparse_nargs nargs) {
    aparse__state_check_before_set_type(state->state);
    aparse__arg_custom_init(state->state->tail, cb, user, nargs);
}

MPTEST_API aparse_error aparse_parse(aparse_state* state, int argc, const char* const* argv) {
    aparse_error err = APARSE_ERROR_NONE;
    if (argc == 0) {
        return APARSE_ERROR_INVALID;
    } else {
        state->state->root->prog_name = argv[0];
        state->state->root->prog_name_size = mptest__slen(state->state->root->prog_name);
        err = aparse__parse_argv(state->state, argc - 1, argv + 1);
        if (err == APARSE_ERROR_PARSE) {
            if ((err = aparse__state_flush(state->state))) {
                return err;
            }
            return APARSE_ERROR_PARSE;
        } else if (err == APARSE_ERROR_SHOULD_EXIT) {
            if ((err = aparse__state_flush(state->state))) {
                return err;
            }
            return APARSE_ERROR_SHOULD_EXIT;
        } else {
            return err;
        }
    }
}
#endif /* MPTEST_USE_APARSE */

#if MPTEST_USE_APARSE
/* aparse */
MPTEST_INTERNAL void aparse__arg_init(aparse__arg* arg) {
    arg->type = 0;
    arg->help = MPTEST_NULL;
    arg->metavar = MPTEST_NULL;
    arg->callback = MPTEST_NULL;
    arg->callback_data = MPTEST_NULL;
    arg->callback_data_2.plain = MPTEST_NULL;
    arg->nargs = 0;
    arg->required = 0;
    arg->was_specified = 0;
    arg->next = MPTEST_NULL;
}

MPTEST_INTERNAL void aparse__arg_destroy(aparse__arg* arg) {
    if (arg->destroy != MPTEST_NULL) {
        arg->destroy(arg);
    }
}

MPTEST_INTERNAL void aparse__arg_bool_destroy(aparse__arg* arg);
MPTEST_INTERNAL aparse_error aparse__arg_bool_cb(aparse__arg* arg, aparse__state* state, mptest_size sub_arg_idx, const char* text, mptest_size text_size);

MPTEST_INTERNAL void aparse__arg_bool_init(aparse__arg* arg, int* out) {
    arg->nargs = APARSE_NARGS_0_OR_1_EQ;
    arg->callback = aparse__arg_bool_cb;
    arg->callback_data = (void*)out;
    arg->destroy = aparse__arg_bool_destroy;
}

MPTEST_INTERNAL void aparse__arg_bool_destroy(aparse__arg* arg) {
    MPTEST__UNUSED(arg);
}

MPTEST_INTERNAL aparse_error aparse__arg_bool_cb(aparse__arg* arg, aparse__state* state, mptest_size sub_arg_idx, const char* text, mptest_size text_size) {
    aparse_error err = APARSE_ERROR_NONE;
    int* out = (int*)arg->callback_data;
    MPTEST__UNUSED(state);
    MPTEST__UNUSED(sub_arg_idx);
    if (text == MPTEST_NULL) {
        *out = 1;
        return APARSE_ERROR_NONE;
    } else if (text_size == 1 && *text == '0') {
        *out = 0;
        return APARSE_ERROR_NONE;
    } else if (text_size == 1 && *text == '1') {
        *out = 1;
        return APARSE_ERROR_NONE;
    } else {
        if ((err = aparse__error_begin_arg(state, arg))) {
            return err;
        }
        if ((err = aparse__state_out_s(state, "invalid value for boolean flag: "))) {
            return err;
        }
        if ((err = aparse__error_quote(state, text, text_size))) {
            return err;
        }
        if ((err = aparse__state_out(state, '\n'))) {
            return err;
        }
        return APARSE_ERROR_PARSE;
    }
}

MPTEST_INTERNAL void aparse__arg_str_destroy(aparse__arg* arg);
MPTEST_INTERNAL aparse_error aparse__arg_str_cb(aparse__arg* arg, aparse__state* state, mptest_size sub_arg_idx, const char* text, mptest_size text_size);

MPTEST_INTERNAL void aparse__arg_str_init(aparse__arg* arg, const char** out, mptest_size* out_size) {
    MPTEST_ASSERT(out != MPTEST_NULL);
    arg->nargs = 1;
    arg->callback = aparse__arg_str_cb;
    arg->callback_data = (void*)out;
    arg->callback_data_2.plain = (void*)out_size;
    arg->destroy = aparse__arg_str_destroy;
}

MPTEST_INTERNAL void aparse__arg_str_destroy(aparse__arg* arg) {
    MPTEST__UNUSED(arg);
}

MPTEST_INTERNAL aparse_error aparse__arg_str_cb(aparse__arg* arg, aparse__state* state, mptest_size sub_arg_idx, const char* text, mptest_size text_size) {
    const char** out = (const char**)arg->callback_data;
    mptest_size* out_size = (mptest_size*)arg->callback_data_2.plain;
    MPTEST_ASSERT(text != MPTEST_NULL);
    MPTEST__UNUSED(state);
    MPTEST__UNUSED(sub_arg_idx);
    *out = text;
    if (out_size) {
        *out_size = text_size;
    }
    return APARSE_ERROR_NONE;
}

MPTEST_INTERNAL void aparse__arg_help_destroy(aparse__arg* arg);
MPTEST_INTERNAL aparse_error aparse__arg_help_cb(aparse__arg* arg, aparse__state* state, mptest_size sub_arg_idx, const char* text, mptest_size text_size);

MPTEST_INTERNAL void aparse__arg_help_init(aparse__arg* arg) {
    arg->nargs = 0;
    arg->callback = aparse__arg_help_cb;
    arg->destroy = aparse__arg_help_destroy;
}

MPTEST_INTERNAL void aparse__arg_help_destroy(aparse__arg* arg) {
    MPTEST__UNUSED(arg);
}

MPTEST_INTERNAL aparse_error aparse__arg_help_cb(aparse__arg* arg, aparse__state* state, mptest_size sub_arg_idx, const char* text, mptest_size text_size) {
    aparse_error err = APARSE_ERROR_NONE;
    MPTEST__UNUSED(arg);
    MPTEST__UNUSED(sub_arg_idx);
    MPTEST__UNUSED(text);
    MPTEST__UNUSED(text_size);
    if ((err = aparse__error_usage(state))) {
        return err;
    }
    {
        int has_printed_header = 0;
        aparse__arg* cur = state->head;
        while (cur) {
            if (cur->type != APARSE__ARG_TYPE_POSITIONAL) {
                cur = cur->next;
                continue;
            }
            if (!has_printed_header) {
                if ((err = aparse__state_out_s(state, "\npositional arguments:\n"))) {
                    return err;
                }
                has_printed_header = 1;
            }
            if ((err = aparse__state_out_s(state, "  "))) {
                return err;
            }
            if (cur->metavar == MPTEST_NULL) {
                if ((err = aparse__state_out_n(state, cur->contents.pos.name, cur->contents.pos.name_size))) {
                    return err;
                }
            } else {
                if ((err = aparse__state_out_n(state, cur->metavar, cur->metavar_size))) {
                    return err;
                }
            }
            if ((err = aparse__state_out(state, '\n'))) {
                return err;
            }
            if (cur->help != MPTEST_NULL) {
                if ((err = aparse__state_out_s(state, "    "))) {
                    return err;
                }
                if ((err = aparse__state_out_n(state, cur->help, cur->help_size))) {
                    return err;
                }
                if ((err = aparse__state_out(state, '\n'))) {
                    return err;
                }
            }
            cur = cur->next;
        }
    }
    {
        int has_printed_header = 0;
        aparse__arg* cur = state->head;
        while (cur) {
            if (cur->type != APARSE__ARG_TYPE_OPTIONAL) {
                cur = cur->next;
                continue;
            }
            if (!has_printed_header) {
                if ((err = aparse__state_out_s(state, "\noptional arguments:\n"))) {
                    return err;
                }
                has_printed_header = 1;
            }
            if ((err = aparse__state_out_s(state, "  "))) {
                return err;
            }
            if (cur->contents.opt.short_opt != '\0') {
                if ((err = aparse__error_print_short_opt(state, cur))) {
                    return err;
                }
                if (cur->nargs != APARSE_NARGS_0_OR_1_EQ &&
                    cur->nargs != 0) {
                    if ((err = aparse__state_out(state, ' '))) {
                        return err;
                    }
                }
                if ((err = aparse__error_print_sub_args(state, cur))) {
                    return err;
                }
            }
            if (cur->contents.opt.long_opt != MPTEST_NULL) {
                if (cur->contents.opt.short_opt != '\0') {
                    if ((err = aparse__state_out_s(state, ", "))) {
                        return err;
                    }
                }
                if ((err = aparse__error_print_long_opt(state, cur))) {
                    return err;
                }
                if (cur->nargs != APARSE_NARGS_0_OR_1_EQ &&
                    cur->nargs != 0) {
                    if ((err = aparse__state_out(state, ' '))) {
                        return err;
                    }
                }
                if ((err = aparse__error_print_sub_args(state, cur))) {
                    return err;
                }
            }
            if ((err = aparse__state_out(state, '\n'))) {
                return err;
            }
            if (cur->help != MPTEST_NULL) {
                if ((err = aparse__state_out_s(state, "    "))) {
                    return err;
                }
                if ((err = aparse__state_out_n(state, cur->help, cur->help_size))) {
                    return err;
                }
                if ((err = aparse__state_out(state, '\n'))) {
                    return err;
                }
            }
            cur = cur->next;
        }
    }
    return APARSE_SHOULD_EXIT;
}

MPTEST_INTERNAL void aparse__arg_version_destroy(aparse__arg* arg);
MPTEST_INTERNAL aparse_error aparse__arg_version_cb(aparse__arg* arg, aparse__state* state, mptest_size sub_arg_idx, const char* text, mptest_size text_size);

MPTEST_INTERNAL void aparse__arg_version_init(aparse__arg* arg) {
    arg->nargs = 0;
    arg->callback = aparse__arg_version_cb;
    arg->destroy = aparse__arg_version_destroy;
}

MPTEST_INTERNAL void aparse__arg_version_destroy(aparse__arg* arg) {
    MPTEST__UNUSED(arg);
}

MPTEST_INTERNAL aparse_error aparse__arg_version_cb(aparse__arg* arg, aparse__state* state, mptest_size sub_arg_idx, const char* text, mptest_size text_size) {
    aparse_error err = APARSE_ERROR_NONE;
    MPTEST__UNUSED(arg);
    MPTEST__UNUSED(sub_arg_idx);
    MPTEST__UNUSED(text);
    MPTEST__UNUSED(text_size);
    /* TODO: print version */
    if ((err = aparse__state_out_s(state, "version\n"))) {
        return err;
    }
    return APARSE_SHOULD_EXIT;
}

MPTEST_INTERNAL void aparse__arg_custom_destroy(aparse__arg* arg);
MPTEST_INTERNAL aparse_error aparse__arg_custom_cb(aparse__arg* arg, aparse__state* state, mptest_size sub_arg_idx, const char* text, mptest_size text_size);

MPTEST_INTERNAL void aparse__arg_custom_init(aparse__arg* arg, aparse_custom_cb cb, void* user, aparse_nargs nargs) {
    arg->nargs = nargs;
    arg->callback = aparse__arg_custom_cb;
    arg->callback_data = (void*)user;
    arg->callback_data_2.custom_cb = cb;
    arg->destroy = aparse__arg_custom_destroy;
}

MPTEST_INTERNAL void aparse__arg_custom_destroy(aparse__arg* arg) {
    MPTEST__UNUSED(arg);
}

MPTEST_INTERNAL aparse_error aparse__arg_custom_cb(aparse__arg* arg, aparse__state* state, mptest_size sub_arg_idx, const char* text, mptest_size text_size) {
    aparse_custom_cb cb = (aparse_custom_cb)arg->callback_data_2.custom_cb;
    aparse_state state_;
    state_.state = state;
    return cb(arg->callback_data, &state_, (int)sub_arg_idx, text, text_size);
}

MPTEST_INTERNAL void aparse__arg_sub_destroy(aparse__arg* arg);

MPTEST_INTERNAL void aparse__arg_sub_init(aparse__arg* arg) {
    arg->type = APARSE__ARG_TYPE_SUBCOMMAND;
    arg->contents.sub.head = MPTEST_NULL;
    arg->contents.sub.tail = MPTEST_NULL;
    arg->destroy = aparse__arg_sub_destroy;
}

MPTEST_INTERNAL void aparse__arg_sub_destroy(aparse__arg* arg) {
    aparse__sub* sub = arg->contents.sub.head;
    MPTEST_ASSERT(arg->type == APARSE__ARG_TYPE_SUBCOMMAND);
    while (sub) {
        aparse__sub* prev = sub;
        aparse__state_destroy(&prev->subparser);
        sub = prev->next;
        MPTEST_FREE(prev);
    }
}
#endif /* MPTEST_USE_APARSE */

#if MPTEST_USE_APARSE
/* aparse */
MPTEST_INTERNAL aparse_error aparse__error_begin_progname(aparse__state* state) {
    aparse_error err = APARSE_ERROR_NONE;
    if ((err = aparse__state_out_n(state, state->root->prog_name, state->root->prog_name_size))) {
        return err;
    }
    if ((err = aparse__state_out_s(state, ": "))) {
        return err;
    }
    return err;
}

MPTEST_INTERNAL aparse_error aparse__error_begin(aparse__state* state) {
    aparse_error err = APARSE_ERROR_NONE;
    if ((err = aparse__error_begin_progname(state))) {
        return err;
    }
    if ((err = aparse__state_out_s(state, "error: "))) {
        return err;
    }
    return err;
}

MPTEST_INTERNAL aparse_error aparse__error_print_short_opt(aparse__state* state, const aparse__arg* arg) {
    aparse_error err = APARSE_ERROR_NONE;
    MPTEST_ASSERT(arg->contents.opt.short_opt);
    if ((err = aparse__state_out(state, '-'))) {
        return err;
    }
    if ((err = aparse__state_out(state, arg->contents.opt.short_opt))) {
        return err;
    }
    return err;
}

MPTEST_INTERNAL aparse_error aparse__error_print_long_opt(aparse__state* state, const aparse__arg* arg) {
    aparse_error err = APARSE_ERROR_NONE;
    MPTEST_ASSERT(arg->contents.opt.long_opt);
    if ((err = aparse__state_out_s(state, "--"))) {
        return err;
    }
    if ((err = aparse__state_out_n(state, arg->contents.opt.long_opt, arg->contents.opt.long_opt_size))) {
        return err;
    }
    return err;
}

MPTEST_INTERNAL aparse_error aparse__error_begin_opt(aparse__state* state, const aparse__arg* arg) {
    aparse_error err = APARSE_ERROR_NONE;
    MPTEST_ASSERT(arg->type == APARSE__ARG_TYPE_OPTIONAL);
    if ((err = aparse__error_begin(state))) {
        return err;
    }
    if ((err = aparse__state_out_s(state, "option "))) {
        return err;
    }
    if (arg->contents.opt.short_opt != '\0') {
        if ((err = aparse__error_print_short_opt(state, arg))) {
            return err;
        }
    }
    if (arg->contents.opt.long_opt != MPTEST_NULL) {
        if (arg->contents.opt.short_opt != '\0') {
            if ((err = aparse__state_out_s(state, ", "))) {
                return err;
            }
        }
        if ((err = aparse__error_print_long_opt(state, arg))) {
            return err;
        }
    }
    if ((err = aparse__state_out_s(state, ": "))) {
        return err;
    }
    return err;
}

MPTEST_INTERNAL aparse_error aparse__error_begin_pos(aparse__state* state, const aparse__arg* arg) {
    aparse_error err = APARSE_ERROR_NONE;
    MPTEST_ASSERT(arg->type == APARSE__ARG_TYPE_POSITIONAL);
    if ((err = aparse__error_begin(state))) {
        return err;
    }
    if ((err = aparse__state_out_s(state, "argument "))) {
        return err;
    }
    if ((err = aparse__state_out_n(state, arg->contents.pos.name, arg->contents.pos.name_size))) {
        return err;
    }
    if ((err = aparse__state_out_s(state, ": "))) {
        return err;
    }
    return err;
}

MPTEST_INTERNAL aparse_error aparse__error_begin_arg(aparse__state* state, const aparse__arg* arg) {
    if (arg->type == APARSE__ARG_TYPE_OPTIONAL) {
        return aparse__error_begin_opt(state, arg);
    } else {
        return aparse__error_begin_pos(state, arg);
    }
}

MPTEST_INTERNAL aparse_error aparse__error_unrecognized_arg(aparse__state* state, const char* arg) {
    aparse_error err = APARSE_ERROR_NONE;
    if ((err = aparse__error_begin(state))) {
        return err;
    }
    if ((err = aparse__state_out_s(state, "unrecognized argument: "))) {
        return err;
    }
    if ((err = aparse__state_out_s(state, arg))) {
        return err;
    }
    if ((err = aparse__state_out(state, '\n'))) {
        return err;
    }
    return err;
}

MPTEST_INTERNAL char aparse__hexdig(unsigned char c) {
    if (c < 10) {
        return '0' + (char)c;
    } else {
        return 'a' + ((char)c - 10);
    }
}

MPTEST_INTERNAL aparse_error aparse__error_quote(aparse__state* state, const char* text, mptest_size text_size) {
    aparse_error err = APARSE_ERROR_NONE;
    mptest_size i;
    if ((err = aparse__state_out(state, '"'))) {
        return err;
    }
    for (i = 0; i < text_size; i++) {
        char c = text[i];
        if (c < ' ') {
            if ((err = aparse__state_out(state, '\\'))) {
                return err;
            }
            if ((err = aparse__state_out(state, 'x'))) {
                return err;
            }
            if ((err = aparse__state_out(state, aparse__hexdig((c >> 4) & 0xF)))) {
                return err;
            }
            if ((err = aparse__state_out(state, aparse__hexdig(c & 0xF)))) {
                return err;
            }
        } else {
            if ((err = aparse__state_out(state, c))) {
                return err;
            }
        }
    }
    if ((err = aparse__state_out(state, '"'))) {
        return err;
    }
    return err;
}

int aparse__error_can_coalesce_in_usage(const aparse__arg* arg) {
    if (arg->type != APARSE__ARG_TYPE_OPTIONAL) {
        return 0;
    }
    if (arg->required) {
        return 0;
    }
    if (arg->contents.opt.short_opt == '\0') {
        return 0;
    }
    if ((arg->nargs != APARSE_NARGS_0_OR_1_EQ) && (arg->nargs != 0)) {
        return 0;
    }
    return 1;
}

MPTEST_INTERNAL aparse_error aparse__error_print_sub_args(aparse__state* state, const aparse__arg* arg) {
    aparse_error err = APARSE_ERROR_NONE;
    const char* var;
    mptest_size var_size;
    if (arg->metavar != MPTEST_NULL) {
        var = arg->metavar;
        var_size = arg->metavar_size;
    } else if (arg->type == APARSE__ARG_TYPE_POSITIONAL) {
        var = arg->contents.pos.name;
        var_size = arg->contents.pos.name_size;
    } else {
        var = "ARG";
        var_size = 3;
    }
    if (arg->nargs == APARSE_NARGS_1_OR_MORE) {
        if ((err = aparse__state_out_n(state, var, var_size))) {
            return err;
        }
        if ((err = aparse__state_out_s(state, " ["))) {
            return err;
        }
        if ((err = aparse__state_out_n(state, var, var_size))) {
            return err;
        }
        if ((err = aparse__state_out_s(state, " ...]]"))) {
            return err;
        }
    } else if (arg->nargs == APARSE_NARGS_0_OR_MORE) {
        if ((err = aparse__state_out(state, '['))) {
            return err;
        }
        if ((err = aparse__state_out_n(state, var, var_size))) {
            return err;
        }
        if ((err = aparse__state_out_s(state, " ["))) {
            return err;
        }
        if ((err = aparse__state_out_n(state, var, var_size))) {
            return err;
        }
        if ((err = aparse__state_out_s(state, " ...]]"))) {
            return err;
        }
    } else if (arg->nargs == APARSE_NARGS_0_OR_1_EQ) {
        /* pass */
    } else if (arg->nargs == APARSE_NARGS_0_OR_1) {
        if ((err = aparse__state_out(state, '['))) {
            return err;
        }
        if ((err = aparse__state_out_n(state, var, var_size))) {
            return err;
        }
        if ((err = aparse__state_out(state, ']'))) {
            return err;
        }
    } else if (arg->nargs > 0) {
        int i;
        for (i = 0; i < arg->nargs; i++) {
            if (i) {
                if ((err = aparse__state_out(state, ' '))) {
                    return err;
                }
            }
            if ((err = aparse__state_out_n(state, var, var_size))) {
                return err;
            }
        }
    }
    return err;
}

MPTEST_INTERNAL aparse_error aparse__error_usage(aparse__state* state) {
    aparse_error err = APARSE_ERROR_NONE;
    const aparse__arg* cur = state->head;
    int has_printed = 0;
    if ((err = aparse__state_out_s(state, "usage: "))) {
        return err;
    }
    if ((err = aparse__state_out_n(state, state->root->prog_name, state->root->prog_name_size))) {
        return err;
    }
    /* print coalesced args */
    while (cur) {
        if (aparse__error_can_coalesce_in_usage(cur)) {
            if (!has_printed) {
                if ((err = aparse__state_out_s(state, " [-"))) {
                    return err;
                }
                has_printed = 1;
            }
            if ((err = aparse__state_out(state, cur->contents.opt.short_opt))) {
                return err;
            }
        }
        cur = cur->next;
    }
    if (has_printed) {
        if ((err = aparse__state_out(state, ']'))) {
            return err;
        }
    }
    /* print other args */
    cur = state->head;
    while (cur) {
        if (!aparse__error_can_coalesce_in_usage(cur)) {
            if ((err = aparse__state_out(state, ' '))) {
                return err;
            }
            if (!cur->required) {
                if ((err = aparse__state_out(state, '['))) {
                    return err;
                }
            }
            if (cur->type == APARSE__ARG_TYPE_OPTIONAL) {
                if (cur->contents.opt.short_opt) {
                    if ((err = aparse__error_print_short_opt(state, cur))) {
                        return err;
                    }
                } else if (cur->contents.opt.long_opt) {
                    if ((err = aparse__error_print_long_opt(state, cur))) {
                        return err;
                    }
                }
                if (cur->nargs != APARSE_NARGS_0_OR_1_EQ &&
                    cur->nargs != 0) {
                    if ((err = aparse__state_out(state, ' '))) {
                        return err;
                    }
                }
            }
            if ((err = aparse__error_print_sub_args(state, cur))) {
                return err;
            }
            if (!cur->required) {
                if ((err = aparse__state_out(state, ']'))) {
                    return err;
                }
            }
        }
        cur = cur->next;
    }
    if ((err = aparse__state_out(state, '\n'))) {
        return err;
    }
    return err;
}

#if 0
MPTEST_INTERNAL void aparse__error_print_opt_name(aparse_state* state,
    const struct aparse__arg*                   opt)
{
    (void)(state);
    if (opt->short_opt && !opt->long_opt) {
        /* Only short option was specified */
        fprintf(stderr, "-%c", opt->short_opt);
    } else if (opt->short_opt && opt->long_opt) {
        /* Both short option and long option were specified */
        fprintf(stderr, "-%c/--%s", opt->short_opt, opt->long_opt);
    } else if (opt->long_opt) {
        /* Only long option was specified */
        fprintf(stderr, "--%s", opt->long_opt);
    }
}

MPTEST_INTERNAL void aparse__error_print_usage_coalesce_short_args(aparse_state* state)
{
    /* Print optional short options without arguments */
    size_t i;
    int    has_printed_short_opt_no_arg = 0;
    for (i = 0; i < state->args_size; i++) {
        const struct aparse__arg* current_opt = &state->args[i];
        /* Filter out positional options */
        if (current_opt->type == APARSE__ARG_TYPE_POSITIONAL) {
            continue;
        }
        /* Filter out required options */
        if (current_opt->required) {
            continue;
        }
        /* Filter out options with no short option */
        if (!current_opt->short_opt) {
            continue;
        }
        /* Filter out options with nargs that don't coalesce */
        if (!aparse__nargs_can_coalesce(current_opt->nargs)) {
            continue;
        }
        /* Print the initial '[-' */
        if (!has_printed_short_opt_no_arg) {
            has_printed_short_opt_no_arg = 1;
            fprintf(stderr, " [-");
        }
        fprintf(stderr, "%c", current_opt->short_opt);
    }
    if (has_printed_short_opt_no_arg) {
        fprintf(stderr, "]");
    }
}

MPTEST_INTERNAL void aparse__error_print_usage_arg(aparse_state* state,
    const struct aparse__arg*                    current_arg)
{
    const char* metavar = "ARG";
    (void)(state);
    if (current_arg->metavar) {
        metavar = current_arg->metavar;
    }
    /* Optional arguments are encased in [] */
    if (!current_arg->required) {
        fprintf(stderr, "[");
    }
    /* Print option name */
    if (current_arg->type == APARSE__ARG_TYPE_OPTIONAL) {
        if (current_arg->short_opt) {
            fprintf(stderr, "-%c", current_arg->short_opt);
        } else {
            fprintf(stderr, "--%s", current_arg->long_opt);
        }
        /* Space separates the option name from the arguments */
        if (!aparse__nargs_can_coalesce(current_arg->nargs)) {
            fprintf(stderr, " ");
        }
    }
    if (current_arg->nargs == APARSE_NARGS_0_OR_1) {
        fprintf(stderr, "[%s]", metavar);
    } else if (current_arg->nargs == APARSE_NARGS_0_OR_MORE) {
        fprintf(stderr, "[%s ...]", metavar);
    } else if (current_arg->nargs == APARSE_NARGS_1_OR_MORE) {
        fprintf(stderr, "%s [%s ...]", metavar, metavar);
    } else {
        int j = (int)current_arg->nargs;
        for (j = 0; j < current_arg->nargs; j++) {
            if (j) {
                fprintf(stderr, " ");
            }
            fprintf(stderr, "%s", metavar);
        }
    }
    if (!current_arg->required) {
        fprintf(stderr, "]");
    }
}

MPTEST_INTERNAL void aparse__error_print_usage(aparse_state* state)
{
    size_t                    i;
    const struct aparse__arg* current_arg;
    if (state->argc == 0) {
        fprintf(stderr, "usage:");
    } else {
        fprintf(stderr, "usage: %s", state->argv[0]);
    }
    /* Print optional short options with no arguments first */
    aparse__error_print_usage_coalesce_short_args(state);
    /* Print other optional options */
    for (i = 0; i < state->args_size; i++) {
        current_arg = &state->args[i];
        /* Filter out positionals */
        if (current_arg->type == APARSE__ARG_TYPE_POSITIONAL) {
            continue;
        }
        /* Filter out required options */
        if (current_arg->required) {
            continue;
        }
        /* Filter out options we printed in coalesce */
        if (aparse__nargs_can_coalesce(current_arg->nargs)) {
            continue;
        }
        fprintf(stderr, " ");
        aparse__error_print_usage_arg(state, current_arg);
    }
    /* Print mandatory options */
    for (i = 0; i < state->args_size; i++) {
        current_arg = &state->args[i];
        /* Filter out positionals */
        if (current_arg->type == APARSE__ARG_TYPE_POSITIONAL) {
            continue;
        }
        /* Filter out optional options */
        if (!current_arg->required) {
            continue;
        }
        fprintf(stderr, " ");
        aparse__error_print_usage_arg(state, current_arg);
    }
    /* Print mandatory positionals */
    for (i = 0; i < state->args_size; i++) {
        current_arg = &state->args[i];
        /* Filter out optionals */
        if (current_arg->type == APARSE__ARG_TYPE_OPTIONAL) {
            continue;
        }
        /* Filter out optional positionals */
        if (!current_arg->required) {
            continue;
        }
        fprintf(stderr, " ");
        aparse__error_print_usage_arg(state, current_arg);
    }
    /* Print optional positionals */
    for (i = 0; i < state->args_size; i++) {
        current_arg = &state->args[i];
        /* Filter out optionals */
        if (current_arg->type == APARSE__ARG_TYPE_OPTIONAL) {
            continue;
        }
        /* Filter out mandatory positionals */
        if (current_arg->required) {
            continue;
        }
        fprintf(stderr, " ");
        aparse__error_print_usage_arg(state, current_arg);
    }
    fprintf(stderr, "\n");
}

MPTEST_INTERNAL void aparse__error_begin(aparse_state* state)
{
    aparse__error_print_usage(state);
    if (state->argc == 0) {
        fprintf(stderr, "error: ");
    } else {
        fprintf(stderr, "%s: error: ", state->argv[0]);
    }
}

MPTEST_INTERNAL void aparse__error_end(aparse_state* state)
{
    MPTEST__UNUSED(state);
    fprintf(stderr, "\n");
}

MPTEST_INTERNAL void aparse__error_arg_begin(aparse_state* state)
{
    MPTEST__UNUSED(state);
}

MPTEST_INTERNAL void aparse__error_arg_end(aparse_state* state)
{
    MPTEST__UNUSED(state);
}

#endif
#endif /* MPTEST_USE_APARSE */

#if MPTEST_USE_APARSE
/* aparse */
#include <stdio.h>

MPTEST_INTERNAL aparse_error aparse__state_default_out_cb(void* user, const char* buf, mptest_size buf_size) {
    MPTEST__UNUSED(user);
    if (fwrite(buf, buf_size, 1, stdout) != 1) {
        return APARSE_ERROR_OUT;
    }
    return APARSE_ERROR_NONE;
}

MPTEST_INTERNAL void aparse__state_init(aparse__state* state) {
    state->head = MPTEST_NULL;
    state->tail = MPTEST_NULL;
    state->help = MPTEST_NULL;
    state->help_size = 0;
    state->out_cb = aparse__state_default_out_cb;
    state->user = MPTEST_NULL;
    state->root = MPTEST_NULL;
    state->is_root = 0;
}

#if 0

MPTEST_INTERNAL void aparse__state_init_from(aparse__state* state, aparse__state* other) {
    aparse__state_init(state);
    state->out_cb = other->out_cb;
    state->user = other->user;
    state->root = other->root;
}

#endif

MPTEST_INTERNAL void aparse__state_destroy(aparse__state* state) {
    aparse__arg* arg = state->head;
    while (arg) {
        aparse__arg* prev = arg;
        arg = arg->next;
        aparse__arg_destroy(prev);
        MPTEST_FREE(prev);
    }
    if (state->is_root) {
        if (state->root != MPTEST_NULL) {
            MPTEST_FREE(state->root);
        }
    }
}

MPTEST_INTERNAL void aparse__state_set_out_cb(aparse__state* state, aparse_out_cb out_cb, void* user) {
    state->out_cb = out_cb;
    state->user = user;
}

MPTEST_INTERNAL void aparse__state_reset(aparse__state* state) {
    aparse__arg* cur = state->head;
    while (cur) {
        cur->was_specified = 0;
        if (cur->type == APARSE__ARG_TYPE_SUBCOMMAND) {
            aparse__sub* sub = cur->contents.sub.head;
            while (sub) {
                aparse__state_reset(&sub->subparser);
                sub = sub->next;
            }
        }
        cur = cur->next;
    }
}

MPTEST_INTERNAL aparse_error aparse__state_arg(aparse__state* state) {
    aparse__arg* arg = (aparse__arg*)MPTEST_MALLOC(sizeof(aparse__arg));
    if (arg == MPTEST_NULL) {
        return APARSE_ERROR_NOMEM;
    }
    aparse__arg_init(arg);
    if (state->head == MPTEST_NULL) {
        state->head = arg;
        state->tail = arg;
    } else {
        state->tail->next = arg;
        state->tail = arg;
    }
    return APARSE_ERROR_NONE;
}

MPTEST_INTERNAL void aparse__state_check_before_add(aparse__state* state) {
    /* for release builds */
    MPTEST__UNUSED(state);

    /* If this fails, you forgot to specifiy a type for the previous argument. */
    MPTEST_ASSERT(MPTEST__IMPLIES(state->tail != MPTEST_NULL, state->tail->callback != MPTEST_NULL));
}

MPTEST_INTERNAL void aparse__state_check_before_modify(aparse__state* state) {
    /* for release builds */
    MPTEST__UNUSED(state);

    /* If this fails, you forgot to call add_opt() or add_pos(). */
    MPTEST_ASSERT(state->tail != MPTEST_NULL);
}

MPTEST_INTERNAL void aparse__state_check_before_set_type(aparse__state* state) {
    /* for release builds */
    MPTEST__UNUSED(state);

    /* If this fails, you forgot to call add_opt() or add_pos(). */
    MPTEST_ASSERT(state->tail != MPTEST_NULL);

    /* If this fails, you are trying to set the argument type of a subcommand. */
    MPTEST_ASSERT(state->tail->type != APARSE__ARG_TYPE_SUBCOMMAND);

    /* If this fails, you called arg_xxx() twice. */
    MPTEST_ASSERT(state->tail->callback == MPTEST_NULL);
}

MPTEST_INTERNAL aparse_error aparse__state_add_opt(aparse__state* state, char short_opt, const char* long_opt) {
    aparse_error err = APARSE_ERROR_NONE;
    /* If either of these fail, you specified both short_opt and long_opt as
     * NULL. For a positional argument, use aparse__add_pos. */
    MPTEST_ASSERT(MPTEST__IMPLIES(short_opt == '\0', long_opt != MPTEST_NULL));
    MPTEST_ASSERT(MPTEST__IMPLIES(long_opt == MPTEST_NULL, short_opt != '\0'));
    if ((err = aparse__state_arg(state))) {
        return err;
    }
    state->tail->type = APARSE__ARG_TYPE_OPTIONAL;
    state->tail->contents.opt.short_opt = short_opt;
    state->tail->contents.opt.long_opt = long_opt;
    if (long_opt != MPTEST_NULL) {
        state->tail->contents.opt.long_opt_size = mptest__slen(long_opt);
    } else {
        state->tail->contents.opt.long_opt_size = 0;
    }
    return err;
}

MPTEST_INTERNAL aparse_error aparse__state_add_pos(aparse__state* state, const char* name) {
    aparse_error err = APARSE_ERROR_NONE;
    if ((err = aparse__state_arg(state))) {
        return err;
    }
    state->tail->type = APARSE__ARG_TYPE_POSITIONAL;
    state->tail->contents.pos.name = name;
    state->tail->contents.pos.name_size = mptest__slen(name);
    return err;
}

MPTEST_INTERNAL aparse_error aparse__state_add_sub(aparse__state* state) {
    aparse_error err = APARSE_ERROR_NONE;
    if ((err = aparse__state_arg(state))) {
        return err;
    }
    aparse__arg_sub_init(state->tail);
    return err;
}

#if 0
MPTEST_INTERNAL aparse_error aparse__state_sub_add_cmd(aparse__state* state, const char* name, aparse__state** subcmd) {
    aparse__sub* sub = (aparse__sub*)MPTEST_MALLOC(sizeof(aparse__sub));
    MPTEST_ASSERT(state->tail->type == APARSE__ARG_TYPE_SUBCOMMAND);
    MPTEST_ASSERT(name != MPTEST_NULL);
    if (sub == MPTEST_NULL) {
        return APARSE_ERROR_NOMEM;
    }
    sub->name = name;
    sub->name_size = mptest__slen(name);
    aparse__state_init_from(&sub->subparser, state);
    sub->next = MPTEST_NULL;
    if (state->tail->contents.sub.head == MPTEST_NULL) {
        state->tail->contents.sub.head = sub;
        state->tail->contents.sub.tail = sub;
    } else {
        state->tail->contents.sub.tail->next = sub;
        state->tail->contents.sub.tail = sub;
    }
    *subcmd = &sub->subparser;
    return 0;
}

#endif


MPTEST_INTERNAL aparse_error aparse__state_flush(aparse__state* state) {
    aparse_error err = APARSE_ERROR_NONE;
    if (state->root->out_buf_ptr) {
        if ((err = state->out_cb(state->user, state->root->out_buf, state->root->out_buf_ptr))) {
            return err;
        }
        state->root->out_buf_ptr = 0;
    }
    return err;
}

MPTEST_INTERNAL aparse_error aparse__state_out(aparse__state* state, char out) {
    aparse_error err = APARSE_ERROR_NONE;
    if (state->root->out_buf_ptr == APARSE__STATE_OUT_BUF_SIZE) {
        if ((err = aparse__state_flush(state))) {
            return err;
        }
    }
    state->root->out_buf[state->root->out_buf_ptr++] = out;
    return err;
}

MPTEST_INTERNAL aparse_error aparse__state_out_s(aparse__state* state, const char* s) {
    aparse_error err = APARSE_ERROR_NONE;
    while (*s) {
        if ((err = aparse__state_out(state, *s))) {
            return err;
        }
        s++;
    }
    return err;
}

MPTEST_INTERNAL aparse_error aparse__state_out_n(aparse__state* state, const char* s, mptest_size n) {
    aparse_error err = APARSE_ERROR_NONE;
    mptest_size i;
    for (i = 0; i < n; i++) {
        if ((err = aparse__state_out(state, s[i]))) {
            return err;
        }
    }
    return err;
}
#endif /* MPTEST_USE_APARSE */

#if MPTEST_USE_APARSE
/* aparse */
/* accepts an lvalue */
#define APARSE__NEXT_POSITIONAL(n) \
    while ((n) != MPTEST_NULL && (n)->type != APARSE__ARG_TYPE_POSITIONAL) { \
        (n) = (n)->next; \
    }

int aparse__is_positional(const char* arg_text) {
    return (arg_text[0] == '\0') /* empty string */
            || (arg_text[0] == '-' && arg_text[1] == '\0') /* just a dash */
            || (arg_text[0] == '-' && arg_text[1] == '-' && arg_text[2] == '\0') /* two dashes */
            || (arg_text[0] != '-'); /* all positionals */
}

/* Returns NULL if the option does not match. */
const char* aparse__arg_match_long_opt(const struct aparse__arg* opt,
    const char* arg_without_dashes)
{
    mptest_size a_pos = 0;
    const char* a_str = opt->contents.opt.long_opt;
    const char* b = arg_without_dashes;
    while (1) {
        if (a_pos == opt->contents.opt.long_opt_size) {
            if (*b != '\0' && *b != '=') {
                return NULL;
            } else {
                /* *b equals '\0' or '=' */
                return b;
            }
        }
        if (*b == '\0' || a_str[a_pos] != *b) {
            /* b ended first or a and b do not match */
            return NULL;
        }
        a_pos++;
        b++;
    }
    return NULL;
}

MPTEST_API aparse_error aparse__parse_argv(aparse__state* state, int argc, const char* const* argv) {
    aparse_error err = APARSE_ERROR_NONE;
    int argc_idx = 0;
    aparse__arg* next_positional = state->head;
    mptest_size arg_text_size;
    APARSE__NEXT_POSITIONAL(next_positional);
    aparse__state_reset(state);
    while (argc_idx < argc) {
        const char* arg_text = argv[argc_idx++];
        if (aparse__is_positional(arg_text)) {
            if (next_positional == MPTEST_NULL) {
                if ((err = aparse__error_unrecognized_arg(state, arg_text))) {
                    return err;
                }
                return APARSE_ERROR_PARSE;
            }
            arg_text_size = mptest__slen((const mptest_char*)arg_text);
            if ((err = next_positional->callback(next_positional, state, 0, arg_text, arg_text_size))) {
                return err;
            }
            APARSE__NEXT_POSITIONAL(next_positional);
        } else {
            int is_long = 0;
            const char* arg_end;
            if (arg_text[0] == '-' && arg_text[1] != '-') {
                arg_end = arg_text + 1;
            } else {
                arg_end = arg_text + 2;
                is_long = 1;
            }
            do {
                aparse__arg* arg = state->head;
                int has_text_left = 0;
                if (!is_long) {
                    char short_opt = *(arg_end++);
                    while (1) {
                        if (arg == MPTEST_NULL) {
                            break;
                        }
                        if (arg->type == APARSE__ARG_TYPE_OPTIONAL) {
                            if (arg->contents.opt.short_opt == short_opt) {
                                break;
                            }
                        }
                        arg = arg->next;
                    }
                    if (arg == MPTEST_NULL) {
                        if ((err = aparse__error_unrecognized_arg(state, arg_text))) {
                            return err;
                        }
                        return APARSE_ERROR_PARSE;
                    }
                    has_text_left = *arg_end != '\0';
                } else {
                    while (1) {
                        if (arg == MPTEST_NULL) {
                            break;
                        }
                        if (arg->type == APARSE__ARG_TYPE_OPTIONAL) {
                            if (arg->contents.opt.long_opt != MPTEST_NULL) {
                                mptest_size opt_pos = 0;
                                const char* opt_ptr = arg->contents.opt.long_opt;
                                const char* arg_ptr = arg_end;
                                int found = 0;
                                while (1) {
                                    if (opt_pos == arg->contents.opt.long_opt_size) {
                                        if (*arg_ptr != '\0' && *arg_ptr != '=') {
                                            break;
                                        } else {
                                            /* *b equals '\0' or '=' */
                                            arg_end = arg_ptr;
                                            found = 1;
                                            break;
                                        }
                                    }
                                    if (*arg_ptr == '\0' || opt_ptr[opt_pos] != *arg_ptr) {
                                        /* b ended first or a and b do not match */
                                        break;
                                    }
                                    opt_pos++;
                                    arg_ptr++;
                                }
                                if (found) {
                                    break;
                                }
                            }
                        }
                        arg = arg->next;
                    }
                    if (arg == MPTEST_NULL) {
                        if ((err = aparse__error_unrecognized_arg(state, arg_text))) {
                            return err;
                        }
                        return APARSE_ERROR_PARSE;
                    }
                }
                if (*arg_end == '=') {
                    /* use equals as argument */
                    if (arg->nargs == 0
                        || arg->nargs > 1) {
                        if ((err = aparse__error_begin_arg(state, arg))) {
                            return err;
                        }
                        if ((err = aparse__state_out_s(state, "cannot parse '='\n"))) {
                            return err;
                        }
                        return APARSE_ERROR_PARSE;
                    } else  {
                        arg_end++;
                        if ((err = arg->callback(arg, state, 0, arg_end, mptest__slen(arg_end)))) {
                            return err;
                        }
                    }
                    break;
                } else if (has_text_left) {
                    /* use rest of arg as argument */
                    if (arg->nargs > 1) {
                        if ((err = aparse__error_begin_arg(state, arg))) {
                            return err;
                        }
                        if ((err = aparse__state_out_s(state, "cannot parse '"))) {
                            return err;
                        }
                        if ((err = aparse__state_out_s(state, arg_end))) {
                            return err;
                        }
                        if ((err = aparse__state_out(state, '\n'))) {
                            return err;
                        }
                        return APARSE_ERROR_PARSE;
                    } else if (arg->nargs != APARSE_NARGS_0_OR_1_EQ &&
                        arg->nargs != 0) {
                        if ((err = arg->callback(arg, state, 0, arg_end, mptest__slen(arg_end)))) {
                            return err;
                        }
                        break;
                    } else {
                        if ((err = arg->callback(arg, state, 0, MPTEST_NULL, 0))) {
                            return err;
                        }
                        /* fallthrough, continue parsing short options */
                    }
                } else if (argc_idx == argc || !aparse__is_positional(argv[argc_idx])) {
                    if (arg->nargs == APARSE_NARGS_1_OR_MORE
                        || arg->nargs == 1
                        || arg->nargs > 1) {
                        if ((err = aparse__error_begin_arg(state, arg))) {
                            return err;
                        }
                        if ((err = aparse__state_out_s(state, "expected an argument\n"))) {
                            return err;
                        }
                        return APARSE_ERROR_PARSE;
                    } else if (arg->nargs == APARSE_NARGS_0_OR_1_EQ
                            || arg->nargs == 0) {
                        if ((err = arg->callback(arg, state, 0, MPTEST_NULL, 0))) {
                            return err;
                        }
                        /* fallthrough */  
                    } else {
                        if ((err = arg->callback(arg, state, 0, MPTEST_NULL, 0))) {
                            return err;
                        }
                    }
                    break;
                } else {
                    if (arg->nargs == APARSE_NARGS_0_OR_1
                        || arg->nargs == 1) {
                        arg_text = argv[argc_idx++];
                        arg_text_size = mptest__slen(arg_text);
                        if ((err = arg->callback(arg, state, 0, arg_text, arg_text_size))) {
                            return err;
                        }
                    } else if (arg->nargs == APARSE_NARGS_0_OR_1_EQ
                            || arg->nargs == 0) {
                        if ((err = arg->callback(arg, state, 0, MPTEST_NULL, 0))) {
                            return err;
                        }
                    } else {
                        mptest_size sub_arg_idx = 0;
                        while (argc_idx < argc) {
                            arg_text = argv[argc_idx++];
                            arg_text_size = mptest__slen(arg_text);
                            if ((err = arg->callback(arg, state, sub_arg_idx++, arg_text, arg_text_size))) {
                                return err;
                            }
                            if ((int)sub_arg_idx == arg->nargs) {
                                break;
                            }
                        }
                        if ((int)sub_arg_idx != arg->nargs) {
                            if ((err = aparse__error_begin_arg(state, arg))) {
                                return err;
                            }
                            if ((err = aparse__state_out_s(state, "expected an argument\n"))) {
                                return err;
                            }
                            return APARSE_ERROR_PARSE;
                        }
                    }
                    break;
                }
            } while (!is_long);
        }
    }
    return err;
}

#undef APARSE__NEXT_POSITIONAL
#endif /* MPTEST_USE_APARSE */

/* mptest */
#if MPTEST_USE_APARSE

const char* mptest__aparse_help = "Runs tests.";

const char* mptest__aparse_version = "0.1.0";

MPTEST_INTERNAL aparse_error mptest__aparse_opt_test_cb(
    void* user, aparse_state* state, int sub_arg_idx, const char* text,
    mptest_size text_size)
{
  mptest__aparse_state* test_state = (mptest__aparse_state*)user;
  mptest__aparse_name* name;
  MPTEST_ASSERT(text);
  MPTEST__UNUSED(state);
  MPTEST__UNUSED(sub_arg_idx);
  name = MPTEST_MALLOC(sizeof(mptest__aparse_name));
  if (name == MPTEST_NULL) {
    return APARSE_ERROR_NOMEM;
  }
  if (test_state->opt_test_name_head == MPTEST_NULL) {
    test_state->opt_test_name_head = name;
    test_state->opt_test_name_tail = name;
  } else {
    test_state->opt_test_name_tail->next = name;
    test_state->opt_test_name_tail = name;
  }
  name->name = text;
  name->name_len = text_size;
  name->next = MPTEST_NULL;
  return APARSE_ERROR_NONE;
}

MPTEST_INTERNAL aparse_error mptest__aparse_opt_suite_cb(
    void* user, aparse_state* state, int sub_arg_idx, const char* text,
    mptest_size text_size)
{
  mptest__aparse_state* test_state = (mptest__aparse_state*)user;
  mptest__aparse_name* name;
  MPTEST_ASSERT(text);
  MPTEST__UNUSED(state);
  MPTEST__UNUSED(sub_arg_idx);
  name = MPTEST_MALLOC(sizeof(mptest__aparse_name));
  if (name == MPTEST_NULL) {
    return APARSE_ERROR_NOMEM;
  }
  if (test_state->opt_suite_name_head == MPTEST_NULL) {
    test_state->opt_suite_name_head = name;
    test_state->opt_suite_name_tail = name;
  } else {
    test_state->opt_suite_name_tail->next = name;
    test_state->opt_suite_name_tail = name;
  }
  name->name = text;
  name->name_len = text_size;
  name->next = MPTEST_NULL;
  return APARSE_ERROR_NONE;
}

MPTEST_INTERNAL int mptest__aparse_init(struct mptest__state* state)
{
  aparse_error err = APARSE_ERROR_NONE;
  mptest__aparse_state* test_state = &state->aparse_state;
  aparse_state* aparse = &test_state->aparse;
  test_state->opt_test_name_head = MPTEST_NULL;
  test_state->opt_test_name_tail = MPTEST_NULL;
  test_state->opt_suite_name_tail = MPTEST_NULL;
  test_state->opt_suite_name_tail = MPTEST_NULL;
  test_state->opt_leak_check = 0;
  if ((err = aparse_init(aparse))) {
    return err;
  }

  if ((err = aparse_add_opt(aparse, 't', "test"))) {
    return err;
  }
  aparse_arg_type_custom(aparse, mptest__aparse_opt_test_cb, test_state, 1);
  aparse_arg_help(aparse, "Run tests that match the substring NAME");
  aparse_arg_metavar(aparse, "NAME");

  if ((err = aparse_add_opt(aparse, 's', "suite"))) {
    return err;
  }
  aparse_arg_type_custom(aparse, mptest__aparse_opt_suite_cb, test_state, 1);
  aparse_arg_help(aparse, "Run suites that match the substring NAME");
  aparse_arg_metavar(aparse, "NAME");

#if MPTEST_USE_LEAKCHECK
  if ((err = aparse_add_opt(aparse, 0, "leak-check"))) {
    return err;
  }
  aparse_arg_type_bool(aparse, &test_state->opt_leak_check);
  aparse_arg_help(aparse, "Instrument tests with memory leak checking");

  if ((err = aparse_add_opt(aparse, 0, "leak-check-oom"))) {
    return err;
  }
  aparse_arg_type_bool(aparse, &test_state->opt_leak_check_oom);
  aparse_arg_help(aparse, "Simulate out-of-memory errors");

  if ((err = aparse_add_opt(aparse, 0, "leak-check-pass"))) {
    return err;
  }
  aparse_arg_type_bool(aparse, &test_state->opt_leak_check_pass);
  aparse_arg_help(
      aparse, "Pass memory allocations without recording, useful with ASAN");
#endif

  if ((err = aparse_add_opt(aparse, 'h', "help"))) {
    return err;
  }
  aparse_arg_type_help(aparse);

  if ((err = aparse_add_opt(aparse, 0, "version"))) {
    return err;
  }
  aparse_arg_type_version(aparse);
  return 0;
}

MPTEST_INTERNAL void mptest__aparse_destroy(struct mptest__state* state)
{
  mptest__aparse_state* test_state = &state->aparse_state;
  mptest__aparse_name* name = test_state->opt_test_name_head;
  while (name) {
    mptest__aparse_name* temp = name;
    MPTEST_FREE(temp);
    name = temp->next;
  }
  name = test_state->opt_suite_name_head;
  while (name) {
    mptest__aparse_name* temp = name;
    MPTEST_FREE(temp);
    name = temp->next;
  }
  aparse_destroy(&test_state->aparse);
}

MPTEST_API int mptest__state_init_argv(
    struct mptest__state* state, int argc, const char* const* argv)
{
  aparse_error stat = 0;
  mptest__state_init(state);
  stat = aparse_parse(&state->aparse_state.aparse, argc, argv);
  if (stat == APARSE_SHOULD_EXIT) {
    return 1;
  } else if (stat != 0) {
    return stat;
  }
#if MPTEST_USE_LEAKCHECK
  if (state->aparse_state.opt_leak_check_oom) {
    state->leakcheck_state.test_leak_checking = MPTEST__LEAKCHECK_MODE_OOM_SET;
  }
  if (state->aparse_state.opt_leak_check) {
    state->leakcheck_state.test_leak_checking = MPTEST__LEAKCHECK_MODE_ON;
  }
  if (state->aparse_state.opt_leak_check_pass) {
    state->leakcheck_state.fall_through = 1;
  }
#endif
  return stat;
}

MPTEST_INTERNAL int mptest__aparse_match_test_name(
    struct mptest__state* state, const char* test_name)
{
  mptest__aparse_name* name = state->aparse_state.opt_test_name_head;
  if (name == MPTEST_NULL) {
    return 1;
  }
  while (name) {
    if (mptest__scmp_n(name->name, name->name_len, test_name)) {
      return 1;
    }
    name = name->next;
  }
  return 0;
}

MPTEST_INTERNAL int mptest__aparse_match_suite_name(
    struct mptest__state* state, const char* suite_name)
{
  mptest__aparse_name* name = state->aparse_state.opt_suite_name_head;
  if (name == MPTEST_NULL) {
    return 1;
  }
  while (name) {
    if (mptest__scmp_n(name->name, name->name_len, suite_name)) {
      return 1;
    }
    name = name->next;
  }
  return 0;
}

#endif

/* mptest */
#if MPTEST_USE_FUZZ

MPTEST_INTERNAL void mptest__fuzz_init(struct mptest__state* state)
{
  mptest__fuzz_state* fuzz_state = &state->fuzz_state;
  fuzz_state->rand_state = 0xDEADBEEF;
  fuzz_state->fuzz_active = 0;
  fuzz_state->fuzz_iterations = 1;
  fuzz_state->fuzz_fail_iteration = 0;
  fuzz_state->fuzz_fail_seed = 0;
}

MPTEST_API mptest_rand mptest__fuzz_rand(struct mptest__state* state)
{
  /* ANSI C LCG (wikipedia) */
  static const mptest_rand a = 1103515245;
  static const mptest_rand m = ((mptest_rand)1) << 31;
  static const mptest_rand c = 12345;
  mptest__fuzz_state* fuzz_state = &state->fuzz_state;
  return (
      fuzz_state->rand_state =
          ((a * fuzz_state->rand_state + c) % m) & 0xFFFFFFFF);
}

MPTEST_API void mptest__fuzz_next_test(struct mptest__state* state, int iterations)
{
  mptest__fuzz_state* fuzz_state = &state->fuzz_state;
  fuzz_state->fuzz_iterations = iterations;
  fuzz_state->fuzz_active = 1;
}

MPTEST_INTERNAL mptest__result
mptest__fuzz_run_test(struct mptest__state* state, mptest__test_func test_func)
{
  int i = 0;
  int iters = 1;
  mptest__result res = MPTEST__RESULT_PASS;
  mptest__fuzz_state* fuzz_state = &state->fuzz_state;
  /* Reset fail variables */
  fuzz_state->fuzz_fail_iteration = 0;
  fuzz_state->fuzz_fail_seed = 0;
  if (fuzz_state->fuzz_active) {
    iters = fuzz_state->fuzz_iterations;
  }
  for (i = 0; i < iters; i++) {
    /* Save the start state */
    mptest_rand start_state = fuzz_state->rand_state;
    int should_finish = 0;
    res = mptest__state_do_run_test(state, test_func);
    /* Note: we don't handle MPTEST__RESULT_SKIPPED because it is handled in
     * the calling function. */
    if (res != MPTEST__RESULT_PASS) {
      should_finish = 1;
    }
#if MPTEST_USE_LEAKCHECK
    if (mptest__leakcheck_has_leaks(state)) {
      should_finish = 1;
    }
#endif
    if (should_finish) {
      /* Save fail context */
      fuzz_state->fuzz_fail_iteration = i;
      fuzz_state->fuzz_fail_seed = start_state;
      fuzz_state->fuzz_failed = 1;
      break;
    }
  }
  fuzz_state->fuzz_active = 0;
  return res;
}

MPTEST_INTERNAL void mptest__fuzz_print(struct mptest__state* state)
{
  mptest__fuzz_state* fuzz_state = &state->fuzz_state;
  if (fuzz_state->fuzz_failed) {
    printf("\n");
    mptest__state_print_indent(state);
    printf(
        "    ...on iteration %i with seed %lX", fuzz_state->fuzz_fail_iteration,
        fuzz_state->fuzz_fail_seed);
  }
  fuzz_state->fuzz_failed = 0;
  /* Reset fuzz iterations, needs to be done after every fuzzed test */
  fuzz_state->fuzz_iterations = 1;
}

#endif

/* mptest */
#if MPTEST_USE_LEAKCHECK

/* Set the guard bytes in `header`. */
MPTEST_INTERNAL void
mptest__leakcheck_header_set_guard(struct mptest__leakcheck_header* header)
{
  /* Currently we choose 0xCC as the guard byte, it's a stripe of ones and
   * zeroes that looks like 11001100b */
  size_t i;
  for (i = 0; i < MPTEST__LEAKCHECK_GUARD_BYTES_COUNT; i++) {
    header->guard_bytes[i] = 0xCC;
  }
}

/* Ensure that `header` has valid guard bytes. */
MPTEST_INTERNAL int
mptest__leakcheck_header_check_guard(struct mptest__leakcheck_header* header)
{
  size_t i;
  for (i = 0; i < MPTEST__LEAKCHECK_GUARD_BYTES_COUNT; i++) {
    if (header->guard_bytes[i] != 0xCC) {
      return 0;
    }
  }
  return 1;
}

/* Determine if `block` contains a `free()`able pointer. */
MPTEST_INTERNAL int
mptest__leakcheck_block_has_freeable(struct mptest__leakcheck_block* block)
{
  /* We can free the pointer if it was not freed or was not reallocated
   * earlier. */
  return !(
      block->flags & (MPTEST__LEAKCHECK_BLOCK_FLAG_FREED |
                      MPTEST__LEAKCHECK_BLOCK_FLAG_REALLOC_OLD));
}

/* Initialize a `struct mptest__leakcheck_block`.
 * If `prev` is NULL, then this function will not attempt to link `block` to
 * any previous element in the malloc linked list. */
MPTEST_INTERNAL void mptest__leakcheck_block_init(
    struct mptest__leakcheck_block* block, size_t size,
    struct mptest__leakcheck_block* prev,
    enum mptest__leakcheck_block_flags flags, const char* file, int line)
{
  block->block_size = size;
  /* Link current block to previous block */
  block->prev = prev;
  /* Link previous block to current block */
  if (prev) {
    block->prev->next = block;
  }
  block->next = NULL;
  /* Keep `realloc()` fields unpopulated for now */
  block->realloc_next = NULL;
  block->realloc_prev = NULL;
  block->flags = flags;
  /* Save source info */
  block->file = file;
  block->line = line;
}

/* Link a block to its respective header. */
MPTEST_INTERNAL void mptest__leakcheck_block_link_header(
    struct mptest__leakcheck_block* block,
    struct mptest__leakcheck_header* header)
{
  block->header = header;
  header->block = block;
}

/* Initialize malloc-checking state. */
MPTEST_INTERNAL void mptest__leakcheck_init(struct mptest__state* state)
{
  mptest__leakcheck_state* leakcheck_state = &state->leakcheck_state;
  leakcheck_state->test_leak_checking = 0;
  leakcheck_state->first_block = NULL;
  leakcheck_state->top_block = NULL;
  leakcheck_state->total_allocations = 0;
  leakcheck_state->total_calls = 0;
  leakcheck_state->oom_failed = 0;
  leakcheck_state->oom_fail_call = -1;
  leakcheck_state->fall_through = 0;
}

/* Destroy malloc-checking state. */
MPTEST_INTERNAL void mptest__leakcheck_destroy(struct mptest__state* state)
{
  /* Walk the malloc list, destroying everything */
  struct mptest__leakcheck_block* current = state->leakcheck_state.first_block;
  while (current) {
    struct mptest__leakcheck_block* prev = current;
    if (mptest__leakcheck_block_has_freeable(current)) {
      MPTEST_FREE(current->header);
    }
    current = current->next;
    MPTEST_FREE(prev);
  }
}

/* Reset (NOT destroy) malloc-checking state. */
/* For now, this is equivalent to a destruction. This may not be the case in
 * the future. */
MPTEST_INTERNAL void mptest__leakcheck_reset(struct mptest__state* state)
{
  /* Preserve `test_leak_checking` */
  mptest__leakcheck_mode test_leak_checking =
      state->leakcheck_state.test_leak_checking;
  /* Preserve fall_through */
  int fall_through = state->leakcheck_state.fall_through;
  mptest__leakcheck_destroy(state);
  mptest__leakcheck_init(state);
  state->leakcheck_state.test_leak_checking = test_leak_checking;
  state->leakcheck_state.fall_through = fall_through;
}

/* Check the block record for leaks, returning 1 if there are any. */
MPTEST_INTERNAL int mptest__leakcheck_has_leaks(struct mptest__state* state)
{
  struct mptest__leakcheck_block* current = state->leakcheck_state.first_block;
  while (current) {
    if (mptest__leakcheck_block_has_freeable(current)) {
      return 1;
    }
    current = current->next;
  }
  return 0;
}

MPTEST_API void* mptest__leakcheck_hook_malloc(
    struct mptest__state* state, const char* file, int line, size_t size)
{
  /* Header + actual memory block */
  char* base_ptr;
  /* Identical to `base_ptr` */
  struct mptest__leakcheck_header* header;
  /* Current block*/
  struct mptest__leakcheck_block* block_info;
  /* Pointer to return to the user */
  char* out_ptr;
  struct mptest__leakcheck_state* leakcheck_state = &state->leakcheck_state;
  if (!leakcheck_state->test_leak_checking) {
    return (char*)MPTEST_MALLOC(size);
  }
  if (leakcheck_state->test_leak_checking == MPTEST__LEAKCHECK_MODE_OOM_ONE) {
    if (leakcheck_state->total_calls == leakcheck_state->oom_fail_call) {
      leakcheck_state->total_calls++;
      mptest_malloc_null_breakpoint();
      return NULL;
    }
  }
  if (leakcheck_state->test_leak_checking == MPTEST__LEAKCHECK_MODE_OOM_SET) {
    if (leakcheck_state->total_calls == leakcheck_state->oom_fail_call) {
      mptest_malloc_null_breakpoint();
      return NULL;
    }
  }
  if (leakcheck_state->fall_through) {
    leakcheck_state->total_calls++;
    return (char*)MPTEST_MALLOC(size);
  }
  /* Allocate the memory the user requested + space for the header */
  base_ptr = (char*)MPTEST_MALLOC(size + MPTEST__LEAKCHECK_HEADER_SIZEOF);
  if (base_ptr == NULL) {
    state->fail_data.memory_block = NULL;
    mptest__longjmp_exec(state, MPTEST__FAIL_REASON_NOMEM, file, line, NULL);
  }
  /* Allocate memory for the block_info structure */
  block_info = (struct mptest__leakcheck_block*)MPTEST_MALLOC(
      sizeof(struct mptest__leakcheck_block));
  if (block_info == NULL) {
    state->fail_data.memory_block = NULL;
    mptest__longjmp_exec(state, MPTEST__FAIL_REASON_NOMEM, file, line, NULL);
  }
  /* Setup the header */
  header = (struct mptest__leakcheck_header*)base_ptr;
  mptest__leakcheck_header_set_guard(header);
  /* Setup the block_info */
  if (leakcheck_state->first_block == NULL) {
    /* If `state->first_block == NULL`, then this is the first allocation.
     * Use NULL as the previous value, and then set the `first_block` and
     * `top_block` to the new block. */
    mptest__leakcheck_block_init(
        block_info, size, NULL, MPTEST__LEAKCHECK_BLOCK_FLAG_INITIAL, file,
        line);
    leakcheck_state->first_block = block_info;
    leakcheck_state->top_block = block_info;
  } else {
    /* If this isn't the first allocation, use `state->top_block` as the
     * previous block. */
    mptest__leakcheck_block_init(
        block_info, size, leakcheck_state->top_block,
        MPTEST__LEAKCHECK_BLOCK_FLAG_INITIAL, file, line);
    leakcheck_state->top_block = block_info;
  }
  /* Link the header and block_info together */
  mptest__leakcheck_block_link_header(block_info, header);
  /* Return the base pointer offset by the header amount */
  out_ptr = base_ptr + MPTEST__LEAKCHECK_HEADER_SIZEOF;
  /* Increment the total number of allocations */
  leakcheck_state->total_allocations++;
  /* Increment the total number of calls */
  leakcheck_state->total_calls++;
  return out_ptr;
}

MPTEST_API void mptest__leakcheck_hook_free(
    struct mptest__state* state, const char* file, int line, void* ptr)
{
  struct mptest__leakcheck_header* header;
  struct mptest__leakcheck_block* block_info;
  struct mptest__leakcheck_state* leakcheck_state = &state->leakcheck_state;
  if (!leakcheck_state->test_leak_checking || leakcheck_state->fall_through) {
    MPTEST_FREE(ptr);
    return;
  }
  if (ptr == NULL) {
    state->fail_data.memory_block = NULL;
    mptest__longjmp_exec(
        state, MPTEST__FAIL_REASON_FREE_OF_NULL, file, line, NULL);
  }
  /* Retrieve header by subtracting header size from pointer */
  header =
      (struct
       mptest__leakcheck_header*)((char*)ptr - MPTEST__LEAKCHECK_HEADER_SIZEOF);
  /* TODO: check for SIGSEGV here */
  if (!mptest__leakcheck_header_check_guard(header)) {
    state->fail_data.memory_block = ptr;
    mptest__longjmp_exec(
        state, MPTEST__FAIL_REASON_FREE_OF_INVALID, file, line, NULL);
  }
  block_info = header->block;
  /* Ensure that the pointer has not been freed or reallocated already */
  if (block_info->flags & MPTEST__LEAKCHECK_BLOCK_FLAG_FREED) {
    state->fail_data.memory_block = ptr;
    mptest__longjmp_exec(
        state, MPTEST__FAIL_REASON_REALLOC_OF_FREED, file, line, NULL);
  }
  if (block_info->flags & MPTEST__LEAKCHECK_BLOCK_FLAG_REALLOC_OLD) {
    state->fail_data.memory_block = ptr;
    mptest__longjmp_exec(
        state, MPTEST__FAIL_REASON_FREE_OF_REALLOCED, file, line, NULL);
  }
  /* We can finally `free()` the pointer */
  MPTEST_FREE(header);
  block_info->flags |= MPTEST__LEAKCHECK_BLOCK_FLAG_FREED;
  /* Decrement the total number of allocations */
  leakcheck_state->total_allocations--;
}

MPTEST_API void* mptest__leakcheck_hook_realloc(
    struct mptest__state* state, const char* file, int line, void* old_ptr,
    size_t new_size)
{
  /* New header + memory */
  char* base_ptr;
  struct mptest__leakcheck_header* old_header;
  struct mptest__leakcheck_header* new_header;
  struct mptest__leakcheck_block* old_block_info;
  struct mptest__leakcheck_block* new_block_info;
  /* Pointer to return to the user */
  char* out_ptr;
  struct mptest__leakcheck_state* leakcheck_state = &state->leakcheck_state;
  if (!leakcheck_state->test_leak_checking) {
    return (void*)MPTEST_REALLOC(old_ptr, new_size);
  }
  if (leakcheck_state->test_leak_checking == MPTEST__LEAKCHECK_MODE_OOM_ONE) {
    if (leakcheck_state->total_calls == leakcheck_state->oom_fail_call) {
      leakcheck_state->total_calls++;
      mptest_malloc_null_breakpoint();
      return NULL;
    }
  }
  if (leakcheck_state->test_leak_checking == MPTEST__LEAKCHECK_MODE_OOM_SET) {
    if (leakcheck_state->total_calls == leakcheck_state->oom_fail_call) {
      mptest_malloc_null_breakpoint();
      return NULL;
    }
  }
  if (leakcheck_state->fall_through) {
    leakcheck_state->total_calls++;
    return (char*)MPTEST_REALLOC(old_ptr, new_size);
  }
  old_header =
      (struct
       mptest__leakcheck_header*)((char*)old_ptr - MPTEST__LEAKCHECK_HEADER_SIZEOF);
  old_block_info = old_header->block;
  if (old_ptr == NULL) {
    state->fail_data.memory_block = NULL;
    mptest__longjmp_exec(
        state, MPTEST__FAIL_REASON_REALLOC_OF_NULL, file, line, NULL);
  }
  if (!mptest__leakcheck_header_check_guard(old_header)) {
    state->fail_data.memory_block = old_ptr;
    mptest__longjmp_exec(
        state, MPTEST__FAIL_REASON_REALLOC_OF_INVALID, file, line, NULL);
  }
  if (old_block_info->flags & MPTEST__LEAKCHECK_BLOCK_FLAG_FREED) {
    state->fail_data.memory_block = old_ptr;
    mptest__longjmp_exec(
        state, MPTEST__FAIL_REASON_REALLOC_OF_FREED, file, line, NULL);
  }
  if (old_block_info->flags & MPTEST__LEAKCHECK_BLOCK_FLAG_REALLOC_OLD) {
    state->fail_data.memory_block = old_ptr;
    mptest__longjmp_exec(
        state, MPTEST__FAIL_REASON_REALLOC_OF_REALLOCED, file, line, NULL);
  }
  /* Allocate the memory the user requested + space for the header */
  base_ptr =
      (char*)MPTEST_REALLOC(old_header, new_size + MPTEST__LEAKCHECK_HEADER_SIZEOF);
  if (base_ptr == NULL) {
    state->fail_data.memory_block = old_ptr;
    mptest__longjmp_exec(state, MPTEST__FAIL_REASON_NOMEM, file, line, NULL);
  }
  /* Allocate memory for the new block_info structure */
  new_block_info = (struct mptest__leakcheck_block*)MPTEST_MALLOC(
      sizeof(struct mptest__leakcheck_block));
  if (new_block_info == NULL) {
    state->fail_data.memory_block = old_ptr;
    mptest__longjmp_exec(state, MPTEST__FAIL_REASON_NOMEM, file, line, NULL);
  }
  /* Setup the header */
  new_header = (struct mptest__leakcheck_header*)base_ptr;
  /* Set the guard again (double bag it per se) */
  mptest__leakcheck_header_set_guard(new_header);
  /* Setup the block_info */
  if (leakcheck_state->first_block == NULL) {
    mptest__leakcheck_block_init(
        new_block_info, new_size, NULL,
        MPTEST__LEAKCHECK_BLOCK_FLAG_REALLOC_NEW, file, line);
    leakcheck_state->first_block = new_block_info;
    leakcheck_state->top_block = new_block_info;
  } else {
    mptest__leakcheck_block_init(
        new_block_info, new_size, leakcheck_state->top_block,
        MPTEST__LEAKCHECK_BLOCK_FLAG_REALLOC_NEW, file, line);
    leakcheck_state->top_block = new_block_info;
  }
  /* Mark `old_block_info` as reallocation target */
  old_block_info->flags |= MPTEST__LEAKCHECK_BLOCK_FLAG_REALLOC_OLD;
  /* Link the block with its respective header */
  mptest__leakcheck_block_link_header(new_block_info, new_header);
  /* Finally, indicate the new allocation in the realloc chain */
  old_block_info->realloc_next = new_block_info;
  new_block_info->realloc_prev = old_block_info;
  out_ptr = base_ptr + MPTEST__LEAKCHECK_HEADER_SIZEOF;
  /* Increment the total number of calls */
  leakcheck_state->total_calls++;
  return out_ptr;
}

MPTEST_API void mptest__leakcheck_set(struct mptest__state* state, int on)
{
  state->leakcheck_state.test_leak_checking = on;
}

MPTEST_INTERNAL mptest__result mptest__leakcheck_oom_run_test(
    struct mptest__state* state, mptest__test_func test_func)
{
  int max_iter = 0;
  int i;
  mptest__result res = MPTEST__RESULT_PASS;
  struct mptest__leakcheck_state* leakcheck_state = &state->leakcheck_state;
  leakcheck_state->oom_fail_call = -1;
  leakcheck_state->oom_failed = 0;
  mptest__leakcheck_reset(state);
  res = mptest__state_do_run_test(state, test_func);
  max_iter = leakcheck_state->total_calls;
  if (res) {
    /* Initial test failed. */
    return res;
  }
  for (i = 0; i < max_iter; i++) {
    int should_finish = 0;
    mptest__leakcheck_reset(state);
    leakcheck_state->oom_fail_call = i;
    res = mptest__state_do_run_test(state, test_func);
    if (res != MPTEST__RESULT_PASS) {
      should_finish = 1;
    }
    if (mptest__leakcheck_has_leaks(state)) {
      should_finish = 1;
    }
    if (should_finish) {
      /* Save fail context */
      leakcheck_state->oom_failed = 1;
      break;
    }
  }
  return res;
}

MPTEST_API void mptest_malloc_null_breakpoint(void) { return; }

#endif

/* mptest */
#if MPTEST_USE_LONGJMP
/* Initialize longjmp state. */
MPTEST_INTERNAL void mptest__longjmp_init(struct mptest__state* state)
{
  state->longjmp_state.checking = MPTEST__FAIL_REASON_NONE;
  state->longjmp_state.reason = MPTEST__FAIL_REASON_NONE;
}

/* Destroy longjmp state. */
MPTEST_INTERNAL void mptest__longjmp_destroy(struct mptest__state* state)
{
  (void)(state);
}

/* Jumps back to either the out-of-test context or the within-assertion context
 * depending on if we wanted `reason` to happen or not. In other words, this
 * will fail the test if we weren't explicitly checking for `reason` to happen,
 * meaning `reason` was unexpected and thus an error. */
MPTEST_INTERNAL void mptest__longjmp_exec(
    struct mptest__state* state, mptest__fail_reason reason, const char* file,
    int line, const char* msg)
{
  state->longjmp_state.reason = reason;
  if (state->longjmp_state.checking == reason) {
    MPTEST_LONGJMP(state->longjmp_state.assert_context, 1);
  } else {
    state->fail_file = file;
    state->fail_line = line;
    state->fail_msg = msg;
    state->fail_reason = reason;
    MPTEST_LONGJMP(state->longjmp_state.test_context, 1);
  }
}

#else

/* TODO: write `mptest__longjmp_exec` for when longjmp isn't on
 */

#endif

/* mptest */
/* Initialize a test runner state. */
MPTEST_API void mptest__state_init(struct mptest__state* state)
{
  state->assertions = 0;
  state->total = 0;
  state->passes = 0;
  state->fails = 0;
  state->errors = 0;
  state->suite_passes = 0;
  state->suite_fails = 0;
  state->suite_failed = 0;
  state->suite_test_setup_cb = NULL;
  state->suite_test_teardown_cb = NULL;
  state->current_test = NULL;
  state->current_suite = NULL;
  state->fail_reason = (enum mptest__fail_reason)0;
  state->fail_msg = NULL;
  /* we do not initialize state->fail_data */
  state->fail_file = NULL;
  state->fail_line = 0;
  state->indent_lvl = 0;
#if MPTEST_USE_LONGJMP
  mptest__longjmp_init(state);
#endif
#if MPTEST_USE_LEAKCHECK
  mptest__leakcheck_init(state);
#endif
#if MPTEST_USE_TIME
  mptest__time_init(state);
#endif
#if MPTEST_USE_APARSE
  mptest__aparse_init(state);
#endif
#if MPTEST_USE_FUZZ
  mptest__fuzz_init(state);
#endif
}

/* Destroy a test runner state. */
MPTEST_API void mptest__state_destroy(struct mptest__state* state)
{
  (void)(state);
#if MPTEST_USE_APARSE
  mptest__aparse_destroy(state);
#endif
#if MPTEST_USE_TIME
  mptest__time_destroy(state);
#endif
#if MPTEST_USE_LEAKCHECK
  mptest__leakcheck_destroy(state);
#endif
#if MPTEST_USE_LONGJMP
  mptest__longjmp_destroy(state);
#endif
}

/* Actually define (create storage space for) the global state */
struct mptest__state mptest__state_g;

/* Print report at the end of testing. */
MPTEST_API void mptest__state_report(struct mptest__state* state)
{
  if (state->suite_fails + state->suite_passes) {
    printf(
        MPTEST__COLOR_SUITE_NAME
        "%i" MPTEST__COLOR_RESET " suites: " MPTEST__COLOR_PASS
        "%i" MPTEST__COLOR_RESET " passed, " MPTEST__COLOR_FAIL
        "%i" MPTEST__COLOR_RESET " failed\n",
        state->suite_fails + state->suite_passes, state->suite_passes,
        state->suite_fails);
  }
  if (state->errors) {
    printf(
        MPTEST__COLOR_TEST_NAME
        "%i" MPTEST__COLOR_RESET " tests (" MPTEST__COLOR_EMPHASIS
        "%i" MPTEST__COLOR_RESET " assertions): " MPTEST__COLOR_PASS
        "%i" MPTEST__COLOR_RESET " passed, " MPTEST__COLOR_FAIL
        "%i" MPTEST__COLOR_RESET " failed, " MPTEST__COLOR_FAIL
        "%i" MPTEST__COLOR_RESET " errors",
        state->total, state->assertions, state->passes, state->fails,
        state->errors);
  } else {
    printf(
        MPTEST__COLOR_TEST_NAME
        "%i" MPTEST__COLOR_RESET " tests (" MPTEST__COLOR_EMPHASIS
        "%i" MPTEST__COLOR_RESET " assertions): " MPTEST__COLOR_PASS
        "%i" MPTEST__COLOR_RESET " passed, " MPTEST__COLOR_FAIL
        "%i" MPTEST__COLOR_RESET " failed",
        state->fails + state->passes, state->assertions, state->passes,
        state->fails);
  }
#if MPTEST_USE_TIME
  {
    clock_t program_end_time = clock();
    double elapsed_time =
        ((double)(program_end_time - state->time_state.program_start_time)) /
        CLOCKS_PER_SEC;
    printf(" in %f seconds", elapsed_time);
  }
#endif
  printf("\n");
}

/* Helper to indent to the current level if nested suites/tests are used. */
MPTEST_INTERNAL void mptest__state_print_indent(struct mptest__state* state)
{
  int i;
  for (i = 0; i < state->indent_lvl; i++) {
    printf("  ");
  }
}

/* Print a formatted source location. */
MPTEST_INTERNAL void mptest__print_source_location(const char* file, int line)
{
  printf(
      MPTEST__COLOR_EMPHASIS "%s" MPTEST__COLOR_RESET ":" MPTEST__COLOR_EMPHASIS
                             "%i" MPTEST__COLOR_RESET,
      file, line);
}

/* fuck string.h */
MPTEST_INTERNAL int mptest__streq(const char* a, const char* b)
{
  while (1) {
    if (*a == '\0') {
      if (*b == '\0') {
        return 1;
      } else {
        return 0;
      }
    } else if (*b == '\0') {
      return 0;
    }
    if (*a != *b) {
      return 0;
    }
    a++;
    b++;
  }
}

/* Ran when setting up for a test before it is run. */
MPTEST_INTERNAL mptest__result mptest__state_before_test(
    struct mptest__state* state, mptest__test_func test_func,
    const char* test_name)
{
  state->current_test = test_name;
#if MPTEST_USE_LEAKCHECK
  if (state->leakcheck_state.test_leak_checking) {
    mptest__leakcheck_reset(state);
  }
#endif
  /* indent if we are running a suite */
  mptest__state_print_indent(state);
  printf(
      "test " MPTEST__COLOR_TEST_NAME "%s" MPTEST__COLOR_RESET "... ",
      state->current_test);
  fflush(stdout);
#if MPTEST_USE_APARSE
  if (!mptest__aparse_match_test_name(state, test_name)) {
    return MPTEST__RESULT_SKIPPED;
  }
#endif
#if MPTEST_USE_LEAKCHECK
  if (state->leakcheck_state.test_leak_checking == MPTEST__LEAKCHECK_MODE_OFF ||
      state->leakcheck_state.test_leak_checking == MPTEST__LEAKCHECK_MODE_ON) {
#if MPTEST_USE_FUZZ
    return mptest__fuzz_run_test(state, test_func);
#else
    return mptest__state_do_run_test(state, test_func);
#endif
  } else {
    return mptest__leakcheck_oom_run_test(state, test_func);
  }
#else
#if MPTEST_USE_FUZZ
  return mptest__fuzz_run_test(state, test_func);
#else
  return mptest__state_do_run_test(state, test_func);
#endif
#endif
}

MPTEST_INTERNAL mptest__result mptest__state_do_run_test(
    struct mptest__state* state, mptest__test_func test_func)
{
  mptest__result res;
#if MPTEST_USE_LONGJMP
  if (MPTEST_SETJMP(state->longjmp_state.test_context) == 0) {
    res = test_func();
  } else {
    res = MPTEST__RESULT_ERROR;
  }
#else
  res = test_func();
#endif
  return res;
}

/* Ran when a test is over. */
MPTEST_INTERNAL void
mptest__state_after_test(struct mptest__state* state, mptest__result res)
{
#if MPTEST_USE_LEAKCHECK
  int has_leaks;
  if (state->leakcheck_state.test_leak_checking == 1) {
    has_leaks = mptest__leakcheck_has_leaks(state);
    if (has_leaks) {
      if (res == MPTEST__RESULT_PASS) {
        state->fail_reason = MPTEST__FAIL_REASON_LEAKED;
      }
      res = MPTEST__RESULT_FAIL;
    }
  }
#endif
  if (res == MPTEST__RESULT_PASS) {
    /* Test passed -> print pass message */
    state->passes++;
    state->total++;
    printf(MPTEST__COLOR_PASS "passed" MPTEST__COLOR_RESET);
  } else if (res == MPTEST__RESULT_FAIL) {
    /* Test failed -> fail the current suite, print diagnostics */
    state->fails++;
    state->total++;
    if (state->current_suite) {
      state->suite_failed = 1;
    }
    printf(MPTEST__COLOR_FAIL "failed" MPTEST__COLOR_RESET "\n");
    if (state->fail_reason == MPTEST__FAIL_REASON_ASSERT_FAILURE) {
      /* Assertion failure -> show expression, message, source */
      mptest__state_print_indent(state);
      printf(
          "  " MPTEST__COLOR_FAIL "assertion failure" MPTEST__COLOR_RESET
          ": " MPTEST__COLOR_EMPHASIS "%s" MPTEST__COLOR_RESET "\n",
          state->fail_msg);
      /* If the message and expression are the same, don't print the
       * expression */
      if (!mptest__streq(
              state->fail_msg, (const char*)state->fail_data.string_data)) {
        mptest__state_print_indent(state);
        printf(
            "    expression: " MPTEST__COLOR_EMPHASIS "%s" MPTEST__COLOR_RESET
            "\n",
            (const char*)state->fail_data.string_data);
      }
      mptest__state_print_indent(state);
      /* Print source location */
      printf("    ...at ");
      mptest__print_source_location(state->fail_file, state->fail_line);
#if MPTEST_USE_FUZZ
      /* Print fuzz information, if any */
      mptest__fuzz_print(state);
#endif
    }
#if MPTEST_USE_SYM
    else if (state->fail_reason == MPTEST__FAIL_REASON_SYM_INEQUALITY) {
      /* Sym inequality -> show both syms, message, source */
      mptest__state_print_indent(state);
      printf(
          "  " MPTEST__COLOR_FAIL
          "assertion failure: s-expression inequality" MPTEST__COLOR_RESET
          ": " MPTEST__COLOR_EMPHASIS "%s" MPTEST__COLOR_RESET "\n",
          state->fail_msg);
      mptest__state_print_indent(state);
      printf("Actual:");
      mptest__sym_dump(
          state->fail_data.sym_fail_data.sym_actual, 0, state->indent_lvl);
      printf("\n");
      mptest__state_print_indent(state);
      printf("Expected:");
      mptest__sym_dump(
          state->fail_data.sym_fail_data.sym_expected, 0, state->indent_lvl);
      mptest__sym_check_destroy();
    }
#endif
#if MPTEST_USE_LEAKCHECK
    else if (state->fail_reason == MPTEST__FAIL_REASON_LEAKED || has_leaks) {
      struct mptest__leakcheck_block* current =
          state->leakcheck_state.first_block;
      mptest__state_print_indent(state);
      printf("  " MPTEST__COLOR_FAIL
             "memory leak(s) detected" MPTEST__COLOR_RESET ":\n");
      while (current) {
        if (mptest__leakcheck_block_has_freeable(current)) {
          mptest__state_print_indent(state);
          printf(
              "    " MPTEST__COLOR_FAIL "leak" MPTEST__COLOR_RESET
              " of " MPTEST__COLOR_EMPHASIS "%lu" MPTEST__COLOR_RESET
              " bytes at " MPTEST__COLOR_EMPHASIS "%p" MPTEST__COLOR_RESET
              ":\n",
              (long unsigned int)current->block_size, (void*)current->header);
          mptest__state_print_indent(state);
          if (current->flags & MPTEST__LEAKCHECK_BLOCK_FLAG_INITIAL) {
            printf("      allocated with " MPTEST__COLOR_EMPHASIS
                   "malloc()" MPTEST__COLOR_RESET "\n");
          } else if (
              current->flags & MPTEST__LEAKCHECK_BLOCK_FLAG_REALLOC_NEW) {
            printf("      reallocated with " MPTEST__COLOR_EMPHASIS
                   "realloc()" MPTEST__COLOR_RESET ":\n");
            printf(
                "        ...from " MPTEST__COLOR_EMPHASIS
                "%p" MPTEST__COLOR_RESET "\n",
                (void*)current->realloc_prev);
          }
          mptest__state_print_indent(state);
          printf("      ...at ");
          mptest__print_source_location(current->file, current->line);
        }
        current = current->next;
      }
    }
#endif
    printf("\n");
  } else if (res == MPTEST__RESULT_ERROR) {
    state->errors++;
    state->total++;
    if (state->current_suite) {
      state->suite_failed = 1;
    }
    printf(MPTEST__COLOR_FAIL "error!" MPTEST__COLOR_RESET "\n");
    if (0) {

    }
#if MPTEST_USE_DYN_ALLOC
    else if (state->fail_reason == MPTEST__FAIL_REASON_NOMEM) {
      mptest__state_print_indent(state);
      printf(
          "  " MPTEST__COLOR_FAIL "out of memory: " MPTEST__COLOR_RESET
          ": " MPTEST__COLOR_EMPHASIS "%s" MPTEST__COLOR_RESET,
          state->fail_msg);
    }
#endif
#if MPTEST_USE_SYM
    else if (state->fail_reason == MPTEST__FAIL_REASON_SYM_SYNTAX) {
      /* Sym syntax error -> show message, source, error info */
      mptest__state_print_indent(state);
      printf(
          "  " MPTEST__COLOR_FAIL
          "s-expression syntax error" MPTEST__COLOR_RESET
          ": " MPTEST__COLOR_EMPHASIS "%s" MPTEST__COLOR_RESET
          ":" MPTEST__COLOR_EMPHASIS "%s at position %i\n",
          state->fail_msg, state->fail_data.sym_syntax_error_data.err_msg,
          (int)state->fail_data.sym_syntax_error_data.err_pos);
    }
#endif
#if MPTEST_USE_LONGJMP
    if (state->fail_reason == MPTEST__FAIL_REASON_UNCAUGHT_PROGRAM_ASSERT) {
      mptest__state_print_indent(state);
      printf(
          "  " MPTEST__COLOR_FAIL
          "uncaught assertion failure" MPTEST__COLOR_RESET
          ": " MPTEST__COLOR_EMPHASIS "%s" MPTEST__COLOR_RESET "\n",
          state->fail_msg);
      if (!mptest__streq(
              state->fail_msg, (const char*)state->fail_data.string_data)) {
        mptest__state_print_indent(state);
        printf(
            "    expression: " MPTEST__COLOR_EMPHASIS "%s" MPTEST__COLOR_RESET
            "\n",
            (const char*)state->fail_data.string_data);
      }
      mptest__state_print_indent(state);
      printf("    ...at ");
      mptest__print_source_location(state->fail_file, state->fail_line);
    }
#if MPTEST_USE_LEAKCHECK
    else if (state->fail_reason == MPTEST__FAIL_REASON_NOMEM) {
      mptest__state_print_indent(state);
      printf("  " MPTEST__COLOR_FAIL "internal error: malloc() returned "
             "a null pointer" MPTEST__COLOR_RESET "\n");
      mptest__state_print_indent(state);
      printf("    ...at ");
      mptest__print_source_location(state->fail_file, state->fail_line);
    } else if (state->fail_reason == MPTEST__FAIL_REASON_REALLOC_OF_NULL) {
      mptest__state_print_indent(state);
      printf("  " MPTEST__COLOR_FAIL "attempt to call realloc() on a NULL "
             "pointer" MPTEST__COLOR_RESET "\n");
      mptest__state_print_indent(state);
      printf("    ...at ");
      mptest__print_source_location(state->fail_file, state->fail_line);
    } else if (state->fail_reason == MPTEST__FAIL_REASON_FREE_OF_NULL) {
      mptest__state_print_indent(state);
      printf("  " MPTEST__COLOR_FAIL "attempt to call free() on a NULL "
             "pointer" MPTEST__COLOR_RESET "\n");
      mptest__state_print_indent(state);
      printf("    ...at ");
      mptest__print_source_location(state->fail_file, state->fail_line);
    } else if (state->fail_reason == MPTEST__FAIL_REASON_REALLOC_OF_INVALID) {
      mptest__state_print_indent(state);
      printf("  " MPTEST__COLOR_FAIL "attempt to call realloc() on an "
             "invalid pointer (pointer was not "
             "returned by malloc() or realloc())" MPTEST__COLOR_RESET ":\n");
      mptest__state_print_indent(state);
      printf("    pointer: %p\n", state->fail_data.memory_block);
      mptest__state_print_indent(state);
      printf("    ...at ");
      mptest__print_source_location(state->fail_file, state->fail_line);
    } else if (state->fail_reason == MPTEST__FAIL_REASON_REALLOC_OF_FREED) {
      mptest__state_print_indent(state);
      printf("  " MPTEST__COLOR_FAIL
             "attempt to call realloc() on a pointer that was already "
             "freed" MPTEST__COLOR_RESET ":\n");
      mptest__state_print_indent(state);
      printf("    pointer: %p\n", state->fail_data.memory_block);
      mptest__state_print_indent(state);
      printf("    ...at ");
      mptest__print_source_location(state->fail_file, state->fail_line);
    } else if (state->fail_reason == MPTEST__FAIL_REASON_REALLOC_OF_REALLOCED) {
      mptest__state_print_indent(state);
      printf("  " MPTEST__COLOR_FAIL
             "attempt to call realloc() on a pointer that was already "
             "reallocated" MPTEST__COLOR_RESET ":\n");
      mptest__state_print_indent(state);
      printf("    pointer: %p\n", state->fail_data.memory_block);
      mptest__state_print_indent(state);
      printf("    ...at ");
      mptest__print_source_location(state->fail_file, state->fail_line);
    } else if (state->fail_reason == MPTEST__FAIL_REASON_FREE_OF_INVALID) {
      mptest__state_print_indent(state);
      printf("  " MPTEST__COLOR_FAIL "attempt to call free() on an "
             "invalid pointer (pointer was not "
             "returned by malloc() or free())" MPTEST__COLOR_RESET ":\n");
      mptest__state_print_indent(state);
      printf("    pointer: %p\n", state->fail_data.memory_block);
      mptest__state_print_indent(state);
      printf("    ...at ");
      mptest__print_source_location(state->fail_file, state->fail_line);
    } else if (state->fail_reason == MPTEST__FAIL_REASON_FREE_OF_FREED) {
      mptest__state_print_indent(state);
      printf("  " MPTEST__COLOR_FAIL
             "attempt to call free() on a pointer that was already "
             "freed" MPTEST__COLOR_RESET ":\n");
      mptest__state_print_indent(state);
      printf("    pointer: %p\n", state->fail_data.memory_block);
      mptest__state_print_indent(state);
      printf("    ...at ");
      mptest__print_source_location(state->fail_file, state->fail_line);
    } else if (state->fail_reason == MPTEST__FAIL_REASON_FREE_OF_FREED) {
      mptest__state_print_indent(state);
      printf("  " MPTEST__COLOR_FAIL
             "attempt to call free() on a pointer that was already "
             "freed" MPTEST__COLOR_RESET ":\n");
      mptest__state_print_indent(state);
      printf("    pointer: %p\n", state->fail_data.memory_block);
      mptest__state_print_indent(state);
      printf("    ...at ");
      mptest__print_source_location(state->fail_file, state->fail_line);
    }
#endif
#endif
    else {
      mptest__state_print_indent(state);
      printf("  " MPTEST__COLOR_FAIL "unknown error" MPTEST__COLOR_RESET
             ": a bug has occurred");
    }
#if MPTEST_USE_FUZZ
    /* Print fuzz information, if any */
    mptest__fuzz_print(state);
#endif
    printf("\n");
  } else if (res == MPTEST__RESULT_SKIPPED) {
    printf("skipped");
  }
#if MPTEST_USE_LEAKCHECK
  /* Reset leak-checking state (IMPORTANT!) */
  mptest__leakcheck_reset(state);
#endif
  printf("\n");
}

MPTEST_API void mptest__run_test(
    struct mptest__state* state, mptest__test_func test_func,
    const char* test_name)
{
  mptest__result res = mptest__state_before_test(state, test_func, test_name);
  mptest__state_after_test(state, res);
}

/* Ran before a suite is executed. */
MPTEST_INTERNAL void mptest__state_before_suite(
    struct mptest__state* state, mptest__suite_func suite_func,
    const char* suite_name)
{
  state->current_suite = suite_name;
  state->suite_failed = 0;
  state->suite_test_setup_cb = NULL;
  state->suite_test_teardown_cb = NULL;
  mptest__state_print_indent(state);
  printf(
      "suite " MPTEST__COLOR_SUITE_NAME "%s" MPTEST__COLOR_RESET ":\n",
      state->current_suite);
  state->indent_lvl++;
#if MPTEST_USE_APARSE
  if (mptest__aparse_match_suite_name(state, suite_name)) {
    suite_func();
  }
#endif
}

/* Ran after a suite is executed. */
MPTEST_INTERNAL void mptest__state_after_suite(struct mptest__state* state)
{
  if (!state->suite_failed) {
    state->suite_passes++;
  } else {
    state->suite_fails++;
  }
  state->current_suite = NULL;
  state->suite_failed = 0;
  state->indent_lvl--;
}

MPTEST_API void mptest__run_suite(
    struct mptest__state* state, mptest__suite_func suite_func,
    const char* suite_name)
{
  mptest__state_before_suite(state, suite_func, suite_name);
  mptest__state_after_suite(state);
}

/* Fills state with information on pass. */
MPTEST_API void mptest__assert_pass(
    struct mptest__state* state, const char* msg, const char* assert_expr,
    const char* file, int line)
{
  MPTEST__UNUSED(msg);
  MPTEST__UNUSED(assert_expr);
  MPTEST__UNUSED(file);
  MPTEST__UNUSED(line);
  state->assertions++;
}

/* Fills state with information on failure. */
MPTEST_API void mptest__assert_fail(
    struct mptest__state* state, const char* msg, const char* assert_expr,
    const char* file, int line)
{
  state->fail_reason = MPTEST__FAIL_REASON_ASSERT_FAILURE;
  state->fail_msg = msg;
  state->fail_data.string_data = assert_expr;
  state->fail_file = file;
  state->fail_line = line;
  mptest_assert_fail_breakpoint();
}

/* Dummy function to break on for test assert failures */
MPTEST_API void mptest_assert_fail_breakpoint() { return; }

/* Dummy function to break on for program assert failures */
MPTEST_API void mptest_uncaught_assert_fail_breakpoint() { return; }

MPTEST_API MPTEST_JMP_BUF* mptest__catch_assert_begin(struct mptest__state* state)
{
  state->longjmp_state.checking = MPTEST__FAIL_REASON_UNCAUGHT_PROGRAM_ASSERT;
  return &state->longjmp_state.assert_context;
}

MPTEST_API void mptest__catch_assert_end(struct mptest__state* state)
{
  state->longjmp_state.checking = 0;
}

MPTEST_API void mptest__catch_assert_fail(
    struct mptest__state* state, const char* msg, const char* assert_expr,
    const char* file, int line)
{
  state->fail_data.string_data = assert_expr;
  mptest__longjmp_exec(
      state, MPTEST__FAIL_REASON_UNCAUGHT_PROGRAM_ASSERT, file, line, msg);
}

/* mptest */
#if MPTEST_USE_SYM

typedef enum mptest__sym_type {
  MPTEST__SYM_TYPE_EXPR,
  MPTEST__SYM_TYPE_ATOM_STRING,
  MPTEST__SYM_TYPE_ATOM_NUMBER
} mptest__sym_type;

typedef union mptest__sym_data {
  mptest__str str;
  mptest_int32 num;
} mptest__sym_data;

typedef struct mptest__sym_tree {
  mptest__sym_type type;
  mptest_int32 first_child_ref;
  mptest_int32 next_sibling_ref;
  mptest__sym_data data;
} mptest__sym_tree;

void mptest__sym_tree_init(mptest__sym_tree* tree, mptest__sym_type type)
{
  tree->type = type;
  tree->first_child_ref = MPTEST__SYM_NONE;
  tree->next_sibling_ref = MPTEST__SYM_NONE;
}

void mptest__sym_tree_destroy(mptest__sym_tree* tree)
{
  if (tree->type == MPTEST__SYM_TYPE_ATOM_STRING) {
    mptest__str_destroy(&tree->data.str);
  }
}

MPTEST__VEC_DECL(mptest__sym_tree);
MPTEST__VEC_IMPL_FUNC(mptest__sym_tree, init)
MPTEST__VEC_IMPL_FUNC(mptest__sym_tree, destroy)
MPTEST__VEC_IMPL_FUNC(mptest__sym_tree, push)
MPTEST__VEC_IMPL_FUNC(mptest__sym_tree, size)
MPTEST__VEC_IMPL_FUNC(mptest__sym_tree, getref)
MPTEST__VEC_IMPL_FUNC(mptest__sym_tree, getcref)

struct mptest_sym {
  mptest__sym_tree_vec tree_storage;
};

void mptest__sym_init(mptest_sym* sym)
{
  mptest__sym_tree_vec_init(&sym->tree_storage);
}

void mptest__sym_destroy(mptest_sym* sym)
{
  mptest_size i;
  for (i = 0; i < mptest__sym_tree_vec_size(&sym->tree_storage); i++) {
    mptest__sym_tree_destroy(
        mptest__sym_tree_vec_getref(&sym->tree_storage, i));
  }
  mptest__sym_tree_vec_destroy(&sym->tree_storage);
}

MPTEST_INTERNAL mptest__sym_tree* mptest__sym_get(mptest_sym* sym, mptest_int32 ref)
{
  MPTEST_ASSERT(ref != MPTEST__SYM_NONE);
  return mptest__sym_tree_vec_getref(&sym->tree_storage, (mptest_size)ref);
}

MPTEST_INTERNAL const mptest__sym_tree*
mptest__sym_getcref(const mptest_sym* sym, mptest_int32 ref)
{
  MPTEST_ASSERT(ref != MPTEST__SYM_NONE);
  return mptest__sym_tree_vec_getcref(&sym->tree_storage, (mptest_size)ref);
}

MPTEST_INTERNAL int mptest__sym_new(
    mptest_sym* sym, mptest_int32 parent_ref, mptest_int32 prev_sibling_ref,
    mptest__sym_tree new_tree, mptest_int32* new_ref)
{
  int err = 0;
  mptest_int32 next_ref = (mptest_int32)mptest__sym_tree_vec_size(&sym->tree_storage);
  if ((err = mptest__sym_tree_vec_push(&sym->tree_storage, new_tree))) {
    return err;
  }
  *new_ref = next_ref;
  if (parent_ref != MPTEST__SYM_NONE) {
    if (prev_sibling_ref == MPTEST__SYM_NONE) {
      mptest__sym_tree* parent = mptest__sym_get(sym, parent_ref);
      parent->first_child_ref = *new_ref;
    } else {
      mptest__sym_tree* sibling = mptest__sym_get(sym, prev_sibling_ref);
      sibling->next_sibling_ref = *new_ref;
    }
  }
  return err;
}

MPTEST_INTERNAL int mptest__sym_isblank(mptest_char ch)
{
  return (ch == '\n') || (ch == '\t') || (ch == '\r') || (ch == ' ');
}

MPTEST__VEC_DECL(mptest_sym_build);
MPTEST__VEC_IMPL_FUNC(mptest_sym_build, init)
MPTEST__VEC_IMPL_FUNC(mptest_sym_build, destroy)
MPTEST__VEC_IMPL_FUNC(mptest_sym_build, push)
MPTEST__VEC_IMPL_FUNC(mptest_sym_build, pop)
MPTEST__VEC_IMPL_FUNC(mptest_sym_build, getref)

typedef struct mptest__sym_parse {
  mptest_sym_build_vec sym_stack;
  mptest_size sym_stack_ptr;
  mptest_sym_build* base_build;
  mptest__str atom_str;
  mptest_int32 num;
} mptest__sym_parse;

typedef enum mptest__sym_parse_state {
  MPTEST__SYM_PARSE_STATE_EXPR,
  MPTEST__SYM_PARSE_STATE_ATOM,
  MPTEST__SYM_PARSE_STATE_NUM,
  MPTEST__SYM_PARSE_STATE_NUM_HEX,
  MPTEST__SYM_PARSE_STATE_NUM_HEX_X,
  MPTEST__SYM_PARSE_STATE_STRING,
  MPTEST__SYM_PARSE_STATE_STRING_ESCAPE,
  MPTEST__SYM_PARSE_STATE_CHAR,
  MPTEST__SYM_PARSE_STATE_CHAR_ESCAPE,
  MPTEST__SYM_PARSE_STATE_CHAR_AFTER
} mptest__sym_parse_state;

MPTEST_INTERNAL int mptest__sym_parse_expr_begin(mptest__sym_parse* parse)
{
  int err = 0;
  mptest_sym_build next_build;
  if (parse->sym_stack_ptr == 0) {
    if ((err = mptest_sym_build_expr(parse->base_build, &next_build))) {
      return err;
    }
  } else {
    if ((err = mptest_sym_build_expr(
             mptest_sym_build_vec_getref(
                 &parse->sym_stack, parse->sym_stack_ptr - 1),
             &next_build))) {
      return err;
    }
  }
  if ((err = mptest_sym_build_vec_push(&parse->sym_stack, next_build))) {
    return err;
  }
  parse->sym_stack_ptr++;
  return err;
}

MPTEST_INTERNAL int mptest__sym_parse_expr_end(mptest__sym_parse* parse)
{
  if (parse->sym_stack_ptr == 0) {
    return 1;
  }
  parse->sym_stack_ptr--;
  mptest_sym_build_vec_pop(&parse->sym_stack);
  return 0;
}

MPTEST_INTERNAL int mptest__sym_parse_esc(mptest__sym_parse* parse, mptest_char ch)
{
  if (ch == 'n') {
    parse->num = '\n';
  } else if (ch == 'r') {
    parse->num = '\r';
  } else if (ch == '0') {
    parse->num = '\0';
  } else if (ch == 't') {
    parse->num = '\t';
  } else {
    return 0;
  }
  return 1;
}

MPTEST_INTERNAL int mptest__sym_do_parse(
    mptest_sym_build* build_in, const mptest__str_view in, const char** err_msg,
    mptest_size* err_pos)
{
  mptest__sym_parse parse;
  mptest_size str_loc = 0;
  int err = 0;
  int state = MPTEST__SYM_PARSE_STATE_EXPR;
  mptest_sym_build_vec_init(&parse.sym_stack);
  parse.sym_stack_ptr = 0;
  parse.num = 0;
  mptest__str_init(&parse.atom_str);
  parse.base_build = build_in;
  while (str_loc != mptest__str_view_size(&in)) {
    mptest_char ch = mptest__str_view_get_data(&in)[str_loc];
    mptest_sym_build* current_build = parse.base_build;
    if (parse.sym_stack_ptr) {
      current_build = mptest_sym_build_vec_getref(
          &parse.sym_stack, parse.sym_stack_ptr - 1);
    }
    if (state == MPTEST__SYM_PARSE_STATE_EXPR) {
      if (mptest__sym_isblank(ch)) {
        state = MPTEST__SYM_PARSE_STATE_EXPR;
      } else if (ch == '(') {
        if ((err = mptest__sym_parse_expr_begin(&parse))) {
          goto error;
        }
        state = MPTEST__SYM_PARSE_STATE_EXPR;
      } else if (ch == ')') {
        if ((err = mptest__sym_parse_expr_end(&parse))) {
          goto error;
        }
        state = MPTEST__SYM_PARSE_STATE_EXPR;
      } else if (ch == '0') {
        parse.num = 0;
        state = MPTEST__SYM_PARSE_STATE_NUM_HEX_X;
      } else if (
          ch == '1' || ch == '2' || ch == '3' || ch == '4' || ch == '5' ||
          ch == '6' || ch == '7' || ch == '8' || ch == '9') {
        parse.num = ch - '0';
        state = MPTEST__SYM_PARSE_STATE_NUM;
      } else if (ch == '"') {
        mptest__str_destroy(&parse.atom_str);
        mptest__str_init(&parse.atom_str);
        state = MPTEST__SYM_PARSE_STATE_STRING;
      } else if (ch == '\'') {
        state = MPTEST__SYM_PARSE_STATE_CHAR;
      } else {
        mptest__str_destroy(&parse.atom_str);
        mptest__str_init(&parse.atom_str);
        if ((err = mptest__str_cat_n(&parse.atom_str, &ch, 1))) {
          goto error;
        }
        state = MPTEST__SYM_PARSE_STATE_ATOM;
      }
    } else if (state == MPTEST__SYM_PARSE_STATE_ATOM) {
      if (mptest__sym_isblank(ch)) {
        if ((err = mptest_sym_build_str(
                 current_build, (const char*)mptest__str_get_data(&parse.atom_str),
                 mptest__str_size(&parse.atom_str)))) {
          goto error;
        }
        state = MPTEST__SYM_PARSE_STATE_EXPR;
      } else if (ch == '(') {
        if ((err = mptest_sym_build_str(
                 current_build, (const char*)mptest__str_get_data(&parse.atom_str),
                 mptest__str_size(&parse.atom_str)))) {
          goto error;
        }
        if ((err = mptest__sym_parse_expr_begin(&parse))) {
          goto error;
        }
        state = MPTEST__SYM_PARSE_STATE_EXPR;
      } else if (ch == ')') {
        if ((err = mptest_sym_build_str(
                 current_build, (const char*)mptest__str_get_data(&parse.atom_str),
                 mptest__str_size(&parse.atom_str)))) {
          goto error;
        }
        if ((err = mptest__sym_parse_expr_end(&parse))) {
          goto error;
        }
        state = MPTEST__SYM_PARSE_STATE_EXPR;
      } else {
        if ((err = mptest__str_cat_n(&parse.atom_str, &ch, 1))) {
          goto error;
        }
        state = MPTEST__SYM_PARSE_STATE_ATOM;
      }
    } else if (state == MPTEST__SYM_PARSE_STATE_NUM) {
      if (mptest__sym_isblank(ch)) {
        if ((err = mptest_sym_build_num(current_build, parse.num))) {
          goto error;
        }
        state = MPTEST__SYM_PARSE_STATE_EXPR;
      } else if (
          ch == '0' || ch == '1' || ch == '2' || ch == '3' || ch == '4' ||
          ch == '5' || ch == '6' || ch == '7' || ch == '8' || ch == '9') {
        parse.num *= 10;
        parse.num += ch - '0';
        state = MPTEST__SYM_PARSE_STATE_NUM;
      } else if (ch == ')') {
        if ((err = mptest_sym_build_num(current_build, parse.num))) {
          goto error;
        }
        if ((err = mptest__sym_parse_expr_end(&parse))) {
          goto error;
        }
        state = MPTEST__SYM_PARSE_STATE_EXPR;
      } else {
        *err_msg = "invalid character for number literal";
        err = 2;
        goto error;
      }
    } else if (state == MPTEST__SYM_PARSE_STATE_NUM_HEX_X) {
      if (mptest__sym_isblank(ch)) {
        if ((err = mptest_sym_build_num(current_build, 0))) {
          goto error;
        }
        state = MPTEST__SYM_PARSE_STATE_EXPR;
      } else if (ch == 'x') {
        state = MPTEST__SYM_PARSE_STATE_NUM_HEX;
      } else if (ch == ')') {
        if ((err = mptest_sym_build_num(current_build, 0))) {
          goto error;
        }
        if ((err = mptest__sym_parse_expr_end(&parse))) {
          goto error;
        }
        state = MPTEST__SYM_PARSE_STATE_EXPR;
      } else {
        *err_msg = "expected a 'x' for hex literal";
        err = 2;
        goto error;
      }
    } else if (state == MPTEST__SYM_PARSE_STATE_NUM_HEX) {
      if (mptest__sym_isblank(ch)) {
        if ((err = mptest_sym_build_num(current_build, parse.num))) {
          goto error;
        }
        state = MPTEST__SYM_PARSE_STATE_EXPR;
      } else if (
          ch == '0' || ch == '1' || ch == '2' || ch == '3' || ch == '4' ||
          ch == '5' || ch == '6' || ch == '7' || ch == '8' || ch == '9') {
        parse.num *= 16;
        parse.num += ch - '0';
        state = MPTEST__SYM_PARSE_STATE_NUM_HEX;
      } else if (
          ch == 'A' || ch == 'B' || ch == 'C' || ch == 'D' || ch == 'E' ||
          ch == 'F') {
        parse.num *= 16;
        parse.num += (ch - 'A') + 10;
        state = MPTEST__SYM_PARSE_STATE_NUM_HEX;
      } else if (ch == ')') {
        if ((err = mptest_sym_build_num(current_build, parse.num))) {
          goto error;
        }
        if ((err = mptest__sym_parse_expr_end(&parse))) {
          goto error;
        }
        state = MPTEST__SYM_PARSE_STATE_EXPR;
      } else {
        *err_msg = "invalid character for hex literal";
        err = 2;
        goto error;
      }
    } else if (state == MPTEST__SYM_PARSE_STATE_STRING) {
      if (ch == '"') {
        if ((err = mptest_sym_build_str(
                 current_build, (const char*)mptest__str_get_data(&parse.atom_str),
                 mptest__str_size(&parse.atom_str)))) {
          goto error;
        }
        state = MPTEST__SYM_PARSE_STATE_EXPR;
      } else if (ch == '\\') {
        state = MPTEST__SYM_PARSE_STATE_STRING_ESCAPE;
      } else {
        if ((err = mptest__str_cat_n(&parse.atom_str, &ch, 1))) {
          goto error;
        }
        state = MPTEST__SYM_PARSE_STATE_STRING;
      }
    } else if (state == MPTEST__SYM_PARSE_STATE_STRING_ESCAPE) {
      if (mptest__sym_parse_esc(&parse, ch)) {
        mptest_char n = (mptest_char)parse.num;
        if ((err = mptest__str_cat_n(&parse.atom_str, &n, 1))) {
          goto error;
        }
        state = MPTEST__SYM_PARSE_STATE_STRING;
      } else {
        *err_msg = "invalid escape character";
        err = 2;
        goto error;
      }
    } else if (state == MPTEST__SYM_PARSE_STATE_CHAR) {
      if (ch == '\'') {
        if ((err = mptest_sym_build_num(current_build, 0))) {
          goto error;
        }
        state = MPTEST__SYM_PARSE_STATE_EXPR;
      } else if (ch == '\\') {
        state = MPTEST__SYM_PARSE_STATE_CHAR_ESCAPE;
      } else {
        parse.num = ch;
        state = MPTEST__SYM_PARSE_STATE_CHAR_AFTER;
      }
    } else if (state == MPTEST__SYM_PARSE_STATE_CHAR_ESCAPE) {
      if (mptest__sym_parse_esc(&parse, ch)) {
        mptest_char n = (mptest_char)parse.num;
        if ((err = mptest__str_cat_n(&parse.atom_str, &n, 1))) {
          goto error;
        }
        state = MPTEST__SYM_PARSE_STATE_CHAR_AFTER;
      } else {
        *err_msg = "invalid escape character";
        err = 2;
        goto error;
      }
    } else if (state == MPTEST__SYM_PARSE_STATE_CHAR_AFTER) {
      if (ch == '\'') {
        if ((err = mptest_sym_build_num(current_build, parse.num))) {
          goto error;
        }
        state = MPTEST__SYM_PARSE_STATE_EXPR;
      } else {
        *err_msg = "expected a \'";
        err = 2;
        goto error;
      }
    }
    str_loc++;
  }
  if (state != MPTEST__SYM_PARSE_STATE_EXPR) {
    *err_msg = "top-level tree must be an expression, not an atom";
    err = 2;
    goto error;
  }
error:
  if (err == 2) {
    *err_pos = str_loc;
  }
  mptest_sym_build_vec_destroy(&parse.sym_stack);
  mptest__str_destroy(&parse.atom_str);
  return err;
}

MPTEST_INTERNAL void
mptest__sym_dump(mptest_sym* sym, mptest_int32 parent_ref, mptest_int32 indent)
{
  mptest__sym_tree* tree;
  mptest_int32 child_ref;
  mptest_int32 i;
  if (parent_ref == MPTEST__SYM_NONE) {
    return;
  }
  tree = mptest__sym_get(sym, parent_ref);
  if (tree->first_child_ref == MPTEST__SYM_NONE) {
    if (tree->type == MPTEST__SYM_TYPE_ATOM_NUMBER) {
      printf("%i", tree->data.num);
    } else if (tree->type == MPTEST__SYM_TYPE_ATOM_STRING) {
      printf("%s", (const char*)mptest__str_get_data(&tree->data.str));
    } else if (tree->type == MPTEST__SYM_TYPE_EXPR) {
      printf("()");
    }
  } else {
    printf("\n");
    for (i = 0; i < indent; i++) {
      printf("  ");
    }
    printf("(");
    child_ref = tree->first_child_ref;
    while (child_ref != MPTEST__SYM_NONE) {
      mptest__sym_tree* child = mptest__sym_get(sym, child_ref);
      mptest__sym_dump(sym, child_ref, indent + 1);
      child_ref = child->next_sibling_ref;
      if (child_ref != MPTEST__SYM_NONE) {
        printf(" ");
      }
    }
    printf(")");
  }
}

MPTEST_INTERNAL int mptest__sym_equals(
    mptest_sym* sym, mptest_sym* other, mptest_int32 sym_ref, mptest_int32 other_ref)
{
  mptest__sym_tree* parent_tree;
  mptest__sym_tree* other_tree;
  if ((sym_ref == other_ref) && sym_ref == MPTEST__SYM_NONE) {
    return 1;
  } else if (sym_ref == MPTEST__SYM_NONE || other_ref == MPTEST__SYM_NONE) {
    return 0;
  }
  parent_tree = mptest__sym_get(sym, sym_ref);
  other_tree = mptest__sym_get(other, other_ref);
  if (parent_tree->type != other_tree->type) {
    return 0;
  } else {
    if (parent_tree->type == MPTEST__SYM_TYPE_ATOM_NUMBER) {
      if (parent_tree->data.num != other_tree->data.num) {
        return 0;
      }
      return 1;
    } else if (parent_tree->type == MPTEST__SYM_TYPE_ATOM_STRING) {
      if (mptest__str_cmp(&parent_tree->data.str, &other_tree->data.str) != 0) {
        return 0;
      }
      return 1;
    } else if (parent_tree->type == MPTEST__SYM_TYPE_EXPR) {
      mptest_int32 parent_child_ref = parent_tree->first_child_ref;
      mptest_int32 other_child_ref = other_tree->first_child_ref;
      mptest__sym_tree* parent_child;
      mptest__sym_tree* other_child;
      while (parent_child_ref != MPTEST__SYM_NONE &&
             other_child_ref != MPTEST__SYM_NONE) {
        if (!mptest__sym_equals(
                sym, other, parent_child_ref, other_child_ref)) {
          return 0;
        }
        parent_child = mptest__sym_get(sym, parent_child_ref);
        other_child = mptest__sym_get(other, other_child_ref);
        parent_child_ref = parent_child->next_sibling_ref;
        other_child_ref = other_child->next_sibling_ref;
      }
      return parent_child_ref == other_child_ref;
    }
    return 0;
  }
}

MPTEST_API void mptest_sym_build_init(
    mptest_sym_build* build, mptest_sym* sym, mptest_int32 parent_ref,
    mptest_int32 prev_child_ref)
{
  build->sym = sym;
  build->parent_ref = parent_ref;
  build->prev_child_ref = prev_child_ref;
}

MPTEST_API void mptest_sym_build_destroy(mptest_sym_build* build)
{
  MPTEST__UNUSED(build);
}

MPTEST_API int mptest_sym_build_expr(mptest_sym_build* build, mptest_sym_build* sub)
{
  mptest__sym_tree new_tree;
  mptest_int32 new_child_ref;
  int err = 0;
  mptest__sym_tree_init(&new_tree, MPTEST__SYM_TYPE_EXPR);
  if ((err = mptest__sym_new(
           build->sym, build->parent_ref, build->prev_child_ref, new_tree,
           &new_child_ref))) {
    return err;
  }
  build->prev_child_ref = new_child_ref;
  mptest_sym_build_init(sub, build->sym, new_child_ref, MPTEST__SYM_NONE);
  return err;
}

MPTEST_API int
mptest_sym_build_str(mptest_sym_build* build, const char* str, mptest_size str_size)
{
  mptest__sym_tree new_tree;
  mptest_int32 new_child_ref;
  int err = 0;
  mptest__sym_tree_init(&new_tree, MPTEST__SYM_TYPE_ATOM_STRING);
  if ((err =
           mptest__str_init_n(&new_tree.data.str, (const mptest_char*)str, str_size))) {
    return err;
  }
  if ((err = mptest__sym_new(
           build->sym, build->parent_ref, build->prev_child_ref, new_tree,
           &new_child_ref))) {
    return err;
  }
  build->prev_child_ref = new_child_ref;
  return err;
}

MPTEST_API int mptest_sym_build_cstr(mptest_sym_build* build, const char* cstr)
{
  return mptest_sym_build_str(build, cstr, mptest__str_slen((const mptest_char*)cstr));
}

MPTEST_API int mptest_sym_build_num(mptest_sym_build* build, mptest_int32 num)
{
  mptest__sym_tree new_tree;
  mptest_int32 new_child_ref;
  int err = 0;
  mptest__sym_tree_init(&new_tree, MPTEST__SYM_TYPE_ATOM_NUMBER);
  new_tree.data.num = num;
  if ((err = mptest__sym_new(
           build->sym, build->parent_ref, build->prev_child_ref, new_tree,
           &new_child_ref))) {
    return err;
  }
  build->prev_child_ref = new_child_ref;
  return err;
}

MPTEST_API int mptest_sym_build_type(mptest_sym_build* build, const char* type)
{
  mptest_sym_build new;
  int err = 0;
  if ((err = mptest_sym_build_expr(build, &new))) {
    return err;
  }
  if ((err = mptest_sym_build_cstr(&new, type))) {
    return err;
  }
  *build = new;
  return err;
}

MPTEST_API void mptest_sym_walk_init(
    mptest_sym_walk* walk, const mptest_sym* sym, mptest_int32 parent_ref,
    mptest_int32 prev_child_ref)
{
  walk->sym = sym;
  walk->parent_ref = parent_ref;
  walk->prev_child_ref = prev_child_ref;
}

MPTEST_API int
mptest__sym_walk_peeknext(mptest_sym_walk* walk, mptest_int32* out_child_ref)
{
  const mptest__sym_tree* prev;
  mptest_int32 child_ref;
  if (walk->parent_ref == MPTEST__SYM_NONE) {
    if (!mptest__sym_tree_vec_size(&walk->sym->tree_storage)) {
      return SYM_EMPTY;
    }
    child_ref = 0;
  } else if (walk->prev_child_ref == MPTEST__SYM_NONE) {
    prev = mptest__sym_getcref(walk->sym, walk->parent_ref);
    child_ref = prev->first_child_ref;
  } else {
    prev = mptest__sym_getcref(walk->sym, walk->prev_child_ref);
    child_ref = prev->next_sibling_ref;
  }
  if (child_ref == MPTEST__SYM_NONE) {
    return SYM_NO_MORE;
  }
  *out_child_ref = child_ref;
  return 0;
}

MPTEST_API int
mptest__sym_walk_getnext(mptest_sym_walk* walk, mptest_int32* out_child_ref)
{
  int err = 0;
  if ((err = mptest__sym_walk_peeknext(walk, out_child_ref))) {
    return err;
  }
  walk->prev_child_ref = *out_child_ref;
  return err;
}

MPTEST_API int mptest_sym_walk_getexpr(mptest_sym_walk* walk, mptest_sym_walk* sub)
{
  int err = 0;
  const mptest__sym_tree* child;
  mptest_int32 child_ref;
  if ((err = mptest__sym_walk_getnext(walk, &child_ref))) {
    return err;
  }
  child = mptest__sym_getcref(walk->sym, child_ref);
  if (child->type != MPTEST__SYM_TYPE_EXPR) {
    return SYM_WRONG_TYPE;
  } else {
    mptest_sym_walk_init(sub, walk->sym, child_ref, MPTEST__SYM_NONE);
    return 0;
  }
}

MPTEST_API int mptest_sym_walk_getstr(
    mptest_sym_walk* walk, const char** str, mptest_size* str_size)
{
  int err = 0;
  const mptest__sym_tree* child;
  mptest_int32 child_ref;
  if ((err = mptest__sym_walk_getnext(walk, &child_ref))) {
    return err;
  }
  child = mptest__sym_getcref(walk->sym, child_ref);
  if (child->type != MPTEST__SYM_TYPE_ATOM_STRING) {
    return SYM_WRONG_TYPE;
  } else {
    *str = (const char*)mptest__str_get_data(&child->data.str);
    *str_size = mptest__str_size(&child->data.str);
    return 0;
  }
}

MPTEST_API int mptest_sym_walk_getnum(mptest_sym_walk* walk, mptest_int32* num)
{
  int err = 0;
  const mptest__sym_tree* child;
  mptest_int32 child_ref;
  if ((err = mptest__sym_walk_getnext(walk, &child_ref))) {
    return err;
  }
  child = mptest__sym_getcref(walk->sym, child_ref);
  if (child->type != MPTEST__SYM_TYPE_ATOM_NUMBER) {
    return SYM_WRONG_TYPE;
  } else {
    *num = child->data.num;
    return 0;
  }
}

MPTEST_API int
mptest_sym_walk_checktype(mptest_sym_walk* walk, const char* expected_type)
{
  const char* str;
  int err = 0;
  mptest_size str_size;
  mptest_size expected_size = mptest__str_slen(expected_type);
  mptest__str_view expected, actual;
  mptest__str_view_init_n(&expected, expected_type, expected_size);
  if ((err = mptest_sym_walk_getstr(walk, &str, &str_size))) {
    return err;
  }
  mptest__str_view_init_n(&actual, str, str_size);
  if (mptest__str_view_cmp(&expected, &actual) != 0) {
    return SYM_WRONG_TYPE;
  }
  return 0;
}

MPTEST_API int mptest_sym_walk_hasmore(mptest_sym_walk* walk)
{
  const mptest__sym_tree* prev;
  if (walk->parent_ref == MPTEST__SYM_NONE) {
    if (mptest__sym_tree_vec_size(&walk->sym->tree_storage) == 0) {
      return 0;
    } else {
      return 1;
    }
  } else if (walk->prev_child_ref == MPTEST__SYM_NONE) {
    prev = mptest__sym_getcref(walk->sym, walk->parent_ref);
    if (prev->first_child_ref == MPTEST__SYM_NONE) {
      return 0;
    } else {
      return 1;
    }
  } else {
    prev = mptest__sym_getcref(walk->sym, walk->prev_child_ref);
    if (prev->next_sibling_ref == MPTEST__SYM_NONE) {
      return 0;
    } else {
      return 1;
    }
  }
}

MPTEST_API int mptest_sym_walk_peekstr(mptest_sym_walk* walk)
{
  int err = 0;
  const mptest__sym_tree* child;
  mptest_int32 child_ref;
  if ((err = mptest__sym_walk_peeknext(walk, &child_ref))) {
    return err;
  }
  child = mptest__sym_getcref(walk->sym, child_ref);
  return child->type == MPTEST__SYM_TYPE_ATOM_STRING;
}

MPTEST_API int mptest_sym_walk_peekexpr(mptest_sym_walk* walk)
{
  int err = 0;
  const mptest__sym_tree* child;
  mptest_int32 child_ref;
  if ((err = mptest__sym_walk_peeknext(walk, &child_ref))) {
    return err;
  }
  child = mptest__sym_getcref(walk->sym, child_ref);
  return child->type == MPTEST__SYM_TYPE_EXPR;
}

MPTEST_API int mptest_sym_walk_peeknum(mptest_sym_walk* walk)
{
  int err = 0;
  const mptest__sym_tree* child;
  mptest_int32 child_ref;
  if ((err = mptest__sym_walk_peeknext(walk, &child_ref))) {
    return err;
  }
  child = mptest__sym_getcref(walk->sym, child_ref);
  return child->type == MPTEST__SYM_TYPE_ATOM_NUMBER;
}

MPTEST_API int mptest__sym_check_init(
    mptest_sym_build* build_out, const char* str, const char* file, int line,
    const char* msg)
{
  int err = 0;
  mptest_sym* sym_actual = MPTEST_NULL;
  mptest_sym* sym_expected = MPTEST_NULL;
  mptest__str_view in_str_view;
  const char* err_msg;
  mptest_size err_pos;
  mptest_sym_build parse_build;
  sym_actual = (mptest_sym*)MPTEST_MALLOC(sizeof(mptest_sym));
  if (sym_actual == MPTEST_NULL) {
    err = 1;
    goto error;
  }
  mptest__sym_init(sym_actual);
  sym_expected = (mptest_sym*)MPTEST_MALLOC(sizeof(mptest_sym));
  if (sym_expected == MPTEST_NULL) {
    err = 1;
    goto error;
  }
  mptest__sym_init(sym_expected);
  mptest_sym_build_init(
      build_out, sym_actual, MPTEST__SYM_NONE, MPTEST__SYM_NONE);
  mptest__str_view_init_n(&in_str_view, str, mptest__str_slen(str));
  mptest_sym_build_init(
      &parse_build, sym_expected, MPTEST__SYM_NONE, MPTEST__SYM_NONE);
  if ((err = mptest__sym_do_parse(
           &parse_build, in_str_view, &err_msg, &err_pos))) {
    goto error;
  }
  mptest__state_g.fail_data.sym_fail_data.sym_actual = sym_actual;
  mptest__state_g.fail_data.sym_fail_data.sym_expected = sym_expected;
  return err;
error:
  if (sym_actual != MPTEST_NULL) {
    mptest__sym_destroy(sym_actual);
    MPTEST_FREE(sym_actual);
  }
  if (sym_expected != MPTEST_NULL) {
    mptest__sym_destroy(sym_expected);
    MPTEST_FREE(sym_expected);
  }
  if (err == 2) { /* parse error */
    mptest__state_g.fail_reason = MPTEST__FAIL_REASON_SYM_SYNTAX;
    mptest__state_g.fail_file = file;
    mptest__state_g.fail_line = line;
    mptest__state_g.fail_msg = msg;
    mptest__state_g.fail_data.sym_syntax_error_data.err_msg = err_msg;
    mptest__state_g.fail_data.sym_syntax_error_data.err_pos = err_pos;
  } else if (err == 1) { /* no mem */
    mptest__state_g.fail_reason = MPTEST__FAIL_REASON_NOMEM;
    mptest__state_g.fail_file = file;
    mptest__state_g.fail_line = line;
    mptest__state_g.fail_msg = msg;
  }
  return err;
}

MPTEST_API int mptest__sym_check(const char* file, int line, const char* msg)
{
  if (!mptest__sym_equals(
          mptest__state_g.fail_data.sym_fail_data.sym_actual,
          mptest__state_g.fail_data.sym_fail_data.sym_expected, 0, 0)) {
    mptest__state_g.fail_reason = MPTEST__FAIL_REASON_SYM_INEQUALITY;
    mptest__state_g.fail_file = file;
    mptest__state_g.fail_line = line;
    mptest__state_g.fail_msg = msg;
    return 1;
  } else {
    return 0;
  }
}

MPTEST_API void mptest__sym_check_destroy()
{
  mptest__sym_destroy(mptest__state_g.fail_data.sym_fail_data.sym_actual);
  mptest__sym_destroy(mptest__state_g.fail_data.sym_fail_data.sym_expected);
  MPTEST_FREE(mptest__state_g.fail_data.sym_fail_data.sym_actual);
  MPTEST_FREE(mptest__state_g.fail_data.sym_fail_data.sym_expected);
}

MPTEST_API int mptest__sym_make_init(
    mptest_sym_build* build_out, mptest_sym_walk* walk_out, const char* str,
    const char* file, int line, const char* msg)
{
  mptest__str_view in_str_view;
  const char* err_msg;
  mptest_size err_pos;
  int err = 0;
  mptest_sym* sym_out;
  sym_out = (mptest_sym*)MPTEST_MALLOC(sizeof(mptest_sym));
  if (sym_out == MPTEST_NULL) {
    err = 1;
    goto error;
  }
  mptest__sym_init(sym_out);
  mptest_sym_build_init(build_out, sym_out, MPTEST__SYM_NONE, MPTEST__SYM_NONE);
  mptest__str_view_init_n(&in_str_view, str, mptest__str_slen(str));
  if ((err =
           mptest__sym_do_parse(build_out, in_str_view, &err_msg, &err_pos))) {
    goto error;
  }
  mptest_sym_walk_init(walk_out, sym_out, MPTEST__SYM_NONE, MPTEST__SYM_NONE);
  return err;
error:
  if (sym_out) {
    mptest__sym_destroy(sym_out);
    MPTEST_FREE(sym_out);
  }
  if (err == 2) { /* parse error */
    mptest__state_g.fail_reason = MPTEST__FAIL_REASON_SYM_SYNTAX;
    mptest__state_g.fail_file = file;
    mptest__state_g.fail_line = line;
    mptest__state_g.fail_msg = msg;
    mptest__state_g.fail_data.sym_syntax_error_data.err_msg = err_msg;
    mptest__state_g.fail_data.sym_syntax_error_data.err_pos = err_pos;
  } else if (err == 1) { /* no mem */
    mptest__state_g.fail_reason = MPTEST__FAIL_REASON_NOMEM;
    mptest__state_g.fail_file = file;
    mptest__state_g.fail_line = line;
    mptest__state_g.fail_msg = msg;
  }
  return err;
}

MPTEST_API void mptest__sym_make_destroy(mptest_sym_build* build)
{
  mptest__sym_destroy(build->sym);
  MPTEST_FREE(build->sym);
}

#endif

/* mptest */
#if MPTEST_USE_TIME

MPTEST_INTERNAL void mptest__time_init(struct mptest__state* state)
{
  state->time_state.program_start_time = clock();
  state->time_state.suite_start_time = 0;
  state->time_state.test_start_time = 0;
}

MPTEST_INTERNAL void mptest__time_destroy(struct mptest__state* state)
{
  (void)(state);
}

#endif

#endif /* MPTEST_IMPLEMENTATION */
#endif /* MPTEST_H */

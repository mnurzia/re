#ifndef MPTEST_H
#define MPTEST_H
/*                  _            _   
 *                 | |          | |  
 *  _ __ ___  _ __ | |_ ___  ___| |_ 
 * | '_ ` _ \| '_ \| __/ _ \/ __| __|
 * | | | | | | |_) | ||  __/\__ \ |_ 
 * |_| |_| |_| .__/ \__\___||___/\__|
 *           | |                     
 *           |_|                      */


/* Set MPTEST_USE_DYN_ALLOC to 1 in order to allow the usage of malloc() and
 * free() inside mptest. Mptest can run without these functions, but certain
 * features are disabled. */
#ifndef MPTEST_USE_DYN_ALLOC
#define MPTEST_USE_DYN_ALLOC 1
#endif

/* Set MPTEST_USE_LONGJMP to 1 in order to use runtime assert checking
 * facilities. Must be enabled to use heap profiling. */
#ifndef MPTEST_USE_LONGJMP
#define MPTEST_USE_LONGJMP 1
#endif

/* Set MPTEST_USE_SYM to 1 in order to use s-expression data structures for
 * testing. MPTEST_USE_DYN_ALLOC must be set. */
#ifndef MPTEST_USE_SYM
#define MPTEST_USE_SYM 1
#endif

/* Set MPTEST_USE_LEAKCHECK to 1 in order to use runtime leak checking
 * facilities. MPTEST_USE_LONGJMP must be set. */
#if MPTEST_USE_LONGJMP
#if MPTEST_USE_DYN_ALLOC
#ifndef MPTEST_USE_LEAKCHECK
#define MPTEST_USE_LEAKCHECK 1
#endif
#endif
#endif /* #if MPTEST_USE_LONGJMP */

/* Set MPTEST_USE_COLOR to 1 if you want ANSI-colored output. */
#ifndef MPTEST_USE_COLOR
#define MPTEST_USE_COLOR 1
#endif

/* Set MPTEST_USE_TIME to 1 if you want to time tests or suites. */
#ifndef MPTEST_USE_TIME
#define MPTEST_USE_TIME 1
#endif

/* Set MPTEST_USE_APARSE to 1 if you want to build a command line version of
 * the test program. */
#if MPTEST_USE_DYN_ALLOC
#ifndef MPTEST_USE_APARSE
#define MPTEST_USE_APARSE 0
#endif
#endif

/* Set MPTEST_USE_FUZZ to 1 if you want to include fuzzing and test shuffling
 * support. */
#ifndef MPTEST_USE_FUZZ
#define MPTEST_USE_FUZZ 1
#endif

/* Whether or not API definitions should be defined as static linkage (local to
 * the including source file), as opposed to external linkage. */
#ifndef MPTEST_STATIC
#define MPTEST_STATIC 0
#endif

/* Set MPTEST_USE_CUSTOM_MALLOC to 1 in order to use your own definitions for
 * malloc(), realloc() and free(). If MPTEST_USE_CUSTOM_MALLOC is set, you must
 * also define MPTEST_MALLOC, MPTEST_REALLOC and MPTEST_FREE. Otherwise,
 * <stdlib.h> is included and standard versions of malloc(), realloc() and
 * free() are used. */
#ifndef MPTEST_USE_CUSTOM_MALLOC
#define MPTEST_USE_CUSTOM_MALLOC 0
#endif

/* a malloc() function. Performs a memory allocation. */
/* See https://en.cppreference.com/w/c/memory/malloc for more information. */
#if MPTEST_USE_CUSTOM_MALLOC
#ifndef MPTEST_MALLOC
#define MPTEST_MALLOC MY_MALLOC
#endif
#endif

/* a realloc() function. Performs a memory reallocation. */
/* See https://en.cppreference.com/w/c/memory/realloc for more information. */
#if MPTEST_USE_CUSTOM_MALLOC
#ifndef MPTEST_REALLOC
#define MPTEST_REALLOC MY_REALLOC
#endif
#endif

/* a free() function. Returns memory back to the operating system. */
/* See https://en.cppreference.com/w/c/memory/free for more information. */
#if MPTEST_USE_CUSTOM_MALLOC
#ifndef MPTEST_FREE
#define MPTEST_FREE MY_FREE
#endif
#endif

/* Set MPTEST_USE_CUSTOM_LONGJMP to 1 in order to use your own definitions for
 * setjmp(), longjmp() and jmp_buf. If MPTEST_USE_CUSTOM_LONGJMP is set, you
 * must also define MPTEST_SETJMP, MPTEST_LONGJMP and MPTEST_JMP_BUF.
 * Otherwise, <setjmp.h> is included and standard versions of setjmp(),
 * longjmp() and jmp_buf are used. */
#ifndef MPTEST_USE_CUSTOM_LONGJMP
#define MPTEST_USE_CUSTOM_LONGJMP 0
#endif

/* setjmp() macro or function. Sets a nonlocal jump context. */
/* See https://en.cppreference.com/w/c/program/setjmp for more information. */
#if MPTEST_USE_CUSTOM_LONGJMP
#ifndef MPTEST_SETJMP
#define MPTEST_SETJMP MY_SETJMP
#endif
#endif

/* longjmp() macro or function. Jumps to a saved jmp_buf context. */
/* See https://en.cppreference.com/w/c/program/longjmp for more information. */
#if MPTEST_USE_CUSTOM_LONGJMP
#ifndef MPTEST_LONGJMP
#define MPTEST_LONGJMP MY_LONGJMP
#endif
#endif

/* jmp_buf type. Holds a context saved by setjmp(). */
/* See https://en.cppreference.com/w/c/program/jmp_buf for more information. */
#if MPTEST_USE_CUSTOM_LONGJMP
#ifndef MPTEST_JMP_BUF
#define MPTEST_JMP_BUF my_jmp_buf
#endif
#endif

/* Set MPTEST_USE_CUSTOM_ASSERT to 1 in order to use your own definition for
 * assert(). If MPTEST_USE_CUSTOM_ASSERT is set, you must also define
 * MPTEST_ASSERT. Otherwise, <assert.h> is included and a standard version of
 * assert() is used. */
#ifndef MPTEST_USE_CUSTOM_ASSERT
#define MPTEST_USE_CUSTOM_ASSERT 0
#endif

/* an assert() macro. Expects a single argument, which is a expression that
 * evaluates to either 1 or 0. */
/* See https://en.cppreference.com/w/c/error/assert for more information. */
#if MPTEST_USE_CUSTOM_ASSERT
#ifndef MPTEST_ASSERT
#define MPTEST_ASSERT MY_ASSERT
#endif
#endif

/* Set MPTEST_USE_CUSTOM_SIZE_TYPE to 1 in order to use your own definition for
 * size_t. If MPTEST_USE_CUSTOM_SIZE_TYPE is set, you must also define
 * MPTEST_SIZE_TYPE. Otherwise, <stddef.h> is included and a standard version
 * of size_t is used. */
#ifndef MPTEST_USE_CUSTOM_SIZE_TYPE
#define MPTEST_USE_CUSTOM_SIZE_TYPE 0
#endif

/* a type that can store any size. */
/* See https://en.cppreference.com/w/c/types/size_t for more information. */
#if MPTEST_USE_CUSTOM_SIZE_TYPE
#ifndef MPTEST_SIZE_TYPE
#define MPTEST_SIZE_TYPE my_size_t
#endif
#endif

/* Set MPTEST_DEBUG to 1 if you want to enable debug asserts,  bounds checks,
 * and more runtime diagnostics. */
#ifndef MPTEST_DEBUG
#define MPTEST_DEBUG 1
#endif


#define MPTEST_VERSION_MAJOR 0
#define MPTEST_VERSION_MINOR 0
#define MPTEST_VERSION_PATCH 1
#define MPTEST_VERSION_STRING "0.0.1"

#if MPTEST_STATIC
    #define MPTEST_API static
#else
    #define MPTEST_API extern
#endif

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
    #endif /*     #if __STDC_VERSION__ >= 201112L */
  #endif /*   #if defined(__STDC_VERSION__) */
#endif /* #ifndef __STDC__ */

#if MPTEST_USE_DYN_ALLOC
#if !MPTEST_USE_CUSTOM_MALLOC
#include <stdlib.h>
#define MPTEST_MALLOC malloc
#define MPTEST_REALLOC realloc
#define MPTEST_FREE free
#else
#if !defined(MPTEST_MALLOC) || !defined(MPTEST_REALLOC) || !defined(MPTEST_FREE)
#error In order to use MPTEST_USE_CUSTOM_MALLOC you must provide defnitions for MPTEST_MALLOC, MPTEST_REALLOC and MPTEST_FREE.
#endif
#endif /* #if !MPTEST_USE_CUSTOM_MALLOC */
#endif /* #if MPTEST_USE_DYN_ALLOC */

#if !MPTEST_USE_CUSTOM_LONGJMP
#include <setjmp.h>
#define MPTEST_SETJMP setjmp
#define MPTEST_LONGJMP longjmp
#define MPTEST_JMP_BUF jmp_buf
#else
#if !defined(MPTEST_SETJMP) || !defined(MPTEST_LONGJMP) || !defined(MPTEST_JMP_BUF)
#error In order to use MPTEST_USE_CUSTOM_LONGJMP you must provide defnitions for MPTEST_SETJMP, MPTEST_LONGJMP and MPTEST_JMP_BUF.
#endif
#endif /* #if !MPTEST_USE_CUSTOM_LONGJMP */

#if !MPTEST_USE_CUSTOM_ASSERT
#include <assert.h>
#define MPTEST_ASSERT assert
#else
#if !defined(MPTEST_ASSERT)
#error In order to use MPTEST_USE_CUSTOM_ASSERT you must provide a definition for MPTEST_ASSERT.
#endif
#endif /* #if !MPTEST_USE_CUSTOM_ASSERT */

#if MPTEST__CSTD >= 1999
#include <stdint.h>
typedef uint8_t mptest_uint8;
typedef int8_t mptest_int8;
typedef uint16_t mptest_uint16;
typedef int16_t mptest_int16;
typedef uint32_t mptest_uint32;
typedef int32_t mptest_int32;
#else
typedef unsigned char mptest_uint8;
typedef signed char mptest_int8;
typedef unsigned short mptest_uint16;
typedef signed short mptest_int16;
typedef unsigned int mptest_uint32;
typedef signed int mptest_int32;
#endif /* #if MPTEST__CSTD >= 1999 */

#define MPTEST_NULL 0

#if !MPTEST_USE_CUSTOM_SIZE_TYPE
#include <stddef.h>
#define MPTEST_SIZE_TYPE size_t
#else
#if !defined(MPTEST_SIZE_TYPE)
#error In order to use MPTEST_USE_CUSTOM_SIZE_TYPE you must provide a definition for MPTEST_SIZE_TYPE.
#endif
#endif /* #if !MPTEST_USE_CUSTOM_SIZE_TYPE */
typedef MPTEST_SIZE_TYPE mptest_size;

#define MPTEST__UNUSED(i) (void)(i)

#if MPTEST_USE_SYM
#if MPTEST_USE_DYN_ALLOC
#endif
#endif

#if MPTEST_USE_SYM
#if MPTEST_USE_DYN_ALLOC
#endif
#endif

#if MPTEST_USE_SYM
#if MPTEST_USE_DYN_ALLOC
#endif
#endif

#ifndef MPTEST_API_H
#define MPTEST_API_H


/* Forward declaration */
struct mptest__state;

/* How assert checking works (and why we need longjmp for it):
 * 1. You use the function ASSERT_ASSERT(statement) in your test code.
 * 2. Under the hood, ASSERT_ASSERT setjmps the current test, and runs the
 *    statement until an assert within the program fails.
 * 3. The assert hook longjmps out of the code into the previous setjmp from
 *    step (2).
 * 4. mptest recognizes this jump back and passes the test.
 * 5. If the jump back doesn't happen, mptest recognizes this too and fails the
 *    test, expecting an assertion failure. */
#if MPTEST_USE_LONGJMP
    #include <setjmp.h>

/* Enumeration of the reasons a `longjmp()` can happen from within a test.
 * When running an assertion like `ASSERT_ASSERT()`, we check the returned
 * jump reason to ensure that an assertion failure happened and not, e.g., a
 * malloc failure. */
typedef enum mptest__longjmp_reason
{
    MPTEST__LONGJMP_REASON_NONE,
    /* An assertion failure. */
    MPTEST__LONGJMP_REASON_ASSERT_FAIL = 1,
    #if MPTEST_USE_LEAKCHECK
    /* `malloc()` (the real one) *actually* returned NULL. As in, an actual
     * error. */
    MPTEST__LONGJMP_REASON_MALLOC_REALLY_RETURNED_NULL = 2,
    /* You passed a NULL pointer to `realloc()`. */
    MPTEST__LONGJMP_REASON_REALLOC_OF_NULL = 4,
    /* You passed an invalid pointer to `realloc()`. */
    MPTEST__LONGJMP_REASON_REALLOC_OF_INVALID = 8,
    /* You passed an already-freed pointer to `realloc()`. */
    MPTEST__LONGJMP_REASON_REALLOC_OF_FREED = 16,
    /* You passed an already-reallocated pointer to `realloc()`. */
    MPTEST__LONGJMP_REASON_REALLOC_OF_REALLOCED = 32,
    /* You passed a NULL pointer to `free()`. */
    MPTEST__LONGJMP_REASON_FREE_OF_NULL = 64,
    /* You passed an invalid pointer to `free()`. */
    MPTEST__LONGJMP_REASON_FREE_OF_INVALID = 128,
    /* You passed an already-freed pointer to `free()`. */
    MPTEST__LONGJMP_REASON_FREE_OF_FREED = 256,
    /* You passed an already-reallocated pointer to `free()`. */
    MPTEST__LONGJMP_REASON_FREE_OF_REALLOCED = 512,
    #endif /*     #if MPTEST_USE_LEAKCHECK */
    MPTEST__LONGJMP_REASON_LAST
} mptest__longjmp_reason;
#endif /* #if MPTEST_USE_LONGJMP */

#if MPTEST_USE_TIME
    #include <time.h>
#endif

#if MPTEST_USE_APARSE
    #define MPTEST__APARSE_ARG_COUNT 16
#endif

#if MPTEST_USE_FUZZ
typedef unsigned long mptest_rand;
#endif

/* Test result types. */
typedef enum mptest__result
{
    MPTEST__RESULT_PASS = 0,
    MPTEST__RESULT_FAIL = 1,
    /* an uncaught error that caused a `longjmp()` out of the test */
    /* or a miscellaneous error like a sym syntax error */
    MPTEST__RESULT_ERROR = 2,
    MPTEST__RESULT_SKIPPED = 3
} mptest__result;

/* The different ways a test can fail. */
typedef enum mptest__fail_reason
{
    MPTEST__FAIL_REASON_ASSERT_FAILURE,
#if MPTEST_USE_DYN_ALLOC
    MPTEST__FAIL_REASON_NOMEM,
#endif
#if MPTEST_USE_LEAKCHECK
    MPTEST__FAIL_REASON_LEAKED,
#endif
#if MPTEST_USE_SYM
    MPTEST__FAIL_REASON_SYM_INEQUALITY,
    MPTEST__FAIL_REASON_SYM_SYNTAX,
    MPTEST__FAIL_REASON_SYM_DESERIALIZE,
#endif
    MPTEST__FAIL_REASON_LAST
} mptest__fail_reason;

/* Type representing a function to be called whenever a suite is set up or torn
 * down. */
typedef void (*mptest__suite_callback)(void* data);

#if MPTEST_USE_SYM
typedef struct mptest_sym mptest_sym;

typedef struct mptest__sym_fail_data {
    mptest_sym* sym_actual;
    mptest_sym* sym_expected;
} mptest__sym_fail_data;

typedef struct mptest__sym_syntax_error_data {
    const char* err_msg;
    mptest_size err_pos;
} mptest__sym_syntax_error_data;
#endif /* #if MPTEST_USE_SYM */

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
    int         fail_line;
    /* Stores information about the failure. */
    /* Assert expression that caused the fail, if `fail_reason` ==
     * `MPTEST__FAIL_REASON_ASSERT_FAILURE` */
    /* Pointer to offending allocation, if `longjmp_reason` is one of the
     * malloc fail reasons */
    mptest__fail_data fail_data;
    /* Indentation level (used for output) */
    int indent_lvl;

#if MPTEST_USE_LONGJMP
    /* Saved setjmp context (used for testing asserts, etc.) */
    MPTEST_JMP_BUF longjmp_assert_context;
    /* Saved setjmp context (used to catch actual errors during testing) */
    MPTEST_JMP_BUF longjmp_test_context;
    /* 1 if we are checking for a jump, 0 if not. Used so that if an assertion
     * *accidentally* goes off, we can catch it. */
    mptest__longjmp_reason longjmp_checking;
    /* Reason for jumping (assertion failure, malloc/free failure, etc) */
    mptest__longjmp_reason longjmp_reason;
#endif /* #if MPTEST_USE_LONGJMP */

#if MPTEST_USE_LEAKCHECK
    /* 1 if current test should be audited for leaks, 0 otherwise. */
    int test_leak_checking;
    /* First and most recent blocks allocated. */
    struct mptest__leakcheck_block* first_block;
    struct mptest__leakcheck_block* top_block;
    /* Total number of allocations. */
    int total_allocations;
#endif /* #if MPTEST_USE_LEAKCHECK */

#if MPTEST_USE_TIME
    /* Start times that will be compared against later */
    clock_t program_start_time;
    clock_t suite_start_time;
    clock_t test_start_time;
#endif

#if MPTEST_USE_APARSE
    aparse_state aparse;
    /* Holds argument info, used instead of a call to malloc() */
    aparse_arg_info aparse_args[MPTEST__APARSE_ARG_COUNT];
    /* -t, --test : the test name to search for and run */
    const char* opt_test_name;
#endif /* #if MPTEST_USE_APARSE */

#if MPTEST_USE_FUZZ
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
#endif /* #if MPTEST_USE_FUZZ */
};

/* Global state object, used in all macros. */
extern struct mptest__state mptest__state_g;

/* Test function signature */
typedef enum mptest__result (*mptest__test_func)(void);

/* Internal functions that API macros call */
MPTEST_API void mptest__state_init(struct mptest__state* state);
MPTEST_API void mptest__state_destroy(struct mptest__state* state);
MPTEST_API void mptest__state_report(struct mptest__state* state);
MPTEST_API enum mptest__result mptest__state_before_test(
    struct mptest__state* state, mptest__test_func test_func,
    const char* test_name);
MPTEST_API void mptest__state_after_test(struct mptest__state* state,
    enum mptest__result                                         res);
MPTEST_API void mptest__state_before_suite(struct mptest__state* state,
    const char*                                                   suite_name);
MPTEST_API void mptest__state_after_suite(struct mptest__state* state);

MPTEST_API void mptest__assert_do_failure(const char* msg, const char* assert_expr, const char* file, int line);

MPTEST_API void mptest_assert_fail(void);

#if MPTEST_USE_LONGJMP
MPTEST_API void mptest__longjmp_exec(struct mptest__state* state,
    enum mptest__longjmp_reason reason, const char* file, int line, const char* msg);
#endif

#if MPTEST_USE_LEAKCHECK
MPTEST_API void* mptest__leakcheck_hook_malloc(struct mptest__state* state,
    const char* file, int line, size_t size);
MPTEST_API void  mptest__leakcheck_hook_free(struct mptest__state* state,
     const char* file, int line, void* ptr);
MPTEST_API void* mptest__leakcheck_hook_realloc(struct mptest__state* state,
    const char* file, int line, void* old_ptr, size_t new_size);
#endif /* #if MPTEST_USE_LEAKCHECK */

#if MPTEST_USE_APARSE
/* declare argv as pointer to const pointer to const char */
/* can change argv, can't change *argv, can't change **argv */
MPTEST_API aparse_error mptest__state_init_argv(struct mptest__state* state,
    int argc, aparse_argv argv);
#endif

#if MPTEST_USE_FUZZ
MPTEST_API mptest_rand mptest__fuzz_rand(struct mptest__state* state);
#endif

#define _ASSERT_PASS_BEHAVIOR(expr, msg) \
    do { \
        mptest__state_g.assertions++; \
    } while (0) 

#define _ASSERT_FAIL_BEHAVIOR(expr, msg)                                      \
    do {                                                                      \
        mptest__assert_do_failure(#msg, #expr,   \
            __FILE__, __LINE__);                                              \
        return MPTEST__RESULT_FAIL;                                           \
    } while (0)

/* Used for binary assertions (<, >, <=, >=, ==, !=) in order to format error
 * messages correctly. */
#define _ASSERT_BINOPm(lhs, rhs, op, msg)                                     \
    do {                                                                      \
        if (!((lhs)op(rhs))) {                                                \
            _ASSERT_FAIL_BEHAVIOR(lhs op rhs, msg);                           \
        } else {                                                              \
            _ASSERT_PASS_BEHAVIOR(lhs op rhs, msg);                           \
        }                                                                     \
    } while (0)

#define _ASSERT_BINOP(lhs, rhs, op)                                           \
    do {                                                                      \
        if (!((lhs)op(rhs))) {                                                \
            _ASSERT_FAIL_BEHAVIOR(lhs op rhs, lhs op rhs);                    \
        } else {                                                              \
            _ASSERT_PASS_BEHAVIOR(lhs op rhs, msg);                           \
        }                                                                     \
    } while (0)

/* Define a test. */
/* Usage:
 * TEST(test_name) {
 *     ASSERT(...);
 *     PASS();
 * } */
#define TEST(name) enum mptest__result mptest__test_##name(void)

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
#define RUN_TEST(test)                                                        \
    do {                                                                      \
        enum mptest__result res;                                              \
        res = mptest__state_before_test(&mptest__state_g,                     \
            mptest__test_##test, #test);                                      \
        mptest__state_after_test(&mptest__state_g, res);                      \
    } while (0)

/* Run a suite. */
#define RUN_SUITE(suite)                                                      \
    do {                                                                      \
        mptest__state_before_suite(&mptest__state_g, #suite);                 \
        mptest__suite_##suite();                                              \
        mptest__state_after_suite(&mptest__state_g);                          \
    } while (0)

#if MPTEST_USE_FUZZ

#define MPTEST__FUZZ_DEFAULT_ITERATIONS 500

/* Run a test a number of times, changing the RNG state each time. */
#define FUZZ_TEST(test) \
    do { \
        mptest__state_g.fuzz_iterations = MPTEST__FUZZ_DEFAULT_ITERATIONS; \
        mptest__state_g.fuzz_active = 1; \
        RUN_TEST(test); \
    } while (0)

#endif /* #if MPTEST_USE_FUZZ */

/* Unconditionally pass a suite. */
#define PASS()                                                                \
    do {                                                                      \
        return MPTEST__RESULT_PASS;                                           \
    } while (0)

#define ASSERTm(expr, msg)                                                    \
    do {                                                                      \
        if (!(expr)) {                                                        \
            _ASSERT_FAIL_BEHAVIOR(expr, msg);                                 \
        } else {                                                              \
            _ASSERT_PASS_BEHAVIOR(lhs op rhs, msg);                           \
        }                                                                     \
    } while (0)

#define ASSERT(expr) ASSERTm(expr, #expr)

#define ASSERT_EQm(lhs, rhs, msg)  _ASSERT_BINOPm(lhs, rhs, ==, msg)
#define ASSERT_NEQm(lhs, rhs, msg) _ASSERT_BINOPm(lhs, rhs, !=, msg)
#define ASSERT_GTm(lhs, rhs, msg)  _ASSERT_BINOPm(lhs, rhs, >, msg)
#define ASSERT_LTm(lhs, rhs, msg)  _ASSERT_BINOPm(lhs, rhs, <, msg)
#define ASSERT_GTEm(lhs, rhs, msg) _ASSERT_BINOPm(lhs, rhs, >=, msg)
#define ASSERT_LTEm(lhs, rhs, msg) _ASSERT_BINOPm(lhs, rhs, <=, msg)

#define ASSERT_EQ(lhs, rhs)  _ASSERT_BINOP(lhs, rhs, ==)
#define ASSERT_NEQ(lhs, rhs) _ASSERT_BINOP(lhs, rhs, !=)
#define ASSERT_GT(lhs, rhs)  _ASSERT_BINOP(lhs, rhs, >)
#define ASSERT_LT(lhs, rhs)  _ASSERT_BINOP(lhs, rhs, <)
#define ASSERT_GTE(lhs, rhs) _ASSERT_BINOP(lhs, rhs, >=)
#define ASSERT_LTE(lhs, rhs) _ASSERT_BINOP(lhs, rhs, <=)

#if MPTEST_USE_LONGJMP

    /* Assert that an assertion failure will occur within statement `stmt`. */
    #define ASSERT_ASSERTm(stmt, msg)                                         \
        do {                                                                  \
            mptest__state_g.longjmp_checking                                  \
                = MPTEST__LONGJMP_REASON_ASSERT_FAIL;                         \
            if (MPTEST_SETJMP(mptest__state_g.longjmp_assert_context) == 0) { \
                stmt;                                                         \
                mptest__state_g.longjmp_checking = 0;                         \
                _ASSERT_FAIL_BEHAVIOR(                                        \
                    "<runtime-assert-checked-function> " #stmt, msg);         \
            } else {                                                          \
                mptest__state_g.longjmp_checking = 0;                         \
                _ASSERT_PASS_BEHAVIOR(                                        \
                    "<runtime-assert-checked-function> " #stmt, msg);         \
            }                                                                 \
        } while (0)

    #define ASSERT_ASSERT(stmt) ASSERT_ASSERTm(stmt, #stmt)

    #if MPTEST_DETECT_UNCAUGHT_ASSERTS

        #define MPTEST_INJECT_ASSERTm(expr, msg)                               \
            do {                                                              \
                if (!(expr)) {                                                \
                    mptest_assert_fail(); \
                    mptest__state_g.fail_data.string_data = #expr;            \
                    mptest__longjmp_exec(&mptest__state_g,                    \
                        MPTEST__LONGJMP_REASON_ASSERT_FAIL, __FILE__,         \
                        __LINE__, msg);                                \
                }                                                             \
            } while (0)

    #else

        #define MPTEST_INJECT_ASSERTm(expr, msg)                               \
            do {                                                              \
                if (mptest__state_g.longjmp_checking                          \
                    & MPTEST__LONGJMP_REASON_ASSERT_FAIL) {                   \
                    if (!(expr)) {                                            \
                        mptest_assert_fail(); \
                        mptest__state_g.fail_data.string_data = #expr;        \
                        mptest__longjmp_exec(&mptest__state_g,                \
                            MPTEST__LONGJMP_REASON_ASSERT_FAIL, __FILE__,     \
                            __LINE__, msg);                            \
                    }                                                         \
                } else {                                                      \
                    MPTEST_ASSERT(expr);                              \
                }                                                             \
            } while (0)

    #endif /*     #if MPTEST_DETECT_UNCAUGHT_ASSERTS */

#else

    #define MPTEST_INJECT_ASSERTm(expr, msg) MPTEST_ASSERT(expr)

#endif /* #if MPTEST_USE_LONGJMP */

#define MPTEST_INJECT_ASSERT(expr) MPTEST_INJECT_ASSERTm(expr, #expr)

#if MPTEST_USE_LEAKCHECK

    #define MPTEST_INJECT_MALLOC(size)                                        \
        mptest__leakcheck_hook_malloc(&mptest__state_g, __FILE__, __LINE__,   \
            (size))
    #define MPTEST_INJECT_FREE(ptr)                                           \
        mptest__leakcheck_hook_free(&mptest__state_g, __FILE__, __LINE__,     \
            (ptr))
    #define MPTEST_INJECT_REALLOC(old_ptr, new_size)                          \
        mptest__leakcheck_hook_realloc(&mptest__state_g, __FILE__, __LINE__,  \
            (old_ptr), (new_size))

    #define MPTEST_ENABLE_LEAK_CHECKING()                                     \
        mptest__state_g.test_leak_checking = 1;

    #define MPTEST_DISABLE_LEAK_CHECKING()                                    \
        mptest__state_g.test_leak_checking = 0;
    
    #define TOTAL_ALLOCATIONS() (mptest__state_g.total_allocations)

#else

    #define MPTEST_INJECT_MALLOC(size) MPTEST_MALLOC(size)
    #define MPTEST_INJECT_FREE(ptr)    MPTEST_FREE(ptr)
    #define MPTEST_INJECT_REALLOC(old_ptr, new_size)                          \
        MPTEST_REALLOC(old_ptr, new_size)

#endif /* #if MPTEST_USE_LEAKCHECK */

#define MPTEST_MAIN_BEGIN() mptest__state_init(&mptest__state_g)

#define MPTEST_MAIN_BEGIN_ARGS(argc, argv)                                    \
    do {                                                                      \
        aparse_error res = mptest__state_init_argv(&mptest__state_g, argc,    \
            (char const* const*)(argv));                                      \
        if (res == APARSE_ERROR_SYNTAX) {                                     \
            return 0;                                                         \
        } else if (res != APARSE_ERROR_OK) {                                  \
            return (int)res;                                                  \
        }                                                                     \
    } while (0)

#define MPTEST_MAIN_END()                                                     \
    do {                                                                      \
        mptest__state_report(&mptest__state_g);                               \
        mptest__state_destroy(&mptest__state_g);                              \
    } while (0)

#if MPTEST_USE_FUZZ

#define RAND_PARAM(mod) \
    (mptest__fuzz_rand(&mptest__state_g) % (mod))

#endif

#if MPTEST_USE_SYM

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

MPTEST_API void mptest_sym_build_init(mptest_sym_build* build, mptest_sym* sym, mptest_int32 parent_ref, mptest_int32 prev_child_ref);
MPTEST_API int mptest_sym_build_expr(mptest_sym_build* build, mptest_sym_build* sub);
MPTEST_API int mptest_sym_build_str(mptest_sym_build* build, const char* str, mptest_size str_size);
MPTEST_API int mptest_sym_build_cstr(mptest_sym_build* build, const char* cstr);
MPTEST_API int mptest_sym_build_num(mptest_sym_build* build, mptest_int32 num);
MPTEST_API int mptest_sym_build_type(mptest_sym_build* build, const char* type);

MPTEST_API void mptest_sym_walk_init(mptest_sym_walk* walk, const mptest_sym* sym, mptest_int32 parent_ref, mptest_int32 prev_child_ref);
MPTEST_API int mptest_sym_walk_getexpr(mptest_sym_walk* walk, mptest_sym_walk* sub);
MPTEST_API int mptest_sym_walk_getstr(mptest_sym_walk* walk, const char** str, mptest_size* str_size);
MPTEST_API int mptest_sym_walk_getnum(mptest_sym_walk* walk, mptest_int32* num);
MPTEST_API int mptest_sym_walk_checktype(mptest_sym_walk* walk, const char* expected_type);
MPTEST_API int mptest_sym_walk_hasmore(mptest_sym_walk* walk);
MPTEST_API int mptest_sym_walk_peekstr(mptest_sym_walk* walk);
MPTEST_API int mptest_sym_walk_peeknum(mptest_sym_walk* walk);
MPTEST_API int mptest_sym_walk_peekexpr(mptest_sym_walk* walk);

MPTEST_API int mptest__sym_check_init(mptest_sym_build* build_out, const char* str, const char* file, int line, const char* msg);
MPTEST_API int mptest__sym_check(const char* file, int line, const char* msg);
MPTEST_API void mptest__sym_check_destroy(void);
MPTEST_API int mptest__sym_make_init(mptest_sym_build* build_out, mptest_sym_walk* walk_out, const char* str, const char* file, int line, const char* msg);
MPTEST_API void mptest__sym_make_destroy(mptest_sym_build* build_out);

#define MPTEST__SYM_NONE (-1)

#define ASSERT_SYMEQm(type, in_var, chexpr, msg) \
    do { \
        mptest_sym_build temp_build; \
        if (mptest__sym_check_init(&temp_build, chexpr, __FILE__, __LINE__, msg)) { \
            return MPTEST__RESULT_ERROR; \
        } \
        if (type ## _to_sym(&temp_build,in_var)) { \
            return MPTEST__RESULT_ERROR; \
        } \
        if (mptest__sym_check(__FILE__, __LINE__, msg)) { \
            return MPTEST__RESULT_FAIL; \
        } \
        mptest__sym_check_destroy(); \
    } while (0);

#define ASSERT_SYMEQ(type, var, chexpr) \
    ASSERT_SYMEQm(type, var, chexpr, #chexpr)

#define SYM_PUT_TYPE(build, type) \
    do { \
        int _sym_err; \
        if ((_sym_err = mptest_sym_build_cstr(build, (type)))) { \
            return _sym_err; \
        } \
    } while (0)

#define SYM_PUT_NUM(build, num) \
    do { \
        int _sym_err; \
        if ((_sym_err = mptest_sym_build_num(build, (num)))) { \
            return _sym_err; \
        } \
    } while (0)

#define SYM_PUT_STR(build, str) \
    do { \
        int _sym_err; \
        if ((_sym_err = mptest_sym_build_cstr(build, (str)))) { \
            return _sym_err; \
        } \
    } while (0)

#define SYM_PUT_STRN(build, str, str_size) \
    do { \
        int _sym_err; \
        if ((_sym_err = mptest_sym_build_str(build, (str), (str_size)))) { \
            return _sym_err; \
        } \
    } while (0)

#define SYM_PUT_EXPR(build, new_build) \
    do { \
        int _sym_err; \
        if ((_sym_err = mptest_sym_build_expr(build, new_build))) { \
            return _sym_err; \
        } \
    } while (0)

#define SYM_PUT_SUB(build, type, in_var) \
    do { \
        int _sym_err; \
        if ((_sym_err = type ## _to_sym ((build), in_var))) { \
            return _sym_err; \
        } \
    } while (0)

#define SYM(type, str, out_var) \
    do { \
        mptest_sym_build temp_build; \
        mptest_sym_walk temp_walk; \
        if (mptest__sym_make_init(&temp_build, &temp_walk, str, __FILE__, __LINE__, MPTEST_NULL)) { \
            return MPTEST__RESULT_ERROR; \
        } \
        if (type ## _from_sym(&temp_walk, out_var)) { \
            return MPTEST__RESULT_ERROR; \
        } \
        mptest__sym_make_destroy(&temp_build); \
    } while (0)

#define SYM_CHECK_TYPE(walk, type_name) \
    do { \
        int _sym_err; \
        if ((_sym_err = mptest_sym_walk_checktype(walk, type_name))) { \
            return _sym_err; \
        } \
    } while (0)

#define SYM_GET_STR(walk, str_out, size_out) \
    do { \
        int _sym_err; \
        if ((_sym_err = mptest_sym_walk_getstr(walk, str_out, size_out))) { \
            return _sym_err; \
        } \
    } while (0)

#define SYM_GET_NUM(walk, num_out) \
    do { \
        int _sym_err; \
        if ((_sym_err = mptest_sym_walk_getnum(walk, num_out))) { \
            return _sym_err; \
        } \
    } while (0)

#define SYM_GET_EXPR(walk, walk_out) \
    do { \
        int _sym_err; \
        if ((_sym_err = mptest_sym_walk_getexpr(walk, walk_out))) { \
            return _sym_err; \
        } \
    } while (0)

#define SYM_GET_SUB(walk, type, out_var) \
    do { \
        int _sym_err; \
        if ((_sym_err = type ## _from_sym((walk), (out_var)))) { \
            return _sym_err; \
        } \
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

#endif /* #if MPTEST_USE_SYM */

#endif /* #ifndef MPTEST_API_H */

#ifdef MPTEST_IMPLEMENTATION


#define MPTEST_INTERNAL static

#if MPTEST_USE_DYN_ALLOC
#endif

#if MPTEST_DEBUG
#include <stdio.h>
#define MPTEST__ASSERT_UNREACHED() MPTEST_ASSERT(0)
#else
#define MPTEST__ASSERT_UNREACHED() (void)(0)
#endif

typedef char mptest_char;

#if MPTEST_USE_SYM
#if MPTEST_USE_DYN_ALLOC
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

#endif /* #if MPTEST_USE_DYN_ALLOC */
#endif /* #if MPTEST_USE_SYM */

#if MPTEST_USE_SYM
#if MPTEST_USE_DYN_ALLOC
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
#endif /* #if MPTEST_USE_DYN_ALLOC */
#endif /* #if MPTEST_USE_SYM */

#define MPTEST__P2(a, b) a ## b
#define MPTEST__P3(a, b, c) a ## b ## c
#define MPTEST__P4(a, b, c, d) a ## b ## c ## d

#if MPTEST_USE_SYM
#if MPTEST_USE_DYN_ALLOC
#define MPTEST_VEC_TYPE(T) \
    MPTEST__P2(T, _vec)

#define MPTEST_VEC_IDENT(T, name) \
    MPTEST__P3(T, _vec_, name)

#define MPTEST_VEC_IDENT_INTERNAL(T, name) \
    MPTEST__P3(T, _vec__, name)

#define MPTEST_VEC_DECL_FUNC(T, func) \
    MPTEST__P2(MPTEST__VEC_DECL_, func)(T)

#define MPTEST_VEC_IMPL_FUNC(T, func) \
    MPTEST__P2(MPTEST__VEC_IMPL_, func)(T)

#if MPTEST_DEBUG

#define MPTEST_VEC_CHECK(vec) \
    do { \
        /* ensure size is not greater than allocation size */ \
        MPTEST_ASSERT(vec->_size <= vec->_alloc); \
        /* ensure that data is not null if size is greater than 0 */ \
        MPTEST_ASSERT(vec->_size ? vec->_data != NULL : 1); \
    } while (0)

#else

#define MPTEST_VEC_CHECK(vec) MPTEST_UNUSED(vec)

#endif /* #if MPTEST_DEBUG */

#define MPTEST_VEC_DECL(T) \
    typedef struct MPTEST_VEC_TYPE(T) { \
        mptest_size _size; \
        mptest_size _alloc; \
        T* _data; \
    } MPTEST_VEC_TYPE(T)

#define MPTEST__VEC_DECL_init(T) \
    void MPTEST_VEC_IDENT(T, init)(MPTEST_VEC_TYPE(T)* vec)

#define MPTEST__VEC_IMPL_init(T) \
    void MPTEST_VEC_IDENT(T, init)(MPTEST_VEC_TYPE(T)* vec) { \
        vec->_size = 0; \
        vec->_alloc = 0; \
        vec->_data = MPTEST_NULL; \
    } 

#define MPTEST__VEC_DECL_destroy(T) \
    void MPTEST_VEC_IDENT(T, destroy)(MPTEST_VEC_TYPE(T)* vec)

#define MPTEST__VEC_IMPL_destroy(T) \
    void MPTEST_VEC_IDENT(T, destroy)(MPTEST_VEC_TYPE(T)* vec) { \
        MPTEST_VEC_CHECK(vec); \
        if (vec->_data != MPTEST_NULL) { \
            MPTEST_FREE(vec->_data); \
        } \
    }

#define MPTEST__VEC_GROW_ONE(T, vec) \
    do { \
        vec->_size += 1; \
        if (vec->_size > vec->_alloc) { \
            if (vec->_data == MPTEST_NULL) { \
                vec->_alloc = 1; \
                vec->_data = (T*)MPTEST_MALLOC(sizeof(T) * vec->_alloc); \
                if (vec->_data == MPTEST_NULL) { \
                    return 1; \
                } \
            } else { \
                vec->_alloc *= 2; \
                vec->_data = (T*)MPTEST_REALLOC(vec->_data, sizeof(T) * vec->_alloc); \
                if (vec->_data == MPTEST_NULL) { \
                    return 1; \
                } \
            } \
        } \
    } while (0)

#define MPTEST__VEC_GROW(T, vec, n) \
    do { \
        vec->_size += n; \
        if (vec->_size > vec->_alloc) { \
            vec->_alloc = vec->_size + (vec->_size >> 1); \
            if (vec->_data == MPTEST_NULL) { \
                vec->_data = (T*)MPTEST_MALLOC(sizeof(T) * vec->_alloc); \
            } else { \
                vec->_data = (T*)MPTEST_REALLOC(vec->_data, sizeof(T) * vec->_alloc); \
            } \
            if (vec->_data == MPTEST_NULL) { \
                return 1; \
            } \
        } \
    } while (0)

#define MPTEST__VEC_SETSIZE(T, vec, n) \
    do { \
        if (vec->_alloc < n) { \
            vec->_alloc = n; \
            if (vec->_data == MPTEST_NULL) { \
                vec->_data = (T*)MPTEST_MALLOC(sizeof(T) * vec->_alloc); \
            } else { \
                vec->_data = (T*)MPTEST_REALLOC(vec->_data, sizeof(T) * vec->_alloc); \
            } \
            if (vec->_data == MPTEST_NULL) { \
                return 1; \
            } \
        } \
    } while (0)

#define MPTEST__VEC_DECL_push(T) \
    int MPTEST_VEC_IDENT(T, push)(MPTEST_VEC_TYPE(T)* vec, T elem)

#define MPTEST__VEC_IMPL_push(T) \
    int MPTEST_VEC_IDENT(T, push)(MPTEST_VEC_TYPE(T)* vec, T elem) { \
        MPTEST_VEC_CHECK(vec); \
        MPTEST__VEC_GROW_ONE(T, vec); \
        vec->_data[vec->_size - 1] = elem; \
        MPTEST_VEC_CHECK(vec); \
        return 0; \
    }

#if MPTEST_DEBUG

#define MPTEST_VEC_CHECK_POP(vec) \
    do { \
        /* ensure that there is an element to pop */ \
        MPTEST_ASSERT(vec->_size > 0); \
    } while (0)

#else

#define MPTEST_VEC_CHECK_POP(vec) MPTEST_UNUSED(vec)

#endif /* #if MPTEST_DEBUG */

#define MPTEST__VEC_DECL_pop(T) \
    T MPTEST_VEC_IDENT(T, pop)(MPTEST_VEC_TYPE(T)* vec)

#define MPTEST__VEC_IMPL_pop(T) \
    T MPTEST_VEC_IDENT(T, pop)(MPTEST_VEC_TYPE(T)* vec) { \
        MPTEST_VEC_CHECK(vec); \
        MPTEST_VEC_CHECK_POP(vec); \
        return vec->_data[--vec->_size]; \
    }

#define MPTEST__VEC_DECL_cat(T) \
    T MPTEST_VEC_IDENT(T, cat)(MPTEST_VEC_TYPE(T)* vec, MPTEST_VEC_TYPE(T)* other)

#define MPTEST__VEC_IMPL_cat(T) \
    int MPTEST_VEC_IDENT(T, cat)(MPTEST_VEC_TYPE(T)* vec, MPTEST_VEC_TYPE(T)* other) { \
        re_size i; \
        re_size old_size = vec->_size; \
        MPTEST_VEC_CHECK(vec); \
        MPTEST_VEC_CHECK(other); \
        MPTEST__VEC_GROW(T, vec, other->_size); \
        for (i = 0; i < other->_size; i++) { \
            vec->_data[old_size + i] = other->_data[i]; \
        } \
        MPTEST_VEC_CHECK(vec); \
        return 0; \
    }

#define MPTEST__VEC_DECL_insert(T) \
    int MPTEST_VEC_IDENT(T, insert)(MPTEST_VEC_TYPE(T)* vec, mptest_size index, T elem)

#define MPTEST__VEC_IMPL_insert(T) \
    int MPTEST_VEC_IDENT(T, insert)(MPTEST_VEC_TYPE(T)* vec, mptest_size index, T elem) { \
        mptest_size i; \
        mptest_size old_size = vec->_size; \
        MPTEST_VEC_CHECK(vec); \
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
    T MPTEST_VEC_IDENT(T, peek)(const MPTEST_VEC_TYPE(T)* vec)

#define MPTEST__VEC_IMPL_peek(T) \
    T MPTEST_VEC_IDENT(T, peek)(const MPTEST_VEC_TYPE(T)* vec) { \
        MPTEST_VEC_CHECK(vec); \
        MPTEST_VEC_CHECK_POP(vec); \
        return vec->_data[vec->_size - 1]; \
    }

#define MPTEST__VEC_DECL_clear(T) \
    void MPTEST_VEC_IDENT(T, clear)(MPTEST_VEC_TYPE(T)* vec)

#define MPTEST__VEC_IMPL_clear(T) \
    void MPTEST_VEC_IDENT(T, clear)(MPTEST_VEC_TYPE(T)* vec) { \
        MPTEST_VEC_CHECK(vec); \
        vec->_size = 0; \
    }

#define MPTEST__VEC_DECL_size(T) \
    mptest_size MPTEST_VEC_IDENT(T, size)(const MPTEST_VEC_TYPE(T)* vec)

#define MPTEST__VEC_IMPL_size(T) \
    mptest_size MPTEST_VEC_IDENT(T, size)(const MPTEST_VEC_TYPE(T)* vec) { \
        return vec->_size; \
    }

#if MPTEST_DEBUG

#define MPTEST_VEC_CHECK_BOUNDS(vec, idx) \
    do { \
        /* ensure that idx is within bounds */ \
        MPTEST_ASSERT(idx < vec->_size); \
    } while (0)

#else

#define MPTEST_VEC_CHECK_BOUNDS(vec, idx) \
    do { \
        MPTEST_UNUSED(vec); \
        MPTEST_UNUSED(idx); \
    } while (0) 

#endif /* #if MPTEST_DEBUG */

#define MPTEST__VEC_DECL_get(T) \
    T MPTEST_VEC_IDENT(T, get)(const MPTEST_VEC_TYPE(T)* vec, mptest_size idx)

#define MPTEST__VEC_IMPL_get(T) \
    T MPTEST_VEC_IDENT(T, get)(const MPTEST_VEC_TYPE(T)* vec, mptest_size idx) { \
        MPTEST_VEC_CHECK(vec); \
        MPTEST_VEC_CHECK_BOUNDS(vec, idx); \
        return vec->_data[idx]; \
    }

#define MPTEST__VEC_DECL_getref(T) \
    T* MPTEST_VEC_IDENT(T, getref)(MPTEST_VEC_TYPE(T)* vec, mptest_size idx)

#define MPTEST__VEC_IMPL_getref(T) \
    T* MPTEST_VEC_IDENT(T, getref)(MPTEST_VEC_TYPE(T)* vec, mptest_size idx) { \
        MPTEST_VEC_CHECK(vec); \
        MPTEST_VEC_CHECK_BOUNDS(vec, idx); \
        return &vec->_data[idx]; \
    }

#define MPTEST__VEC_DECL_getcref(T) \
    const T* MPTEST_VEC_IDENT(T, getcref)(const MPTEST_VEC_TYPE(T)* vec, mptest_size idx)

#define MPTEST__VEC_IMPL_getcref(T) \
    const T* MPTEST_VEC_IDENT(T, getcref)(const MPTEST_VEC_TYPE(T)* vec, mptest_size idx) { \
        MPTEST_VEC_CHECK(vec); \
        MPTEST_VEC_CHECK_BOUNDS(vec, idx); \
        return &vec->_data[idx]; \
    }

#define MPTEST__VEC_DECL_set(T) \
    void MPTEST_VEC_IDENT(T, set)(MPTEST_VEC_TYPE(T)* vec, mptest_size idx, T elem)

#define MPTEST__VEC_IMPL_set(T) \
    void MPTEST_VEC_IDENT(T, set)(MPTEST_VEC_TYPE(T)* vec, mptest_size idx, T elem) { \
        MPTEST_VEC_CHECK(vec); \
        MPTEST_VEC_CHECK_BOUNDS(vec, idx); \
        vec->_data[idx] = elem; \
    }

#define MPTEST__VEC_DECL_capacity(T) \
    mptest_size MPTEST_VEC_IDENT(T, capacity)(MPTEST_VEC_TYPE(T)* vec)

#define MPTEST__VEC_IMPL_capacity(T) \
    mptest_size MPTEST_VEC_IDENT(T, capacity)(MPTEST_VEC_TYPE(T)* vec) { \
        return vec->_alloc; \
    }

#define MPTEST__VEC_DECL_get_data(T) \
    const T* MPTEST_VEC_IDENT(T, get_data)(const MPTEST_VEC_TYPE(T)* vec)

#define MPTEST__VEC_IMPL_get_data(T) \
    const T* MPTEST_VEC_IDENT(T, get_data)(const MPTEST_VEC_TYPE(T)* vec) { \
        return vec->_data; \
    }

#define MPTEST__VEC_DECL_move(T) \
    void MPTEST_VEC_IDENT(T, move)(MPTEST_VEC_TYPE(T)* vec, MPTEST_VEC_TYPE(T)* old);

#define MPTEST__VEC_IMPL_move(T) \
    void MPTEST_VEC_IDENT(T, move)(MPTEST_VEC_TYPE(T)* vec, MPTEST_VEC_TYPE(T)* old) { \
        MPTEST_VEC_CHECK(old); \
        *vec = *old; \
        MPTEST_VEC_IDENT(T, init)(old); \
    }

#define MPTEST__VEC_DECL_reserve(T) \
    int MPTEST_VEC_IDENT(T, reserve)(MPTEST_VEC_TYPE(T)* vec, mptest_size cap);

#define MPTEST__VEC_IMPL_reserve(T) \
    int MPTEST_VEC_IDENT(T, reserve)(MPTEST_VEC_TYPE(T)* vec, mptest_size cap) { \
        MPTEST_VEC_CHECK(vec); \
        MPTEST__VEC_SETSIZE(T, vec, cap); \
        return 0; \
    }

#endif /* #if MPTEST_USE_DYN_ALLOC */
#endif /* #if MPTEST_USE_SYM */

#ifndef MPTEST_INTERNAL_H
#define MPTEST_INTERNAL_H


#include <stdio.h>

MPTEST_INTERNAL enum mptest__result mptest__state_run_test(struct mptest__state* state, mptest__test_func test_func);
MPTEST_INTERNAL void mptest__state_print_indent(struct mptest__state* state);
#if MPTEST_USE_LONGJMP

MPTEST_INTERNAL void mptest__longjmp_init(struct mptest__state* state);
MPTEST_INTERNAL void mptest__longjmp_destroy(struct mptest__state* state);

#endif

#if MPTEST_USE_LEAKCHECK
    /* Number of guard bytes to put at the top of each block. */
    #define MPTEST__LEAKCHECK_GUARD_BYTES_COUNT 16

/* Flags kept for each block. */
enum mptest__leakcheck_block_flags
{
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
    int         line;
};

    #define MPTEST__LEAKCHECK_HEADER_SIZEOF                                   \
        (sizeof(struct mptest__leakcheck_header))

MPTEST_INTERNAL void mptest__leakcheck_init(struct mptest__state* state);
MPTEST_INTERNAL void mptest__leakcheck_destroy(struct mptest__state* state);
MPTEST_INTERNAL void mptest__leakcheck_reset(struct mptest__state* state);
MPTEST_INTERNAL int mptest__leakcheck_has_leaks(struct mptest__state* state);
MPTEST_INTERNAL int mptest__leakcheck_block_has_freeable(
    struct mptest__leakcheck_block* block);
#endif /* #if MPTEST_USE_LEAKCHECK */

#if MPTEST_USE_COLOR
    #define MPTEST__COLOR_PASS       "\x1b[1;32m" /* Pass messages */
    #define MPTEST__COLOR_FAIL       "\x1b[1;31m" /* Fail messages */
    #define MPTEST__COLOR_TEST_NAME  "\x1b[1;36m" /* Test names */
    #define MPTEST__COLOR_SUITE_NAME "\x1b[1;35m" /* Suite names */
    #define MPTEST__COLOR_EMPHASIS   "\x1b[1m"    /* Important numbers */
    #define MPTEST__COLOR_RESET      "\x1b[0m"    /* Regular text */
#else
    #define MPTEST__COLOR_PASS       ""
    #define MPTEST__COLOR_FAIL       ""
    #define MPTEST__COLOR_TEST_NAME  ""
    #define MPTEST__COLOR_SUITE_NAME ""
    #define MPTEST__COLOR_EMPHASIS   ""
    #define MPTEST__COLOR_RESET      ""
#endif /* #if MPTEST_USE_COLOR */

#if MPTEST_USE_TIME
MPTEST_INTERNAL void mptest__time_init(struct mptest__state* state);
MPTEST_INTERNAL void mptest__time_destroy(struct mptest__state* state);
#endif

#if MPTEST_USE_APARSE
MPTEST_INTERNAL void mptest__aparse_init(struct mptest__state* state);
MPTEST_INTERNAL void mptest__aparse_destroy(struct mptest__state* state);
#endif

#if MPTEST_USE_FUZZ
MPTEST_INTERNAL void mptest__fuzz_init(struct mptest__state* state);
#endif

#if MPTEST_USE_SYM
MPTEST_INTERNAL void mptest__sym_dump(mptest_sym* sym, mptest_int32 parent_ref, mptest_int32 indent);
#endif

#endif /* #ifndef MPTEST_INTERNAL_H */



#if MPTEST_USE_APARSE

MPTEST_INTERNAL const char* mptest__aparse_help = "Runs tests.";

MPTEST_INTERNAL const char* mptest__aparse_version = MPTEST_VERSION_STRING;

MPTEST_INTERNAL void mptest__aparse_init(struct mptest__state* state)
{
    aparse_state* aparse = &state->aparse;
    aparse_init_fixed(aparse, state->aparse_args, MPTEST__APARSE_ARG_COUNT);

    state->opt_test_name = NULL;
    aparse_add_opt(aparse, 't', "test");
    aparse_type_string(aparse, &state->opt_test_name);
    aparse_desc(aparse, "Runs tests that match the substring TEST_NAME");
    aparse_metavar(aparse, "TEST_NAME");

    aparse_add_opt(aparse, 'h', "help");
    aparse_type_help(aparse, mptest__aparse_help);

    aparse_add_opt(aparse, 0, "version");
    aparse_type_version(aparse, mptest__aparse_version);
}

MPTEST_INTERNAL void mptest__aparse_destroy(struct mptest__state* state)
{
    aparse_destroy(&state->aparse);
}

MPTEST_API int mptest__state_init_argv(struct mptest__state* state,
    int argc, char const* const* argv)
{
    mptest__state_init(state);
    return aparse_parse(&state->aparse, argc, argv);
}

MPTEST_INTERNAL int mptest__test_name_match(const char* test_name,
    const char*                                          match_name)
{
    const char* test_name_ptr  = test_name;
    const char* match_name_ptr = match_name;
    while ((*test_name_ptr != '\0') && (*match_name_ptr != '\0')) {
        if (*test_name_ptr == *match_name_ptr) {
            match_name_ptr++;
        } else {
            match_name_ptr = match_name;
        }
        test_name_ptr++;
    }
    if (!*match_name_ptr) {
        return 1;
    }
    return 0;
}

#endif /* #if MPTEST_USE_APARSE */


#if MPTEST_USE_FUZZ

MPTEST_INTERNAL void mptest__fuzz_init(struct mptest__state* state) {
    state->rand_state = 0xDEADBEEF;
    state->fuzz_active = 0;
    state->fuzz_iterations = 1;
    state->fuzz_fail_iteration = 0;
    state->fuzz_fail_seed = 0;
}

MPTEST_API mptest_rand mptest__fuzz_rand(struct mptest__state* state) {
    /* ANSI C LCG (wikipedia) */
    static const mptest_rand a = 1103515245;
    static const mptest_rand m = ((mptest_rand)1) << 31;
    static const mptest_rand c = 12345;
    return (state->rand_state = ((a * state->rand_state + c) % m) & 0xFFFFFFFF);
}

MPTEST_INTERNAL enum mptest__result mptest__fuzz_run_test(struct mptest__state* state, mptest__test_func test_func) {
    int i = 0;
    int iters = 1;
    enum mptest__result res;
    /* Reset fail variables */
    state->fuzz_fail_iteration = 0;
    state->fuzz_fail_seed = 0;
    if (state->fuzz_active) {
        iters = state->fuzz_iterations;
    }
    for (i = 0; i < iters; i++) {
        /* Save the start state */
        mptest_rand start_state = state->rand_state;
        int should_finish = 0;
        res = mptest__state_run_test(state, test_func);
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
            state->fuzz_fail_iteration = i;
            state->fuzz_fail_seed = start_state;
            state->fuzz_failed = 1;
            break;
        }
    }
    state->fuzz_active = 0;
    return res;
}

MPTEST_INTERNAL void mptest__fuzz_print(struct mptest__state* state) {
    if (state->fuzz_failed) {
        printf("\n");
        mptest__state_print_indent(state);
        printf("    ...on iteration %i with seed %lX", state->fuzz_fail_iteration, state->fuzz_fail_seed);
    }
    state->fuzz_failed = 0;
    /* Reset fuzz iterations, needs to be done after every fuzzed test */
    state->fuzz_iterations = 1;
}

#endif /* #if MPTEST_USE_FUZZ */


#if MPTEST_USE_LEAKCHECK

/* Set the guard bytes in `header`. */
MPTEST_INTERNAL void mptest__leakcheck_header_set_guard(
    struct mptest__leakcheck_header* header)
{
    /* Currently we choose 0xCC as the guard byte, it's a stripe of ones and
     * zeroes that looks like 11001100b */
    size_t i;
    for (i = 0; i < MPTEST__LEAKCHECK_GUARD_BYTES_COUNT; i++) {
        header->guard_bytes[i] = 0xCC;
    }
}

/* Ensure that `header` has valid guard bytes. */
MPTEST_INTERNAL int mptest__leakcheck_header_check_guard(
    struct mptest__leakcheck_header* header)
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
MPTEST_INTERNAL int mptest__leakcheck_block_has_freeable(
    struct mptest__leakcheck_block* block)
{
    /* We can free the pointer if it was not freed or was not reallocated
     * earlier. */
    return !(block->flags
             & (MPTEST__LEAKCHECK_BLOCK_FLAG_FREED
                 | MPTEST__LEAKCHECK_BLOCK_FLAG_REALLOC_OLD));
}

/* Initialize a `struct mptest__leakcheck_block`.
 * If `prev` is NULL, then this function will not attempt to link `block` to
 * any previous element in the malloc linked list. */
MPTEST_INTERNAL void mptest__leakcheck_block_init(
    struct mptest__leakcheck_block* block, size_t size,
    struct mptest__leakcheck_block*    prev,
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
    block->flags        = flags;
    /* Save source info */
    block->file = file;
    block->line = line;
}

/* Link a block to its respective header. */
MPTEST_INTERNAL void mptest__leakcheck_block_link_header(
    struct mptest__leakcheck_block*  block,
    struct mptest__leakcheck_header* header)
{
    block->header = header;
    header->block = block;
}

/* Initialize malloc-checking state. */
MPTEST_INTERNAL void mptest__leakcheck_init(struct mptest__state* state)
{
    state->test_leak_checking = 0;
    state->first_block        = NULL;
    state->top_block          = NULL;
    state->total_allocations = 0;
}

/* Destroy malloc-checking state. */
MPTEST_INTERNAL void mptest__leakcheck_destroy(struct mptest__state* state)
{
    /* Walk the malloc list, destroying everything */
    struct mptest__leakcheck_block* current = state->first_block;
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
    int test_leak_checking = state->test_leak_checking;
    mptest__leakcheck_destroy(state);
    mptest__leakcheck_init(state);
    state->test_leak_checking = test_leak_checking;
}

/* Check the block record for leaks, returning 1 if there are any. */
MPTEST_INTERNAL int mptest__leakcheck_has_leaks(struct mptest__state* state)
{
    struct mptest__leakcheck_block* current = state->first_block;
    while (current) {
        if (mptest__leakcheck_block_has_freeable(current)) {
            return 1;
        }
        current = current->next;
    }
    return 0;
}

MPTEST_API void* mptest__leakcheck_hook_malloc(struct mptest__state* state,
    const char* file, int line, size_t size)
{
    /* Header + actual memory block */
    char* base_ptr;
    /* Identical to `base_ptr` */
    struct mptest__leakcheck_header* header;
    /* Current block*/
    struct mptest__leakcheck_block* block_info;
    /* Pointer to return to the user */
    char* out_ptr;
    /* Allocate the memory the user requested + space for the header */
    base_ptr = (char*)MPTEST_MALLOC(size + MPTEST__LEAKCHECK_HEADER_SIZEOF);
    if (base_ptr == NULL) {
        state->fail_data.memory_block = NULL;
        mptest__longjmp_exec(state,
            MPTEST__LONGJMP_REASON_MALLOC_REALLY_RETURNED_NULL, file, line,
            NULL);
    }
    /* Allocate memory for the block_info structure */
    block_info = (struct mptest__leakcheck_block*)MPTEST_MALLOC(
        sizeof(struct mptest__leakcheck_block));
    if (block_info == NULL) {
        state->fail_data.memory_block = NULL;
        mptest__longjmp_exec(state,
            MPTEST__LONGJMP_REASON_MALLOC_REALLY_RETURNED_NULL, file, line,
            NULL);
    }
    /* Setup the header */
    header = (struct mptest__leakcheck_header*)base_ptr;
    mptest__leakcheck_header_set_guard(header);
    /* Setup the block_info */
    if (state->first_block == NULL) {
        /* If `state->first_block == NULL`, then this is the first allocation.
         * Use NULL as the previous value, and then set the `first_block` and
         * `top_block` to the new block. */
        mptest__leakcheck_block_init(block_info, size, NULL,
            MPTEST__LEAKCHECK_BLOCK_FLAG_INITIAL, file, line);
        state->first_block = block_info;
        state->top_block   = block_info;
    } else {
        /* If this isn't the first allocation, use `state->top_block` as the
         * previous block. */
        mptest__leakcheck_block_init(block_info, size, state->top_block,
            MPTEST__LEAKCHECK_BLOCK_FLAG_INITIAL, file, line);
        state->top_block = block_info;
    }
    /* Link the header and block_info together */
    mptest__leakcheck_block_link_header(block_info, header);
    /* Return the base pointer offset by the header amount */
    out_ptr = base_ptr + MPTEST__LEAKCHECK_HEADER_SIZEOF;
    /* Increment the total number of allocations */
    state->total_allocations++;
    return out_ptr;
}

MPTEST_API void mptest__leakcheck_hook_free(struct mptest__state* state,
    const char* file, int line, void* ptr)
{
    struct mptest__leakcheck_header* header;
    struct mptest__leakcheck_block*  block_info;
    if (ptr == NULL) {
        state->fail_data.memory_block = NULL;
        mptest__longjmp_exec(state, MPTEST__LONGJMP_REASON_FREE_OF_NULL, file,
            line, NULL);
    }
    /* Retrieve header by subtracting header size from pointer */
    header = (struct
        mptest__leakcheck_header*)((char*)ptr
                                   - MPTEST__LEAKCHECK_HEADER_SIZEOF);
    /* TODO: check for SIGSEGV here */
    if (!mptest__leakcheck_header_check_guard(header)) {
        state->fail_data.memory_block = NULL;
        mptest__longjmp_exec(state, MPTEST__LONGJMP_REASON_FREE_OF_INVALID,
            file, line, NULL);
    }
    block_info = header->block;
    /* Ensure that the pointer has not been freed or reallocated already */
    if (block_info->flags & MPTEST__LEAKCHECK_BLOCK_FLAG_FREED) {
        state->fail_data.memory_block = NULL;
        mptest__longjmp_exec(state, MPTEST__LONGJMP_REASON_FREE_OF_FREED, file,
            line, NULL);
    }
    if (block_info->flags & MPTEST__LEAKCHECK_BLOCK_FLAG_REALLOC_OLD) {
        state->fail_data.memory_block = NULL;
        mptest__longjmp_exec(state, MPTEST__LONGJMP_REASON_FREE_OF_REALLOCED,
            file, line, NULL);
    }
    /* We can finally `free()` the pointer */
    MPTEST_FREE(header);
    block_info->flags |= MPTEST__LEAKCHECK_BLOCK_FLAG_FREED;
    /* Decrement the total number of allocations */
    state->total_allocations--;
}

MPTEST_API void* mptest__leakcheck_hook_realloc(struct mptest__state* state,
    const char* file, int line, void* old_ptr, size_t new_size)
{
    /* New header + memory */
    char*                           base_ptr;
    struct mptest__leakcheck_header* old_header
        = (struct mptest__leakcheck_header*)((char*)old_ptr
                                            - MPTEST__LEAKCHECK_HEADER_SIZEOF);
    struct mptest__leakcheck_header* new_header;
    struct mptest__leakcheck_block* old_block_info = old_header->block;
    struct mptest__leakcheck_block*  new_block_info;
    /* Pointer to return to the user */
    char* out_ptr;
    (void)(new_size);
    if (old_ptr == NULL) {
        state->fail_data.memory_block = NULL;
        mptest__longjmp_exec(state, MPTEST__LONGJMP_REASON_REALLOC_OF_NULL,
            file, line, NULL);
    }
    if (!mptest__leakcheck_header_check_guard(old_header)) {
        state->fail_data.memory_block = NULL;
        mptest__longjmp_exec(state, MPTEST__LONGJMP_REASON_REALLOC_OF_INVALID,
            file, line, NULL);
    }
    if (old_block_info->flags & MPTEST__LEAKCHECK_BLOCK_FLAG_FREED) {
        state->fail_data.memory_block = NULL;
        mptest__longjmp_exec(state, MPTEST__LONGJMP_REASON_REALLOC_OF_FREED,
            file, line, NULL);
    }
    if (old_block_info->flags & MPTEST__LEAKCHECK_BLOCK_FLAG_REALLOC_OLD) {
        state->fail_data.memory_block = NULL;
        mptest__longjmp_exec(state,
            MPTEST__LONGJMP_REASON_REALLOC_OF_REALLOCED, file, line, NULL);
    }
    /* Allocate the memory the user requested + space for the header */
    base_ptr = (char*)MPTEST_REALLOC(old_header,
        new_size + MPTEST__LEAKCHECK_HEADER_SIZEOF);
    if (base_ptr == NULL) {
        state->fail_data.memory_block = NULL;
        mptest__longjmp_exec(state,
            MPTEST__LONGJMP_REASON_MALLOC_REALLY_RETURNED_NULL, file, line,
            NULL);
    }
    /* Allocate memory for the new block_info structure */
    new_block_info = (struct mptest__leakcheck_block*)MPTEST_MALLOC(
        sizeof(struct mptest__leakcheck_block));
    if (new_block_info == NULL) {
        state->fail_data.memory_block = NULL;
        mptest__longjmp_exec(state,
            MPTEST__LONGJMP_REASON_MALLOC_REALLY_RETURNED_NULL, file, line,
            NULL);
    }
    /* Setup the header */
    new_header = (struct mptest__leakcheck_header*)base_ptr;
    /* Set the guard again (double bag it per se) */
    mptest__leakcheck_header_set_guard(new_header);
    /* Setup the block_info */
    if (state->first_block == NULL) {
        mptest__leakcheck_block_init(new_block_info, new_size, NULL,
            MPTEST__LEAKCHECK_BLOCK_FLAG_REALLOC_NEW, file, line);
        state->first_block = new_block_info;
        state->top_block   = new_block_info;
    } else {
        mptest__leakcheck_block_init(new_block_info, new_size,
            state->top_block, MPTEST__LEAKCHECK_BLOCK_FLAG_REALLOC_NEW, file,
            line);
        state->top_block = new_block_info;
    }
    /* Mark `old_block_info` as reallocation target */
    old_block_info->flags |= MPTEST__LEAKCHECK_BLOCK_FLAG_REALLOC_OLD;
    /* Link the block with its respective header */
    mptest__leakcheck_block_link_header(new_block_info, new_header);
    /* Finally, indicate the new allocation in the realloc chain */
    old_block_info->realloc_next = new_block_info;
    new_block_info->realloc_prev = old_block_info;
    out_ptr                      = base_ptr + MPTEST__LEAKCHECK_HEADER_SIZEOF;
    return out_ptr;
}

#endif /* #if MPTEST_USE_LEAKCHECK */


#if MPTEST_USE_LONGJMP
/* Initialize longjmp state. */
MPTEST_INTERNAL void mptest__longjmp_init(struct mptest__state* state)
{
    state->longjmp_checking = MPTEST__LONGJMP_REASON_NONE;
    state->longjmp_reason   = MPTEST__LONGJMP_REASON_NONE;
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
MPTEST_API void mptest__longjmp_exec(struct mptest__state* state,
    enum mptest__longjmp_reason reason, const char* file, int line, const char* msg)
{
    state->longjmp_reason = reason;
    if (reason & state->longjmp_checking) {
        MPTEST_LONGJMP(state->longjmp_assert_context, 1);
    } else {
        state->fail_file = file;
        state->fail_line = line;
        state->fail_msg  = msg;
        MPTEST_LONGJMP(state->longjmp_test_context, 1);
    }
}

#else

    /* TODO: write `mptest__longjmp_exec` for when longjmp isn't on
     */

#endif /* #if MPTEST_USE_LONGJMP */


/* Initialize a test runner state. */
MPTEST_API void mptest__state_init(struct mptest__state* state)
{
    state->assertions             = 0;
    state->total                  = 0;
    state->passes                 = 0;
    state->fails                  = 0;
    state->errors                 = 0;
    state->suite_passes           = 0;
    state->suite_fails            = 0;
    state->suite_failed           = 0;
    state->suite_test_setup_cb    = NULL;
    state->suite_test_teardown_cb = NULL;
    state->current_test           = NULL;
    state->current_suite          = NULL;
    state->fail_reason            = (enum mptest__fail_reason)0;
    state->fail_msg               = NULL;
    /* we do not initialize state->fail_data */
    state->fail_file              = NULL;
    state->fail_line              = 0;
    state->indent_lvl             = 0;
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
        printf(MPTEST__COLOR_SUITE_NAME
            "%i" MPTEST__COLOR_RESET " suites: " MPTEST__COLOR_PASS
            "%i" MPTEST__COLOR_RESET " passed, " MPTEST__COLOR_FAIL
            "%i" MPTEST__COLOR_RESET " failed\n",
            state->suite_fails + state->suite_passes, state->suite_passes,
            state->suite_fails);
    }
    if (state->errors) {
        printf(MPTEST__COLOR_TEST_NAME
            "%i" MPTEST__COLOR_RESET " tests (" MPTEST__COLOR_EMPHASIS
            "%i" MPTEST__COLOR_RESET " assertions): " MPTEST__COLOR_PASS
            "%i" MPTEST__COLOR_RESET " passed, " MPTEST__COLOR_FAIL
            "%i" MPTEST__COLOR_RESET " failed, " MPTEST__COLOR_FAIL
            "%i" MPTEST__COLOR_RESET " errors",
            state->total, state->assertions, state->passes, state->fails,
            state->errors);
    } else {
        printf(MPTEST__COLOR_TEST_NAME
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
        double  elapsed_time
            = ((double)(program_end_time - state->program_start_time))
            / CLOCKS_PER_SEC;
        printf(" in %f seconds", elapsed_time);
    }
#endif /* #if MPTEST_USE_TIME */
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
    printf(MPTEST__COLOR_EMPHASIS "%s" MPTEST__COLOR_RESET
                                  ":" MPTEST__COLOR_EMPHASIS
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
MPTEST_API enum mptest__result mptest__state_before_test(
    struct mptest__state* state, mptest__test_func test_func,
    const char* test_name)
{
    enum mptest__result res;
    state->current_test = test_name;
#if MPTEST_USE_LEAKCHECK
    if (state->test_leak_checking) {
        mptest__leakcheck_reset(state);
    }
#endif
    /* indent if we are running a suite */
    mptest__state_print_indent(state);
    printf("test " MPTEST__COLOR_TEST_NAME "%s" MPTEST__COLOR_RESET "... ",
        state->current_test);
    fflush(stdout);
#if MPTEST_USE_APARSE
    if (state->opt_test_name) {
        if (!mptest__test_name_match(test_name, state->opt_test_name)) {
            return MPTEST__RESULT_SKIPPED;
        }
    }
#endif /* #if MPTEST_USE_APARSE */
#if MPTEST_USE_FUZZ
    res = mptest__fuzz_run_test(state, test_func);
#else
    res = mptest__state_run_test(state, test_func);
#endif
    return res;
}

MPTEST_INTERNAL enum mptest__result mptest__state_run_test(struct mptest__state* state, mptest__test_func test_func) {
    enum mptest__result res;
#if MPTEST_USE_LONGJMP
    if (MPTEST_SETJMP(state->longjmp_test_context) == 0) {
        res = test_func();
    } else {
        res = MPTEST__RESULT_ERROR;
    }
#else
    res = test_func();
#endif /* #if MPTEST_USE_LONGJMP */
    return res;
}

/* Ran when a test is over. */
MPTEST_API void mptest__state_after_test(struct mptest__state* state,
    enum mptest__result                                         res)
{
#if MPTEST_USE_LEAKCHECK
    int has_leaks;
    if (state->test_leak_checking == 1) {
        has_leaks = mptest__leakcheck_has_leaks(state);
        if (has_leaks) {
            if (res == MPTEST__RESULT_PASS) {
                state->fail_reason = MPTEST__FAIL_REASON_LEAKED;
            }
            res = MPTEST__RESULT_FAIL;
        }
    }
#endif /* #if MPTEST_USE_LEAKCHECK */
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
            printf("  " MPTEST__COLOR_FAIL
                   "assertion failure" MPTEST__COLOR_RESET
                   ": " MPTEST__COLOR_EMPHASIS "%s" MPTEST__COLOR_RESET "\n",
                state->fail_msg);
            /* If the message and expression are the same, don't print the
             * expression */
            if (!mptest__streq(state->fail_msg,
                    (const char*)state->fail_data.string_data)) {
                mptest__state_print_indent(state);
                printf("    expression: " MPTEST__COLOR_EMPHASIS
                       "%s" MPTEST__COLOR_RESET "\n",
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
            printf("  " MPTEST__COLOR_FAIL
                   "assertion failure: s-expression inequality" MPTEST__COLOR_RESET
                   ": " MPTEST__COLOR_EMPHASIS "%s" MPTEST__COLOR_RESET "\n",
                state->fail_msg);
            mptest__state_print_indent(state);
            printf("Actual:");
            mptest__sym_dump(state->fail_data.sym_fail_data.sym_actual, 0, state->indent_lvl);
            printf("\n");
            mptest__state_print_indent(state);
            printf("Expected:");
            mptest__sym_dump(state->fail_data.sym_fail_data.sym_expected, 0, state->indent_lvl);
            mptest__sym_check_destroy();
        } 
#endif /* #if MPTEST_USE_SYM */
#if MPTEST_USE_LEAKCHECK
        else if (state->fail_reason == MPTEST__FAIL_REASON_LEAKED || has_leaks) {
            struct mptest__leakcheck_block* current = state->first_block;
            mptest__state_print_indent(state);
            printf("  " MPTEST__COLOR_FAIL
                   "memory leak(s) detected" MPTEST__COLOR_RESET ":\n");
            while (current) {
                if (mptest__leakcheck_block_has_freeable(current)) {
                    mptest__state_print_indent(state);
                    printf("    " MPTEST__COLOR_FAIL "leak" MPTEST__COLOR_RESET
                           " of " MPTEST__COLOR_EMPHASIS
                           "%lu" MPTEST__COLOR_RESET
                           " bytes at " MPTEST__COLOR_EMPHASIS
                           "%p" MPTEST__COLOR_RESET ":\n",
                        (long unsigned int)current->block_size,
                        (void*)current->header);
                    mptest__state_print_indent(state);
                    if (current->flags
                        & MPTEST__LEAKCHECK_BLOCK_FLAG_INITIAL) {
                        printf("      allocated with " MPTEST__COLOR_EMPHASIS
                               "malloc()" MPTEST__COLOR_RESET "\n");
                    } else if (current->flags
                               & MPTEST__LEAKCHECK_BLOCK_FLAG_REALLOC_NEW) {
                        printf("      reallocated with " MPTEST__COLOR_EMPHASIS
                               "realloc()" MPTEST__COLOR_RESET ":\n");
                        printf("        ...from " MPTEST__COLOR_EMPHASIS
                               "%p" MPTEST__COLOR_RESET "\n",
                            (void*)current->realloc_prev);
                    }
                    mptest__state_print_indent(state);
                    printf("      ...at ");
                    mptest__print_source_location(current->file,
                        current->line);
                }
                current = current->next;
            }
        }
#endif /* #if MPTEST_USE_LEAKCHECK */
        printf("\n");
    }
    else if (res == MPTEST__RESULT_ERROR) {
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
            printf("  " MPTEST__COLOR_FAIL
                   "out of memory: " MPTEST__COLOR_RESET
                   ": " MPTEST__COLOR_EMPHASIS "%s" MPTEST__COLOR_RESET,
                state->fail_msg);
        }
#endif /* #if MPTEST_USE_DYN_ALLOC */
#if MPTEST_USE_SYM
        else if (state->fail_reason == MPTEST__FAIL_REASON_SYM_SYNTAX) {
            /* Sym syntax error -> show message, source, error info */
            mptest__state_print_indent(state);
            printf("  " MPTEST__COLOR_FAIL
                   "s-expression syntax error" MPTEST__COLOR_RESET
                   ": " MPTEST__COLOR_EMPHASIS "%s" MPTEST__COLOR_RESET ":"
                   MPTEST__COLOR_EMPHASIS "%s at position %i\n",
                state->fail_msg, state->fail_data.sym_syntax_error_data.err_msg,
                (int)state->fail_data.sym_syntax_error_data.err_pos);
        }
#endif /* #if MPTEST_USE_SYM */
#if MPTEST_USE_LONGJMP
        if (state->longjmp_reason == MPTEST__LONGJMP_REASON_ASSERT_FAIL) {
            mptest__state_print_indent(state);
            printf("  " MPTEST__COLOR_FAIL
                   "uncaught assertion failure" MPTEST__COLOR_RESET
                   ": " MPTEST__COLOR_EMPHASIS "%s" MPTEST__COLOR_RESET "\n",
                state->fail_msg);
            if (!mptest__streq(state->fail_msg,
                    (const char*)state->fail_data.string_data)) {
                mptest__state_print_indent(state);
                printf("    expression: " MPTEST__COLOR_EMPHASIS
                       "%s" MPTEST__COLOR_RESET "\n",
                    (const char*)state->fail_data.string_data);
            }
            mptest__state_print_indent(state);
            printf("    ...at ");
            mptest__print_source_location(state->fail_file, state->fail_line);
        }
    #if MPTEST_USE_LEAKCHECK
        else if (state->longjmp_reason
                 == MPTEST__LONGJMP_REASON_MALLOC_REALLY_RETURNED_NULL) {
            mptest__state_print_indent(state);
            printf("  " MPTEST__COLOR_FAIL "internal error: malloc() returned "
                   "a null pointer" MPTEST__COLOR_RESET "\n");
            mptest__state_print_indent(state);
            printf("    ...at ");
            mptest__print_source_location(state->fail_file, state->fail_line);
        } else if (state->longjmp_reason
                   == MPTEST__LONGJMP_REASON_REALLOC_OF_NULL) {
            mptest__state_print_indent(state);
            printf(
                "  " MPTEST__COLOR_FAIL "attempt to call realloc() on a NULL "
                "pointer" MPTEST__COLOR_RESET "\n");
            mptest__state_print_indent(state);
            printf("    ...attempt to reallocate at ");
            mptest__print_source_location(state->fail_file, state->fail_line);
        } else if (state->longjmp_reason
                   == MPTEST__LONGJMP_REASON_REALLOC_OF_INVALID) {
            mptest__state_print_indent(state);
            printf("  " MPTEST__COLOR_FAIL "attempt to call realloc() on an "
                   "invalid pointer (pointer was not "
                   "returned by malloc() or realloc())" MPTEST__COLOR_RESET
                   ":\n");
            mptest__state_print_indent(state);
            printf("    pointer: %p\n", state->fail_data.memory_block);
            mptest__state_print_indent(state);
            printf("    ...attempt to reallocate at ");
            mptest__print_source_location(state->fail_file, state->fail_line);
        } else if (state->longjmp_reason
                   == MPTEST__LONGJMP_REASON_REALLOC_OF_FREED) {
            mptest__state_print_indent(state);
            printf("  " MPTEST__COLOR_FAIL
                   "attempt to call realloc() on a pointer that was already "
                   "freed" MPTEST__COLOR_RESET ":\n");
            mptest__state_print_indent(state);
            printf("    pointer: %p\n", state->fail_data.memory_block);
            mptest__state_print_indent(state);
            printf("    ...at ");
            mptest__print_source_location(state->fail_file, state->fail_line);
        }
    #endif /*     #if MPTEST_USE_LEAKCHECK */
#endif /* #if MPTEST_USE_LONGJMP */
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
    }
    else if (res == MPTEST__RESULT_SKIPPED) {
        printf("skipped");
    }
#if MPTEST_USE_LEAKCHECK
    /* Reset leak-checking state (IMPORTANT!) */
    mptest__leakcheck_reset(state);
#endif
    if (res == MPTEST__RESULT_FAIL || res == MPTEST__RESULT_ERROR ||
        res == MPTEST__RESULT_PASS) {
        printf("\n");
    }
}

/* Ran before a suite is executed. */
MPTEST_API void mptest__state_before_suite(struct mptest__state* state,
    const char*                                                   suite_name)
{
    state->current_suite          = suite_name;
    state->suite_failed           = 0;
    state->suite_test_setup_cb    = NULL;
    state->suite_test_teardown_cb = NULL;
    mptest__state_print_indent(state);
    printf("suite " MPTEST__COLOR_SUITE_NAME "%s" MPTEST__COLOR_RESET ":\n",
        state->current_suite);
    state->indent_lvl++;
}

/* Ran after a suite is executed. */
MPTEST_API void mptest__state_after_suite(struct mptest__state* state)
{
    if (!state->suite_failed) {
        state->suite_passes++;
    } else {
        state->suite_fails++;
    }
    state->current_suite = NULL;
    state->suite_failed  = 0;
    state->indent_lvl--;
}

/* Fills state with information on failure. */
MPTEST_API void mptest__assert_do_failure(const char* msg, const char* assert_expr, const char* file, int line)
{
    mptest__state_g.fail_reason = MPTEST__FAIL_REASON_ASSERT_FAILURE;
    mptest__state_g.fail_msg    = msg;
    mptest__state_g.fail_data.string_data = assert_expr;
    mptest__state_g.fail_file   = file;
    mptest__state_g.fail_line   = line;
}

/* Dummy function to break on for assert failures */
MPTEST_API void mptest_assert_fail() {
    return;
}


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

void mptest__sym_tree_init(mptest__sym_tree* tree, mptest__sym_type type) {
    tree->type = type;
    tree->first_child_ref = MPTEST__SYM_NONE;
    tree->next_sibling_ref = MPTEST__SYM_NONE;
}

void mptest__sym_tree_destroy(mptest__sym_tree* tree) {
    if (tree->type == MPTEST__SYM_TYPE_ATOM_STRING) {
        mptest__str_destroy(&tree->data.str);
    }
}

MPTEST_VEC_DECL(mptest__sym_tree);
MPTEST_VEC_IMPL_FUNC(mptest__sym_tree, init)
MPTEST_VEC_IMPL_FUNC(mptest__sym_tree, destroy)
MPTEST_VEC_IMPL_FUNC(mptest__sym_tree, push)
MPTEST_VEC_IMPL_FUNC(mptest__sym_tree, size)
MPTEST_VEC_IMPL_FUNC(mptest__sym_tree, getref)
MPTEST_VEC_IMPL_FUNC(mptest__sym_tree, getcref)

struct mptest_sym {
    mptest__sym_tree_vec tree_storage;
};

void mptest__sym_init(mptest_sym* sym) {
    mptest__sym_tree_vec_init(&sym->tree_storage);
}

void mptest__sym_destroy(mptest_sym* sym) {
    mptest_size i;
    for (i = 0; i < mptest__sym_tree_vec_size(&sym->tree_storage); i++) {
        mptest__sym_tree_destroy(mptest__sym_tree_vec_getref(&sym->tree_storage, i));
    }
    mptest__sym_tree_vec_destroy(&sym->tree_storage);
}

MPTEST_INTERNAL mptest__sym_tree* mptest__sym_get(mptest_sym* sym, mptest_int32 ref) {
    MPTEST_ASSERT(ref != MPTEST__SYM_NONE);
    return mptest__sym_tree_vec_getref(&sym->tree_storage, (mptest_size)ref);
}

MPTEST_INTERNAL const mptest__sym_tree* mptest__sym_getcref(const mptest_sym* sym, mptest_int32 ref) {
    MPTEST_ASSERT(ref != MPTEST__SYM_NONE);
    return mptest__sym_tree_vec_getcref(&sym->tree_storage, (mptest_size)ref);
}

MPTEST_INTERNAL int mptest__sym_new(mptest_sym* sym, mptest_int32 parent_ref, mptest_int32 prev_sibling_ref, mptest__sym_tree new_tree, mptest_int32* new_ref) {
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

MPTEST_INTERNAL int mptest__sym_isblank(mptest_char ch) {
    return (ch == '\n') || (ch == '\t') || (ch == '\r') || (ch == ' ');
}

MPTEST_VEC_DECL(mptest_sym_build);
MPTEST_VEC_IMPL_FUNC(mptest_sym_build, init)
MPTEST_VEC_IMPL_FUNC(mptest_sym_build, destroy)
MPTEST_VEC_IMPL_FUNC(mptest_sym_build, push)
MPTEST_VEC_IMPL_FUNC(mptest_sym_build, pop)
MPTEST_VEC_IMPL_FUNC(mptest_sym_build, getref)

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

MPTEST_INTERNAL int mptest__sym_parse_expr_begin(mptest__sym_parse* parse) {
    int err = 0;
    mptest_sym_build next_build;
    if (parse->sym_stack_ptr == 0) {
        if ((err = mptest_sym_build_expr(parse->base_build, &next_build))) {
            return err;
        }
    } else {
        if ((err = mptest_sym_build_expr(
            mptest_sym_build_vec_getref(&parse->sym_stack, parse->sym_stack_ptr - 1), 
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

MPTEST_INTERNAL int mptest__sym_parse_expr_end(mptest__sym_parse* parse) {
    if (parse->sym_stack_ptr == 0) {
        return 1;
    }
    parse->sym_stack_ptr--;
    mptest_sym_build_vec_pop(&parse->sym_stack);
    return 0;
}

MPTEST_INTERNAL int mptest__sym_parse_esc(mptest__sym_parse* parse, mptest_char ch) {
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

MPTEST_INTERNAL int mptest__sym_do_parse(mptest_sym_build* build_in, const mptest__str_view in, const char** err_msg, mptest_size* err_pos) {
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
            current_build = mptest_sym_build_vec_getref(&parse.sym_stack, parse.sym_stack_ptr - 1);
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
            } else if (ch == '1' || ch == '2' || ch == '3' || ch == '4' ||
                       ch == '5' || ch == '6' || ch == '7' || ch == '8' ||
                       ch == '9') {
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
                if ((err = mptest_sym_build_str(current_build, (const char*)mptest__str_get_data(&parse.atom_str), mptest__str_size(&parse.atom_str)))) {
                    goto error;
                }
                state = MPTEST__SYM_PARSE_STATE_EXPR;
            } else if (ch == '(') {
                if ((err = mptest_sym_build_str(current_build, (const char*)mptest__str_get_data(&parse.atom_str), mptest__str_size(&parse.atom_str)))) {
                    goto error;
                }
                if ((err = mptest__sym_parse_expr_begin(&parse))) {
                    goto error;
                }
                state = MPTEST__SYM_PARSE_STATE_EXPR;
            } else if (ch == ')') {
                if ((err = mptest_sym_build_str(current_build, (const char*)mptest__str_get_data(&parse.atom_str), mptest__str_size(&parse.atom_str)))) {
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
            } else if (ch == '0' || ch == '1' || ch == '2' || ch == '3' ||
                       ch == '4' || ch == '5' || ch == '6' || ch == '7' || 
                       ch == '8' || ch == '9') {
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
            }else {
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
            } else if (ch == '0' || ch == '1' || ch == '2' || ch == '3' ||
                       ch == '4' || ch == '5' || ch == '6' || ch == '7' || 
                       ch == '8' || ch == '9') {
                parse.num *= 16;
                parse.num += ch - '0';
                state = MPTEST__SYM_PARSE_STATE_NUM_HEX;
            } else if (ch == 'A' || ch == 'B' || ch == 'C' || ch == 'D' ||
                       ch == 'E' || ch == 'F') {
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
                if ((err = mptest_sym_build_str(current_build, (const char*)mptest__str_get_data(&parse.atom_str), mptest__str_size(&parse.atom_str)))) {
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

MPTEST_INTERNAL void mptest__sym_dump(mptest_sym* sym, mptest_int32 parent_ref, mptest_int32 indent) {
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
            mptest__sym_dump(sym, child_ref, indent+1);
            child_ref = child->next_sibling_ref;
            if (child_ref != MPTEST__SYM_NONE) {
                printf(" ");
            }
        }
        printf(")");
    }
}

MPTEST_INTERNAL int mptest__sym_equals(mptest_sym* sym, mptest_sym* other, mptest_int32 sym_ref, mptest_int32 other_ref) {
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
                if (!mptest__sym_equals(sym, other, parent_child_ref, other_child_ref)) {
                    return 0;
                }
                parent_child = mptest__sym_get(sym,parent_child_ref);
                other_child = mptest__sym_get(other, other_child_ref);
                parent_child_ref = parent_child->next_sibling_ref;
                other_child_ref = other_child->next_sibling_ref;
            }
            return parent_child_ref == other_child_ref;
        }
        return 0;
    }
}

MPTEST_API void mptest_sym_build_init(mptest_sym_build* build, mptest_sym* sym, mptest_int32 parent_ref, mptest_int32 prev_child_ref) {
    build->sym = sym;
    build->parent_ref = parent_ref;
    build->prev_child_ref = prev_child_ref;
}

MPTEST_API void mptest_sym_build_destroy(mptest_sym_build* build) {
    MPTEST__UNUSED(build);
}

MPTEST_API int mptest_sym_build_expr(mptest_sym_build* build, mptest_sym_build* sub) {
    mptest__sym_tree new_tree;
    mptest_int32 new_child_ref;
    int err = 0;
    mptest__sym_tree_init(&new_tree, MPTEST__SYM_TYPE_EXPR);
    if ((err = mptest__sym_new(build->sym, build->parent_ref, build->prev_child_ref, new_tree, &new_child_ref))) {
        return err;
    }
    build->prev_child_ref = new_child_ref;
    mptest_sym_build_init(sub, build->sym, new_child_ref, MPTEST__SYM_NONE);
    return err;
}

MPTEST_API int mptest_sym_build_str(mptest_sym_build* build, const char* str, mptest_size str_size) {
    mptest__sym_tree new_tree;
    mptest_int32 new_child_ref;
    int err = 0;
    mptest__sym_tree_init(&new_tree, MPTEST__SYM_TYPE_ATOM_STRING);
    if ((err = mptest__str_init_n(&new_tree.data.str, (const mptest_char*)str, str_size))) {
        return err;
    }
    if ((err = mptest__sym_new(build->sym, build->parent_ref, build->prev_child_ref, new_tree, &new_child_ref))) {
        return err;
    }
    build->prev_child_ref = new_child_ref;
    return err;
}

MPTEST_API int mptest_sym_build_cstr(mptest_sym_build* build, const char* cstr) {
    return mptest_sym_build_str(build, cstr, mptest__str_slen((const mptest_char*)cstr));
}

MPTEST_API int mptest_sym_build_num(mptest_sym_build* build, mptest_int32 num) {
    mptest__sym_tree new_tree;
    mptest_int32 new_child_ref;
    int err = 0;
    mptest__sym_tree_init(&new_tree, MPTEST__SYM_TYPE_ATOM_NUMBER);
    new_tree.data.num = num;
    if ((err = mptest__sym_new(build->sym, build->parent_ref, build->prev_child_ref, new_tree, &new_child_ref))) {
        return err;
    }
    build->prev_child_ref = new_child_ref;
    return err;
}

MPTEST_API int mptest_sym_build_type(mptest_sym_build* build, const char* type) {
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

MPTEST_API void mptest_sym_walk_init(mptest_sym_walk* walk, const mptest_sym* sym, mptest_int32 parent_ref, mptest_int32 prev_child_ref) {
    walk->sym = sym;
    walk->parent_ref = parent_ref;
    walk->prev_child_ref = prev_child_ref;
}

MPTEST_API int mptest__sym_walk_peeknext(mptest_sym_walk* walk, mptest_int32* out_child_ref) {
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

MPTEST_API int mptest__sym_walk_getnext(mptest_sym_walk* walk, mptest_int32* out_child_ref) {
    int err = 0;
    if ((err = mptest__sym_walk_peeknext(walk, out_child_ref))) {
        return err;
    }
    walk->prev_child_ref = *out_child_ref;
    return err;
}

MPTEST_API int mptest_sym_walk_getexpr(mptest_sym_walk* walk, mptest_sym_walk* sub) {
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

MPTEST_API int mptest_sym_walk_getstr(mptest_sym_walk* walk, const char** str, mptest_size* str_size) {
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

MPTEST_API int mptest_sym_walk_getnum(mptest_sym_walk* walk, mptest_int32* num) {
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

MPTEST_API int mptest_sym_walk_checktype(mptest_sym_walk* walk, const char* expected_type) {
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

MPTEST_API int mptest_sym_walk_hasmore(mptest_sym_walk* walk) {
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

MPTEST_API int mptest_sym_walk_peekstr(mptest_sym_walk* walk) {
    int err = 0;
    const mptest__sym_tree* child;
    mptest_int32 child_ref;
    if ((err = mptest__sym_walk_peeknext(walk, &child_ref))) {
        return err;
    }
    child = mptest__sym_getcref(walk->sym, child_ref);
    return child->type == MPTEST__SYM_TYPE_ATOM_STRING;
}

MPTEST_API int mptest_sym_walk_peekexpr(mptest_sym_walk* walk) {
    int err = 0;
    const mptest__sym_tree* child;
    mptest_int32 child_ref;
    if ((err = mptest__sym_walk_peeknext(walk, &child_ref))) {
        return err;
    }
    child = mptest__sym_getcref(walk->sym, child_ref);
    return child->type == MPTEST__SYM_TYPE_EXPR;
}

MPTEST_API int mptest_sym_walk_peeknum(mptest_sym_walk* walk) {
    int err = 0;
    const mptest__sym_tree* child;
    mptest_int32 child_ref;
    if ((err = mptest__sym_walk_peeknext(walk, &child_ref))) {
        return err;
    }
    child = mptest__sym_getcref(walk->sym, child_ref);
    return child->type == MPTEST__SYM_TYPE_ATOM_NUMBER;
}

MPTEST_API int mptest__sym_check_init(mptest_sym_build* build_out, const char* str, const char* file, int line, const char* msg) {
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
    mptest_sym_build_init(build_out, sym_actual, MPTEST__SYM_NONE, MPTEST__SYM_NONE);
    mptest__str_view_init_n(&in_str_view, str, mptest__str_slen(str));
    mptest_sym_build_init(&parse_build, sym_expected, MPTEST__SYM_NONE, MPTEST__SYM_NONE);
    if ((err = mptest__sym_do_parse(&parse_build, in_str_view, &err_msg, &err_pos))) {
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

MPTEST_API int mptest__sym_check(const char* file, int line, const char* msg) {
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

MPTEST_API void mptest__sym_check_destroy() {
    mptest__sym_destroy(mptest__state_g.fail_data.sym_fail_data.sym_actual);
    mptest__sym_destroy(mptest__state_g.fail_data.sym_fail_data.sym_expected);
    MPTEST_FREE(mptest__state_g.fail_data.sym_fail_data.sym_actual);
    MPTEST_FREE(mptest__state_g.fail_data.sym_fail_data.sym_expected);
}


MPTEST_API int mptest__sym_make_init(mptest_sym_build* build_out, mptest_sym_walk* walk_out, const char* str, const char* file, int line, const char* msg) {
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
    if ((err = mptest__sym_do_parse(build_out, in_str_view, &err_msg, &err_pos))) {
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

MPTEST_API void mptest__sym_make_destroy(mptest_sym_build* build) {
    mptest__sym_destroy(build->sym);
    MPTEST_FREE(build->sym);
}

#endif /* #if MPTEST_USE_SYM */


#if MPTEST_USE_TIME

MPTEST_INTERNAL void mptest__time_init(struct mptest__state* state)
{
    state->program_start_time = clock();
    state->suite_start_time   = 0;
    state->test_start_time    = 0;
}

MPTEST_INTERNAL void mptest__time_destroy(struct mptest__state* state)
{
    (void)(state);
}

#endif /* #if MPTEST_USE_TIME */



int mptest__uint8_check[sizeof(mptest_uint8)==1];
int mptest__int8_check[sizeof(mptest_int8)==1];
int mptest__uint16_check[sizeof(mptest_uint16)==2];
int mptest__int16_check[sizeof(mptest_int16)==2];
int mptest__uint32_check[sizeof(mptest_uint32)==4];
int mptest__int32_check[sizeof(mptest_int32)==4];

#if MPTEST_USE_SYM
#if MPTEST_USE_DYN_ALLOC
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

#define MPTEST__STR_CHECK(str) MPTEST_UNUSED(str)

#endif /* #if MPTEST_DEBUG */

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
                return 1;
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
                return 1;
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

#endif /* #if MPTEST_USE_DYN_ALLOC */
#endif /* #if MPTEST_USE_SYM */

#if MPTEST_USE_SYM
#if MPTEST_USE_DYN_ALLOC
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
#endif /* #if MPTEST_USE_DYN_ALLOC */
#endif /* #if MPTEST_USE_SYM */

#endif /* #ifdef MPTEST_IMPLEMENTATION */

#endif /* #ifndef MPTEST_H */

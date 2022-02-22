cmake_minimum_required(VERSION 3.0.0)
project(re VERSION 0.0.1)
add_executable(re re_ast.c re_charclass.c re_compile.c re_compile_charclass.c re_exec.c re_parse.c re_prog.c re_re.c re_range.c re_debug.c pack/re__bits.c tests/test_ast.c tests/test_charclass.c tests/test_compile.c tests/test_exec.c tests/test_helpers.c tests/test_main.c tests/test_parse.c tests/test_prog.c tests/test_range.c)
set(DEBUG_OPTS 
 "-g" "-O0" "-Wall" "-Werror" "-Wextra" "-Wshadow" "-Wconversion" 
 "-Wstrict-prototypes" "-Wuninitialized" "-Wpedantic" "--std=c89")
set(RELEASE_OPTS "-O3")
target_compile_options(re PUBLIC "$<$<CONFIG:DEBUG>:${DEBUG_OPTS}>")
target_compile_options(re PUBLIC "$<$<CONFIG:RELEASE>:${RELEASE_OPTS}>")

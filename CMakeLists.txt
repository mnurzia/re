cmake_minimum_required(VERSION 3.0.0)
project(re VERSION 0.0.1)
add_executable(re re_ast.c re_charclass.c re_compile.c re_compile_charclass.c re_exec.c re_main.c re_parse.c re_prog.c re_re.c re_range.c pack/re__bits.c)
set(DEBUG_OPTS 
 "-g" "-O0" "-Wall" "-Werror" "-Wextra" "-Wshadow" "-Wconversion" 
 "-Wstrict-prototypes" "-Wuninitialized" "-Wpedantic" "--std=c89")
set(RELEASE_OPTS "-O3")
target_compile_options(re PUBLIC "$<$<CONFIG:DEBUG>:${DEBUG_OPTS}>")
target_compile_options(re PUBLIC "$<$<CONFIG:RELEASE>:${RELEASE_OPTS}>")

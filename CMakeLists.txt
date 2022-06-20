cmake_minimum_required(VERSION 3.0.0)

project(re VERSION 0.1.0)

set(
  SOURCES 
  re_ast.c 
  re_api.c 
  re_charclass.c 
  re_compile_charclass.c 
  re_compile.c 
  re_exec_dfa.c 
  re_exec_nfa.c 
  re_parse.c 
  re_prog.c 
  re_prog_data.c 
  re_rune_data.c 
  re_range.c 
  _cpack/impl.c)

set(TEST_SOURCES 
  tests/test_ast.c 
  tests/test_charclass.c 
  tests/test_compile.c 
  tests/test_compile_charclass.c 
  tests/test_exec.c
  tests/test_helpers.c 
  tests/test_main.c 
  tests/test_match.c 
  tests/test_parse.c 
  tests/test_prog.c 
  tests/test_regr.c 
  tests/test_rune_data.c 
  tests/test_range.c)

set(FUZZ_SOURCES tests/test_fuzz.c)

if (MSVC)
  set(ANY_OPTS "/W3")
  set(DEBUG_OPTS "")
  set(DEFS "MPTEST_USE_COLOR=0")
  set(RELEASE_OPTS "")
  set(COV_OPTS "")
  set(FUZZ_OPTS "")
  set(ASAN_OPTS "")
else()
  set(ANY_OPTS "-Wall" "-Werror" "-Wextra" "-Wshadow" "-Wconversion" "-Wstrict-prototypes" "-Wuninitialized" "-Wpedantic" "--std=c89")
  set(DEBUG_OPTS "-g" "-O0")
  set(DEFS "")
  set(RELEASE_OPTS "-O3")
  set(COV_OPTS "-fprofile-instr-generate" "-fcoverage-mapping")
  set(FUZZ_OPTS "-fsanitize=fuzzer,address")
  if (ASAN)
    set(ASAN_OPTS "-fsanitize=address")
  else()
    set(ASAN_OPTS "")
  endif()
endif()

add_executable(re_tests ${SOURCES} ${TEST_SOURCES})
target_compile_definitions(re_tests PUBLIC MN__SPLIT_BUILD MN_DEBUG DEFS)
target_compile_options(re_tests PUBLIC "${ANY_OPTS}")
target_compile_options(re_tests PUBLIC "$<$<CONFIG:RELEASE>:${RELEASE_OPTS}>")
target_compile_options(re_tests PUBLIC "$<$<CONFIG:DEBUG>:${DEBUG_OPTS}>")
target_compile_options(re_tests PUBLIC "$<$<CONFIG:DEBUG>:${ASAN_OPTS}>")
target_link_options(re_tests PUBLIC "${ASAN_OPTS}")

if (FUZZ)
  add_executable(re_fuzz ${SOURCES} ${FUZZ_SOURCES})
  target_compile_definitions(re_fuzz PUBLIC MN__SPLIT_BUILD MN_DEBUG DEFS)
  target_compile_options(re_fuzz PUBLIC "${ANY_OPTS}" "${FUZZ_OPTS}")
  target_compile_options(re_fuzz PUBLIC "$<$<CONFIG:RELEASE>:${RELEASE_OPTS}>")
  target_compile_options(re_fuzz PUBLIC "$<$<CONFIG:DEBUG>:${DEBUG_OPTS}>")
  target_compile_options(re_fuzz PUBLIC "$<$<CONFIG:DEBUG>:${ASAN_OPTS}>")
  target_link_options(re_fuzz PUBLIC "${ASAN_OPTS}")
endif()

if(COV)
  add_executable(re_cov ${SOURCES} ${TEST_SOURCES})
  target_compile_definitions(re_cov PUBLIC MN__SPLIT_BUILD MN_DEBUG MN__COVERAGE DEFS)
  target_compile_options(re_cov PUBLIC "${ANY_OPTS}" "${COV_OPTS}")
  target_compile_options(re_cov PUBLIC "$<$<CONFIG:RELEASE>:${RELEASE_OPTS}>")
  target_compile_options(re_cov PUBLIC "$<$<CONFIG:DEBUG>:${DEBUG_OPTS}>")
  target_compile_options(re_cov PUBLIC "$<$<CONFIG:DEBUG>:${ASAN_OPTS}>")
  target_link_options(re_cov PUBLIC "${COV_OPTS}")
  target_link_options(re_cov PUBLIC "${ASAN_OPTS}")
endif()

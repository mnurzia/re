#!/bin/sh
set -euxo pipefail
rm -f default.profraw
rm -f cov.profraw
rm -f oom.profraw
rm -f cov.profdata
rm -f lcov.info
$1 -s $(echo $2 | sed 's/test_/s_/g')
mv default.profraw cov.profraw
$1 -s $(echo $2 | sed 's/test_/s_/g') --leak-check-oom
mv default.profraw oom.profraw
llvm-profdata merge cov.profraw oom.profraw -o cov.profdata
llvm-cov export re_cov -instr-profile=cov.profdata -format=lcov > lcov.info
mkdir -p cov-html/
genhtml lcov.info -q --function-coverage --branch-coverage --output-directory=cov-html/

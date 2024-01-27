@echo off
pushd out
set files=../lib/dreid.c ../lib/dd_gfx.c ../lib/dd_log.c 
set flags=-fsanitize=address -O0 -gfull -g3 -Wall -Wno-switch -Wno-microsoft-enum-forward-reference -Wno-unused-variable -Wno-unused-function 
clang %files% -c %flags% 
popd
llvm-ar rc dreid.lib out/dreid.o out/dd_gfx.o out/dd_log.o
@echo on

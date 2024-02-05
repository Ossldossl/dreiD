@echo off
call build_lib.bat
set libs=-l shell32.lib -l gdi32.lib -l user32.lib -l opengl32.lib -l glew32s.lib -l dreid.lib
set flags=-fsanitize=address -O0 -gfull -g3 -Wall -Wno-switch -Wno-microsoft-enum-forward-reference -Wno-unused-variable -Wno-unused-function 
clang example.c -o out/main.exe %flags% %libs%
@echo on

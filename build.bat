@echo off
set files=example.c lib/dreid.c lib/dd_gfx.c lib/dd_log.c 
set libs=-l shell32.lib -l gdi32.lib -l user32.lib -l opengl32.lib -l glew32s.lib
set flags=-fsanitize=address -O0 -gfull -g3 -Wall -Wno-switch -Wno-microsoft-enum-forward-reference -Wno-unused-variable -Wno-unused-function 
clang %files% -o out/main.exe %flags% %libs%
@echo on
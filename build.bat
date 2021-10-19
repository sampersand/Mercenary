call vcvars64

if not exist "build/" mkdir build

"G:\NASM\nasm.exe" src/lexer.asm -f win64 -o build/lexer.obj
clang src/main.c -D_CRT_SECURE_NO_WARNINGS -g -gcodeview build/lexer.obj -o build/main.exe

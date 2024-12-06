cd build
cd src
./compiler "$(cat ../../input.txt)" > compiler.ll
llc --filetype=obj -o=compiler.o compiler.ll
clang -o compilerbin compiler.o ../../rtCompiler.c
./compilerbin

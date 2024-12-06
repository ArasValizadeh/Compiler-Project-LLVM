add_executable (compiler
  Compiler.cpp
  CodeGen.cpp
  Lexer.cpp
  Parser.cpp
  Sema.cpp
  )
target_link_libraries(compiler PRIVATE ${llvm_libs})

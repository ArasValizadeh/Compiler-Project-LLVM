#include "Lexer.h"                // Includes the lexer header for tokenizing input
#include "llvm/Support/CommandLine.h" // LLVM support library for command-line options
#include "llvm/Support/InitLLVM.h"    // LLVM initialization utilities
#include "llvm/Support/raw_ostream.h" // LLVM raw output stream support
#include <iostream>                   // Standard I/O library
#include "AST.h"                      // Abstract Syntax Tree definitions
#include "CodeGen.h"                  // Code generation module
#include "Parser.h"                   // Parser for generating AST from tokens
#include "Sema.h"                     // Semantic analysis for AST

// Command-line option for specifying the input expression
static llvm::cl::opt<std::string>
    Input(llvm::cl::Positional,      // Positional argument (not prefixed by a flag)
          llvm::cl::desc("<input expression>"), // Description of the option
          llvm::cl::init(""));      // Default value if none is provided

// Main function of the program
int main(int argc, const char **argv){
    llvm::errs() << "call main function in Compiler.cpp\n"; //TODO Debug log
    // Initialize the LLVM framework with the given command-line arguments
    llvm::InitLLVM X(argc, argv);

    // Parse the command-line options using LLVM's command-line parser
    llvm::cl::ParseCommandLineOptions(argc, argv, "Simple Compiler\n");

    // Create a Lexer object initialized with the input expression from the command line
    llvm::errs() << "before Lexer\n"; //TODO Debug log
    Lexer Lex(Input);
    llvm::errs() << "after Lexer\n"; //TODO Debug log

    // Create a Parser object initialized with the Lexer
    Parser Parser(Lex);

    //TODO: should be uncommented
    // Parse the input expression to generate an Abstract Syntax Tree (AST)
    Program *Tree = Parser.parse();

    // Check if parsing was successful or if syntax errors occurred
    if (!Tree || Parser.hasError()){
        llvm::errs() << "Syntax errors occurred\n"; // Report syntax errors to the user
        return 1; // Exit with an error code
    }

    // Create a Semantic Analyzer object to perform semantic checks on the AST
    Sema Semantic;
    if (Semantic.semantic(Tree)){
        llvm::errs() << "Semantic errors occurred\n"; // Report semantic errors
        return 1; // Exit with an error code
    }

    // // Create a Code Generator object to translate the AST into LLVM IR
    // CodeGen CodeGenerator;
    // CodeGenerator.compile(Tree);

    // Indicate successful execution
    return 0;
}
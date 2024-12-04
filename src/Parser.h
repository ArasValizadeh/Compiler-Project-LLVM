#ifndef PARSER_H
#define PARSER_H

#include "AST.h"
#include "Lexer.h"
#include "llvm/Support/raw_ostream.h"

class Parser
{
    Lexer &Lex;    // retrieve the next token from the input
    Token Tok;     // stores the next token
    bool HasError; // indicates if an error was detected

    void error()
    {
        llvm::errs() << "Unexpected: " << Tok.getText() << Tok.getKind() << "\n";
        HasError = true;
    }

    // retrieves the next token from the lexer.expect()
    // tests whether the look-ahead is of the expected kind
    void advance() { 
        //llvm::errs() << "Consuming Token: " << getTokenName(Tok.getKind()) << " Text:" << Tok.getText() << "\n";
        Lex.next(Tok); 
        }

    bool expect(Token::TokenKind Kind)
    {
        if (Tok.getKind() != Kind)
        {
            //error();
            return true;
        }
        return false;
    }

    // retrieves the next token if the look-ahead is of the expected kind
    bool consume(Token::TokenKind Kind)
    {
        if (expect(Kind))
            return true;
        advance();
        return false;
    }

    Program *parseProgram();
    DeclarationInt *parseIntDec();
    DeclarationBool *parseBoolDec();
    DeclarationFloat *parseFloatDec();  // TODO added for float type declaration
    DeclarationVar *parseVarDec();      // TODO added for var type declaration
    DeclarationConst *parseConstDec(); // TODO added for constant variables
    DefineStmt *parseDefine();         // TODO added for #define macros
    SwitchStmt *parseSwitch();         // TODO added for switch-case construct
    DoWhileStmt *parseDoWhile();       // TODO added for do-while loop
    Expr *parseMinFunction();          // TODO added for min function
    Expr *parseMaxFunction();          // TODO added for max function
    Expr *parseMeanFunction();         // TODO added for mean function
    Expr *parseSqrtNFunction();        // TODO added for sqrtN function
    Expr *parseFunctionCall();         // TODO added for function calls like min(), max(), mean(), sqrtN()
    Assignment *parseBoolAssign();
    Assignment *parseIntAssign();
    UnaryOp *parseUnary();
    Expr *parseExpr();
    Expr *parseTerm();
    Expr *parseFinal();
    Expr *parseFactor();
    Logic *parseLogic();
    Logic *parseComparison();
    IfStmt *parseIf();
    WhileStmt *parseWhile();
    ForStmt *parseFor();
    PrintStmt *parsePrint();
    void parseComment();
    llvm::SmallVector<AST *> getBody();

public:
    // initializes all members and retrieves the first token
    Parser(Lexer &Lex) : Lex(Lex), HasError(false)
    {
        advance();
    }

    // get the value of error flag
    bool hasError() { return HasError; }

    Program *parse();
};

#endif
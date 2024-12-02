#ifndef LEXER_H // Prevent multiple inclusions of the header file
#define LEXER_H

#include "llvm/ADT/StringRef.h"        // Provides a lightweight reference to a string
#include "llvm/Support/MemoryBuffer.h" // Supports read-only access to file content in memory

// Forward declaration of the Lexer class
class Lexer;

class Token{
    friend class Lexer; // Grants the Lexer class access to Token's private members

public:
    // Enumeration of token types (kinds of tokens)
    enum TokenKind : unsigned short{
        eoi,            // End of input
        unknown,        // Unknown token (used for lexical errors)
        ident,          // Identifier (e.g., variable names)
        number,         // Numeric literal
        assign,         // '=' (assignment operator)
        minus_assign,   // '-=' (compound subtraction assignment)
        plus_assign,    // '+=' (compound addition assignment)
        star_assign,    // '*=' (compound multiplication assignment)
        slash_assign,   // '/=' (compound division assignment)
        eq,             // '==' (equality operator)
        neq,            // '!=' (inequality operator)
        gt,             // '>' (greater than)
        lt,             // '<' (less than)
        gte,            // '>=' (greater than or equal to)
        lte,            // '<=' (less than or equal to)
        plus_plus,      // '++' (increment operator)
        minus_minus,    // '--' (decrement operator)
        start_comment,  // '/*' (start of a comment)
        end_comment,    // '*/' (end of a comment)
        comma,          // ',' (comma separator)
        semicolon,      // ';' (statement terminator)
        plus,           // '+' (addition operator)
        minus,          // '-' (subtraction operator)
        star,           // '*' (multiplication operator)
        slash,          // '/' (division operator)
        mod,            // '%' (modulus operator)
        exp,            // '^' (exponentiation operator)
        l_paren,        // '(' (left parenthesis)
        minus_paren,    // '-(' (negative expression start)
        r_paren,        // ')' (right parenthesis)
        l_brace,        // '{' (left brace)
        r_brace,        // '}' (right brace)
        KW_int,         // Keyword 'int'
        KW_bool,        // Keyword 'bool'
        KW_true,        // Keyword 'true'
        KW_false,       // Keyword 'false'
        KW_if,          // Keyword 'if'
        KW_else,        // Keyword 'else'
        KW_for,         // Keyword 'for'
        KW_and,         // Keyword 'and'
        KW_or,          // Keyword 'or'
        KW_print,       // Keyword 'print'
        KW_float,       // Keyword 'float' //TODO added for float variables
        KW_var,         // Keyword 'var' //TODO added for dynamic typing
        KW_const,       // Keyword 'const' //TODO added for constant variables  
        KW_define,     // Keyword '#define' //TODO added for macros
        KW_switch,      // Keyword 'switch' //TODO added for switch cases
        KW_case,        // Keyword 'case' //TODO added for switch cases
        KW_default,     // Keyword 'default' //TODO added for switch cases
        KW_break,       // Keyword 'break' //TODO added for breaking out of loops
        KW_continue,    // Keyword 'continue' //TODO added for skipping to the next iteration of loops
        KW_do,          // Keyword 'do' //TODO added for do while loops
        KW_while,       // Keyword 'while' //TODO added for do while loops
        KW_min,         // Keyword 'min'   // TODO added for min function
        KW_max,         // Keyword 'max' // TODO added for max function
        KW_mean,        // Keyword 'mean' // TODO added for mean function
        KW_sqrtN,       // Keyword 'sqrtN' // TODO added for sqrtN function
        colon,          // ':' (for switch-case statements) //TODO added for switch-case statements
    };

private:
    TokenKind Kind;          // The type of the token
    llvm::StringRef Text;    // Textual representation of the token

public:
    // Accessor for the token's kind
    TokenKind getKind() const { return Kind; }

    // Accessor for the token's text
    llvm::StringRef getText() const { return Text; }

    // Check if the token matches a specific kind
    bool is(TokenKind K) const { return Kind == K; }

    // Check if the token matches one of two kinds
    bool isOneOf(TokenKind K1, TokenKind K2) const{
        return is(K1) || is(K2);
    }

    // Check if the token matches any of a list of kinds
    template <typename... Ts>
    bool isOneOf(TokenKind K1, TokenKind K2, Ts... Ks) const{
        return is(K1) || isOneOf(K2, Ks...);
    }
};

class Lexer{
    const char *BufferStart; // Pointer to the start of the input buffer
    const char *BufferPtr;   // Pointer to the current unprocessed character in the buffer

public:
    // Constructor: Initializes the lexer with the input buffer
    Lexer(const llvm::StringRef &Buffer){
        BufferStart = Buffer.begin(); // Set the buffer start pointer
        BufferPtr = BufferStart;      // Initialize the processing pointer to the start
    }

    // Retrieves the next token from the input stream
    void next(Token &token);

    // Allows the buffer pointer to be reset to a specific position
    void setBufferPtr(const char *buffer);//TODO change from  void setBufferPtr(const char *buffer) { BufferPtr = buffer; }

    // Getter for the current position of the buffer pointer
    const char *getBuffer() { return BufferPtr; }

private:
    // Creates a token of the specified kind with the given range
    void formToken(Token &Result, const char *TokEnd, Token::TokenKind Kind);
};

#endif // End of conditional compilation for LEXER_H
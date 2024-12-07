#ifndef LEXER_H // Prevent multiple inclusions of the header file
#define LEXER_H

#include "llvm/ADT/StringRef.h"        // Provides a lightweight reference to a string
#include "llvm/Support/MemoryBuffer.h" // Supports read-only access to file content in memory

// Forward declaration of the Lexer class
class Lexer;

class Token{
    int Line; // TODO
    int Column; // TODO
    friend class Lexer; // Grants the Lexer class access to Token's private members

public:
    // Enumeration of token types (kinds of tokens)
    enum TokenKind : unsigned short{
        eoi,            // End of input //!0
        unknown,        // Unknown token (used for lexical errors) //!1
        ident,          // Identifier (e.g., variable names) //!2
        number,         // Numeric literal //!3
        assign,         // '=' (assignment operator) //!4
        minus_assign,   // '-=' (compound subtraction assignment) //!5
        plus_assign,    // '+=' (compound addition assignment) //!6
        star_assign,    // '*=' (compound multiplication assignment) //!7
        slash_assign,   // '/=' (compound division assignment) //!8
        eq,             // '==' (equality operator) //!9
        neq,            // '!=' (inequality operator) //!10
        gt,             // '>' (greater than) //!11 
        lt,             // '<' (less than) //!12
        gte,            // '>=' (greater than or equal to) //!13
        lte,            // '<=' (less than or equal to) //!14
        plus_plus,      // '++' (increment operator) //!15
        minus_minus,    // '--' (decrement operator) //!16
        start_comment,  // '/*' (start of a comment) //!17 
        end_comment,    // '*/' (end of a comment) //!18
        comma,          // ',' (comma separator) //!19
        semicolon,      // ';' (statement terminator) //!20
        plus,           // '+' (addition operator) //!21
        minus,          // '-' (subtraction operator) //!22
        star,           // '*' (multiplication operator) //!23
        slash,          // '/' (division operator) //!24
        mod,            // '%' (modulus operator) //!25
        exp,            // '^' (exponentiation operator) //!26
        l_paren,        // '(' (left parenthesis) //!27
        minus_paren,    // '-(' (negative expression start) //!28
        r_paren,        // ')' (right parenthesis) //!29
        l_brace,        // '{' (left brace) //!30
        r_brace,        // '}' (right brace) //!31
        KW_int,         // Keyword 'int' //!32
        KW_bool,        // Keyword 'bool' //!33
        KW_true,        // Keyword 'true' //!34
        KW_false,       // Keyword 'false' //!35
        KW_if,          // Keyword 'if' //!36
        KW_else,        // Keyword 'else' //!37
        KW_for,         // Keyword 'for' //!38
        KW_and,         // Keyword 'and' //!39
        KW_or,          // Keyword 'or' //!40
        KW_print,       // Keyword 'print' //!41
        KW_float,       // Keyword 'float' //TODO added for float variables 42
        KW_var,         // Keyword 'var' //TODO added for dynamic typing 43
        KW_const,       // Keyword 'const' //TODO added for constant variables 44 
        KW_define,     // Keyword '#define' //TODO added for macros 45
        KW_switch,      // Keyword 'switch' //TODO added for switch cases 46
        KW_case,        // Keyword 'case' //TODO added for switch cases 47
        KW_default,     // Keyword 'default' //TODO added for switch cases 48
        KW_break,       // Keyword 'break' //TODO added for breaking out of loops 49
        KW_continue,    // Keyword 'continue' //TODO added for skipping to the next iteration of loops 50
        KW_do,          // Keyword 'do' //TODO added for do while loops 51
        KW_while,       // Keyword 'while' //TODO added for do while loops 52
        KW_min,         // Keyword 'min'   // TODO added for min function 53
        KW_max,         // Keyword 'max' // TODO added for max function 54
        KW_mean,        // Keyword 'mean' // TODO added for mean function 55
        KW_sqrtN,       // Keyword 'sqrtN' // TODO added for sqrtN function 56
        colon,          // ':' (for switch-case statements) //TODO added for switch-case statements 57
        KW_xor,         // Keyword xor // TODO 58
        KW_not_in,         // Keyword not // TODO 59
        questionmark, //  ? // TODO 60
        KW_in, //  in // TODO 61
        l_squarebracket, //  [ // TODO 62
        r_squarebracket, //  ] // TODO 63
        mod_assign, // %= // TODO 64
        oneline_comment, // "//" TODO //!65
        floatNumber, // floatnumber TODO //!66
    };

private:
    TokenKind Kind;          // The type of the token
    llvm::StringRef Text;    // Textual representation of the token

public:
    int getLine() const { return Line; } //TODO

    int getColumn() const { return Column; } //TODO
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
std::string getTokenName(Token::TokenKind kind);

#endif // End of conditional compilation for LEXER_H
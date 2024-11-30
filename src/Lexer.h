#define LEXER_H // conditional compilations(checks whether a macro is not defined)
#define LEXER_H

#include "llvm/ADT/StringRef.h"        // encapsulates a pointer to a C string and its length
#include "llvm/Support/MemoryBuffer.h" // read-only access to a block of memory, filled with the content of a file

class Lexer;

class Token
{
    friend class Lexer; // Lexer can access private and protected members of Token

public:
    enum TokenKind : unsigned short
    {
        eoi,            // end of input
        unknown,        // in case of error at the lexical level
        ident,          // identifier
        number,         // integer literal
        assign,         // =
        minus_assign,   // -=
        plus_assign,    // +=
        star_assign,    // *=
        slash_assign,   // /=
        eq,             // ==
        neq,            // !=
        gt,             // >
        lt,             // <
        gte,            // >=
        lte,            // <=
        plus_plus,      // ++
        minus_minus,    // --
        start_comment,  // /*
        end_comment,    // */
        comma,          // ,
        semicolon,      // ;
        plus,           // +
        minus,          // -
        star,           // *
        slash,          // /
        mod,            // %
        exp,            // ^
        l_paren,        // (
        minus_paren,    // -(
        r_paren,        // )
        l_brace,        // {
        r_brace,        // }
        KW_int,         // int
        KW_bool,        // bool
        KW_true,        // true
        KW_false,       // false
        KW_if,          // if
        KW_else,        // else
        KW_while,       // while
        KW_for,         // for
        KW_and,         // and
        KW_or,          // or
        KW_print,        // print
        KW_xor,                  //xor        
        KW_const,               //const               
        KW_define,               // #define  
        KW_float,                // float              
        KW_var,                 // var                  
        KW_min,                 //min
        KW_max,                  //max
        KW_mean,                  //mean
        KW_switch,                  //switch
        KW_case,                    //case
        KW_default,                 //default
        KW_break,                   //break
        KW_continue,                 //continue
        KW_do,          //do 
        KW_in,          //in
        KW_not,          //not
        modules_assign,      // %=  
        oneLine_comment,   // "//"  
        left_bracket,          // [
        right_bracket,          // ]   
        questionMark,       // ?       //int a = x>y ? 1 : 2
        colonMark,          //:        //int a = x>y ? 1 : 2 
        floatNumber,

    };

private:
    TokenKind Kind;
    llvm::StringRef Text; // points to the start of the text of the token

public:
    TokenKind getKind() const { return Kind; }
    llvm::StringRef getText() const { return Text; }

    // to test if the token is of a certain kind
    bool is(TokenKind K) const { return Kind == K; }
    bool isOneOf(TokenKind K1, TokenKind K2) const
    {
        return is(K1) || is(K2);
    }
    template <typename... Ts>
    bool isOneOf(TokenKind K1, TokenKind K2, Ts... Ks)
        const { return is(K1) || isOneOf(K2, Ks...); }
};

class Lexer
{
    const char *BufferStart; // pointer to the beginning of the input
    const char *BufferPtr;   // pointer to the next unprocessed character

public:
    Lexer(const llvm::StringRef &Buffer)
    {
        BufferStart = Buffer.begin();
        BufferPtr = BufferStart;
    }

    void next(Token &token); // return the next token
    void setBufferPtr(const char* buffer);
    const char* getBuffer(){return BufferPtr;};

private:
    void formToken(Token &Result, const char *TokEnd, Token::TokenKind Kind);
};
#endif
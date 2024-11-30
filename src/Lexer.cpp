#include "Lexer.h"
#include "llvm/Support/raw_ostream.h"

// classifying characters
namespace charinfo
{
    // ignore whitespaces
    LLVM_READNONE inline bool isWhitespace(char c)
    {
        return c == ' ' || c == '\t' || c == '\f' || c == '\v' ||
               c == '\r' || c == '\n';
    }

    LLVM_READNONE inline bool isDigit(char c)
    {
        return c >= '0' && c <= '9';
    }

    LLVM_READNONE inline bool isLetter(char c)
    {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
    }

    LLVM_READNONE inline bool isSpecialCharacter(char c)
    {
        return || c == '[' || c == ']' || c == '?' || c == ':' || c == '=' || c == '+' || c == '-' || c == '*' || c == '/' || c == '!' || c == '>' || c == '<' || c == '(' || c == ')' || c == '{' || c == '}'|| c == ',' || c == ';' || c == '%' || c == '^';
    }

    LLVM_READNONE inline bool isSharp(char c)
    {
        return (c=='#');
    }

    LLVM_READNONE inline bool isUnderScore(char c)
    {
        return (c=='_');
    }
}

void Lexer::next(Token &token) {
    while (*BufferPtr && charinfo::isWhitespace(*BufferPtr)) {
        ++BufferPtr;
    }
    // make sure we didn't reach the end of input
    if (!*BufferPtr) {
        token.Kind = Token::eoi;
        return;
    }
    // collect characters and check for keywords or ident
    if (charinfo::isLetter(*BufferPtr)) {
        const char *end = BufferPtr + 1;
        while (charinfo::isLetter(*end) || charinfo::isDigit(*end) || charinfo::isUnderScore(*end))
            ++end;
        llvm::StringRef Name(BufferPtr, end - BufferPtr);
        Token::TokenKind kind;
        if (Name == "int")
            kind = Token::KW_int;
        else if (Name == "bool")
            kind = Token::KW_bool;
        else if (Name == "print")
            kind = Token::KW_print;
        else if (Name == "while")
            kind = Token::KW_while;
        else if (Name == "for")
            kind = Token::KW_for;
        else if (Name == "if")
            kind = Token::KW_if;
        else if (Name == "else")
            kind = Token::KW_else;
        else if (Name == "true")
            kind = Token::KW_true;
        else if (Name == "false")
            kind = Token::KW_false;
        else if (Name == "and")
            kind = Token::KW_and;
        else if (Name == "or")
            kind = Token::KW_or;
        else if(Name == "do")
            kind = Token::KW_do;
        else if(Name == "in")
            kind = Token::KW_in;
        else if(Name == "not")
            kind = Token::KW_not;
        else if (Name == 'define')
            kind = Token::KW_define;
        else if(Name == "xor")
            kind = Token::KW_xor;
        else if(Name == "const")
            kind = Token::KW_const;
        else if(Name == "float")
            kind = Token::KW_float;
        else if(Name == "var")
            kind = Token::KW_var;
        else if(Name == "min")
            kind = Token::KW_min;
        else if(Name == "max")
            kind = Token::KW_max;
        else if(Name == "mean")
            kind = Token::KW_mean;
        else if(Name == "switch")
            kind = Token::KW_switch;
        else if(Name == "case")
            kind = Token::KW_case;
        else if(Name == "default")
            kind = Token::KW_default;
        else if(Name == "break")
            kind = Token::KW_break;
        else if(Name == "continue")
            kind = Token::KW_continue;
        else
            kind = Token::ident;
        formToken(token, end, kind);
        return;
    } else if (charinfo::isDigit(*BufferPtr) || (*BufferPtr == '.' && charinfo::isDigit(*(BufferPtr + 1)))) { // check for integers and float
        // Process the integer part
        while (charinfo::isDigit(*end)) {
            ++end;
        }

        // Check for a decimal point
        if (*end == '.') {
            ++end;
            isFloat = true;

            // Process the fractional part
            while (charinfo::isDigit(*end)) {
                ++end;
            }
        }

        // Validate if '.' is not followed by digits
        if (*start == '.' && !isFloat) {
            formToken(token, end, Token::unknown);
            return;
        }

        // Decide token kind based on the presence of a fractional part
        formToken(token, end, isFloat ? Token::floatNumber : Token::number);
        return;
    } else if (charinfo::isSpecialCharacter(*BufferPtr)) {
        const char *endWithOneLetter = BufferPtr + 1;
        const char *endWithTwoLetter = BufferPtr + 2;
        const char *end;
        llvm::StringRef NameWithOneLetter(BufferPtr, endWithOneLetter - BufferPtr);
        llvm::StringRef NameWithTwoLetter(BufferPtr, endWithTwoLetter - BufferPtr);
        Token::TokenKind kind;
        bool isFound = false;
        if (NameWithTwoLetter == "=="){
            kind = Token::eq;
            isFound = true;
            end = endWithTwoLetter;
        } else if (NameWithOneLetter == "=") {
            kind = Token::assign;
            isFound = true;
            end = endWithOneLetter;
        } else if (NameWithTwoLetter == "!="){
            kind = Token::neq;
            isFound = true;
            end = endWithTwoLetter;
        } else if (NameWithTwoLetter == "-("){
            kind = Token::minus_paren;
            isFound = true;
            end = endWithTwoLetter;
        }else if (NameWithTwoLetter == "+="){
            kind = Token::plus_assign;
            isFound = true;
            end = endWithTwoLetter;
        } else if (NameWithTwoLetter == "-="){
            kind = Token::minus_assign;
            isFound = true;
            end = endWithTwoLetter;
        } else if (NameWithTwoLetter == "*="){
            kind = Token::star_assign;
            isFound = true;
            end = endWithTwoLetter;
        } else if (NameWithTwoLetter == "*/"){
            kind = Token::end_comment;
            isFound = true;
            end = endWithTwoLetter;
        } else if (NameWithTwoLetter == "/="){
            kind = Token::slash_assign;
            isFound = true;
            end = endWithTwoLetter;
        } else if (NameWithTwoLetter == "/*"){
            kind = Token::start_comment;
            isFound = true;
            end = endWithTwoLetter;
        } else if (NameWithTwoLetter == ">="){
            kind = Token::gte;
            isFound = true;
            end = endWithTwoLetter;
        } else if (NameWithTwoLetter == "<="){
            kind = Token::lte;
            isFound = true;
            end = endWithTwoLetter;
        } else if (NameWithTwoLetter == "++"){
            kind = Token::plus_plus;
            isFound = true;
            end = endWithTwoLetter;
        } else if (NameWithTwoLetter == "--"){
            kind = Token::minus_minus;
            isFound = true;
            end = endWithTwoLetter;
        } else if (NameWithOneLetter == "+"){
            kind = Token::plus;
            isFound = true;
            end = endWithOneLetter;
        } else if (NameWithOneLetter == "-"){
            kind = Token::minus;
            isFound = true;
            end = endWithOneLetter;
        } else if (NameWithOneLetter == "*"){
            kind = Token::star;
            isFound = true;
            end = endWithOneLetter;
        } else if (NameWithOneLetter == "/"){
            kind = Token::slash;
            isFound = true;
            end = endWithOneLetter;
        } else if (NameWithOneLetter == "+"){
            kind = Token::plus;
            isFound = true;
            end = endWithOneLetter;
        } else if (NameWithOneLetter == ">"){
            kind = Token::gt;
            isFound = true;
            end = endWithOneLetter;
        } else if (NameWithOneLetter == "<"){
            kind = Token::lt;
            isFound = true;
            end = endWithOneLetter;
        } else if (NameWithOneLetter == "("){
            kind = Token::l_paren;
            isFound = true;
            end = endWithOneLetter;
        } else if (NameWithOneLetter == ")"){
            kind = Token::r_paren;
            isFound = true;
            end = endWithOneLetter;
        } else if (NameWithOneLetter == "{"){
            kind = Token::l_brace;
            isFound = true;
            end = endWithOneLetter;
        } else if (NameWithOneLetter == "}"){
            kind = Token::r_brace;
            isFound = true;
            end = endWithOneLetter;
        } else if (NameWithOneLetter == ";"){
            kind = Token::semicolon;
            isFound = true;
            end = endWithOneLetter;
        } else if (NameWithOneLetter == ","){
            kind = Token::comma;
            isFound = true;
            end = endWithOneLetter;
        } else if (NameWithOneLetter == "%"){
            kind = Token::mod;
            isFound = true;
            end = endWithOneLetter;
        } else if (NameWithOneLetter == "^"){
            kind = Token::exp;
            isFound = true;
            end = endWithOneLetter;
        }else if(NameWithOneLetter == "?"){ 
            kind = Token::questionMark;
            isFound = true;
            end =endWithOneLetter;
        } else if(NameWithOneLetter == ":"){ 
            kind = Token::colonMark;
            isFound = true;
            end = endWithOneLetter;
        }else if(NameWithOneLetter == "["){ 
            kind = Token::l_bracket;
            isFound = true;
            end =endWithOneLetter;
        } else if(NameWithOneLetter == "]"){
            kind = Token::r_bracket;
            isFound = true;
            end = endWithOneLetter;
        } else if(NameWithTwoLetter == "%="){ 
            kind = Token::mod_assign;
            isFound = true;
            end = endWithTwoLetter;
        } 
        
        // generate the token
        if (isFound) formToken(token, end, kind);
        else formToken(token, BufferPtr + 1, Token::unknown);
        return;
    } else if (charinfo::isSharp(*BufferPtr)) {  // Check for #define
        const char *end = BufferPtr + 1;

        // Consume letters following '#'
        while (charinfo::isLetter(*end)) {
            ++end;
        }

        llvm::StringRef Name(BufferPtr, end - BufferPtr);

        if (Name == "#define") {
            formToken(token, end, Token::KW_define);
        } else {
            formToken(token, BufferPtr + 1, Token::unknown);
        }
        return;
    } else {
        formToken(token, BufferPtr + 1, Token::unknown); 
        return;         
    }
    return;
}

void Lexer::setBufferPtr(const char *buffer){
    BufferPtr = buffer;
}

void Lexer::formToken(Token &Tok, const char *TokEnd,
                      Token::TokenKind Kind)
{
    Tok.Kind = Kind;
    Tok.Text = llvm::StringRef(BufferPtr, TokEnd - BufferPtr);
    BufferPtr = TokEnd;
}
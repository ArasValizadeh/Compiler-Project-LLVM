#include "Lexer.h"
#include "llvm/Support/raw_ostream.h"
// xor, not, ?, in, [, ]
// classifying characters
#include "Lexer.h"
#include "llvm/Support/raw_ostream.h"

// Include other relevant headers as needed

// Function to convert TokenKind enum to a string
std::string getTokenName(Token::TokenKind kind) {
    switch (kind) {
        case Token::eoi: return "eoi";
        case Token::unknown: return "unknown";
        case Token::ident: return "ident";
        case Token::number: return "number";
        case Token::assign: return "assign";
        case Token::minus_assign: return "minus_assign";
        case Token::plus_assign: return "plus_assign";
        case Token::star_assign: return "star_assign";
        case Token::slash_assign: return "slash_assign";
        case Token::eq: return "eq";
        case Token::neq: return "neq";
        case Token::gt: return "gt";
        case Token::lt: return "lt";
        case Token::gte: return "gte";
        case Token::lte: return "lte";
        case Token::plus_plus: return "plus_plus";
        case Token::minus_minus: return "minus_minus";
        case Token::start_comment: return "start_comment";
        case Token::end_comment: return "end_comment";
        case Token::comma: return "comma";
        case Token::semicolon: return "semicolon";
        case Token::plus: return "plus";
        case Token::minus: return "minus";
        case Token::star: return "star";
        case Token::slash: return "slash";
        case Token::mod: return "mod";
        case Token::exp: return "exp";
        case Token::l_paren: return "l_paren";
        case Token::minus_paren: return "minus_paren";
        case Token::r_paren: return "r_paren";
        case Token::l_brace: return "l_brace";
        case Token::r_brace: return "r_brace";
        case Token::KW_int: return "KW_int";
        case Token::KW_bool: return "KW_bool";
        case Token::KW_true: return "KW_true";
        case Token::KW_false: return "KW_false";
        case Token::KW_if: return "KW_if";
        case Token::KW_else: return "KW_else";
        case Token::KW_for: return "KW_for";
        case Token::KW_and: return "KW_and";
        case Token::KW_or: return "KW_or";
        case Token::KW_print: return "KW_print";
        case Token::KW_float: return "KW_float";
        case Token::KW_var: return "KW_var";
        case Token::KW_const: return "KW_const";
        case Token::KW_define: return "KW_define";
        case Token::KW_switch: return "KW_switch";
        case Token::KW_case: return "KW_case";
        case Token::KW_default: return "KW_default";
        case Token::KW_break: return "KW_break";
        case Token::KW_continue: return "KW_continue";
        case Token::KW_do: return "KW_do";
        case Token::KW_while: return "KW_while";
        case Token::KW_min: return "KW_min";
        case Token::KW_max: return "KW_max";
        case Token::KW_mean: return "KW_mean";
        case Token::KW_sqrtN: return "KW_sqrtN";
        case Token::colon: return "colon";
        case Token::KW_xor: return "KW_xor";
        case Token::KW_not: return "KW_not";
        case Token::questionmark: return "questionmark";
        case Token::KW_in: return "KW_in";
        case Token::l_squarebracket: return "l_squarebracket";
        case Token::r_squarebracket: return "r_squarebracket";
        case Token::mod_assign: return "mod_assign";
        case Token::oneline_comment: return "oneline_comment";
        case Token::floatNumber: return "floatNumber";
        default: return "unknown";
    }
}

// Existing functions in Lexer.cpp follow here


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
    LLVM_READNONE inline bool isSpecialCharacter(char c){
        // TODO: 
        return c == '=' || c == '+' || c == '-' || c == '*' || c == '/' ||
           c == '!' || c == '>' || c == '<' || c == '(' || c == ')' ||
           c == '{' || c == '}' || c == ',' || c == ';' || c == '%' ||
           c == '^' || c == ':' || c == '?' || c == '[' || c == ']'; // TODO added : , ?, [, ] 
        // return c == '=' || c == '+' || c == '-' || c == '*' || c == '/' || c == '!' || c == '>' || c == '<' || c == '(' || c == ')' || c == '{' || c == '}'|| c == ',' || c == ';' || c == '%' || c == '^';
    }
    LLVM_READNONE inline bool isSharp(char c){
        return c == '#';
    }
    LLVM_READNONE inline bool isUnderScore (char c){
        return c == '_';
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
        while (charinfo::isLetter(*end) || charinfo::isDigit(*end))
            ++end;
        llvm::StringRef Name(BufferPtr, end - BufferPtr);
        Token::TokenKind kind;
        if (Name == "int")
            kind = Token::KW_int;
        else if (Name == "bool")
            kind = Token::KW_bool;
        else if (Name == "xor")            // TODO
            kind = Token::KW_xor;
        else if (Name == "not")             // TODO
            kind = Token::KW_not;
        else if (Name == "in")              // TODO
            kind = Token::KW_in;
        else if (Name == "float")               // TODO added for float type
            kind = Token::KW_float;
        else if (Name == "var")                 // TODO added for var type
            kind = Token::KW_var;
        else if (Name == "const")               // TODO added for constant variables
            kind = Token::KW_const;
        else if (Name == "define")              // TODO added for macros
            kind = Token::KW_define;
        else if (Name == "switch")              // TODO added for switch cases
            kind = Token::KW_switch;
        else if (Name == "case")                // TODO added for switch cases
            kind = Token::KW_case;
        else if (Name == "default")             // TODO added for default case in switch
            kind = Token::KW_default;
        else if (Name == "break")               // TODO added for breaking loops/switch
            kind = Token::KW_break;
        else if (Name == "continue")            // TODO added for skipping loop iteration
            kind = Token::KW_continue;
        else if (Name == "do")                  // TODO added for do-while loop
            kind = Token::KW_do;
        else if (Name == "min")                 // TODO added for min function
            kind = Token::KW_min;
        else if (Name == "max")                 // TODO added for max function
            kind = Token::KW_max;
        else if (Name == "mean")                // TODO added for mean function
            kind = Token::KW_mean;
        else if (Name == "sqrtN")               // TODO added for sqrtN function
            kind = Token::KW_sqrtN;
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
        else
            kind = Token::ident;
        // generate the token
        formToken(token, end, kind);
        return;
    } else if (charinfo::isDigit(*BufferPtr) || (*BufferPtr == '.' && charinfo::isDigit(*(BufferPtr + 1)))) { // Check for integers and floats
    const char *start = BufferPtr;  // Define start to point to the beginning of the token
    const char *end = BufferPtr;    // Use end to iterate through the number
    bool isFloat = false;           // Initialize isFloat to false

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
    if (isFloat && end == BufferPtr + 1) {  // Case where only '.' exists
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
        }else if (NameWithTwoLetter == "//"){ // TODO
            kind = Token::oneline_comment;
            isFound = true;
            end = endWithTwoLetter;
        } else if (NameWithTwoLetter == "%="){ // TODO
            kind = Token::mod_assign;
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
        }else if (NameWithOneLetter == ":"){ // TODO
            kind = Token::colon;
            isFound = true;
            end = endWithOneLetter;
        } else if (NameWithOneLetter == "?"){ // TODO
            kind = Token::questionmark;
            isFound = true;
            end = endWithOneLetter;
        } else if (NameWithOneLetter == "["){ // TODO
            kind = Token::l_squarebracket;
            isFound = true;
            end = endWithOneLetter;
        } else if (NameWithOneLetter == "]"){ // TODO
            kind = Token::r_squarebracket;
            isFound = true;
            end = endWithOneLetter;
        }
        else if (NameWithOneLetter == ","){
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
        }
        // generate the token
        if (isFound){ 
            formToken(token, end, kind);
        }
        else formToken(token, BufferPtr + 1, Token::unknown);
        return;
    } else if (charinfo::isSharp(*BufferPtr)) {  // Check for preprocessing directives
    const char *start = BufferPtr;  // Start at the current buffer pointer
    const char *end = BufferPtr + 1;
    while (charinfo::isLetter(*end)) 
        ++end;

    llvm::StringRef Name(BufferPtr, end - BufferPtr);  // Get the directive name
    Token::TokenKind kind;

    if (Name == "#define") {  // Match the directive `define`
        kind = Token::KW_define;
        formToken(token, end, kind);
        return;
    } else {
        // Handle unknown directives or invalid cases
        formToken(token, start, Token::unknown);
        return;
     }
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
    // llvm::errs() <<"------------------------------------------------------------------\n";
    // llvm::errs() << "Token formed: Kind: " << Kind << ", Text: " << Tok.Text << "\n"; // Debug log
    // llvm::errs() <<"------------------------------------------------------------------\n";
    BufferPtr = TokEnd;
}
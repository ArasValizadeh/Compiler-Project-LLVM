#include "Parser.h"
#include "AST.h"


// main point is that the whole input has been consumed
Program *Parser::parse()
{
    Program *Res = parseProgram();
    return Res;
}

Program *Parser::parseProgram()
{
    llvm::SmallVector<AST *> data;
    
    while (!Tok.is(Token::eoi)){
        switch (Tok.getKind()){
            case Token::KW_int: {
                DeclarationInt *d;
                d = parseIntDec();
                if (d)
                    data.push_back(d);
                else
                    goto _error;
                    
                break;
            }
            case Token::KW_bool: {
                DeclarationBool *dbool;
                dbool = parseBoolDec();
                if (dbool)
                    data.push_back(dbool);
                else
                    goto _error;

                break;
            }
            // TODO: Added for break
            case Token::KW_break: { 
                BreakStmt *b = parseBreak();
                if (b) data.push_back(b);
                else goto _error;
                break;
            }
            // TODO: Added for continue
            case Token::KW_continue: { 
                ContinueStmt *c = parseContinue();
                if (c) data.push_back(c);
                else goto _error;
                break;
            }
            case Token::KW_float: { // TODO added for float type
                DeclarationFloat *dfloat = parseFloatDec();
                if (dfloat) data.push_back(dfloat);
                else goto _error;
                break;
            }
            case Token::KW_var: { // TODO added for var type
                DeclarationVar *dvar = parseVarDec();
                if (dvar) data.push_back(dvar);
                else goto _error;
                break;
            }
            case Token::KW_const: { // TODO added for const type
                DeclarationConst *dconst = parseConstDec();
                if (dconst) data.push_back(dconst);
                else goto _error;
                break;
            }
            case Token::KW_define: { // TODO added for #define macros
                DefineStmt *ddefine = parseDefine();
                if (ddefine) data.push_back(ddefine);
                else goto _error;
                break;
            }
            case Token::KW_switch: { // TODO added for switch-case
                SwitchStmt *sswitch = parseSwitch();
                if (sswitch) data.push_back(sswitch);
                else goto _error;
                break;
            }
            case Token::KW_do: { // TODO added for do-while
                DoWhileStmt *dwhile = parseDoWhile();
                if (dwhile) data.push_back(dwhile);
                else goto _error;
                break;
            }
            case Token::ident: {
                Token prev_token = Tok;
                const char* prev_buffer = Lex.getBuffer();
                UnaryOp *u;
                u = parseUnary();
                if (Tok.is(Token::semicolon)){
                    if (u){
                        data.push_back(u);
                        break;
                    }else{
                        goto _error;
                        
                    }
                }else{
                    if (u){
                        goto _error;
                    }else{
                        Tok = prev_token;
                        Lex.setBufferPtr(prev_buffer);
                    }
                }
            
                Assignment *a_int;
                Assignment *a_bool;
                prev_token = Tok;
                prev_buffer = Lex.getBuffer();

                a_bool = parseBoolAssign();

                if (a_bool){
                    data.push_back(a_bool);
                    break;
                }
                Tok = prev_token;
                Lex.setBufferPtr(prev_buffer);

                a_int = parseIntAssign();
                if (!Tok.is(Token::semicolon))
                {
                    goto _error;
                }
                if (a_int)
                    data.push_back(a_int);
                else
                    goto _error;
                    
                break;
            }
            case Token::KW_if: {
                IfStmt *i;
                i = parseIf();
                if (i)
                    data.push_back(i);
                else
                    goto _error;
                
                break;
            }
            case Token::KW_while: {
                WhileStmt *w;
                w = parseWhile();
                if (w)
                    data.push_back(w);
                else {
                    goto _error;
                }
                break;
            }
            case Token::KW_for: {
                ForStmt *f;
                f = parseFor();
                if (f)
                    data.push_back(f);
                else {
                    goto _error;
                }
                break;
            }
            case Token::KW_print: {
                PrintStmt *p;
                p = parsePrint();
                if (p)
                    data.push_back(p);
                else {
                    goto _error;
                }
                break;
            }
            case Token::start_comment: {
                parseComment();
                if (!Tok.is(Token::end_comment))
                    goto _error;
                break;
            }
            default: {
                llvm::errs() << "balaye avalin error() dakhele parse program\n";
                error();

                goto _error;
                break;
            }
        }
        advance();
        
    }
    
    return new Program(data);
_error:
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

DeclarationInt *Parser::parseIntDec() {
    Expr *E = nullptr;
    llvm::SmallVector<llvm::StringRef> Vars;
    llvm::SmallVector<Expr *> Values;

    if (expect(Token::KW_int)) {
        llvm::errs() << "in parse int declaration keyword\n";
        goto _error;
    }
    advance();

    while (true) {
        if (expect(Token::ident)) {
            llvm::errs() << "in parse int declaration identifier\n";
            goto _error;
        }

        Vars.push_back(Tok.getText());
        advance();

        // If no more variables or assignments, stop processing
        if (!Tok.is(Token::comma) && !Tok.is(Token::assign)) {
            break;
        }

        // Handle multiple variables with explicit assignments
        if (Tok.is(Token::comma)) {
            advance(); // Move to next identifier
        } else if (Tok.is(Token::assign)) {
            advance(); // Move past `=`

            // Parse the list of explicit values
            while (true) {
                E = parseExpr();
                if (!E) {
                    llvm::errs() << "in parse int declaration after assign\n";
                    goto _error;
                }
                Values.push_back(E);

                // If there's a `,`, continue parsing more values
                if (Tok.is(Token::comma)) {
                    advance();
                } else {
                    break; // Stop when no more values
                }
            }
            break; // Stop variable parsing after explicit values
        }
    }

    if (expect(Token::semicolon)) {
        llvm::errs() << "in parse int declaration after semicolon\n";
        goto _error;
    }

    // Assign default values for any variables without explicit values
    while (Values.size() < Vars.size()) {
        Values.push_back(new Final(Final::Number, llvm::StringRef("0")));
    }

    // Ensure the number of values matches the variables
    if (Values.size() != Vars.size()) {
        llvm::errs() << "Number of variables and values do not match\n";
        goto _error;
    }

    return new DeclarationInt(Vars, Values);

_error:
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

DeclarationBool *Parser::parseBoolDec() {
    Logic *L = nullptr;
    llvm::SmallVector<llvm::StringRef> Vars;
    llvm::SmallVector<Logic *> Values;

    if (expect(Token::KW_bool)) {
        goto _error;
    }
    advance();

    while (true) {
        if (expect(Token::ident)) {
            goto _error;
        }

        Vars.push_back(Tok.getText());
        advance();

        // Stop processing if there are no more variables or assignments
        if (!Tok.is(Token::comma) && !Tok.is(Token::assign)) {
            break;
        }

        // Handle explicit assignments
        if (Tok.is(Token::comma)) {
            advance(); // Move to next variable
        } else if (Tok.is(Token::assign)) {
            advance(); // Move past `=`

            // Parse the list of explicit values
            while (true) {
                L = parseLogic();
                if (!L) {
                    goto _error;
                }
                Values.push_back(L);

                // Continue parsing more values if a comma is found
                if (Tok.is(Token::comma)) {
                    advance();
                } else {
                    break; // Stop when no more values
                }
            }
            break; // Stop variable parsing after explicit values
        }
    }

    if (expect(Token::semicolon)) {
        goto _error;
    }

    // Assign default values for variables without explicit values
    while (Values.size() < Vars.size()) {
        Values.push_back(new Comparison(nullptr, nullptr, Comparison::False));
    }

    // Ensure the number of values matches the number of variables
    if (Values.size() != Vars.size()) {
        llvm::errs() << "Number of variables and values do not match\n";
        goto _error;
    }

    return new DeclarationBool(Vars, Values);

_error:
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

DeclarationFloat *Parser::parseFloatDec() { // Updated for multiple float variables
    Expr *E = nullptr;
    llvm::SmallVector<llvm::StringRef> Vars;
    llvm::SmallVector<Expr *> Values;

    if (expect(Token::KW_float)) {
        goto _error;
    }
    advance();

    while (true) {
        if (expect(Token::ident)) {
            goto _error;
        }

        Vars.push_back(Tok.getText());
        advance();

        // Stop processing if there are no more variables or assignments
        if (!Tok.is(Token::comma) && !Tok.is(Token::assign)) {
            break;
        }

        // Handle explicit assignments
        if (Tok.is(Token::comma)) {
            advance(); // Move to next variable
        } else if (Tok.is(Token::assign)) {
            advance(); // Move past `=`

            // Parse the list of explicit values
            while (true) {
                E = parseExpr();
                if (!E) {
                    goto _error;
                }
                Values.push_back(E);

                // Continue parsing more values if a comma is found
                if (Tok.is(Token::comma)) {
                    advance();
                } else {
                    break; // Stop when no more values
                }
            }
            break; // Stop variable parsing after explicit values
        }
    }

    if (expect(Token::semicolon)) {
        goto _error;
    }

    // Assign default values for variables without explicit values
    while (Values.size() < Vars.size()) {
        Values.push_back(new Final(Final::Number, llvm::StringRef("0.0")));
    }

    // Ensure the number of values matches the number of variables
    if (Values.size() != Vars.size()) {
        llvm::errs() << "Number of variables and values do not match\n";
        goto _error;
    }

    return new DeclarationFloat(Vars, Values);

_error:
    while (Tok.getKind() != Token::eoi) advance();
    return nullptr;
}

// DeclarationVar *Parser::parseVarDec() { // TODO added for var type
//     Expr *E = nullptr;
//     llvm::StringRef Var;
//     if (expect(Token::KW_var)) goto _error;
//     advance();
//     if (expect(Token::ident)) goto _error;
//     Var = Tok.getText();
//     advance();
//     if (expect(Token::assign)) goto _error;
//     advance();
//     E = parseExpr();
//     if (!E) goto _error;
//     if (expect(Token::semicolon)) goto _error;
//     return new DeclarationVar(Var, E);
// _error:
//     while (Tok.getKind() != Token::eoi) advance();
//     return nullptr;
// }

DeclarationVar *Parser::parseVarDec() { //TODO Updated for multiple variables with different types
    llvm::SmallVector<llvm::StringRef> Vars;
    llvm::SmallVector<Expr *> Values;

    if (expect(Token::KW_var)) goto _error;
    advance();

    while (true) {
        if (expect(Token::ident)) {
            goto _error;
        }
        Vars.push_back(Tok.getText());
        advance();

        // Stop processing if there are no more variables or assignments
        if (!Tok.is(Token::comma) && !Tok.is(Token::assign)) {
            break;
        }

        // Handle explicit assignments
        if (Tok.is(Token::comma)) {
            advance(); // Move to the next variable
        } else if (Tok.is(Token::assign)) {
            advance(); // Move past `=`

            // Parse the list of explicit values
            while (true) {
                Expr *E = parseExpr();
                if (!E) {
                    goto _error;
                }
                Values.push_back(E);

                // Continue parsing more values if a comma is found
                if (Tok.is(Token::comma)) {
                    advance();
                } else {
                    break; // Stop when no more values
                }
            }
            break; // Stop variable parsing after explicit values
        }
    }

    if (expect(Token::semicolon)) goto _error;

    // Assign default values for variables without explicit values
    while (Values.size() < Vars.size()) {
        Values.push_back(new Final(Final::Number, llvm::StringRef("0.0")));
    }

    // Ensure the number of values matches the number of variables
    if (Values.size() != Vars.size()) {
        llvm::errs() << "Number of variables and values do not match\n";
        goto _error;
    }

    return new DeclarationVar(Vars, Values);

_error:
    while (Tok.getKind() != Token::eoi) advance();
    return nullptr;
}

// DeclarationConst *Parser::parseConstDec() { // TODO added for constant variables
//     Expr *E = nullptr;
//     llvm::StringRef Var;
//     if (expect(Token::KW_const)) goto _error;
//     advance();
//     if (expect(Token::ident)) goto _error;
//     Var = Tok.getText();
//     advance();
//     if (expect(Token::assign)) goto _error;
//     advance();
//     E = parseExpr();
//     if (!E) goto _error;
//     if (expect(Token::semicolon)) goto _error;
//     return new DeclarationConst(Var, E);
// _error:
//     while (Tok.getKind() != Token::eoi) advance();
//     return nullptr;
// }

DeclarationConst *Parser::parseConstDec() { // TODO added for constant variables
    llvm::StringRef Type;
    llvm::StringRef Var;
    Expr *E = nullptr;

    if (expect(Token::KW_const)) goto _error;
    advance();

    // Check the type of the constant (int, float, bool, or var)
    if (Tok.is(Token::KW_int) || Tok.is(Token::KW_float) || 
        Tok.is(Token::KW_bool) || Tok.is(Token::KW_var)) {
        Type = Tok.getText();
        advance();
    } else {
        llvm::errs() << "Error: Expected a type (int, float, bool, var) after 'const'.\n";
        goto _error;
    }

    if (expect(Token::ident)) goto _error;
    Var = Tok.getText();
    advance();

    if (expect(Token::assign)) goto _error;
    advance();

    E = parseExpr(); // Parse the initializer expression
    if (!E) {
        llvm::errs() << "Error: Expected an initializer for the constant.\n";
        goto _error;
    }

    if (expect(Token::semicolon)) goto _error;

    return new DeclarationConst(Type, Var, E);

_error:
    while (Tok.getKind() != Token::eoi) advance();
    return nullptr;
}

DefineStmt *Parser::parseDefine() { // TODO added for #define macros
    llvm::StringRef MacroName;
    llvm::StringRef MacroValue;

    if (expect(Token::KW_define)){
        llvm::errs() << "in parse define1\n";
        goto _error;
    }
    advance();

    if (expect(Token::ident)){
        llvm::errs() << "in parse define2\n";
        goto _error;
    }
    MacroName = Tok.getText();
    advance();

    if (!Tok.is(Token::number) && !Tok.is(Token::ident) && !Tok.is(Token::floatNumber) && !Tok.is(Token::KW_bool)){
        llvm::errs() << "in parse define3\n";
        goto _error;
    }
    MacroValue = Tok.getText();

    return new DefineStmt(MacroName, MacroValue);

_error:
    while (Tok.getKind() != Token::eoi) advance();
    return nullptr;
}

Assignment *Parser::parseBoolAssign()
{
    Final *F = nullptr;
    Assignment::AssignKind AK;
    Logic *L = nullptr;

    F = (Final *)(parseFinal());
    if (F == nullptr)
    {
        goto _error;
    }
    
    if (Tok.is(Token::assign))
    {
        AK = Assignment::Assign;
        advance();
        L = parseLogic();   // check if expr is logical

        if(L)
        {
            if (!Tok.is(Token::semicolon))
            {
                goto _error;
            }
            return new Assignment(F, nullptr, AK, L);
        }
        else
            goto _error;
    }
    else
    {
        goto _error;
    }
    
_error:
        while (Tok.getKind() != Token::eoi)
            advance();
        return nullptr;
    
}

Assignment *Parser::parseIntAssign()
{
    Expr *E = nullptr;
    Final *F = nullptr;
    Assignment::AssignKind AK;
    F = (Final *)(parseFinal());
    if (F == nullptr)
    {
        goto _error;
    }
    
    if (Tok.is(Token::assign))
    {
        AK = Assignment::Assign;
    }
    else if (Tok.is(Token::plus_assign))
    {
        AK = Assignment::Plus_assign;
    }
    else if (Tok.is(Token::minus_assign))
    {
        AK = Assignment::Minus_assign;
    }
    else if (Tok.is(Token::star_assign))
    {
        AK = Assignment::Star_assign;
    }
    else if (Tok.is(Token::slash_assign))
    {
        AK = Assignment::Slash_assign;
    }
    else
    {
        goto _error;
    }
    advance();
    E = parseExpr();    // check for mathematical expr
    if(E){
        return new Assignment(F, E, AK, nullptr);
    }
    else{
        llvm::errs() << "here i give error\n";
        goto _error;
    }

_error:
        while (Tok.getKind() != Token::eoi)
            advance();
        return nullptr;
}

UnaryOp *Parser::parseUnary()
{
    UnaryOp* Res = nullptr;
    llvm::StringRef var;

    if (expect(Token::ident)){
        goto _error;
    }

    var = Tok.getText();
    advance();
    if (Tok.getKind() == Token::plus_plus){
        Res = new UnaryOp(UnaryOp::Plus_plus, var);
    }
    else if(Tok.getKind() == Token::minus_minus){
        Res = new UnaryOp(UnaryOp::Minus_minus, var);
    }
    else{
        goto _error;
    }

    advance();

    return Res;

_error:
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

Expr *Parser::parseExpr()
{
    Expr *Left = parseTerm();

    if (Left == nullptr)
    {
        goto _error;
    }
    
    // TODO
    if (Tok.is(Token::questionmark)) { // Handle ternary operations
        return parseTernary();
    }
    while (Tok.isOneOf(Token::plus, Token::minus, Token::KW_xor))
    {
        BinaryOp::Operator Op;
        if (Tok.is(Token::plus))
            Op = BinaryOp::Plus;
        else if (Tok.is(Token::minus))
            Op = BinaryOp::Minus;
        else if (Tok.is(Token::KW_xor))
            Op = BinaryOp::Xor; // Handle xor
        else if (Tok.is(Token::KW_in)) {
            Op = BinaryOp::In;
        } else if (Tok.is(Token::KW_not_in)) {
            Op = BinaryOp::NotIn;
        } else {
            llvm::errs() << "balaye dovomin error() dakhele parse expression\n";
            error();

            goto _error;
        }
        advance();
        Expr *Right = parseTerm();
        if (Right == nullptr)
        {
            goto _error;
        }
        Left = new BinaryOp(Op, Left, Right);
    }
    return Left;

_error:
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

Expr *Parser::parseTerm()
{
    Expr *Left = parseFactor();
    if (Left == nullptr)
    {
        goto _error;
    }
    while (Tok.isOneOf(Token::star, Token::mod, Token::slash))
    {
        BinaryOp::Operator Op;
        if (Tok.is(Token::star))
            Op = BinaryOp::Mul;
        else if (Tok.is(Token::slash))
            Op = BinaryOp::Div;
        else if (Tok.is(Token::mod))
            Op = BinaryOp::Mod;
        else {
            llvm::errs() << "balaye sevomin error() dakhele parse term\n";
            error();

            goto _error;
        }
        advance();
        Expr *Right = parseFactor();
        if (Right == nullptr)
        {
            goto _error;
        }
        Left = new BinaryOp(Op, Left, Right);
    }
    return Left;

_error:
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

Expr *Parser::parseFactor() {
    Expr *Left = nullptr;

    // Handle casting expressions
    if ((Tok.getText() == "int" || Tok.getText() == "float" || Tok.getText() == "bool")) {
        return parseCastExpr(); // Handle casting
    }

    // Handle function calls
    if ((Tok.getText() == "min" || Tok.getText() == "max" || Tok.getText() == "sqrtN" || Tok.getText() == "mean")) {
        return parseFunctionCall(); // Handle function calls
    }

    switch (Tok.getKind()) {
        case Token::number: {
            Left = new Final(Final::Number, Tok.getText());
            advance();
            break;
        }
        case Token::floatNumber: {
            Left = new Final(Final::FloatNumber, Tok.getText());
            advance();
            break;
        }
        case Token::ident: {
            Left = new Final(Final::Ident, Tok.getText());
            advance();
            break;
        }
        case Token::KW_false: {
            Left = new Final(Final::Bool, Tok.getText());
            advance();
            break;
        }
        case Token::KW_true: {
            Left = new Final(Final::Bool, Tok.getText());
            advance();
            break;
        }
        case Token::l_paren: {
            advance();
            Left = parseExpr();
            if (!consume(Token::r_paren)) {
                llvm::errs() << "Expected ')' after expression\n";
                goto _error;
            }
            break;
        }
        default: {
            llvm::errs() << "Error: Unexpected token in parse factor\n";
            error();
            goto _error;
        }
    }

    return Left;

_error:
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

Expr *Parser::parseFinal()
{
    Expr *Res = nullptr;
    switch (Tok.getKind())
    {
    case Token::number:{
        Res = new Final(Final::Number, Tok.getText());
        advance();
        break;
    }
    case Token::floatNumber:{
        Res = new Final(Final::FloatNumber, Tok.getText());
        advance();
        break;
    }
    case Token::ident: {
        Res = new Final(Final::Ident, Tok.getText());
        Token prev_tok = Tok;
        const char* prev_buffer = Lex.getBuffer();
        Expr* u = parseUnary();
        if(u)
            return u;
        else{
            Tok = prev_tok;
            Lex.setBufferPtr(prev_buffer);
            advance();
        }
        break;
    }
    case Token::plus:{
        advance();
        if(Tok.getKind() == Token::number){
            Res = new SignedNumber(SignedNumber::Plus, Tok.getText());
            advance();
            break;
        }
        goto _error;
    }

    case Token::minus:{
        advance();
        if (Tok.getKind() == Token::number){
            Res = new SignedNumber(SignedNumber::Minus, Tok.getText());
            advance();
            break;
        }
        goto _error;
    }
    case Token::minus_paren:{
        advance();
        Expr *math_expr = parseExpr();
        if(math_expr == nullptr)
            goto _error;
        Res = new NegExpr(math_expr);
        if (!consume(Token::r_paren))
            break;
        
        goto _error;

    }
    case Token::l_paren:{
        advance();
        Res = parseExpr();
        if(Res == nullptr){
            goto _error;
        }
        if (!consume(Token::r_paren))
            break;
        
    }
    default:{
        llvm::errs() << "balaye panjomin error() dakhele parse final\n";
        error();
        goto _error;
    }
    }
    return Res;

_error:
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}
// TODO
Expr *Parser::parseTernary() {
    Logic *Condition = nullptr;  // Ensure the condition is of type Logic*
    Expr *TrueExpr = nullptr;
    Expr *FalseExpr = nullptr;

    Condition = parseLogic();  // Use parseLogic() to get a Logic* condition
    if (!Condition) goto _error;
    advance();

    if (!consume(Token::questionmark)) goto _error;
    advance();

    TrueExpr = parseExpr();
    if (!TrueExpr) goto _error;
    advance();

    if (!consume(Token::colon)) goto _error;
    advance();

    FalseExpr = parseExpr();
    if (!FalseExpr) goto _error;

    return new TernaryAssignment(Condition, TrueExpr, FalseExpr);

_error:
    while (Tok.getKind() != Token::eoi) advance();
    return nullptr;
}

Logic *Parser::parseComparison()
{
    Logic *Res = nullptr;
    Final *Ident = nullptr;
    Expr *Left = nullptr;
    Expr *Right = nullptr;
    Token prev_Tok;
    const char* prev_buffer;
    if (Tok.is(Token::l_paren)) {
        advance();
        Res = parseLogic();
        if (Res == nullptr)
        {
            goto _error;
        }
        if (consume(Token::r_paren))
            goto _error;
    }
    else {
        if(Tok.is(Token::KW_true)){
            Res = new Comparison(nullptr, nullptr, Comparison::True);
            advance();
            return Res;
        }
        else if(Tok.is(Token::KW_false)){
            Res = new Comparison(nullptr, nullptr, Comparison::False);
            advance();
            return Res;
        }
        else if(Tok.is(Token::ident)){
            Ident = new Final(Final::Ident, Tok.getText());
        }
        prev_Tok = Tok;
        prev_buffer = Lex.getBuffer();
        Left = parseExpr();
        if(Left == nullptr)
            goto _error;
        

        Comparison::Operator Op;
            if (Tok.is(Token::eq))
                Op = Comparison::Equal;
            else if (Tok.is(Token::neq))
                Op = Comparison::Not_equal;
            else if (Tok.is(Token::gt))
                Op = Comparison::Greater;
            else if (Tok.is(Token::lt))
                Op = Comparison::Less;
            else if (Tok.is(Token::gte))
                Op = Comparison::Greater_equal;
            else if (Tok.is(Token::lte))
                Op = Comparison::Less_equal;    
            else {
                if (Ident){
                    Tok = prev_Tok;
                    Lex.setBufferPtr(prev_buffer);
                    Res = new Comparison(Ident, nullptr, Comparison::Ident);
                    advance();
                    return Res;
                }
                goto _error;
            }
            advance();
            Right = parseExpr();
            if (Right == nullptr)
            {
                goto _error;
            }
            
            Res = new Comparison(Left, Right, Op);
    }
    
    return Res;

_error:
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

Logic *Parser::parseLogic()
{
    Logic *Left = parseComparison();
    Logic *Right;
    if (Left == nullptr)
    {
        goto _error;
    }
    while (Tok.isOneOf(Token::KW_and, Token::KW_or))
    {
        LogicalExpr::Operator Op;
        if (Tok.is(Token::KW_and))
            Op = LogicalExpr::And;
        else if (Tok.is(Token::KW_or))
            Op = LogicalExpr::Or;
        else {
            llvm::errs() << "balaye sheshomin error() dakhele parse logic\n";
            error();

            goto _error;
        }
        advance();
        Right = parseComparison();
        if (Right == nullptr)
        {
            goto _error;
        }
        Left = new LogicalExpr(Left, Right, Op);
    }
    return Left;

_error:
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

IfStmt *Parser::parseIf()
{
    llvm::SmallVector<AST *> ifStmts;
    llvm::SmallVector<AST *> elseStmts;
    llvm::SmallVector<elifStmt *> elifStmts;
    llvm::SmallVector<AST *> Stmts;
    Logic *Cond = nullptr;
    Token prev_token_if;
    const char* prev_buffer_if;
    Token prev_token_elif;
    const char* prev_buffer_elif;
    bool hasElif = false;
    bool hasElse = false;


    if (expect(Token::KW_if)){
        llvm::errs() <<"here in if i'm wrong\n";
        goto _error;
    }

    advance();

    if (expect(Token::l_paren)){
        llvm::errs() <<"here in ( i'm wrong\n";
        goto _error;
    }

    advance();

    Cond = parseLogic();
    if (Cond == nullptr){
        llvm::errs() <<"here in condition i'm wrong\n";
        goto _error;
    }

    if (expect(Token::r_paren)){
        llvm::errs() <<"here in ) i'm wrong\n";
        goto _error;
    }
        
    advance();

    if (expect(Token::l_brace)){
        llvm::errs() <<"here in { i'm wrong\n";
        goto _error;
    }

    advance();
    
    ifStmts = getBody();
        
    if(ifStmts.empty()){
        llvm::errs() <<"here in body i'm wrong\n";
        goto _error;
    }
    prev_token_if = Tok;
    prev_buffer_if = Lex.getBuffer();
    
    advance();

    while (true)
    {
        if (Tok.is(Token::KW_else))
        {
            advance();
            if (Tok.is(Token::KW_if))
            {
                hasElif = true;
                advance();
                
                if (expect(Token::l_paren)){
                    llvm::errs() <<"here in ( for else i'm wrong\n";
                    goto _error;
                }

                advance();

                Logic *Cond = parseLogic();

                if (Cond == nullptr)
                {
                    llvm::errs() <<"here in condition for else i'm wrong\n";
                    goto _error;
                }
                if (expect(Token::r_paren)){
                    llvm::errs() <<"here in ) for else i'm wrong\n";
                    goto _error;
                }

                advance();

                if (expect(Token::l_brace)){
                    llvm::errs() <<"here in { i'm wrong\n";
                    goto _error;
                }

                advance();

                Stmts = getBody();
                prev_token_elif = Tok;
                prev_buffer_elif = Lex.getBuffer();
                
                if(!Stmts.empty())
                    advance();
                else
                    goto _error;
                
                elifStmt *elif = new elifStmt(Cond, Stmts);
                elifStmts.push_back(elif);
            }
            else
            {
                hasElse = true;

                if (expect(Token::l_brace)){
                    llvm::errs() <<"here in { for else if i'm wrong\n";
                    goto _error;
                }

                advance();

                elseStmts = getBody();
                
                if(elseStmts.empty())
                    goto _error;

                break;
            }
        }
        else
            break;
    }

    if(hasElif && !hasElse){
        Tok = prev_token_elif;
        Lex.setBufferPtr(prev_buffer_elif);
    }
    else if(!hasElif && !hasElse){
        Tok = prev_token_if;
        Lex.setBufferPtr(prev_buffer_if);
    }
        
    return new IfStmt(Cond, ifStmts, elseStmts, elifStmts);

_error:
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}
// TODO cast Parsing
// Expr *Parser::parseCastExpr() {
//     llvm::StringRef TargetType = Tok.getText();
//     advance(); // Consume the type identifier
//     if (!consume(Token::l_paren)) {
//         llvm::errs() << "Expected '(' after type in cast\n";
//         return nullptr;
//     }

//     Expr *Operand = parseExpr();
//     if (!Operand) {
//         llvm::errs() << "Expected expression inside cast\n";
//         return nullptr;
//     }

//     if (!consume(Token::r_paren)) {
//         llvm::errs() << "Expected ')' after cast expression\n";
//         return nullptr;
//     }

//     return new CastExpr(TargetType, Operand);
// }

Expr *Parser::parseCastExpr() {
    llvm::StringRef TargetType = Tok.getText();

    // Ensure that the type is valid and followed by '('
    if (TargetType != "int" && TargetType != "float" && TargetType != "bool") {
        llvm::errs() << "Expected valid type in cast expression\n";
        return nullptr;
    }
    //advance(); // Consume the type identifier

    if (!consume(Token::l_paren)) {
        llvm::errs() << "Expected '(' after type in cast\n";
        return nullptr;
    }

    Expr *Operand = parseExpr();
    if (!Operand) {
        llvm::errs() << "Expected expression inside cast\n";
        return nullptr;
    }

    if (!consume(Token::r_paren)) {
        llvm::errs() << "Expected ')' after cast expression\n";
        return nullptr;
    }

    return new CastExpr(TargetType, Operand);
}

SwitchStmt *Parser::parseSwitch() {// TODO added for switch-case construct
    Logic *Cond = nullptr;
    llvm::SmallVector<CaseStmt *> Cases;
    DefaultStmt *Default = nullptr; // Declare Default as a pointer to DefaultStmt
    llvm::SmallVector<AST *> DefaultBody;

    if (expect(Token::KW_switch)){
        llvm::errs() << "i'm here in switch\n";
        goto _error;
    }
    advance();

    if (expect(Token::l_paren)){
            llvm::errs() << "i'm here in (\n";
            goto _error;
    }
    advance();

    Cond = parseLogic();
    if (!Cond){
        llvm::errs() << "i'm here in condition of switch\n";
        goto _error;
    }

    if (expect(Token::r_paren)){
        llvm::errs() << "i'm here in )\n";
        goto _error;
    }
    advance();

    if (expect(Token::l_brace)){
        llvm::errs() << "i'm here in {\n";
        goto _error;
    }
    advance();

    while (Tok.is(Token::KW_case)) {
        advance();

        Expr *CaseValue = parseExpr();
        if (!CaseValue){ 
            llvm::errs() << "the value of case\n";
            goto _error;
        }

        if (expect(Token::colon)){
            llvm::errs() << "here i should see a :\n";
            goto _error;
        }
        advance();

        llvm::SmallVector<AST *> CaseBody = getBody();
        Cases.push_back(new CaseStmt(CaseValue, CaseBody));
    }

    if (Tok.is(Token::KW_default)) { // TODO
        advance();
        if (expect(Token::colon)) {
            llvm::errs() << "didn't see : after default\n";
            goto _error;
        }
        advance();
        llvm::SmallVector<AST *> DefaultBody = getBody();
        Default = new DefaultStmt(DefaultBody); // Wrap the body in DefaultStmt 
        } else {
            Default = nullptr; // No default case
        }
        if (!consume(Token::r_brace)) {
            llvm::errs() << "Expected '}' at the end of switch statement\n";
            goto _error;
        }
        llvm::errs() << "alan daghighan posht in returnam\n";
        advance();
        return new SwitchStmt(Cond, Cases, Default);

_error:
    while (Tok.getKind() != Token::eoi) advance();
    return nullptr;
}

DoWhileStmt *Parser::parseDoWhile() {// TODO added for do-while loop
    llvm::SmallVector<AST *> Body;
    Logic *Cond = nullptr;

    if (expect(Token::KW_do)){ 
        llvm::errs() << "here in keyword do\n"; //TODO Debug log
        goto _error;
    }
    advance();

    if (expect(Token::l_brace)){
        llvm::errs() << "here in {\n"; //TODO Debug log
        goto _error;
    }
    advance();

    Body = getBody();
    if (Body.empty()){
        llvm::errs() << "here in body\n"; //TODO Debug log
        goto _error;
    }
    advance();

    if (expect(Token::KW_while)){
        llvm::errs() << "here in keyword while\n"; //TODO Debug log
        goto _error;
    }
    advance();

    if (expect(Token::l_paren)){
        llvm::errs() << "here in (\n"; //TODO Debug log
        goto _error;
    }
    advance();

    Cond = parseLogic();
    if (!Cond){
        llvm::errs() << "here in condition of while\n"; //TODO Debug log
        goto _error;
    }

    if (expect(Token::r_paren)){
        llvm::errs() << "here in )\n"; //TODO Debug log
        goto _error;
    }
    advance();

    if (expect(Token::semicolon)){
        llvm::errs() << "here in semicolon\n"; //TODO Debug log
        goto _error;
    }
    return new DoWhileStmt(Body, Cond);

_error:
    while (Tok.getKind() != Token::eoi) advance();
    return nullptr;
}
// TODO min, max, mean, sqrtN
// Expr *Parser::parseFunctionCall() {
//     std::string FunctionName = Tok.getText().str(); // Store function name
//     advance(); // Move past the function name

//     llvm::SmallVector<Expr *> Arguments; // Arguments list

//     // Ensure the next token is '('
//     if (!Tok.is(Token::l_paren)) {
//         llvm::errs() << "Expected '(' after function name\n";
//         goto _error;
//     }
//     advance(); // Consume '('

//     // Parse arguments
//     if (!Tok.is(Token::r_paren)) { // Check if there are arguments
//         do {
//             Expr *Arg = parseExpr(); // Parse each argument
//             if (!Arg) {
//                 llvm::errs() << "Expected an expression as an argument\n";
//                 goto _error;
//             }
//             Arguments.push_back(Arg);

//             if (!Tok.is(Token::comma)) break; // Break if no comma
//             advance(); // Consume comma
//         } while (true);
//     }

//     // Ensure the next token is ')'
//     if (!Tok.is(Token::r_paren)) {
//         llvm::errs() << "Expected ')' after function arguments\n";
//         goto _error;
//     }
//     advance(); // Consume ')'

//     return new FunctionCall(FunctionName, Arguments); // Create function call AST node

// _error:
//     while (Tok.getKind() != Token::eoi) advance();
//     return nullptr;
// }
Expr *Parser::parseFunctionCall() {
    llvm::errs() << "Parsing function call, function name: " << Tok.getText() << "\n";
std::string FunctionName = Tok.getText().str();
llvm::errs() << "Captured function name: " << FunctionName << "\n";

    llvm::SmallVector<Expr *> Arguments;

    if (!consume(Token::l_paren)) {
        llvm::errs() << "Expected '(' after function name\n";
        goto _error;
    }
    advance();
    advance();
    // Expect the first float number
    if (Tok.is(Token::floatNumber)) {
        Arguments.push_back(new Final(Final::FloatNumber, Tok.getText()));
        advance(); // Move to next token
    } else {
        llvm::errs() << "Expected a float number as the first argument, got: " << Tok.getText() << "\n";
        goto _error;
    }

    // Expect a comma
    if (!Tok.is(Token::comma)) {
        llvm::errs() << "Expected ',' after the first argument, got: " << Tok.getText() << "\n";
        goto _error;
    }
    advance(); // Consume the comma

    // Expect the second float number
    if (Tok.is(Token::floatNumber)) {
        Arguments.push_back(new Final(Final::FloatNumber, Tok.getText()));
        advance(); // Move to next token
    } else {
        llvm::errs() << "Expected a float number as the second argument, got: " << Tok.getText() << "\n";
        goto _error;
    }

    // Expect a closing parenthesis
    if (!Tok.is(Token::r_paren)) {
        llvm::errs() << "Expected ')' after the arguments, got: " << Tok.getText() << "\n";
        goto _error;
    }
    advance(); // Consume the closing parenthesis

    return new FunctionCall(FunctionName, Arguments);

_error:
    while (Tok.getKind() != Token::eoi) advance();
    return nullptr;
}


PrintStmt *Parser::parsePrint()
{
    llvm::StringRef Var;
    if (expect(Token::KW_print)){
        goto _error;
    }
    advance();
    if (expect(Token::l_paren)){
        goto _error;
    }
    advance();
    if (expect(Token::ident)){
        goto _error;
    }
    Var = Tok.getText();
    advance();
    if (expect(Token::r_paren)){
        goto _error;
    }
    advance();
    if (expect(Token::semicolon)){
        goto _error;
    }
    return new PrintStmt(Var);

_error:
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;

}

WhileStmt *Parser::parseWhile()
{
    llvm::SmallVector<AST *> Body;
    Logic *Cond = nullptr;

    if (expect(Token::KW_while)){
        goto _error;
    }
        
    advance();

    if(expect(Token::l_paren)){
        goto _error;
    }

    advance();

    Cond = parseLogic();
    if (Cond == nullptr)
    {
        goto _error;
    }
    if(expect(Token::r_paren)){
        goto _error;
    }

    advance();

    if (expect(Token::l_brace)){
        goto _error;
    }

    advance();

    Body = getBody();
    if(Body.empty())
        goto _error;
        

    return new WhileStmt(Cond, Body);

_error:
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;
}

ForStmt *Parser::parseFor()
{
    Assignment *First = nullptr;
    Logic *Second = nullptr;
    Assignment *ThirdAssign = nullptr;
    UnaryOp *ThirdUnary = nullptr;
    llvm::SmallVector<AST *> Body;
    Token prev_token;
    const char* prev_buffer;

    if (expect(Token::KW_for)){
        goto _error;
    }
        
    advance();

    if(expect(Token::l_paren)){
        goto _error;
    }

    advance();

    First = parseIntAssign();

    if (First == nullptr)
        goto _error;
        
    if (First->getAssignKind() != Assignment::Assign)    // The first part can only have a '=' sign
        goto _error;

    if(expect(Token::semicolon)){
        goto _error;
    }

    advance();

    Second = parseLogic();

    if (Second == nullptr)
        goto _error;
        
    if(expect(Token::semicolon)){
        goto _error;
    }

    advance();

    prev_token = Tok;
    prev_buffer = Lex.getBuffer();

    ThirdAssign = parseIntAssign();

    if (ThirdAssign == nullptr){
        Tok = prev_token;
        Lex.setBufferPtr(prev_buffer);

        ThirdUnary = parseUnary();
        if (ThirdUnary == nullptr){
            goto _error;
        }

    }
    else{
        if(ThirdAssign->getAssignKind() == Assignment::Assign)   // The third part cannot have only '=' sign
            goto _error;
    }


    if(expect(Token::r_paren)){
        goto _error;
    }

    advance();

    if(expect(Token::l_brace)){
        goto _error;
    }

    advance();

    Body = getBody();

    if (Body.empty())
        goto _error;

    return new ForStmt(First, Second, ThirdAssign, ThirdUnary, Body);

_error:
    while (Tok.getKind() != Token::eoi)
        advance();
    return nullptr;  

}

BreakStmt *Parser::parseBreak() { //TODO
    // Expect and consume the 'break' keyword
    if (expect(Token::KW_break)) {
        llvm::errs() << "Expected 'break' keyword but not found\n";
        goto _error;
    }
    advance();

    // Expect and consume a semicolon after the 'break' keyword
    if (expect(Token::semicolon)) {
        llvm::errs() << "Expected ';' after 'break'\n";
        goto _error;
    }

    return new BreakStmt(getCurrentTokenLocation());

_error:
    // Error handling: consume tokens until end-of-input or recovery point
    while (Tok.getKind() != Token::eoi) {
        llvm::errs() << "it was at this moment that he knew he fucked up\n";
        advance();
    }
    return nullptr;
}

ContinueStmt *Parser::parseContinue() { // TODO
    // Expect and consume the 'continue' keyword
    if (expect(Token::KW_continue)) {
        llvm::errs() << "Expected 'continue' keyword but not found\n";
        goto _error;
    }
    advance();

    // Expect and consume a semicolon after the 'continue' keyword
    if (expect(Token::semicolon)) {
        llvm::errs() << "Expected ';' after 'continue'\n";
        goto _error;
    }
    advance();

    return new ContinueStmt(getCurrentTokenLocation());

_error:
    // Error handling: consume tokens until end-of-input or recovery point
    while (Tok.getKind() != Token::eoi) {
        advance();
    }
    return nullptr;
}

Location Parser::getCurrentTokenLocation() {// TODO
    // Assuming `Tok` contains current token details and has methods like `getLine()` and `getColumn()`
    return Location(Tok.getLine(), Tok.getColumn());
}

void Parser::parseComment()
{
    if (expect(Token::start_comment)) {
        goto _error;
    }
    advance();

    while (!Tok.isOneOf(Token::end_comment, Token::eoi)) advance();

    return;
_error: 
    while (Tok.getKind() != Token::eoi)
        advance();
}


llvm::SmallVector<AST *> Parser::getBody()
{
    llvm::SmallVector<AST *> body;
    while (!Tok.is(Token::r_brace)) {
        switch (Tok.getKind()) {
        case Token::ident: {
            Token prev_token = Tok;
            const char *prev_buffer = Lex.getBuffer();
            UnaryOp *u;
            u = parseUnary();
            if (Tok.is(Token::semicolon)) {
                if (u) {
                    body.push_back(u);
                    break;
                } else {
                    goto _error;
                }
            } else {
                if (u) {
                    goto _error;
                } else {
                    Tok = prev_token;
                    Lex.setBufferPtr(prev_buffer);
                }
            }

            Assignment *a_int;
            Assignment *a_bool;
            prev_token = Tok;
            prev_buffer = Lex.getBuffer();

            a_bool = parseBoolAssign();
            if (a_bool) {
                body.push_back(a_bool);
                break;
            }
            Tok = prev_token;
            Lex.setBufferPtr(prev_buffer);

            a_int = parseIntAssign();
            if (a_int) {
                body.push_back(a_int);
            } else {
                goto _error;
            }
            if (!Tok.is(Token::semicolon)) {
                goto _error;
            }
            break;
        }
        case Token::KW_if: {
            IfStmt *i = parseIf();
            if (i) {
                body.push_back(i);
            } else {
                goto _error;
            }
            break;
        }
        case Token::KW_while: {
            WhileStmt *w = parseWhile();
            if (w) {
                body.push_back(w);
            } else {
                goto _error;
            }
            break;
        }
        case Token::KW_for: {
            ForStmt *f = parseFor();
            if (f) {
                body.push_back(f);
            } else {
                goto _error;
            }
            break;
        }
        case Token::KW_do: { // TODO: Added for do-while loop
            DoWhileStmt *dwhile = parseDoWhile();
            if (dwhile) {
                body.push_back(dwhile);
            } else {
                goto _error;
            }
            break;
        }
        case Token::KW_switch: { // TODO: Added for switch-case
            SwitchStmt *sswitch = parseSwitch();
            if (sswitch) {
                body.push_back(sswitch);
            } else {
                goto _error;
            }
            break;
        }
        case Token::KW_break: { // TODO: Added for break
            BreakStmt *b = parseBreak();
            if (b) {
                llvm::errs() << "inja ham miam ke agha sadra eshgh kone\n";
                body.push_back(b);
            } else {
                goto _error;
            }
            break;
        }
        case Token::KW_continue: { // TODO: Added for continue
            ContinueStmt *c = parseContinue();
            if (c) {
                body.push_back(c);
            } else {
                goto _error;
            }
            break;
        }
        case Token::KW_print: {
            PrintStmt *p = parsePrint();
            if (p) {
                body.push_back(p);
            } else {
                goto _error;
            }
            break;
        }
        case Token::start_comment: {
            parseComment();
            if (!Tok.is(Token::end_comment)) {
                goto _error;
            }
            break;
        }
        default:{
            goto _error;
            break;
        }
        }
        advance();
    }
    if (Tok.is(Token::r_brace)) {
        return body;
    }

_error:
    while (Tok.getKind() != Token::eoi) advance();
    return body;
}

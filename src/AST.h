#ifndef AST_H
#define AST_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"

// Forward declarations of classes used in the AST
class AST;
class Expr;
class Program;
class DeclarationInt;
class DeclarationBool;
class DeclarationFloat;  // TODO: Added forward declaration for float variable declaration
class DeclarationVar;    // TODO: Added forward declaration for var variable declaration
class DeclarationConst;  // TODO: Added forward declaration for const variable declaration
class DefineStmt;        // TODO: Added forward declaration for #define statement
class Final;
class BinaryOp;
class UnaryOp;
class SignedNumber;
class NegExpr;
class Assignment;
class Logic;
class Comparison;
class LogicalExpr;
class IfStmt;
class WhileStmt;
class elifStmt;
class ForStmt;
class PrintStmt;
class DoWhileStmt;       // TODO: Added forward declaration for do-while statement
class SwitchStmt;        // TODO: Added forward declaration for switch-case statement
class CaseStmt;          // TODO: Added forward declaration for a single case in switch-case
class FunctionCall;      // TODO: Added forward declaration for function call (min, max, mean, sqrtN)


// ASTVisitor class defines a visitor pattern to traverse the AST
class ASTVisitor
{
public:
  // Virtual visit functions for each AST node type
  virtual void visit(AST &) {}               // Visit the base AST node
  virtual void visit(Expr &) {}              // Visit the expression node
  virtual void visit(Logic &) {}             // Visit the Logic node
  virtual void visit(Program &) {};          // Visit the group of expressions node
  virtual void visit(Final &) = 0;           // Visit the Final node
  virtual void visit(BinaryOp &) = 0;        // Visit the binary operation node
  virtual void visit(UnaryOp &) = 0;
  virtual void visit(SignedNumber &) = 0;
  virtual void visit(NegExpr &) = 0;
  virtual void visit(Assignment &) = 0;      // Visit the assignment expression node
  virtual void visit(DeclarationInt &) = 0;     // Visit the variable declaration node
  virtual void visit(DeclarationBool &) = 0;     // Visit the variable declaration node
  virtual void visit(DeclarationFloat &) = 0; // TODO added
  virtual void visit(DeclarationVar &) = 0;   // TODO added
  virtual void visit(DeclarationConst &) = 0; // TODO added
  virtual void visit(Comparison &) = 0;      // Visit the Comparison node
  virtual void visit(LogicalExpr &) = 0;     // Visit the LogicalExpr node
  virtual void visit(IfStmt &) = 0;          // Visit the IfStmt node
  virtual void visit(WhileStmt &) = 0;        // Visit the IterStmt node
  virtual void visit(elifStmt &) = 0;        // Visit the elifStmt node
  virtual void visit(ForStmt &) = 0;
  virtual void visit(PrintStmt &) = 0;
  virtual void visit(DefineStmt &) = 0;       // TODO added
  virtual void visit(SwitchStmt &) = 0;      // TODO added
  virtual void visit(CaseStmt &) = 0;        // TODO added
  virtual void visit(DoWhileStmt &) = 0;     // TODO added
  virtual void visit(FunctionCall &) = 0;    // TODO added
};

// AST class serves as the base class for all AST nodes
class AST
{
public:
  virtual ~AST() {}
  virtual void accept(ASTVisitor &V) = 0;    // Accept a visitor for traversal
};

// Expr class represents an expression in the AST
class Expr : public AST
{
public:
  Expr() {}
};

class Logic : public AST
{
public:
  Logic() {}
};

// Program class represents a group of expressions in the AST
class Program : public AST
{
  using dataVector = llvm::SmallVector<AST *>;

private:
  dataVector data;                          // Stores the list of expressions

public:
  Program(llvm::SmallVector<AST *> data) : data(data) {}
  Program() = default;

  llvm::SmallVector<AST *> getdata() { return data; }

  dataVector::const_iterator begin() { return data.begin(); }

  dataVector::const_iterator end() { return data.end(); }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

// Declaration class represents a variable declaration with an initializer in the AST
class DeclarationInt : public Program
{
  using VarVector = llvm::SmallVector<llvm::StringRef>;
  using ValueVector = llvm::SmallVector<Expr *>;
  VarVector Vars;                           // Stores the list of variables
  ValueVector Values;                       // Stores the list of initializers

public:
  // Declaration(llvm::SmallVector<llvm::StringRef> Vars, Expr *E) : Vars(Vars), E(E) {}
  DeclarationInt(llvm::SmallVector<llvm::StringRef> Vars, llvm::SmallVector<Expr *> Values) : Vars(Vars), Values(Values) {}

  VarVector::const_iterator varBegin() { return Vars.begin(); }

  VarVector::const_iterator varEnd() { return Vars.end(); }

  ValueVector::const_iterator valBegin() { return Values.begin(); }

  ValueVector::const_iterator valEnd() { return Values.end(); }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

// Declaration class represents a variable declaration with an initializer in the AST
class DeclarationBool : public Program
{
  using VarVector = llvm::SmallVector<llvm::StringRef>;
  using ValueVector = llvm::SmallVector<Logic *>;
  VarVector Vars;                           // Stores the list of variables
  ValueVector Values;                       // Stores the list of initializers

public:
  // Declaration(llvm::SmallVector<llvm::StringRef> Vars, Expr *E) : Vars(Vars), E(E) {}
  DeclarationBool(llvm::SmallVector<llvm::StringRef> Vars, llvm::SmallVector<Logic *> Values) : Vars(Vars), Values(Values) {}

  VarVector::const_iterator varBegin() { return Vars.begin(); }

  VarVector::const_iterator varEnd() { return Vars.end(); }

  ValueVector::const_iterator valBegin() { return Values.begin(); }

  ValueVector::const_iterator valEnd() { return Values.end(); }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

// DeclarationFloat represents a float variable declaration in the AST
class DeclarationFloat : public Program { // TODO added for float type
    using VarVector = llvm::SmallVector<llvm::StringRef>;
    using ValueVector = llvm::SmallVector<Expr *>;
    VarVector Vars;
    ValueVector Values;

public:
    DeclarationFloat(llvm::SmallVector<llvm::StringRef> Vars, llvm::SmallVector<Expr *> Values) 
        : Vars(Vars), Values(Values) {}

    VarVector::const_iterator varBegin() { return Vars.begin(); }
    VarVector::const_iterator varEnd() { return Vars.end(); }
    ValueVector::const_iterator valBegin() { return Values.begin(); }
    ValueVector::const_iterator valEnd() { return Values.end(); }

    virtual void accept(ASTVisitor &V) override {
        V.visit(*this); // TODO: Add a visitor method for DeclarationFloat
    }
};

// DeclarationVar represents a var (dynamically typed) variable declaration
class DeclarationVar : public Program { // TODO added for var type
    llvm::StringRef Var;
    Expr *Value;

public:
    DeclarationVar(llvm::StringRef Var, Expr *Value) : Var(Var), Value(Value) {}

    llvm::StringRef getVar() { return Var; }
    Expr *getValue() { return Value; }

    virtual void accept(ASTVisitor &V) override {
        V.visit(*this); // TODO: Add a visitor method for DeclarationVar
    }
};

// DeclarationConst represents a constant variable declaration
class DeclarationConst : public Program { // TODO added for constant variables
    llvm::StringRef Var;
    Expr *Value;

public:
    DeclarationConst(llvm::StringRef Var, Expr *Value) : Var(Var), Value(Value) {}

    llvm::StringRef getVar() { return Var; }
    Expr *getValue() { return Value; }

    virtual void accept(ASTVisitor &V) override {
        V.visit(*this); // TODO: Add a visitor method for DeclarationConst
    }
};

// Final class represents a Final in the AST (either an identifier or a number or true or false)
class Final : public Expr
{
public:
  enum ValueKind
  {
    Ident,
    Number
  };

private:
  ValueKind Kind;                            // Stores the kind of Final (identifier or number or true or false)
  llvm::StringRef Val;                       // Stores the value of the Final

public:
  Final(ValueKind Kind, llvm::StringRef Val) : Kind(Kind), Val(Val) {}

  ValueKind getKind() { return Kind; }

  llvm::StringRef getVal() { return Val; }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

// BinaryOp class represents a binary operation in the AST (plus, minus, multiplication, division)
class BinaryOp : public Expr
{
public:
  enum Operator
  {
    Plus,
    Minus,
    Mul,
    Div,
    Mod,
    Exp
  };

private:
  Expr *Left;                               // Left-hand side expression
  Expr *Right;                              // Right-hand side expression
  Operator Op;                              // Operator of the binary operation

public:
  BinaryOp(Operator Op, Expr *L, Expr *R) : Op(Op), Left(L), Right(R) {}

  Expr *getLeft() { return Left; }

  Expr *getRight() { return Right; }

  Operator getOperator() { return Op; }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

// naryOp class represents a unary operation in the AST (plus plus, minus minus)
class UnaryOp : public Expr
{
public:
  enum Operator
  {
    Plus_plus,
    Minus_minus
  };

private:
  llvm::StringRef Ident;                      
  Operator Op;                              // Operator of the unary operation

public:
  UnaryOp(Operator Op, llvm::StringRef I) : Op(Op), Ident(I) {}

  llvm::StringRef getIdent() { return Ident; }

  Operator getOperator() { return Op; }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

class SignedNumber : public Expr
{
public:
  enum Sign
  {
    Plus,
    Minus
  };

private:
  llvm::StringRef Value;                              
  Sign s;                              

public:
  SignedNumber(Sign S, llvm::StringRef V) : s(S), Value(V) {}

  llvm::StringRef getValue() { return Value; }

  Sign getSign() { return s; }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

class NegExpr : public Expr
{

private:
  Expr *expr;                              

public:
  NegExpr(Expr *E) : expr(E) {}

  Expr *getExpr() { return expr; }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

// Assignment class represents an assignment expression in the AST
class Assignment : public Program
{
  public:
  enum AssignKind
  {
    Assign,         // =
    Minus_assign,   // -=
    Plus_assign,    // +=
    Star_assign,    // *=
    Slash_assign,   // /=
};
private:
  Final *Left;                             // Left-hand side Final (identifier)
  Expr *RightExpr;                         // Right-hand side expression
  Logic *RightLogicExpr;                   // Right-hand side logical expression
  AssignKind AK;                           // Kind of assignment

public:
  Assignment(Final *L, Expr *RE, AssignKind AK, Logic *RL) : Left(L), RightExpr(RE), AK(AK), RightLogicExpr(RL) {}

  Final *getLeft() { return Left; }

  Expr *getRightExpr() { return RightExpr; }

  Logic *getRightLogic() { return RightLogicExpr; }

  AssignKind getAssignKind() { return AK; }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

// Comparison class represents a comparison expression in the AST
class Comparison : public Logic
{
  public:
  enum Operator
  {
    Equal,          // ==
    Not_equal,      // !=
    Greater,        // >
    Less,           // <
    Greater_equal,  // >=
    Less_equal,     // <=
    True,           //CHECK???
    False,
    Ident           // only one boolean ident
  };
    
private:
  Expr *Left;                                // Left-hand side expression
  Expr *Right;                               // Right-hand side expression
  Operator Op;                               // Kind of assignment

public:
  Comparison(Expr *L, Expr *R, Operator Op) : Left(L), Right(R), Op(Op) {}

  Expr *getLeft() { return Left; }

  Expr *getRight() { return Right; }

  Operator getOperator() { return Op; }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

// LogicalExpr class represents a logical expression in the AST
class LogicalExpr : public Logic
{
  public:
  enum Operator
  {
    And,          // &&
    Or,           // ||
  };

private:
  Logic *Left;                                // Left-hand side expression
  Logic *Right;                               // Right-hand side expression
  Operator Op;                                // Kind of assignment

public:
  LogicalExpr(Logic *L, Logic *R, Operator Op) : Left(L), Right(R), Op(Op) {}

  Logic *getLeft() { return Left; }

  Logic *getRight() { return Right; }

  Operator getOperator() { return Op; }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

class elifStmt : public Program
{
  using Stmts = llvm::SmallVector<AST *>;

private:
  Stmts S;
  Logic *Cond;

public:
  elifStmt(Logic *Cond, llvm::SmallVector<AST *> S) : Cond(Cond), S(S) {}

  Logic *getCond() { return Cond; }

  Stmts::const_iterator begin() { return S.begin(); }

  Stmts::const_iterator end() { return S.end(); }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }

};

class IfStmt : public Program
{
using BodyVector = llvm::SmallVector<AST *>;
using elifVector = llvm::SmallVector<elifStmt *>;

private:
  BodyVector ifStmts;
  elifVector elifStmts;
  BodyVector elseStmts;
  Logic *Cond;

public:
  IfStmt(Logic *Cond, llvm::SmallVector<AST *> ifStmts, llvm::SmallVector<AST *> elseStmts, llvm::SmallVector<elifStmt *> elifStmts) : Cond(Cond), ifStmts(ifStmts), elseStmts(elseStmts), elifStmts(elifStmts) {}

  Logic *getCond() { return Cond; }

  BodyVector::const_iterator begin() { return ifStmts.begin(); }

  BodyVector::const_iterator end() { return ifStmts.end(); }

  BodyVector::const_iterator beginElse() { return elseStmts.begin(); }

  BodyVector::const_iterator endElse() { return elseStmts.end(); }

  elifVector::const_iterator beginElif() { return elifStmts.begin(); }

  elifVector::const_iterator endElif() { return elifStmts.end(); }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

class WhileStmt : public Program
{
using BodyVector = llvm::SmallVector<AST *>;
BodyVector Body;

private:
  Logic *Cond;

public:
  WhileStmt(Logic *Cond, llvm::SmallVector<AST *> Body) : Cond(Cond), Body(Body) {}

  Logic *getCond() { return Cond; }

  BodyVector::const_iterator begin() { return Body.begin(); }

  BodyVector::const_iterator end() { return Body.end(); }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

// DefineStmt represents a #define macro in the AST
class DefineStmt : public Program { // TODO added for #define macros
    llvm::StringRef MacroName;
    llvm::StringRef MacroValue;

public:
    DefineStmt(llvm::StringRef MacroName, llvm::StringRef MacroValue)
        : MacroName(MacroName), MacroValue(MacroValue) {}

    llvm::StringRef getMacroName() { return MacroName; }
    llvm::StringRef getMacroValue() { return MacroValue; }

    virtual void accept(ASTVisitor &V) override {
        V.visit(*this); // TODO: Add a visitor method for DefineStmt
    }
};

// CaseStmt represents a single case in a switch statement
class CaseStmt : public AST {
    Expr *CaseValue;
    llvm::SmallVector<AST *> Body;

public:
    CaseStmt(Expr *CaseValue, llvm::SmallVector<AST *> Body)
        : CaseValue(CaseValue), Body(Body) {}

    Expr *getCaseValue() { return CaseValue; }
    llvm::SmallVector<AST *> getBody() { return Body; }

    virtual void accept(ASTVisitor &V) override {
        V.visit(*this); // TODO: Add a visitor method for CaseStmt
    }
};

// SwitchStmt represents a switch-case construct in the AST
class SwitchStmt : public Program { // TODO added for switch-case construct
    Logic *Cond;
    llvm::SmallVector<CaseStmt *> Cases;
    llvm::SmallVector<AST *> DefaultBody;

public:
    SwitchStmt(Logic *Cond, llvm::SmallVector<CaseStmt *> Cases, llvm::SmallVector<AST *> DefaultBody)
        : Cond(Cond), Cases(Cases), DefaultBody(DefaultBody) {}

    Logic *getCond() { return Cond; }
    llvm::SmallVector<CaseStmt *> getCases() { return Cases; }
    llvm::SmallVector<AST *> getDefaultBody() { return DefaultBody; }

    virtual void accept(ASTVisitor &V) override {
        V.visit(*this); // TODO: Add a visitor method for SwitchStmt
    }
};

// DoWhileStmt represents a do-while loop in the AST
class DoWhileStmt : public Program { // TODO added for do-while loop
    llvm::SmallVector<AST *> Body;
    Logic *Cond;

public:
    DoWhileStmt(llvm::SmallVector<AST *> Body, Logic *Cond) 
        : Body(Body), Cond(Cond) {}

    llvm::SmallVector<AST *> getBody() { return Body; }
    Logic *getCond() { return Cond; }

    virtual void accept(ASTVisitor &V) override {
        V.visit(*this); // TODO: Add a visitor method for DoWhileStmt
    }
};

// FunctionCall represents a function call in the AST
class FunctionCall : public Expr { // TODO added for function calls like min(), max(), mean(), sqrtN()
    llvm::StringRef FuncName;
    llvm::SmallVector<Expr *> Args;

public:
    FunctionCall(llvm::StringRef FuncName, llvm::SmallVector<Expr *> Args)
        : FuncName(FuncName), Args(Args) {}

    llvm::StringRef getFuncName() { return FuncName; }
    llvm::SmallVector<Expr *> getArgs() { return Args; }

    virtual void accept(ASTVisitor &V) override {
        V.visit(*this); // TODO: Add a visitor method for FunctionCall
    }
};

class ForStmt : public Program
{
using BodyVector = llvm::SmallVector<AST *>;
BodyVector Body;

private:
  Assignment *First;
  Logic *Second;
  Assignment *ThirdAssign;
  UnaryOp *ThirdUnary;


public:
  ForStmt(Assignment *First, Logic *Second, Assignment *ThirdAssign, UnaryOp* ThirdUnary, llvm::SmallVector<AST *> Body) : First(First), Second(Second), ThirdAssign(ThirdAssign), ThirdUnary(ThirdUnary), Body(Body) {}

  Assignment *getFirst() { return First; }

  Logic *getSecond() { return Second; }

  Assignment *getThirdAssign() { return ThirdAssign; }

  UnaryOp *getThirdUnary() { return ThirdUnary; }

  BodyVector::const_iterator begin() { return Body.begin(); }

  BodyVector::const_iterator end() { return Body.end(); }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

class PrintStmt : public Program
{
private:
  llvm::StringRef Var;
  
public:
  PrintStmt(llvm::StringRef Var) : Var(Var) {}

  llvm::StringRef getVar() { return Var; }

  virtual void accept(ASTVisitor &V) override
  {
    V.visit(*this);
  }
};

#endif
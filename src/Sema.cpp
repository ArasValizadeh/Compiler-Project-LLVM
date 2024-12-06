#include "Sema.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/Support/raw_ostream.h"


namespace nms{
class InputCheck : public ASTVisitor {
  llvm::StringSet<> IntScope; // StringSet to store declared int variables
  llvm::StringSet<> BoolScope;
  llvm::StringSet<> FloatScope;// TODO: add float scope
  llvm::StringSet<> VarScope; // TODO: add var scope
  llvm::StringSet<> ConstScope; // TODO: add const scope
  llvm::StringSet<> DefineScope; // TODO: add define scope
  llvm::StringSet<> FunctionScope; // TODO: add function scope
  bool HasError; // Flag to indicate if an error occurred

  enum ErrorType { Twice, Not }; // Enum to represent error types: Twice - variable declared twice, Not - variable not declared

  void locationError(const Location &Loc, llvm::StringRef Msg) {
    llvm::errs() << "Error at Line " << Loc.Line
                 << ", Column " << Loc.Column << ": " << Msg << "\n";
    HasError = true;
  }

  
  private:
    int LoopDepth = 0; // TODO Counter for tracking loop depth
    int SwitchDepth = 0; // TODO Tracks switch-case nesting level


  void error(ErrorType ET, llvm::StringRef V) {
    // Function to report errors
    llvm::errs() << "Variable " << V << " is "
                 << (ET == Twice ? "already" : "not")
                 << " declared\n";
    HasError = true; // Set error flag to true
  }

public:
  InputCheck() : HasError(false) {
      // Initialize predefined functions
    FunctionScope.insert("min");// TODO: add implementation
    FunctionScope.insert("max");// TODO: add implementation
    FunctionScope.insert("mean");// TODO: add implementation
    FunctionScope.insert("sqrtN");// TODO: add implementation
  } // Constructor

  bool hasError() { return HasError; } // Function to check if an error occurred

  // Visit function for Program nodes
  virtual void visit(Program &Node) override { 

    for (llvm::SmallVector<AST *>::const_iterator I = Node.begin(), E = Node.end(); I != E; ++I)
    {
      (*I)->accept(*this); // Visit each child node
    }
  };

  virtual void visit(AST &Node) override {
    Node.accept(*this);
  }

  // Visit function for Final nodes
  virtual void visit(Final &Node) override {
    if (Node.getKind() == Final::Ident) {
      // Check if identifier is in the scope
      if (IntScope.find(Node.getVal()) == IntScope.end() && BoolScope.find(Node.getVal()) == BoolScope.end() && FloatScope.find(Node.getVal()) == FloatScope.end() && VarScope.find(Node.getVal()) == VarScope.end() && DefineScope.find(Node.getVal()) == DefineScope.end()){
        llvm::errs() << "node value is: " << Node.getVal() << " and it is of kind: " << Node.getKind() <<"\n";

        error(Not, Node.getVal());
      }
    }
  };

  // Visit function for BinaryOp nodes
  virtual void visit(BinaryOp &Node) override {
    Expr* right = Node.getRight();
    Expr* left = Node.getLeft();
    if (left)
      left->accept(*this);
    else
      HasError = true;

    if (right)
      right->accept(*this);
    else
      HasError = true;

    Final* l = (Final*)left;
    if (l->getKind() == Final::Ident){
      if (BoolScope.find(l->getVal()) != BoolScope.end()) {
        llvm::errs() << "Cannot use binary operation on a boolean variable: " << l->getVal() << "\n";
        HasError = true;
      }
    }

    Final* r = (Final*)right;
    if (r->getKind() == Final::Ident){
      if (BoolScope.find(r->getVal()) != BoolScope.end()) {
        llvm::errs() << "Cannot use binary operation on a boolean variable: " << r->getVal() << "\n";
        HasError = true;
      }
    }
    

    if (Node.getOperator() == BinaryOp::Operator::Div || Node.getOperator() == BinaryOp::Operator::Mod ) {
      Final* f = (Final*)right;

      if (f->getKind() == Final::ValueKind::Number) {
        llvm::StringRef intval = f->getVal();

        if (intval == "0") {
          llvm::errs() << "Division by zero is not allowed." << "\n";
          HasError = true;
        }
      }
    }
    
  };

  // Visit function for Assignment nodes
 virtual void visit(Assignment &Node) override {
    Final *dest = Node.getLeft();
    Expr *RightExpr;
    Logic *RightLogic;

    dest->accept(*this);

    if (dest->getKind() == Final::Number) {
        llvm::errs() << "Assignment destination must be an identifier, not a number.";
        HasError = true;
    }

    if (BoolScope.find(dest->getVal()) != BoolScope.end()) {
      RightLogic = Node.getRightLogic();
      if (RightLogic){
        RightLogic->accept(*this);
        if(Node.getAssignKind() != Assignment::AssignKind::Assign){
          llvm::errs() << "Cannot use mathematical operation on boolean variable: " << dest->getVal() << "\n";
          HasError = true;
        }
      }
      else{
        llvm::errs() << "you should assign a boolean value to boolean variable: " << dest->getVal() << "\n";
        HasError = true;
      }
    }
      
    else if (IntScope.find(dest->getVal()) != IntScope.end()){
      RightExpr = Node.getRightExpr();
      RightLogic = Node.getRightLogic();
      if (RightExpr){
        RightExpr->accept(*this);
      }
      else if(RightLogic){
        RightLogic->accept(*this);
        Comparison* RL = (Comparison*) RightLogic;
        if (RL){
          if (RL->getOperator() == Comparison::Ident){
            Final* F = (Final*)(RL->getLeft());
            if (IntScope.find(F->getVal()) == IntScope.end()) {
              llvm::errs() << "you should assign an integer value to an integer variable: " << dest->getVal() << "\n";
              HasError = true;
            } 
          }
          else{
            llvm::errs() << "you should assign an integer value to an integer variable: " << dest->getVal() << "\n";
            HasError = true;
          }
        }
        
      }
      else{
        llvm::errs() << "you should assign an integer value to an integer variable: " << dest->getVal() << "\n";
        HasError = true;
      }
        
    }
    
    
    if (Node.getAssignKind() == Assignment::AssignKind::Slash_assign) {

      Final* f = (Final*)(RightExpr);
      if (f)
      {
        if (f->getKind() == Final::ValueKind::Number) {
        llvm::StringRef intval = f->getVal();

        if (intval == "0") {
          llvm::errs() << "Division by zero is not allowed." << "\n";
          HasError = true;
        }
        }
      }
    }
  };

  virtual void visit(DeclarationInt &Node) override {
    for (llvm::SmallVector<Expr *>::const_iterator I = Node.valBegin(), E = Node.valEnd(); I != E; ++I){
      (*I)->accept(*this); // If the Declaration node has an expression, recursively visit the expression node
    }
    for (llvm::SmallVector<llvm::StringRef>::const_iterator I = Node.varBegin(), E = Node.varEnd(); I != E;
         ++I) {
      if(BoolScope.find(*I) != BoolScope.end()){
        llvm::errs() << "Variable " << *I << " is already declared as an boolean" << "\n";
        HasError = true; 
      }
      else if(FloatScope.find(*I) != FloatScope.end()){
        llvm::errs() << "Variable " << *I << " is already declared as an float" << "\n";
        HasError = true; 
      }
      if(VarScope.find(*I) != VarScope.end()){
        llvm::errs() << "Variable " << *I << " is already declared as an variable" << "\n";
        HasError = true; 
      }
      else{
        if (!IntScope.insert(*I).second)
          error(Twice, *I); // If the insertion fails (element already exists in Scope), report a "Twice" error
      }
    }
  };

  virtual void visit(DeclarationFloat &Node) override {
    for (llvm::SmallVector<Expr *>::const_iterator I = Node.valBegin(), E = Node.valEnd(); I != E; ++I){
      (*I)->accept(*this); // If the Declaration node has an expression, recursively visit the expression node
    }
    for (llvm::SmallVector<llvm::StringRef>::const_iterator I = Node.varBegin(), E = Node.varEnd(); I != E;
         ++I) {
      if(BoolScope.find(*I) != BoolScope.end()){
        llvm::errs() << "Variable " << *I << " is already declared as an boolean" << "\n";
        HasError = true; 
      }
      else if(IntScope.find(*I) != IntScope.end()){
        llvm::errs() << "Variable " << *I << " is already declared as an integer" << "\n";
        HasError = true; 
      }
      if(VarScope.find(*I) != VarScope.end()){
        llvm::errs() << "Variable " << *I << " is already declared as an variable" << "\n";
        HasError = true; 
      }
      else{
        if (!IntScope.insert(*I).second)
          error(Twice, *I); // If the insertion fails (element already exists in Scope), report a "Twice" error
      }
    }
  };

  virtual void visit(DeclarationBool &Node) override {
    for (llvm::SmallVector<Logic *>::const_iterator I = Node.valBegin(), E = Node.valEnd(); I != E; ++I){
      (*I)->accept(*this); // If the Declaration node has an expression, recursively visit the expression node
    }
    for (llvm::SmallVector<llvm::StringRef>::const_iterator I = Node.varBegin(), E = Node.varEnd(); I != E;
         ++I) {
      if(FloatScope.find(*I) != FloatScope.end()){
        llvm::errs() << "Variable " << *I << " is already declared as an float" << "\n";
        HasError = true; 
      }
      else if(IntScope.find(*I) != IntScope.end()){
        llvm::errs() << "Variable " << *I << " is already declared as an integer" << "\n";
        HasError = true; 
      }
      if(VarScope.find(*I) != VarScope.end()){
        llvm::errs() << "Variable " << *I << " is already declared as an variable" << "\n";
        HasError = true; 
      }
      else{
        if (!BoolScope.insert(*I).second)
          error(Twice, *I); // If the insertion fails (element already exists in Scope), report a "Twice" error
      }
    }
    
  };

  virtual void visit(Comparison &Node) override {
    if(Node.getLeft()){
      Node.getLeft()->accept(*this);
    }
    if(Node.getRight()){
      Node.getRight()->accept(*this);
    }
    // else{
    //   if (Node.getOperator() == Comparison::Ident){
    //     Final* F = (Final*)(Node.getLeft());
    //     if (BoolScope.find(F->getVal()) == BoolScope.end()) {
    //       llvm::errs() << "you need a boolean varaible to compare or assign: "<< F->getVal() << "\n";
    //       HasError = true;
    //     } 
    //   }
    // }

    if (Node.getOperator() != Comparison::True && Node.getOperator() != Comparison::False && Node.getOperator() != Comparison::Ident){
      Final* L = (Final*)(Node.getLeft());
      if(L){
        if (L->getKind() == Final::ValueKind::Ident && IntScope.find(L->getVal()) == IntScope.end() && FloatScope.find(L->getVal()) == FloatScope.end()) {
          llvm::errs() << "you can only compare a defined integer variable: "<< L->getVal() << "\n";
          HasError = true;
        } 
      }
      
      Final* R = (Final*)(Node.getRight());
      if(R){
        if (R->getKind() == Final::ValueKind::Ident && IntScope.find(R->getVal()) == IntScope.end() && FloatScope.find(R->getVal()) == FloatScope.end()) {
          llvm::errs() << "you can only compare a defined integer variable: "<< R->getVal() << "\n";
          HasError = true;
        } 
      }
    }
  };

  virtual void visit(LogicalExpr &Node) override {
    if(Node.getLeft()){
      Node.getLeft()->accept(*this);
    }
    if(Node.getRight()){
      Node.getRight()->accept(*this);
    }
  };

  virtual void visit(UnaryOp &Node) override {
    if (IntScope.find(Node.getIdent()) == IntScope.end()){
      llvm::errs() << "Variable "<<Node.getIdent() << " is not a defined integer variable." << "\n";
      HasError = true;
    }
  };

  virtual void visit(NegExpr &Node) override {
    Expr *expr = Node.getExpr();
    (*expr).accept(*this);
  };

  virtual void visit(PrintStmt &Node) override {
    // Check if identifier is in the scope
    if (IntScope.find(Node.getVar()) == IntScope.end() && BoolScope.find(Node.getVar()) == BoolScope.end())
      error(Not, Node.getVar());
    
  };

  virtual void visit(IfStmt &Node) override {
    Logic *l = Node.getCond();
    (*l).accept(*this);

    for (llvm::SmallVector<AST *>::const_iterator I = Node.begin(), E = Node.end(); I != E; ++I) {
      (*I)->accept(*this);
    }
    for (llvm::SmallVector<AST *>::const_iterator I = Node.beginElse(), E = Node.endElse(); I != E; ++I){
      (*I)->accept(*this);
    }
    for (llvm::SmallVector<elifStmt *>::const_iterator I = Node.beginElif(), E = Node.endElif(); I != E; ++I){
      (*I)->accept(*this);
    }
  };

  virtual void visit(elifStmt &Node) override {
    Logic* l = Node.getCond();
    (*l).accept(*this);

    for (llvm::SmallVector<AST *>::const_iterator I = Node.begin(), E = Node.end(); I != E; ++I) {
      (*I)->accept(*this);
    }
  };

  virtual void visit(WhileStmt &Node) override {
    LoopDepth++;

    Logic* l = Node.getCond();
    (*l).accept(*this);

    for (llvm::SmallVector<AST *>::const_iterator I = Node.begin(), E = Node.end(); I != E; ++I) {
      (*I)->accept(*this);
    }
    LoopDepth--;
  };

  virtual void visit(ForStmt &Node) override {
    LoopDepth++;

    Assignment *first = Node.getFirst();
    (*first).accept(*this);

    Logic *second = Node.getSecond();
    (*second).accept(*this);

    Assignment *assign = Node.getThirdAssign();
    if(assign)
      (*assign).accept(*this);
    else{
      UnaryOp *unary = Node.getThirdUnary();
      (*unary).accept(*this);
    }
      

    for (llvm::SmallVector<AST *>::const_iterator I = Node.begin(), E = Node.end(); I != E; ++I) {
      (*I)->accept(*this);
    }

    LoopDepth--;
  };

  virtual void visit(SignedNumber &Node) override {
  };

//   virtual void visit(DeclarationVar &Node) override {// TODO: add implementation
//     for (llvm::SmallVector<AST *>::const_iterator I = Node.valBegin(), E = Node.valEnd(); I != E; ++I) {
//         (*I)->accept(*this); // Check initializer expressions
//     }

//     for (llvm::SmallVector<llvm::StringRef>::const_iterator I = Node.varBegin(), E = Node.varEnd(); I != E; ++I) {
//         if (IntScope.find(*I) != IntScope.end() || BoolScope.find(*I) != BoolScope.end() || FloatScope.find(*I) != FloatScope.end()) {
//             llvm::errs() << "Variable " << *I << " is already declared as a specific type." << "\n";
//             HasError = true;
//         } else {
//             VarScope.insert(*I); // Add variable to dynamically typed scope
//         }
//     }
// }

  virtual void visit(DeclarationVar &Node) override {
    // Iterate over the values (initializers)
    for (DeclarationVar::ValueVector::const_iterator I = Node.valBegin(), 
                                                     E = Node.valEnd(); 
         I != E; 
         ++I) {
        if (*I) { // Check if the value is not null
            (*I)->accept(*this); // Visit initializer expressions
        }
    }

    // Iterate over the variables
    for (llvm::SmallVector<llvm::StringRef>::const_iterator I = Node.varBegin(), 
                                                            E = Node.varEnd(); 
         I != E; 
         ++I) {
        if (IntScope.find(*I) != IntScope.end() || 
            BoolScope.find(*I) != BoolScope.end() || 
            FloatScope.find(*I) != FloatScope.end()) {
            llvm::errs() << "Variable " << *I << " is already declared as a specific type.\n";
            HasError = true;
        } else {
            VarScope.insert(*I); // Add variable to dynamically typed scope
        }
    }
  }

  virtual void visit(DeclarationConst &Node) override {// TODO: add implementation
    Node.getInitializer()->accept(*this); // Check initializer expression

    if (ConstScope.find(Node.getConstName()) != ConstScope.end()) {
        llvm::errs() << "Constant " << Node.getConstName() << " is already declared." << "\n";
        HasError = true;
    } else {
        ConstScope.insert(Node.getConstName()); // Add constant to scope
    }
  }

  virtual void visit(DefineStmt &Node) override {// TODO: add implementation
    if (DefineScope.find(Node.getMacroName()) != DefineScope.end()) {
        llvm::errs() << "Macro " << Node.getMacroName() << " is already defined." << "\n";
        HasError = true;
    } else {
        DefineScope.insert(Node.getMacroName()); // Add macro to scope
    }
  }

  virtual void visit(SwitchStmt &Node) override {// TODO: add implementation
    SwitchDepth++;
    Node.getCondition()->accept(*this); // Check switch condition

    llvm::StringSet<> CaseValues; // To track unique case values
    for (CaseStmt *Case : Node.getCases()) {
        llvm::StringRef CaseVal;
        Final *CaseValueFinal = dynamic_cast<Final *>(Case->getCaseValue()); // TODO here is new changes. old changes are commented.
        if (CaseValueFinal && (CaseValueFinal->getKind() == Final::Ident || CaseValueFinal->getKind() == Final::Number || CaseValueFinal->getKind() == Final::Bool || CaseValueFinal->getKind() == Final:: FloatNumber)) {
            llvm::StringRef CaseVal = CaseValueFinal->getVal();
        } else {
        llvm::errs() << "Error: Unsupported case value\n";
        }

        if (CaseValues.find(CaseVal) != CaseValues.end()) {
            llvm::errs() << "Duplicate case value: " << CaseVal << "\n";
            HasError = true;
          } else {
              CaseValues.insert(CaseVal); // Add unique value to the set
          }
        Case->accept(*this); // Check case body
    }

    DefaultStmt *Default = Node.getDefault(); //TODO
    if (Default && !Default->getBody().empty()) { // Check if Default exists and its body is not empty
    for (AST *Stmt : Default->getBody()) {
        Stmt->accept(*this); // Visit each statement in the default body
    }

    SwitchDepth--; 
}


  }

  virtual void visit(CaseStmt &Node) override {// TODO: add implementation
    for (llvm::SmallVector<AST *>::const_iterator I = Node.getBody().begin(), E = Node.getBody().end(); I != E; ++I) {
        (*I)->accept(*this); // Check each statement in the case body
    }
  }

  virtual void visit(DoWhileStmt &Node) override {// TODO: add implementation
    LoopDepth++;
    for (llvm::SmallVector<AST *>::const_iterator I = Node.getBody().begin(), E = Node.getBody().end(); I != E; ++I) {
        (*I)->accept(*this); // Check each statement in the body
    }

    Logic *Condition = Node.getCond();
    if (Condition) {
        Condition->accept(*this); // Validate the condition
    } else {
        llvm::errs() << "Do-While loop must have a valid condition.\n";
        HasError = true;
    }
    LoopDepth--;
  }

  // virtual void visit(FunctionCall &Node) override {// TODO: add implementation
  //   if (FunctionScope.find(Node.getFuncName()) == FunctionScope.end()) {
  //       llvm::errs() << "Unknown function: " << Node.getFuncName() << "\n";
  //       HasError = true;
  //   }
  //   for (llvm::SmallVector<Expr *>::const_iterator I = Node.getArgs().begin(), E = Node.getArgs().end(); I != E; ++I) {
  //       (*I)->accept(*this); // Check each argument expression
  //   }
  // }

  // TODO: Implement missing virtual methods to make InputCheck concrete
  virtual void visit(TernaryAssignment &Node) override {
    // Validate the condition part of the ternary expression
    Logic *Condition = Node.getCondition();
    if (Condition) {
        Condition->accept(*this); // Check the logic expression
    } else {
        llvm::errs() << "Ternary operation missing condition.\n";
        HasError = true;
        return;
    }

    // Validate the true expression
    Expr *TrueExpr = Node.getTrueExpr();
    if (TrueExpr) {
        TrueExpr->accept(*this); // Check the true expression
    } else {
        llvm::errs() << "Ternary operation missing 'true' branch expression.\n";
        HasError = true;
        return;
    }

    // Validate the false expression
    Expr *FalseExpr = Node.getFalseExpr();
    if (FalseExpr) {
        FalseExpr->accept(*this); // Check the false expression
    } else {
        llvm::errs() << "Ternary operation missing 'false' branch expression.\n";
        HasError = true;
        return;
    }

    // Type-check to ensure both branches are compatible
    if (TrueExpr && FalseExpr) {
        if (!isCompatibleType(TrueExpr, FalseExpr)) {
            llvm::errs() << "Mismatched types in ternary operation: true branch is "
                         << getType(TrueExpr) << ", false branch is " << getType(FalseExpr) << ".\n";
            HasError = true;
        }
    }
}

  virtual void visit(DefaultStmt &Node) override { // TODO
    // Validate that the default statement is inside a switch-case
    if (SwitchDepth <= 0) {
        llvm::errs() << "Error: 'default' statement not inside a switch-case.\n";
        HasError = true;
    }

    // Visit all statements in the default body
    for (AST *Stmt : Node.getBody()) {
        if (Stmt) {
            Stmt->accept(*this); // Recursively validate each statement
        } else {
            llvm::errs() << "Error: Null statement in 'default' body.\n";
            HasError = true;
        }
    }
}

  virtual void visit(Cast &Node) override { // TODO
    // Visit the expression being casted
    Expr *ExprToCast = Node.getOperand();
    if (ExprToCast) {
        ExprToCast->accept(*this);
    } else {
        llvm::errs() << "Error: Missing expression in cast operation.\n";
        HasError = true;
        return;
    }

    // Convert TargetType to std::string
    std::string TargetType = std::string(Node.getTargetType());
    if (TargetType.empty()) {
        llvm::errs() << "Error: Target type missing in cast operation.\n";
        HasError = true;
        return;
    }

    // Type compatibility check
    std::string ExprType = getType(ExprToCast);
    if (ExprType != TargetType) {
        llvm::errs() << "Warning: Implicit cast from " << ExprType
                     << " to " << TargetType << ". Ensure this is intended.\n";
    }
}
 
  virtual void visit(BreakStmt &Node) override { // TODO
    if (LoopDepth <= 0 && SwitchDepth <= 0) {
        llvm::errs() << "Error: 'break' statement not inside a loop or switch-case.\n";
        llvm::errs() << "At " << Node.getLocation() << "\n";
        // Use a new function for Location-based error reporting
        locationError(Node.getLocation(), "'break' used outside of a loop or switch-case");
    }
  } 
  
  virtual void visit(ContinueStmt &Node) override { // TODO
    if (LoopDepth <= 0) {
        llvm::errs() << "Error: 'continue' statement not inside a loop.\n";
        llvm::errs() << "At " << Node.getLocation() << "\n";
        locationError(Node.getLocation(), "'continue' used outside of a loop");
    }
  }

  virtual void visit(FunctionCall &Node) override { // TODO
    // Validate function existence
    if (FunctionScope.find(Node.getFuncName()) == FunctionScope.end()) {
        llvm::errs() << "Unknown function: " << Node.getFuncName() << "\n";
        HasError = true;
        return;
    }

    // Validate arguments
    const auto &Args = Node.getArgs();
    if (Args.empty()) {
        llvm::errs() << "Function " << Node.getFuncName() << " requires arguments.\n";
        HasError = true;
        return;
    }

    if (Node.getFuncName() == "min" || Node.getFuncName() == "max") {
        if (Args.size() < 2) {
            llvm::errs() << "Function " << Node.getFuncName() << " requires at least two arguments.\n";
            HasError = true;
        }
    } else if (Node.getFuncName() == "mean") {
        if (Args.empty()) {
            llvm::errs() << "Function " << Node.getFuncName() << " requires at least one argument.\n";
            HasError = true;
        }
    } else if (Node.getFuncName() == "sqrtN") {
        if (Args.size() != 1) {
            llvm::errs() << "Function " << Node.getFuncName() << " requires exactly one argument.\n";
            HasError = true;
        }
    }

    // Validate each argument
    for (Expr *Arg : Args) {
        if (Arg) {
            Arg->accept(*this);
            std::string ArgType = getType(Arg);
            if (ArgType != "int" && ArgType != "float") {
                llvm::errs() << "Invalid argument type for function " << Node.getFuncName()
                             << ": expected numeric type but got " << ArgType << ".\n";
                HasError = true;
            }
        } else {
            llvm::errs() << "Null argument in function " << Node.getFuncName() << ".\n";
            HasError = true;
        }
    }
}


  bool isCompatibleType(Expr *Expr1, Expr *Expr2) { // TODO 
    // Implement type compatibility logic based on your type system
    // Example: Assume getType returns a string representation of the type
    return getType(Expr1) == getType(Expr2);
}
  
  
std::string getType(Expr *Expression) {
    if (Final *F = dynamic_cast<Final *>(Expression)) {
        if (IntScope.find(F->getVal()) != IntScope.end()) return "int";
        if (BoolScope.find(F->getVal()) != BoolScope.end()) return "bool";
        if (FloatScope.find(F->getVal()) != FloatScope.end()) return "float";
    }
    return "unknown"; // Default fallback
}


};
}

bool Sema::semantic(Program *Tree) {
  if (!Tree)
    return false; // If the input AST is not valid, return false indicating no errors
  nms::InputCheck *Check = new nms::InputCheck();;// Create an instance of the InputCheck class for semantic analysis
  Tree->accept(*Check); // Initiate the semantic analysis by traversing the AST using the accept function

  return Check->hasError(); // Return the result of Check.hasError() indicating if any errors were detected during the analysis
}

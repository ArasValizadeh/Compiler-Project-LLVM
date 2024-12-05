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
      if (IntScope.find(Node.getVal()) == IntScope.end() && BoolScope.find(Node.getVal()) == BoolScope.end()){
        llvm::errs() << "node value is: " << Node.getVal() << " and it is of kind: " << Node.getKind() <<"\n";

        // for (const auto &entry : IntScope) {
        //   llvm::errs() << "IntScope contains: " << entry << "it's value is: " << entry.getVal() << "and it is of kind: " << entry.getKind() << "\n";
        // }
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
      if(IntScope.find(*I) != IntScope.end()){
        llvm::errs() << "Variable " << *I << " is already declared as an integer" << "\n";
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
        if (L->getKind() == Final::ValueKind::Ident && IntScope.find(L->getVal()) == IntScope.end()) {
          llvm::errs() << "you can only compare a defined integer variable: "<< L->getVal() << "\n";
          HasError = true;
        } 
      }
      
      Final* R = (Final*)(Node.getRight());
      if(R){
        if (R->getKind() == Final::ValueKind::Ident && IntScope.find(R->getVal()) == IntScope.end()) {
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
    Logic* l = Node.getCond();
    (*l).accept(*this);

    for (llvm::SmallVector<AST *>::const_iterator I = Node.begin(), E = Node.end(); I != E; ++I) {
      (*I)->accept(*this);
    }
  };

  virtual void visit(ForStmt &Node) override {
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
  };

  virtual void visit(SignedNumber &Node) override {
  };

  virtual void visit(DeclarationFloat &Node) override {// TODO: add implementation
    for (llvm::SmallVector<Expr *>::const_iterator I = Node.valBegin(), E = Node.valEnd(); I != E; ++I) {
        (*I)->accept(*this); // Check initializer expressions
    }

    for (llvm::SmallVector<llvm::StringRef>::const_iterator I = Node.varBegin(), E = Node.varEnd(); I != E; ++I) {
        if (IntScope.find(*I) != IntScope.end() || BoolScope.find(*I) != BoolScope.end()) {
            llvm::errs() << "Variable " << *I << " is already declared as an integer or boolean." << "\n";
            HasError = true;
        } else {
            if (!FloatScope.insert(*I).second)
                error(Twice, *I); // Report "Twice" error if already declared
        }
    }
  }

  virtual void visit(DeclarationVar &Node) override {// TODO: add implementation
    for (llvm::SmallVector<AST *>::const_iterator I = Node.valBegin(), E = Node.valEnd(); I != E; ++I) {
        (*I)->accept(*this); // Check initializer expressions
    }

    for (llvm::SmallVector<llvm::StringRef>::const_iterator I = Node.varBegin(), E = Node.varEnd(); I != E; ++I) {
        if (IntScope.find(*I) != IntScope.end() || BoolScope.find(*I) != BoolScope.end() || FloatScope.find(*I) != FloatScope.end()) {
            llvm::errs() << "Variable " << *I << " is already declared as a specific type." << "\n";
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

    // if (Node.hasDefault()) {
    //     Node.getDefault()->accept(*this); // Check default body
    // }
    // if (!Node.getDefaultBody().empty()) { // TODO Check if default body exists. old changes are commented.
    //   for (AST *Stmt : Node.getDefaultBody()) {
    //       Stmt->accept(*this); // Visit each statement in the default body
    //   } 
    // }
    DefaultStmt *Default = Node.getDefault(); //TODO
    if (Default && !Default->getBody().empty()) { // Check if Default exists and its body is not empty
    for (AST *Stmt : Default->getBody()) {
        Stmt->accept(*this); // Visit each statement in the default body
    } 
}


  }

  virtual void visit(CaseStmt &Node) override {// TODO: add implementation
    for (llvm::SmallVector<AST *>::const_iterator I = Node.getBody().begin(), E = Node.getBody().end(); I != E; ++I) {
        (*I)->accept(*this); // Check each statement in the case body
    }
  }

  virtual void visit(DoWhileStmt &Node) override {// TODO: add implementation
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
  }

  virtual void visit(FunctionCall &Node) override {// TODO: add implementation
    if (FunctionScope.find(Node.getFuncName()) == FunctionScope.end()) {
        llvm::errs() << "Unknown function: " << Node.getFuncName() << "\n";
        HasError = true;
    }

    for (llvm::SmallVector<Expr *>::const_iterator I = Node.getArgs().begin(), E = Node.getArgs().end(); I != E; ++I) {
        (*I)->accept(*this); // Check each argument expression
    }
  }

  // TODO: Implement missing virtual methods to make InputCheck concrete
  virtual void visit(TernaryAssignment &) override {} // TODO
  virtual void visit(DefaultStmt &) override {} // TODO
  virtual void visit(Cast &) override {} // TODO
  virtual void visit(BreakStmt &) override {} // TODO
  virtual void visit(ContinueStmt &) override {} // TODO


};
}

bool Sema::semantic(Program *Tree) {
  if (!Tree)
    return false; // If the input AST is not valid, return false indicating no errors
  nms::InputCheck *Check = new nms::InputCheck();;// Create an instance of the InputCheck class for semantic analysis
  Tree->accept(*Check); // Initiate the semantic analysis by traversing the AST using the accept function

  return Check->hasError(); // Return the result of Check.hasError() indicating if any errors were detected during the analysis
}

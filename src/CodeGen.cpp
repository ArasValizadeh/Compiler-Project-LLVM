#include "CodeGen.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Constants.h"

using namespace llvm;

// Define a visitor class for generating LLVM IR from the AST.
namespace ns
{
  class ToIRVisitor : public ASTVisitor
  {
    Module *M;
    IRBuilder<> Builder;
    Type *VoidTy;
    Type *Int1Ty;
    Type *Int32Ty;
    Type *Int8PtrTy;
    Type *Int8PtrPtrTy;
    Constant *Int32Zero;
    Constant *Int32One;
    Constant *Int1False;
    Constant *Int1True;

    Value *V;
    StringMap<AllocaInst *> nameMapInt;
    StringMap<AllocaInst *> nameMapBool;
    StringMap<AllocaInst *> nameMapFloat;// TODO: add float scope
    StringMap<AllocaInst *> nameMapVar;// TODO: add var scope

    FunctionType *PrintIntFnTy;
    Function *PrintIntFn;

    FunctionType *PrintBoolFnTy;
    Function *PrintBoolFn;

  public:
    // Constructor for the visitor class.
    ToIRVisitor(Module *M) : M(M), Builder(M->getContext())
    {
      // Initialize LLVM types and constants.
      VoidTy = Type::getVoidTy(M->getContext());
      Int1Ty = Type::getInt1Ty(M->getContext());
      Int32Ty = Type::getInt32Ty(M->getContext());
      Int8PtrTy = Type::getInt8PtrTy(M->getContext());
      Int8PtrPtrTy = Int8PtrTy->getPointerTo();

      Int1False = ConstantInt::getFalse(Int1Ty);
      Int1True = ConstantInt::getTrue(Int1Ty);
      Int32Zero = ConstantInt::get(Int32Ty, 0, true);
      Int32One = ConstantInt::get(Int32Ty, 1, true);

      PrintIntFnTy = FunctionType::get(VoidTy, {Int32Ty}, false);
      // Create a function declaration for the "compiler_write" function.
      PrintIntFn = Function::Create(PrintIntFnTy, GlobalValue::ExternalLinkage, "print_int", M);

      PrintBoolFnTy = FunctionType::get(VoidTy, {Int1Ty}, false);
      // Create a function declaration for the "compiler_write" function.
      PrintBoolFn = Function::Create(PrintBoolFnTy, GlobalValue::ExternalLinkage, "print_bool", M);
    }

    // Entry point for generating LLVM IR from the AST.
    void run(Program *Tree)
    {
      // Create the main function with the appropriate function type.
      FunctionType *MainFty = FunctionType::get(Int32Ty, {Int32Ty, Int8PtrPtrTy}, false);
      Function *MainFn = Function::Create(MainFty, GlobalValue::ExternalLinkage, "main", M);

      // Create a basic block for the entry point of the main function.
      BasicBlock *BB = BasicBlock::Create(M->getContext(), "entry", MainFn);
      Builder.SetInsertPoint(BB);

      // Visit the root node of the AST to generate IR.
      Tree->accept(*this);

      // Create a return instruction at the end of the main function.
      Builder.CreateRet(Int32Zero);
    }

    // Visit function for the Program node in the AST.
    virtual void visit(Program &Node) override
    {
      // Iterate over the children of the Program node and visit each child.
      for (llvm::SmallVector<AST *>::const_iterator I = Node.begin(), E = Node.end(); I != E; ++I)
      {
        (*I)->accept(*this); // Visit each child node
      }
    };

    virtual void visit(DeclarationInt &Node) override
    {
      llvm::SmallVector<Value *, 8> vals;

      llvm::SmallVector<Expr *, 8>::const_iterator E = Node.valBegin();
      for (llvm::SmallVector<llvm::StringRef, 8>::const_iterator Var = Node.varBegin(), End = Node.varEnd(); Var != End; ++Var)
      {
        if (E < Node.valEnd() && *E != nullptr)
        {
          (*E)->accept(*this); // If the Declaration node has an expression, recursively visit the expression node
          vals.push_back(V);
        }
        else
        {
          vals.push_back(nullptr);
        }
        E++;
      }
      StringRef Var;
      Value *val;
      llvm::SmallVector<Value *, 8>::const_iterator itVal = vals.begin();
      for (llvm::SmallVector<llvm::StringRef, 8>::const_iterator S = Node.varBegin(), End = Node.varEnd(); S != End; ++S)
      {

        Var = *S;

        // Create an alloca instruction to allocate memory for the variable.
        nameMapInt[Var] = Builder.CreateAlloca(Int32Ty);

        // Store the initial value (if any) in the variable's memory location.
        if (*itVal != nullptr)
        {
          Builder.CreateStore(*itVal, nameMapInt[Var]);
        }
        else
        {
          Builder.CreateStore(Int32Zero, nameMapInt[Var]);
        }
        itVal++;
      }
    };

    virtual void visit(DeclarationBool &Node) override
    {
      llvm::SmallVector<Value *, 8> vals;

      llvm::SmallVector<Logic *, 8>::const_iterator L = Node.valBegin();
      for (llvm::SmallVector<llvm::StringRef, 8>::const_iterator Var = Node.varBegin(), End = Node.varEnd(); Var != End; ++Var)
      {
        if (L < Node.valEnd() && *L != nullptr)
        {
          (*L)->accept(*this); // If the Declaration node has an expression, recursively visit the expression node
          vals.push_back(V);
        }
        else
        {
          vals.push_back(nullptr);
        }
        L++;
      }
      StringRef Var;
      Value *val;
      llvm::SmallVector<Value *, 8>::const_iterator itVal = vals.begin();
      for (llvm::SmallVector<llvm::StringRef, 8>::const_iterator S = Node.varBegin(), End = Node.varEnd(); S != End; ++S)
      {

        Var = *S;

        // Create an alloca instruction to allocate memory for the variable.
        nameMapBool[Var] = Builder.CreateAlloca(Int1Ty);

        // Store the initial value (if any) in the variable's memory location.
        if (*itVal != nullptr)
        {
          Builder.CreateStore(*itVal, nameMapBool[Var]);
        }
        else
        {
          Builder.CreateStore(Int1False, nameMapBool[Var]);
        }
        itVal++;
      }
    };
    
    virtual void visit(Assignment &Node) override
    {
      // Get the name of the variable being assigned.
      llvm::StringRef varName = Node.getLeft()->getVal();
      Node.getLeft()->accept(*this);
      Value *varVal = V;

      if (Node.getRightExpr() == nullptr)
        Node.getRightLogic()->accept(*this);
      else
        Node.getRightExpr()->accept(*this);

      Value *val = V;

      switch (Node.getAssignKind())
      {
      case Assignment::Plus_assign:
        val = Builder.CreateNSWAdd(varVal, val);
        break;
      case Assignment::Minus_assign:
        val = Builder.CreateNSWSub(varVal, val);
        break;
      case Assignment::Star_assign:
        val = Builder.CreateNSWMul(varVal, val);
        break;
      case Assignment::Slash_assign:
        val = Builder.CreateSDiv(varVal, val);
        break;
      default:
        break;
      }

      // Create a store instruction to assign the value to the variable.
      if (isBool(((Final *)Node.getLeft())->getVal()))
        Builder.CreateStore(val, nameMapBool[varName]);
      else
        Builder.CreateStore(val, nameMapInt[varName]);
    };

    virtual void visit(Final &Node) override
    {
      if (Node.getKind() == Final::Ident)
      {
        // If the Final is an identifier, load its value from memory.
        if (isBool(Node.getVal()))
          V = Builder.CreateLoad(Int1Ty, nameMapBool[Node.getVal()]);
        else
          V = Builder.CreateLoad(Int32Ty, nameMapInt[Node.getVal()]);
      }
      else
      {
        // If the Final is a literal, convert it to an integer and create a constant.
        int intval;
        Node.getVal().getAsInteger(10, intval);
        V = ConstantInt::get(Int32Ty, intval, true);
      }
    };

    virtual void visit(BinaryOp &Node) override
    {
      // Visit the left-hand side of the binary operation and get its value.
      Node.getLeft()->accept(*this);
      Value *Left = V;

      // Visit the right-hand side of the binary operation and get its value.
      Node.getRight()->accept(*this);
      Value *Right = V;

      // Perform the binary operation based on the operator type and create the corresponding instruction.
      switch (Node.getOperator())
      {
      case BinaryOp::Plus:
        V = Builder.CreateNSWAdd(Left, Right);
        break;
      case BinaryOp::Minus:
        V = Builder.CreateNSWSub(Left, Right);
        break;
      case BinaryOp::Mul:
        V = Builder.CreateNSWMul(Left, Right);
        break;
      case BinaryOp::Div:
        V = Builder.CreateSDiv(Left, Right);
        break;
      case BinaryOp::Mod:
        V = Builder.CreateSRem(Left, Right);
        break;
      case BinaryOp::Exp:
        V = CreateExp(Left, Right);
        break;
      default:
        break;
      }
    };

    Value *CreateExp(Value *Left, Value *Right)
    {
      AllocaInst *counterAlloca = Builder.CreateAlloca(Int32Ty);
      AllocaInst *resultAlloca = Builder.CreateAlloca(Int32Ty);
      Builder.CreateStore(Int32Zero, counterAlloca);
      Builder.CreateStore(Int32One, resultAlloca);

      llvm::BasicBlock *ForCondBB = llvm::BasicBlock::Create(M->getContext(), "exp.cond", Builder.GetInsertBlock()->getParent());
      llvm::BasicBlock *ForBodyBB = llvm::BasicBlock::Create(M->getContext(), "exp.body", Builder.GetInsertBlock()->getParent());
      llvm::BasicBlock *AfterForBB = llvm::BasicBlock::Create(M->getContext(), "after.exp", Builder.GetInsertBlock()->getParent());

      Builder.CreateBr(ForCondBB); //?

      Builder.SetInsertPoint(ForCondBB);
      Value *counterLoad = Builder.CreateLoad(counterAlloca);

      Value *cond = Builder.CreateICmpSLT(counterLoad, Right);
      Builder.CreateCondBr(cond, ForBodyBB, AfterForBB);

      Builder.SetInsertPoint(ForBodyBB);
      Value *resultLoad = Builder.CreateLoad(resultAlloca);

      Value *resultMul = Builder.CreateMul(resultLoad, Left);
      Value *counterInc = Builder.CreateAdd(counterLoad, Int32One);
      Builder.CreateStore(resultMul, resultAlloca);
      Builder.CreateStore(counterInc, counterAlloca);

      Builder.CreateBr(ForCondBB);
      Builder.SetInsertPoint(AfterForBB);

      Value *result = Builder.CreateLoad(resultAlloca);
      return result;
    }

    virtual void visit(UnaryOp &Node) override
    {
      // Visit the left-hand side of the binary operation and get its value.
      Value *Left = Builder.CreateLoad(Int32Ty, nameMapInt[Node.getIdent()]);
      ;

      // Perform the binary operation based on the operator type and create the corresponding instruction.
      switch (Node.getOperator())
      {
      case UnaryOp::Plus_plus:
        V = Builder.CreateNSWAdd(Left, Int32One);
        break;
      case UnaryOp::Minus_minus:
        V = Builder.CreateNSWSub(Left, Int32One);
      default:
        break;
      }

      Builder.CreateStore(V, nameMapInt[Node.getIdent()]);
    };

    virtual void visit(SignedNumber &Node) override
    {
      int intval;
      Node.getValue().getAsInteger(10, intval);
      V = ConstantInt::get(Int32Ty, (Node.getSign() == SignedNumber::Minus) ? -intval : intval, true);
    };

    virtual void visit(NegExpr &Node) override
    {
      Node.getExpr()->accept(*this);
      V = Builder.CreateNeg(V);
    };

    virtual void visit(LogicalExpr &Node) override
    {
      // Visit the left-hand side of the Logical operation and get its value.
      Node.getLeft()->accept(*this);
      Value *Left = V;

      if (Node.getRight() == nullptr)
      {
        V = Left;
        return;
      }
      // Visit the right-hand side of the Logical operation and get its value.
      Node.getRight()->accept(*this);
      Value *Right = V;

      switch (Node.getOperator())
      {
      case LogicalExpr::And:
        V = Builder.CreateAnd(Left, Right);
        break;
      case LogicalExpr::Or:
        V = Builder.CreateOr(Left, Right);
        break;
      default:
        break;
      }
    };

    virtual void visit(Comparison &Node) override
    {
      // Visit the left-hand side of the Comparison operation and get its value.
      if (Node.getRight() == nullptr)
      {
        switch (Node.getOperator())
        {
        case Comparison::True:
          V = Int1True;
          break;
        case Comparison::False:
          V = Int1False;
          break;
        case Comparison::Ident:
          if (isBool(((Final *)Node.getLeft())->getVal()))
          {
            V = Builder.CreateLoad(Int1Ty, nameMapBool[((Final *)Node.getLeft())->getVal()]);
            break;
          }

          V = Builder.CreateLoad(Int32Ty, nameMapInt[((Final *)Node.getLeft())->getVal()]);
          break;

        default:
          break;
        }
        return;
      }
      Node.getLeft()->accept(*this);
      Value *Left = V;

      // Visit the right-hand side of the Comparison operation and get its value.
      Node.getRight()->accept(*this);
      Value *Right = V;

      switch (Node.getOperator())
      {
      case Comparison::Equal:
        V = Builder.CreateICmpEQ(Left, Right);
        break;
      case Comparison::Not_equal:
        V = Builder.CreateICmpNE(Left, Right);
        break;
      case Comparison::Less:
        V = Builder.CreateICmpSLT(Left, Right);
        break;
      case Comparison::Greater:
        V = Builder.CreateICmpSGT(Left, Right);
        break;
      case Comparison::Less_equal:
        V = Builder.CreateICmpSLE(Left, Right);
        break;
      case Comparison::Greater_equal:
        V = Builder.CreateICmpSGE(Left, Right);
        break;
      default:
        break;
      }
    };

    bool isBool(llvm::StringRef Var)
    {
      return nameMapBool.find(Var) != nameMapBool.end();
    }

    virtual void visit(PrintStmt &Node) override
    {
      // Visit the right-hand side of the assignment and get its value.
      if (isBool(Node.getVar()))
      {
        V = Builder.CreateLoad(Int1Ty, nameMapBool[Node.getVar()]);
        CallInst *Call = Builder.CreateCall(PrintBoolFnTy, PrintBoolFn, {V});
      }
      else
      {
        V = Builder.CreateLoad(Int32Ty, nameMapInt[Node.getVar()]);
        CallInst *Call = Builder.CreateCall(PrintIntFnTy, PrintIntFn, {V});
      }
    };

    virtual void visit(WhileStmt &Node) override
    {
      llvm::BasicBlock *WhileCondBB = llvm::BasicBlock::Create(M->getContext(), "while.cond", Builder.GetInsertBlock()->getParent());
      llvm::BasicBlock *WhileBodyBB = llvm::BasicBlock::Create(M->getContext(), "while.body", Builder.GetInsertBlock()->getParent());
      llvm::BasicBlock *AfterWhileBB = llvm::BasicBlock::Create(M->getContext(), "after.while", Builder.GetInsertBlock()->getParent());

      Builder.CreateBr(WhileCondBB); //?
      Builder.SetInsertPoint(WhileCondBB);
      Node.getCond()->accept(*this);
      Value *val = V;
      Builder.CreateCondBr(val, WhileBodyBB, AfterWhileBB);
      Builder.SetInsertPoint(WhileBodyBB);

      for (llvm::SmallVector<AST *>::const_iterator I = Node.begin(), E = Node.end(); I != E; ++I)
      {
        (*I)->accept(*this);
      }

      Builder.CreateBr(WhileCondBB);

      Builder.SetInsertPoint(AfterWhileBB);
    };

    virtual void visit(ForStmt &Node) override
    {
      llvm::BasicBlock *ForCondBB = llvm::BasicBlock::Create(M->getContext(), "for.cond", Builder.GetInsertBlock()->getParent());
      llvm::BasicBlock *ForBodyBB = llvm::BasicBlock::Create(M->getContext(), "for.body", Builder.GetInsertBlock()->getParent());
      llvm::BasicBlock *AfterForBB = llvm::BasicBlock::Create(M->getContext(), "after.for", Builder.GetInsertBlock()->getParent());

      Node.getFirst()->accept(*this);

      Builder.CreateBr(ForCondBB); //?

      Builder.SetInsertPoint(ForCondBB);
      Node.getSecond()->accept(*this);
      Value *val = V;
      Builder.CreateCondBr(val, ForBodyBB, AfterForBB);

      Builder.SetInsertPoint(ForBodyBB);
      for (llvm::SmallVector<AST *>::const_iterator I = Node.begin(), E = Node.end(); I != E; ++I)
      {
        (*I)->accept(*this);
      }

      if (Node.getThirdAssign() == nullptr)
        Node.getThirdUnary()->accept(*this);
      else
        Node.getThirdAssign()->accept(*this);

      Builder.CreateBr(ForCondBB);

      Builder.SetInsertPoint(AfterForBB);
    };

    virtual void visit(IfStmt &Node) override
    {
      llvm::BasicBlock *IfCondBB = llvm::BasicBlock::Create(M->getContext(), "if.cond", Builder.GetInsertBlock()->getParent());
      llvm::BasicBlock *IfBodyBB = llvm::BasicBlock::Create(M->getContext(), "if.body", Builder.GetInsertBlock()->getParent());
      llvm::BasicBlock *AfterIfBB = llvm::BasicBlock::Create(M->getContext(), "after.if", Builder.GetInsertBlock()->getParent());

      Builder.CreateBr(IfCondBB); //?
      Builder.SetInsertPoint(IfCondBB);
      Node.getCond()->accept(*this);
      Value *IfCondVal = V;

      Builder.SetInsertPoint(IfBodyBB);

      for (llvm::SmallVector<AST *>::const_iterator I = Node.begin(), E = Node.end(); I != E; ++I)
      {
        (*I)->accept(*this);
      }

      Builder.CreateBr(AfterIfBB);

      llvm::BasicBlock *PreviousCondBB = IfCondBB;
      llvm::BasicBlock *PreviousBodyBB = IfBodyBB;
      Value *PreviousCondVal = IfCondVal;

      for (llvm::SmallVector<elifStmt *, 8>::const_iterator I = Node.beginElif(), E = Node.endElif(); I != E; ++I)
      {
        llvm::BasicBlock *ElifCondBB = llvm::BasicBlock::Create(M->getContext(), "elif.cond", Builder.GetInsertBlock()->getParent());
        llvm::BasicBlock *ElifBodyBB = llvm::BasicBlock::Create(M->getContext(), "elif.body", Builder.GetInsertBlock()->getParent());

        Builder.SetInsertPoint(PreviousCondBB);
        Builder.CreateCondBr(PreviousCondVal, PreviousBodyBB, ElifCondBB);

        Builder.SetInsertPoint(ElifCondBB);
        (*I)->getCond()->accept(*this);
        Value *ElifCondVal = V;

        Builder.SetInsertPoint(ElifBodyBB);
        (*I)->accept(*this);
        Builder.CreateBr(AfterIfBB);

        PreviousCondBB = ElifCondBB;
        PreviousCondVal = ElifCondVal;
        PreviousBodyBB = ElifBodyBB;
      }
      if (Node.beginElse() != Node.endElse())
      {
        llvm::BasicBlock *ElseBB = llvm::BasicBlock::Create(M->getContext(), "else.body", Builder.GetInsertBlock()->getParent());
        Builder.SetInsertPoint(ElseBB);
        for (llvm::SmallVector<AST *>::const_iterator I = Node.beginElse(), E = Node.endElse(); I != E; ++I)
        {
          (*I)->accept(*this);
        }
        Builder.CreateBr(AfterIfBB);

        Builder.SetInsertPoint(PreviousCondBB);
        Builder.CreateCondBr(PreviousCondVal, PreviousBodyBB, ElseBB);
      }
      else
      {
        Builder.SetInsertPoint(PreviousCondBB);
        Builder.CreateCondBr(IfCondVal, PreviousBodyBB, AfterIfBB);
      }

      Builder.SetInsertPoint(AfterIfBB);
    };

    virtual void visit(elifStmt &Node) override
    {
      for (llvm::SmallVector<AST *>::const_iterator I = Node.begin(), E = Node.end(); I != E; ++I)
      {
        (*I)->accept(*this);
      }
    };

    void visit(DeclarationFloat &Node) override{ // TODO
      llvm::SmallVector<Value *, 8> vals;

      // Visit each initializer expression to generate their IR
      llvm::SmallVector<Expr *, 8>::const_iterator E = Node.valBegin();
      for (llvm::SmallVector<llvm::StringRef, 8>::const_iterator Var = Node.varBegin(), End = Node.varEnd(); Var != End; ++Var)
      {
        if (E < Node.valEnd() && *E != nullptr)
        {
          (*E)->accept(*this); // Visit initializer expression
          vals.push_back(V);
        }
        else
        {
          vals.push_back(ConstantFP::get(Type::getFloatTy(M->getContext()), 0.0)); // Default to 0.0
        }
        E++;
      }

      llvm::SmallVector<Value *, 8>::const_iterator itVal = vals.begin();
      for (llvm::SmallVector<llvm::StringRef, 8>::const_iterator S = Node.varBegin(), End = Node.varEnd(); S != End; ++S)
      {
        StringRef Var = *S;

        // Allocate memory for the variable
        nameMapFloat[Var] = Builder.CreateAlloca(Type::getFloatTy(M->getContext()));

        // Store the initializer value
        Builder.CreateStore(*itVal, nameMapFloat[Var]);
        itVal++;
      }
    }

    void visit(DeclarationVar &Node) override{  // TODO: Implement declaration var logic
      Node.getInitializer()->accept(*this); // Visit initializer expression
      Value *initializer = V;

      Type *varType = initializer->getType();
      AllocaInst *allocaInst = Builder.CreateAlloca(varType, nullptr, Node.getVar().str());

      nameMapVar[Node.getVar()] = allocaInst;       // Add to the variable map
      Builder.CreateStore(initializer, allocaInst); // Store the initializer
    }

    void visit(DeclarationConst &Node) override{ // TODO: Implement declaration const logic
      Node.getInitializer()->accept(*this); // Visit initializer expression
      Value *initializer = V;

      // Create an LLVM global constant
      new GlobalVariable(
          *M, initializer->getType(), /*isConstant=*/true,
          GlobalValue::PrivateLinkage, dyn_cast<Constant>(initializer),
          Node.getConstName().str());
    }

    void visit(DefineStmt &Node) override{ // TODO: Implement #define logic
      Constant *constValue = ConstantDataArray::getString(M->getContext(), Node.getMacroValue().str());
      new GlobalVariable(
          *M, constValue->getType(), true, GlobalValue::ExternalLinkage, constValue, Node.getMacroName().str());
    }

    void visit(SwitchStmt &Node) override{ // TODO: Implement switch statement logic
      BasicBlock *AfterSwitchBB = BasicBlock::Create(M->getContext(), "after.switch", Builder.GetInsertBlock()->getParent());
      BasicBlock *DefaultBB = nullptr;

      Node.getCondition()->accept(*this);
      Value *CondVal = V;

      std::vector<std::pair<ConstantInt *, BasicBlock *>> CaseBlocks;

      for (CaseStmt *Case : Node.getCases()) {
          BasicBlock *CaseBB = BasicBlock::Create(M->getContext(), "case", Builder.GetInsertBlock()->getParent());
          Case->accept(*this);
          Final* caseValueFinal = (Final*)Case->getCaseValue();//TODO:chaged from gpt to curser
          llvm::ConstantInt* caseValue = llvm::ConstantInt::get(M->getContext(), 
              llvm::APInt(32, caseValueFinal->getVal().str(), 10));
          CaseBlocks.emplace_back(caseValue, CaseBB);
      }

      if (Node.hasDefault()) {
          DefaultBB = BasicBlock::Create(M->getContext(), "default", Builder.GetInsertBlock()->getParent());
          Builder.SetInsertPoint(DefaultBB);
          Node.getDefault()->accept(*this);
          Builder.CreateBr(AfterSwitchBB);
      }

      // Generate the switch instruction
      SwitchInst *Switch = Builder.CreateSwitch(CondVal, DefaultBB ? DefaultBB : AfterSwitchBB, CaseBlocks.size());
      for (auto &[CaseVal, CaseBB] : CaseBlocks) {
          Switch->addCase(CaseVal, CaseBB);
      }

      Builder.SetInsertPoint(AfterSwitchBB);
    }

    void visit(CaseStmt &Node) override{ // TODO: Implement case statement logic
      BasicBlock *CaseBB = Builder.GetInsertBlock();
      for (AST* stmt : Node.getBody()) {//TODO:changed from gpt to curser
        stmt->accept(*this);
    }
      // Node.getBody()->accept(*this); // Visit case body 
      Builder.CreateBr(CaseBB); // Branch to the next block
    }

    void visit(DoWhileStmt &Node) override{ // TODO: Implement do-while loop logic
      BasicBlock *LoopBB = BasicBlock::Create(M->getContext(), "do.body", Builder.GetInsertBlock()->getParent());
      BasicBlock *CondBB = BasicBlock::Create(M->getContext(), "do.cond", Builder.GetInsertBlock()->getParent());
      BasicBlock *AfterDoBB = BasicBlock::Create(M->getContext(), "after.do", Builder.GetInsertBlock()->getParent());

      Builder.CreateBr(LoopBB);

      Builder.SetInsertPoint(LoopBB);
      for (AST *stmt : Node.getBody()) {
        stmt->accept(*this);
      }

      Builder.CreateBr(CondBB);

      Builder.SetInsertPoint(CondBB);
      Node.getCond()->accept(*this);
      Value *CondVal = V;
      Builder.CreateCondBr(CondVal, LoopBB, AfterDoBB);

      Builder.SetInsertPoint(AfterDoBB);
    }

    void visit(FunctionCall &Node) override{// TODO: Implement function call logic
      std::vector<Value *> Args;
      for (Expr *Arg : Node.getArgs()) {
          Arg->accept(*this);
          Args.push_back(V);
      }

      Function *Callee = M->getFunction(Node.getFuncName().str());
      if (!Callee) {
          llvm::errs() << "Unknown function referenced: " << Node.getFuncName() << "\n";
          return;
      }

      V = Builder.CreateCall(Callee, Args);
    }
  
    void visit(TernaryAssignment &Node) { // TODO
      // Handle ternary assignment code generation
      llvm::errs() << "Generating IR for Ternary Assignment\n";
      Node.getCondition()->accept(*this);
      Node.getTrueExpr()->accept(*this);
      Node.getFalseExpr()->accept(*this);
    }

    void visit(DefaultStmt &Node) { // TODO
      llvm::errs() << "Generating IR for Default Statement\n";
      for (AST *Stmt : Node.getBody()) {
      Stmt->accept(*this);
      }
    }

    void visit(Cast &Node) { // TODO
      llvm::errs() << "Generating IR for Cast\n";
      //Node.getExpr()->accept(*this);
    }

    void visit(BreakStmt &Node) { // TODO
      llvm::errs() << "Generating IR for Break Statement\n";
      // Add break-specific IR generation code here
    }

    void visit(ContinueStmt &Node) { // TODO
      llvm::errs() << "Generating IR for Continue Statement\n";
      // Add continue-specific IR generation code here
    }
  };
}; // namespace

void CodeGen::compile(Program *Tree)
{
  // Create an LLVM context and a module.
  LLVMContext Ctx;
  Module *M = new Module("simple-compiler", Ctx);

  // Create an instance of the ToIRVisitor and run it on the AST to generate LLVM IR.
  ns::ToIRVisitor *ToIR = new ns::ToIRVisitor(M);

  ToIR->run(Tree);

  // Print the generated module to the standard output.
  M->print(outs(), nullptr);
}

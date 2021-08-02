#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "algorithm"
#include "string"
#include "vector"
#include "map"
using namespace llvm;

//===----------------------------------------------------------------------===//
// Lexer
//===----------------------------------------------------------------------===//

// 这个词法解析发现如果不认识这个字符串会返回 0-255，否则会正确返回 tokens
static std::string IdentifierStr; //如果是 tok_identifier 就会赋值
static double NumVal;             //如果是 tok_number 就会赋值
enum Token
{
  tok_eof = -1,
  //comands 关键字
  tok_def = -2,
  tok_extern = -3,
  //primary 基础元素
  tok_identifier = -4,
  tok_number = -5,
};
// gettok - 从标准输入中返回下一个token
static int gettok()
{
  static int LastChar = ' ';

  //跳过任何空白字符
  while (isspace(LastChar))
    LastChar = getchar();

  //identifier: [a-zA-Z][a-zA-Z0-9]*
  if (isalpha(LastChar))
  {
    IdentifierStr = LastChar;
    while (isalnum(LastChar = getchar()))
      IdentifierStr += LastChar;

    if (IdentifierStr == "def")
      return tok_def;
    if (IdentifierStr == "extern")
      return tok_extern;
    return tok_identifier;
  }

  //Number: [0-9.]+
  if (isdigit(LastChar) || LastChar == '.')
  {
    std::string NumStr;
    do
    {
      NumStr += LastChar;
      LastChar = getchar();
    } while (isdigit(LastChar) || LastChar == '.');
    //未处理异常输入 1.23.45.67
    NumVal = strtod(NumStr.c_str(), 0);
    return tok_number;
  }

  //单行注释
  if (LastChar == '#')
  {
    do
      LastChar = getchar();
    while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');
    if (LastChar != EOF)
      return gettok();
  }

  //检查文件尾，不要吞掉EOF
  if (LastChar == EOF)
    return tok_eof;

  //否则，就会返回字符本身的 ascii 值
  int ThisChar = LastChar;
  LastChar = getchar();
  return ThisChar;
}

//===----------------------------------------------------------------------===//
// Abstract Syntax Tree (aka Parse Tree)
//===----------------------------------------------------------------------===//

//AST是忽略了语言的语法捕获了语言的特征的一种中间表示
//所有表达式的基类
namespace
{
  class ExprAST
  {
  public:
    virtual ~ExprAST() {}
    virtual Value *codegen() = 0;
  };
  // NumberExprAST - 数字字面量表达式节点 比如"1.0"
  class NumberExprAST : public ExprAST
  {
    double Val;

  public:
    NumberExprAST(double Val) : Val(Val) {}
    virtual Value *codegen() override;
  };
  // VariableExprAST - 变量节表达式点 比如 "a"
  class VariableExprAST : public ExprAST
  {
    std::string Name;

  public:
    VariableExprAST(const std::string &Name) : Name(Name) {}
    Value *codegen() override;
  };
  // BinaryExprAST - 二元操作符表达式节点
  class BinaryExprAST : public ExprAST
  {
    char Op;
    std::unique_ptr<ExprAST> LHS, RHS;

  public:
    BinaryExprAST(char op, std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS) : Op(op), LHS(move(LHS)), RHS(move(RHS)){};
    Value *codegen() override;
  };
  // CallExprAST - 函数调用表达式节点
  class CallExprAST : public ExprAST
  {
    std::string Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;

  public:
    CallExprAST(const std::string &Callee, std::vector<std::unique_ptr<ExprAST>> Args) : Callee(Callee), Args(move(Args)) {}
    Value *codegen() override;
  };
  // PrototypeAST - 表示函数的原型节点（包含函数自身的信息）
  // 记录了函数的名字和参数名字列表，隐含包含了参数的数量
  // 因为所有参数的数值都是同一个类型double,所以参数的类型不用额外存储
  class PrototypeAST
  {
    std::string Name;
    std::vector<std::string> Args;

  public:
    PrototypeAST(const std::string &name, std::vector<std::string> Args) : Name(name), Args(move(Args)) {}
    Function *codegen();
    const std::string &getName() { return Name; }
  };
  // FunctionAST - 表示函数定义节点
  class FunctionAST
  {
    std::unique_ptr<PrototypeAST> Proto;
    std::unique_ptr<ExprAST> Body;

  public:
    FunctionAST(std::unique_ptr<PrototypeAST> Proto, std::unique_ptr<ExprAST> Body) : Proto(move(Proto)), Body(move(Body)) {}
    Function *codegen();
  };
}
//===----------------------------------------------------------------------===//
// Parser
//===----------------------------------------------------------------------===//
/**
 * CurTok/getNextToken - 提供了一个简单的 token buffer.
 * CurTok 是当前已经解析出来的token
 * getNextToken会从词法解析中读取更新当前token
 **/
static int CurTok;
static int getNextToken()
{
  return CurTok = gettok();
}
// LogError* - 这是一些帮助处理错误的函数
std::unique_ptr<ExprAST> LogError(const char *Str)
{
  fprintf(stderr, "LogError: %s\n", Str);
  return nullptr;
}
std::unique_ptr<PrototypeAST> LogErrorP(const char *Str)
{
  LogError(Str);
  return nullptr;
}

static std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec, std::unique_ptr<ExprAST> LHS);
static std::unique_ptr<ExprAST> ParseExpression();
static std::unique_ptr<ExprAST> ParseNumberExpr();
static std::unique_ptr<ExprAST> ParseParenExpr();
static std::unique_ptr<ExprAST> ParseIdentifierExpr();

// 最基础的元素如数据等
// primary
//   ::= identifierexpr
//   ::= numberexpr
//   ::= parenexpr
std::unique_ptr<ExprAST> ParsePrimary()
{
  switch (CurTok)
  {
  default:
    return LogError("unknow token when expecting an expression");
  case tok_identifier:
    return ParseIdentifierExpr();
  case tok_number:
    return ParseNumberExpr();
  case '(':
    return ParseParenExpr();
  }
}
// 解析数字字面量表达式 numberexper ::= number
static std::unique_ptr<ExprAST> ParseNumberExpr()
{
  auto Result = std::make_unique<NumberExprAST>(NumVal);
  getNextToken(); // eat the number
  return std::move(Result);
}
// 解析括号表达式 parenexpr :: = '(' expression ')'
std::unique_ptr<ExprAST> ParseParenExpr()
{
  getNextToken(); //eat (
  auto V = ParseExpression();
  if (!V)
    return nullptr;

  if (CurTok != ')')
    return LogError("expected ')'");
  getNextToken(); // eat )
  return V;
}
// 解析标识符表达式 identifierexpr
// ::= identifier
// ::= identifier '(' expression* ')'
std::unique_ptr<ExprAST> ParseIdentifierExpr()
{
  std::string IdName = IdentifierStr;
  getNextToken(); // eat identifier
  if (CurTok != '(')
    return std::make_unique<VariableExprAST>(IdName);

  //说明这个是调用表达式了
  getNextToken(); // eat (
  std::vector<std::unique_ptr<ExprAST>> Args;
  if (CurTok != ')')
  {
    while (1)
    {
      //解析每个实参为节点
      if (auto Arg = ParseExpression())
        Args.push_back(move(Arg));
      else
        return nullptr;
      if (CurTok == ')')
        break;

      if (CurTok != ',')
        return LogError("Expected ') or ',' in argument list");
      getNextToken();
    }
  }
  //eat )
  getNextToken();
  return std::make_unique<CallExprAST>(IdName, move(Args));
}
// BinopPrecedence - 定义了每个二元操作符的优先级
// 数值越高，优先级越高
static std::map<char, int> BinopPrecedence;
// GetTokPrecedence - 获取待处理二元操作符token的优先级
static int GetTokPrecedence()
{
  if (!isascii(CurTok))
    return -1;
  //保证它是我们声明的二元操作符
  int TokPrec = BinopPrecedence[CurTok];
  if (TokPrec <= 0)
    return -1;
  return TokPrec;
}

// expression ::= primary binoprhs
// 表达式解析，目前只支持primary和二元操作符的解析
static std::unique_ptr<ExprAST> ParseExpression()
{
  //LHS = left hand side
  //RHS = right handle side
  auto LHS = ParsePrimary();
  if (!LHS)
    return nullptr;

  return ParseBinOpRHS(0, std::move(LHS));
}
// binoprhs ::= ('+' primary)*
static std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec, std::unique_ptr<ExprAST> LHS)
{
  while (1)
  {
    //如果这是一个二元操作符，找到它的优先级
    int TokPrec = GetTokPrecedence();
    //ExprPrec作为结束的最低优先级,比如a[+b*c+d],作为RHS解析应该返回b*c
    if (TokPrec < ExprPrec)
      return LHS;

    int BinOp = CurTok;
    getNextToken(); // eat 二元操作符
    //解析后面的primary表达式
    auto RHS = ParsePrimary();
    if (!RHS)
      return nullptr;
    //比较当前符号和下个一个右手的符号。如果当前的优先级更高就直接集合成一个LHS节点。否则右侧的优先级更高就继续将右边合并成一个RHS节点
    int NextPrec = GetTokPrecedence();
    if (TokPrec < NextPrec)
    {
      //TokPrec + 1作为最低优先级，保证左结合性
      RHS = ParseBinOpRHS(TokPrec + 1, move(RHS));
      if (!RHS)
        return nullptr;
    }
    //合并LHS和RHS两个节点以及操作符BinOp 结合成相对于后面的LHS节点
    LHS = std::make_unique<BinaryExprAST>(BinOp, move(LHS), move(RHS));
  }
}
// prototype ::= id '(' id* ')'
// 解析函数定义基础元素
static std::unique_ptr<PrototypeAST> ParsePrototype()
{
  if (CurTok != tok_identifier)
    return LogErrorP("Expected function name in prototype");

  std::string FnName = IdentifierStr;
  getNextToken();

  if (CurTok != '(')
    return LogErrorP("Expected '(' in prototype");
  //读取定义参数列表的name
  std::vector<std::string> ArgNames;
  //这里的函数定义 参数中间是以空格来分开的
  while (getNextToken() == tok_identifier)
    ArgNames.push_back(IdentifierStr);
  if (CurTok != ')')
    return LogErrorP("Expected ')' in prototype");

  getNextToken(); //eat ')'
  return std::make_unique<PrototypeAST>(FnName, move(ArgNames));
}
// definition ::= 'def' prototype expression
// 解析函数整体定义，包括函数基础元素以及函数体
static std::unique_ptr<FunctionAST> ParseDefinition()
{
  getNextToken(); // eat def
  auto Proto = ParsePrototype();
  if (!Proto)
    return nullptr;

  //解析函数体，应该下面表达式解析目前不支持块，所以现在只是单行函数体
  if (auto E = ParseExpression())
    return std::make_unique<FunctionAST>(move(Proto), move(E));
  return nullptr;
}
// external ::= 'extern' prototype
// 用来支持外部函数调用的，所以它不需要函数体
static std::unique_ptr<PrototypeAST> ParseExtern()
{
  getNextToken(); //eat extern
  return ParsePrototype();
}
// toplevelexpr ::= expression
// 顶层代码解析
static std::unique_ptr<FunctionAST> ParseTopLevelExpr()
{
  if (auto E = ParseExpression())
  {
    // 构建一个匿名的proto 函数基础信息
    auto Proto = std::make_unique<PrototypeAST>("", std::vector<std::string>());
    //将顶层没有函数包裹的代码丢到匿名的函数体中
    return std::make_unique<FunctionAST>(move(Proto), move(E));
  }
  return nullptr;
}

//===----------------------------------------------------------------------===//
// Code Generation
//===----------------------------------------------------------------------===//
//这些全局变量目的都是为了生成IR
//Value类标识SSA：静态一次性赋值。标识每个变量仅被赋值一次
/**
 * y :=1    y1:=1
 * y :=2 => y2:=2
 * x :=y    x1:=y2
 **/
static std::unique_ptr<LLVMContext> TheContext;    //保存了llvm构建的核心数据结构，包括常量池等
static std::unique_ptr<Module> TheModule;          //存储代码段中所有函数和全局变量
static std::unique_ptr<IRBuilder<>> Builder;       //简化指令生成的辅助对象，IRBuilder类模板可有用于跟踪当前指令的插入位置，还带有生成新指令的方法
static std::map<std::string, Value *> NamedValues; //用于记录当前解析AST过程中遇到的作用域内的变量，相当于符号表。目前主要是函数的参数
Value *LogErrorV(const char *Str)
{
  LogError(Str);
  return nullptr;
}

Value *NumberExprAST::codegen()
{
  //APFloat (Arbitrary Precision Float 可用于存储任意精度的浮点数常量)
  //这些数值字面量都被认为是常量，而常量共享同一份内存。猜测这个编译的elf文件结果应该存在数据段中
  return ConstantFP::get(*TheContext, APFloat(Val));
}
Value *VariableExprAST::codegen()
{
  //在函数中查找这个变量，目前可以认为所有变量都已经被提前定义好，都是函数调用传参的符号
  Value *V = NamedValues[Name];
  if (!V)
    LogErrorV("Unknow variable name");
  return V;
}
Value *BinaryExprAST::codegen()
{
  Value *L = LHS->codegen();
  Value *R = RHS->codegen();
  if (!L || !R)
    return nullptr;
  //LLVM指令遵循严格约束，操作数必须是同一个类型，结果必须与操作数类型兼容
  switch (Op)
  {
  case '+':
    //addtmp指令如何组织L,R的插入位置，通过IRbuilder的CreateFAdd来做。
    return Builder->CreateFAdd(L, R, "addtmp");
  case '-':
    return Builder->CreateFSub(L, R, "subtmp");
  case '*':
    return Builder->CreateFMul(L, R, "multmp");
  case '<':
    //fcmp指令要求必须返回但比特类型，kaleidoscope只接收0.0或1.0
    L = Builder->CreateFCmpULT(L, R, "cmptmp");
    //需要通过下面的指令将0/1 bool值转成 0.0/1.0的double值
    return Builder->CreateUIToFP(L, Type::getDoubleTy(*TheContext), "booltmp");
  default:
    return LogErrorV("invalid binary operator");
  }
}
Value *CallExprAST::codegen()
{
  //在全局的模块表中通过函数名查找函数
  Function *CalleeF = TheModule->getFunction(Callee);
  if (!CalleeF)
    return LogErrorV("Unknow function referenced");

  //如果参数不匹配
  if (CalleeF->arg_size() != Args.size())
    return LogErrorV("Incorrect # arguments passed");

  std::vector<Value *> ArgsV;
  for (unsigned i = 0, e = Args.size(); i != e; ++i)
  {
    ArgsV.push_back(Args[i]->codegen());
    if (!ArgsV.back())
      return nullptr;
  }
  return Builder->CreateCall(CalleeF, ArgsV, "calltmp");
}
Function *PrototypeAST::codegen()
{
  //因为数据类型只支持double，所以构建的函数的类型就很固定 double(double,double) etc.
  std::vector<Type *> Doubles(Args.size(), Type::getDoubleTy(*TheContext));
  FunctionType *FT = FunctionType::get(Type::getDoubleTy(*TheContext), Doubles, false);
  //创建一个函数附加到TheModule的符号表中，将函数放到由模块数据布局指定的程序地址空间中
  //ExternalLinkage 表示该函数可能定义与函数之外，且可以被当前模块之外的函数调用
  Function *F = Function::Create(FT, Function::ExternalLinkage, Name, TheModule.get());
  //给函数的参数都设置上名字
  unsigned Idx = 0;
  for (auto &Arg : F->args())
    Arg.setName(Args[Idx++]);
  return F;
}
/**
 * 这里存在一个bug。没有验证当前函数与已经前面定义的extern的函数签名。
 **/
Function *FunctionAST::codegen()
{
  //首先，检查是否已经存在函数，extern声明会提前创建好只包含函数原型的函数
  Function *TheFunction = TheModule->getFunction(Proto->getName());

  if (!TheFunction)
    TheFunction = Proto->codegen(); //不存在就正常创建函数原型

  if (!TheFunction)
    return nullptr;

  //这就说明代码存在函数重复定义，报错
  if (!TheFunction->empty())
    return (Function *)LogErrorV("Function cannot be redefined.");

  //创建一个新的基础代码块插入到指定函数的末尾
  BasicBlock *BB = BasicBlock::Create(*TheContext, "entry", TheFunction);
  //让新指令的位置插入到新基础代码块的末尾
  Builder->SetInsertPoint(BB);

  //记录函数参数到 NameValues 中，为了后面生成函数体内变量的访问指令
  NamedValues.clear();
  for (auto &Arg : TheFunction->args())
    NamedValues[std::string(Arg.getName())] = &Arg;

  //函数体的指令会填充到上面创建的entry代码块中
  if (Value *RetVal = Body->codegen())
  {
    //这个指令用来从函数块中返回控制流到调用者
    Builder->CreateRet(RetVal);

    //验证生成的代码，检查一致性，保证编译的代码能够被正确执行
    verifyFunction(*TheFunction);
    return TheFunction;
  }
  //出错，就从父模块中删除它
  TheFunction->eraseFromParent();
  return nullptr;
}

//===----------------------------------------------------------------------===//
// Top-Level parsing
//===----------------------------------------------------------------------===//
static void InitializeModule()
{
  // 打开一个新的 context and module.
  TheContext = std::make_unique<LLVMContext>();
  TheModule = std::make_unique<Module>("my cool jit", *TheContext);

  // 为上下文创建新的Builder
  Builder = std::make_unique<IRBuilder<>>(*TheContext);
}

static void HandleDefinition()
{
  if (auto FnAST = ParseDefinition())
  {
    if (auto *FnIR = FnAST->codegen())
    {
      fprintf(stderr, "Read function definition:\n");
      FnIR->print(errs());
      fprintf(stderr, "\n");
    }
  }
  else
    getNextToken(); //如果解析失败，跳过错误的token
}
static void HandleExtern()
{
  if (auto ProtoAST = ParseExtern())
  {
    if (auto *FnIR = ProtoAST->codegen())
    {
      fprintf(stderr, "Read extern: \n");
      FnIR->print(errs());
      fprintf(stderr, "\n");
    }
  }
  else
    getNextToken(); //如果解析失败，跳过错误的token
}
static void HandleTopLevelExpression()
{
  //解析顶层表达式到匿名函数中
  if (auto FnAST = ParseTopLevelExpr())
  {
    if (auto *FnIR = FnAST->codegen())
    {
      fprintf(stderr, "Read top-level expression:\n");
      FnIR->print(errs());
      fprintf(stderr, "\n");

      //删除匿名表达式。防止前面的定义影响到后面
      FnIR->eraseFromParent();
    }
  }
  else
    getNextToken(); //如果解析失败，跳过错误的token
}
// top ::= definition | externl | expression | ';'
static void MainLoop()
{
  while (1)
  {
    switch (CurTok)
    {
    case tok_eof:
      return;
    case ';': //忽略顶层表达式的；继续阻塞读取下个token
      fprintf(stderr, "ready> ");
      getNextToken();
      break;
    case tok_def:
      HandleDefinition();
      break;
    case tok_extern:
      HandleExtern();
      break;
    default:
      HandleTopLevelExpression();
      break;
    }
  }
}

//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//

int main()
{
  // 准备二元操作符优先级
  // 1 是最小的优先级.
  BinopPrecedence['<'] = 10;
  BinopPrecedence['+'] = 20;
  BinopPrecedence['-'] = 20;
  BinopPrecedence['*'] = 40; // 最高.

  //准备好当前的CurTok
  fprintf(stderr, "ready> ");
  getNextToken();

  // 构建持有所有代码的模块
  InitializeModule();

  //运行解析执行循环
  MainLoop();

  // 打印所有生成的代码
  TheModule->print(errs(), nullptr);

  return 0;
}
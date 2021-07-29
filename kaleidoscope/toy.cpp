#include "string"
#include "vector"
#include "map"
using namespace std;

//===----------------------------------------------------------------------===//
// Lexer
//===----------------------------------------------------------------------===//

// 这个词法解析发现如果不认识这个字符串会返回 0-255，否则会正确返回 tokens
static string IdentifierStr; //如果是 tok_identifier 就会赋值
static double NumVal;        //如果是 tok_number 就会赋值
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
    string NumStr;
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
  };
  // NumberExprAST - 数字字面量表达式节点 比如"1.0"
  class NumberExprAST : public ExprAST
  {
    double Val;

  public:
    NumberExprAST(double Val) : Val(Val) {}
  };
  // VariableExprAST - 变量节表达式点 比如 "a"
  class VariableExprAST : public ExprAST
  {
    string Name;

  public:
    VariableExprAST(const string &Name) : Name(Name) {}
  };
  // BinaryExprAST - 二元操作符表达式节点
  class BinaryExprAST : public ExprAST
  {
    char Op;
    unique_ptr<ExprAST> LHS, RHS;

  public:
    BinaryExprAST(char op, unique_ptr<ExprAST> LHS, unique_ptr<ExprAST> RHS) : Op(op), LHS(move(LHS)), RHS(move(RHS)){};
  };
  // CallExprAST - 函数调用表达式节点
  class CallExprAST : public ExprAST
  {
    string Callee;
    vector<unique_ptr<ExprAST>> Args;

  public:
    CallExprAST(const string &Callee, vector<unique_ptr<ExprAST>> Args) : Callee(Callee), Args(move(Args)) {}
  };
  // PrototypeAST - 表示函数的原型节点（包含函数自身的信息）
  // 记录了函数的名字和参数名字列表，隐含包含了参数的数量
  // 因为所有参数的数值都是同一个类型double,所以参数的类型不用额外存储
  class PrototypeAST
  {
    string Name;
    vector<string> Args;

  public:
    PrototypeAST(const string &name, vector<string> Args) : Name(name), Args(move(Args)) {}
    const string &getName() { return Name; }
  };
  // FunctionAST - 表示函数定义节点
  class FunctionAST
  {
    unique_ptr<PrototypeAST> Proto;
    unique_ptr<ExprAST> Body;

  public:
    FunctionAST(unique_ptr<PrototypeAST> Proto, unique_ptr<ExprAST> Body) : Proto(move(Proto)), Body(move(Body)) {}
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
unique_ptr<ExprAST> LogError(const char *Str)
{
  fprintf(stderr, "LogError: %s\n", Str);
  return nullptr;
}
unique_ptr<PrototypeAST> LogErrorP(const char *Str)
{
  LogError(Str);
  return nullptr;
}

static unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec, std::unique_ptr<ExprAST> LHS);
static unique_ptr<ExprAST> ParseExpression();
static unique_ptr<ExprAST> ParseNumberExpr();
static unique_ptr<ExprAST> ParseParenExpr();
static unique_ptr<ExprAST> ParseIdentifierExpr();

// 最基础的元素如数据等
// primary
//   ::= identifierexpr
//   ::= numberexpr
//   ::= parenexpr
unique_ptr<ExprAST> ParsePrimary()
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
static unique_ptr<ExprAST> ParseNumberExpr()
{
  auto Result = make_unique<NumberExprAST>(NumVal);
  getNextToken(); // eat the number
  return std::move(Result);
}
// 解析括号表达式 parenexpr :: = '(' expression ')'
unique_ptr<ExprAST> ParseParenExpr()
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
unique_ptr<ExprAST> ParseIdentifierExpr()
{
  string IdName = IdentifierStr;
  getNextToken(); // eat identifier
  if (CurTok != '(')
    return make_unique<VariableExprAST>(IdName);

  //说明这个是调用表达式了
  getNextToken(); // eat (
  vector<unique_ptr<ExprAST>> Args;
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
  return make_unique<CallExprAST>(IdName, move(Args));
}
// BinopPrecedence - 定义了每个二元操作符的优先级
// 数值越高，优先级越高
static map<char, int> BinopPrecedence;
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
static unique_ptr<ExprAST> ParseExpression()
{
  //LHS = left hand side
  //RHS = right handle side
  auto LHS = ParsePrimary();
  if (!LHS)
    return nullptr;

  return ParseBinOpRHS(0, std::move(LHS));
}
// binoprhs ::= ('+' primary)*
static unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec, unique_ptr<ExprAST> LHS)
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
    LHS = make_unique<BinaryExprAST>(BinOp, move(LHS), move(RHS));
  }
}
// prototype ::= id '(' id* ')'
// 解析函数定义基础元素
static unique_ptr<PrototypeAST> ParsePrototype()
{
  if (CurTok != tok_identifier)
    return LogErrorP("Expected function name in prototype");

  string FnName = IdentifierStr;
  getNextToken();

  if (CurTok != '(')
    return LogErrorP("Expected '(' in prototype");
  //读取定义参数列表的name
  vector<string> ArgNames;
  //这里的函数定义 参数中间是以空格来分开的
  while (getNextToken() == tok_identifier)
    ArgNames.push_back(IdentifierStr);
  if (CurTok != ')')
    return LogErrorP("Expected ')' in prototype");

  getNextToken(); //eat ')'
  return make_unique<PrototypeAST>(FnName, move(ArgNames));
}
// definition ::= 'def' prototype expression
// 解析函数整体定义，包括函数基础元素以及函数体
static unique_ptr<FunctionAST> ParseDefinition()
{
  getNextToken(); // eat def
  auto Proto = ParsePrototype();
  if (!Proto)
    return nullptr;

  //解析函数体，应该下面表达式解析目前不支持块，所以现在只是单行函数体
  if (auto E = ParseExpression())
    return make_unique<FunctionAST>(move(Proto), move(E));
  return nullptr;
}
// external ::= 'extern' prototype
// 用来支持外部函数调用的，所以它不需要函数体
static unique_ptr<PrototypeAST> ParseExtern()
{
  getNextToken(); //eat extern
  return ParsePrototype();
}
// toplevelexpr ::= expression
// 顶层代码解析
static unique_ptr<FunctionAST> ParseTopLevelExpr()
{
  if (auto E = ParseExpression())
  {
    // 构建一个匿名的proto 函数基础信息
    auto Proto = make_unique<PrototypeAST>("", vector<string>());
    //将顶层没有函数包裹的代码丢到匿名的函数体中
    return make_unique<FunctionAST>(move(Proto), move(E));
  }
  return nullptr;
}

//===----------------------------------------------------------------------===//
// Top-Level parsing
//===----------------------------------------------------------------------===//

static void HandleDefinition()
{
  if (ParseDefinition())
    fprintf(stderr, "Parsed a function definition.\n");
  else
    getNextToken(); //如果解析失败，跳过错误的token
}
static void HandleExtern()
{
  if (ParseExtern())
    fprintf(stderr, "Parsed an extern.\n");
  else
    getNextToken(); //如果解析失败，跳过错误的token
}
static void HandleTopLevelExpression()
{
  //解析顶层表达式到匿名函数中
  if (ParseTopLevelExpr())
    fprintf(stderr, "Parsed a top-level expr.\n");
  else
    getNextToken(); //如果解析失败，跳过错误的token
}
// top ::= definition | externl | expression | ';'
static void MainLoop()
{
  while (1)
  {
    fprintf(stderr, "ready> ");
    switch (CurTok)
    {
    case tok_eof:
      return;
    case ';'://忽略顶层表达式的；
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

  fprintf(stderr, "ready> ");
  //准备好当前的CurTok
  getNextToken();

  //运行解析执行循环
  MainLoop();
  return 0;
}
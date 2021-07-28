#include "string"
#include "vector"
using namespace std;
//=======词法分析========
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

//==========语法分析=========
//AST是忽略了语言的语法捕获了语言的特征的一种中间表示
//所有表达式的基类
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
}
unique_ptr<PrototypeAST> LogErrorP(const char *Str)
{
    LogError(Str);
    return nullptr;
}
// 下面的函数声明
unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec, std::unique_ptr<ExprAST> LHS);
unique_ptr<ExprAST> ParseExpression();
static unique_ptr<ExprAST> ParseNumberExpr();
unique_ptr<ExprAST> ParseParenExpr();
unique_ptr<ExprAST> ParseIdentifierExpr();
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

static unique_ptr<ExprAST> ParseExpression()
{
    auto LHS = ParsePrimary();
    if (!LHS)
        return nullptr;

    return ParseBinOpRHS(0, std::move(LHS));
}
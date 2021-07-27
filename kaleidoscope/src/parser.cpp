#include "string"
using namespace std;
//所有表达式的基类
class ExprAST
{
public:
  virtual ~ExprAST() {}
};
//NumberExprAST - 数字字面量节点 比如"1.0"
class NumberExprAST : public ExprAST
{
  double Val;

public:
  NumberExprAST(double Val) : Val(Val) {}
};
//VariableExprAST - 变量节点 比如 "a"
class VariableExprAST : public ExprAST
{
  string Name;

public:
  VariableExprAST(const string &Name) : Name(Name) {}
};
//BinaryExprAST - 二元操作符节点
class BinaryExprAST : public ExprAST
{
};
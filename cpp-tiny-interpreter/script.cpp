#include "models.h"
#include "lexer.h"
#include "parser.h"
#include "functional"
#include "iostream"
#define BINARY_NUMBER(opt, lval, rval, type)                                 \
  {                                                                          \
    Entry *res = new Entry();                                                \
    res->numberData = ((type)lval->getNumber())opt((type)rval->getNumber()); \
    return res;                                                              \
  }
#define BINARY_COMP(opt, lval, rval)                           \
  {                                                            \
    Entry *res = new Entry();                                  \
    res->boolData = (lval->getNumber())opt(rval->getNumber()); \
    return res;                                                \
  }
#define BINARY_LOGIC(opt, lval, rval)                      \
  {                                                        \
    Entry *res = new Entry();                              \
    res->boolData = (lval->getBool())opt(rval->getBool()); \
    return res;                                            \
  }

class Script
{
private:
  bool verbose = false;
  Context *kc;

public:
  Script()
  {
    kc = new Context(10);
  }
  void process(string code)
  {
    auto tokens = tokenize(code);
    auto node = parser(tokens);
    auto res = evaluate(node, "");
    if (res != nullptr && verbose)
      cout << "result:" << res->getString();
  }
  Entry *evaluate(ASTNode *node, string indent)
  {
    indent += "\t";
    Entry *res = nullptr;
    switch (node->type)
    {
    case ASTNodeType::Function:
    {
      res = new Entry();
      res->flag = PrimaryFlag::methodFlag;
      res->methodData = new Method();
      res->methodData->parentFrame = kc->frame;
      res->methodData->children = node->children[0]->children;
      //因为暂时不支持对象，所以先去掉this作用域链
      res->methodData->call =(vector<Entry *> arguments)
      {
        //在函数内部再获取函数附加数据
        kc.get();
        StackFrame localFrame = new StackFrame(parentFrame);
        //参数初始化
        for (int i = 0; i < node->params.size() && i < arguments.size(); i++)
          localFrame.declare(node->params[i], arguments[i]);
        //语句执行
        evaluate(executeNode, indent);
      }
    }
    break;
    case ASTNodeType::Block:
    case ASTNodeType::Program:
      for (auto child : node->children)
      {
        res = evaluate(child, indent);
        if (child->type == ASTNodeType::Return)
          break;
      }
      break;
    case ASTNodeType::Return:
      if (node->children.size())
        res = evaluate(node->children[0], indent);
      break;
    case ASTNodeType::Primary:
      res = primaryExpression(node->text);
      break;
    case ASTNodeType::Unary:
    {
      auto val = evaluate(node->children[0], indent);
      res = unaryExpression(node->text, val);
    }
    break;
    case ASTNodeType::Binary:
    {
      auto lval = evaluate(node->children[0], indent);
      auto rval = evaluate(node->children[1], indent);
      res = binaryExpression(node->text, lval, rval);
    }
    break;
    case ASTNodeType::Call:
    {
      //解析出所有参数节点
      Entry *method = kc.get(node->text);
      if (method == nullptr || method->isUndefined())
        throw "not found function " + node->text;
      vector<Entry *> params;
      for (auto paramNode : node->children)
        params.push_back(evaluate(paramNode, indent));
      auto func = (function<void(StackFrame, vector<Entry *>)>)method->methodData;
    }
    break;
    case ASTNodeType::NumberLiteral:
      res = new Entry();
      res->flag = PrimaryFlag::numberFlag;
      res->numberData = stod(node->text);
      break;
    case ASTNodeType::Identifier:
      res = kc.get(node->text);
      break;
    case ASTNodeType::AssignmentStmt:
    case ASTNodeType::VarDeclaration:
    {
      Entry *rval;
      if (node->children.size()) //如果没有就只是变量声明
        rval = evaluate(node->children[0], indent);
      else
      {
        rval = new Entry();
        rval->flag = PrimaryFlag::undefinedFlag;
      }
      kc->set(node->text, rval);
    }
    break;
    }
    return res;
  }
};

Entry *primaryExpression(string text)
{
  Entry *res = new Entry();
  res->flag = PrimaryFlag::undefinedFlag;
  if (text == "true" || text == "false")
  {
    res->flag = PrimaryFlag::boolFlag;
    res->boolData = text == "true";
  }
  return res;
}

Entry *unaryExpression(string opt, Entry *val)
{
  Entry *res = new Entry();
  if (opt == "!")
  {
    res->flag = PrimaryFlag::boolFlag;
    res->boolData = !val->getBool();
    return res;
  }
  else if (opt == "~")
  {
    res->flag == PrimaryFlag::numberFlag;
    res->numberData = ~((int)val->getNumber());
    return res;
  }
  delete res;
  return nullptr;
}

extern unordered_map<string, function<Entry *(Entry *lval, Entry *rval)>> optMap;
Entry *binaryExpression(string opt, Entry *lval, Entry *rval)
{
  if (opt == "+")
    BINARY_NUMBER(+, lval, rval, double);
  else if (opt == "-")
    BINARY_NUMBER(-, lval, rval, double);
  else if (opt == "*")
    BINARY_NUMBER(*, lval, rval, double);
  else if (opt == "/")
    BINARY_NUMBER(/, lval, rval, double);
  else if (opt == "%")
    BINARY_NUMBER(%, lval, rval, long);
  else if (opt == ">>")
    BINARY_NUMBER(>>, lval, rval, long);
  else if (opt == "<<")
    BINARY_NUMBER(<<, lval, rval, long);
  else if (opt == "<")
    BINARY_COMP(<, lval, rval);
  else if (opt == ">")
    BINARY_COMP(>, lval, rval);
  else if (opt == "<=")
    BINARY_COMP(<=, lval, rval);
  else if (opt == ">=")
    BINARY_COMP(>=, lval, rval);
  else if (opt == "==")
    BINARY_COMP(==, lval, rval);
  else if (opt == "!=")
    BINARY_COMP(!=, lval, rval);
  else if (opt == "||")
    BINARY_LOGIC(||, lval, rval);
  else if (opt == "&&")
    BINARY_LOGIC(&&, lval, rval);
  else if (opt == "|")
    BINARY_NUMBER(|, lval, rval, long);
  else if (opt == "^")
    BINARY_NUMBER(^, lval, rval, long);
  else if (opt == "&")
    BINARY_NUMBER(&, lval, rval, long);

  return nullptr;
}
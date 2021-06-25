#include "models.h"
#include "lexer.h"
#include "parser.h"
#include "script.h"
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

Entry *primaryExpression(string text);
Entry *unaryExpression(string opt, Entry *val);
Entry *binaryExpression(string opt, Entry *lval, Entry *rval);

Script::Script()
{
  kc = new Context(10);
}
void Script::process(string code)
{
  auto tokens = tokenize(code);
  auto node = parser(tokens);
  auto res = evaluate(node, "");
  if (res != nullptr && verbose)
    cout << "result:" << res->getString();
}
Entry *Script::evaluate(ASTNode *node, string indent)
{
  indent += "\t";
  Entry *res = nullptr;
  cout << indent << "evaluate: " << ToString(node->type) << " " << node->text << endl;
  switch (node->type)
  {
  case ASTNodeType::Function:
  {
    res = new Entry();
    res->flag = PrimaryFlag::methodFlag;
    res->methodData = new Method();
    res->methodData->stackLink = kc->frame;
    res->methodData->params = node->params;
    res->methodData->funcBody = node->children[0]; //block节点指针
    kc->set(node->text, res);
  }
  break;
  case ASTNodeType::Block:
  case ASTNodeType::Program:
  {
    //定义先执行
    for (auto child : node->children)
    {
      if (child->type == ASTNodeType::VarDeclaration || child->type == ASTNodeType::Function)
        evaluate(child, indent);
    }
    for (auto child : node->children)
    {
      if (child->type == ASTNodeType::VarDeclaration || child->type == ASTNodeType::Function)
        continue;
      res = evaluate(child, indent);
      if (child->type == ASTNodeType::Return)
        break;
    }
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
    Entry *methodEntry = kc->get(node->text);
    if (methodEntry == nullptr || methodEntry->isUndefined())
      throw "not found function " + node->text;
    vector<Entry *> params;
    for (auto paramNode : node->children)
      params.push_back(evaluate(paramNode, indent));
    res = callFunction(methodEntry, params, indent);
  }
  break;
  case ASTNodeType::NumberLiteral:
    res = new Entry();
    res->flag = PrimaryFlag::numberFlag;
    res->numberData = stod(node->text);
    break;
  case ASTNodeType::Identifier:
    res = kc->get(node->text);
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
    res = rval;
  }
  break;
  }
  return res;
};
Entry *Script::callFunction(Entry *methodEntry, vector<Entry *> params, string indent)
{
  kc->pushFrame(new StackFrame(methodEntry->methodData->stackLink, kc->globalScope));
  //实参设置到当前的作用域中
  auto vParams = methodEntry->methodData->params;
  for (int i = 0; i < vParams.size(); i++)
    kc->set(vParams[i], params[i]);
  //开始执行函数语句
  for (auto child : methodEntry->methodData->funcBody->children)
  {
    Entry *res = evaluate(child, indent);
    if (child->type == ASTNodeType::Return)
      return res;
  }
  return nullptr;
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
    res->flag = PrimaryFlag::numberFlag;
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
  if (opt == "-")
    BINARY_NUMBER(-, lval, rval, double);
  if (opt == "*")
    BINARY_NUMBER(*, lval, rval, double);
  if (opt == "/")
    BINARY_NUMBER(/, lval, rval, double);
  if (opt == "%")
    BINARY_NUMBER(%, lval, rval, long);
  if (opt == ">>")
    BINARY_NUMBER(>>, lval, rval, long);
  if (opt == "<<")
    BINARY_NUMBER(<<, lval, rval, long);
  if (opt == "<")
    BINARY_COMP(<, lval, rval);
  if (opt == ">")
    BINARY_COMP(>, lval, rval);
  if (opt == "<=")
    BINARY_COMP(<=, lval, rval);
  if (opt == ">=")
    BINARY_COMP(>=, lval, rval);
  if (opt == "==")
    BINARY_COMP(==, lval, rval);
  if (opt == "!=")
    BINARY_COMP(!=, lval, rval);
  if (opt == "||")
    BINARY_LOGIC(||, lval, rval);
  if (opt == "&&")
    BINARY_LOGIC(&&, lval, rval);
  if (opt == "|")
    BINARY_NUMBER(|, lval, rval, long);
  if (opt == "^")
    BINARY_NUMBER(^, lval, rval, long);
  if (opt == "&")
    BINARY_NUMBER(&, lval, rval, long);

  return nullptr;
}

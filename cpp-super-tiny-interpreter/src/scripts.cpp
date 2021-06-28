#include "scripts.h"
int Script::evaluate(ASTNode &node, string indent)
{
  int result;
  switch (node.type)
  {
  case ASTNodeType::Program:
    for (list<ASTNode>::iterator iter = node.children.begin(); iter != node.children.end(); ++iter)
    {
      result = evaluate(*iter, indent);
    }
    break;
  case ASTNodeType::Additive:
  {
    auto iter = node.children.begin();
    auto child1 = *iter;
    int val1 = evaluate(child1, indent + "\t");
    ++iter;
    auto child2 = *iter;
    int val2 = evaluate(child2, indent + "\t");
    if (node.text == "+")
      result = val1 + val2;
    else
      result = val1 - val2;
    break;
  }
  case ASTNodeType::Multiplicative:
  {
    auto iter = node.children.begin();
    auto child1 = *iter;
    int val1 = evaluate(child1, indent + "\t");
    ++iter;
    auto child2 = *iter;
    int val2 = evaluate(child2, indent + "\t");
    if (node.text == "*")
      result = val1 * val2;
    else
      result = val1 / val2;
    break;
  }
  case ASTNodeType::IntLiteral:
    result = stoi(node.text);
    break;
  case ASTNodeType::Identifier:
  {
    string varName = node.text;
    try
    {
      int value = variables.at(varName);
      result = value;
    }
    catch (exception e)
    {
      throw("unknown variable" + varName);
    }
    break;
  }
  case ASTNodeType::AssignmentStmt:
  {
    auto varName = node.text;
    try
    {
      variables.at(varName);
    }
    catch (exception e)
    {
      throw "unknown variable" + varName;
    }
    //继续给变量赋值
  }
  case ASTNodeType::IntDeclaration:
  {
    auto varName = node.text;
    int varValue;
    if (node.children.size())
    { //说明是初始化语句，给变量赋值
      auto child = node.children.front();
      varValue = evaluate(child, indent + "\t");
    }
    variables.erase(varName);
    variables.emplace(varName, varValue);
    result = varValue;
    break;
  }
  }
  if (verbose)
  {
    cout << indent + "Result:" << result << endl;
  }
  return result;
}

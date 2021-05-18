
#include "models.h"
#include "map"
#include "iostream"
inline string ToString(ASTNodeType v);

void ASTNode::addChild(ASTNode node)
{
  children.push_back(node);
}
void ASTNode::dumpAST(string indent)
{
  cout << indent << ToString(type) << " " << token->text << endl;
  for (list<ASTNode>::iterator iter = children.begin(); iter != children.end(); ++iter)
  {
    iter->dumpAST(indent + indent);
  }
}

map<ASTNodeType, string> strTypeMap = {
    {ASTNodeType::Program, "Program"},
    {ASTNodeType::AssignmentStmt, "AssignmentStmt"},
    {ASTNodeType::VarDeclaration, "VarDeclaration"},
    {ASTNodeType::NumberLiteral, "NumberLiteral"},
    {ASTNodeType::Identifier, "Identifier"},
    {ASTNodeType::Function, "Function"},
    {ASTNodeType::For, "For"},
    {ASTNodeType::DoWhile, "DoWhile"},
    {ASTNodeType::While, "While"},
    {ASTNodeType::Block, "Block"},
    {ASTNodeType::Return, "Return"},
    {ASTNodeType::Binary, "Binary"},
    {ASTNodeType::Unary, "Unary"},
    {ASTNodeType::Primary, "Primary"},
    {ASTNodeType::Call, "Call"},
};
inline string ToString(ASTNodeType v)
{
  if (strTypeMap.find(v) != strTypeMap.end())
    return strTypeMap.at(v);
  return "[Unknown ASTNodeType]";
}
Entry *StackFrame::get(string name)
{
  if (localScope.find(name) != localScope.end())
    return &localScope.at(name);
  else if (stackStackFrame != NULL)
    return stackStackFrame->get(name);
  else if (globalScope->find(name) != globalScope->end())
    return &globalScope->at(name);
  throw name + " is not defined";
}
void StackFrame::set(string name, Entry *value)
{
  if (localScope.find(name) != localScope.end())
  {
    localScope.erase(name);
    localScope.emplace(name, value);
    return;
  }
  else if (stackStackFrame != NULL)
  {
    return stackStackFrame->set(name, value);
  }
  globalScope->erase(name);
  globalScope->emplace(name, value);
}
void StackFrame::declare(string name, Entry *value)
{
  localScope.erase(name);
  localScope.emplace(name, value);
}

void Context::pushFrame(StackFrame *frame)
{
  frames.push_back(frame);
  this->frame = frame;
}
StackFrame *Context::popFrame()
{
  frames.pop_back();
  return frames.back();
}
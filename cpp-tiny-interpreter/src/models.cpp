#include "models.h"
#include "iostream"
inline string ToString(ASTNodeType v);

void ASTNode::addChild(ASTNode *node)
{
  children.push_back(node);
}
void ASTNode::dumpAST(string indent)
{
  cout << indent << ToString(type) << " " << text << endl;
  for (auto child : children)
    child->dumpAST(indent + indent);
}

unordered_map<ASTNodeType, string> strTypeMap = {
    {ASTNodeType::Program, "Program"},
    {ASTNodeType::AssignmentStmt, "AssignmentStmt"},
    {ASTNodeType::VarDeclaration, "VarDeclaration"},
    {ASTNodeType::NumberLiteral, "NumberLiteral"},
    {ASTNodeType::Identifier, "Identifier"},
    {ASTNodeType::Function, "Function"},
    // {ASTNodeType::For, "For"},
    // {ASTNodeType::DoWhile, "DoWhile"},
    // {ASTNodeType::While, "While"},
    {ASTNodeType::Block, "Block"},
    {ASTNodeType::Return, "Return"},
    {ASTNodeType::Binary, "Binary"},
    {ASTNodeType::Unary, "Unary"},
    {ASTNodeType::Primary, "Primary"},
    {ASTNodeType::Call, "Call"},
};
string ToString(ASTNodeType v)
{
  if (strTypeMap.find(v) != strTypeMap.end())
    return strTypeMap.at(v);
  return "[Unknown ASTNodeType]";
}
Entry *StackFrame::get(string name)
{
  if (localScope.find(name) != localScope.end())
    return localScope.at(name);
  else if (stackStackFrame != nullptr)
    return stackStackFrame->get(name);
  else if (globalScope.find(name) != globalScope.end())
    return globalScope.at(name);
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
  else if (stackStackFrame != nullptr)
  {
    return stackStackFrame->set(name, value);
  }
  globalScope.erase(name);
  globalScope.emplace(name, value);
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
Context::Context(int maxFrameSize) : maxFrameSize(maxFrameSize)
{
}
Entry *Context::get(string name)
{
  if (frame != nullptr)
    return frame->get(name);
  if (globalScope.find(name) != globalScope.end())
    return globalScope.at(name);
  throw name + " is not defined";
}
void Context::set(string name, Entry *value)
{
  if (frame != nullptr)
    return frame->set(name, value);
  globalScope.erase(name);
  globalScope.emplace(name, value);
}
void Context::declare(string name, Entry *value)
{
  if (frame != nullptr)
    return frame->declare(name, value);
  globalScope.erase(name);
  globalScope.emplace(name, value);
}
//Entry???
bool Entry::isNumber()
{
  return flag == PrimaryFlag::numberFlag;
}
bool Entry::isString()
{
  return flag == PrimaryFlag::stringFlag;
}
bool Entry::isBool()
{
  return flag == PrimaryFlag::boolFlag;
}
bool Entry::isUndefined()
{
  return flag == PrimaryFlag::undefinedFlag;
}
double Entry::getNumber()
{
  if (isNumber())
    return numberData;
  if (isBool())
    return boolData == true ? 1 : 0;
  return 0;
}
bool Entry::getBool()
{
  if (isBool())
    return boolData;
  if (isNumber())
    return numberData != 0;
  if (isString())
    return stringData.size() != 0;
  return false;
}
string Entry::getString()
{
  if (isString())
    return stringData;
  if (isNumber())
    return to_string(numberData);
  if (isBool())
    return boolData ? "true" : "false";
  return "";
}

unordered_map<string, TokenType> defineKeywords = {
    {"break", TokenType::BREAK},
    {"case", TokenType::CASE},
    {"continue", TokenType::CONTINUE},
    {"default", TokenType::DEFAULT},
    {"delete", TokenType::UNARYOP},
    {"do", TokenType::DO},
    {"else", TokenType::ELSE},
    {"false", TokenType::PRIMARY},
    {"for", TokenType::FOR},
    {"function", TokenType::FUNCTION},
    {"if", TokenType::IF},
    {"in", TokenType::IN},
    {"new", TokenType::NEW},
    {"null", TokenType::PRIMARY},
    {"return", TokenType::RETURN},
    {"switch", TokenType::SWITCH},
    {"this", TokenType::PRIMARY},
    {"true", TokenType::PRIMARY},
    {"typeof", TokenType::UNARYOP},
    {"var", TokenType::VAR},
    {"void", TokenType::UNARYOP},
    {"while", TokenType::WHILE},
    {"with", TokenType::WITH},
    //R,SERVE_JAVA_KEYWORD}S
    {"abstract", TokenType::RESERVED},
    {"boolean", TokenType::RESERVED},
    {"byte", TokenType::RESERVED},
    {"catch", TokenType::RESERVED},
    {"char", TokenType::RESERVED},
    {"class", TokenType::RESERVED},
    {"const", TokenType::RESERVED},
    {"double", TokenType::RESERVED},
    {"extends", TokenType::RESERVED},
    {"final", TokenType::RESERVED},
    {"finally", TokenType::RESERVED},
    {"float", TokenType::RESERVED},
    {"goto", TokenType::RESERVED},
    {"implements", TokenType::RESERVED},
    {"import", TokenType::RESERVED},
    {"instanceof", TokenType::RESERVED},
    {"int", TokenType::RESERVED},
    {"interface", TokenType::RESERVED},
    {"long", TokenType::RESERVED},
    {"native", TokenType::RESERVED},
    {"package", TokenType::RESERVED},
    {"private", TokenType::RESERVED},
    {"protected", TokenType::RESERVED},
    {"public", TokenType::RESERVED},
    {"short", TokenType::RESERVED},
    {"static", TokenType::RESERVED},
    {"super", TokenType::PRIMARY},
    {"synchronized", TokenType::RESERVED},
    {"throw", TokenType::RESERVED},
    {"throws", TokenType::RESERVED},
    {"transient", TokenType::RESERVED},
    {"try", TokenType::RESERVED},
    {"volatile", TokenType::RESERVED},
};

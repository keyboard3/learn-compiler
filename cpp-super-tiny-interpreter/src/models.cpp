
#include "models.h"
#include "map"
inline string ToString(ASTNodeType v);

void ASTNode::addChild(ASTNode node)
{
  children.push_back(node);
}
void ASTNode::dumpAST(string indent)
{
  cout << indent << ToString(type) << " " << text << endl;
  for (list<ASTNode>::iterator iter = children.begin(); iter != children.end(); ++iter)
  {
    iter->dumpAST(indent + indent);
  }
}

map<ASTNodeType, string> strTypeMap = {
    {ASTNodeType::Program, "Program"},
    {ASTNodeType::AssignmentStmt, "AssignmentStmt"},
    {ASTNodeType::IntDeclaration, "IntDeclaration"},
    {ASTNodeType::Additive, "Additive"},
    {ASTNodeType::Multiplicative, "Multiplicative"},
    {ASTNodeType::IntLiteral, "IntLiteral"},
    {ASTNodeType::Identifier, "Identifier"},
};
inline string ToString(ASTNodeType v)
{
  try
  {
    return strTypeMap.at(v);
  }
  catch (exception e)
  {
    return "[Unknown ASTNodeType]";
  }
}
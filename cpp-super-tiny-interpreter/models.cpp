
#include "models.h"
inline const char *ToString(ASTNodeType v);

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

inline const char *ToString(ASTNodeType v)
{
  switch (v)
  {
  case ASTNodeType::Program:
    return "Program";
  case ASTNodeType::AssignmentStmt:
    return "AssignmentStmt";
  case ASTNodeType::IntDeclaration:
    return "IntDeclaration";
  case ASTNodeType::Additive:
    return "Additive";
  case ASTNodeType::Multiplicative:
    return "Multiplicative";
  case ASTNodeType::IntLiteral:
    return "IntLiteral";
  case ASTNodeType::Identifier:
    return "Identifier";
  default:
    return "[Unknown ASTNodeType]";
  }
}
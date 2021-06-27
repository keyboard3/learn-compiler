
#include "models.h"
#include "map"

void ASTNode::addChild(ASTNode *node)
{
  children.push_back(node);
}
void ASTNode::dumpAST(string indent)
{
  cout << indent << ToString(type) << " " << text << endl;
  for (auto child : children)
  {
    child->dumpAST(indent + indent);
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
map<OP_TYPE, string> strOpTypeMap = {
    {OP_TYPE::NUMBER, "NUMBER"},
    {OP_TYPE::NAME, "NAME"},
    {OP_TYPE::ASSIGN, "ASSIGN"},
    {OP_TYPE::INT_DECL, "INT_DECL"},
    {OP_TYPE::ADD, "ADD"},
    {OP_TYPE::MINUS, "MINUS"},
    {OP_TYPE::MUTI, "MUTI"},
    {OP_TYPE::DIVID, "DIVID"},
};
inline string ToString(ASTNodeType type)
{
  if (strTypeMap.find(type) == strTypeMap.end())
    return "[Unknown ASTNodeType]";
  return strTypeMap[type];
}
inline string ToString(OP_TYPE type)
{
  if (strOpTypeMap.find(type) == strOpTypeMap.end())
    return "[Unknown ASTNodeType]";
  return strOpTypeMap[type];
}
Atom *getAtom(Context &kc, int index)
{
  return kc.atoms[index];
}
Atom *generateAtom(int val)
{
  Atom *atom = new Atom();
  atom->flags = ATOM_TYPE::ATOM_NUMBER;
  atom->ival = val;
  return atom;
}
Atom *generateAtom(string val)
{
  Atom *atom = new Atom();
  atom->flags = ATOM_TYPE::ATOM_NAME;
  atom->sval = val;
  return atom;
}
void NewCodeGenerator(Context &kc)
{
  CodeGenerator *cg = new CodeGenerator();
  //固定指令区长度。目前不支持动态扩容指令区
  cg->ptr = cg->base = (uint8_t *)malloc(sizeof(uint8_t) * 1000);
  kc.cg = cg;
}
void NewScript(Context &kc)
{
  CodeGenerator *cg = kc.cg;
  Script *script = new Script();
  script->length = cg->ptr - cg->base;
  script->code = (uint8_t *)malloc(sizeof(uint8_t) * script->length);
  memcpy(script->code, cg->base, script->length);
  kc.script = script;
}
void emit1(CodeGenerator *cg, OP_TYPE type)
{
  cout << "emit1 " << ToString(type) << endl;
  cg->ptr[0] = (uint8_t)type;
  cg->ptr++;
}
void emit2(CodeGenerator *cg, OP_TYPE type, uint8_t op1)
{
  cout << "emit2 " << ToString(type) << endl;
  cg->ptr[0] = (uint8_t)type;
  cg->ptr[1] = op1;
  cg->ptr += 2;
}
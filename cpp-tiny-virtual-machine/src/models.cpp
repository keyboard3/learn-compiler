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

// 指令生成解析相关
Datum *popDatum(Stack *stack)
{
  return --stack->ptr;
}
void pushDatum(Stack *stack, Datum *d)
{
  stack->ptr = d;
  stack->ptr++;
}
void pushSymbol(Symbol *sym, Scope *scope)
{
  auto next = scope->list;
  sym->next = next;
  scope->list = sym;
}
void pushAtom(Atom *atom, Stack *stack)
{
  Datum d;
  d.type = DATUM_TYPE::ATOM;
  d.u.atom = atom;
  stack->ptr[0] = d;
  stack->ptr++;
}
void pushNumber(number value, Stack *stack)
{
  Datum val;
  val.u.nval = value;
  val.type = DATUM_TYPE::NUMBER;
  stack->ptr[0] = val;
  stack->ptr++;
}
bool resolveValue(Context *context, Datum *dp)
{
  bool isOk = resolveSymbol(context, dp);
  auto primaryAtomToDatum = [&]()
  {
    if (dp->type == DATUM_TYPE::ATOM)
    {
      auto atom = dp->u.atom;
      if (atom->type == ATOM_TYPE::NUMBER)
      {
        dp->type = DATUM_TYPE::NUMBER;
        dp->u.nval = atom->nval;
        // cout << "primary value:" << dp->u.nval << endl;
        return true;
      }
    }
    // else if (dp->type == DATUM_TYPE::NUMBER)
    //   cout << "primary value:" << dp->u.nval << endl;
    return false;
  };
  if (dp->type == DATUM_TYPE::SYMBOL)
  {
    auto sym = dp->u.sym;
    if (sym->type == SYMBOL_TYPE::PROPERTY)
    {
      Property *prop = (Property *)sym->entry.value;
      memcpy(dp, prop->datum, sizeof(Datum));
      primaryAtomToDatum();
      return true;
    }
    if (sym->entry.value)
    {
      memcpy(dp, (Datum *)sym->entry.value, sizeof(Datum));
      primaryAtomToDatum();
      return true;
    }
    Frame *targetFp = nullptr;
    for (Frame *fp = context->stack.frame; fp != nullptr; fp = fp->down)
    {
      if (fp->fun->scope == sym->scope)
      {
        targetFp = fp;
        break;
      }
    }
    if (targetFp == nullptr)
    {
      cout << "未找到栈帧" << endl;
      return false;
    }
    //证明这个是从根栈帧上的符号，所以要找到scope对应的栈帧
    if (sym->type == SYMBOL_TYPE::ARGUMENT)
      memcpy(dp, &targetFp->argv[sym->slot], sizeof(Datum));
    else if (sym->type == SYMBOL_TYPE::VARIABLE)
      memcpy(dp, &targetFp->vars[sym->slot], sizeof(Datum));
    primaryAtomToDatum();
    return true;
  }
  else
    return primaryAtomToDatum();
  return false;
}
bool resolveSymbol(Context *context, Datum *dp)
{
  if (dp->type == DATUM_TYPE::ATOM)
  {
    auto atom = dp->u.atom;
    if (atom->type == ATOM_TYPE::NAME)
    {
      auto sym = findSymbol(context->staticLink, atom);
      if (!sym)
        return false;
      //只有name atom需要去作用域中找到是否存在已解析的符号
      dp->type = DATUM_TYPE::SYMBOL;
      dp->u.sym = sym;
      return true;
    }
  }
  else if (dp->type == DATUM_TYPE::SYMBOL)
    return true;
  return false;
}

Symbol *findSymbol(Scope *scope, Atom *atom)
{
  if (scope == nullptr)
    return nullptr;
  for (Symbol *sym = scope->list; sym != nullptr; sym = sym->next)
  {
    if (sym->entry.key == atom)
      return sym;
  }
  return findSymbol(scope->parent, atom);
}
Atom *getAtom(Script *script, int index)
{
  return script->atoms[index];
}
Atom *generateAtom(number val)
{
  Atom *atom = new Atom();
  atom->type = ATOM_TYPE::NUMBER;
  atom->nval = val;
  return atom;
}
Atom *generateAtom(string val)
{
  Atom *atom = new Atom();
  atom->type = ATOM_TYPE::NAME;
  atom->sval = val;
  return atom;
}
//TODO 可以尝试改成泛型
unsigned getAtomIndex(Script *script, number val)
{
  unsigned index = 0;
  for (auto atom : script->atoms)
  {
    if (atom->nval == val)
      return index;
    index++;
  }
  Atom *at = generateAtom(val);
  script->atoms.push_back(at);
  return script->atoms.size() - 1;
}
unsigned getAtomIndex(Script *script, string val)
{
  unsigned index = 0;
  for (auto atom : script->atoms)
  {
    if (atom->sval == val)
      return index;
    index++;
  }
  Atom *at = generateAtom(val);
  script->atoms.push_back(at);
  return script->atoms.size() - 1;
}
void dumpScope(Scope *scope)
{
  for (Symbol *sym = scope->list; sym; sym = sym->next)
  {
    Datum *temp = (Datum *)sym->entry.value;
    cout << sym->entry.key->sval << " " << temp->u.nval << endl;
  }
}
string to_op_str(OP_TYPE op)
{
  switch (op)
  {
  case OP_TYPE::NUMBER:
    return "number";
  case OP_TYPE::NAME:
    return "name";
  case OP_TYPE::ADD:
    return "add";
  case OP_TYPE::MINUS:
    return "minus";
  case OP_TYPE::MUTIL:
    return "mutil";
  case OP_TYPE::DIVID:
    return "divid";
  case OP_TYPE::ASSIGN:
    return "assign";
  case OP_TYPE::RETURN:
    return "return";
  case OP_TYPE::CALL:
    return "call";
  }
}
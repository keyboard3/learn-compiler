#include "iostream"
#include "string"
#include "list"
#include "vector"
#include "map"

using namespace std;
#ifndef def_models
#define def_models

enum class DfaState
{
  int1,
  int2,
  int3,
  Initial,
  Id,
  IntLiteral,
  GT,
  GE,
  Assignment,
  Plus,
  Minus,
  Star,
  Slash,
  Equals,
  LeftParen,
  RightParen,
  SemiColon
};

enum class TokenType
{
  Int,
  Identifier,
  IntLiteral,
  GT,
  GE,
  Assignment,
  Plus,
  Minus,
  Star,
  Slash,
  Equals,
  LeftParen,
  RightParen,
  SemiColon,
};

enum class ASTNodeType
{
  Program,
  AssignmentStmt,
  IntDeclaration,
  Additive,
  Multiplicative,
  IntLiteral,
  Identifier
};

class Token
{
public:
  TokenType type;
  string text;
};

class ASTNode
{
public:
  ASTNodeType type;
  string text;
  vector<ASTNode *> children;
  ASTNode(ASTNodeType type, string text) : type(type), text(text) {}
  void addChild(ASTNode *node);
  void dumpAST(string indent);
};
enum class OP_TYPE
{
  NUMBER,
  NAME,
  ASSIGN,
  INT_DECL,
  ADD,
  MINUS,
  MUTI,
  DIVID,
};
enum class ATOM_TYPE
{
  ATOM_NAME,
  ATOM_NUMBER
};
struct Atom
{
  ATOM_TYPE flags;
  int ival;
  string sval;
};
struct CodeGenerator
{
  uint8_t *base; //指令的起始地址
  uint8_t *ptr;  //指令移动指针
};
struct Script
{
  uint8_t *code;
  unsigned length;
};
struct Context
{
  vector<Atom *> atoms;
  map<string, Atom *> globalScope;
  CodeGenerator *cg;
  Script *script;
};
Atom *getAtom(Context &kc, int index);
Atom *generateAtom(string val);
Atom *generateAtom(int val);
void NewCodeGenerator(Context &kc);
void NewScript(Context &kc);
void emit1(CodeGenerator *cg, OP_TYPE type);
void emit2(CodeGenerator *cg, OP_TYPE type, uint8_t op1);
inline string ToString(ASTNodeType v);
inline string ToString(OP_TYPE v);
#endif
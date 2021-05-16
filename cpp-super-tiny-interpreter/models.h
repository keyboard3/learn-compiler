#include "iostream"
#include "string"
#include "list"

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
  list<ASTNode> children;
  ASTNode(ASTNodeType type, string text) : type(type), text(text) {}
  void addChild(ASTNode node);
  void dumpAST(string indent);
};
#endif
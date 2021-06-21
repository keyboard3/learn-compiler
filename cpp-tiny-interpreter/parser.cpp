#include "models.h"
#include "vector"
#define ASSET_TS(tokens, tt)      \
  if (tokens.empty())             \
    return nullptr;               \
  if (tokens.front()->type != tt) \
    return nullptr;
#define IS_TYPE(tokens, tt) \
  !tokens.empty() && tokens.front()->type == tt

vector<long> getRootFuncs();
ASTNode *statement(list<Token *> &tokens);
typedef ASTNode *(*Statement)(list<Token *> &); //动态表达式函数类型定义
ASTNode *parser(list<Token *> &tokens)
{
  auto rootNode = new ASTNode(ASTNodeType::Program, "pwc");
  while (!tokens.empty())
  {
    auto child = statement(tokens);
    rootNode->addChild(child);
  }
  return rootNode;
}
ASTNode *statement(list<Token *> &tokens)
{
  auto funclist = getRootFuncs();
  for (auto func : funclist)
  {
    auto child = ((Statement)func)(tokens);
    if (child != nullptr)
      return child;
  };
  return nullptr;
}
/**
 * var变量声明语句
 * varDeclaration :: 'var' Id ( '=' expressionStatement)?
 * @param tokens
 */
ASTNode *varDeclare(list<Token *> &tokens)
{
  ASSET_TS(tokens, TokenType::VAR);
  tokens.pop_front();
  if (!IS_TYPE(tokens, TokenType::VAR))
    throw "variable name expected";
  auto node = new ASTNode(ASTNodeType::VarDeclaration, tokens.front()->text);
  tokens.pop_front();
  if (!IS_TYPE(tokens, TokenType::ASSIGN))
    return node;
  auto child = expressionStatement(tokens);
  if (child == nullptr)
    throw "invalide variable initialization, expecting an expression";
  node->addChild(child);
  return node;
}
/**
 * 赋值语句
 * assignment :: Identifier '=' expressionStatement;
 * @param tokens
 */
ASTNode *assignmentStatement(list<Token *> &tokens)
{
  ASSET_TS(tokens, TokenType::NAME);
  auto nameToken = tokens.front();
  tokens.pop_front();
  if (!IS_TYPE(tokens, TokenType::ASSIGN))
  {
    tokens.push_front(nameToken);
    return nullptr;
  }
  auto node = new ASTNode(ASTNodeType::AssignmentStmt, nameToken->text);
  auto child = expressionStatement(tokens);
  if (child != nullptr)
    node->addChild(child);
  return node;
};
/**
 * 基础表达式
 * primary :: NumberLiteral | Identifier | '(' expressionStatement ')'
 * @param tokens
 */
ASTNode *primary(list<Token *> &tokens)
{
  ASTNode *node = nullptr;
  if (IS_TYPE(tokens, TokenType::PRIMARY))
    node = new ASTNode(TokenType::PRIMARY, tokens.front()->text);
  if (IS_TYPE(tokens, ASTNodeType::NumberLiteral))
    node = new ASTNode(TokenType::NumberLiteral, tokens.front()->text);
  if (IS_TYPE(tokens, ASTNodeType::Identifier))
    node = new ASTNode(TokenType::Identifier, tokens.front()->text);
  if (IS_TYPE(tokens, TokenType::LP))
  {
    tokens.pop_front();
    node = expressionStatement(tokens);
    if (node == nullptr)
      throw "expecting an additive expression inside parenthesis";
    if (tokens.front() != TokenType::RP)
      throw "expecting right parenthesis";
  }
  if (node != nullptr)
    tokens.pop_front();
  return node;
}
/**
 * 函数调用声明 callExpression :: Identifier '(' primary (',',primary) ')'
 */
ASTNode *callExpression(list<Token *> &tokens)
{
  ASSET_TS(tokens, TokenType::NAME);
  auto nameToken = tokens.front();
  tokens.pop_front();
  if (!IS_TYPE(tokens, TokenType::LP))
  {
    tokens.push_front(nameToken);
    return nullptr;
  }
  tokens.pop_front();
  auto node = new ASTNode(ASTNodeType::Call, nameToken->text);
  if (IS_TYPE(tokens, TokenType::RP))
  {
    tokens.pop_front();
    return node;
  }
  auto child = primary(tokens);
  if (child != nullptr)
    node->addChild(child);
  while (!tokens.empty() && tokens.front() != ASTNodeType::RP)
  {
    auto child = primary(tokens);
    if (child != nullptr)
      node->addChild(child);
  }
  if (tokens.empty())
    throw "call args syntax error";
  tokens.pop_front();
  return node;
};
/**
 * 一元前缀表达式 unaryExpression :: [!,~] primary
 */
ASTNode *unaryExpression(list<Token *> &tokens)
{
  ASSET_TS(tokens, ASTNodeType::Unary);
  auto node = new ASTNode(ASTNodeType::Unary, tokens.front()->text);
  auto child = primary(tokens);
  if (child == nullptr)
    throw "mising primary";
  node->addChild(child);
  return node;
};
/**
 * 乘除法表达式
 * multiplicative :: primary ( '*'|'/' primary)*
 * @param tokens
 */
ASTNode *multiplicative(list<Token *> &tokens)
{
  auto child1 = primary(tokens);
  if (child1 == nullptr)
    return child1;

  auto node = child1;
  while (!tokens.empty())
  {
    if (!IS_TYPE(tokens, TokenType::MULOP))
      break;
    auto opToken = tokens.front();
    tokens.pop_front();
    auto child2 = primary(tokens);
    if (child2 == nullptr)
      throw "invalid additive expression, expecting the right part.";
    node = new ASTNode(ASTNodeType::Binary, opToken->text);
    node->addChild(child1);
    node->addChild(child2);
    chil1 = node;
  }
  return node;
}
/**
 * 加减法表达式：+-属于加
 * additiveExpression :: multiplicative ( '+'|'-' multiplicative)*
 * @param tokens
 */
ASTNode *additive(list<Token *> &tokens)
{
  auto child1 = multiplicative(tokens);
  if (child1 == nullptr)
    return child1;

  auto node = child1;
  while (!tokens.empty())
  {
    auto opToken = tokens.front();
    if (!IS_TYPE(tokens, TokenType::PLUS) || !IS_TYPE(tokens, TokenType::MINUS))
      break;
    tokens.pop_front();
    auto child2 = multiplicative(tokens);
    if (child2 == nullptr)
      throw "invalid additive expression, expecting the right part.";
    node = new ASTNode(ASTNodeType::Binary, opToken->text);
    node->addChild(child1);
    node->addChild(child2);
    child1 = node;
  }
  return node;
}
/**
 * 二元表达式 BinaryExpression :: [<<,>>,==,!=,<,<=,>,>=,additive]
 */
ASTNode *binaryExpression(list<Token *> &tokens)
{
  auto child = additive(tokens);
  if (child == nullptr || child == ASTNodeType::Binary)
    return child;
  //说明 child 是普通的 primary 可与后面的结合成二元表达式
  switch (tokens.front()->type)
  {
  case TokenType::SHOP:   //<< >>
  case TokenType::RELOP:  //< > <= >=
  case TokenType::EQOP:   //== !=
  case TokenType::OR:     // ||
  case TokenType::AND:    // &&
  case TokenType::BITOR:  // |
  case TokenType::BITXOR: // ^
  case TokenType::BITAND: // &
    auto opToken = tokens.front();
    tokens.pop_front();
    auto child2 = additive(tokens);
    if (child2 == nullptr)
      throw "invalid additive expression, expecting the right part.";
    auto node = new ASTNode(ASTNodeType::Binary, opToken->text);
    node->addChild(child);
    node->addChild(child2);
    return node;
  default:
    return child;
  }
};
ASTNode *expressionStatement(list<Token *> &tokens)
{
  vector<long> funcs = {
      (long)&callExpression,
      (long)&unaryExpression,
      (long)&binaryExpression,
  };
  for (auto func : funcs)
  {
    auto node = ((Statement)func)(tokens);
    if (node != nullptr)
      return node;
  }
};
/**
 * 块级语句 blackStatement :: '{' (statement)* '}'
 */
ASTNode *blockStatement(list<Token *> &tokens)
{
  ASSET_TS(tokens, TokenType::LC);
  auto node = new ASTNode(ASTNodeType::Block, "{}");
  tokens.pop_front();
  while (!tokens.empty())
  {
    if (IS_TYPE(tokens, TokenType::RC))
    {
      tokens.pop_front();
      return node;
    }
    auto child = statement();
    if (child != nullptr)
      node->addChild(child);
  }
  throw "block missing }";
};
/**
 * 表达式语句 returnStatement :: return expressionStatement?
 */
ASTNode *returnStatement(list<Token *> &tokens)
{
  ASSET_TS(tokens, TokenType::RETURN);
  auto node = new ASTNode(ASTNodeType::Return, tokens.front()->text);
  tokens.pop_front();
  auto child = expressionStatement(tokens);
  if (child != nullptr)
    node->addChild(child);
  return node;
};
/**
 * 函数声明语句 functionDefinition :: function Identifier '(' Identifier (',' Identifier)* ')' blockStatement
 */
ASTNode *functionDefinition(list<Token *> &tokens)
{
  ASSET_TS(tokens, TokenType::FUNCTION);
  tokens.pop_front();
  if (!IS_TYPE(tokens, TokenType::NAME))
    throw "function must have name";
  auto nameToken = tokens.front();
  tokens.pop_front();
  if (!IS_TYPE(tokens, TokenType::LP))
    throw "function define error";
  //函数参数不做签名检查，不保留参数
  while (!tokens.empty())
  {
    if (IS_TYPE(tokens, TokenType::RP))
    {
      tokens.pop_front();
      auto node = new ASTNode(ASTNodeType::Function, nameToken->text);
      auto child = blockStatement(tokens);
      if (child == nullptr)
        throw "function must have body";
      node->addChild(child);
      return node;
    }
    tokens.pop_front();
  }
  return nullptr;
};
vector<long> getRootFuncs()
{
  vector<long> funcs = {
      (long)&varDeclare,
      (long)&assignmentStatement,
      (long)&expressionStatement,
      (long)&blockStatement,
      (long)&returnStatement,
      (long)&functionDefinition,
      (long)&callExpression};
  return funcs;
}
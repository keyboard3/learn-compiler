#include "lexer.h"

#define MATCH_TOKEN(token, ctype) \
  if (token.type != ctype)        \
    return NULL;

#define NOT_EMPTY(tokens) \
  if (tokens.empty())     \
    return NULL;
ASTNode* prog(list<Token> &tokens);
ASTNode *expressionStatement(list<Token> &tokens);
ASTNode *assignmentStatement(list<Token> &tokens);
ASTNode *intDeclare(list<Token> &tokens);
ASTNode *additive(list<Token> &tokens);
ASTNode *multiplicative(list<Token> &tokens);
ASTNode *primary(list<Token> &tokens);
ASTNode *statementEnd(ASTNode *node, list<Token> &tokens);

ASTNode* parse(string code)
{
  auto tokens = tokenize(code);
  ASTNode* rootNode = prog(tokens);
  return rootNode;
}

/**
 * AST根节点
 * @param tokens
 */
ASTNode* prog(list<Token> &tokens)
{
  ASTNode* node = new ASTNode(ASTNodeType::Program, "pwc");
  while (!tokens.empty())
  {
    ASTNode *child = intDeclare(tokens);
    if (!child)
      child = assignmentStatement(tokens);
    if (!child)
      child = expressionStatement(tokens);
    if (child)
    {
      node->addChild(child);
    }
  }
  return node;
}

/**
 * 表达式语句 expressionStatement :: additiveExpression ';’;
 */
ASTNode *expressionStatement(list<Token> &tokens)
{
  return statementEnd(additive(tokens), tokens);
}

/**
 * 赋值语句
 * assignment :: Identifier '=' additiveExpression;
 * @param tokens
 */
ASTNode *assignmentStatement(list<Token> &tokens)
{
  NOT_EMPTY(tokens);

  ASTNode *node;
  Token token = tokens.front();
  MATCH_TOKEN(token, TokenType::Identifier);
  tokens.pop_front(); // - Identifier
  if (tokens.empty() || tokens.front().type != TokenType::Assignment)
  {
    tokens.push_front(token); // + Identifier
    return NULL;
  }
  node = new ASTNode(ASTNodeType::AssignmentStmt, token.text);
  tokens.pop_front();                // - =
  ASTNode *child = additive(tokens); // - additive
  if (!child)
    throw "invalide assignment statement, expecting an expression";
  node->addChild(child);
  return statementEnd(node, tokens); //直接返回子节点，简化了AST。
}

/**
 * 整形变量声明语句
 * intDeclaration :: 'int' Id ( '=' additiveExpression)? ';’
 * @param tokens
 */
ASTNode *intDeclare(list<Token> &tokens)
{
  NOT_EMPTY(tokens);
  ASTNode *node;
  Token token = tokens.front();
  MATCH_TOKEN(token, TokenType::Int);
  tokens.pop_front(); // - Int
  if (tokens.empty() || tokens.front().type != TokenType::Identifier)
    throw "variable name expected";

  token = tokens.front();
  tokens.pop_front(); // - Identifier
  node = new ASTNode(ASTNodeType::IntDeclaration, token.text);
  if (tokens.empty() || tokens.front().type != TokenType::Assignment)
    return statementEnd(node, tokens);
  tokens.pop_front();                // - =
  ASTNode *child = additive(tokens); // - additive
  if (!child)
    throw "invalide variable initialization, expecting an expression";
  node->addChild(child);
  return statementEnd(node, tokens); //直接返回子节点，简化了AST。
}

/**
 * 加减法表达式：+-属于加
 * additiveExpression :: multiplicative ( '+'|'-' multiplicative)*
 * @param tokens
 */
ASTNode *additive(list<Token> &tokens)
{
  NOT_EMPTY(tokens);
  ASTNode *child1 = multiplicative(tokens);
  ASTNode *node = child1;
  if (!child1 || tokens.empty())
    return node;

  while (!tokens.empty())
  {
    Token token = tokens.front();
    //应用()*规则
    if (token.type != TokenType::Plus && token.type != TokenType::Minus)
      break;
    tokens.pop_front(); // - '+'|'-'
    ASTNode *child2 = multiplicative(tokens);
    if (!child2)
      throw "invalid additive expression, expecting the right part.";
    node = new ASTNode(ASTNodeType::Additive, token.text);
    node->children.push_back(child1);
    node->children.push_back(child2);
  }
  return node;
}

/**
 * 乘除法表达式
 * multiplicative :: primary ( '*'|'/' primary)*
 * @param tokens
 */
ASTNode *multiplicative(list<Token> &tokens)
{
  ASTNode *child1 = primary(tokens);
  ASTNode *node = child1;
  if (!child1 || tokens.empty())
    return node;

  while (!tokens.empty())
  {
    Token token = tokens.front();
    //应用()*规则
    if (token.type != TokenType::Star && token.type != TokenType::Slash)
      break;
    tokens.pop_front(); // - '+'|'-'
    ASTNode *child2 = primary(tokens);
    if (!child2)
      throw "invalid additive expression, expecting the right part.";
    node = new ASTNode(ASTNodeType::Multiplicative, token.text);
    node->children.push_back(child1);
    node->children.push_back(child2);
  }
  return node;
}

/**
 * 基础表达式
 * primary :: IntLiteral | Identifier | '(' additiveExpression ')'
 * @param tokens
 */
ASTNode *primary(list<Token> &tokens)
{
  NOT_EMPTY(tokens);
  ASTNode *node;
  Token token = tokens.front();
  if (token.type == TokenType::IntLiteral)
  {
    tokens.pop_front(); // - IntLiteral
    node = new ASTNode(ASTNodeType::IntLiteral, token.text);
  }
  else if (token.type == TokenType::Identifier)
  {
    tokens.pop_front(); // - Identifier
    node = new ASTNode(ASTNodeType::Identifier, token.text);
  }
  else if (token.type == TokenType::LeftParen)
  {
    tokens.pop_front(); // - (
    node = additive(tokens);
    if (!node)
      throw "expecting an additive expression inside parenthesis";
    if (!tokens.empty() && tokens.front().type == TokenType::RightParen)
    {
      tokens.pop_front(); // - )
    }
    else
      throw "expecting right parenthesis";
  }
  return node;
}

/**
 * 语句结尾 消化;
 * @param node
 * @param tokens
 */
ASTNode *statementEnd(ASTNode *node, list<Token> &tokens)
{
  if (!tokens.empty() && tokens.front().type == TokenType::SemiColon)
  {
    tokens.pop_front();
  }
  return node;
}

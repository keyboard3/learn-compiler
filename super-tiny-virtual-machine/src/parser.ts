import * as lexer from "./lexer";
import { ASTNode, ASTNodeType, Token, TokenType } from "./models";

function peek<T>(array: T[]): T | null {
  if (!array.length) return null;
  return array[0];
}

export function parse(code: string) {
  const tokens = lexer.tokenize(code);
  const rootNode = prog(tokens);
  return rootNode;
}

/**
 * AST根节点
 * @param tokens
 */
function prog(tokens: Token[]): ASTNode {
  // console.log("===parser===")
  const node = new ASTNode(ASTNodeType.Program, "pwc");
  while (peek(tokens)) {
    let child = intDeclare(tokens)
      || assignmentStatement(tokens)
      || expressionStatement(tokens);
    if (child) node.addChild(child);
  }
  // console.log(node.dumpAST(""));
  return node;
}

/**
 * 表达式语句 expressionStatement :: additiveExpression ';’;
 */
function expressionStatement(tokens: Token[]): ASTNode | null {
  return statementEnd(additive(tokens), tokens);
}

/**
 * 赋值语句
 * assignment :: Identifier '=' additiveExpression;
 * @param tokens
 */
function assignmentStatement(tokens: Token[]): ASTNode | null {
  let node: ASTNode | null = null;
  let token = peek(tokens);
  if (token?.type != TokenType.Identifier) return null;
  token = tokens.shift(); // - Identifier
  if (peek(tokens)?.type != TokenType.Assignment) {
    tokens.unshift(token)//+ Identifier
    return null;
  }
  node = new ASTNode(ASTNodeType.AssignmentStmt, token.text);
  tokens.shift();// - =
  let child: ASTNode = additive(tokens); // - additive
  if (!child) throw Error("invalide assignment statement, expecting an expression");
  node.addChild(child);
  return statementEnd(node, tokens); //直接返回子节点，简化了AST。
}

/**
 * 整形变量声明语句
 * intDeclaration :: 'int' Id ( '=' additiveExpression)? ';’
 * @param tokens
 */
function intDeclare(tokens: Token[]): ASTNode | null {
  let node: ASTNode | null = null;
  let token = peek(tokens);
  if (token?.type != TokenType.Int) return null;
  tokens.shift(); // - Int
  if (peek(tokens)?.type != TokenType.Identifier) throw Error("variable name expected");
  token = tokens.shift(); // - Identifier
  node = new ASTNode(ASTNodeType.IntDeclaration, token.text);
  if (peek(tokens)?.type != TokenType.Assignment) return statementEnd(node, tokens);
  tokens.shift(); // - =
  let child: ASTNode = additive(tokens); // - additive
  if (!child) throw Error("invalide variable initialization, expecting an expression");
  node.addChild(child);
  return statementEnd(node, tokens); //直接返回子节点，简化了AST。
}

/**
 * 加减法表达式：+-属于加
 * additiveExpression :: multiplicative ( '+'|'-' multiplicative)*
 * @param tokens
 */
function additive(tokens: Token[]): ASTNode | null {
  let child1 = multiplicative(tokens);
  let node = child1;
  if (!child1) return null;
  let token = null;
  while (token = peek(tokens)) { //应用()*规则
    if (token.type != TokenType.Plus && token.type != TokenType.Minus) break;
    token = tokens.shift();// - '+'|'-'
    let child2 = multiplicative(tokens);
    if (!child2) throw Error("invalid additive expression, expecting the right part.");
    node = new ASTNode(ASTNodeType.Additive, token.text);
    node.children.push(child1, child2);
  }
  return node;
}

/**
 * 乘除法表达式
 * multiplicative :: primary ( '*'|'/' primary)*
 * @param tokens
 */
function multiplicative(tokens: Token[]): ASTNode | null {
  let child1 = primary(tokens);
  let node = child1;
  if (!child1) return null;
  let token = null;
  while (token = peek(tokens)) { //应用()*规则
    if (token.type != TokenType.Star && token.type != TokenType.Slash) break;
    token = tokens.shift();// - '+'|'-'
    let child2 = primary(tokens);
    if (!child2) throw Error("invalid additive expression, expecting the right part.");
    node = new ASTNode(ASTNodeType.Multiplicative, token.text);
    node.children.push(child1, child2);
  }
  return node;
}

/**
 * 基础表达式
 * primary :: IntLiteral | Identifier | '(' additiveExpression ')'
 * @param tokens
 */
function primary(tokens: Token[]): ASTNode | null {
  let node: ASTNode | null = null;
  const token = peek(tokens);
  if (!token) return null;
  if (token.type == TokenType.IntLiteral) {
    tokens.shift(); // - IntLiteral
    node = new ASTNode(ASTNodeType.IntLiteral, token.text);
  } else if (token.type == TokenType.Identifier) {
    tokens.shift(); // - Identifier
    node = new ASTNode(ASTNodeType.Identifier, token.text);
  } else if (token.type == TokenType.LeftParen) {
    tokens.shift(); // - (
    node = additive(tokens);
    if (!node) throw Error("expecting an additive expression inside parenthesis");
    if (peek(tokens)?.type == TokenType.RightParen) {
      tokens.shift(); // - )
    } else throw Error("expecting right parenthesis");
  }
  return node;
}

/**
 * 语句结尾 消化;
 * @param node
 * @param tokens
 */
function statementEnd(node: ASTNode, tokens: Token[]) {
  if (!node) return null;
  if (peek(tokens)?.type != TokenType.SemiColon) throw Error("invalid statement, expecting semicolon")
  tokens.shift();
  return node;
}

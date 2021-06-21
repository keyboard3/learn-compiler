import * as lexer from "./lexer";
import { ASTNode, ASTNodeType, Token, TokenType as TType, TokenStream } from "./models";

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
  const ts = new TokenStream(tokens);
  let node = new ASTNode(ASTNodeType.Program, null);
  let token;
  while (token = ts.peekToken()) {
    if (token.type == TType.EOF) break;
    let child = statement(ts);
    if (!child) continue;
    node.addChild(child)
  }
  return node;
}

function statement(ts: TokenStream) {
  let child = varDeclare(ts)
    || assignmentStatement(ts)
    || expressionStatement(ts)
    || blockStatement(ts)
    || returnStatement(ts)
    || functionDefinition(ts)
    || callExpression(ts);
  return statementEnd(child, ts);
}

/**
 *
 * 函数声明语句 functionDefinition :: function Identifier '(' Identifier (',' Identifier)* ')' blockStatement
 */
function functionDefinition(ts: TokenStream): ASTNode | null {
  let token = ts.getToken();
  if (notToken(token, TType.FUNCTION)) {
    ts.unGetToken(token);
    return null;
  }
  token = ts.getToken();
  if (token.type !== TType.NAME) throw Error("function must has name");
  let node = new ASTNode(ASTNodeType.Function, token);
  token = ts.getToken();
  if (token.type !== TType.LP) throw Error("function define error");
  //函数定义参数不做签名检查，不用保留参数
  while (token = ts.getToken()) {
    if (token.type === TType.RP) break;
    if (token.type != TType.COMMA) throw Error("function args define error");
    token = ts.getToken();
    if (token.type !== TType.NAME) throw Error("function args define error");
  }
  token = ts.peekToken();
  if (token?.type != TType.LC) throw Error("function missing body")
  let child = blockStatement(ts);
  node.addChild(child);
  return node;
}

/**
 * 函数调用声明 callExpression :: Identifier '(' primary (',',primary) ')'
 */
function callExpression(ts: TokenStream) {
  let token = ts.getToken();
  if (notToken(token, TType.NAME)) {
    ts.unGetToken(token);
    return null;
  }
  if (notToken(ts.peekToken(), TType.LP)) {
    ts.unGetToken(token);
    return null;
  }
  let node = new ASTNode(ASTNodeType.Call, token);
  ts.getToken();
  token = ts.peekToken();
  if (!notToken(token, TType.RP)) {
    ts.getToken();
    return node;
  }
  let child = primary(ts);
  if (child) node.addChild(child);
  while (notToken(ts.peekToken(), TType.RP)) {
    if (ts.getToken()?.type != TType.COMMA) throw Error("call args syntax error");
    child = primary(ts);
    node.addChild(child);
  }
  ts.getToken();
  return node;
}

/**
 * 块级语句 blackStatement :: '{' (statement)* '}'
 */
function blockStatement(ts: TokenStream): ASTNode | null {
  let token = ts.getToken();
  if (notToken(token, TType.LC)) {
    ts.unGetToken(token);
    return null;
  }
  let node = new ASTNode(ASTNodeType.Block, null);
  do {
    let child = statement(ts);
    if (!child) break;
    node.addChild(child);
    token = ts.peekToken();
  } while (token && token.type !== TType.RC)
  ts.getToken();
  return node;
}

/**
 * 表达式语句 returnStatement :: return expressionStatement?
 */
function returnStatement(ts: TokenStream): ASTNode | null {
  let token = ts.getToken();
  if (notToken(token, TType.RETURN)) {
    ts.unGetToken(token);
    return null;
  }
  let node = new ASTNode(ASTNodeType.Return, null);
  const child = expressionStatement(ts);
  if (child) node.addChild(child);
  return node;
}

/**
 * 表达式语句 expressionStatement :: additiveExpression | callExpression
 */
function expressionStatement(ts: TokenStream): ASTNode | null {
  return callExpression(ts) || unaryExpression(ts) || binaryExpression(ts);
}

/**
 * 一元前缀表达式 unaryExpression :: [!,~] primary
 */
function unaryExpression(ts: TokenStream) {
  const token = ts.getToken();

  if (notToken(token, TType.UNARYOP)) {
    ts.unGetToken(token);
    return null;
  }
  let node = new ASTNode(ASTNodeType.Unary, token);
  let child = primary(ts);
  node.addChild(child);
  return node;
}

/**
 * 二元表达式 BinaryExpression :: [<<,>>,==,!=,<,<=,>,>=,additive]
 */
function binaryExpression(ts: TokenStream) {
  let child = additive(ts);
  if (!child) return null;
  if (child.type == ASTNodeType.Binary) return child;
  //证明当前node是number/string
  let token = ts.peekToken();
  switch (token?.type) {
    case TType.SHOP://<< >>
    case TType.RELOP://< > <= >=
    case TType.EQOP://== !=
    case TType.OR:// ||
    case TType.AND:// &&
    case TType.BITOR:// |
    case TType.BITXOR:// ^
    case TType.BITAND:// &
      {
        ts.getToken();
        let child2 = additive(ts);
        let node = new ASTNode(ASTNodeType.Binary, token);
        node.addChild(child);
        node.addChild(child2);
        return node;
      }
    default:
      return child;
  }
}

/**
 * 赋值语句
 * assignment :: Identifier '=' expressionStatement;
 * @param tokens
 */
function assignmentStatement(ts: TokenStream): ASTNode | null {
  let node: ASTNode | null = null;
  let token = ts.getToken();
  if (notToken(token, TType.NAME)) {
    ts.unGetToken(token);
    return null;
  }
  node = new ASTNode(ASTNodeType.AssignmentStmt, token);
  token = ts.getToken();
  if (notToken(token, TType.ASSIGN)) {
    ts.unGetToken(token);//+ =
    ts.unGetToken(token);//+ Identifier
    return null;
  }
  let child: ASTNode = expressionStatement(ts); // - additive
  if (!child) throw Error("invalidate assignment statement, expecting an expression");
  node.addChild(child);
  return node;
}

/**
 * var变量声明语句
 * varDeclaration :: 'var' Id ( '=' expressionStatement)?
 * @param tokens
 */
function varDeclare(ts: TokenStream): ASTNode | null {
  let node: ASTNode | null = null;
  let token = ts.getToken();
  if (notToken(token, TType.VAR)) {
    ts.unGetToken(token);
    return null;
  }
  token = ts.getToken();
  if (notToken(token, TType.NAME)) throw Error("variable name expected");
  node = new ASTNode(ASTNodeType.VarDeclaration, token);
  token = ts.getToken();
  if (notToken(token, TType.ASSIGN)) {
    ts.unGetToken(token);
    return node;
  }
  let child: ASTNode = expressionStatement(ts); // - additive
  if (!child) throw Error("invalide variable initialization, expecting an expression");
  node.addChild(child);
  return node;
}

/**
 * 加减法表达式：+-属于加
 * additiveExpression :: multiplicative ( '+'|'-' multiplicative)*
 * @param tokens
 */
function additive(ts: TokenStream): ASTNode | null {
  let child1 = multiplicative(ts);
  if (!child1) return null;

  let node = child1;
  let token = null;
  while (token = ts.getToken()) { //应用()*规则
    if (token.type != TType.PLUS && token.type != TType.MINUS) break;
    let child2 = multiplicative(ts);
    if (!child2) throw Error("invalid additive expression, expecting the right part.");
    node = new ASTNode(ASTNodeType.Binary, token);
    node.addChild(child1);
    node.addChild(child2)
    child1 = node;
  }
  ts.unGetToken(token);
  return node;
}

/**
 * 乘除法表达式
 * multiplicative :: primary ( '*'|'/' primary)*
 * @param tokens
 */
function multiplicative(ts: TokenStream): ASTNode | null {
  let child1 = primary(ts);
  let node = child1;
  if (!child1) return null;
  let token = null;
  while (token = ts.getToken()) { //应用()*规则
    if (notToken(token, TType.MULOP)) break;
    let child2 = primary(ts);
    if (!child2) throw Error("invalid additive expression, expecting the right part.");
    node = new ASTNode(ASTNodeType.Binary, token);
    node.addChild(child1);
    node.addChild(child2)
    child1 = node;
  }
  ts.unGetToken(token);
  return node;
}

/**
 * 基础表达式
 * primary :: NumberLiteral | Identifier | '(' expressionStatement ')'
 * @param tokens
 */
function primary(ts: TokenStream): ASTNode | null {
  let node: ASTNode | null = null;
  const token = ts.getToken();
  if (!token) return null;
  if (token.type == TType.PRIMARY) {
    node = new ASTNode(ASTNodeType.Primary, token);
  } else if (token.type == TType.NUMBER) {
    node = new ASTNode(ASTNodeType.NumberLiteral, token);
  } else if (token.type == TType.NAME) {
    node = new ASTNode(ASTNodeType.Identifier, token);
  } else if (token.type == TType.LP) {
    node = expressionStatement(ts);
    if (!node) throw Error("expecting an additive expression inside parenthesis");
    if (ts.getToken()?.type == TType.RP) {
    } else throw Error("expecting right parenthesis");
  } else {
    ts.unGetToken(token);
  }
  return node;
}

/**
 * 语句结尾 消化;
 * @param node
 * @param tokens
 */
function statementEnd(node: ASTNode, ts: TokenStream) {
  let token = ts.peekToken();
  if (token.type == TType.SEMI) {
    ts.getToken();
  }
  return node;
}

function notToken(token: Token | null, ...types: TType[]) {
  if (!token) return true;
  for (let type of types) {
    if (type == token.type) return false;
  }
  return true;
}

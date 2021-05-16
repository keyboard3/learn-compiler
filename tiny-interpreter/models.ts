import { initGlobalScope } from "./api";


export enum K3_TokenType {
  EOF,//文件末尾
  EOL, //行尾
  SEMI,//语句末尾 ;s
  LB, RB,//数组标识 []
  LC, RC,//块级标识 {}
  LP, RP,//函数声明和调用标识 ()
  COMMA,//数组或者多个声明标识 ,
  ASSIGN,//赋值符号 =,+=,-=
  HOOK,//条件判断符号 ?
  COLON,//三目符号中的:
  OR, AND,//逻辑符号||和&&
  BITOR, BITXOR, BITAND,//二进制运算|^&
  EQOP,//等于符号 == !=
  RELOP,//比较符号 < <= > >=
  SHOP,//移位符号 << >> >>>
  PLUS,//加法 +
  MINUS, //减法 -
  MULOP,//乘除符号 * / %
  UNARYOP,//一元前缀符号 !=的! 和~
  INCOP,//自增和自减符号 ++ --
  DOT, //成员访问符号 .
  NAME,//标识符
  NUMBER,//数字常量
  STRING,//字符串常量
  PRIMARY,//true,false,null,this,super
  FUNCTION,//函数关键字 函数定义用
  IF,//if关键字 bool判断true用
  ELSE,//else关键字 bool判断false用
  SWITCH,//switch关键字 匹配值用
  CASE,//case关键字 每种case对应处理用
  DEFAULT,//default关键字 没有匹配时用
  WHILE,//while关键字 while循环判断用
  DO,//do关键字 do-while循环入口
  FOR,//for关键字 for循环入口
  BREAK,//break关键字 循环调出
  CONTINUE,//continue关键字 跳过这次，继续循环下一个
  IN,//in关键字 for循环取出对象属性用
  VAR,//var关键字 变量声明
  WITH,//with关键字 with作用域
  RETURN,//return关键字 函数返回用
  NEW,//new关键字 创建对象
  RESERVED, //保留关键字
}

export const defineKeywords = {
  "break": K3_TokenType.BREAK,
  "case": K3_TokenType.CASE,
  "continue": K3_TokenType.CONTINUE,
  "default": K3_TokenType.DEFAULT,
  "delete": K3_TokenType.UNARYOP,
  "do": K3_TokenType.DO,
  "else": K3_TokenType.ELSE,
  "false": K3_TokenType.PRIMARY,
  "for": K3_TokenType.FOR,
  "function": K3_TokenType.FUNCTION,
  "if": K3_TokenType.IF,
  "in": K3_TokenType.IN,
  "new": K3_TokenType.NEW,
  "null": K3_TokenType.PRIMARY,
  "return": K3_TokenType.RETURN,
  "switch": K3_TokenType.SWITCH,
  "this": K3_TokenType.PRIMARY,
  "true": K3_TokenType.PRIMARY,
  "typeof": K3_TokenType.UNARYOP,
  "var": K3_TokenType.VAR,
  "void": K3_TokenType.UNARYOP,
  "while": K3_TokenType.WHILE,
  "with": K3_TokenType.WITH,
  //RESERVE_JAVA_KEYWORDS
  "abstract": K3_TokenType.RESERVED,
  "boolean": K3_TokenType.RESERVED,
  "byte": K3_TokenType.RESERVED,
  "catch": K3_TokenType.RESERVED,
  "char": K3_TokenType.RESERVED,
  "class": K3_TokenType.RESERVED,
  "const": K3_TokenType.RESERVED,
  "double": K3_TokenType.RESERVED,
  "extends": K3_TokenType.RESERVED,
  "final": K3_TokenType.RESERVED,
  "finally": K3_TokenType.RESERVED,
  "float": K3_TokenType.RESERVED,
  "goto": K3_TokenType.RESERVED,
  "implements": K3_TokenType.RESERVED,
  "import": K3_TokenType.RESERVED,
  "instanceof": K3_TokenType.RESERVED,
  "int": K3_TokenType.RESERVED,
  "interface": K3_TokenType.RESERVED,
  "long": K3_TokenType.RESERVED,
  "native": K3_TokenType.RESERVED,
  "package": K3_TokenType.RESERVED,
  "private": K3_TokenType.RESERVED,
  "protected": K3_TokenType.RESERVED,
  "public": K3_TokenType.RESERVED,
  "short": K3_TokenType.RESERVED,
  "static": K3_TokenType.RESERVED,
  "super": K3_TokenType.PRIMARY,
  "synchronized": K3_TokenType.RESERVED,
  "throw": K3_TokenType.RESERVED,
  "throws": K3_TokenType.RESERVED,
  "transient": K3_TokenType.RESERVED,
  "try": K3_TokenType.RESERVED,
  "volatile": K3_TokenType.RESERVED,
};

export class K3_Token {
  type: K3_TokenType;
  text: string
}

export enum K3_ASTNode_Type {
  Program = "Program",
  AssignmentStmt = "AssignmentStmt",
  VarDeclaration = "VarDeclaration",
  NumberLiteral = "NumberLiteral",
  Identifier = "Identifier",
  Function = "Function",
  For = "For",
  DoWhile = "DoWhile",
  While = "While",
  Block = "Block",
  Return = "Return",
  Binary = "Binary",
  Unary = "Unary",
  Primary = "Primary",
  Call = "Call"
}

export class K3_ASTNode {
  type: K3_ASTNode_Type;
  token: K3_Token;
  prefix: boolean;//主要是为了区分自增符号的
  args: K3_Symbol[];//函数声明这个参数就是形参
  children: Array<K3_ASTNode>;
  parent: K3_ASTNode;

  constructor(type: K3_ASTNode_Type, token: K3_Token, prefix = false) {
    this.type = type;
    this.token = token;
    this.children = [];
    this.args = [];
    this.prefix = prefix;
  }

  addChild(node: K3_ASTNode) {
    this.children.push(node);
    node.parent = this;
  }

  /**
   * 打印输出AST树状结构
   * @param node
   * @param indent 缩进字符，由tab组成，每一级多一个tab
   */
  dumpAST(indent: string) {
    console.log(`${indent}${this.type} ${this.token?.text}`);
    for (let child of this.children) {
      child.dumpAST(`${indent}\t`);
    }
  }
}

export class K3_Context {
  constructor(script: K3_ASTNode, maxFrameSize: number) {
    this.script = this.ptr = script;
    const globalScope = new K3_Scope(null);
    initGlobalScope(globalScope);
    this.globalScope = globalScope;
    this.stack = new K3_Stack(maxFrameSize);
  }

  globalScope: K3_Scope;
  stack: K3_Stack;
  script: K3_ASTNode;
  ptr: K3_ASTNode;
}

export class K3_Stack {
  maxFrameSize: number;
  frames: K3_StackFrame[];
  ptr: number;

  constructor(maxFrameSize: number) {
    this.maxFrameSize = maxFrameSize;
    this.frames = [];
    this.ptr = -1;
  }

  pushFrame(frame: K3_StackFrame) {
    if (this.frames.length + 1 >= this.maxFrameSize) throw Error("stack overflow")
    this.frames.push(frame);
    this.ptr++;
  }

  popFrame() {
    if (this.frames.length - 1 < 0) return;
    this.frames.pop();
    this.ptr--;
  }
}

export interface K3_StackFrame {
  scope: K3_Scope;
}

export class K3_Scope {
  constructor(parent: K3_Scope) {
    this.parent = parent;
    this.declareSymbols = new Map();
    this.declareFunctions = new Map();
  }

  parent: K3_Scope;
  declareSymbols: Map<string, K3_Symbol>;
  declareFunctions: Map<string, K3_Function>;
  callResult: any;
}

export interface K3_Symbol {
  value: any;
  type: K3_SymbolType;
}

export interface K3_Function {
  call?: (...any) => any;
  script?: K3_ASTNode;
  parentScope: K3_Scope;
  args?: K3_Symbol[]
}

export enum K3_SymbolType {
  SYM_UNDEF,
  SYM_ARGUMENT,
  SYM_VARIABLE,
  SYM_PROPERTY
}

export const K3_EOF = null;

export class CharStream {
  token: K3_Token;//最后一个匹配的token
  tokenBuf: string;
  code: string;
  ptr: number;

  constructor(code: string) {
    this.code = code;
    this.ptr = 0;
    this.tokenBuf = "";
    this.token = new K3_Token();
  }

  getChar() {
    if (this.ptr >= this.code.length) return K3_EOF;
    const char = this.code[this.ptr];
    this.ptr++;
    return char;
  }

  peekChar() {
    const char = this.getChar();
    this.unGetChar(char);
    return char;
  }

  unGetChar(char: string) {
    //非空白字符，不回退。只回退有效字符
    if (char == K3_EOF) return;
    if (/\s/.test(char)) return;

    if (this.ptr - char.length <= 0) return;
    this.ptr = this.ptr - char.length;
  }

  matchChar(c: string) {
    const char = this.getChar();
    if (char === c) return true;
    this.unGetChar(c);
    return false;
  }

  appendToTokenBuf(char: string) {
    this.tokenBuf += char;
    return this.getChar();
  }
}

export class TokenStream {
  node: K3_ASTNode;
  tokens: K3_Token[];
  ptr: number;

  constructor(tokens: K3_Token[]) {
    this.tokens = tokens;
    this.ptr = 0;
  }

  getToken(): K3_Token | null {
    if (this.ptr > this.tokens.length) return K3_EOF;
    const token = this.tokens[this.ptr];
    this.ptr++;
    return token;
  }

  unGetToken(token: K3_Token) {
    if (token == null) return;
    if (this.ptr - 1 < 0) return;
    this.ptr--;
  }

  peekToken(): K3_Token | null {
    const token = this.getToken();
    if (!token) return null;
    this.unGetToken(token);
    return token;
  }
}

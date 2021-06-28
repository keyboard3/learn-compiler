export enum TokenType {
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
  "break": TokenType.BREAK,
  "case": TokenType.CASE,
  "continue": TokenType.CONTINUE,
  "default": TokenType.DEFAULT,
  "delete": TokenType.UNARYOP,
  "do": TokenType.DO,
  "else": TokenType.ELSE,
  "false": TokenType.PRIMARY,
  "for": TokenType.FOR,
  "function": TokenType.FUNCTION,
  "if": TokenType.IF,
  "in": TokenType.IN,
  "new": TokenType.NEW,
  "null": TokenType.PRIMARY,
  "return": TokenType.RETURN,
  "switch": TokenType.SWITCH,
  "this": TokenType.PRIMARY,
  "true": TokenType.PRIMARY,
  "typeof": TokenType.UNARYOP,
  "var": TokenType.VAR,
  "void": TokenType.UNARYOP,
  "while": TokenType.WHILE,
  "with": TokenType.WITH,
  //RESERVE_JAVA_KEYWORDS
  "abstract": TokenType.RESERVED,
  "boolean": TokenType.RESERVED,
  "byte": TokenType.RESERVED,
  "catch": TokenType.RESERVED,
  "char": TokenType.RESERVED,
  "class": TokenType.RESERVED,
  "const": TokenType.RESERVED,
  "double": TokenType.RESERVED,
  "extends": TokenType.RESERVED,
  "final": TokenType.RESERVED,
  "finally": TokenType.RESERVED,
  "float": TokenType.RESERVED,
  "goto": TokenType.RESERVED,
  "implements": TokenType.RESERVED,
  "import": TokenType.RESERVED,
  "instanceof": TokenType.RESERVED,
  "int": TokenType.RESERVED,
  "interface": TokenType.RESERVED,
  "long": TokenType.RESERVED,
  "native": TokenType.RESERVED,
  "package": TokenType.RESERVED,
  "private": TokenType.RESERVED,
  "protected": TokenType.RESERVED,
  "public": TokenType.RESERVED,
  "short": TokenType.RESERVED,
  "static": TokenType.RESERVED,
  "super": TokenType.PRIMARY,
  "synchronized": TokenType.RESERVED,
  "throw": TokenType.RESERVED,
  "throws": TokenType.RESERVED,
  "transient": TokenType.RESERVED,
  "try": TokenType.RESERVED,
  "volatile": TokenType.RESERVED,
};

export class Token {
  type: TokenType;
  text: string
}

export enum ASTNodeType {
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

export class ASTNode {
  type: ASTNodeType;
  token: Token;
  children: Array<ASTNode>;

  constructor(type: ASTNodeType, token: Token) {
    this.type = type;
    this.token = token;
    this.children = [];
  }

  addChild(node: ASTNode) {
    this.children.push(node);
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

export class Context {
  constructor(maxFrameSize: number) {
    this.globalScope = {};
    this.maxFrameSize = maxFrameSize;
    this.frames = [];
  }

  globalScope: { [key: string]: any };
  maxFrameSize: number;
  frames: StackFrame[];
  stack: StackFrame;//当前栈帧

  pushFrame(frame: StackFrame) {
    if (this.frames.length + 1 >= this.maxFrameSize) throw Error("stack overflow")
    this.frames.push(frame);
    this.stack = frame;
  }

  popFrame() {
    if (this.frames.length - 1 < 0) return;
    this.stack = this.frames.pop();
  }

  get(name: string) {
    if (this.stack) return this.stack.get(name);
    if (this.globalScope[name]) return this.globalScope[name];
    throw `${name} is not defined`;
  }
  set(name: string, value: any) {
    if (this.stack) return this.stack.set(name, value);
    this.globalScope[name] = value;
  }
  declare(name: string, value: any) {
    if (this.stack) return this.stack.declare(name, value);
    this.globalScope[name] = value;
  }
}

export class StackFrame {
  staticStackFrame: StackFrame | null;//静态作用域的栈帧
  localScope: { [key: string]: any };
  globalScope: Object;
  constructor(staticStackFrame: StackFrame | null, globalScope: Object) {
    this.staticStackFrame = staticStackFrame;
    this.localScope = new Map();
    this.globalScope = globalScope;
  }
  get(name: string) {
    if (this.localScope[name]) return this.localScope[name];
    else if (this.staticStackFrame) return this.staticStackFrame.get(name);
    else if (this.globalScope[name]) return this.globalScope[name];
    throw `${name} is not defined`;
  }
  declare(name: string, value: any) {
    this.localScope[name] = value;
  }
  set(name: string, value: any) {
    if (this.localScope[name]) this.localScope[name] = value;
    else if (this.staticStackFrame) this.staticStackFrame.set(name, value);
    else this.globalScope[name] = value;//未找到直接丢到全局变量
  }
}
export const EOF = null;

export class CharStream {
  token: Token;//最后一个匹配的token
  tokenBuf: string;
  code: string;
  ptr: number;

  constructor(code: string) {
    this.code = code;
    this.ptr = 0;
    this.tokenBuf = "";
    this.token = new Token();
  }

  getChar() {
    if (this.ptr >= this.code.length) return EOF;
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
    if (char == EOF) return;
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
  tokens: Token[];
  ptr: number;

  constructor(tokens: Token[]) {
    this.tokens = tokens;
    this.ptr = 0;
  }

  getToken(): Token | null {
    if (this.ptr > this.tokens.length) return EOF;
    const token = this.tokens[this.ptr];
    this.ptr++;
    return token;
  }

  unGetToken(token: Token) {
    if (token == null) return;
    if (this.ptr - 1 < 0) return;
    this.ptr--;
  }

  peekToken(): Token | null {
    const token = this.getToken();
    if (!token) return null;
    this.unGetToken(token);
    return token;
  }
}

export const EOF = null;
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
  args: Token[];
  constructor(type: ASTNodeType, token: Token) {
    this.type = type;
    this.token = token;
    this.children = [];
    this.args = [];
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

//全局上下文
export class _Context {
  constructor() {
    this.stack = new _Stack();
    this.staticLink = this.globalObject = new _Scope();
  }
  staticLink: _Scope;//静态作用域
  globalObject: _Scope;//顶级作用域
  stack: _Stack;//执行的全局操作数存储区
  script: _Script;
}
export class _Stack {
  base: _Datum[] = [];
  frame: _Frame;//当前函数执行栈帧
  _ptr: number = 0;
  get ptr() {
    return this.base[this._ptr];
  }
  push(data: _Datum) {
    this.base.push(data);
    this._ptr++;
  }
  pop() {
    this._ptr--;
    return this.base.pop();
  }
}
//函数执行的上下文
export class _Frame {
  _stack: _Stack;
  fun: _Function;
  constructor(stack: _Stack) {
    this._stack = stack;
    this.argc = 0;
    this.nvars = 0;
    this._argv = this._stack._ptr;
    this._vars = this._stack._ptr;
  }
  argc: number;//函数参数数量
  nvars: number;//本地变量数量
  _argv: number;//stack的参数索引
  _vars: number;//stack的参数索引
  get argv() {
    return this._stack.base.slice(this._argv, this._argv + this.argc);
  };
  get vars() {
    return this._stack.base.slice(this._vars, this._vars + this.nvars);
  }
  down: _Frame;//上一个执行栈帧
}
//表示字面量
export enum ATOM_TYPE {
  NUMBER,
  STRING,
  NAME
}
export class _Atom {
  constructor(flag: ATOM_TYPE, val: number | string) {
    this.val = val;
    this.flag = flag;
  }
  val: number | string;
  flag: ATOM_TYPE;
}
export class _Script {
  constructor() {
    this.code = new ArrayBuffer(1000);
    this.atoms = [];
    this.args = [];
  }
  args: _Symbol[];//参数
  code: ArrayBuffer;//二进制指令
  atoms: _Atom[];//字面量数组
}
export class _Function {
  constructor(name: _Atom, script: _Script, parent: _Scope) {
    this.name = name;
    this.script = script;
    this.scope = parent;
  }
  scope: _Scope;
  name: _Atom;
  script: _Script;
  call?: (...args) => void;//原生方法
}
export class _Property {
  datum: _Datum;
  constructor(datum: _Datum) {
    this.datum = datum;
  }
}
export enum SYMOBL_TYPE {
  ARGUMENT,//栈中
  VARIABLE,//栈中
  PROPERTY,//symbol的值
}
type _SymbolEntry = {
  key: _Atom;
  value?: _Datum | _Property;
}
export class _Symbol {
  constructor(scope: _Scope, type: SYMOBL_TYPE, entry: _SymbolEntry) {
    this.scope = scope;
    this.type = type;
    this.entry = entry;
  }
  scope: _Scope;//通过scope确定栈帧，在通过当前栈帧和sym的栈帧和slot确定stack中的datum
  entry: _SymbolEntry;
  slot: number;//符号在栈中的索引
  type: SYMOBL_TYPE;//符号的类型 参数/变量
}
//存储入栈数据源 method,virable
export enum DATUM_TYPE {
  FUNCTION,//如果是方法则从获得函数信息
  SYMBOL,//已解析的符号，在stack中有引用
  ATOM,//基础元数据(字面量)类型，从atom中获取
  STRING,
  NUMBER,
  OBJECT,
  BOOL,
  UNDEF,
}
type DATUM_VALUE = _Atom | _Function | _Symbol | number | boolean | string | _Scope;
export class _Datum {
  constructor(flag: DATUM_TYPE, value?: DATUM_VALUE) {
    this.flag = flag;
    if (flag == DATUM_TYPE.FUNCTION) this.fun = value as _Function;
    if (flag == DATUM_TYPE.ATOM) this.atom = value as _Atom;
    if (flag == DATUM_TYPE.SYMBOL) this.symbol = value as _Symbol;
    if (flag == DATUM_TYPE.STRING) this.sval = value as string;
    if (flag == DATUM_TYPE.NUMBER) this.nval = value as number;
    if (flag == DATUM_TYPE.BOOL) this.bval = value as boolean;
    if (flag == DATUM_TYPE.OBJECT) this.object = value as _Scope;
  }
  atom: _Atom;
  object: _Scope;
  fun: _Function;
  symbol: _Symbol;
  nval: number;
  bval: boolean;
  sval: string;
  flag: DATUM_TYPE;
}
export class _Scope {
  constructor(parent?: _Scope) {
    this.parent = parent;
  }
  list: _Symbol[] = [];//所有符号
  parent: _Scope | null;
}

export enum OP_TYPE {
  NUMBER,
  NAME,
  STRING,
  ADD,
  MINUS,
  MUTIL,
  DIVID,
  ASSIGN,
  RETURN,
  THIS,
  CALL,
}
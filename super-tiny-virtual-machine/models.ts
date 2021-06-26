export enum DfaState {
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
}

export enum TokenType {
  Int = "Int",
  Identifier = "Identifier",
  IntLiteral = "IntLiteral",
  GT = "GT",
  GE = "GE",
  Assignment = "Assignment",
  Plus = "Plus",
  Minus = "Minus",
  Star = "*",
  Slash = "/",
  Equals = "Equals",
  LeftParen = "LeftParen",
  RightParen = "RightParen",
  SemiColon = "SemiColon"
}

export class Token {
  type: TokenType;
  text: string
}

export enum ASTNodeType {
  Program = "Program",
  AssignmentStmt = "AssignmentStmt",
  IntDeclaration = "IntDeclaration",
  Additive = "Additive",
  Multiplicative = "Multiplicative",
  IntLiteral = "IntLiteral",
  Identifier = "Identifier"
}

export class ASTNode {
  type: ASTNodeType;
  text: string;
  children: Array<ASTNode>;

  constructor(type: ASTNodeType, text: string) {
    this.type = type;
    this.text = text;
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
    console.log(`${indent}${this.type} ${this.text}`);
    for (let child of this.children) {
      child.dumpAST(`${indent}\t`);
    }
  }
}

export function str2Uint8Array(input: string): Uint8Array {
  const encoder = new TextEncoder()
  const view = encoder.encode(input)
  return view
}
export function ab2str(
  input: ArrayBuffer | Uint8Array | Int8Array | Uint16Array | Int16Array | Uint32Array | Int32Array,
  outputEncoding = 'utf8',
): string {
  const decoder = new TextDecoder(outputEncoding)
  return decoder.decode(input)
}

//指令类型
export enum OpType {
  INT_DECL,//压入作用域中undefined
  NUMBER,//数字压入栈中
  NAME,//取变量的值压入栈中
  ASSIGN,//将当前操作数存入作用域中
  ADD,//弹出栈中2个操作数计算
  MINUS,
  MUTI,
  DIVID
}
export function to_optype_str(type) {
  if (type == OpType.INT_DECL) return "intDecl";
  if (type == OpType.NAME) return "pushVaribal";
  if (type == OpType.NUMBER) return "pushNum";
  if (type == OpType.ASSIGN) return "assign";
  if (type == OpType.ADD) return "add";
  if (type == OpType.MINUS) return "minus";
  if (type == OpType.MUTI) return "muti";
  if (type == OpType.DIVID) return "divid";
}
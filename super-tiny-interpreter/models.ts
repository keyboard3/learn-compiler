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
  parent: ASTNode;

  constructor(type: ASTNodeType, text: string) {
    this.type = type;
    this.text = text;
    this.children = [];
  }

  addChild(node: ASTNode) {
    this.children.push(node);
    node.parent = this;
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

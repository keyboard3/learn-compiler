/**
 * 参考自estree es5定义
 * https://github.com/estree/estree/blob/master/es5.md
 */
export interface Node {
  type: string;
  // loc: SourceLocation | null;
  //acorn 适配
  start: number;
  end: number;
}
export interface SourceLocation {
  source: string | null;
  start: Position;
  end: Position;
}
export interface Position {
  line: number;//>=1
  column: number;//>=0
}
export interface Identifier extends Expression {
  type: "Identifier";
  name: string;
}
export interface Literal {
  type: "Literal";
  value: string | boolean | null | number | RegExp;
}
export interface RegExpLiteral extends Literal {
  regex: {
    pattern: string;
    flags: string;
  }
}
export interface Program extends Node {
  type: "Program";
  body: (Directive | Statement)[];
}
export interface Function extends Node {
  id: Identifier | null;
  params: Identifier[];
  body: FunctionBody;
}
export interface Statement extends Node { }
export interface ExpressionStatement extends Statement {
  type: "ExpressionStatement";
  expression: Expression;
}
export interface Directive extends Node {
  type: "ExpressionStatement";
  expression: Literal;
  directive: string;
}
export interface BlockStatement extends Statement {
  type: "BlockStatement";
  body: Statement[];
}
export interface FunctionBody extends BlockStatement {
  body: (Directive | Statement)[];
}
export interface EmptyStatement extends Statement {
  type: "EmptyStatement";
}
export interface DebuggerStatement extends Statement {
  type: "DebuggerStatement";
}
export interface WithStatement extends Statement {
  type: "WithStatement";
  object: Expression;
  body: Statement;
}
export interface ReturnStatement extends Statement {
  type: "ReturnStatement";
  argument: Expression | null;
}
export interface LabeledStatement extends Statement {
  type: "LabeledStatement";
  label: Identifier;
  body: Statement;
}
export interface BreakStatement extends Statement {
  type: "BreakStatement";
  label: Identifier | null;
}
export interface ContinueStatement extends Statement {
  type: "ContinueStatement";
  label: Identifier | null;
}
export interface IfStatement extends Statement {
  type: "IfStatement";
  test: Expression;
  consequent: Statement;
  alternate: Statement | null;
}
export interface SwitchStatement extends Statement {
  type: "SwitchStatement";
  discriminant: Expression;
  cases: SwitchCase[];
}
export interface SwitchCase extends Node {
  type: "SwitchCase";
  test: Expression | null;
  consequent: Statement[];
}
export interface ThrowStatement extends Statement {
  type: "ThrowStatement";
  argument: Expression;
}
export interface TryStatement extends Statement {
  type: "TryStatement";
  block: BlockStatement;
  handler: CatchClause | null;
  finalizer: BlockStatement | null;
}
export interface CatchClause extends Node {
  type: "CatchClause";
  param: Identifier;
  body: BlockStatement;
}
export interface WhileStatement extends Statement {
  type: "WhileStatement";
  test: Expression;
  body: Statement;
}
export interface DoWhileStatement extends Statement {
  type: "DoWhileStatement";
  body: Statement;
  test: Expression;
}
export interface ForStatement extends Statement {
  type: "ForStatement";
  init: VariableDeclaration | Expression | null;
  test: Expression | null;
  update: Expression | null;
  body: Statement;
}
export interface ForInStatement extends Statement {
  type: "ForInStatement";
  left: VariableDeclaration | Identifier;
  right: Expression;
  body: Statement;
}
export interface Declaration extends Statement { }
export interface FunctionDeclaration extends Function, Declaration {
  type: "FunctionDeclaration";
  id: Identifier;
}
export interface VariableDeclaration extends Declaration {
  type: "VariableDeclaration";
  declarations: VariableDeclarator[];
  kind: "var";
}
export interface VariableDeclarator extends Node {
  type: "VariableDeclarator";
  id: Identifier;
  init: Expression | null;
}
export interface Expression extends Node { }
export interface ThisExpression extends Expression {
  type: "ThisExpression";
}
export interface ArrayExpression extends Expression {
  type: "ArrayExpression";
  elements: (Expression | null)[];
}
export interface ObjectExpression extends Expression {
  type: "ObjectExpression";
  properties: Property[];
}
export interface Property extends Node {
  type: "Property";
  key: Literal | Identifier;
  value: Expression;
  kind: "init" | "get" | "set";
}
export interface FunctionExpression extends Function, Expression {
  type: "FunctionExpression";
}
export interface UnaryExpression extends Expression {
  type: "UnaryExpression";
  operator: UnaryOperator;
  prefix: boolean;
  argument: Expression;
}
type UnaryOperator = "-" | "+" | "!" | "~" | "typeof" | "void" | "delete";
export interface UpdateExpression extends Expression {
  type: "UpdateExpression";
  operator: UpdateOperator;
  argument: Identifier;
  prefix: boolean;
}
type UpdateOperator = "++" | "--";
export interface BinaryExpression extends Expression {
  type: "BinaryExpression";
  operator: BinaryOperator;
  left: Expression;
  right: Expression;
}
type BinaryOperator =
  "==" | "!=" | "===" | "!=="
  | "<" | "<=" | ">" | ">="
  | "<<" | ">>" | ">>>"
  | "+" | "-" | "*" | "/" | "%"
  | "|" | "^" | "&" | "in"
  | "instanceOf";
export interface AssignmentExpression extends Expression {
  type: "AssignmentExpression";
  operator: AssignmentOperator;
  left: Identifier | Expression;
  right: Expression;
}
type AssignmentOperator =
  "=" | "+=" | "-=" | "*=" | "/=" | "%="
  | "<<=" | ">>=" | ">>>="
  | "|=" | "^=" | "&=";
export interface LogicalExpression extends Expression {
  type: "LogicalExpression";
  operator: LogicalOperator;
  left: Expression;
  right: Expression;
}
type LogicalOperator = "||" | "&&";
export interface MemberExpression extends Expression {
  type: "MemberExpression";
  object: Expression;
  property: Identifier;
  computed: boolean;
}
export interface ConditionalExpression extends Expression {
  type: "ConditionalExpression";
  test: Expression;
  alternate: Expression;
  consequent: Expression;
}
export interface CallExpression extends Expression {
  type: "CallExpression";
  callee: Expression;
  arguments: Expression[];
}
export interface NewExpression extends Expression {
  type: "NewExpression";
  callee: Expression;
  arguments: Expression[];
}
export interface SequenceExpression extends Expression {
  type: "SequenceExpression";
  expressions: Expression[];
}
// export interface Pattern extends Node { }
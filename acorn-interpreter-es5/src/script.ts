import * as acorn from "acorn";
import { Context, StackFrame } from './context';
import { ExpressionStatement, Identifier, Literal, Program, VariableDeclaration, Node, CallExpression, FunctionDeclaration, MemberExpression, FunctionExpression, NewExpression, UpdateExpression, ThisExpression, BlockStatement, VariableDeclarator, ReturnStatement, ArrayExpression, ObjectExpression, AssignmentExpression, UnaryExpression, BinaryExpression, LogicalExpression, ForStatement, ForInStatement, WhileStatement, DoWhileStatement, BreakStatement, ContinueStatement, IfStatement, SwitchStatement, SwitchCase, ConditionalExpression, ThrowStatement, TryStatement, CatchClause } from "./ast";
import Signal from './signal';
import { Entry } from "./model";

const ES_VERSION = 5;
export default class Script {
  context: Context;
  constructor() {
    this.context = new Context(null, 20);
  }
  process(code: string) {
    this.context.ptr = this.context.script = acorn.parse(code, { ecmaVersion: ES_VERSION }) as any;
    const visitor = new Visitor(this.context);
    visitor.Program(this.context.script as any);
  }
}

class Visitor {
  context: Context;
  constructor(context: Context) {
    this.context = context;
  }
  traverse = (node: Node) => {
    return this[node.type](node);
  }
  Program = (node: Program) => {
    node.body.forEach(child => {
      this.traverse(child);
    })
  }
  VariableDeclaration = (node: VariableDeclaration) => {
    node.declarations.forEach(declaration => {
      const name = declaration.id.name;
      const value = declaration.init ? this.traverse(declaration.init) : undefined;
      this.context.getScope().declare(name, value);
    })
  }
  Literal = (node: Literal) => {
    return node.value;
  }
  Identifier = (node: Identifier) => {
    if (node.name === "undefined") return undefined;
    return this.context.getScope().get(node.name).value;
  }
  ExpressionStatement = (node: ExpressionStatement) => {
    return this.traverse(node.expression);
  }
  CallExpression = (node: CallExpression) => {
    const func = this.traverse(node.callee);
    const args = node.arguments.map(arg => this.traverse(arg));
    let value;
    if (node.callee.type == "MemberExpression") {
      const callee = node.callee as MemberExpression;
      value = this.traverse(callee.object);
    }
    return func.apply(value, args);
  }
  MemberExpression = (node: MemberExpression) => {
    const obj = this.traverse(node.object);
    const name = node.property.name;
    return obj[name];
  }
  BlockStatement = (node: BlockStatement) => {
    const isDeclare = (type) => type === "FunctionDeclaration" || type === "VariableDeclaration";
    node.body.forEach(node => {
      if (isDeclare(node.type)) this.traverse(node);
    });
    for (const child of node.body) {
      if (isDeclare(child)) continue;
      const signal = this.traverse(child);
      if (Signal.isSignal(signal)) return signal;
    }
  }
  FunctionDeclaration = (node: FunctionDeclaration) => {
    const fn = this.FunctionExpression(node as any);
    this.context.getScope().declare(node.id.name, fn);
  }
  FunctionExpression = (node: FunctionExpression) => {
    const context = this.context;
    const that = this;
    const fn = function () {
      context.stack.pushFrame(new StackFrame(context.stack.getFrame(), context.globalScope));
      const scope = context.getScope();
      scope.declare('this', this);
      scope.declare('arguments', arguments);

      node.params.forEach(function (param, index) {
        const name = param.name;
        scope.declare(name, arguments[index]);
      })
      const signal = that.traverse(node.body);
      if (Signal.isReturn(signal)) return signal.value;
    }
    Object.defineProperties(fn, {
      name: { value: node.id?.name || '' },
      length: { value: node.params.length }
    });
    return fn;
  }
  ObjectExpression = (node: ObjectExpression) => {
    const obj = {}
    for (const prop of node.properties) {
      let key
      if (prop.key.type === 'Literal') {
        key = `${prop.key.value}`
      } else if (prop.key.type === 'Identifier') {
        key = prop.key.name
      } else {
        throw new Error(`[ObjectExpression] Unsupported property key type`)
      }
      obj[key] = this.traverse(prop.value)
    }
    return obj
  }
  ArrayExpression = (node: ArrayExpression) => {
    return node.elements.map(ele => this.traverse(ele))
  }
  ThisExpression = (node: ThisExpression) => {
    const that = this.context.getScope().get('this');
    return that?.value || null;
  }
  NewExpression = (node: NewExpression) => {
    const func = this.traverse(node.callee)
    const args = node.arguments.map(arg => this.traverse(arg))
    return new (func.bind(null, ...args))
  }
  UpdateExpression = (node: UpdateExpression) => {
    const { operator, prefix } = node
    const { name } = node.argument;
    const scope = this.context.getScope();
    let vEntry = scope.get(name);

    if (operator === "++" && prefix) return ++vEntry.value;
    else if (operator === "++" && !prefix) return vEntry.value++;
    else if (operator === "--" && prefix) return --vEntry.value;
    else return vEntry.value--;
  }
  AssignmentExpression = (node: AssignmentExpression) => {
    const that = this;
    const rightVal = this.traverse(node.right);
    const ops = {
      '=': () => setValue(node.left, (leftVal) => rightVal, true),
      '+=': () => setValue(node.left, (leftVal) => leftVal + rightVal),
      '-=': () => setValue(node.left, (leftVal) => leftVal - rightVal),
      '*=': () => setValue(node.left, (leftVal) => leftVal * rightVal),
      '/=': () => setValue(node.left, (leftVal) => leftVal / rightVal),
      '%=': () => setValue(node.left, (leftVal) => leftVal % rightVal),
      '**=': () => setValue(node.left, (leftVal) => leftVal ** rightVal),
      '<<=': () => setValue(node.left, (leftVal) => leftVal << rightVal),
      '>>=': () => setValue(node.left, (leftVal) => leftVal >> rightVal),
      '>>>=': () => setValue(node.left, (leftVal) => leftVal >>> rightVal),
      '|=': () => setValue(node.left, (leftVal) => leftVal | rightVal),
      '^=': () => setValue(node.left, (leftVal) => leftVal ^ rightVal),
      '&=': () => setValue(node.left, (leftVal) => leftVal & rightVal),
    }
    return ops[node.operator]();
    function setValue(left: Node, right: (leftVal) => any, disableGetLeft: boolean = false) {
      const scope = that.context.getScope();
      if (left.type == "Identifier") {
        let leftNode = left as Identifier;
        scope.set(leftNode.name, right(disableGetLeft ? null : scope.get(leftNode.name)));
      } else if (left.type == "MemberExpression") {
        let memberNode = left as MemberExpression;
        let obj = that.traverse(memberNode);
        let name = memberNode.property.name;
        obj[name] = right(obj[name]);
      } else {
        throw Error(`not support node type ${left.type}`)
      }
    }
  }
  UnaryExpression = (node: UnaryExpression) => {
    const scope = this.context.getScope();
    const ops = {
      '-': () => -this.traverse(node.argument),
      '+': () => +this.traverse(node.argument),
      '!': () => !this.traverse(node.argument),
      '~': () => ~this.traverse(node.argument),
      'typeof': () => {
        if (node.argument.type === 'Identifier') {
          const argument = node.argument as Identifier;
          try {
            const value = scope.get(argument.name);
            return value ? typeof value.value : 'undefined';
          } catch (err) {
            if (err.message === `${argument.name} is not defined`) return 'undefined'
            else throw err
          }
        } else {
          return typeof this.traverse(node.argument)
        }
      },
      'void': () => void this.traverse(node.argument),
      'delete': () => {
        const argument = node.argument
        if (argument.type === 'MemberExpression') {
          let argumentNode = argument as MemberExpression;
          const obj = this.traverse(argumentNode.object)
          const name = this.getPropertyName(argumentNode)
          return delete obj[name]
        }
        else if (argument.type === 'Identifier') return false;
        else if (argument.type === 'Literal') return true;
      }
    }
    return ops[node.operator]();
  }
  BinaryExpression = (node: BinaryExpression) => {
    const left = this.traverse(node.left);
    const right = this.traverse(node.right);
    const ops = {
      '==': () => left == right,
      '!=': () => left != right,
      '===': () => left === right,
      '!==': () => left !== right,
      '<': () => left < right,
      '<=': () => left <= right,
      '>': () => left > right,
      '>=': () => left >= right,
      '<<': () => left << right,
      '>>': () => left >> right,
      '>>>': () => left >>> right,
      '+': () => left + right,
      '-': () => left - right,
      '*': () => left * right,
      '/': () => left / right,
      '%': () => left % right,
      '**': () => { throw new Error('es5 doesn\'t supports operator "**"') },
      '|': () => left | right,
      '^': () => left ^ right,
      '&': () => left & right,
      'in': () => left in right,
      'instanceof': () => left instanceof right
    }
    return ops[node.operator]();
  }
  LogicalExpression = (node: LogicalExpression) => {
    const left = this.traverse(node.left)
    if (left) {
      if (node.operator == '||') return true;
    } else if (node.operator == '&&') return false;
    const right = this.traverse(node.right)

    const ops = {
      '||': () => left || right,
      '&&': () => left && right
    }
    return ops[node.operator]();
  }
  ForStatement = (node: ForStatement) => {
    for (
      node.init && this.traverse(node.init);
      node.test ? this.traverse(node.test) : true;
      node.update && this.traverse(node.update)
    ) {
      const signal = this.traverse(node.body)

      if (Signal.isBreak(signal)) break;
      else if (Signal.isContinue(signal)) continue;
      else if (Signal.isReturn(signal)) return signal;
    }
  }
  ForInStatement = (node: ForInStatement) => {
    const { left, right, body } = node;
    const scope = this.context.getScope();

    let vEntry: Entry = null;
    if (left.type === 'VariableDeclaration') {
      const id = left.declarations[0].id
      vEntry = scope.declare(id.name, undefined)
    } else if (left.type === 'Identifier') {
      vEntry = scope.get(left.name)
    } else {
      throw new Error(`[ForInStatement] Unsupported left type "${left}"`)
    }

    for (const key in this.traverse(right)) {
      vEntry.value = key;
      const signal = this.traverse(body);
      if (Signal.isBreak(signal)) break;
      else if (Signal.isContinue(signal)) continue;
      else if (Signal.isReturn(signal)) return signal;
    }
  }
  WhileStatement = (node: WhileStatement) => {
    while (this.traverse(node.test)) {
      const signal = this.traverse(node.body);

      if (Signal.isBreak(signal)) break;
      else if (Signal.isContinue(signal)) continue;
      else if (Signal.isReturn(signal)) return signal;
    }
  }
  DoWhileStatement = (node: DoWhileStatement) => {
    do {
      const signal = this.traverse(node.body);

      if (Signal.isBreak(signal)) break;
      else if (Signal.isContinue(signal)) continue;
      else if (Signal.isReturn(signal)) return signal;
    } while (this.traverse(node.test))
  }
  ReturnStatement = (node: ReturnStatement) => {
    let value = node.argument ? this.traverse(node.argument) : undefined;
    return Signal.Return(value);
  }
  BreakStatement = (node: BreakStatement) => {
    let label = node.label?.name || undefined;
    return Signal.Break(label)
  }
  ContinueStatement = (node: ContinueStatement) => {
    let label = node.label?.name || undefined;
    return Signal.Continue(label)
  }
  IfStatement = (node: IfStatement) => {
    if (this.traverse(node.test)) return this.traverse(node.consequent);
    else if (node.alternate) return this.traverse(node.alternate);
  }
  SwitchStatement = (node: SwitchStatement) => {
    const discriminant = this.traverse(node.discriminant)
    let isMatch = false;
    for (const theCase of node.cases) {
      if (!theCase.test || isMatch || discriminant === this.traverse(theCase.test)) {
        isMatch = true;
        const signal = this.traverse(theCase)

        if (Signal.isBreak(signal)) break;
        else if (Signal.isContinue(signal)) continue;
        else if (Signal.isReturn(signal)) return signal;
      }
    }
  }
  SwitchCase = (node: SwitchCase) => {
    for (const child of node.consequent) {
      const signal = this.traverse(child)
      if (Signal.isSignal(signal)) return signal
    }
  }
  ConditionalExpression = (node: ConditionalExpression) => {
    return this.traverse(node.test)
      ? this.traverse(node.consequent)
      : this.traverse(node.alternate)
  }
  ThrowStatement(node: ThrowStatement) {
    throw this.traverse(node.argument)
  }
  TryStatement(node: TryStatement) {
    const { block, handler, finalizer } = node;
    try {
      return this.traverse(block)
    } catch (err) {
      if (handler) {
        const param = handler.param
        const scope = this.context.getScope();
        scope.declare(param.name, err)
        return this.traverse(handler)
      }
      throw err
    } finally {
      if (finalizer) return this.traverse(finalizer)
    }
  }
  CatchClause(node: CatchClause) {
    return this.traverse(node.body);
  }
  getPropertyName(node: MemberExpression) {
    if (node.computed) return this.traverse(node.property)
    else return node.property.name;
  }
}

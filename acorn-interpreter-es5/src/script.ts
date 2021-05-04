import * as acorn from "acorn";
import { Context, Scope } from './context';
import { ExpressionStatement, Identifier, Literal, Program, VariableDeclaration, Node, CallExpression, FunctionDeclaration, MemberExpression, FunctionExpression, NewExpression, UpdateExpression, ThisExpression, BlockStatement, VariableDeclarator, ReturnStatement } from "./ast";
import Signal from './signal';

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
    try {
      return this[node.type](node);
    } catch (err) {
      console.log("error", node);
      throw Error(`not found this ${node.type} handle`);
    }
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
    return this.context.getScope().get(node.name);
  }
  ExpressionStatement = (node: ExpressionStatement) => {
    return this[node.expression.type](node.expression);
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
    const scope = this.context.getScope();
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
    const staticParentScope = this.context.getScope();
    const context = this.context;
    const that = this;
    const fn = function () {
      const scope = new Scope(staticParentScope);
      context.stack.pushFrame({ scope });

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
  ThisExpression = (node: ThisExpression) => {
    const that = this.context.getScope().get('this');
    return that?.value || null;
  }
  ReturnStatement = (node: ReturnStatement) => {
    let value = node.argument ? this.traverse(node.argument) : undefined;
    return Signal.Return(value);
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
    let val = scope.get(name).value;

    if (operator === "++" && prefix) return ++val
    else if (operator === "++" && !prefix) return val++
    else if (operator === "--" && prefix) return --val
    else return val--;
  }
}
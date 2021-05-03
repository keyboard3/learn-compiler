import { findFunScope, findSymbolScope } from "./api";
import { K3_ASTNode_Type, K3_Context, K3_Scope, K3_SymbolType } from "./models";
import * as parser from "./parser";

/**
 * 遍历AST,计算
 */
export default class Script {
  static verbose: boolean = false;
  kc: K3_Context;

  constructor() {
    this.kc = new K3_Context(null, 10);
  }

  process(code: string) {
    try {
      const tree = parser.parse(code);
      if (Script.verbose) {
        tree.dumpAST("");
      }
      this.kc.ptr = tree;
      const result = this.evaluate(this.kc, "");
      console.log(result)
    } catch (err) {
      console.error(err);
    }
  }

  evaluate(kc: K3_Context, indent: string): number | null {
    let result: number = null;

    const node = kc.ptr;
    switch (node.type) {
      case K3_ASTNode_Type.Function: {
        const funName = node.token.text;
        const scope = findFunScope(kc, funName);
        scope.declareFunctions.set(funName, {
          script: node,
          parentScope: scope,
          args: node.args
        });
        break;
      }
      case K3_ASTNode_Type.Block:
      case K3_ASTNode_Type.Program:
        for (let child of node.children) {
          kc.ptr = child;
          result = this.evaluate(kc, indent)
          if (child.type == K3_ASTNode_Type.Return) break;
        }
        break;
      case K3_ASTNode_Type.Return:
        if (!node.children.length) break;
        kc.ptr = node.children[0];
        result = this.evaluate(kc, indent);
        break;
      case K3_ASTNode_Type.Primary:
        result = primaryExpression(node.token.text);
        break;
      case K3_ASTNode_Type.Unary: {
        kc.ptr = node.children[0];
        let val = this.evaluate(kc, `${indent}\t`);
        result = unaryExpression(node.token.text, val);
        break;
      }
      case K3_ASTNode_Type.Binary: {
        kc.ptr = node.children[0];
        let val1 = this.evaluate(kc, `${indent}\t`);
        kc.ptr = node.children[1];
        let val2 = this.evaluate(kc, `${indent}\t`);
        result = binaryExpression(node.token.text, val1, val2);
        break;
      }
      case K3_ASTNode_Type.Call: {
        let funName = node.token.text;
        const scope = findFunScope(kc, funName);
        let fun = scope.declareFunctions.get(funName);
        //加入新栈帧
        let frame = { parentFrame: kc.stack.frames[kc.stack.ptr], scope: new K3_Scope(fun.parentScope) };
        kc.stack.pushFrame(frame);
        //设置新栈帧参数
        let args = fun.args || [];
        let funcNode = fun.script;
        if (!funcNode) throw Error("not fund function " + funName);
        for (let i = 0; i < args.length; i++) {
          let argNode = i < node.children.length ? node.children[i] : null;
          if (!argNode) break;
          kc.ptr = argNode;
          let argValue = this.evaluate(kc, indent);
          frame.scope.declareSymbols.set(args[i].value, { type: K3_SymbolType.SYM_ARGUMENT, value: argValue });
        }
        //调用方法
        let block = funcNode.children[0];
        kc.ptr = block;
        result = this.evaluate(kc, indent);
        kc.stack.popFrame();
        break;
      }
      case K3_ASTNode_Type.NumberLiteral:
        result = parseInt(node.token.text);
        break;
      case K3_ASTNode_Type.Identifier: {
        let varName = node.token.text;
        const scope = findSymbolScope(kc, varName);
        if (scope.declareSymbols.has(varName)) {
          let symbol = scope.declareSymbols.get(varName);
          if (!symbol) throw Error(`variable ${varName} has not been set any value`)
          result = symbol.value;
        } else throw Error(`unknown variable: ${varName}`)
        break;
      }
      case K3_ASTNode_Type.AssignmentStmt:
      case K3_ASTNode_Type.VarDeclaration: {
        let varName = node.token.text;
        let rval = null;
        if (node.children.length) { //说明是初始化语句，给变量赋值
          kc.ptr = node.children[0];
          rval = this.evaluate(kc, `${indent}\t`);
        }
        const scope = findSymbolScope(kc, varName);
        scope.declareSymbols.set(varName, { value: rval, type: K3_SymbolType.SYM_VARIABLE });
        result = rval;
        break;
      }
    }
    if (Script.verbose) {
      console.log(`${indent} Result: ${result}`)
    }
    return result;
  }
}

function assignExpression(operator, lval, rval) {
  let assignMap = {
    '=': () => rval,
    '>>=': () => lval >> rval,
    '<<=': () => lval << rval,
    '+=': () => lval + rval,
    '-=': () => lval - rval,
    '*=': () => lval * rval,
    '/=': () => lval / rval,
    '%=': () => lval % rval,
  }
  let fun = assignMap[operator];
  return fun ? fun() : null;
}
function primaryExpression(text): any {
  switch (text) {
    case "null":
      return null;
    case "true":
      return true;
    case "false":
      return false;
    case "this": //todo
    case "super"://todo;
    default:
      return undefined;
  }
}

function unaryExpression(operator, val) {
  let unaryMap = {
    '!': () => !val,
    '~': () => ~val
  };
  let fun = unaryMap[operator];
  return fun ? fun() : null;
}

function binaryExpression(operator, val1, val2) {
  let binaryMap = {
    '+': () => val1 + val2,
    '-': () => val1 - val2,
    '*': () => val1 * val2,
    '/': () => val1 / val2,
    '%': () => val1 % val2,
    '<<': () => val1 << val2,
    '>>': () => val1 >> val2,
    '>': () => val1 > val2,
    '<': () => val1 < val2,
    '<=': () => val1 <= val2,
    '>=': () => val1 >= val2,
    '==': () => val1 == val2,
    '!=': () => val1 != val2,
    '||': () => val1 || val2,
    '&&': () => val1 && val2,
    '|': () => val1 | val2,
    '^': () => val1 ^ val2,
    '&': () => val1 & val2,
  }
  let fun = binaryMap[operator];
  if (fun) return fun();
  return null;
}

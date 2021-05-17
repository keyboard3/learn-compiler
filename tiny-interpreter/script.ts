import { ASTNode, ASTNode_Type, Context, StackFrame } from "./models";
import * as parser from "./parser";

/**
 * 遍历AST,计算
 */
export default class Script {
  static verbose: boolean = false;
  kc: Context;

  constructor() {
    this.kc = new Context(10);
  }

  process(code: string) {
    try {
      const tree = parser.parse(code);
      if (Script.verbose) {
        tree.dumpAST("");
      }
      const result = this.evaluate(tree, "");
      console.log(result)
    } catch (err) {
      console.error(err);
    }
  }

  evaluate(node: ASTNode, indent: string): number | null {
    let result: number = null;
    let kc = this.kc;
    switch (node.type) {
      case ASTNode_Type.Function: {
        const funName = node.token.text;
        kc.set(funName, {
          script: node,
          parentFrame: kc.stack,
        });
        break;
      }
      case ASTNode_Type.Block:
      case ASTNode_Type.Program:
        for (let child of node.children) {
          result = this.evaluate(child, indent)
          if (child.type == ASTNode_Type.Return) break;
        }
        break;
      case ASTNode_Type.Return:
        if (!node.children.length) break;
        result = this.evaluate(node.children[0], indent);
        break;
      case ASTNode_Type.Primary:
        result = primaryExpression(node.token.text);
        break;
      case ASTNode_Type.Unary: {
        let val = this.evaluate(node.children[0], `${indent}\t`);
        result = unaryExpression(node.token.text, val);
        break;
      }
      case ASTNode_Type.Binary: {
        let val1 = this.evaluate(node.children[0], `${indent}\t`);
        let val2 = this.evaluate(node.children[1], `${indent}\t`);
        result = binaryExpression(node.token.text, val1, val2);
        break;
      }
      case ASTNode_Type.Call: {
        let funName = node.token.text;
        let fun = kc.get(funName);
        //加入新栈帧
        kc.pushFrame(new StackFrame(fun.parentFrame, kc.globalScope));
        //设置新栈帧参数
        let args = fun.args || [];
        let funcNode = fun.script;
        if (!funcNode) throw Error("not fund function " + funName);
        for (let i = 0; i < args.length; i++) {
          let argNode = i < node.children.length ? node.children[i] : null;
          if (!argNode) break;
          let argValue = this.evaluate(argNode, indent);
          kc.set(args[i].value, argValue)
        }
        //调用方法
        let block = funcNode.children[0];
        result = this.evaluate(block, indent);
        kc.popFrame();
        break;
      }
      case ASTNode_Type.NumberLiteral:
        result = parseInt(node.token.text);
        break;
      case ASTNode_Type.Identifier: {
        let varName = node.token.text;
        result = kc.get(varName);
        break;
      }
      case ASTNode_Type.AssignmentStmt:
      case ASTNode_Type.VarDeclaration: {
        let varName = node.token.text;
        let rval = null;
        if (node.children.length) { //说明是初始化语句，给变量赋值
          rval = this.evaluate(node.children[0], `${indent}\t`);
        }
        kc.set(varName, rval);
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

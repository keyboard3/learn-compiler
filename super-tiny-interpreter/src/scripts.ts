import { ASTNode, ASTNodeType } from "./models";

/**
 * 遍历AST,计算
 */

export default class Script {
  static verbose: boolean = false;
  variables: Map<string, number> = new Map();

  evaluate(node: ASTNode, indent: string): number | null {
    let result: number = null;
    switch (node.type) {
      case ASTNodeType.Program:
        for (let child of node.children) {
          result = this.evaluate(child, indent)
        }
        break;
      case ASTNodeType.Additive: {
        let child1 = node.children[0];
        let val1 = this.evaluate(child1, `${indent}\t`);
        let child2 = node.children[1];
        let val2 = this.evaluate(child2, `${indent}\t`);
        if (node.text == "+") result = val1 + val2;
        else result = val1 - val2;
        break;
      }
      case ASTNodeType.Multiplicative: {
        let child1 = node.children[0];
        let val1 = this.evaluate(child1, `${indent}\t`);
        let child2 = node.children[1];
        let val2 = this.evaluate(child2, `${indent}\t`);
        if (node.text == "*") result = val1 * val2;
        else result = val1 / val2;
        break;
      }
      case ASTNodeType.IntLiteral:
        result = parseInt(node.text);
        break;
      case ASTNodeType.Identifier: {
        let varName = node.text;
        if (this.variables.has(varName)) {
          let value = this.variables.get(varName);
          if (!value) throw Error(`variable ${varName} has not been set any value`)
          result = value;
        } else throw Error(`unknown variable: ${varName}`)
        break;
      }
      case ASTNodeType.AssignmentStmt: {
        let varName = node.text;
        if (!this.variables.has(varName)) throw Error(`unknown variable: ${varName}`)
        //继续给变量赋值
      }
      case ASTNodeType.IntDeclaration: {
        let varName = node.text;
        let varValue = null;
        if (node.children.length) { //说明是初始化语句，给变量赋值
          let child = node.children[0];
          varValue = this.evaluate(child, `${indent}\t`);
        }
        this.variables.set(varName, varValue);
        result = varValue;
        break;
      }
    }
    if (Script.verbose) {
      console.log(`${indent} Result: ${result}`)
    }
    return result;
  }
}

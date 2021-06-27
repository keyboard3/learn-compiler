import { OpType, ab2str, to_optype_str } from "./models";

/**
 * 遍历AST,生成字节码
 */

export default class Script {
  static verbose: boolean = false;
  variables: Map<string, number> = new Map();
  stack: number[] = [];//操作数栈

  evaluate(opBuffer: ArrayBuffer): number | null {
    console.log("===evaluate===")
    let pc = 0//指令计数器
    let result: number = null;
    const dataView = new DataView(opBuffer);
    while (pc < opBuffer.byteLength) {
      let opt = getOpt();
      //分析处理指令
      switch (opt) {
        case OpType.NUMBER:
          this.stack.push(getOpNumber());
          break;
        case OpType.NAME:
          {
            let name = getOpStr();
            if (!this.variables.has(name)) throw `${name} not defined`;
            this.stack.push(this.variables.get(name));
          }
          break;
        case OpType.INT_DECL:
        case OpType.ASSIGN:
          {
            let name = getOpStr();
            this.variables.set(name, this.stack.pop());
          }
          break;
        case OpType.ADD:
        case OpType.MINUS:
        case OpType.MUTI:
        case OpType.DIVID:
          const rval = this.stack.pop();
          const lval = this.stack.pop();
          let val = 0;
          if (opt == OpType.ADD) val = lval + rval;
          else if (opt == OpType.MINUS) val = lval - rval;
          else if (opt == OpType.MUTI) val = lval * rval;
          else if (opt == OpType.DIVID) val = lval / rval;
          this.stack.push(val);
          break;
      }
    }
    if (Script.verbose) {
      console.log(`Result: ${result}`)
    }
    console.log(this.variables, this.stack);
    return result;
    //获取指令类型
    function getOpt() {
      let opt: OpType = dataView.getInt8(pc);//取指令
      pc += 8;
      console.log(`\topt:${to_optype_str(opt)}`);
      return opt;
    }
    //获取指令数字操作数
    function getOpNumber() {
      let num = dataView.getInt32(pc);//取指令
      pc += 32;
      console.log(`\topNumber:${num}`, pc);
      return num;
    }
    //获取指令字符串操作数
    function getOpStr() {
      let strByteLen = dataView.getInt32(pc);//取指令
      pc += 32;
      let name = ab2str(new Uint8Array(opBuffer, pc, strByteLen));
      pc += strByteLen;
      console.log(`\topStr:${name}`, pc);
      return name;
    }
  }
}
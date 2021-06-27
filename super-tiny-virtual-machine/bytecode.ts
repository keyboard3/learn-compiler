import { ASTNode, ASTNodeType, OpType, str2Uint8Array, to_optype_str } from "./models";

/**
 * 遍历AST,生成字节码
 */
const BUFFER_SIZE = 10000;
export default class Bytecode {
    offset: number = 0;
    parse(node: ASTNode, indent: string): ArrayBuffer {
        console.log("===byte generator===")
        this.offset = 0;
        let opBuffer = new ArrayBuffer(BUFFER_SIZE);//二进制指令buffer
        const dataView = new DataView(opBuffer);
        this.execute(node, indent, dataView);
        return opBuffer.slice(0, this.offset);
    }
    execute(node: ASTNode, indent: string, dataView: DataView): any {
        indent = `${indent}\t`;
        let that = this;
        switch (node.type) {
            case ASTNodeType.Program:
                for (let child of node.children) {
                    this.execute(child, indent, dataView)
                }
                break;
            case ASTNodeType.Multiplicative:
            case ASTNodeType.Additive: {
                node.children.sort((a, b) => b.children.length - a.children.length);
                this.execute(node.children[0], indent, dataView);
                this.execute(node.children[1], indent, dataView);
                if (node.text == "+") emit1(OpType.ADD, dataView);
                else if (node.text == "-") emit1(OpType.MINUS, dataView);
                else if (node.text == "*") emit1(OpType.MUTI, dataView);
                else if (node.text == "/") emit1(OpType.DIVID, dataView);
                break;
            }
            case ASTNodeType.IntLiteral:
                emit2(OpType.NUMBER, dataView, parseInt(node.text));
                break;
            case ASTNodeType.Identifier:
                emit2(OpType.NAME, dataView, node.text);
                break;
            case ASTNodeType.AssignmentStmt:
                this.execute(node.children[0], indent, dataView);
                emit2(OpType.ASSIGN, dataView, node.text);
                break;
            case ASTNodeType.IntDeclaration: {
                this.execute(node.children[0], indent, dataView);
                emit2(OpType.INT_DECL, dataView, node.text);
                break;
            }
        }
        function emit1(op: OpType, dataView: DataView) {
            that.emit1(op, dataView);
            console.log(indent, node.type, node.text, "emit1", to_optype_str(op), that.offset);
        }
        function emit2(op: OpType, dataView: DataView, val: string | number) {
            that.emit2(op, dataView, val);
            console.log(indent, node.type, node.text, "emit2", to_optype_str(op), val, that.offset);
        }
    }
    emit1(op: OpType, dataView: DataView) {
        dataView.setInt8(this.offset, op);
        this.offset += 8;
    }
    emit2(op: OpType, dataView: DataView, val: string | number) {
        dataView.setInt8(this.offset, op);
        this.offset += 8;
        if (typeof val == "string") {
            let strUint8Array = str2Uint8Array(val);
            dataView.setInt32(this.offset, strUint8Array.byteLength);
            this.offset += 32;
            const baseUnit8Array = new Uint8Array(dataView.buffer, this.offset, strUint8Array.byteLength);
            baseUnit8Array.set(strUint8Array);
            this.offset += strUint8Array.byteLength;
        } else if (typeof val == "number") {
            dataView.setInt32(this.offset, val);
            this.offset += 32;
        }
    }
}
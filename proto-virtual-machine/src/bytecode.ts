import { ASTNode, ASTNodeType, ATOM_TYPE, DATUM_TYPE, OP_TYPE, SYMOBL_TYPE, _Atom, _Context, _Datum, _Function, _Property, _Scope, _Script, _Symbol } from "./models";
/**
 * 语义解析生成字节码
 */
export function createCode(context: _Context, node: ASTNode) {
    context.script = new _Script();
    generateCode(context, context.script, node, 0, "");
}
function generateCode(context: _Context, script: _Script, node: ASTNode, offset: number = 0, indent: string) {
    indent += "\t";
    const atoms = script.atoms;
    const dataView = new DataView(script.code);
    console.log(indent, node.type, node.token?.text);
    switch (node.type) {
        case ASTNodeType.Block:
        case ASTNodeType.Program:
            node.children.forEach(child => offset = generateCode(context, script, child, offset, indent));
            break;
        case ASTNodeType.Function:
            const nameAtom = atoms[getAtomIndex(ATOM_TYPE.NAME, node.token.text)];
            const funScript = new _Script();
            const fun = new _Function(nameAtom, funScript, context.staticLink);
            fun.scope = new _Scope();
            generateCode(context, fun.script, node.children[0], 0, "");
            fun.script.args = node.args.map(arg => new _Symbol(fun.scope, SYMOBL_TYPE.ARGUMENT, { key: funScript.atoms.find(item => item.val == arg.text) }));
            fun.scope.list = fun.script.args;
            const funSym = new _Symbol(context.staticLink, SYMOBL_TYPE.PROPERTY, {
                key: nameAtom,
                value: new _Property(new _Datum(DATUM_TYPE.FUNCTION, fun))
            });
            context.staticLink.list.push(funSym);
            break;
        case ASTNodeType.Binary:
            node.children.forEach(child => offset = generateCode(context, script, child, offset, indent));
            switch (node.token.text) {
                case "+":
                    appendBuffer(OP_TYPE.ADD);
                    break;
                case "-":
                    appendBuffer(OP_TYPE.MINUS);
                    break;
                case "*":
                    appendBuffer(OP_TYPE.MUTIL);
                    break;
                case "/":
                    appendBuffer(OP_TYPE.DIVID);
                    break;
            }
            break;
        case ASTNodeType.New:
            appendBuffer(OP_TYPE.NAME, getAtomIndex(ATOM_TYPE.NAME, node.children[0].token.text));
            node.children[0].children.forEach(child => offset = generateCode(context, script, child, offset, indent));
            appendBuffer(OP_TYPE.NEW, node.children[0].children.length);
            break;
        case ASTNodeType.Call:
            appendBuffer(OP_TYPE.NAME, getAtomIndex(ATOM_TYPE.NAME, node.token.text));
            node.children.forEach(child => offset = generateCode(context, script, child, offset, indent));
            appendBuffer(OP_TYPE.CALL, node.children.length);
            break;
        case ASTNodeType.Return:
            node.children.forEach(child => offset = generateCode(context, script, child, offset, indent));
            appendBuffer(OP_TYPE.RETURN);
            break;
        case ASTNodeType.VarDeclaration:
        case ASTNodeType.AssignmentStmt:
            offset = generateCode(context, script, node.children[0], offset, indent)
            appendBuffer(OP_TYPE.NAME, getAtomIndex(ATOM_TYPE.NAME, node.token.text));
            appendBuffer(OP_TYPE.ASSIGN);
            break;
        case ASTNodeType.NumberLiteral:
            appendBuffer(OP_TYPE.NUMBER, getAtomIndex(ATOM_TYPE.NUMBER, parseInt(node.token.text)));
            break;
        case ASTNodeType.Identifier:
            appendBuffer(OP_TYPE.NAME, getAtomIndex(ATOM_TYPE.NAME, node.token.text));
            break;
    }
    if (indent.length == 1) script.code = script.code.slice(0, offset);
    return offset;
    function appendBuffer(...params: number[]) {
        params.forEach(value => {
            dataView.setUint8(offset, value);
            offset += 8;
        });
    }
    function getAtomIndex(type, val) {
        let index = atoms.findIndex(item => item.flag == type && item.val == val);
        if (index >= 0) return index;
        atoms.push(new _Atom(type, val));
        return atoms.length - 1;
    }
}
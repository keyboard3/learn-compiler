import { ATOM_TYPE, DATUM_TYPE, OP_TYPE, _Atom, _Context, _Datum, _Property, _Scope, _Script, _Symbol, _Frame, SYMOBL_TYPE } from "./models";
import * as lexer from "./lexer";
import * as parser from "./parser";
import * as bytecode from "./bytecode";

export default class VitrulMachine {
    verbose: boolean = false;
    context: _Context;
    constructor() {
        this.context = new _Context();
    }
    process(code: string) {
        const tokens = lexer.tokenize(code);
        const rootNode = parser.prog(tokens);
        bytecode.createCode(this.context, rootNode);
        const result = new _Datum(DATUM_TYPE.UNDEF);
        this.codeInterpret(this.context, this.context.script, this.context.globalObject, result);
        this.context.staticLink.list.forEach(item => {
            console.log(item.entry.key.val, item.entry.value);
        })
    }
    codeInterpret(context: _Context, script: _Script, slink: _Scope, result: _Datum) {
        const stack = context.stack;
        const atoms = script.atoms;
        const codeView = new DataView(script.code);
        let pc = 0;
        //保存旧上下文
        const oldslink = context.staticLink;
        context.staticLink = slink;
        while (pc < codeView.byteLength) {
            //取指令
            let opt = popBuffer();
            console.log("pc", to_command_str(opt));
            //分析处理指令
            switch (opt) {
                case OP_TYPE.CALL:
                    {
                        //通过nameAtom从当前作用域中找到symobl
                        let argc = popBuffer();
                        //直接从symobl中获取到function
                        const funDatum = stack.base[stack._ptr - argc - 1];
                        resolveValue(funDatum);
                        //创建栈帧
                        const frame = new _Frame(stack);
                        frame.argc = argc;
                        frame._argv = stack._ptr - argc;
                        frame._vars = stack._ptr;
                        frame.fun = funDatum.fun;
                        frame.down = context.stack.frame;
                        //将已压入栈的参数，参数符号的栈实参位置设置
                        if (frame.fun.script) {
                            for (let i = 0; i < argc; i++)
                                frame.fun.script.args[i].slot = i;
                        }
                        //压入栈
                        stack.frame = frame;
                        const result = new _Datum(DATUM_TYPE.UNDEF);
                        if (frame.fun.call) {
                            let params = frame.argv.map(item => {
                                resolveValue(item);
                                return item;
                            }).map(item => item.nval || item.sval);
                            frame.fun.call(...params);
                        } else {
                            this.codeInterpret(context, frame.fun.script, frame.fun.scope, result);
                        }
                        //调用完毕恢复栈帧
                        stack.frame.vars.forEach(() => stack.pop());//动态变量弹出
                        stack.frame.argv.forEach(() => stack.pop());//参数弹出
                        stack.pop();//方法名弹出
                        stack.frame = frame.down;
                        //结果压入栈中
                        stack.push(result);
                    }
                    break;
                case OP_TYPE.NUMBER:
                    stack.push(atomTempDatum(getAtom()));
                    break;
                case OP_TYPE.NAME:
                    stack.push(atomTempDatum(getAtom()));
                    break;
                case OP_TYPE.ASSIGN:
                    {
                        let isOk: boolean, lval: _Datum, rval: _Datum;
                        lval = stack.pop();
                        resolveSymbol(lval);

                        rval = stack.pop();
                        isOk = resolveValue(rval);
                        if (!isOk) return;

                        if (lval.flag != DATUM_TYPE.SYMBOL) {
                            //定义symbol
                            let symbol = new _Symbol(context.staticLink, SYMOBL_TYPE.VARIABLE, { key: lval.atom });
                            if (stack.frame) {
                                stack.push(rval);
                                stack.frame.nvars++;
                                symbol.slot = stack.frame.nvars - 1;
                            } else {
                                symbol.entry.value = rval;
                            }
                            context.staticLink.list.push(symbol);
                        } else {
                            const sym = lval.symbol;
                            //已经是符号，根据符号类型，修改值
                            switch (sym.type) {
                                case SYMOBL_TYPE.ARGUMENT:
                                case SYMOBL_TYPE.VARIABLE:
                                    if (sym.entry.value || !stack.frame)
                                        sym.entry.value = rval;
                                    else if (sym.type == SYMOBL_TYPE.ARGUMENT) stack.frame.argv[sym.slot] = rval;
                                    else stack.frame.nvars[sym.slot] = rval;
                                    break;
                                case SYMOBL_TYPE.PROPERTY:
                                    (sym.entry.value as _Property).datum = rval;
                                    break;
                            }
                        }
                    }
                    break;
                case OP_TYPE.RETURN:
                    {
                        let val = stack.pop();
                        resolveValue(val);
                        Object.assign(result, val);
                    }
                    break;
                case OP_TYPE.ADD:
                    {
                        let rval: _Datum, lval: _Datum;
                        rval = stack.pop();
                        lval = stack.pop();
                        resolveValue(rval);
                        resolveValue(lval);
                        const value = rval.nval + lval.nval;
                        stack.push(new _Datum(DATUM_TYPE.NUMBER, value));
                    }
                    break;
            }
        }
        //还原旧上下文
        context.staticLink = oldslink;
        function resolveValue(datum: _Datum): boolean {
            resolveSymbol(datum);
            //如果datum还是符号，则需要进一步解析符号
            if (datum.flag == DATUM_TYPE.SYMBOL) {
                if (SYMOBL_TYPE.PROPERTY == datum.symbol.type) {
                    Object.assign(datum, (datum.symbol.entry.value as _Property).datum);
                    resolvePrimary(datum);
                    return true;
                }
                //参数和变量先从符号上尝试读取
                if (datum.symbol.entry.value) {
                    Object.assign(datum, datum.symbol.entry.value);
                    resolvePrimary(datum);
                    return true;
                }
                //否则就从栈里找到该符号的栈帧中存储的变量
                let targetFp: _Frame;
                for (let fp = stack.frame; fp; fp = fp.down) {
                    if (fp.fun.scope == datum.symbol.scope) {
                        targetFp = fp;
                        break;
                    }
                }
                if (!targetFp) return false;
                if (datum.symbol.type == SYMOBL_TYPE.ARGUMENT) Object.assign(datum, targetFp.argv[datum.symbol.slot]);
                else Object.assign(datum, targetFp.vars[datum.symbol.slot]);
                resolvePrimary(datum);
            } else if (datum.flag == DATUM_TYPE.ATOM) {
                if (datum.atom.flag == ATOM_TYPE.NUMBER) {
                    datum.flag = DATUM_TYPE.NUMBER;
                    datum.nval = datum.atom.val as number;
                } else if (datum.atom.flag == ATOM_TYPE.STRING) {
                    datum.flag = DATUM_TYPE.STRING;
                    datum.sval = datum.atom.val as string;
                } else return false;
            }
            return true;
        }
        function resolvePrimary(datum: _Datum) {
            if (datum.flag != DATUM_TYPE.ATOM) return;
            resolveValue(datum);
        }
        /**
         * 将如果是字面量找到symbol
         */
        function resolveSymbol(datum: _Datum): boolean {
            if (datum.flag == DATUM_TYPE.SYMBOL) return true;
            if (datum.flag == DATUM_TYPE.ATOM) {
                const symbol = findSymbolByAtom(context.staticLink, datum.atom);
                if (!symbol) return false;
                datum.symbol = symbol;
                datum.flag = DATUM_TYPE.SYMBOL;
                return true;
            }
            return false;
            function findSymbolByAtom(scope: _Scope, atom: _Atom) {
                let symbol = scope.list.find(symbol => symbol.entry?.key == atom);
                if (!symbol && scope.parent) {
                    symbol = findSymbolByAtom(scope.parent, atom);
                }
                return symbol;
            }
        }
        function atomTempDatum(atom: _Atom) {
            return new _Datum(DATUM_TYPE.ATOM, atom);
        }
        function getAtom(): _Atom {
            return atoms[popBuffer()];
        }
        function popBuffer() {
            const index = codeView.getUint8(pc);
            pc += 8;
            return index;
        }
    }
}

function to_command_str(type) {
    if (type == OP_TYPE.NUMBER) return "number";
    if (type == OP_TYPE.NAME) return "name";
    if (type == OP_TYPE.ADD) return "add";
    if (type == OP_TYPE.ASSIGN) return "assign";
    if (type == OP_TYPE.FUNCTION_DEFINE) return "function defined";
    if (type == OP_TYPE.FUNCTION_NATIVE) return "function native";
    if (type == OP_TYPE.CALL) return "call";
    if (type == OP_TYPE.RETURN) return "return";
}
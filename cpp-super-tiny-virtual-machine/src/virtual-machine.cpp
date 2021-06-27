#include "parser.h"
#include "bytecode.h"
#include "virtual-machine.h"
#include "stack"
VirtualMachine::VirtualMachine()
{
    NewCodeGenerator(kc);
}

int VirtualMachine::process(string code)
{
    int result;
    ASTNode *node = parse(code);
    bytecodeParse(kc, node, "");
    NewScript(kc);

    stack<Atom *> stack;
    uint8_t *ptr = kc.script->code;
    uint8_t *end = ptr + kc.script->length;

    while (ptr < end)
    {
        int value;
        Atom *nameAtom, *valueAtom, *rval, *lval;
        //取指令
        OP_TYPE type = (OP_TYPE)ptr[0];
        switch (type)
        {
        case OP_TYPE::NUMBER:
            stack.push(getAtom(kc, ptr[1]));
            ptr++;
            break;
        case OP_TYPE::NAME:
        {
            nameAtom = getAtom(kc, ptr[1]);
            if (globalScope.find(nameAtom->sval) == globalScope.end())
                throw nameAtom->sval + " not defined";
            stack.push(globalScope[nameAtom->sval]);
            ptr++;
            result = stack.top()->ival;
            break;
        }
        case OP_TYPE::ADD:
        case OP_TYPE::MINUS:
        case OP_TYPE::MUTI:
        case OP_TYPE::DIVID:
            rval = stack.top();
            stack.pop();
            lval = stack.top();
            stack.pop();
            value = 0;
            if (type == OP_TYPE::ADD)
                value = lval->ival + rval->ival;
            if (type == OP_TYPE::MINUS)
                value = lval->ival - rval->ival;
            if (type == OP_TYPE::MUTI)
                value = lval->ival * rval->ival;
            if (type == OP_TYPE::DIVID)
                value = lval->ival / rval->ival;
            stack.push(generateAtom(value));
            result = value;
            break;
        case OP_TYPE::INT_DECL:
        case OP_TYPE::ASSIGN:
        {
            nameAtom = getAtom(kc, ptr[1]);
            valueAtom = nullptr;
            if (!stack.empty())
            {
                valueAtom = stack.top();
                stack.pop();
            }
            globalScope[nameAtom->sval] = valueAtom;
            ptr++;
            if (valueAtom != nullptr)
                result = valueAtom->ival;
        }
        break;
        }
        ptr++;
    }
    return result;
};

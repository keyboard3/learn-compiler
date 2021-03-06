#include "machine.h"
#include "lexer.h"
#include "parser.h"
#include "bytecode.h"
#include "iostream"
void codeInterpret(Context *context, Script *script, Scope *slink, Datum *result, string indent);
Machine::Machine()
{
    context = new Context(1000);
}
void Machine::process(string code)
{
    auto tokens = tokenize(code);
    auto node = parser(tokens);
    createCode(context, node);
    Datum *result = new Datum();
    codeInterpret(context, context->script, context->staticLink, result, "");
    dumpScope(context->staticLink, "");
}
void codeInterpret(Context *context, Script *script, Scope *slink, Datum *result, string indent)
{
    indent += "\t";
    uint8_t *ptr = script->code;
    uint8_t *end = ptr + script->length;
    Stack *stack = &context->stack;
    Scope *oldslink = context->staticLink;
    context->staticLink = slink;
    // dumpScope(oldslink, indent);
    while (ptr < end)
    {
        OP_TYPE op = (OP_TYPE)ptr[0];
        cout << indent << "op:" << to_op_str(op) << endl;
        ptr++;

        switch (op)
        {
        case OP_TYPE::NAME:
        case OP_TYPE::NUMBER:
            pushAtom(getAtom(script, ptr[0]), stack);
            ptr++;
            break;
        case OP_TYPE::ASSIGN:
        {
            Datum *lval = popDatum(stack);
            resolveSymbol(context, lval);
            Datum *rval = popDatum(stack);
            resolveValue(context, rval);
            if (lval->type == DATUM_TYPE::ATOM)
            {
                Symbol *sym = new Symbol(SYMBOL_TYPE::VARIABLE, context->staticLink);
                sym->entry.key = lval->u.atom;
                if (stack->frame != nullptr)
                {
                    pushDatum(stack, rval);
                    sym->slot = stack->frame->nvars;
                    stack->frame->nvars++;
                }
                else
                {
                    sym->entry.value = new Datum();
                    memcpy(sym->entry.value, rval, sizeof(Datum));
                }
                pushSymbol(sym, context->staticLink);
            }
            else if (lval->u.sym->type == SYMBOL_TYPE::PROPERTY)
            {
                Property *prop = (Property *)lval->u.sym->entry.value;
                memcpy(prop->datum, rval, sizeof(Datum));
            }
            else if (stack->frame == nullptr)
                lval->u.sym->entry.value = rval;
            else if (lval->u.sym->type == SYMBOL_TYPE::ARGUMENT)
                memcpy(&stack->frame->argv[lval->u.sym->slot], rval, sizeof(Datum));
            else if (lval->u.sym->type == SYMBOL_TYPE::VARIABLE)
                memcpy(&stack->frame->vars[lval->u.sym->slot], rval, sizeof(Datum));
        }
        break;
        case OP_TYPE::ADD:
        case OP_TYPE::MINUS:
        case OP_TYPE::MUTIL:
        case OP_TYPE::DIVID:
        {
            Datum *lval = popDatum(stack);
            resolveValue(context, lval);
            Datum *rval = popDatum(stack);
            resolveValue(context, rval);
            number value = 0;
            if (op == OP_TYPE::ADD)
                value = lval->u.nval + rval->u.nval;
            if (op == OP_TYPE::MINUS)
                value = lval->u.nval - rval->u.nval;
            if (op == OP_TYPE::MUTIL)
                value = lval->u.nval * rval->u.nval;
            if (op == OP_TYPE::DIVID)
                value = lval->u.nval / rval->u.nval;
            //?????????????????????
            pushNumber(value, stack);
        }
        break;
        case OP_TYPE::RETURN:
        {
            unsigned hasVal = ptr[0];
            ptr++;
            if (hasVal != 1)
                break;
            Datum *rval = popDatum(stack);
            bool isOk = resolveValue(context, rval);
            memcpy(result, rval, sizeof(Datum));
        }
        break;
        case OP_TYPE::CALL:
        {
            unsigned argc = ptr[0];
            ptr++;
            Datum *funDatum = stack->ptr - argc - 1;
            bool isOK = resolveValue(context, funDatum);
            if (funDatum->type != DATUM_TYPE::FUNCTION)
                throw funDatum->u.atom->sval + " not defined";
            //????????????
            Frame *frame = new Frame();
            frame->down = stack->frame;
            stack->frame = frame;
            frame->fun = funDatum->u.fun;
            frame->argc = argc;
            frame->argv = stack->ptr - argc;
            frame->nvars = 0;
            frame->vars = stack->ptr;
            //???????????????????????????????????????
            int slot = 0;
            for (Symbol *sym = frame->fun->scope->list; sym != nullptr && slot < argc; sym = sym->next, slot++)
            {
                resolveValue(context, &frame->argv[slot]); //????????????????????????????????????
                sym->scope = frame->fun->scope;
                sym->slot = slot;
            }
            //??????
            Datum *result = new Datum();
            result->type = DATUM_TYPE::UNDEF;
            codeInterpret(context, frame->fun->script, frame->fun->scope, result, indent);
            //????????????????????????
            stack->frame = frame->down;
            stack->ptr = frame->argv;
            if (result->type != DATUM_TYPE::UNDEF)
                pushDatum(stack, result);
        }
        break;
        }
    }
    // dumpScope(oldslink, indent);
    context->staticLink = oldslink;
}

#include "bytecode.h"
#include "iostream"
void emit1(Script *script, OP_TYPE type);
void emit2(Script *script, OP_TYPE type, uint8_t op1);
void generateCode(Script *script, ASTNode *node, Scope *slink, string indent);
void createCode(Context *context, ASTNode *node)
{
    cout << "createCode" << endl;
    generateCode(context->script, node, context->staticLink, "");
};
void generateCode(Script *script, ASTNode *node, Scope *slink, string indent)
{
    indent += "\t";
    switch (node->type)
    {
    case ASTNodeType::Program:
    case ASTNodeType::Block:
        for (auto child : node->children)
            generateCode(script, child, slink, indent);
        break;
    case ASTNodeType::Function:
    {
        Atom *nameAtom = script->atoms[getAtomIndex(script, node->text)];
        //函数对象
        Script *funScript = new Script(1000);
        Function *fun = new Function(nameAtom, funScript, slink);
        generateCode(funScript, node->children[0], fun->slink, indent);
        //存储函数对象的数据
        Datum *fund = new Datum();
        fund->type = DATUM_TYPE::FUNCTION;
        fund->u.fun = fun;
        Property *funProp = new Property(fund);
        //函数符号，指向函数对象数据
        Symbol *funSym = new Symbol(SYMBOL_TYPE::PROPERTY, slink);
        funSym->entry.key = nameAtom;
        funSym->entry.value = funProp;
        pushSymbol(funSym, slink);
        //函数形参符号,倒叙的参数链
        for (int i = node->params.size() - 1; i >= 0; i--)
        {
            string param = node->params[i];
            Symbol *sym = new Symbol(SYMBOL_TYPE::ARGUMENT, fun->slink);
            sym->entry.key = funScript->atoms[getAtomIndex(funScript, param)];
            Symbol *next = funScript->args;
            sym->next = next;
            funScript->args = sym;
        }
    }
    break;
    case ASTNodeType::Call:
    {
        emit2(script, OP_TYPE::NAME, getAtomIndex(script, node->text));
        //实参入栈
        for (auto child : node->children)
            generateCode(script, child, slink, indent);
        emit2(script, OP_TYPE::CALL, node->children.size());
    }
    break;
    case ASTNodeType::Binary:
        generateCode(script, node->children[0], slink, indent);
        generateCode(script, node->children[1], slink, indent);
        if (node->text == "+")
            emit1(script, OP_TYPE::ADD);
        else if (node->text == "-")
            emit1(script, OP_TYPE::MINUS);
        else if (node->text == "*")
            emit1(script, OP_TYPE::MUTIL);
        else if (node->text == "/")
            emit1(script, OP_TYPE::DIVID);
        break;
    case ASTNodeType::Return:
        if (node->children.size())
            generateCode(script, node->children[0], slink, indent);
        emit2(script, OP_TYPE::RETURN, node->children.size() ? 1 : 0);
        break;
    case ASTNodeType::AssignmentStmt:
    case ASTNodeType::VarDeclaration:
        generateCode(script, node->children[0], slink, indent);
        emit2(script, OP_TYPE::NAME, getAtomIndex(script, node->text));
        emit1(script, OP_TYPE::ASSIGN);
        break;
    case ASTNodeType::NumberLiteral:
        emit2(script, OP_TYPE::NUMBER, getAtomIndex(script, stoi(node->text)));
        break;
    case ASTNodeType::Identifier:
        emit2(script, OP_TYPE::NAME, getAtomIndex(script, node->text));
        break;
    default:
        break;
    }
}
void emit1(Script *script, OP_TYPE type)
{
    unsigned base = script->length;
    script->code[base + 0] = (uint8_t)type;
    script->length++;
}
void emit2(Script *script, OP_TYPE type, uint8_t op1)
{
    unsigned base = script->length;
    script->code[base + 0] = (uint8_t)type;
    script->code[base + 1] = op1;
    script->length += 2;
}
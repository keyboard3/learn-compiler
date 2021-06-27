#include "models.h"
bool sortNode(ASTNode *a, ASTNode *b);
Atom *getAtomByVal(Context &kc, string val);
Atom *getAtomByVal(Context &kc, int val);
int bytecodeParse(Context &kc, ASTNode *node, string indent)
{
    auto emit = [&](int index, ASTNode *node)
    {
        emit2(kc.cg, node->type == ASTNodeType::IntLiteral ? OP_TYPE::NUMBER : OP_TYPE::NAME, index);
    };
    switch (node->type)
    {
        int lindex, rindex, index;
    case ASTNodeType::Program:
        for (auto child : node->children)
            bytecodeParse(kc, child, indent);
        break;
    case ASTNodeType::Additive:
    case ASTNodeType::Multiplicative:
    {
        lindex = bytecodeParse(kc, node->children[0], indent);
        rindex = bytecodeParse(kc, node->children[1], indent);
        if (lindex != -1)
            emit(lindex, node->children[0]);
        if (rindex != -1)
            emit(rindex, node->children[1]);

        if (node->text == "+")
            emit1(kc.cg, OP_TYPE::ADD);
        else if (node->text == "-")
            emit1(kc.cg, OP_TYPE::MINUS);
        else if (node->text == "*")
            emit1(kc.cg, OP_TYPE::MUTI);
        else if (node->text == "/")
            emit1(kc.cg, OP_TYPE::DIVID);
        break;
    };
    case ASTNodeType::IntLiteral:
        getAtomByVal(kc, stoi(node->text));
        return kc.atoms.size() - 1;
    case ASTNodeType::Identifier:
        getAtomByVal(kc, node->text);
        return kc.atoms.size() - 1;
    case ASTNodeType::AssignmentStmt:
    case ASTNodeType::IntDeclaration:
    {
        index = bytecodeParse(kc, node->children[0], indent);
        if (index >= 0)
            emit(index, node->children[0]);
        Atom *atom = getAtomByVal(kc, node->text);
        emit2(kc.cg, node->type == ASTNodeType::AssignmentStmt ? OP_TYPE::ASSIGN : OP_TYPE::INT_DECL, kc.atoms.size() - 1);
        break;
    }
    }
    return -1;
}
//可以允许重复的atom，空间换时间O(1)找到index
Atom *getAtomByVal(Context &kc, string val)
{
    Atom *atom = generateAtom(val);
    kc.atoms.push_back(atom);
    return atom;
}
Atom *getAtomByVal(Context &kc, int val)
{
    Atom *atom = generateAtom(val);
    kc.atoms.push_back(atom);
    return atom;
}
bool sortNode(ASTNode *a, ASTNode *b)
{
    return b->children.size() - a->children.size();
}
#include "string"
#include "list"
#include "unordered_map"
#include "vector"
using namespace std;
#ifndef models_h
#define models_h
enum class TokenType
{
  DEFULT, //初始化默认值
  EOL,    //行尾
  SEMI,   //语句末尾 ;
  LB,
  RB, //数组标识 []
  LC,
  RC, //块级标识 {}
  LP,
  RP,     //函数声明和调用标识 ()
  COMMA,  //数组或者多个声明标识 ,
  ASSIGN, //赋值符号 =,+=,-=
  HOOK,   //条件判断符号 ?
  COLON,  //三目符号中的:
  OR,
  AND, //逻辑符号||和&&
  BITOR,
  BITXOR,
  BITAND,   //二进制运算|^&
  EQOP,     //等于符号 == !=
  RELOP,    //比较符号 < <= > >=
  SHOP,     //移位符号 << >> >>>
  PLUS,     //加法 +
  MINUS,    //减法 -
  MULOP,    //乘除符号 * / %
  UNARYOP,  //一元前缀符号 !=的! 和~
  INCOP,    //自增和自减符号 ++ --
  DOT,      //成员访问符号 .
  NAME,     //标识符
  NUMBER,   //数字常量
  STRING,   //字符串常量
  PRIMARY,  //true,false,null,this,super
  FUNCTION, //函数关键字 函数定义用
  IF,       //if关键字 bool判断true用
  ELSE,     //else关键字 bool判断false用
  SWITCH,   //switch关键字 匹配值用
  CASE,     //case关键字 每种case对应处理用
  DEFAULT,  //default关键字 没有匹配时用
  WHILE,    //while关键字 while循环判断用
  DO,       //do关键字 do-while循环入口
  FOR,      //for关键字 for循环入口
  BREAK,    //break关键字 循环调出
  CONTINUE, //continue关键字 跳过这次，继续循环下一个
  IN,       //in关键字 for循环取出对象属性用
  VAR,      //var关键字 变量声明
  WITH,     //with关键字 with作用域
  RETURN,   //return关键字 函数返回用
  NEW,      //new关键字 创建对象
  RESERVED, //保留关键字
};
//token定义
class Token
{
public:
  TokenType type;
  string text;
  Token(TokenType type, char chr) : type(type)
  {
    text += chr;
  };
  Token(TokenType type, string chr) : type(type)
  {
    text += chr;
  };
};
//语法树的节点类型
enum class ASTNodeType
{
  Program,
  AssignmentStmt,
  VarDeclaration,
  NumberLiteral,
  Identifier,
  Function,
  // For,
  // DoWhile,
  // While,
  Block,
  Return,
  Binary,
  Unary,
  Primary,
  Call,
};
class ASTNode
{
public:
  ASTNodeType type;
  string text;
  vector<string> params;
  vector<ASTNode *> children;
  ASTNode(ASTNodeType type, string text) : type(type), text(text){};
  void addChild(ASTNode *node);
  void dumpAST(string indent);
};
enum class PrimaryFlag
{
  numberFlag,
  stringFlag,
  boolFlag,
  undefinedFlag,
  methodFlag
};
class TokenStream
{
public:
  list<Token *> &tokens;
  Token *ptr;
  TokenStream(list<Token *> &tokens) : tokens(tokens){};
};
extern unordered_map<string, TokenType> defineKeywords;
string ToString(ASTNodeType v);
typedef unsigned long number;
class Scope;
class Function;
enum class OP_TYPE
{
  NUMBER,
  NAME,
  ADD,
  MINUS,
  MUTIL,
  DIVID,
  ASSIGN,
  RETURN,
  CALL
};
enum class ATOM_TYPE
{
  NAME,
  NUMBER
};
struct Atom
{
  ATOM_TYPE type;
  double fval; //TODO 删除
  number nval;
  string sval;
};
enum class SYMBOL_TYPE
{
  ARGUMENT,
  VARIABLE,
  PROPERTY
};
enum class DATUM_TYPE
{
  UNDEF,
  NUMBER,
  STRING,
  FUNCTION,
  ATOM,
  SYMBOL
};
class Symbol
{
public:
  Symbol(SYMBOL_TYPE type, Scope *scope) : type(type), scope(scope)
  {
    next = nullptr;
  }
  SYMBOL_TYPE type;
  uint8_t slot;
  Symbol *next;
  Scope *scope;
  struct Entry
  {
    Atom *key;
    void *value; //datum | property
  } entry;
};
struct Datum
{
  DATUM_TYPE type;
  struct
  {
    long long int nval;
    string sval;
    Atom *atom;
    Symbol *sym;
    Function *fun;
  } u;
};
class Property
{
public:
  Property(Datum *d)
  {
    datum = d;
    next = nullptr;
  }
  Datum *datum; //属性一般是直接挂到symbol上的
  Property *next;
};
class Scope
{
public:
  Scope(Scope *parent) : parent(parent)
  {
    list = nullptr;
  }
  Symbol *list;    //符号链
  Property *props; //属性链
  Scope *parent;
};
//指令集合
class Script
{
public:
  Script(unsigned codeLimit)
  {
    code = (uint8_t *)malloc(sizeof(uint8_t) * codeLimit);
    args = nullptr;
    length = 0;
  }
  vector<Atom *> atoms; //指令中引用的字面量
  Symbol *args;         //指令的形参符号（主要在函数中）
  uint8_t *code;        //代码
  unsigned length;      //代码长度
};
//函数对象存储是函数的指令集合
class Function
{
public:
  Function(Atom *name, Script *script, Scope *slink) : name(name), script(script), scope(slink) {}
  Atom *name;     //函数名
  Script *script; //指令集合
  Scope *scope;   //函数定义所在的静态作用域
};
//函数执行在栈中的具体体现
struct Frame
{
  unsigned argc;  //实际参数数量
  Datum *argv;    //参数起始地址
  unsigned nvars; //变量数量
  Datum *vars;    //变量起始地址
  Function *fun;  //当前栈帧所指向的函数
  Frame *down;    //上一个栈帧
};
struct Stack
{
  Datum *base;  //数据栈
  Datum *ptr;   //指向最近的数据栈
  Frame *frame; //当前运行的栈帧
};
class Context
{
public:
  Context(int stackSize)
  {
    script = new Script(1000);
    staticLink = nullptr;
    stack.ptr = stack.base = (Datum *)malloc(sizeof(Datum) * stackSize);
    stack.frame = nullptr;
    globalObject = staticLink = new Scope(nullptr);
  }
  Script *script;      //根代码
  Stack stack;         //数据栈
  Scope *staticLink;   //顶层的作用域
  Scope *globalObject; //全局对象，在这里暂时无用
};
Symbol *findSymbol(Scope *scope, Atom *atom);
void pushAtom(Atom *atom, Stack *stack);
void pushNumber(number value, Stack *stack);
unsigned getAtomIndex(Script *script, number val);
unsigned getAtomIndex(Script *script, string val);
Atom *generateAtom(number val);
Datum *popDatum(Stack *stack);
bool resolveSymbol(Context *context, Datum *dp);
bool resolveValue(Context *context, Datum *dp);
Atom *getAtom(Script *script, int index);
string to_op_str(OP_TYPE op);
void dumpScope(Scope *scope);
void pushDatum(Stack *stack, Datum *d);
void pushSymbol(Symbol *sym, Scope *scope);
#endif
#include "string"
#include "list"
#include "unordered_map"
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
  For,
  DoWhile,
  While,
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
  list<ASTNode *> children;
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
struct Method
{
  StackFrame *parentFrame;
  list<ASTNode *> children;
  long call; //函数指针
};
// //解释器推导Primary存的值
class Entry
{
public:
  double numberData;
  string stringData;
  bool boolData;
  Method *methodData;
  PrimaryFlag flag;
  bool isNumber();
  bool isString();
  bool isBool();
  bool isUndefined();
  double getNumber();
  string getString();
  bool getBool();
};
class TokenStream
{
public:
  list<Token *> &tokens;
  Token *ptr;
  TokenStream(list<Token *> &tokens) : tokens(tokens){};
};
//栈帧
class StackFrame
{
public:
  StackFrame *stackStackFrame;
  unordered_map<string, Entry *> localScope;
  unordered_map<string, Entry *> *globalScope;
  StackFrame(StackFrame *stackStackFrame, unordered_map<string, Entry *> *globalScope) : stackStackFrame(stackStackFrame), globalScope(globalScope){};
  Entry *get(string key);
  void declare(string name, Entry *value);
  void set(string name, Entry *value);
};
//执行上下文
class Context
{
public:
  int maxFrameSize;
  list<StackFrame *> frames;
  StackFrame *frame;
  unordered_map<string, Entry *> *globalScope;
  Context(int maxFrameSize) : maxFrameSize(maxFrameSize)
  {
    globalScope = {};
  };
  void pushFrame(StackFrame *frame);
  StackFrame *popFrame();
  Entry *get(string name);
  void set(string name, Entry *value);
  void declare(string name, Entry *value);
};
extern unordered_map<string, TokenType> defineKeywords;
#endif
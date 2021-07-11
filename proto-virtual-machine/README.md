# proto-virtual-machine
JS解释器原型，逐渐完善<br>
字节码指令解释部分，参考自[mocha1995](https://github.com/doodlewind/mocha1995)

## 支持
  - [x] 赋值语句
  - [x] 函数调用
  - [x] 算数表达式
  - [x] 一等公民函数
  - [x] 嵌套函数声明
  - [ ] 对象数据类型
  - [ ] 引用计数垃圾回收
  - [ ] 函数表达式
  - [ ] 闭包
  - [ ] 函数的call和apply方法
  - [ ] 基于原型的继承
  - [ ] 对undefined的全局绑定
  - [ ] try-catch-finally语句
## 前言
- REPL
```
npm i
//进入REPL
npm run ks
//
var c=0;
function add(a,b){ return a+b+c;}
a=add(1,2);
```
- 代码测试

npm run test
```
//test.ts
function getAdd() {
    function add(a,b) {
        return a+b;
    }
    return add;
}
var add = getAdd();
var c = add(1,2);
```

## 支持

- 词法分析
  - 参考[mocha1995](https://github.com/doodlewind/mocha1995)的词法解析
  - 算法逻辑采用了NFA有限自动机
```
LB: [
RB: ] //数组标识
LC: { 
RC: }
LP: ( 
RP: ) //函数声明和调用标识
COMMA: ',' //数组或者多个声明标识 ,
ASSIGN: =,+=,-= //赋值符号 =,+=,-=
HOOK: ? //条件判断符号 ?
COLON: : //三目符号中的:
OR: || 
AND: &&
BITOR: | 
BITXOR: ^
BITAND: &
EQOP: ==,!= //等于符号
RELOP: < <= > >= //比较符号
SHOP: << >> >>> //移位符号
PLUS: +
MINUS: -
MULOP: * / %
UNARYOP: ! ~ //一元前缀符号 !=的! 和~
INCOP: ++ -- //自增和自减符号
DOT: . //成员访问符号 .
NAME: [a-zA-Z]([a-zA-Z_][0-9])* //标识符
NUMBER: [0-9].?[0-9] //数字常量
STRING: [a-zA-Z]([a-zA-Z_][0-9])* //字符串常量
PRIMARY: true,false,null,this,super
//关键字
FUNCTION: function //函数关键字 函数定义用
IF: if //if关键字 bool判断true用
ELSE: else //else关键字 bool判断false用
SWITCH: switch //switch关键字 匹配值用
CASE: case //case关键字 每种case对应处理用
DEFAULT: default //default关键字 没有匹配时用
WHILE: while //while关键字 while循环判断用
DO: do //do关键字 do-while循环入口
FOR: for //for关键字 for循环入口
BREAK: break //break关键字 循环调出
CONTINUE: continue //continue关键字 跳过这次，继续循环下一个
IN: in //in关键字 for循环取出对象属性用
VAR: var //var关键字 变量声明
WITH: with //with关键字 with作用域
RETURN: return //return关键字 函数返回用
NEW: new //new关键字 创建对象
RESERVED: reserved  //保留关键字
```
- 语法分析
  - 依据[EBNF(扩展巴克斯范式)](https://zh.wikipedia.org/wiki/%E6%89%A9%E5%B1%95%E5%B7%B4%E7%A7%91%E6%96%AF%E8%8C%83%E5%BC%8F)的语法规则来实现生成AST
```
stmt : varDecl | assignmentStat | expStmt | returnStmt | functionDecl | callExp ; //语句
functionDecl : Function Identifier '(' Identifier (',' Identifier)* ')' ; //函数定义
block : '{' (stmt)* '}' ; //块语句
varDecl : Var Identifier ('=' exp)? ';' ; //var变量初始化
assignmentStat : Identifier '=' exp ; //变量赋值
returnStmt : Return stmt ; //返回语句
expStmt : exp ';' //表达式语句
unaryExp : [!, ~] pri ; //一元表达式
callExp : Identifier '(' pri (',',pri)* ')' ; //调用表达式
binaryExp : [<<,>>,==,!=,<,<=,>,>=,+,-,*,/] ; //二元表达式
exp : add ; //表达式
add : mul ('+' mul)* ;//加法表达式
mul : pri ('*' pri)* ;//乘法表达式
pri : NumberLiteral | Identifier | '(' exp ')' ; //基础表达式
```
- 运行时（解释执行字节码指令）åå

## 扩展知识
- DFA 确定的有限自动机

状态机在任何一个状态，基于输入的字符，都能做一个确定的状态转换。不会产生回溯

- NFA 不确定的有限自动机

存在某些状态，针对某些输入，不能做一个确定的转换。会回溯

- REPL Read-Eval-Print Loop

读取-求值-输出 循环。简单的交互式编程环境

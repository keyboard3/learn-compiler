# super-tiny-virtual-machine
用c++实现的超简单的虚拟机（c++17编译）
参考自祖师爷mocha的虚拟机实现
## 前言
```
//进入REPL
make cmd&&./cmd.out

//测试 必须以';'结尾
int a =14*2;
int b=(a+2)*(3-1);
```

## 支持
- int变量声明和初始化
- 赋值语句
- 表达式语句


## 原理
- 词法分析
  - 使用DFA(确定的有限自动机)来生成token串
```
词法规则-正则表达式表示
IntLiteral : [0-9]+
Identifier : [a-zA-Z]([a-zA-Z]|[0-9])*
Int : int
GT : >
GE : >=
Assignment : =
Plus : +
Minus : -
Star : *
Slash : /
Equals : ==
LeftParen : (
RightParen : )
SemiColon : ;
```
- 语法分析
  - 依据[EBNF(扩展巴克斯范式)](https://zh.wikipedia.org/wiki/%E6%89%A9%E5%B1%95%E5%B7%B4%E7%A7%91%E6%96%AF%E8%8C%83%E5%BC%8F)的语法规则来实现生成AST
```
非终结符 : 正则表达的语法规则(EBNF);
stmt : varDecl | expStmt ; //语句
varDecl : Int Identifier varInitializer? ';' ; //int变量初始化
varInitializer : '=' exp ; //变量初始化
expStmt : exp ';' //表达式语句
exp : add ; //表达式
add : mul ('+' mul)* ;//加法表达式
mul : pri ('*' pri)* ;//乘法表达式
pri : IntLiteral | Identifier | '(' exp ')' ; //基础表达式
```
- 语义分析
  - 递归展开语法树生成指令，操作数都引用的是上下文中Atom数组中的索引，Atom结构体可以区分出当前是int还是string
- 运行时
  - 取指令，通过取出操作数上的索引，从上下文中拿到Atom，将Atom的值取出根据指令类型进行操作。压栈和取栈以及存入作用域中。
## 扩展知识
- DFA 确定的有限自动机

状态机在任何一个状态，基于输入的字符，都能做一个确定的状态转换。不会产生回溯
- NFA 不确定的有限自动机

存在某些状态，针对某些输入，不能做一个确定的转换。会回溯
- REPL Read-Eval-Print Loop

读取-求值-输出 循环。简单的交互式编程环境
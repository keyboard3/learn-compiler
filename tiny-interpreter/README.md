# tiny-interpreter
用js实现的js解释器的子集

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
var c=0;
function add(a,b){
  return a+b+c;
}
a=add(1,2);
a=add(a,a)
```

## 支持

- 词法解析
  - 参考mocha1995较为完备
  - 除了特殊进制表示的数字字面量和字符串字面量
- 语法解析
  - 手写构建AST，目的支持语法解析
- 语法支持
  - [x] 变量声明
  - [x] 函数声明
  - [x] 函数调用
  - [x] primary表达式
  - [x] return语句
  - [x] 一元表达式
  - [x] 二元表达式
- 运行时
  - [x] 函数调用栈
  - [x] 静态作用域

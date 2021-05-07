# acorn-interpreter-es5

- 基于 acorn 的 parser 用 js 实现 es5 的解释器
- 利用 js 特性，在此基础上实现对 es5 的语法解析

## 前言

- 代码测试

npm run test

```
new Script().process(`
function hello(){
  for(var i=0;i<10;i++){
    console.log(i);
  }
} 
hello();
`)
```

## 支持

- 语法支持
  - [x] 变量声明
  - [x] 函数声明
  - [x] 函数调用
  - [x] 成员表达式
  - [x] 赋值表达式
  - [x] 一元表达式
  - [x] 二元表达式
  - [x] 条件表达式
  - [x] 自增表达式
  - [x] 循环语句
  - [x] new 表达式
  - [x] primary表达式
  - [x] 数组对象
  - [x] 数组字面量
  - [x] 对象字面量
  - [x] try-catch-finally
  - [x] throw 表达式
  - [x] es5 标准库
- 运行时
  - [x] 函数调用栈
  - [x] 静态作用域
  - [x] 闭包

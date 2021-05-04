# acorn-interpreter-es5

- 基于 acorn 的 parser 用 js 实现 es5 的解释器
- 利用 js 特性，在此基础上实现对 es5 的语法解析

## 前言

- 代码测试

npm run test

```
new Script().process(`
function hello(){
  console.log('2')
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
  - [ ] 赋值表达式
  - [x] 一元表达式
  - [ ] 二元表达式
  - [ ] 条件表达式
  - [ ] 循环语句
  - [x] new 表达式
  - [ ] 数组对象
  - [ ] 对象字面量
  - [ ] 数组字面量
  - [x] es5 标准库
- 运行时
  - [x] 函数调用栈
  - [x] 静态作用域
  - [ ] 闭包

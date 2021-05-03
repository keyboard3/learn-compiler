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
//src/test.ts
var c=0;
function add(a,b){
  return a+b+c;
}
a=add(1,2);
a=add(a,a)
```

## 支持

- 词法支持
  - 完整的词法解析(除特殊的进制转换和科学计数法)
- 语法支持
  - 函数声明
  - 函数调用
  - return语句
  - +-*/表达式
  - 变量声明
  - 变量赋值
- 运行时支持
  - 函数调用栈
  - 静态作用域

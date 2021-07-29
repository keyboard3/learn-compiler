## 前言
来源[我的第一个语言前端与 LLVM 教程](https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/index.html)

## 运行
```
g++ -g -std=c++1z toy.cpp `llvm-config --cxxflags`
./a.out
```
## 进度
 - 词法分析
 - 实现语法分析和语法树
    - 抽象语法树
    - 语法解析基础
    - 基础表达式解析
    - 二元表达式解析
    - 解析其余部分：函数定义
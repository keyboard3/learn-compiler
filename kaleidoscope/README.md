## 前言
来源[我的第一个语言前端与 LLVM 教程](https://llvm.org/docs/tutorial/MyFirstLanguageFrontend/index.html)

## 运行步骤
- 准备构建安装 llvm4.0
因为brew已经没有llvm@4了，所以需要我们自己构建
```
git clone -b 4.0 --depth 1 https://github.com/llvm/llvm-project.git
cd llvm-project && cd llvm && mkdir build && cd build
cmake ..
cmake —build . --target install
```
- 解释运行
```
make
./toy
```
## 进度
 - 词法分析
 - 实现语法分析和语法树
    - 抽象语法树
    - 语法解析基础
    - 基础表达式解析
    - 二元表达式解析
    - 解析其余部分：函数定义
-  生成 LLVM IR 代码
   - 表达式代码生成
   - 函数代码生成
- 添加即时编译和代码优化
   - 常见的常量折叠
   - LLVM 优化通道
   - 添加即时编译
- 扩展语言：控制流
   - If/Then/Else
      - 词法扩展
      - AST扩展
      - 语法解析器扩展
      - LLVM IR
      - 代码生成
   - 'for' 循环扩展
      - 词法扩展
      - AST扩展
      - 语法解析器扩展
      - LLVM IR
      - 代码生成
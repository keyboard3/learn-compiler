# learn-compiler
通过JS语言为入口，学习实践编译原理。<br/>
面对JS复杂的语言实现，选择穿越历史迷雾，站在Brendan Eich的视角上，利用现代的思想和知识站来演变这门语言。<br>
理解和实践之间有巨大的鸿沟，如果不去实践理解，那么永远都是纸上谈兵，没有任何战斗力。


## JS 一览
建议阅读 [JavaScript 20年](cn.history.js.org)
| 版本   | 实现(参考)                                           |
| ------ | ---------------------------------------------------- |
| [ES 1.0](https://www.ecma-international.org/wp-content/uploads/ECMA-262_1st_edition_june_1997.pdf) | [macha(JavaScript-1.1)](https://github.com/doodlewind/mocha1995) |
| [ES 2.0](https://www.ecma-international.org/wp-content/uploads/ECMA-262_2nd_edition_august_1998.pdf) | [SpiderMonkey(JavaScript-1.4.2)](https://github.com/Historic-Spidermonkey-Source-Code/JavaScript-1.4.2) |
| [ES 3.0](https://www.ecma-international.org/wp-content/uploads/ECMA-262_3rd_edition_december_1999.pdf) | [SpiderMonkey(JavaScript-1.5.0)](https://github.com/Historic-Spidermonkey-Source-Code/JavaScript-1.5.0)</br>[v8 tag 0.1.5](https://chromium.googlesource.com/v8/v8.git/+/refs/tags/0.1.5) |
| [ES 5.1](https://www.ecma-international.org/wp-content/uploads/ECMA-262_5.1_edition_june_2011.pdf) |  [SpiderMonkey(JavaScript-1.8.5)](https://github.com/Historic-Spidermonkey-Source-Code/JavaScript-1.8.5)</br>[v8 tag 3.2.9](https://chromium.googlesource.com/v8/v8.git/+/refs/tags/3.2.9) </br> [JerryScript](https://github.com/jerryscript-project/jerryscript) </br> [v7](https://github.com/cesanta/v7/) | 
| [ES 2015-ES Next](https://www.ecma-international.org/publications-and-standards/standards/ecma-262/) | [v8-dev](https://v8.dev/)</br>[QuickJS](https://github.com/quickjs-zh/QuickJS)
## 其他推荐
[Mozilla JavaScript 1.1-1.8.5](https://web.archive.org/web/20131113070148/https://developer.mozilla.org/en-US/docs/Web/JavaScript/New_in_JavaScript) </br>
[Exploring JS](https://exploringjs.com/)</br>
[how to read ES](https://timothygu.me/es-howto/#navigating-the-spec) </br>
[v8 changelog](https://chromium.googlesource.com/v8/v8/+/4.3.61/ChangeLog)
## 计划
- [x] 超简单的解释器`js`：[super tiny interpreter](./super-tiny-interpreter)
- [x] 超简单的解释器`c++`：[super tiny interpreter](./cpp-super-tiny-interpreter)
- [x] 简单的js解释器`js`：[tiny interpreter](./tiny-interpreter)
- [x] 简单的js解释器`c++`：[tiny interpreter](./cpp-tiny-interpreter)
- [x] 基于js实现es5语法的解释器：[acorn interpreter es5](./acorn-interpreter-es5)
- [x] 超简单解释器基于栈机解释`js`：[super tiny virtual machine](./super-tiny-virtual-machine)
- [x] 超简单解释器基于栈机解释`c++`：[super tiny virtual machine](./cpp-super-tiny-virtual-machine)
- [x] 简单解释器基于栈机解释`js`：[tiny virtual machine](./tiny-virtual-machine)
- [x] 简单解释器基于栈机解释`c++`：[tiny virtual machine](./cpp-tiny-virtual-machine)
- [ ] LLVM 第一个语言教程 [kaleidoscope](./kaleidoscope)
    - [x] 词法分析
    - [ ] 语法分析和抽象语法树
    - [ ] 代码生成 LLVM IR
    - [ ] 添加 JIT
    - [ ] 扩展支持控制流
    - [ ] 扩展支持用户定义操作符
    - [ ] 扩展支持修改变量
    - [ ] 编译成目标文件
    - [ ] 支持 Debug
- [ ] 基于 babel 实现原型栈机解释器`js`
    - [ ] 基础类型
    - [ ] 算数表达式
    - [ ] 一等公民函数
    - [ ] 嵌套函数声明
    - [ ] 对象数据类型
    - [ ] 函数表达式
    - [ ] 函数的 call 和 apply 方法
    - [ ] 基于原型的继承
    - [ ] 闭包
    - [ ] 对 undefined 的全局绑定
    - [ ] try-catch-finally 语句
- [ ] 跳过语法树基于字节码的原型栈机解释器`c++`
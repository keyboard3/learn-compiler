# learn-compiler
通过JS语言为入口，学习实践编译原理。<br/>
面对JS复杂的语言实现，选择穿越历史迷雾，站在Brendan Eich的视角上，利用现代的思想和知识站来演变这门语言。<br>
理解和实践之间有巨大的鸿沟，如果不去实践理解，那么永远都是纸上谈兵，没有任何战斗力。
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
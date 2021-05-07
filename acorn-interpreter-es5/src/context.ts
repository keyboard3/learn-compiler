import standardMap from './standard';
import { Entry } from "./model";
export class Context {
  constructor(script: Node, maxFrameSize: number) {
    this.script = this.ptr = script;
    this.stack = new Stack(maxFrameSize);
    this.globalScope = new GlobalScope();
  }
  getScope(): Scope {
    if (this.stack.frames.length < 0) return this.globalScope;
    return this.stack.frames[this.stack.ptr] || this.globalScope;
  }
  stack: Stack;
  script: Node;
  ptr: Node;
  globalScope: GlobalScope;
}

export class Stack {
  maxFrameSize: number;
  frames: StackFrame[];
  ptr: number;

  constructor(maxFrameSize: number) {
    this.maxFrameSize = maxFrameSize;
    this.frames = [];
    this.ptr = -1;
  }

  pushFrame(frame: StackFrame) {
    if (this.frames.length + 1 >= this.maxFrameSize) throw Error("stack overflow")
    this.frames.push(frame);
    this.ptr++;
  }

  popFrame() {
    if (this.frames.length - 1 < 0) return;
    this.frames.pop();
    this.ptr--;
  }

  getFrame() {
    return this.frames[this.ptr];
  }
}

export interface Scope {
  get: (name) => Entry;
  set: (name, value) => void;
  declare: (name, value) => Entry;
}

export class GlobalScope implements Scope {
  scope: { [key: string]: Entry };
  standardLib: { [key: string]: Entry };//标准库
  constructor() {
    this.scope = {};
    this.standardLib = standardMap;
  }
  get(name: string) {
    if (this.scope[name]) return this.scope[name];
    else if (this.standardLib[name]) return this.standardLib[name];
    throw new ReferenceError(`${name} is not defined`);
  }
  set(name, value) {
    this.scope[name] = new Entry(value);
  }
  declare = (name, value): Entry => {
    this.scope[name] = new Entry(value);
    return this.scope[name];
  }
}
export class StackFrame implements Scope {
  staticStackFrame: StackFrame;//静态作用域的栈帧
  localScope: { [key: string]: Entry };
  closure: { [key: string]: Entry };
  globalScope: GlobalScope;
  constructor(staticStackFrame: StackFrame, globalScope: GlobalScope) {
    this.staticStackFrame = staticStackFrame;
    this.closure = {};
    this.localScope = {};
    this.globalScope = globalScope;
  }
  get = (name): Entry => {
    if (this.localScope[name]) return this.localScope[name];
    if (this.closure[name]) return this.closure[name];
    if (this.staticStackFrame) {
      //捕获变量
      let entry = this.staticStackFrame.get(name);
      this.closure[name] = entry;
      return entry;
    }
    return this.globalScope.get(name);
  }
  set = (name, value) => {
    if (this.localScope[name]) this.localScope[name] = new Entry(value);
    else if (this.closure[name]) {
      let entry = this.closure[name];
      entry.value = value;
    }
    else this.globalScope.set(name, value);
  }
  declare = (name, value): Entry => {
    this.localScope[name] = new Entry(value);
    return this.localScope[name];
  }
}
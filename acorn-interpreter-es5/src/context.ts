import { Literal } from "./ast";
import standardMap from './standard';

export class Context {
  constructor(script: Node, maxFrameSize: number) {
    this.script = this.ptr = script;
    this.globalScope = new Scope(null);
    this.stack = new Stack(maxFrameSize);
  }
  getScope() {
    if (!this.stack.frames.length) return this.globalScope;
    return this.stack.frames[this.stack.ptr]?.scope || this.globalScope;
  }
  globalScope: Scope;
  stack: Stack;
  script: Node;
  ptr: Node;
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
  getScope() {
    this.frames[this.ptr]
  }
}

export interface StackFrame {
  scope: Scope;
}

export class Scope {
  constructor(parent: Scope) {
    this.parent = parent;
    this.declaration = {};
    this.globalDeclaration = standardMap;
  }

  parent: Scope;
  declaration: { [key: string]: Literal["value"] | undefined | Function };
  globalDeclaration: { [key: string]: Object };
  get(name) {
    if (this.declaration[name]) return this.declaration[name];
    else if (this.parent) return this.parent.get(name);
    else if (this.globalDeclaration[name]) return this.globalDeclaration[name];
    throw new ReferenceError(`${name} is not defined`);
  }
  set(name, value) {
    if (this.declaration[name]) this.declaration[name] = value;
    else if (this.parent) this.parent.set(name, value);
    else if (this.globalDeclaration[name]) this.globalDeclaration[name] = value;
    throw new ReferenceError(`${name} is not defined`);
  }
  declare(name, value) {
    this.declaration[name] = value;
  }
}
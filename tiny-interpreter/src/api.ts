import { K3_Context, K3_Scope, K3_StackFrame, K3_SymbolType } from "./models";

let windowObj = null
let globalObj = null

try {
  windowObj = window
} catch (e) { }

try {
  globalObj = global
} catch (e) { }

const defineSymbols = [
  // Fundamental objects
  { name: "Object", value: Object },
  { name: "Function", value: Function },
  { name: "Boolean", value: Boolean },
  { name: "Symbol", value: Symbol },
  { name: "Error", value: Error },
  { name: "EvalError", value: EvalError },
  { name: "RangeError", value: RangeError },
  { name: "ReferenceError", value: ReferenceError },
  { name: "SyntaxError", value: SyntaxError },
  { name: "TypeError", value: TypeError },
  { name: "URIError", value: URIError },
  // Numbers and dates
  { name: "Number", value: Number },
  { name: "Math", value: Math },
  { name: "Date", value: Date },
  // Text processing
  { name: "Array", value: Array },
  { name: "Int8Array", value: Int8Array },
  { name: "Uint8Array", value: Uint8Array },
  { name: "Uint8ClampedArray", value: Uint8ClampedArray },
  { name: "Int16Array", value: Int16Array },
  { name: "Uint16Array", value: Uint16Array },
  { name: "Int32Array", value: Int32Array },
  { name: "Uint32Array", value: Uint32Array },
  { name: "Float32Array", value: Float32Array },
  { name: "Float64Array", value: Float64Array },
  // Structured data
  { name: "ArrayBuffer", value: ArrayBuffer },
  { name: "DataView", value: DataView },
  { name: "JSON", value: JSON },
  // Other
  { name: "window", value: windowObj },
  { name: "global", value: globalObj },
  { name: "console", value: console },
]
const defineFunctions = [
  // Function properties
  { name: "isFinite", call: isFinite },
  { name: "isNaN", call: isNaN },
  { name: "parseFloat", call: parseFloat },
  { name: "parseInt", call: parseInt },
  { name: "decodeURI", call: decodeURI },
  { name: "decodeURIComponent", call: decodeURIComponent },
  { name: "encodeURI", call: encodeURI },
  { name: "encodeURIComponent", call: encodeURIComponent },
  //Other
  { name: "setTimeout", call: setTimeout },
  { name: "clearTimeout", call: clearTimeout },
  { name: "setInterval", call: setInterval },
  { name: "clearInterval", call: clearInterval },
]
export function initGlobalScope(scope: K3_Scope) {
  defineSymbols.map(item => scope.declareSymbols.set(item.name, { value: item.value, type: K3_SymbolType.SYM_PROPERTY }))
  defineFunctions.map(item => scope.declareFunctions.set(item.name, { call: item.call, parentScope: scope }))
}

export function findSymbolScope(kc: K3_Context, name: string): K3_Scope | null {
  if (!kc.stack.frames.length) return kc.globalScope;
  return findSymbolScope(kc.stack.frames[kc.stack.ptr].scope, name) || kc.globalScope;
  function findSymbolScope(scope: K3_Scope, name: string) {
    if (scope == null) return null;
    if (scope.declareSymbols.has(name)) return scope;
    return findSymbolScope(scope.parent, name);
  }
}
export function findFunScope(kc: K3_Context, name: string): K3_Scope | null {
  if (!kc.stack.frames.length) return kc.globalScope;
  return findFunScope(kc.stack.frames[kc.stack.ptr].scope, name) || kc.globalScope;
  function findFunScope(scope: K3_Scope, name: string) {
    if (scope == null) return null;
    if (scope.declareSymbols.has(name)) return scope;
    return findFunScope(scope.parent, name);
  }
}
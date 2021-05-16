import { Entry } from "./model";

let windowObj = null
let globalObj = null

try {
  windowObj = window
} catch (e) { }

try {
  globalObj = global
} catch (e) { }

const standardMap = {
  // Function properties
  isFinite: new Entry(isFinite),
  isNaN: new Entry(isNaN),
  parseFloat: new Entry(parseFloat),
  parseInt: new Entry(parseInt),
  decodeURI: new Entry(decodeURI),
  decodeURIComponent: new Entry(decodeURIComponent),
  encodeURI: new Entry(encodeURI),
  encodeURIComponent: new Entry(encodeURIComponent),

  // Fundamental objects
  Object: new Entry(Object),
  Function: new Entry(Function),
  Boolean: new Entry(Boolean),
  Symbol: new Entry(Symbol),
  Error: new Entry(Error),
  EvalError: new Entry(EvalError),
  RangeError: new Entry(RangeError),
  ReferenceError: new Entry(ReferenceError),
  SyntaxError: new Entry(SyntaxError),
  TypeError: new Entry(TypeError),
  URIError: new Entry(URIError),

  // Numbers and dates
  Number: new Entry(Number),
  Math: new Entry(Math),
  Date: new Entry(Date),

  // Text processing
  String: new Entry(String),
  RegExp: new Entry(RegExp),

  // Indexed collections
  Array: new Entry(Array),
  Int8Array: new Entry(Int8Array),
  Uint8Array: new Entry(Uint8Array),
  Uint8ClampedArray: new Entry(Uint8ClampedArray),
  Int16Array: new Entry(Int16Array),
  Uint16Array: new Entry(Uint16Array),
  Int32Array: new Entry(Int32Array),
  Uint32Array: new Entry(Uint32Array),
  Float32Array: new Entry(Float32Array),
  Float64Array: new Entry(Float64Array),

  // Structured data
  ArrayBuffer: new Entry(ArrayBuffer),
  DataView: new Entry(DataView),
  JSON: new Entry(JSON),

  // // Other
  window: new Entry(windowObj),
  global: new Entry(globalObj),
  console: new Entry(console),
  setTimeout: new Entry(setTimeout),
  clearTimeout: new Entry(clearTimeout),
  setInterval: new Entry(setInterval),
  clearInterval: new Entry(clearInterval)
}

export default standardMap;
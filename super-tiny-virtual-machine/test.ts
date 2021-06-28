import * as lexer from "./src/lexer";
import * as parser from "./src/parser";
import Bytecode from "./src/bytecode";
import Script from "./src/scripts";

const code: string = `int a=18;int b=a-(1+4)*2;`;

function testLexer(codes: string[]) {
  for (let code of codes) {
    console.log(lexer.tokenize(code))
  }
}

function testParser(codes: string[]) {
  for (let code of codes) {
    parser.parse(code).dumpAST("  ");
  }
}

function testBytecode() {
  let node = parser.parse(code);
  let buf = new Bytecode().parse(node, "");
  console.log(buf.byteLength);
}
function testScript() {
  let node = parser.parse(code);
  let buf = new Bytecode().parse(node, "");
  let script = new Script();
  script.evaluate(buf);
}

// testLexer();
// testParser([code]);
// testBytecode();
testScript();
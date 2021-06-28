import * as lexer from "./src/lexer";
import * as parser from "./src/parser";

const code: string = `int a=18;int a=19+2*(8-2);`;

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

// testLexer();
testParser([code]);

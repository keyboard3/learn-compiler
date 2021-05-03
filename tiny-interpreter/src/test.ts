import * as lexer from "./lexer";
import Script from "./script";
// import * as parser from "./parser";

// const code: string = `int a=18;int a=19+2*(8-2);`;

// function testLexer(codes: string[]) {
//   for (let code of codes) {
//     console.log(lexer.tokenize(code))
//   }
// }

// function testParser(codes: string[]) {
//   for (let code of codes) {
//     parser.parse(code).dumpAST("  ");
//   }
// }

// // testLexer();
// testParser([code]);

// console.log(lexer.tokenize("var a=1+2*3/4%4;"))
// console.log(lexer.tokenize("var a=[1,2,3];"))
// console.log(lexer.tokenize("a[2]=2;"))
// console.log(lexer.tokenize("a={age:4};"))
// console.log(lexer.tokenize("a.age=4.5;"))
// console.log(lexer.tokenize("if(a!=2){}else{}"))
// console.log(lexer.tokenize("for(var item in list){}"))
// console.log(lexer.tokenize("while(true){} do{}while(a==1)"))
// console.log(lexer.tokenize("function hello() {}"))
// console.log(lexer.tokenize("a++,a--,a>>,b>>,a!="))
// console.log(lexer.tokenize("a='dfdf'+2+3"))
// console.log(lexer.tokenize("true?a:b"))
// console.log(lexer.tokenize("switch(a){case:b;break;}"))
// console.log(lexer.tokenize("a=1|2,a=1^2,a=1&2"))
// console.log(lexer.tokenize("a=1||2,a=1&&2"))

const script = new Script();
script.process(`
var a=2;
var b=3;
a = true&&false;
`);

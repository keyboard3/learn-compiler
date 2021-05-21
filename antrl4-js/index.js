import antlr4 from 'antlr4';
import JavaScriptLexer from './JavaScriptLexer.js';
import JavaScriptParser from './JavaScriptParser.js';
import JavaScriptParserVisitor from './JavaScriptParserVisitor.js';

const input = "var a=1;"
const chars = new antlr4.InputStream(input);
const lexer = new JavaScriptLexer(chars);
const tokens = new antlr4.CommonTokenStream(lexer);
const parser = new JavaScriptParser(tokens);
parser.buildParseTrees = true;
const tree = parser.program();

// antlr4.tree.ParseTreeWalker.DEFAULT.walk(new JavaScriptListener(), tree);
tree.accept(new JavaScriptParserVisitor());
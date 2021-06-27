import { createInterface } from 'readline';
import * as parser from "./parser";
import Bytecode from "./bytecode";
import Script from "./scripts";

const script = new Script();
Script.verbose = false;

const readline = createInterface({
  input: process.stdin,
  output: process.stdout,
  prompt: '>'
})
readline.prompt();

readline.on('line', (line) => {
  const lineCode = line.trim();
  if (lineCode == "exit()") {
    console.log('再见!');
    process.exit(0);
    return;
  }
  const node = parser.parse(lineCode);
  if (Script.verbose) {
    node.dumpAST("");
  }
  let buf = new Bytecode().parse(node, "");
  let result = script.evaluate(buf);
  console.log(result)
  // prompt();
}).on('close', () => {
  console.log('再见!');
  process.exit(0);
});

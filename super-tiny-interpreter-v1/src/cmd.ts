import { createInterface } from 'readline';
import * as parser from "./parser";
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
  const tree = parser.parse(lineCode);
  if (Script.verbose) {
    tree.dumpAST("");
  }
  let result = script.evaluate(tree, "");
  console.log(result)
  // prompt();
}).on('close', () => {
  console.log('再见!');
  process.exit(0);
});

import { createInterface } from 'readline';
import Script from "./src/script";

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
  if (lineCode == "context") {
    console.log(script.kc);
    return;
  }

  script.process(lineCode);
  // prompt();
}).on('close', () => {
  console.log('再见!');
  process.exit(0);
});

import { createInterface } from 'readline';
import VitrulMachine from "./src/machine"
const machine = new VitrulMachine();

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
    console.log(machine.context);
    return;
  }

  machine.process(lineCode);
  prompt();
}).on('close', () => {
  console.log('再见!');
  process.exit(0);
});

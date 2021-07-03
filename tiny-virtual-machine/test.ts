import VitrulMachine from "./src/machine"
const machine = new VitrulMachine();
machine.process(`
var a=10-(1+2+(2*4));
`)
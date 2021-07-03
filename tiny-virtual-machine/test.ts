import VitrulMachine from "./src/machine"
const machine = new VitrulMachine();
machine.process(`
function add(a,b) {
    return a+b*2;
}
var c=add(10,10);
`)
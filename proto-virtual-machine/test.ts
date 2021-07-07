import VitrulMachine from "./src/machine"
const machine = new VitrulMachine();
machine.process(`
var a = 1*2;
function add(a,b) {
    return a+b*2;
}
var c= new add(a,10);
`)
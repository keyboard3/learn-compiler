import VitrulMachine from "./src/machine"
const machine = new VitrulMachine();
machine.process(`
function getThis() {
    return this;
}
var c = getThis();
`)
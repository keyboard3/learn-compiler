import VitrulMachine from "./src/machine"
const machine = new VitrulMachine();
machine.process(`
function getAdd() {
    function add(a,b) {
        return a+b;
    }
    return add;
}
var add = getAdd();
var c = add(1,2);
`)
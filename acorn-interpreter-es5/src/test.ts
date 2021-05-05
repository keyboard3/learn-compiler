import Script from "./script";

new Script().process(`
var age=0;
function hello(){
  return function changeAge() {
    age = 2;
  }
} 
var changeAge=hello();
changeAge();
console.log(age);
`)
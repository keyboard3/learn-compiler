const parser = require("@babel/parser");
var result = parser.parse("1.2", {
    sourceType: "module",
    plugins: ["estree"]
})

console.log(result.program.body)
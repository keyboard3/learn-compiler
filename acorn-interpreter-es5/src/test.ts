const acorn = require("acorn");
console.log(acorn.parse("1 + 1", { ecmaVersion: 5 }));
#include "src/machine.h"
#include "iostream"
int main()
{
  Machine *machine = new Machine();
  machine->process("\
  function getAdd() {\
   function add(a,b) {\
     return a+b;\
   }\
   return add;\
  }\
  var outAdd = getAdd();\
  var c=outAdd(1,2);\
");
}
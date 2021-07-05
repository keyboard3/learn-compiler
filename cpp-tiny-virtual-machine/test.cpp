#include "src/machine.h"
#include "iostream"
int main()
{
  Machine *machine = new Machine();
  machine->process("function hello(a,b){return a+10+a;}  var c=hello(2);");
}
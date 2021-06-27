#include "iostream"
#include "./src/virtual-machine.h"
using namespace std;
bool VirtualMachine::verbose = false;
int main()
{
   VirtualMachine machine = VirtualMachine();
   int result = machine.process("int a =14*2;int b=(a+2)*(3-1);");
   cout << result << endl;
   return 0;
}
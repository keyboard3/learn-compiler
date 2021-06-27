#include "iostream"
#include "./src/virtual-machine.h"
using namespace std;
bool VirtualMachine::verbose = false;
int main()
{
   VirtualMachine machine = VirtualMachine();
   int result = machine.process("int a=18;int b=a-(1+4)*2;");
   cout << result << endl;
   return 0;
}
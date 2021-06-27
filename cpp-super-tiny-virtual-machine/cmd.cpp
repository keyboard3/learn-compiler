#include "iostream"
#include "./src/virtual-machine.h"
using namespace std;
bool VirtualMachine::verbose = false;
int main()
{
  VirtualMachine machine = VirtualMachine();
  string inputCodeLine = "";
  cout << ">";
  while (getline(cin, inputCodeLine))
  {
    if (inputCodeLine.find("exit") != string::npos)
      return 0;
    int result = machine.process(inputCodeLine);
    cout << result << endl;
    cout << ">";
  }
  return 0;
}
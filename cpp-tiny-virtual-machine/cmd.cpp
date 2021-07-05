#include "iostream"
#include "src/machine.h"
using namespace std;
int main()
{
  Machine *machine = new Machine();
  string inputCodeLine = "";
  cout << ">";
  while (getline(cin, inputCodeLine))
  {
    if (inputCodeLine.find("exit") != string::npos)
      return 0;
    machine->process(inputCodeLine);
    cout << ">";
  }
  return 0;
}
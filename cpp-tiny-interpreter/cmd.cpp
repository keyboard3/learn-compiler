#include "iostream"
#include "./src/script.h"
using namespace std;
int main()
{
  Script *script = new Script();
  script->verbose = true;
  string inputCodeLine = "";
  cout << ">";
  while (getline(cin, inputCodeLine))
  {
    if (inputCodeLine.find("exit") != string::npos)
      return 0;
    script->process(inputCodeLine);
    cout << ">";
  }
  return 0;
}
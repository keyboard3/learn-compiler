#include "iostream"
#include "scripts.h"
#include "parser.h"
using namespace std;
bool Script::verbose = false;
int main()
{
  Script script = Script();
  string inputCodeLine = "";
  cout<<">";
  while (getline(cin, inputCodeLine))
  {
    if (inputCodeLine.find("exit") != string::npos)
      return 0;
    ASTNode node = parse(inputCodeLine);
    int result = script.evaluate(node, "");
    cout << result << endl;
    cout<<">";
  }
  return 0;
}
#include "iostream"
#include "scripts.h"
#include "parser.h"
using namespace std;
bool Script::verbose = false;
int main()
{
   Script script = Script();
   ASTNode node = parse("int a =14;int b=a;");
   auto result = script.evaluate(node, "");
   cout << result << endl;
   return 0;
}
#include "iostream"
#include "./src/scripts.h"
#include "./src/parser.h"
using namespace std;
bool Script::verbose = false;
int main()
{
   Script script = Script();
   ASTNode node = parse("int a =14;int b=a;");
   node.dumpAST(" ");
   auto result = script.evaluate(node, "");
   cout << result << endl;
   return 0;
}
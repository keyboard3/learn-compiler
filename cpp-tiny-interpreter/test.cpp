#include "lexer.h"
#include "iostream"

int main()
{
  cout << "begin" << endl;
  auto list = tokenize("int a=1;");
  cout << "end" << list.size() << endl;
  return;
}
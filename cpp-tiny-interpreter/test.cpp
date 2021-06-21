#include "lexer.h"
#include "parser.h"
#include "iostream"
int main()
{
  auto tokens = tokenize("int a=13;a+=3;");
  for (auto item : tokens)
    cout << item->text << endl;
  auto node = parser(tokens);
  node->dumpAST(" ");
}
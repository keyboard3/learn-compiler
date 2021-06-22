#include "lexer.h"
#include "parser.h"
#include "iostream"
int main()
{
  auto tokens = tokenize("var a=1*3+4*4");
  for (auto item : tokens)
    cout << "token" << item->text << ":" << (long)(item->type) << endl;
  auto node = parser(tokens);
  node->dumpAST(" ");
}
#include "lexer.h"
#include "parser.h"
#include "iostream"
int main()
{
  auto tokens = tokenize("var a=3*2*4+1+5/4;function hello(){var c=4;}");
  cout << "tokens size:" << tokens.size() << endl;
  for (auto item : tokens)
    cout << "token " << item->text << ":" << (long)(item->type) << endl;
  cout << "parser begin" << endl;
  auto node = parser(tokens);
  cout << "parser end" << endl;
  node->dumpAST(" ");
}
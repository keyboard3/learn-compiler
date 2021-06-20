#include "lexer.h"
#include "iostream"

int main()
{
  auto list = tokenize("int a=13;a+=3;");
  for(auto item:list) cout<<item->text<<endl;
}
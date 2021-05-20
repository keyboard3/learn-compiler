
#include "models.h"

list<Token *> tokenize(string code)
{
  char ch;
  auto skipLine = [&ch, &code]()
  {
    while (!code.empty() && code.front() == '\n')
      code.pop_back();
  };
}
bool isWhitespace(char ch)
{
  return (ch == ' ') || (ch == '\t') || (ch == '\n') || (ch == '\r');
}
bool isalpha(char ch)
{
  return ((ch >= 'a') && (ch <= 'z')) || ((ch >= 'A') && (ch <= 'Z')) || ch == '_';
}
bool isdigit(char ch)
{
  return (ch >= '0') && (ch <= '9');
}
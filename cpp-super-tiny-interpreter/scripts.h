#include "parser.h"
#include "map"

class Script
{
public:
  static bool verbose;
  map<string, int> variables;
  int evaluate(ASTNode &node, string indent);
};

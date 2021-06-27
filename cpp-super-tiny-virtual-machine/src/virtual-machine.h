#include "parser.h"
#include "map"

class VirtualMachine
{
public:
  Context kc;
  static bool verbose;
  map<string, Atom*> globalScope;
  VirtualMachine();
  int process(string code);
};

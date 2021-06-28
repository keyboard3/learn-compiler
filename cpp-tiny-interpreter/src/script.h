#include "models.h"
class Script
{
private:
  Context *kc;

public:
  bool verbose = false;
  Script();
  void process(string code);
  Entry *evaluate(ASTNode *node, string indent);
  Entry *callFunction(Entry *methodEntry, vector<Entry *> params, string indent);
};
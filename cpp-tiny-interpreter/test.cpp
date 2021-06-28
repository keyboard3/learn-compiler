#include "./src/script.h"
#include "iostream"
int main()
{
  Script *script = new Script();
  script->verbose = true;
  script->process("function hello(a) {var c=a+2;return c;}; hello(1);");
}
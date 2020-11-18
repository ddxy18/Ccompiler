//
// Created by dxy on 2020/8/13.
//

#include <fstream>
#include <sstream>

#include "environment.h"
#include "lex/lexer.h"
#include "lex/token.h"
#include "parser/parser.h"

using namespace CCompiler;
using namespace std;

int main(int argc, char **argv) {
  Environment::EnvironmentInit();

  for (int i = 0; i < 100; ++i) {
    ifstream file("/mnt/e/cs_learning/project/CCompiler/test/source.c");
    Parser parser(file);
    parser.Parse();
  }

  return 0;
}
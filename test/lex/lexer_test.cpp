//
// Created by dxy on 2020/8/26.
//

#include "gtest/gtest.h"
#include "lex/lexer.h"

#include "environment.h"
#include "lex/token.h"

using namespace CCompiler;
using namespace std;

class CCompilerEnvironment : public ::testing::Environment {
 public:
  void SetUp() override {
    CCompiler::Environment::EnvironmentInit();
  }
};

[[maybe_unused]] auto c_compiler_env = AddGlobalTestEnvironment(
        new CCompilerEnvironment);

void NextStream(vector<string> tokens, const string &source) {
  Lexer lexer(source);
  Token token;

  while (!(token = lexer.Next()).Empty()) {
    EXPECT_EQ(token.GetToken(), tokens.front());
    tokens.erase(tokens.cbegin());
  }
}

void PeekStream(vector<string> tokens, const string &source) {
  Lexer lexer(source);
  Token token;

  while (!(token = lexer.Peek()).Empty()) {
    EXPECT_EQ(token.GetToken(), tokens.front());
    tokens.erase(tokens.cbegin());
  }
}

TEST(Lexer, LineComment) {
  string source("// Created by dxy on 2020/8/26.\r\n"
                "\r\n"
                "int i = 3 * 5u;  // something\r\n"
                "const char *s = \"abc\";");
  vector<string> tokens{
          {"int", "i", "=", "3", "*", "5u", ";",
                  "const", "char", "*", "s", "=", "\"abc\"", ";"}};

  NextStream(tokens, source);
}

TEST(Lexer, BlockComment) {
  string source("int main() {\r\n"
                "    /* something */"
                "    int i = 0;\r\n"
                "\r\n"
                "    while (i++ < 10);\r\n"
                "}");
  vector<string> tokens{{"int", "main", "(", ")", "{",
                                "int", "i", "=", "0", ";",
                                "while", "(", "i", "++", "<", "10", ")", ";",
                                "}"}};

  NextStream(tokens, source);
}

TEST(Lexer, InvalidToken) {
  string source("int $i;");
  vector<string> tokens{{"int", "i", ";"}};

  NextStream(tokens, source);
}

TEST(Lexer, Peek) {
  vector<string> tokens{{"int", "a", "[", "10", "]", ";"}};

  PeekStream(tokens, string("int a[10];"));
}

TEST(Lexer, Rollback) {
  vector<string> tokens{{"int", "a", ";"}};
  string source("int a;");
  Lexer lexer(source);

  Token token = lexer.Next();
  EXPECT_EQ(token.GetToken(), tokens.front());
  tokens.erase(tokens.cbegin());
  token = lexer.Next();
  EXPECT_EQ(token.GetToken(), tokens.front());
  tokens.erase(tokens.cbegin());
  token = lexer.Next();
  EXPECT_EQ(token.GetToken(), tokens.front());
  tokens.erase(tokens.cbegin());

  lexer.Rollback(token);
  EXPECT_EQ(token.GetToken(), ";");
}
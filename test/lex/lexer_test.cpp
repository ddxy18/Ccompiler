//
// Created by dxy on 2020/8/26.
//

#include "gtest/gtest.h"
#include "lex/lexer.h"

#include "environment.h"

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

void NextStream(queue<string> token_queue, stringbuf buffer) {
  Lexer lexer(buffer);
  Token token;

  while (!(token = lexer.Next()).Empty()) {
    EXPECT_EQ(token.GetToken(), token_queue.front());
    token_queue.pop();
  }
}

void PeekStream(queue<string> token_queue, stringbuf buffer) {
  Lexer lexer(buffer);
  Token token;

  while (!(token = lexer.Peek()).Empty()) {
    EXPECT_EQ(token.GetToken(), token_queue.front());
    token_queue.pop();
  }
}

TEST(Lexer, LineComment) {
  stringbuf buffer("// Created by dxy on 2020/8/26.\r\n"
                   "\r\n"
                   "int i = 3 * 5u;  // something\r\n"
                   "const char *s = \"abc\";");
  queue <string> token_queue{
          {"int", "i", "=", "3", "*", "5u", ";",
                  "const", "char", "*", "s", "=", "\"abc\"", ";"}};

  NextStream(token_queue, std::move(buffer));
}

TEST(Lexer, BlockComment) {
  stringbuf buffer("int main() {\r\n"
                   "    /* something */"
                   "    int i = 0;\r\n"
                   "\r\n"
                   "    while (i++ < 10);\r\n"
                   "}");
  queue <string> token_queue{{"int", "main", "(", ")", "{",
                                     "int", "i", "=", "0", ";",
                                     "while", "(", "i", "++", "<", "10", ")", ";",
                                     "}"}};

  NextStream(token_queue, std::move(buffer));
}

TEST(Lexer, InvalidToken) {
  stringbuf buffer("int $i;");
  queue <string> token_queue{{"int", "i", ";"}};

  NextStream(token_queue, std::move(buffer));
}

TEST(Lexer, Peek) {
  queue <string> token_queue{{"int", "a", "[", "10", "]", ";"}};

  PeekStream(token_queue, stringbuf("int a[10];"));
}
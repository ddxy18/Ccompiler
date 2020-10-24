//
// Created by dxy on 2020/8/14.
//

#include "gtest/gtest.h"
#include "lex/nfa.h"
#include "lex/token.h"

using namespace CCompiler;
using namespace std;

TEST(Nfa, Alternative) {
  Nfa nfa({{"a|b", TokenType::kEmpty}});
  string s = "ab";
  auto begin = s.cbegin(), end = s.cend();

  auto match_end = nfa.NextMatch(begin, end)->second;
  EXPECT_EQ(string(begin, match_end), "a");

  begin = match_end;
  match_end = nfa.NextMatch(begin, end)->second;
  EXPECT_EQ(string(begin, match_end), "b");

  begin = match_end;
  EXPECT_EQ(nfa.NextMatch(begin, end), nullptr);
}

TEST(Nfa, And) {
  Nfa nfa({{"ab", TokenType::kEmpty}});
  string s = "abc";
  auto begin = s.cbegin(), end = s.cend();

  auto match_end = nfa.NextMatch(begin, end)->second;
  EXPECT_EQ(string(begin, match_end), "ab");

  begin = match_end;
  EXPECT_EQ(nfa.NextMatch(begin, end), nullptr);
}

TEST(Nfa, Range) {
  Nfa nfa({{"[a-c]", TokenType::kEmpty}});
  string s = "abc";
  auto begin = s.cbegin(), end = s.cend();

  auto match_end = nfa.NextMatch(begin, end)->second;
  EXPECT_EQ(string(begin, match_end), "a");

  begin = match_end;
  match_end = nfa.NextMatch(begin, end)->second;
  EXPECT_EQ(string(begin, match_end), "b");

  begin = match_end;
  match_end = nfa.NextMatch(begin, end)->second;
  EXPECT_EQ(string(begin, match_end), "c");

  begin = match_end;
  EXPECT_EQ(nfa.NextMatch(begin, end), nullptr);
}

TEST(Nfa, Quantifier_0Or1) {
  Nfa nfa({{"[a-c]?", TokenType::kEmpty}});
  string s = "abc";
  auto begin = s.cbegin(), end = s.cend();

  auto match_end = nfa.NextMatch(begin, end)->second;
  EXPECT_EQ(string(begin, match_end), "a");

  begin = match_end;
  match_end = nfa.NextMatch(begin, end)->second;
  EXPECT_EQ(string(begin, match_end), "b");

  begin = match_end;
  match_end = nfa.NextMatch(begin, end)->second;
  EXPECT_EQ(string(begin, match_end), "c");

  begin = match_end;
  match_end = nfa.NextMatch(begin, end)->second;
  EXPECT_EQ(string(begin, match_end), "");
}

TEST(Nfa, Quantifier_0OrMore) {
  Nfa nfa({{"[a-c]*", TokenType::kEmpty}});
  string s = "abc";
  auto begin = s.cbegin(), end = s.cend();

  auto match_end = nfa.NextMatch(begin, end)->second;
  EXPECT_EQ(string(begin, match_end), "abc");

  begin = match_end;
  match_end = nfa.NextMatch(begin, end)->second;
  EXPECT_EQ(string(begin, match_end), "");
}

TEST(Nfa, Quantifier_1OrMore) {
  Nfa nfa{{{"[a-c]+", TokenType::kEmpty}}};
  string s = "abcd";
  auto begin = s.cbegin(), end = s.cend();

  auto match_end = nfa.NextMatch(begin, end)->second;
  EXPECT_EQ(string(begin, match_end), "abc");

  begin = match_end;
  EXPECT_EQ(nfa.NextMatch(begin, end), nullptr);
}

TEST(Nfa, Quantifier_Exact) {
  Nfa nfa({{"[a-c]{2}", TokenType::kEmpty}});
  string s = "abcd";
  auto begin = s.cbegin(), end = s.cend();

  auto match_end = nfa.NextMatch(begin, end)->second;
  EXPECT_EQ(string(begin, match_end), "ab");

  begin = match_end;
  EXPECT_EQ(nfa.NextMatch(begin, end), nullptr);
}

TEST(Nfa, Quantifier_nOrMore) {
  Nfa nfa({{"[a-c]{2,}", TokenType::kEmpty}});
  string s = "abcd";
  auto begin = s.cbegin(), end = s.cend();

  auto match_end = nfa.NextMatch(begin, end)->second;
  EXPECT_EQ(string(begin, match_end), "abc");

  begin = match_end;
  EXPECT_EQ(nfa.NextMatch(begin, end), nullptr);
}

TEST(Nfa, Quantifier_mTon) {
  Nfa nfa({{"[a-c]{2,4}", TokenType::kEmpty}});
  string s = "abcabd";
  auto begin = s.cbegin(), end = s.cend();

  auto match_end = nfa.NextMatch(begin, end)->second;
  EXPECT_EQ(string(begin, match_end), "abca");

  begin = match_end;
  EXPECT_EQ(nfa.NextMatch(begin, end), nullptr);
}

TEST(Nfa, PassiveGroup) {
  Nfa nfa({{"(?:aa)ab", TokenType::kEmpty}});
  string s = "aaabc";
  auto begin = s.cbegin(), end = s.cend();

  auto match_end = nfa.NextMatch(begin, end)->second;
  EXPECT_EQ(string(begin, match_end), "aaab");

  begin = match_end;
  EXPECT_EQ(nfa.NextMatch(begin, end), nullptr);
}

TEST(Nfa, EscapeCharacter) {
  Nfa nfa({{"\\(a+\\)", TokenType::kEmpty}});
  string s = "(a)";
  auto begin = s.cbegin(), end = s.cend();

  auto match_end = nfa.NextMatch(begin, end)->second;
  EXPECT_EQ(string(begin, match_end), "(a)");

  begin = match_end;
  EXPECT_EQ(nfa.NextMatch(begin, end), nullptr);
}

TEST(Nfa, NotNewLine) {
  Nfa nfa({{"...", TokenType::kEmpty}});
  string s = "(a)";
  auto begin = s.cbegin(), end = s.cend();

  auto match_end = nfa.NextMatch(begin, end)->second;
  EXPECT_EQ(string(begin, match_end), "(a)");

  begin = match_end;
  EXPECT_EQ(nfa.NextMatch(begin, end), nullptr);
}

TEST(Nfa, SpecialPatternInRange) {
  Nfa nfa({{"[\\w]", TokenType::kEmpty}});
  string s = "a1";
  auto begin = s.cbegin(), end = s.cend();

  auto match_end = nfa.NextMatch(begin, end)->second;
  EXPECT_EQ(string(begin, match_end), "a");

  begin = match_end;
  match_end = nfa.NextMatch(begin, end)->second;
  EXPECT_EQ(string(begin, match_end), "1");

  begin = match_end;
  EXPECT_EQ(nfa.NextMatch(begin, end), nullptr);
}

TEST(Nfa, ExceptRange) {
  Nfa nfa({{"[^abc\\d]", TokenType::kEmpty}});
  string s = "d";
  auto begin = s.cbegin(), end = s.cend();

  auto match_end = nfa.NextMatch(begin, end)->second;
  EXPECT_EQ(string(begin, match_end), "d");

  begin = match_end;
  EXPECT_EQ(nfa.NextMatch(begin, end), nullptr);
}

TEST(Nfa, InvalidRegex) {
  Nfa nfa{{{"a|b|", TokenType::kEmpty}}};
  EXPECT_TRUE(nfa.Empty());
}

TEST(Nfa, MultiRegex) {
  Nfa nfa({{"while",                  TokenType::kWhile},
           {"[a-zA-Z_][a-zA-Z0-9_]*", TokenType::kIdentifier}});

  string s = "while";
  auto begin = s.cbegin(), end = s.cend();

  auto match = nfa.NextMatch(begin, end);
  EXPECT_EQ(string(begin, match->second), "while");
  EXPECT_EQ(match->first, TokenType::kWhile);

  begin = match->second;
  EXPECT_EQ(nfa.NextMatch(begin, end), nullptr);

  s = "while1";
  begin = s.cbegin(), end = s.cend();

  match = nfa.NextMatch(begin, end);
  EXPECT_EQ(string(begin, match->second), "while1");
  EXPECT_EQ(match->first, TokenType::kIdentifier);

  begin = match->second;
  EXPECT_EQ(nfa.NextMatch(begin, end), nullptr);
}
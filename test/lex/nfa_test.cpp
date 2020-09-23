//
// Created by dxy on 2020/8/14.
//

#include "gtest/gtest.h"
#include "lex/nfa.h"

using namespace CCompiler;
using namespace std;

TEST(Nfa, Alternative) {
    Nfa nfa{{{"a|b", -1}}};
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
    Nfa nfa{{{"ab", -1}}};
    string s = "abc";
    auto begin = s.cbegin(), end = s.cend();

    auto match_end = nfa.NextMatch(begin, end)->second;
    EXPECT_EQ(string(begin, match_end), "ab");

    begin = match_end;
    EXPECT_EQ(nfa.NextMatch(begin, end), nullptr);
}

TEST(Nfa, Range) {
    Nfa nfa{{{"[a-c]", -1}}};
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
    Nfa nfa{{{"[a-c]?", -1}}};
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
    Nfa nfa{{{"[a-c]*", -1}}};
    string s = "abc";
    auto begin = s.cbegin(), end = s.cend();

    auto match_end = nfa.NextMatch(begin, end)->second;
    EXPECT_EQ(string(begin, match_end), "abc");

    begin = match_end;
    match_end = nfa.NextMatch(begin, end)->second;
    EXPECT_EQ(string(begin, match_end), "");
}

TEST(Nfa, Quantifier_1OrMore) {
    Nfa nfa{{{"[a-c]+", -1}}};
    string s = "abcd";
    auto begin = s.cbegin(), end = s.cend();

    auto match_end = nfa.NextMatch(begin, end)->second;
    EXPECT_EQ(string(begin, match_end), "abc");

    begin = match_end;
    EXPECT_EQ(nfa.NextMatch(begin, end), nullptr);
}

TEST(Nfa, Quantifier_Exact) {
    Nfa nfa{{{"[a-c]{2}", -1}}};
    string s = "abcd";
    auto begin = s.cbegin(), end = s.cend();

    auto match_end = nfa.NextMatch(begin, end)->second;
    EXPECT_EQ(string(begin, match_end), "ab");

    begin = match_end;
    EXPECT_EQ(nfa.NextMatch(begin, end), nullptr);
}

TEST(Nfa, Quantifier_nOrMore) {
    Nfa nfa{{{"[a-c]{2,}", -1}}};
    string s = "abcd";
    auto begin = s.cbegin(), end = s.cend();

    auto match_end = nfa.NextMatch(begin, end)->second;
    EXPECT_EQ(string(begin, match_end), "abc");

    begin = match_end;
    EXPECT_EQ(nfa.NextMatch(begin, end), nullptr);
}

TEST(Nfa, Quantifier_mTon) {
    Nfa nfa{{{"[a-c]{2,4}", -1}}};
    string s = "abcabd";
    auto begin = s.cbegin(), end = s.cend();

    auto match_end = nfa.NextMatch(begin, end)->second;
    EXPECT_EQ(string(begin, match_end), "abca");

    begin = match_end;
    EXPECT_EQ(nfa.NextMatch(begin, end), nullptr);
}

TEST(Nfa, PassiveGroup) {
    Nfa nfa{{{"(?:aa)ab", -1}}};
    string s = "aaabc";
    auto begin = s.cbegin(), end = s.cend();

    auto match_end = nfa.NextMatch(begin, end)->second;
    EXPECT_EQ(string(begin, match_end), "aaab");

    begin = match_end;
    EXPECT_EQ(nfa.NextMatch(begin, end), nullptr);
}

TEST(Nfa, EscapeCharacter) {
    Nfa nfa{{{"\\(a+\\)", -1}}};
    string s = "(a)";
    auto begin = s.cbegin(), end = s.cend();

    auto match_end = nfa.NextMatch(begin, end)->second;
    EXPECT_EQ(string(begin, match_end), "(a)");

    begin = match_end;
    EXPECT_EQ(nfa.NextMatch(begin, end), nullptr);
}

TEST(Nfa, NotNewLine) {
    Nfa nfa{{{"...", -1}}};
    string s = "(a)";
    auto begin = s.cbegin(), end = s.cend();

    auto match_end = nfa.NextMatch(begin, end)->second;
    EXPECT_EQ(string(begin, match_end), "(a)");

    begin = match_end;
    EXPECT_EQ(nfa.NextMatch(begin, end), nullptr);
}

TEST(Nfa, SpecialPatternInRange) {
    Nfa nfa{{{"[\\w]", -1}}};
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
    Nfa nfa{{{"[^abc\\d]", -1}}};
    string s = "d";
    auto begin = s.cbegin(), end = s.cend();

    auto match_end = nfa.NextMatch(begin, end)->second;
    EXPECT_EQ(string(begin, match_end), "d");

    begin = match_end;
    EXPECT_EQ(nfa.NextMatch(begin, end), nullptr);
}

TEST(Nfa, InvalidRegex) {
    Nfa nfa{{{"a|b|", -1}}};
    EXPECT_TRUE(nfa.Empty());
}

TEST(Nfa, MultiRegex) {
    Nfa nfa{{{"while", 1}, {"[a-zA-Z_][a-zA-Z0-9_]*", 2}}};

    string s = "while";
    auto begin = s.cbegin(), end = s.cend();

    auto match = nfa.NextMatch(begin, end);
    EXPECT_EQ(string(begin, match->second), "while");
    EXPECT_EQ(match->first, 1);

    begin = match->second;
    EXPECT_EQ(nfa.NextMatch(begin, end), nullptr);

    s = "while1";
    begin = s.cbegin(), end = s.cend();

    match = nfa.NextMatch(begin, end);
    EXPECT_EQ(string(begin, match->second), "while1");
    EXPECT_EQ(match->first, 2);

    begin = match->second;
    EXPECT_EQ(nfa.NextMatch(begin, end), nullptr);
}
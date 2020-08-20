//
// Created by dxy on 2020/8/14.
//

#include "lex/nfa.h"

#include <fstream>

#include "gtest/gtest.h"

using namespace Ccompiler;
using namespace std;

TEST(CcompilerTest, NfaConstructor_1) {
    Nfa nfa("a|b*c", "test", 0);

    string s1 = "abbce", s2 = "acde", s3 = "abbd";

    auto begin1 = s1.cbegin(), end1 = s1.cend();
    EXPECT_EQ(nfa.NextToken(begin1, end1).GetToken(), "a");

    auto begin2 = s2.cbegin(), end2 = s2.cend();
    EXPECT_EQ(nfa.NextToken(begin2, end2).GetToken(), "a");

    auto begin3 = s3.cbegin(), end3 = s3.cend();
    EXPECT_EQ(nfa.NextToken(begin3, end3).GetToken(), "a");
}

TEST(CcompilerTest, NfaConstructor_2) {
    Nfa number_nfa("[0-9]*", "number", 0);
    Nfa id_nfa("[a-zA-Z][A-Za-z0-9_]*", "id", 1);
    Nfa nfa = Nfa(vector<Nfa>{number_nfa, id_nfa});

    string s = "a123";

    auto begin = s.cbegin(), end = s.cend();
    EXPECT_EQ(nfa.NextToken(begin, end).GetToken(), "a123");
}

TEST(CcompilerTest, NfaConstructor_3) {
    Nfa nfa(R"(a(\n|\t|\b)*)", "delim", 0);

    string s1 = "\b\ba123", s2 = "a\n\ta", s3 = "15.90";

    auto begin1 = s1.cbegin(), end1 = s1.cend();
    EXPECT_EQ(nfa.NextToken(begin1, end1).GetToken(), "");

    auto begin2 = s2.cbegin(), end2 = s2.cend();
    EXPECT_EQ(nfa.NextToken(begin2, end2).GetToken(), "a\n\t");

    auto begin3 = s3.cbegin(), end3 = s3.cend();
    EXPECT_EQ(nfa.NextToken(begin3, end3).GetToken(), "");
}

TEST(CcompilerTest, NfaConstructor_4) {
    Nfa nfa(R"(\(|\)|\*)", "test", 0);

    string s = "(*)";

    auto begin = s.cbegin(), end = s.cend();
    EXPECT_EQ(nfa.NextToken(begin, end).GetToken(), "(");
    EXPECT_EQ(nfa.NextToken(begin, end).GetToken(), "*");
    EXPECT_EQ(nfa.NextToken(begin, end).GetToken(), ")");
}

TEST(CcompilerTest, NfaConstructor_5) {
    Nfa nfa("[a-zA-Z][a-zA-Z0-9_]*", "id", 0);

    string s = "string int";

    auto begin = s.cbegin(), end = s.cend();
    EXPECT_EQ(nfa.NextToken(begin, end).GetToken(), "string");
    begin++;
    EXPECT_EQ(nfa.NextToken(begin, end).GetToken(), "int");
}

TEST(CcompilerTest, WriteNfa_1){
    Nfa("(a|b)*c","test",0).WriteNfa(
            "/mnt/e/cs_learning/project/Ccompiler/test/lex/lex1.txt");
    Nfa nfa=Nfa::ReadNfa("/mnt/e/cs_learning/project/Ccompiler/test/lex/lex1"
                       ".txt");

    string s="ababca";
    auto begin=s.cbegin(),end=s.cend();
    EXPECT_EQ(nfa.NextToken(begin,end).GetToken(),"ababc");
}

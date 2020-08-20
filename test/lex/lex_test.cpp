//
// Created by dxy on 2020/8/14.
//

#include "lex/lex.h"

#include "gtest/gtest.h"

using namespace Ccompiler;
using namespace std;

TEST(Ccompiler, NfaInit_1) {
    Nfa nfa = NfaInit("/mnt/e/cs_learning/project/Ccompiler/test/lex/lex.txt");

    string s = "19\nstring";
    auto begin = s.cbegin(), end = s.cend();
    EXPECT_EQ(nfa.NextToken(begin, end).GetToken(), "19");
    EXPECT_EQ(nfa.NextToken(begin, end).GetToken(), "\n");
    EXPECT_EQ(nfa.NextToken(begin, end).GetToken(), "string");
}

TEST(Ccompiler, NfaInit_2) {
    Nfa nfa = NfaInit("/mnt/e/cs_learning/project/Ccompiler/test/lex/lex.txt");

    string s = "if ife else";
    auto begin = s.cbegin(), end = s.cend();
    TokenPtr token;

    token = make_unique<Token>(nfa.NextToken(begin, end));
    EXPECT_EQ(token->GetToken(), "if");
    EXPECT_EQ(token->GetType(), "if");

    begin++;
    token = make_unique<Token>(nfa.NextToken(begin, end));
    EXPECT_EQ(token->GetToken(), "ife");
    EXPECT_EQ(token->GetType(), "identifier");

    begin++;
    token = make_unique<Token>(nfa.NextToken(begin, end));
    EXPECT_EQ(token->GetToken(), "else");
    EXPECT_EQ(token->GetType(), "else");
}

TEST(Ccompiler, NfaInit_3) {
    Nfa nfa = NfaInit("/mnt/e/cs_learning/project/Ccompiler/test/lex/lex.txt");

    string s;
    vector<pair<string, string>> token_vec;
    ifstream in("/mnt/e/cs_learning/project/Ccompiler/temp/temp.cpp");
    if (in) {
        while (getline(in, s)) {
            auto begin = s.cbegin(), end = s.cend();
            while (begin != end) {
                Token token = nfa.NextToken(begin, end);
                token_vec.emplace_back(token.GetToken(), token.GetType());
            }
        }
    }

    vector<pair<string, string>> v{
            {"//",        "comment"},
            {"\r",        "delim"},
            {"//",        "comment"},
            {" ",         "delim"},
            {"Created",   "identifier"},
            {" ",         "delim"},
            {"by",        "identifier"},
            {" ",         "delim"},
            {"dxy",       "identifier"},
            {" ",         "delim"},
            {"on",        "identifier"},
            {" ",         "delim"},
            {"2020",      "number"},
            {"/",         "operator"},
            {"8",         "number"},
            {"/",         "operator"},
            {"13",        "number"},
            {".",         "operator"},
            {"\r",        "delim"},
            {"//",        "comment"},
            {"\r",        "delim"},
            {"\r",        "delim"},
            {"#",         "punctuator"},
            {"include",   "identifier"},
            {" ",         "delim"},
            {"<",         "operator"},
            {"iostream",  "identifier"},
            {">",         "operator"},
            {"\r",        "delim"},
            {"\r",        "delim"},
            {"using",     "identifier"},
            {" ",         "delim"},
            {"namespace", "identifier"},
            {" ",         "delim"},
            {"std",       "identifier"},
            {";",         "punctuator"},
            {"\r",        "delim"},
            {"\r",        "delim"},
            {"int",       "int"},
            {" ",         "delim"},
            {"main",      "identifier"},
            {"(",         "punctuator"},
            {")",         "punctuator"},
            {" ",         "delim"},
            {"{",         "punctuator"},
            {"\r",        "delim"},
            {"    ",      "delim"},
            {"cout",      "identifier"},
            {" ",         "delim"},
            {"<<",        "operator"},
            {" ",         "delim"},
            {"\"",        "punctuator"},
            {"Hello",     "identifier"},
            {" ",         "delim"},
            {"World",     "identifier"},
            {"!",         "operator"},
            {"\"",        "punctuator"},
            {";",         "punctuator"},
            {"\r",        "delim"},
            {"    ",      "delim"},
            {"return",    "return"},
            {" ",         "delim"},
            {"0",         "number"},
            {";",         "punctuator"},
            {"\r",        "delim"},
            {"}",         "punctuator"},
            {"\r",        "delim"}
    };

    for (int i = 0; i < token_vec.size(); ++i) {
        EXPECT_EQ(token_vec[i], v[i]);
    }
}

TEST(Ccompiler, NfaInit_4) {
    Nfa nfa = NfaInit("/mnt/e/cs_learning/project/Ccompiler/test/lex/lex.txt");
    string s = "\r\n";

    auto begin = s.cbegin(), end = s.cend();
    Token token = nfa.NextToken(begin, end);
    EXPECT_EQ(token.GetToken(), "\r\n");
    EXPECT_EQ(token.GetType(), "delim");
}

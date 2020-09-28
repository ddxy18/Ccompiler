//
// Created by dxy on 2020/8/25.
//

#include "environment.h"

#include <sstream>

#include "lex/lexer.h"

using namespace CCompiler;
using namespace std;

map<string, int> Environment::symbol_map_;

Environment::Environment(const string &lex_file) {
    int i = 0;  // record symbol type in integer
    map<string, int> regex_rules;

    ifstream lex_stream(lex_file);
    if (!lex_stream) {
        exit(-1);
    } else {  // read all user-defined terminal symbols
        string line, symbol, rule;

        while (getline(lex_stream, line)) {
            istringstream str(line);
            str >> symbol;
            // Lex file must use \n as line separator.
            getline(str, rule, '\n');
            rule.erase(rule.cbegin());  // erase ' '
            regex_rules.emplace(rule, i);
            symbol_map_.emplace(symbol, i++);
        }

        lex_stream.close();
    }

    Lexer::NfaInit(regex_rules);  // initialize NFA in Lexer
}

int Environment::IntSymbol(const string &symbol) {
    auto it = symbol_map_.find(symbol);
    if (it != symbol_map_.cend()) {
        return it->second;
    }
    return -1;
}

string Environment::StrSymbol(int symbol) {
    return std::find_if(symbol_map_.cbegin(),
                        symbol_map_.cend(),
                        [symbol](const auto &p) {
                            return p.second == symbol;
                        })->first;
}
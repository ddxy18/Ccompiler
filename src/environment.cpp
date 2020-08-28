//
// Created by dxy on 2020/8/25.
//

#include "environment.h"

#include <sstream>

#include "lex/lexer.h"
#include "parser/parser.h"

using namespace CCompiler;
using namespace std;

map<string, int> Environment::symbol_map_;

Environment::Environment(string lex_file, string nfa_file,
                         string grammar_file) :
        lex_file_(std::move(lex_file)), nfa_file_(std::move(nfa_file)),
        grammar_file_(std::move(grammar_file)) {
    // detect whether 'lex_file_' exists
    fstream lex_stream(lex_file_);
    if (!lex_stream) {
        exit(-1);
    }
    lex_stream.close();
    // detect whether 'grammar_file_' exists
    fstream grammar_stream(lex_file_);
    if (!grammar_stream) {
        exit(-1);
    }
    grammar_stream.close();

    SymbolMapInit();  // initialize symbol map

    // initialize NFA in Lexer
    ifstream nfa_stream(nfa_file_);
    if (!nfa_stream) {
        // Create a new NFA using 'lex_file_' if it does not exist.
        CreateNfa(lex_file_).WriteNfa(nfa_file_);
    } else {
        nfa_stream.close();
    }
    Lexer::NfaInit(nfa_file_);

    // initialize 'grammar_map_' in Parser
    Parser::GrammarMapInit(grammar_file_);
}

void Environment::SymbolMapInit() {
    ifstream lex_stream(lex_file_);
    string line;
    string symbol;
    int i = 2;

    // initialize specific terminal symbols
    symbol_map_.insert({"error", -1});
    symbol_map_.insert({"empty", 0});
    symbol_map_.insert({"end", 1});

    // read all user-defined terminal symbols
    // 'lex_file_' is guaranteed to be a valid file so we can use it directly.
    while (getline(lex_stream, line)) {
        istringstream str(line);
        str >> symbol;
        symbol_map_.insert({symbol, i++});
    }
    lex_stream.close();

    // read all non-terminal symbols, say, all productions' left side
    // 'grammar_file_' is guaranteed to be a valid file so we can use it directly.
    ifstream grammar_stream(grammar_file_);
    string word;
    while (grammar_stream >> word) {
        if (*(word.cend() - 1) == ':') {
            symbol = string(word.cbegin(), word.cend() - 1);
        }
        symbol_map_.insert({symbol, i++});
    }
}

int Environment::IntSymbol(const string &symbol) {
    auto it = symbol_map_.find(symbol);
    if (it != symbol_map_.cend()) {
        return it->second;
    }
    return kError;
}

string Environment::StrSymbol(int symbol) {
    return std::find_if(symbol_map_.cbegin(),
                        symbol_map_.cend(),
                        [symbol](const auto &p) {
                            return p.second == symbol;
                        })->first;
}

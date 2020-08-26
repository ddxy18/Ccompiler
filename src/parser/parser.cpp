//
// Created by dxy on 2020/8/22.
//

#include "parser/parser.h"

#include <set>

#include "environment.h"

using namespace Ccompiler;
using namespace std;

std::map<int, std::vector<std::vector<int>>> Parser::grammar_map_;

AstNodePtr LL1Parser::NewNode(int symbol) {
    auto rule = grammar_map_.find(symbol);
    static Token token = lexer_.NextToken();

    if (rule->second.empty()) {  // terminal symbol
        if (symbol != kEmpty) {
            auto tmp = token;
            if (token.IsEmptyToken() || token.GetType() != symbol) {
                exit(-1);
            }
            token = lexer_.NextToken();
            return make_shared<AstNode>(tmp.GetToken(), symbol);
        }
        return make_shared<AstNode>("", symbol);
    } else {  // non-terminal symbol
        AstNodePtr cur_node = make_shared<AstNode>("", symbol);
        // choose a production
        vector<int> production;
        if (token.IsEmptyToken()) {
            production = grammar_map_[symbol][predict_table_[symbol][kEnd]];
        } else {
            if (predict_table_[symbol][token.GetType()] == kError) {
                exit(-1);
            }
            production = grammar_map_[symbol][predict_table_[symbol][token.GetType()]];
        }

        // match symbols in the chosen production in order
        for (auto &s:production) {
            cur_node->AddNode(NewNode(s));
        }
        return cur_node;
    }
}

void Parser::GrammarMapInit(const string &grammar_file) {
    ifstream grammar_stream(grammar_file);

    if (grammar_stream) {
        string word;
        int left;  // production's left side

        // parse a word in the file to initialize 'grammar_map_'
        while (grammar_stream >> word) {
            if (*(word.cend() - 1) == ':') {  // new production
                left = Environment::IntSymbol(
                        string(word.cbegin(), word.cend() - 1));
                grammar_map_[left] = vector<vector<int>>();
                grammar_map_[left].push_back(vector<int>());
            } else if (word == "|") {
                grammar_map_[left].push_back(vector<int>());
            } else if ((*word.cbegin() == '\'' && *(word.cend() - 1) == '\'') ||
                       (*word.cbegin() == '\"' && *(word.cend() - 1) == '\"')) {
                // literal string
                grammar_map_[left][grammar_map_[left].size() - 1]
                        .push_back(Environment::IntSymbol(
                                string(word.cbegin() + 1, word.cend() - 1)));
            } else if (word == ";") {  // end of a production
                continue;
            } else if (word == "%empty") {  // empty production
                grammar_map_[left][grammar_map_[left].size() - 1].push_back(
                        kEmpty);
            } else {  // a symbol in a production's right side
                grammar_map_[left][grammar_map_[left].size() - 1]
                        .push_back(Environment::IntSymbol(word));
            }
        }

        grammar_stream.close();
    }
}

set<int> LL1Parser::First(int symbol) {
    set<int> first;
    bool empty_flag = false;

    if (IsTerminalSymbol(symbol)) {
        first.insert(symbol);
    } else {
        for (auto &production:grammar_map_[symbol]) {
            int i = 0;
            for (; i < production.size(); i++) {
                for (auto &f_symbol:First(production[i])) {
                    first.insert(f_symbol);
                }
                if (!first.contains(kEmpty)) {
                    break;
                }
                first.erase(kEmpty);
            }
            if (i == production.size()) {
                empty_flag = true;
            }
        }
        if (empty_flag) {
            first.insert(kEmpty);
        }
    }

    return first;
}

set<int> LL1Parser::First(vector<int> symbols) {
    set<int> first;
    int i = 0;

    for (; i < symbols.size(); ++i) {
        for (auto &f_symbol:First(symbols[i])) {
            first.insert(f_symbol);
        }
        if (!first.contains(kEmpty)) {
            break;
        }
        first.erase(kEmpty);
    }
    if (i == symbols.size()) {
        first.insert(kEmpty);
    }

    return first;
}

map<int, set<int>> LL1Parser::Follow() {
    map<int, set<int>> follow;
    // record how many symbols stored in 'follow'
    int cur_follow_size = 0, last_follow_size = 0;

    follow[start_symbol_].insert(kEnd);  // add '$' to set of start symbol
    cur_follow_size++;
    while (cur_follow_size != last_follow_size) {
        for (auto &pair:grammar_map_) {
            for (auto &production:pair.second) {
                for (int i = 0; i < production.size(); ++i) {
                    if (!IsTerminalSymbol(production[i])) {
                        // copy symbols after 'production[i]' in 'production'
                        vector<int> v;
                        for (int j = i + 1; j < production.size(); ++j) {
                            v.push_back(production[j]);
                        }
                        // add symbols in first(symbols after production[i])
                        // except the empty symbol to follow(production[i])
                        auto first_vec = First(v);
                        first_vec.erase(kEmpty);
                        for (auto &e:first_vec) {
                            follow[production[i]].insert(e);
                        }
                    }
                }

                for (int i = (int) production.size() - 1; i >= 0; --i) {
                    if (IsTerminalSymbol(production[i])) {
                        break;
                    }
                    vector<int> v;
                    for (int j = i + 1; j < production.size(); ++j) {
                        v.push_back(production[j]);
                    }
                    if (First(v).contains(kEmpty)) {
                        for (auto &e:follow[pair.first]) {
                            follow[production[i]].insert(e);
                        }
                    } else {
                        break;
                    }
                }
            }
        }

        last_follow_size = cur_follow_size;
        cur_follow_size = 0;
        for (auto &pair:follow) {
            cur_follow_size += pair.second.size();
        }
    }

    return follow;
}

void LL1Parser::PredictTableInit() {
    auto follow = Follow();

    for (auto &pair:grammar_map_) {
        for (int i = 0; i < pair.second.size(); ++i) {
            auto production = pair.second[i];
            auto first_set = First(production);
            for (auto terminal_symbol:first_set) {
                predict_table_[pair.first][terminal_symbol] = i;
            }
            if (find(first_set.cbegin(), first_set.cend(), kEmpty) !=
                first_set.cend()) {
                auto follow_set = follow[pair.first];
                for (auto &terminal_symbol:follow_set) {
                    predict_table_[pair.first][terminal_symbol] = i;
                }
                if (find(follow_set.cbegin(), follow_set.cend(), kEnd) !=
                    follow_set.cend()) {
                    predict_table_[pair.first][kEnd] = i;
                }
            }
        }
    }
    for (auto &pair:predict_table_) {
        pair.second.erase(kEmpty);
    }
}

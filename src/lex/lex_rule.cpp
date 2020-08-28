//
// Created by dxy on 2020/8/13.
//

#include "lex/lex_rule.h"

#include <sstream>
#include <string>
#include <vector>

#include "environment.h"

using namespace CCompiler;
using namespace std;

// TODO(dxy): realise it
/**
 * Split an alphabet table to blocks according to the given several
 * characters. So when we create a NFA, we can use a block as an input type
 * to reduce NFA state numbers.
 *
 * @param characters All character expressions extracted from a given regex.
 * @return Every element in the vector stores a continuous character range.
 */
vector<pair<int, int>> SplitAlphabetTable(
        vector<string> characters, const string &char_ranges);

//vector<string> LexRule::GetCharaters() {
//    vector<string> str_vec;
//    auto begin = regex_.cbegin(), tmp = begin;
//    while (begin != regex_.cend()) {
//        switch (*begin) {
//            case '\\':  // escaped characters
//                begin++;
//                if (begin++ != regex_.cend()) {
//                    str_vec.emplace_back(begin - 2, begin);
//                }
//                break;
//            case '[':  // [...]
//                tmp = begin;
//                while (++begin != regex_.cend()) {
//                    if (*begin == ']' && *(begin - 1) != '\\') {
//                        str_vec.emplace_back(tmp, ++begin);
//                        break;
//                    }
//                }
//                break;
//            case '{':  // skip '{...}'
//                tmp = begin;
//                while (++begin != regex_.cend()) {
//                    if (*begin == '}' && *(begin - 1) != '\\') {
//                        begin++;
//                        break;
//                    }
//                }
//                break;
//            case '(':
//            case ')':
//            case '.':
//            case '?':
//            case '*':
//            case '+':
//            case '^':
//            case '$':
//            case '|':
//                begin++;
//                break;
//            default:
//                str_vec.emplace_back(begin, begin + 1);
//                begin++;
//        }
//    }
//    return str_vec;
//}

Nfa CCompiler::CreateNfa(const string &lex_file_name) {
    ifstream lex_file_stream(lex_file_name);
    string line;

    if (lex_file_stream) {
        string regex_name, regex_rule;
        vector<LexRule> lex_rules;
        int sequence = 0;  // record regex sequences in lex files

        // read regex rules
        while (getline(lex_file_stream, line)) {
            istringstream line_stream(line);
            line_stream >> regex_name;
            getline(line_stream, regex_rule);
            regex_rule.erase(regex_rule.cbegin());
            // use sequences in lex files to determine regex rule's priority
            lex_rules.emplace_back(sequence++, regex_name, regex_rule);
        }

        // TODO(dxy): parse regex actions, see TODO in lex/lex.h line 42

        Nfa::CharRangesInit(vector<string>());
        // build NFA separately for all 'LexRule' objects
        vector<Nfa> regex_nfa;
        regex_nfa.reserve(lex_rules.size());
        for (auto &lex_rule:lex_rules) {
            regex_nfa.emplace_back(lex_rule.regex_,
                                   Environment::IntSymbol(lex_rule.name_),
                                   lex_rule.priority_);
            if (regex_nfa[regex_nfa.size() - 1].IsEmptyNfa()) {
                return Nfa();
            }
        }
        return Nfa(std::move(regex_nfa));
    }
    return Nfa();
}

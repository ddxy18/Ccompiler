//
// Created by dxy on 2020/8/13.
//

#include "lex/nfa.h"

#include <fstream>
#include <memory>
#include <sstream>
#include <stack>
#include <vector>

using namespace Ccompiler;
using namespace std;

/**
 * lex classification
 *
 * kChar(including [...] and single character)
 * kRepetition('*' '?' '+' '{...}' and their ungreedy mode)
 * kOr('|')
 * kNested('(...)')
 */
enum class LexType {
    kChar, kRepetition, KOr, kNested
};

/**
 * Find next valid part in a regex string. Now we only support '|' and '*'
 * operator. And characters can be single character,[...] and escaped character.
 *
 * @param begin can be any position from 'cbegin()' to 'cend()'
 * @param end must be 'cend()' of a string
 * @return If we find a valid token, return the token. Otherwise return "".
 */
string NextTokenInRegex(StrConstIt &begin, StrConstIt &end);

string ParseCharacterClasses(const string &regex);

/**
 * It determines which 'LexType' should be chosen according to 'lex''s a few
 * characters from its head.
 *
 * @param lex
 * @return LexType
 */
LexType GetLexType(const string &lex);

[[nodiscard]] bool
PushAnd(stack<AstNodePtr> &op_stack, stack<AstNodePtr> &rpn_stack);

[[nodiscard]] bool
PushOr(stack<AstNodePtr> &op_stack, stack<AstNodePtr> &rpn_stack);

[[nodiscard]] bool
PushRepetition(stack<AstNodePtr> &op_stack, stack<AstNodePtr> &rpn_stack,
               const string &regex);

int Nfa::i_ = 0;

/**
 * It works like add '|' between regexes. But it only adds empty edges from
 * the new begin state to all nfas' begin states. All accept states are
 * reserved.
 */
Nfa::Nfa(std::vector<Nfa> nfa_vec) :
        char_ranges_(CharRangesInit(vector<string>())) {
    AddState(char_ranges_.size());  // begin state
    begin_state_number_ = i_ - 1;

    for (auto &nfa:nfa_vec) {
        *this += nfa;
        // copy the only accept state in 'nfa'
        this->accept_states_[nfa.accept_states_.cbegin()->first] =
                nfa.accept_states_.cbegin()->second;
        // add an empty edge to nfa's begin state
        exchange_map_[begin_state_number_][0].push_back(
                nfa.begin_state_number_);
    }
}

Nfa::Nfa(const string &regex, const string &regex_type, int priority) {
    auto char_ranges = CharRangesInit(vector<string>());
    *this = Nfa(ParseRegex(regex), char_ranges);
    char_ranges_ = std::move(char_ranges);
    accept_states_[accept_states_.cbegin()->first] =
            std::move(make_pair(regex_type, priority));
}

Nfa &Nfa::operator+=(Nfa &nfa) {
    for (auto &pair:nfa.exchange_map_) {
        exchange_map_[pair.first] = std::move(pair.second);
    }
    return *this;
}

Nfa Nfa::ReadNfa(const string &nfa_file_name) {
    ifstream in(nfa_file_name);

    if (in) {
        Nfa nfa;
        in >> nfa;
        in.close();
        return std::move(nfa);
    }
    return Nfa();
}

vector<int> Nfa::CharRangesInit(const vector<string> &delim) {
    vector<int> char_ranges;

    /**
     * By default initializing 'char_set_' with ascii and see every character
     * as a range.
     */
    if (delim.empty()) {
        for (int i = 0; i <= 256; ++i) {
            char_ranges.push_back(i);
        }
    }

    return char_ranges;
    // TODO(dxy): see TODO in lex/lex.h line 29
}

void Nfa::WriteNfa(const string &nfa_file_name) const {
    ofstream out(nfa_file_name);

    if (out) {
        out << *this;
        out.close();
    }
}

Token Nfa::NextToken(StrConstIt &begin, StrConstIt &end) {
    if (begin == end) {
        return Token("", "");
    }

    vector<vector<int>> state_vec;
    auto tmp = begin;

    /**
     * Use the begin state and states that can be accessed through empty
     * edges as an initial driver.
     */
    vector<int> initial_states;
    initial_states.push_back(begin_state_number_);
    for (int i = 0; i < initial_states.size(); ++i) {
        for (auto state:exchange_map_[initial_states[i]][0]) {
            if (find(initial_states.cbegin(), initial_states.cend(), state) ==
                initial_states.cend()) {
                initial_states.push_back(state);
            }
        }
    }
    state_vec.push_back(initial_states);

    // find all reachable states from current states
    while (!state_vec[state_vec.size() - 1].empty()) {
        vector<int> cur_states;
        for (auto cur_state:state_vec[state_vec.size() - 1]) {
            for (auto next_state:NextState(cur_state, begin, end)) {
                if (find(cur_states.cbegin(), cur_states.cend(), next_state)
                    == cur_states.cend()) {
                    cur_states.push_back(next_state);
                }
            }
        }
        state_vec.push_back(cur_states);
        begin++;
    }
    // remove the last empty vector
    state_vec.pop_back();
    begin--;

    vector<int> accept_states;
    auto reverse_it = state_vec.rbegin();
    // find the last vector that contains accept states
    while (reverse_it != state_vec.rend() && accept_states.empty()) {
        string regex_type;
        for (auto cur_state:*reverse_it) {
            if (!(regex_type = GetStateType(cur_state)).empty()) {
                accept_states.push_back(cur_state);
            }
        }
        reverse_it++;
        begin--;
    }
    reverse_it--;
    begin++;

    if (accept_states.empty()) {
        return Token("", "");
    }
    // choose the accept state with the min priority
    int accept_state = accept_states[0];
    for (auto state:accept_states) {
        if (GetRegexPriority(accept_state) > GetRegexPriority(state)) {
            accept_state = state;
        }
    }

    return Token(string(tmp, begin), GetStateType(accept_state));
}

string Nfa::GetStateType(int state) {
    auto result = accept_states_.find(state);
    if (result != accept_states_.end()) {
        return result->second.first;
    }
    return "";
}

ostream &Ccompiler::operator<<(ostream &os, const Nfa &nfa) {
    // empty NFA object
    if (nfa.exchange_map_.empty() || nfa.accept_states_.empty()) {
        return os;
    }

    os << "char_ranges_:" << endl;
    // print character ranges
    for (auto char_range : nfa.char_ranges_) {
        os << char_range << " ";
    }
    os << endl;

    os << "exchange_map_:" << endl;
    // print edge in format like 'state number  edge1,edge2...  edge1,edge2...'
    for (auto &element:nfa.exchange_map_) {
        os << element.first << " ";
        for (auto &v:element.second) {
            if (v.empty()) {
                os << "-1";
            }
            for (auto &edge:v) {
                os << edge << ",";
            }
            os << " ";
        }
        os << endl;
    }

    os << "begin_state_number_:" << endl;
    os << nfa.begin_state_number_ << endl;

    os << "accept_states_:" << endl;
    // print accept states in format like 'state number,regex type name'
    for (auto &pair:nfa.accept_states_) {
        os << pair.first << "," << pair.second.first << ","
           << pair.second.second
           << " ";
    }

    return os;
}

istream &Ccompiler::operator>>(istream &is, Nfa &nfa) {
    string line;

    // detect whether read from a valid NFA istream
    getline(is, line);
    if (line != "char_ranges_:") {
        return is;
    }

    // read character ranges
    vector<int> char_ranges;
    getline(is, line);
    istringstream str_stream(line);
    string s;
    while (str_stream >> s) {
        nfa.char_ranges_.push_back(stoi(s));
    }

    // read exchange map body
    while (getline(is, line) && line != "begin_state_number_:") {
        int state_number;
        vector<vector<int>> v;
        istringstream line_stream(line);
        line_stream >> state_number;
        while (line_stream >> s) {
            vector<int> edge_vec;
            if (s != "-1") {
                istringstream edges_stream(s);
                // split edges according to the character range
                string str_number;
                while (getline(edges_stream, str_number, ',')) {
                    edge_vec.push_back(stoi(str_number));
                }
            }
            v.push_back(edge_vec);
        }
        nfa.exchange_map_[state_number] = v;
    }

    // read begin state number
    getline(is, line);
    nfa.begin_state_number_ = stoi(line);

    // read accept states
    getline(is, line);
    if (line != "accept_states_:") {
        return is;
    }
    getline(is, line);
    istringstream line_stream(line);
    while (line_stream >> s) {
        istringstream acpt_states_stream(s);
        string str_state_number, reg_type_name, reg_priority;

        getline(acpt_states_stream, str_state_number, ',');
        getline(acpt_states_stream, reg_type_name, ',');
        getline(acpt_states_stream, reg_priority, ',');
        nfa.accept_states_[stoi(str_state_number)] =
                {reg_type_name, stoi(reg_priority)};
    }

    return is;
}

Nfa::Nfa(AstNodePtr ast_head, vector<int> &char_ranges) {
    if (ast_head) {
        Nfa left_nfa(std::move(ast_head->left_son_), char_ranges);
        Nfa right_nfa(std::move(ast_head->right_son_), char_ranges);
        *this += left_nfa;
        *this += right_nfa;

        if (left_nfa.IsEmptyNfa() && right_nfa.IsEmptyNfa()) {
            // parse individual character
            AddState(char_ranges.size() - 1);
            begin_state_number_ = i_ - 1;
            AddState(char_ranges.size() - 1);
            exchange_map_[i_ - 2][GetCharLocation(
                    ast_head->regex_[0], char_ranges)].push_back(i_ - 1);
            accept_states_[i_ - 1] = {"", 0};
        } else {
            // TODO(dxy): support more operators and characters
            if (ast_head->regex_ == "|") {
                /**
             * Add empty edges from new begin state to 'left_nfa''s and
             * 'right_nfa''s begin state.
             */
                AddState(char_ranges.size() - 1);
                begin_state_number_ = i_ - 1;
                exchange_map_[begin_state_number_][0].push_back(
                        left_nfa.begin_state_number_);
                exchange_map_[begin_state_number_][0].push_back(
                        right_nfa.begin_state_number_);

                /**
                 * Add empty edges from 'left_nfa''s and 'right_nfa''s accept
                 * state to the new accept state.
                 */
                AddState(char_ranges.size() - 1);
                // Now all NFAs have the only accept state.
                exchange_map_[left_nfa.accept_states_.cbegin()->first][0].push_back(
                        i_ - 1);

                exchange_map_[right_nfa.accept_states_.cbegin()->first][0].push_back(
                        i_ - 1);

                accept_states_[i_ - 1] = {"", 0};
            } else if (ast_head->regex_ == "&") {
                /**
                * Combine 'left_nfa''s accept state and 'right_nfa''s begin
                 * state.
                */
                begin_state_number_ = left_nfa.begin_state_number_;
                accept_states_[right_nfa.accept_states_.cbegin()->first] =
                        {"", 0};

                /**
                 * Copy 'right_nfa''s begin state edges to 'left_nfa''s accept
                 * state and remove 'right_nfa''s begin state.
                 */
                for (int i = 0;
                     i < exchange_map_[right_nfa.begin_state_number_].size();
                     ++i) {
                    for (auto edge:
                            exchange_map_[right_nfa.begin_state_number_][i]) {
                        exchange_map_
                        [left_nfa.accept_states_.cbegin()->first][i].push_back(
                                edge);
                    }
                }
                exchange_map_.erase(right_nfa.begin_state_number_);
            } else if (ast_head->regex_ == "*") {
                /**
                * Add an empty edge from 'left_nfa''s accept state to begin state
                * to represent repeat several times.
                */
                exchange_map_[left_nfa.accept_states_.cbegin()->first][0].push_back(
                        left_nfa.begin_state_number_);

                /**
                 * Add an empty edge from new begin state to 'left_nfa''s begin
                 * state.
                 */
                AddState(char_ranges.size() - 1);
                begin_state_number_ = i_ - 1;
                exchange_map_[begin_state_number_][0].push_back(
                        left_nfa.begin_state_number_);

                /**
                 * Add an empty edge from 'left_nfa''s accept state new to accept
                 * state.
                 */
                AddState(char_ranges.size() - 1);
                exchange_map_[left_nfa.accept_states_.cbegin()->first][0].push_back(
                        i_ - 1);
                /**
                 * Add an empty edge from new begin state to new accept state to
                 * represent repeat 0 time.
                 */
                exchange_map_[begin_state_number_][0].push_back(i_ - 1);

                accept_states_[i_ - 1] = {"", 0};
            }
        }
    }
}

AstNodePtr Nfa::ParseRegex(const string &regex) {
    stack<AstNodePtr> op_stack;
    stack<AstNodePtr> rpn_stack;
    string lex;
    auto cur_it = regex.cbegin(), end = regex.cend();
    bool or_flag = true;  // records whether the last lex is '|'
    AstNodePtr son;

    while (!(lex = NextTokenInRegex(cur_it, end)).empty()) {
        switch (GetLexType(lex)) {
            case LexType::KOr:
                or_flag = true;
                if (!PushOr(op_stack, rpn_stack)) {
                    return nullptr;
                }
                break;
            case LexType::kRepetition:
                if (!PushRepetition(op_stack, rpn_stack, lex)) {
                    return nullptr;
                }
                break;
            case LexType::kChar:
                /**
                     * We need to explicitly add '&' before a character when the
                     * last lex isn't '|'.
                     */
                if (!or_flag) {
                    if (!PushAnd(op_stack, rpn_stack)) {
                        return nullptr;
                    }
                }

                if (lex.size() == 1) {  // individual character
                    rpn_stack.push(make_unique<AstNode>(lex));
                } else {  // [...] and escape character
                    auto s = ParseCharacterClasses(lex);
                    if (s == lex) {
                        rpn_stack.push(
                                make_unique<AstNode>(string(lex.cbegin() + 1,
                                                            lex.cend())));
                    } else {
                        son = ParseRegex(s);
                        rpn_stack.push(std::move(son));
                    }

                }
                or_flag = false;
                break;
            case LexType::kNested:
                if (!or_flag) {
                    if (!PushAnd(op_stack, rpn_stack)) {
                        return nullptr;
                    }
                }
                // build AST for the nested regex
                son = ParseRegex(string(lex.cbegin() + 1, lex.cend() - 1));
                if (son) {
                    rpn_stack.push(std::move(son));
                }
                or_flag = false;
                break;
        }
    }

    // empty 'op_stack'
    if (!PushOr(op_stack, rpn_stack) || rpn_stack.size() != 1) {
        return nullptr;
    }
    return std::move(rpn_stack.top());
}

vector<int> Nfa::NextState(int cur_state, StrConstIt cur_it, StrConstIt end) {
    vector<int> state_vec;

    if (cur_it != end) {
        // add states that can be reached through '*cur_it' edges
        for (auto state:exchange_map_[cur_state][GetCharLocation(
                *cur_it, char_ranges_)]) {
            state_vec.push_back(state);
        }

        // add states that can be reached through empty edges
        int i = -1;
        while (++i != state_vec.size()) {
            for (auto state:exchange_map_[state_vec[i]][0]) {
                if (find(state_vec.cbegin(), state_vec.cend(), state) ==
                    state_vec.cend()) {
                    state_vec.push_back(state);
                }
            }
        }
    }

    return std::move(state_vec);
}

void Nfa::AddState(unsigned long size) {
    vector<vector<int>> edge_vec;
    edge_vec.reserve(char_ranges_.size());
    for (int i = 0; i < size; ++i) {
        edge_vec.emplace_back(vector<int>());
    }
    exchange_map_.insert({i_++, edge_vec});
}

int Nfa::GetCharLocation(char c, const vector<int> &char_ranges) {
    int begin = 0, end = char_ranges.size(), mid = (begin + end) / 2;
    while (begin != end - 1) {
        if (c < char_ranges[mid]) {
            end = mid;
        } else {
            begin = mid;
        }
        mid = (begin + end) / 2;
    }
    return begin;
}

string NextTokenInRegex(StrConstIt &begin, StrConstIt &end) {
    if (begin != end) {
        auto cur_begin = begin;
        map<char, char> bracket{
                {']', '['}
        };
        int bracket_num = 1;

        switch (*begin) {
            case '|':
            case '.':
            case '^':
            case '$':
//                return string(1, *begin++);

                // repeat(greedy)
            case '*':
            case '+':
            case '?':
//                if (++begin != end) {
//                    if (*begin == '?') {
//                        begin++;
//                        return string(begin - 2, begin);
//                    }
//                }
                return string(1, *begin++);

            case '\\':  // escaped characters
                if (++begin != end) {
                    begin++;
                    return string(begin - 2, begin);
                }
                return "";

                /**
                 * See '[...]' '{...}' '<...>' as a lex. Although it cannot
                 * be nested, nested error is detected by caller and now
                 * we only find the first matched right bracket.
                 */
            case '[':
                cur_begin = begin;
                while (++begin != end) {
                    auto it = bracket.find(*begin);
                    if (it != bracket.end() && it->second == *cur_begin) {
                        return string(cur_begin, ++begin);
                    }
                }
                // lack of ']'
                return "";

                /**
                 * '()' and contents between them are seen as one token and it
                 * can be nested.
                 */
            case '(':
                cur_begin = begin;
                while (bracket_num != 0 && ++begin != end) {
                    switch (*begin) {
                        case '(':
                            bracket_num++;
                            break;
                        case ')':
                            bracket_num--;
                            break;
                    }
                }
                if (bracket_num == 0) {
                    begin++;
                    return string(cur_begin, begin);
                }
                // lack of ')'
                return "";
            default:
                return string(1, *begin++);
        }
    }

    // All characters in 'regex' have been scanned.
    return "";
}

string ParseCharacterClasses(const string &regex) {
    if (regex[0] == '[') {  // [...]
        // TODO(dxy): consider '[^...]'
        string s;
        auto cur = regex.cbegin();
        while (++cur != regex.cend() - 1) {
            if (*cur == '\\') {  // escape character
                if (++cur != regex.cend() - 1) {
                    s += *(cur - 1);
                    s += *cur;
                    s += '|';
                }
            } else if (*cur == '-') {  // range
                string range;
                char c = *(s.cend() - 2);
                s.erase(s.cend() - 2, s.cend());
                if (++cur != regex.cend() - 1) {
                    while (c <= *cur) {
                        range += c;
                        range += '|';
                        c++;
                    }
                    s += range;
                }
            } else {  // individual character
                s += *cur;
                s += '|';
            }
        }
        s.erase(s.cend() - 1);  // remove the last '|'

        return s;
    } else if (regex[0] == '\\') {  // escape character
        switch (regex[1]) {
            case 'd':
                return "0|1|2|3|4|5|6|7|8|9";
            case 't':
                return "\t";
            case 'n':
                return "\n";
            case 'r':
                return "\r";
            default:  // parse as normal characters
                return string(regex.cbegin(), regex.cend());

                // TODO(dxy): add more escape character
        }
    }

    return string();
}

LexType GetLexType(const string &lex) {
    switch (lex[0]) {
        // '&' is implicit so we don't have to think about it.
        case '|':
            return LexType::KOr;
        case '*':
            return LexType::kRepetition;
        case '(':
            return LexType::kNested;
        default:
            return LexType::kChar;
    }
}

bool PushAnd(stack<AstNodePtr> &op_stack, stack<AstNodePtr> &rpn_stack) {
    while (!op_stack.empty() && op_stack.top()->GetRegex() != "|") {
        if (op_stack.top()->GetRegex() == "&") {
            if (rpn_stack.size() < 2) {
                return false;
            }
            op_stack.top()->SetRightSon(std::move(rpn_stack.top()));
            rpn_stack.pop();
            op_stack.top()->SetLeftSon(std::move(rpn_stack.top()));
            rpn_stack.pop();
        } else {
            if (rpn_stack.empty()) {
                return false;
            }
            op_stack.top()->SetLeftSon(std::move(rpn_stack.top()));
            rpn_stack.pop();
        }
        rpn_stack.push(std::move(op_stack.top()));
        op_stack.pop();
    }
    op_stack.push(make_unique<AstNode>("&"));

    return true;
}

bool PushOr(stack<AstNodePtr> &op_stack, stack<AstNodePtr> &rpn_stack) {
    while (!op_stack.empty()) {
        if (op_stack.top()->GetRegex() == "&" ||
            op_stack.top()->GetRegex() == "|") {
            if (rpn_stack.size() < 2) {
                return false;
            }
            op_stack.top()->SetRightSon(std::move(rpn_stack.top()));
            rpn_stack.pop();
            op_stack.top()->SetLeftSon(std::move(rpn_stack.top()));
            rpn_stack.pop();
        } else {
            if (rpn_stack.empty()) {
                return false;
            }
            op_stack.top()->SetLeftSon(std::move(rpn_stack.top()));
            rpn_stack.pop();
        }
        rpn_stack.push(std::move(op_stack.top()));
        op_stack.pop();
    }
    op_stack.push(make_unique<AstNode>("|"));

    return true;
}

bool PushRepetition(stack<AstNodePtr> &op_stack, stack<AstNodePtr> &rpn_stack,
                    const string &regex) {
    while (!op_stack.empty() && op_stack.top()->GetRegex() != "|" &&
           op_stack.top()->GetRegex() != "&") {
        if (rpn_stack.empty()) {
            return false;
        }
        op_stack.top()->SetLeftSon(std::move(rpn_stack.top()));
        rpn_stack.pop();
        rpn_stack.push(std::move(op_stack.top()));
        op_stack.pop();
    }
    op_stack.push(make_unique<AstNode>(regex));

    return true;
}

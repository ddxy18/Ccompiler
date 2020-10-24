//
// Created by dxy on 2020/8/6.
//

#include "lex/nfa.h"

#include <cctype>
#include <climits>
#include <sstream>
#include <stack>

#include "lex/token.h"

using namespace CCompiler;
using namespace std;

int Nfa::i_ = 0;

/**
 * It determines which RegexPart should be chosen according to regex's a few
 * characters from its head.
 *
 * @param regex
 * @return
 */
RegexPart GetRegexType(const string &regex);

set<string> GetDelim(const string &regex);

/**
 * It finds one token a time and sets 'begin' to the head of the next token.
 * Notice that (...) {...} <...> [...] are seen as a token.
 *
 * @param begin Always point to the begin of the next token. If no valid
 * token remains, it points to iterator.cend().
 * @param end Always point to the end of the given regex.
 * @return A valid token. If it finds an invalid token or reaching to the end
 * of the regex, it returns "".
 */
std::string NextTokenInRegex(StrConstIt &begin, StrConstIt &end);

/**
     * @param begin
     * @param end
     * @return Return the iterator of the first character after the escape
     * characters. If no valid escape characters are found, return begin.
     */
StrConstIt SkipEscapeCharacters(StrConstIt begin, StrConstIt end);

/**
 * Add a character range [begin, end) to char_ranges.
 *
 * @param begin
 * @param end
 */
void AddCharRange(
        set<unsigned int> &char_ranges, unsigned int begin, unsigned int end);

/**
 * Add a char range [begin, begin + 1) to char_ranges.
 *
 * @param begin
 */
void AddCharRange(set<unsigned int> &char_ranges, unsigned int begin);

bool
PushAnd(stack<RegexAstNodePtr> &op_stack, stack<RegexAstNodePtr> &rpn_stack);

bool
PushOr(stack<RegexAstNodePtr> &op_stack, stack<RegexAstNodePtr> &rpn_stack);

bool PushQuantifier(stack<RegexAstNodePtr> &op_stack,
                    stack<RegexAstNodePtr> &rpn_stack, const string &regex);

AcptStatePtr Nfa::NextMatch(StrConstIt begin, StrConstIt end) {
  vector<ReachableStatesMap> state_vec = StateRoute(begin, end);
  auto it = state_vec.cbegin();
  State state = *state_vec[0].find({begin_state_, begin});

  // find the longest match
  while (it != state_vec.cend()) {
    for (const auto &cur_state:*it) {
      if (accept_states_.contains(cur_state.first)) {
        if (cur_state.second > state.second) {
          state = cur_state;
        } else if (cur_state.second == state.second) {
          if (accept_states_.contains(state.first)) {
            if (accept_states_[cur_state.first] < accept_states_[state.first]) {
              state = cur_state;
            }
          } else {
            state = cur_state;
          }
        }
      }
    }
    it++;
  }

  if (state.first == begin_state_) {
    return nullptr;
  } else {
    return make_unique<AcceptState>(accept_states_[state.first], state.second);
  }
}

StrConstIt SkipEscapeCharacters(StrConstIt begin, StrConstIt end) {
  auto cur_it = begin;

  if (cur_it != end && *cur_it == '\\') {
    cur_it++;

    switch (*cur_it) {
      case 'u':
        return cur_it + 5;
      case 'c':
        return cur_it + 2;
      case 'x':
        return cur_it + 3;
      case '0':
        return begin;
      default:
        return cur_it + 1;
    }
  }

  return begin;  // not escape characters
}

vector<ReachableStatesMap> Nfa::StateRoute(StrConstIt begin, StrConstIt end) {
  vector<ReachableStatesMap> state_vec;
  ReachableStatesMap cur_states;

  // Use states that can be accessed through empty edges from begin_state_
  // and begin_state_ itself as an initial driver.
  State begin_state = {begin_state_, begin};
  if (GetStateType(begin_state_) == StateType::kCommon) {
    cur_states.merge(NextState(begin_state));
  }
  cur_states.insert(begin_state);
  state_vec.push_back(cur_states);

  // find all reachable states from current states
  while (!state_vec[state_vec.size() - 1].empty()) {
    cur_states.clear();
    for (const auto &last_state:state_vec[state_vec.size() - 1]) {
      cur_states.merge(NextState(last_state, end));
    }
    state_vec.push_back(cur_states);
  }
  // remove the last empty vector
  state_vec.pop_back();

  return state_vec;
}

ReachableStatesMap Nfa::NextState(const State &cur_state, StrConstIt str_end) {
  ReachableStatesMap next_states;
  auto begin = cur_state.second;

  switch (GetStateType(cur_state.first)) {
    case StateType::kSpecialPattern:
      begin = special_pattern_states_.find(cur_state.first)->second.NextMatch(
              cur_state, str_end);
      if (begin != cur_state.second) {
        next_states.insert({cur_state.first, begin});
      }
      break;
    case StateType::kRange:
      begin = range_states_.find(cur_state.first)->second.NextMatch(
              cur_state, str_end);
      if (begin != cur_state.second) {
        next_states.insert({cur_state.first, begin});
      }
      break;
    case StateType::kCommon:
      // add states that can be reached through *cur_it
      if (begin < str_end) {
        for (auto state:
                exchange_map_[cur_state.first][GetCharLocation(*begin)]) {
          next_states.insert({state, begin + 1});
        }
      }
      break;
  }

  ReachableStatesMap next_states_from_empty;
  for (const auto &state:next_states) {
    next_states_from_empty.merge(NextState(state));
  }
  next_states.erase({cur_state.first, begin});
  next_states.merge(next_states_from_empty);

  return next_states;
}

ReachableStatesMap Nfa::NextState(const State &cur_state) {
  vector<int> common_states;
  set<int> func_states;

  common_states.push_back(cur_state.first);
  int i = -1;
  while (++i != common_states.size()) {
    for (auto state:exchange_map_[common_states[i]][kEmptyEdge]) {
      if (GetStateType(state) == StateType::kCommon) {
        if (find(common_states.cbegin(), common_states.cend(), state) ==
            common_states.cend()) {
          common_states.push_back(state);
        }
      } else {
        func_states.insert(state);
      }
    }
  }
  // erase cur_state
  common_states.erase(common_states.cbegin());

  ReachableStatesMap next_states_map;
  for (auto state:common_states) {
    next_states_map.insert({state, cur_state.second});
  }
  for (auto state:func_states) {
    next_states_map.insert({state, cur_state.second});
  }

  return next_states_map;
}

Nfa::StateType Nfa::GetStateType(int state) {
  if (special_pattern_states_.contains(state)) {
    return StateType::kSpecialPattern;
  }
  if (range_states_.contains(state)) {
    return StateType::kRange;
  }
  return StateType::kCommon;
}

set<string> GetDelim(const string &regex) {
  auto begin = regex.cbegin(), end = regex.cend();
  set<string> delim;
  string token;

  while (!(token = NextTokenInRegex(begin, end)).empty()) {
    switch (GetRegexType(token)) {
      case RegexPart::kChar:
        delim.insert(token);
        break;
      case RegexPart::kPassiveGroup:
        delim.merge(GetDelim(string{token.cbegin() + 3, token.cend() - 1}));
        break;
      default:
        break;
    }
  }

  return delim;
}

Nfa &Nfa::operator+=(Nfa &nfa) {
  if (char_ranges_.empty()) {
    char_ranges_.assign(nfa.char_ranges_.cbegin(), nfa.char_ranges_.cend());
  }
  for (auto &pair:nfa.exchange_map_) {
    exchange_map_[pair.first] = std::move(pair.second);
  }
  special_pattern_states_.merge(nfa.special_pattern_states_);
  range_states_.merge(nfa.range_states_);

  return *this;
}

void Nfa::CharRangesInit(const set<string> &delim) {
  set<unsigned int> char_ranges;

  // determine encode range
  unsigned int max_encode = 0x7f;

  // initialize special ranges
  AddCharRange(char_ranges, kEmptyEdge);
  AddCharRange(char_ranges, max_encode);

  for (auto &s:delim) {
    if (s.size() == 1 && s != ".") {  // single character
      // see single character as a range [s[0], s[0] + 1)
      AddCharRange(char_ranges, s[0]);
    }
  }

  char_ranges_.assign(char_ranges.begin(), char_ranges.end());
}

void AddCharRange(
        set<unsigned int> &char_ranges, unsigned int begin, unsigned int end) {
  char_ranges.insert(begin);
  char_ranges.insert(end);
}

void AddCharRange(set<unsigned int> &char_ranges, unsigned int begin) {
  AddCharRange(char_ranges, begin, begin + 1);
}

int Nfa::GetCharLocation(int c) {
  for (int i = 0; i < char_ranges_.size(); ++i) {
    if (char_ranges_[i] > c) {
      return i - 1;
    }
  }
  return -1;
}

/**
 * It works like add | between regexes. But it only adds empty edges from
 * the new begin state to all nfas' begin states. All accept states are
 * reserved.
 */
Nfa::Nfa(const map<string, TokenType> &regex_rules) {
  // initialize char_ranges_
  if (!regex_rules.empty()) {
    string regex;
    for (auto &regex_rule:regex_rules) {
      regex += regex_rule.first + "|";
    }
    regex.erase(regex.cend() - 1);
    auto delim = GetDelim(regex);
    CharRangesInit(delim);

    NewState();  // begin state
    begin_state_ = i_;

    for (auto &regex_rule:regex_rules) {
      Nfa nfa(regex_rule.first, regex_rule.second, char_ranges_);
      *this += nfa;
      // copy the only accept state in nfa
      this->accept_states_.merge(nfa.accept_states_);
      // add an empty edge to nfa's begin state
      exchange_map_[begin_state_][0].insert(nfa.begin_state_);
    }
  }
}

Nfa::Nfa(const string &regex, TokenType type,
         const vector<unsigned int> &char_ranges) {
  auto ast_head = ParseRegex(regex);
  if (ast_head) {
    *this = Nfa(ast_head, char_ranges);

    // add a new state as the accept state to prevent that accept state is a
    // functional state
    NewState();
    exchange_map_[accept_states_.cbegin()->first][kEmptyEdge].insert(i_);
    accept_states_.clear();
    accept_states_[i_] = type;
  }
}

RegexAstNodePtr Nfa::ParseRegex(const string &regex) {
  stack<RegexAstNodePtr> op_stack;
  stack<RegexAstNodePtr> rpn_stack;
  string lex;
  auto cur_it = regex.cbegin(), end = regex.cend();
  bool or_flag = true;  // whether the last lex is |
  RegexAstNodePtr son;

  while (!(lex = NextTokenInRegex(cur_it, end)).empty()) {
    switch (GetRegexType(lex)) {
      case RegexPart::kAlternative:
        or_flag = true;
        if (!PushOr(op_stack, rpn_stack)) {
          return nullptr;
        }
        break;
      case RegexPart::kQuantifier:
        if (!PushQuantifier(op_stack, rpn_stack, lex)) {
          return nullptr;
        }
        break;
      case RegexPart::kChar:
        // We need to explicitly add & before a character when the last lex
        // isn't |.
        if (!or_flag && !PushAnd(op_stack, rpn_stack)) {
          return nullptr;
        }
        rpn_stack.push(make_unique<RegexAstNode>(RegexPart::kChar, lex));
        or_flag = false;
        break;
      case RegexPart::kPassiveGroup:
        if (!or_flag && !PushAnd(op_stack, rpn_stack)) {
          return nullptr;
        }
        son = ParseRegex(string{lex.cbegin() + 3, lex.cend() - 1});
        rpn_stack.push(std::move(son));
        or_flag = false;
        break;
      case RegexPart::kAnd:
        // kAnd should not exist here, so it is seen as an error.
      case RegexPart::kError:  // skip the error token
        break;
    }
  }

  // empty 'op_stack'
  if (!PushOr(op_stack, rpn_stack) || rpn_stack.size() != 1) {
    return nullptr;
  }
  return std::move(rpn_stack.top());
}

string NextTokenInRegex(StrConstIt &begin, StrConstIt &end) {
  if (begin != end) {
    auto cur_begin = begin;
    map<char, char> pair_characters{
            {'}', '{'},
            {']', '['}
    };

    // count ( to ensure correct matches of nested ()
    int parentheses = 1;
    switch (*begin) {
      case '|':
      case '.':
      case '^':
      case '$':
        return string(1, *begin++);

        // repeat(greedy and non-greedy)
      case '*':
      case '+':
      case '?':
        if (++begin != end) {
          if (*begin == '?') {
            return string(cur_begin, ++begin);
          }
        }
        return string(cur_begin, begin);

      case '\\':  // escape characters
        begin = SkipEscapeCharacters(begin, end);
        return string(cur_begin, begin);

        // See [...] {...} <...> as a lex. Although it cannot be nested,
        // nested error is detected by caller and now we only find the first
        // matched right bracket.
      case '[':
      case '{':
        while (++begin != end) {
          // skip \] \}
          begin = SkipEscapeCharacters(begin, end);
          if (begin != end) {
            auto it = pair_characters.find(*begin);
            if (it != pair_characters.end() && it->second == *cur_begin) {
              return string(cur_begin, ++begin);
            }
          }
        }
        return "";  // lack of ] } >

        // () and contents between them are seen as one token and it
        // can be nested.
      case '(':
        while (parentheses != 0 && ++begin != end) {
          begin = SkipEscapeCharacters(begin, end);
          if (begin != end) {
            switch (*begin) {
              case '(':
                parentheses++;
                break;
              case ')':
                parentheses--;
                break;
            }
          }
        }
        if (parentheses == 0) {
          return string(cur_begin, ++begin);
        }
        return "";  // lack of )

        // lack of their left pairs
      case ']':
      case '}':
      case ')':
        return "";
      default:
        return string(1, *begin++);
    }
  }
  return "";  // All characters in regex have been scanned.
}

RegexPart GetRegexType(const string &regex) {
  switch (regex[0]) {
    // & is implicit so we don't have to think about it.
    case '|':
      return RegexPart::kAlternative;
    case '*':
    case '+':
    case '?':
    case '{':
      return RegexPart::kQuantifier;
    case '\\':
      return RegexPart::kChar;
    case '(':
      if (regex[1] == '?') {
        switch (regex[2]) {
          case ':':
            return RegexPart::kPassiveGroup;
          default:  // ? should have characters before it
            return RegexPart::kError;
        }
      }
    default:
      return RegexPart::kChar;
  }
}

bool
PushAnd(stack<RegexAstNodePtr> &op_stack, stack<RegexAstNodePtr> &rpn_stack) {
  while (!op_stack.empty() &&
         op_stack.top()->GetType() != RegexPart::kAlternative) {
    if (op_stack.top()->GetType() == RegexPart::kAnd) {
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
  op_stack.push(make_unique<RegexAstNode>(RegexPart::kAnd, ""));

  return true;
}

bool
PushOr(stack<RegexAstNodePtr> &op_stack, stack<RegexAstNodePtr> &rpn_stack) {
  while (!op_stack.empty()) {
    if (op_stack.top()->GetType() == RegexPart::kAnd ||
        op_stack.top()->GetType() == RegexPart::kAlternative) {
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
  op_stack.push(make_unique<RegexAstNode>(RegexPart::kAlternative, ""));

  return true;
}

bool PushQuantifier(stack<RegexAstNodePtr> &op_stack,
                    stack<RegexAstNodePtr> &rpn_stack, const string &regex) {
  while (!op_stack.empty() &&
         op_stack.top()->GetType() != RegexPart::kAlternative &&
         op_stack.top()->GetType() != RegexPart::kAnd) {
    if (rpn_stack.empty()) {
      return false;
    }
    op_stack.top()->SetLeftSon(std::move(rpn_stack.top()));
    rpn_stack.pop();
    rpn_stack.push(std::move(op_stack.top()));
    op_stack.pop();
  }
  op_stack.push(make_unique<RegexAstNode>(RegexPart::kQuantifier, regex));

  return true;
}

Nfa::Nfa(RegexAstNodePtr &ast_head, const vector<unsigned int> &char_ranges) {
  if (ast_head) {
    switch (ast_head->regex_type_) {
      case RegexPart::kChar:
        *this = NfaFactory::MakeCharacterNfa(ast_head->regex_, char_ranges);
        break;
      case RegexPart::kAlternative:
        *this = NfaFactory::MakeAlternativeNfa(
                Nfa(ast_head->left_son_, char_ranges),
                Nfa(ast_head->right_son_, char_ranges));
        break;
      case RegexPart::kAnd:
        *this = NfaFactory::MakeAndNfa(
                Nfa(ast_head->left_son_, char_ranges),
                Nfa(ast_head->right_son_, char_ranges));
        break;
      case RegexPart::kQuantifier:
        *this = NfaFactory::MakeQuantifierNfa(
                ast_head->regex_, ast_head->left_son_, char_ranges);
        break;
      case RegexPart::kPassiveGroup:
        // It is handled in ParseRegex(), so it shouldn't appear here.
      case RegexPart::kError:
        break;
    }
  }
}

void Nfa::NewState() {
  vector<set<int>> edges_vec;
  edges_vec.reserve(char_ranges_.size());
  for (int i = 0; i < char_ranges_.size(); ++i) {
    edges_vec.emplace_back(set<int>());
  }
  exchange_map_.insert({++i_, edges_vec});
}

Nfa NfaFactory::MakeCharacterNfa(
        const string &characters, const vector<unsigned int> &char_ranges) {
  Nfa nfa;
  nfa.char_ranges_.assign(char_ranges.cbegin(), char_ranges.cend());

  nfa.NewState();
  nfa.begin_state_ = Nfa::i_;

  if (characters.size() == 1) {
    if (characters == ".") {
      nfa.accept_states_[nfa.begin_state_] = TokenType::kEmpty;
      nfa.special_pattern_states_.insert(
              {nfa.begin_state_, SpecialPatternNfa(characters)});
    } else {  // single character
      nfa.NewState();
      nfa.accept_states_[Nfa::i_] = TokenType::kEmpty;
      nfa.exchange_map_[nfa.begin_state_][nfa.GetCharLocation(
              characters[0])].insert(nfa.accept_states_.cbegin()->first);
    }
  } else if (characters[0] == '[') {  // [...]
    nfa.accept_states_[nfa.begin_state_] = TokenType::kEmpty;
    nfa.range_states_.insert({nfa.begin_state_, RangeNfa(characters)});
  } else {  // special pattern characters
    nfa.accept_states_[nfa.begin_state_] = TokenType::kEmpty;
    nfa.special_pattern_states_.insert(
            {nfa.begin_state_, SpecialPatternNfa(characters)});
  }

  return nfa;
}

Nfa NfaFactory::MakeAlternativeNfa(Nfa left_nfa, Nfa right_nfa) {
  Nfa nfa;

  nfa += left_nfa;
  nfa += right_nfa;
  // Add empty edges from new begin state to left_nfa's and right_nfa's begin
  // state.
  nfa.NewState();
  nfa.begin_state_ = Nfa::i_;
  nfa.exchange_map_[nfa.begin_state_][Nfa::kEmptyEdge].insert(
          left_nfa.begin_state_);
  nfa.exchange_map_[nfa.begin_state_][Nfa::kEmptyEdge].insert(
          right_nfa.begin_state_);
  // Add empty edges from left_nfa's and right_nfa's accept states to the new
  // accept state.
  nfa.NewState();
  nfa.exchange_map_[left_nfa.accept_states_.cbegin()->first][
          Nfa::kEmptyEdge].insert(Nfa::i_);
  nfa.exchange_map_[right_nfa.accept_states_.cbegin()->first][
          Nfa::kEmptyEdge].insert(Nfa::i_);
  nfa.accept_states_[Nfa::i_] = TokenType::kEmpty;

  return nfa;
}

Nfa NfaFactory::MakeAndNfa(Nfa left_nfa, Nfa right_nfa) {
  Nfa nfa;

  nfa += left_nfa;
  nfa += right_nfa;
  nfa.begin_state_ = left_nfa.begin_state_;
  nfa.accept_states_ = right_nfa.accept_states_;
  // Add empty edges from 'left_nfa''s accept state to 'right_nfa''s begin
  // state.
  nfa.exchange_map_[left_nfa.accept_states_.cbegin()->first][
          Nfa::kEmptyEdge].insert(right_nfa.begin_state_);

  return nfa;
}

Nfa NfaFactory::MakeQuantifierNfa(const string &quantifier,
                                  RegexAstNodePtr &left,
                                  const vector<unsigned int> &char_ranges) {
  Nfa nfa;
  nfa.char_ranges_.assign(char_ranges.cbegin(), char_ranges.cend());

  auto repeat_range = ParseQuantifier(quantifier);

  nfa.NewState();
  nfa.begin_state_ = Nfa::i_;
  nfa.accept_states_[Nfa::i_] = TokenType::kEmpty;

  int i = 1;
  for (; i < repeat_range.first; ++i) {
    Nfa left_nfa(left, nfa.char_ranges_);
    // connect 'left_nfa' to the end of the current nfa
    nfa = MakeAndNfa(nfa, left_nfa);
  }

  nfa.NewState();
  int final_accept_state = Nfa::i_;

  if (repeat_range.second == INT_MAX) {
    Nfa left_nfa(left, nfa.char_ranges_);
    // connect left_nfa to the end of the current nfa
    nfa = MakeAndNfa(nfa, left_nfa);
    nfa.exchange_map_[nfa.accept_states_.cbegin()->first][
            Nfa::kEmptyEdge].insert(left_nfa.begin_state_);
    nfa.exchange_map_[nfa.accept_states_.cbegin()->first][
            Nfa::kEmptyEdge].insert(final_accept_state);
  } else {
    for (; i <= repeat_range.second; ++i) {
      Nfa left_nfa{left, nfa.char_ranges_};
      // connect left_nfa to the end of the current nfa
      nfa = MakeAndNfa(nfa, left_nfa);
      nfa.exchange_map_[left_nfa.accept_states_.cbegin()->first][
              Nfa::kEmptyEdge].insert(final_accept_state);
    }
  }

  nfa.accept_states_.clear();
  nfa.accept_states_[final_accept_state] = TokenType::kEmpty;

  if (repeat_range.first == 0) {
    // add an empty edge from the begin state to the accept state
    nfa.exchange_map_[nfa.begin_state_][Nfa::kEmptyEdge].insert(
            nfa.accept_states_.cbegin()->first);
  }

  return nfa;
}

pair<int, int> NfaFactory::ParseQuantifier(const string &quantifier) {
  pair<int, int> repeat_range;

  // initialize 'repeat_range'
  if (quantifier == "*") {
    repeat_range.first = 0;
    repeat_range.second = INT_MAX;
  } else if (quantifier == "+") {
    repeat_range.first = 1;
    repeat_range.second = INT_MAX;
  } else if (quantifier == "?") {
    repeat_range.first = 0;
    repeat_range.second = 1;
  } else {  // {...}
    // extract first number
    auto beg_it = find_if(quantifier.cbegin(), quantifier.cend(),
                          [](auto c) { return isdigit(c); });
    auto end_it = find_if_not(beg_it, quantifier.cend(),
                              [](auto c) { return isdigit(c); });
    repeat_range.first = stoi(string(beg_it, end_it));

    auto comma = quantifier.find(',');
    if (comma == string::npos) {  // exact times
      repeat_range.second = repeat_range.first;
    } else {  // {min,max} or {min,}
      // extract first number
      beg_it = find_if(end_it, quantifier.cend(),
                       [](auto c) { return isdigit(c); });
      end_it = find_if_not(beg_it, quantifier.cend(),
                           [](auto c) { return isdigit(c); });

      if (beg_it == end_it) {  // {min,}
        repeat_range.second = INT_MAX;
      } else {  // {min,max}
        repeat_range.second = stoi(string(beg_it, end_it));
      }
    }
  }

  return repeat_range;
}

StrConstIt SpecialPatternNfa::NextMatch(
        const State &state, StrConstIt str_end) {
  auto begin = state.second;

  if (begin < str_end) {
    if (characters_ == ".") {  // not new line
      if (*begin != '\n' && *begin != '\r') {
        return begin + 1;
      }
    } else if (characters_ == "\\d") {  // digit
      if (isdigit(*begin)) {
        return begin + 1;
      }
    } else if (characters_ == "\\D") {  // not digit
      if (!isdigit(*begin)) {
        return begin + 1;
      }
    } else if (characters_ == "\\s") {  // whitespace
      if (isspace(*begin)) {
        return begin + 1;
      }
    } else if (characters_ == "\\S") {  // not whitespace
      if (!isspace(*begin)) {
        return begin + 1;
      }
    } else if (characters_ == "\\w") {  // word
      if (isalnum(*begin)) {
        return begin + 1;
      }
    } else if (characters_ == "\\W") {  // not word
      if (!isalnum(*begin)) {
        return begin + 1;
      }
    } else if (characters_ == "\\t") {  // \t
      if (*begin == '\t') {
        return begin + 1;
      }
    } else if (characters_ == "\\n") {  // \n
      if (*begin == '\n') {
        return begin + 1;
      }
    } else if (characters_ == "\\v") {  // \v
      if (*begin == '\v') {
        return begin + 1;
      }
    } else if (characters_ == "\\f") {  // \f
      if (*begin == '\f') {
        return begin + 1;
      }
    } else if (characters_ == "\\0") {  // \0
      if (*begin == '\0') {
        return begin + 1;
      }
    } else {  // \\^ \\$ \\\\ \\. \\* \\+ \\? \\( \\) \\[ \\] \\{ \\} \\|
      if (*begin == characters_[1]) {
        return begin + 1;
      }
    }
  }
  // TODO(dxy): \\c \\x \\u

  return begin;
}

RangeNfa::RangeNfa(const string &regex) {
  auto begin = regex.cbegin() + 1, end = regex.cend() - 1;

  if (*begin == '^') {  // [^...]
    except_ = true;
    begin++;
  } else {
    except_ = false;
  }

  while (begin != end) {
    if (*begin == '\\') {  // skip special patterns
      auto tmp_it = SkipEscapeCharacters(begin, end);
      special_patterns_.emplace_back(string(begin, tmp_it));
      begin = tmp_it;
    } else if (*begin == '.') {
      special_patterns_.emplace_back(string(begin, begin + 1));
      begin++;
    } else {
      if (begin + 1 < end && *(begin + 1) == '-') {  // range
        if (begin + 2 < end) {
          ranges_.insert({*begin, *(begin + 2)});
          begin += 3;
        }
      } else {  // single character
        ranges_.insert({*begin, *(begin + 1)});
        begin++;
      }
    }
  }
}

StrConstIt RangeNfa::NextMatch(const State &state, StrConstIt str_end) {
  auto begin = state.second;

  if (begin == str_end) {  // no character to match
    return begin;
  }

  if (except_) {
    for (auto &range:ranges_) {
      if (*begin >= range.first && *begin <= range.second) {
        return begin;
      }
    }

    for (auto special_pattern:special_patterns_) {
      begin = special_pattern.NextMatch(state, str_end);
      if (begin != state.second) {
        return state.second;
      }
    }

    return begin + 1;
  } else {
    for (auto &range:ranges_) {
      if (*begin >= range.first && *begin <= range.second) {
        return begin + 1;
      }
    }

    for (auto special_pattern:special_patterns_) {
      begin = special_pattern.NextMatch(state, str_end);
      if (begin != state.second) {
        return begin;
      }
    }

    return begin;
  }
}
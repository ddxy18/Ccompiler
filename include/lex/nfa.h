//
// Created by dxy on 2020/8/13.
//

#ifndef CCOMPILER_NFA_H
#define CCOMPILER_NFA_H

#include <map>
#include <memory>
#include <ostream>
#include <stack>
#include <string>
#include <vector>

#include "token.h"

namespace CCompiler {
    class LexAstNode;

    using LexAstNodePtr = std::unique_ptr<LexAstNode>;
    using StrConstIt = std::string::const_iterator;

    class Nfa {
        friend std::ostream &operator<<(std::ostream &os, const Nfa &nfa);

        friend std::istream &operator>>(std::istream &is, Nfa &nfa);

    public:
        Nfa() = default;

        /**
         * Combine NFAs created from several regex rules to a final NFA.
         * Notice that the final NFA can have several accept states.
         *
         * @param nfa_vec every NFA in it is created from a regex rule, say
         * created from 'Nfa(const std::string &, std::string)'. Caller
         * should ensure that all nfas in 'nfa_vec' is not empty.
         */
        explicit Nfa(std::vector<Nfa> nfa_vec);

        /**
         * Build a partial NFA for a regex type. The final NFA combines all
         * regexes together cannot use this constructor to generate. Notice
         * that if 'regex' is invalid, it creates an empty NFA.
         *
         * @param regex
         * @param regex_type
         */
        Nfa(const std::string &regex, int regex_type, int priority);

        /**
         * Read an NFA from a file named 'nfa_file_name'. NFA is stored in
         * the file in exchange table format. Moreover, we store accept state
         * number and split alphabet table blocks separately. Notice that
         * this file should be generated from 'WriteNfa(std::string)' to
         * ensure its valid format.
         *
         * @param nfa_file
         * @return
         */
        static Nfa ReadNfa(const std::string &nfa_file);

        /**
         * Use 'delim' to split an alphabet table to several ranges. By
         * default, every range in an alphabet table is a character.
         *
         * @param delim An empty vector means use default split mode.
         * @return initial value of 'char_set_'
         */
        static std::vector<int> CharRangesInit(
                const std::vector<std::string> &delim);

        void WriteNfa(const std::string &nfa_file) const;

        /**
         * // TODO(dxy): modify function description
         *
         *
         * @param begin Iterator of given string. If we successfully find a
         * token, then 'begin' points to the beginning of the next token. If
         * no valid token exists in given string, 'begin' stays still.
         * @param end The caller should use 'cend()' as the actual parameter.
         * @return A token. The caller should use 'IsValidToken()' to check
         * the return value.
         */
        [[nodiscard]] Token NextToken(StrConstIt &begin, StrConstIt &end);

        /**
         *
         * @param state state number
         * @return If 'state' is an accept state, return its type number. If
         * 'state' is an intermediate state, return 0.
         */
        int GetStateType(int state);

        bool IsEmptyNfa() {
            return accept_states_.empty();
        }

    private:
        /**
         * Build a NFA according to an AST. If 'ast_head' is an invalid AST,
         * it creates an empty NFA.
         *
         * @param ast_head can be nullptr
         */
        Nfa(LexAstNodePtr ast_head, std::vector<int> &char_ranges);

        /**
         * Add edges and states stored in 'nfa.exchange_map_' to
         * 'this->exchange_map_'.
         *
         * @param nfa After calling the function, 'nfa.exchange_map_' turns
         * to unknown states and should never be accessed.
         * @return
         */
        Nfa &operator+=(Nfa &nfa);

        /**
         * Parse a regex string to an AST. We assume that regex can only
         * include several valid elements:
         * single character
         *  *
         *  [...]
         *  |
         *  & (It is implicit in a regex string but we need to explicitly
         *  deal with them when parsing a regex)
         *  (...)  (not mean capture groups and assertion, just change priority)
         *  escaped characters
         *
         * @param regex
         * @return If 'regex' is in valid format, return an pointer to the head
         * of AST. Otherwise return 'nullptr'.
         */
        static LexAstNodePtr ParseRegex(const std::string &regex);

        /**
         * Get all reachable states starting from 'cur_state' through '*cur_it'.
         * '*cur_it' should be matched firstly before matching any empty edges.
         * And states that can be accessed through empty edges from currently
         * matched states should also be marked reachable.
         *
         * @param cur_state starting state number
         * @param cur_it Can be any position from 'cbegin()' to 'cend()'.
         * After executing the function, 'cur_it' points to the next
         * character to be matched.
         * @param end should always be 'cend()' of a string
         * @return A vector stores all reachable states from 'cur_state'. If
         * no edges can be reached, it returns a empty vector.
         */
        std::vector<int> NextState(
                int cur_state, StrConstIt cur_it, StrConstIt end);

        /**
         * Add a new state to 'exchange_map_'. Now it has no edges. Its
         * number is determined by 'i_' and 'i_' increase 1 when a new state
         * is added.
         *
         * @param size how many character ranges
         */
        void AddState(unsigned long size);

        int GetRegexPriority(int accept_state) {
            return accept_states_[accept_state].second;
        }

        /**
         * Find 'c' in which character range in the 'char_set'.
         *
         * @param c
         * @param char_ranges
         * @return If 'c' is in range i, return 'char_ranges[i]'.
         */
        static int GetCharLocation(char c, const std::vector<int> &char_ranges);

        /**
         * Record several continuous character ranges. Range i refers to
         * ['char_ranges_[i]', 'char_ranges_[i + 1]').
         */
        std::vector<int> char_ranges_;

        /**
         * We use 'i_' to generate a new state number. It will be increased
         * after creating a new state.
         */
        static int i_;

        /**
         * pair.first -- state number
         * pair.second -- alphabet map
         * pair.second.first -- a pair in 'char_set_'
         * pair.second.second -- reachable state numbers if input is in the
         * range of 'pair.second.first'
         */
        std::map<int, std::vector<std::vector<int>>> exchange_map_;

        int begin_state_number_{};

        /**
         * Since accept states and regex rules have a 1-1 relationship, so we
         * store additional information of a regex rule together with accept
         * states.
         *
         * pair.first -- state number
         * pair.second.first -- terminal symbol type
         * pair.second.second -- terminal symbol priority
         */
        std::map<int, std::pair<int, int>> accept_states_;
    };

    class LexAstNode {
        friend class Nfa;

    public:
        explicit LexAstNode(std::string regex) : regex_(std::move(regex)) {}

        void SetLeftSon(LexAstNodePtr left_son) {
            left_son_ = std::move(left_son);
        }

        void SetRightSon(LexAstNodePtr right_son) {
            right_son_ = std::move(right_son);
        }

        std::string GetRegex() {
            return regex_;
        }

    private:
        std::string regex_;
        LexAstNodePtr left_son_;
        LexAstNodePtr right_son_;
    };

    std::ostream &operator<<(std::ostream &os, const Nfa &nfa);

    std::istream &operator>>(std::istream &is, Nfa &nfa);
}

#endif //CCOMPILER_NFA_H

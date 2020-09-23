//
// Created by dxy on 2020/8/6.
//

#ifndef CCOMPILER_NFA_H
#define CCOMPILER_NFA_H

#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace CCompiler {
    class RegexAstNode;

    class SpecialPatternNfa;

    class RangeNfa;

    using RegexAstNodePtr = std::unique_ptr<RegexAstNode>;
    using StrConstIt = std::string::const_iterator;
    // pair.first -- state
    // pair.second -- current begin iterator
    using ReachableStatesMap = std::set<std::pair<int, StrConstIt>>;
    using State = std::pair<int, StrConstIt>;
    using StatePtr = std::unique_ptr<State>;
    // pair.first -- type
    // pair.second -- current begin iterator
    using AcceptState = State;

    enum class Encoding {
        kAscii, kUtf8
    };

    /**
     * We split a regex to several parts and classify them to types in
     * RegexPart.
     *
     * kAnd('&': it is implicit in the regex)
     * kChar(including [...], escape character and single character)
     * kQuantifier('*' '?' '+' and their non-greedy mode, '{...}')
     * kAlternative('|')
     * kCapture('(...)' or '(?:...)')
     * kAssertion('(?=...)', '(?!...)', '^', '$', '\b', '\B')
     * kError(invalid token)
     */
    enum class RegexPart {
        kAnd, kChar, kQuantifier, kAlternative, kPassiveGroup, kError
    };

    using StrConstIt = std::string::const_iterator;

    class Nfa {
        friend class NfaFactory;

    public:
        /**
         * Combine several regex rules to a final NFA. Notice that the final
         * NFA can have several accept states.
         *
         * @param regex_rules
         */
        explicit Nfa(const std::map<std::string, int> &regex_rules);

        /**
         * Build a NFA for 'regex'. Notice that if 'regex' is invalid, it
         * creates an empty NFA.
         *
         * @param regex
         */
        explicit Nfa(const std::string &regex, int type,
                     const std::vector<unsigned int> &char_ranges);

        /**
         * Get the next match in a given string in the range of [begin, end).
         * Notice that it matches from begin, say, the successful match must
         * be [begin, any location no more than end).
         *
         * @param begin First iterator of the given string. If we successfully
         * find a token, 'begin' will be moved to the beginning of the first
         * character after the match.
         * @param end Last iterator of the given string.
         * @return A matched substring. If no match exists, it returns "".
         */
        StatePtr NextMatch(StrConstIt begin, StrConstIt end);

        /**
         * It is used to determine whether a NFA is a valid NFA.
         *
         * @return
         */
        [[nodiscard]] bool Empty() const {
            return accept_states_.empty();
        }

    protected:
        enum class StateType {
            kSpecialPattern, kRange, kCommon
        };

        static const int kEmptyEdge = 0;

        Nfa() = default;

        /**
         * Build a NFA according to an AST. If ast_head is an invalid AST or
         * a nullptr pointer, it creates an empty NFA.
         *
         * @param ast_head can be nullptr
         */
        Nfa(RegexAstNodePtr &ast_head,
            const std::vector<unsigned int> &char_ranges);

        /**
         * It copies exchange_map_, assertion_states_, not_assertion_states_
         * and group_states_ to the new NFA.
         *
         * @param nfa After calling the function, nfa.exchange_map_ becomes
         * undefined and should never be accessed again.
         * @return
         */
        Nfa &operator+=(Nfa &nfa);

        /**
         * It tries to move to the next state until to the end of the string or
         * no next states can be found.
         *
         * @param begin
         * @param end
         * @return all possible routines
         */
        std::vector<ReachableStatesMap> StateRoute(
                StrConstIt begin, StrConstIt end);

        /**
         * Get all reachable states starting from cur_state. When cur_state
         * is a functional state, it will be handled first. For common
         * states, we match *begin first. Then we deal with states that can
         * be reached through empty edges.
         *
         * @param cur_state
         * @param str_end
         * @return reachable states after handling cur_state
         */
        ReachableStatesMap NextState(
                const State &cur_state, StrConstIt str_end);

        /**
         * All reachable states starting from cur_state through empty edges.
         * Notice that cur_state is excluded and it does nothing to handle
         * functional states.
         *
         * @param cur_state
         * @return
         */
        ReachableStatesMap NextState(const State &cur_state);

        StateType GetStateType(int state);

        /**
         * Use 'delim' to split an encoding to several ranges.
         *
         * @param delim You should ensure it contains valid character classes.
         * @param encoding
         */
        void CharRangesInit(
                const std::vector<std::string> &delim, Encoding encoding);

        /**
         * Find which character range in the char_ranges_ c is in.
         *
         * @param c must be in the range of the current encoding
         * @return range index in char_ranges_ where c is in
         */
        int GetCharLocation(int c);

        /**
         * Parse a regex to an AST. We assume that regex can only include
         * several valid elements:
         * characters(include single character, escape character and '[...]')
         * quantifiers
         * |
         * & (It is implicit in a regex string but we need to explicitly deal
         * with them when parsing a regex)
         * groups
         * assertions
         *
         * All characters should be represented as a node, say, ranges and
         * escape characters are seen as single character here. Moreover, groups
         * and lookaheads will add a flag node to the head to remember its type.
         *
         * @param regex
         * @return If regex is in a valid format, return the head of AST.
         * Otherwise return nullptr.
         */
        static RegexAstNodePtr ParseRegex(const std::string &regex);

        /**
         * Add a new state to exchange_map_. Now it has no edges.
         */
        void NewState();

        /**
         * We use i_ to generate a new state. First state should have a id 1.
         * After that, it will be increased when creating a new state. i_
         * points to the lastly added state at any time after creating the
         * first state.
         */
        static int i_;

        /**
         * Record several continuous character ranges. Ranges are stored
         * orderly. Range i refers to [char_ranges_[i], char_ranges_[i + 1]).
         * Since characters with code of 0-4 are almost never appears in
         * text, we use them to represent special edges. And char_ranges_
         * should coverage every character in the given encoding.
         */
        std::vector<unsigned int> char_ranges_;

        /**
         * map.first -- state
         * map.second.element -- reachable states from an input character that
         * is in the corresponding range
         */
        std::map<int, std::vector<std::set<int>>> exchange_map_;

        std::map<int, SpecialPatternNfa> special_pattern_states_;

        std::map<int, RangeNfa> range_states_;

        int begin_state_{-1};
        /**
         * map.first -- accept state
         * map.second -- regex type
         */
        std::map<int, int> accept_states_;
    };

    /**
     * Escape characters, special meaning escape characters and back-reference.
     */
    class SpecialPatternNfa {
    public:
        explicit SpecialPatternNfa(std::string characters) :
                characters_(std::move(characters)) {}

        /**
         * Determine whether a substring can match characters_.
         *
         * @param begin
         * @param str_end
         * @return If a substring [begin, end_it) matches, return end_it.
         * Otherwise return begin.
         */
        StrConstIt NextMatch(const State &state, StrConstIt str_end);

    private:
        std::string characters_;
    };

    /**
     * [...]. It can contain single characters, ranges and special patterns.
     */
    class RangeNfa {
    public:
        explicit RangeNfa(const std::string &regex);

        /**
         * @param begin
         * @param str_end
         * @return If a substring [begin, end_it) matches, return end_it.
         * Otherwise return begin.
         */
        StrConstIt NextMatch(const State &state, StrConstIt str_end);

    private:
        std::map<int, int> ranges_;
        std::vector<SpecialPatternNfa> special_patterns_;
        bool except_;  // true for [^...] and false for [...]
    };

    /**
     * Provide a function to create a sub-NFA for common operators in the
     * RegexPart. It's usually used to create NFAs for split parts.
     */
    class NfaFactory {
    public:
        static Nfa MakeCharacterNfa(
                const std::string &characters,
                const std::vector<unsigned int> &char_ranges);

        static Nfa MakeAlternativeNfa(Nfa left_nfa, Nfa right_nfa);

        static Nfa MakeAndNfa(Nfa left_nfa, Nfa right_nfa);

        static Nfa MakeQuantifierNfa(const std::string &quantifier,
                                     RegexAstNodePtr &left,
                                     const std::vector<unsigned int> &char_ranges);

    private:
        static std::pair<int, int> ParseQuantifier(
                const std::string &quantifier);
    };

    /**
     * Node in AST that is used as a intermediate format of a regex. It helps
     * to show the regex structure more clearly and reduce the work to build
     * the NFA. AST for regex is designed as a binary tree, so every AstNode
     * has at most two son nodes.
     */
    class RegexAstNode {
        friend class Nfa;

    public:
        RegexAstNode(RegexPart regex_type, std::string regex) :
                regex_type_(regex_type), regex_(std::move(regex)) {}

        void SetLeftSon(RegexAstNodePtr left_son) {
            left_son_ = std::move(left_son);
        }

        void SetRightSon(RegexAstNodePtr right_son) {
            right_son_ = std::move(right_son);
        }

        [[nodiscard]] RegexPart GetType() const {
            return regex_type_;
        }

    private:
        RegexPart regex_type_;
        std::string regex_;
        RegexAstNodePtr left_son_;
        RegexAstNodePtr right_son_;
    };

    std::ostream &operator<<(std::ostream &os, const Nfa &nfa);

    std::istream &operator>>(std::istream &is, Nfa &nfa);
}

#endif // CCOMPILER_NFA_H
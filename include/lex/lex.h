//
// Created by dxy on 2020/8/13.
//

#ifndef CCOMPILER_LEX_H
#define CCOMPILER_LEX_H

#include <fstream>
#include <map>
#include <string>
#include <vector>

#include "lex/nfa.h"

namespace Ccompiler {
    class LexRule {
        friend Nfa NfaInit(const std::string &lex_file_name);

    public:
        LexRule(int priority, std::string name, std::string regex) :
                priority_(priority), name_(std::move(name)),
                regex_(std::move(regex)) {}
        /**
         * Extract all character expressions from 'regex_'(including '[...]',
         * single character, some escaped characters and '.').
         *
         * @return
         */
        // TODO(dxy): It's used to split an alphabet table. Now we only
        //  support that every character is seen as a range.
//        std::vector<std::string> GetCharaters();

    private:
        /**
         * Used to determine which regex to match when several regexes can
         * match the given string.
         */
        int priority_;
        std::string name_;  // regex type name
        std::string regex_;  // regex contents

        // TODO(dxy): Now we only support token type names and regex rules in
        //  lex files. We maybe add support for regex actions later.
        /**
         * Every time we find a token satisfies the rule, we execute
         * 'action()' to complete additional work.
         *
         * @return lex type
         */
//        int (*action)();
    };

    /**
     * We use contents stored in a file named 'lex_file_name' to generate a
     * NFA. The file should have a format like:
     * Constants(token type name) definition
     * regex type 1  regex rules
     * regex type 2  regex rules
     *
     * @param lex_file_name a file stores several regex rules to distinguish
     * lex.
     * @return
     */
    Nfa NfaInit(const std::string &lex_file_name);
}

#endif //CCOMPILER_LEX_H

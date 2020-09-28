//
// Created by dxy on 2020/8/25.
//

#ifndef CCOMPILER_ENVIRONMENT_H
#define CCOMPILER_ENVIRONMENT_H

#include <map>

#include "lex/lexer.h"

namespace CCompiler {
    class Nfa;

    class Environment {
    public:
        static void EnvironmentInit() {
            Lexer::nfa_ = Nfa{regex_rules_};
        }

        static int IntSymbol(const std::string &symbol);

        static std::string StrSymbol(int symbol);

    private:
        /**
         * map symbols from string to integer
         */
        static std::map<std::string, int> symbol_map_;

        /**
         * map regex rules from string to integer
         */
        static std::map<std::string, int> regex_rules_;
    };
}


#endif // CCOMPILER_ENVIRONMENT_H
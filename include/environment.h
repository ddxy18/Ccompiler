//
// Created by dxy on 2020/8/25.
//

#ifndef CCOMPILER_ENVIRONMENT_H
#define CCOMPILER_ENVIRONMENT_H

#include <map>

namespace CCompiler {
    class Nfa;

    class Environment {
    public:
        /**
         * Initialize symbol_map_ and NFA stores in Lexer.
         *
         * @param lex_file must exist
         */
        explicit Environment(const std::string &lex_file);

        static int IntSymbol(const std::string &symbol);

        static std::string StrSymbol(int symbol);

    private:
        /**
         * map symbols from string to integer
         */
        static std::map<std::string, int> symbol_map_;
    };
}


#endif // CCOMPILER_ENVIRONMENT_H
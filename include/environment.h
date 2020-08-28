//
// Created by dxy on 2020/8/25.
//

#ifndef CCOMPILER_ENVIRONMENT_H
#define CCOMPILER_ENVIRONMENT_H

#include <algorithm>
#include <map>

namespace CCompiler {
    /**
     * Special terminal symbols. THey are useful in syntax analysis.
     */
    const int kError = -1;
    const int kEmpty = 0;
    const int kEnd = 1;

    class Environment {
    public:
        /**
         * Initialize 'terminal_symbol_map_' and NFA stores in 'Lexer'.
         *
         * @param lex_file must exist
         * @param nfa_file can be a nonexistent file
         * @param grammar_file must exist
         */
        explicit Environment(std::string lex_file, std::string nfa_file,
                             std::string grammar_file);

        static int IntSymbol(const std::string &symbol);

        static std::string StrSymbol(int symbol);

    private:
        /**
         * Initialize user-defined and reserved symbols.
         */
        void SymbolMapInit();

        /**
         * map symbol names to numbers
         */
        static std::map<std::string, int> symbol_map_;

        /**
         * A file stores NFA for Lexer to use.
         */
        const std::string nfa_file_;
        /**
         * A file stores user-defined terminal symbols' regex rules.
         */
        const std::string lex_file_;

        /**
         * A file stores grammars for the language.
         */
        const std::string grammar_file_;
    };
}


#endif //CCOMPILER_ENVIRONMENT_H

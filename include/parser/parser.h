//
// Created by dxy on 2020/8/22.
//

#ifndef CCOMPILER_PARSER_H
#define CCOMPILER_PARSER_H

#include <map>
#include <set>
#include <vector>

#include "ast/ast.h"
#include "ast/declaration.h"
#include "ast/type.h"
#include "lex/token.h"
#include "lex/lexer.h"

namespace Ccompiler {
    class Parser {
    public:
        Parser(int start_symbol, const std::string &source_file) :
                start_symbol_(start_symbol), lexer_(source_file) {}

        /**
         * Initialize 'grammar_map_'.
         *
         * @param grammar_file A file stores the grammar which is defined by
         * bison like syntax rules.
         */
        static void GrammarMapInit(const std::string &grammar_file);

        virtual AstNodePtr NewNode() = 0;

        virtual AstNodePtr NewNode(int symbol) = 0;

    protected:
        static bool IsTerminalSymbol(int symbol) {
            return grammar_map_[symbol].empty();
        }

        /**
         * pair.first -- symbol
         * pair.second.element -- production
         * pair.second.element.element -- symbol in a generator
         */
        static std::map<int, std::vector<std::vector<int>>> grammar_map_;

        int start_symbol_;

        Lexer lexer_;
    };

    class LL1Parser : public Parser {
    public:
        LL1Parser(int start_symbol, const std::string &source_file) :
                Parser(start_symbol, source_file) {
            PredictTableInit();
        }

        AstNodePtr NewNode() override {
            return NewNode(start_symbol_);
        }

        AstNodePtr NewNode(int symbol) override;

    private:
        void PredictTableInit();

        std::set<int> First(int symbol);

        std::set<int> First(std::vector<int> symbols);

        std::map<int, std::set<int>> Follow();

        /**
         * map.first -- non-terminal symbol
         * map.second.first -- input symbol
         * map.second.second -- generator sequence in 'grammar_map_'
         */
        std::map<int, std::map<int, int>> predict_table_;
    };
}

#endif //CCOMPILER_PARSER_H

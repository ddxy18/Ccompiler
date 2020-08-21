//
// Created by dxy on 2020/8/20.
//

#ifndef CCOMPILER_LEXER_H
#define CCOMPILER_LEXER_H

#include <fstream>

#include "lex/nfa.h"

namespace Ccompiler {
    class Lexer {
    public:
        explicit Lexer(const std::string &source_file) :
                line_(0), column_(0), source_file_stream_(source_file) {}

        virtual ~Lexer() {
            source_file_stream_.close();
        }

        /**
         * It gets a token from 'source_file_stream_' and do some actions
         * according to the token's type. It can automatically exclude some
         * useless and invalid tokens.
         *
         * @param nfa the NFA tool to find a token
         * @return If no valid token remains, it returns an empty token.
         */
        Token NextToken(Nfa &nfa);

    private:
        Token NextTokenInLine(std::string::const_iterator &begin,
                              std::string::const_iterator &end, Nfa &nfa);

        std::ifstream source_file_stream_;
        int line_;
        int column_;
    };
}

#endif //CCOMPILER_LEXER_H

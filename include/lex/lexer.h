//
// Created by dxy on 2020/8/20.
//

#ifndef CCOMPILER_LEXER_H
#define CCOMPILER_LEXER_H

#include <fstream>
#include <queue>

#include "lex/nfa.h"
#include "lex/token.h"

namespace CCompiler {
    class Lexer {
        friend class Environment;

    public:
        explicit Lexer(std::streambuf &source_file) :
                line_(0), column_(0), source_file_stream_(&source_file) {}

        /**
         * Get and consume the next token.
         *
         * @return
         */
        Token Next();

        /**
         * Get but not consume the next token.
         *
         * @return
         */
        Token Peek();

    private:
        /**
         * It gets a token from source_file_stream_. It can automatically
         * exclude some useless and invalid tokens.
         *
         * @return If no valid token remains, it returns an empty token.
         */
        Token NextToken();

        Token NextTokenInLine(StrConstIt &begin, StrConstIt &end);

        static Nfa nfa_;

        std::istream source_file_stream_;
        int line_;
        int column_;
        // store tokens that are got but not consumed immediately
        std::queue<Token> tokens_;
    };
}

#endif //CCOMPILER_LEXER_H

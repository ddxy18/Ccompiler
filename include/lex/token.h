//
// Created by dxy on 2020/8/13.
//

#ifndef CCOMPILER_TOKEN_H
#define CCOMPILER_TOKEN_H

#include <map>
#include <memory>
#include <string>

namespace CCompiler {
    class Token {
    public:
        explicit Token(std::string token = "", int type = 0,
                       int line = 0, int column = 0) :
                token_(std::move(token)), type_(type),
                line_(line), column_(column) {}

        /**
         * Check 'Token.token_' to determine whether it's a valid token.
         *
         * @return
         */
        bool IsEmptyToken() {
            return token_.empty();
        }

        // TODO(dxy): see TODO in lex/lex.h line 42
//        static void AddType(std::string name, int (*)());

        // TODO(dxy): see TODO in lex/lex.h line 42
        /**
         * Execute a function corresponding to 'reg_type_' to do some
         * additional work(e.g. set 'lex_type_').
         */
//        void ExecTokenAction();

        [[nodiscard]] std::string GetToken() {
            return token_;
        }

        [[nodiscard]] int GetType() const {
            return type_;
        }

        [[nodiscard]] int GetLine() const {
            return line_;
        }

        [[nodiscard]] int GetColumn() const {
            return column_;
        }

        void SetLine(int line) {
            line_ = line;
        }

        void SetColumn(int column) {
            column_ = column;
        }

        void SetToken(const std::string &token) {
            token_ = token;
        }

    private:
        // TODO(dxy):see TODO in lex/lex.h line 42
        /**
         * Map a regex type name to a function pointer which is executed
         * after finding a token.
         */
//        static std::map<std::string, int (*)()> reg_to_action_;

        std::string token_;  // the matched string
        int type_;  // terminal symbol type
        // record token location in the source file
        int line_;
        int column_;
    };
}

#endif //CCOMPILER_TOKEN_H

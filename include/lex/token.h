//
// Created by dxy on 2020/8/13.
//

#ifndef CCOMPILER_TOKEN_H
#define CCOMPILER_TOKEN_H

#include <map>
#include <memory>
#include <string>

namespace Ccompiler {
    class Token;

    using TokenPtr = std::unique_ptr<Token>;

    class Token {
    public:
        Token(std::string token, std::string type)
                : token_(std::move(token)), type_(std::move(type)) {}

        static void AddTokenType(std::string name, int number) {
            token_types_.insert({number, std::move(name)});
        }

        /**
         * Check 'Token.token_' to determine whether it's a valid token.
         *
         * @return
         */
        bool IsValidToken() {
            return !token_.empty();
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

        [[nodiscard]] const std::string &GetType() const {
            return type_;
        }

    private:
        // map a number to a token type name
        static std::map<int, std::string> token_types_;

        // TODO(dxy):see TODO in lex/lex.h line 42
        /**
         * Map a regex type name to a function pointer which is executed
         * after finding a token.
         */
//        static std::map<std::string, int (*)()> reg_to_action_;

        std::string token_;  // the matched string
        std::string type_;  // regex type name
    };
}

#endif //CCOMPILER_TOKEN_H

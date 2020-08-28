//
// Created by dxy on 2020/8/22.
//

#ifndef CCOMPILER_AST_H
#define CCOMPILER_AST_H

#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "environment.h"

namespace CCompiler {
    class AstNode;

    using AstNodePtr = std::shared_ptr<AstNode>;

    class AstNode {
    public:
        AstNode(std::string token, int symbol) :
                token_(std::move(token)), symbol_(symbol) {}

        void AddNode(const AstNodePtr &new_node) {
            son_vec_.push_back(new_node);
        }

        [[nodiscard]] int GetSymbol(int n) const {
            return symbol_;
        }

        [[nodiscard]] std::string GetSymbol(const std::string &s) const {
            return Environment::StrSymbol(symbol_);
        }

        [[nodiscard]] const std::vector<AstNodePtr> &GetSonVec() const {
            return son_vec_;
        }

        [[nodiscard]] const std::string &GetToken() const {
            return token_;
        }

    private:
        std::vector<AstNodePtr> son_vec_;
        std::string token_;
        int symbol_;
    };
}

#endif //CCOMPILER_AST_H

//
// Created by dxy on 2020/8/22.
//

#ifndef CCOMPILER_DECLARATION_H
#define CCOMPILER_DECLARATION_H

#include <memory>

#include "ast/statement.h"
#include "ast/translation_unit.h"

namespace CCompiler {
class Expr;

class Initializer;

class Object;

class Type;

/**
 * @brief declarations for objects
 *
 * It contains information about a initializer if it is given explicitly in
 * the code. Decl is inherited from Stmt so we can combine the declarations
 * and statements in a block, which simplifies the structure for CompoundStmt.
 */
class Decl : public Stmt, public ExternalDef {
 public:
  Decl(std::unique_ptr<Object> object, std::unique_ptr<Initializer> initializer)
          : object_(std::move(object)),
            initializer_(std::move(initializer)) {}

 private:
  std::unique_ptr<Object> object_;
  std::unique_ptr<Initializer> initializer_;
};

class Initializer {
 private:
  std::list<std::unique_ptr<Expr>> initializers_;  // list is for {} initializer
};
}

#endif // CCOMPILER_DECLARATION_H
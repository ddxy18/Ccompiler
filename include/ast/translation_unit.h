//
// Created by dxy on 2020/10/24.
//

#ifndef CCOMPILER_TRANSLATION_UNIT_H
#define CCOMPILER_TRANSLATION_UNIT_H

#include <list>
#include <memory>

namespace CCompiler {
class Decl;

class Function;

using DeclList = std::list<std::unique_ptr<Decl>>;
using FuncList = std::list<std::unique_ptr<Function>>;

/**
 * A translation unit contains either declarations(have richer semantics than
 * our Decl class,including object, function, struct/union/enum structure) or
 * function definitions.
 */
class TranslationUnit {
 private:
  DeclList decl_list_;
  FuncList func_list_;
};
}

#endif // CCOMPILER_TRANSLATION_UNIT_H
//
// Created by dxy on 2020/10/24.
//

#ifndef CCOMPILER_TRANSLATION_UNIT_H
#define CCOMPILER_TRANSLATION_UNIT_H

#include <list>

namespace CCompiler {
class Decl;

/**
 * A translation unit contains either declarations(have richer semantics than
 * our Decl class,including object, function, struct/union/enum structure) or
 * function definitions.
 */
class TranslationUnit {
 public:
  void AddExternalDef(Decl *decl) {
    decls_.push_back(decl);
  }

 private:
  std::list<Decl *> decls_;
};
}

#endif // CCOMPILER_TRANSLATION_UNIT_H
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

class ExternalDef {
};

using ExternalDefList = std::list<std::unique_ptr<ExternalDef>>;

/**
 * A translation unit contains either declarations(have richer semantics than
 * our Decl class,including object, function, struct/union/enum structure) or
 * function definitions.
 */
class TranslationUnit {
 private:
  ExternalDefList external_defs_;
};
}

#endif // CCOMPILER_TRANSLATION_UNIT_H
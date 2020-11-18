//
// Created by dxy on 2020/11/12.
//

#include "gtest/gtest.h"
#include "parser/parser.h"

using namespace CCompiler;
using namespace std;

// TODO(dxy): override == operator for all classes in ast
void TestAst(const string &source, TranslationUnit *expected_trans_unit) {
  Parser parser(source);
  auto trans_unit = parser.Parse();
  EXPECT_EQ(trans_unit, expected_trans_unit);
}

TEST(Parser, EmptyMain) {
  auto trans_unit = new TranslationUnit();
  trans_unit->AddExternalDef(new FuncDecl(
          new Function(new QualType(QualType::Specifier::kInt, 0),
                       Identifier::Linkage::kNone,
                       "main",
                       Function::ParamList(),
                       new CompoundStmt(new Scope(Scope::ScopeType::kBlock,
                                                  trans_unit->GetScope())))));

  TestAst("int main() {}", trans_unit);
}

TEST(Parser, UninitializedObj) {
  auto trans_unit = new TranslationUnit();
  trans_unit->AddExternalDef(new ObjectDecl(
          new Object(new Identifier(new QualType(
                  QualType::Specifier::kInt | QualType::Specifier::kUnsigned,
                  0),
                                    Identifier::Linkage::kNone,
                                    "main"),
                     0),
          nullptr));

  TestAst("unsigned int i;", trans_unit);
}

TEST(Parser, InitializedObj) {
  auto trans_unit = new TranslationUnit();


  TestAst("int i = 0;", trans_unit);
}
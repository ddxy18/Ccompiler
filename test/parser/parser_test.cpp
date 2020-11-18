//
// Created by dxy on 2020/11/12.
//

#include "gtest/gtest.h"
#include "parser/parser.h"

using namespace CCompiler;
using namespace std;

/**
 * It tests whether a tu constructed from source is equal to the
 * expected_trans_unit.
 * @param source
 * @param expected_trans_unit
 */
void TestAst(const string &source, TranslationUnit *expected_trans_unit) {
  Parser parser(source);
  auto trans_unit = parser.Parse();
  EXPECT_TRUE(trans_unit->Equal(expected_trans_unit));
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
          new Object(new Identifier(
                  new QualType(QualType::Specifier::kInt |
                               QualType::Specifier::kUnsigned,
                               0),
                  Identifier::Linkage::kNone,
                  "i"),
                     0),
          nullptr));

  // TODO(dxy): decl equal jumps to func in Decl
  TestAst("unsigned int i;", trans_unit);
}

TEST(Parser, InitializedObj) {
  auto trans_unit = new TranslationUnit();
  trans_unit->AddExternalDef(new ObjectDecl(
          new Object(
                  new Identifier(new QualType(QualType::Specifier::kInt,
                                              Qualifier::kEmpty),
                                 CCompiler::Identifier::Linkage::kNone,
                                 "i"),
                  0),
          new BaseInitializer(0, new Constant(0))));

  TestAst("int i = 0;", trans_unit);
}


// test statements and declarations in a function body, we use main here.
class FuncBodyTest : public ::testing::Test {
 protected:
  void SetUp() override {
    tu_ = new TranslationUnit();
    tu_->AddExternalDef(new FuncDecl(
            new Function(new QualType(QualType::Specifier::kInt, 0),
                         Identifier::Linkage::kNone,
                         "main",
                         Function::ParamList(),
                         new CompoundStmt(new Scope(Scope::ScopeType::kBlock,
                                                    tu_->GetScope())))));
  }

  TranslationUnit *tu_;
  //!< All tests should call body_->AddStmt() to complete the remaining
  // construction of the tu_.
  CompoundStmt *body_;
};

TEST_F(FuncBodyTest, ForStmt) {
  auto obj = new Object(
          new Identifier(new QualType(QualType::Specifier::kInt, 0),
                         Identifier::Linkage::kNone, "i"),
          0);
  auto obj_decl = new ObjectDecl(obj, new BaseInitializer(0, new Constant(0)));
  auto for_scope = new Scope(Scope::ScopeType::kBlock, body_->GetOwnedScope());
  for_scope->AddIdent(obj_decl);
  auto for_stmt =
          new ForStmt(for_scope,
                      StmtList(),
                      StmtList{obj_decl},
                      new BinaryExpr(TokenType::kLess, obj, new Constant(100)),
                      new UnaryExpr(TokenType::kIncrement, obj));

  body_->AddStmt(for_stmt);

  TestAst("int main() {"
          "for (int i = 0; i < 100; ++i) {}"
          "}",
          tu_);
}
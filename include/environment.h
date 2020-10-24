//
// Created by dxy on 2020/8/25.
//

#ifndef CCOMPILER_ENVIRONMENT_H
#define CCOMPILER_ENVIRONMENT_H

#include <map>

namespace CCompiler {
enum class TokenType;

class Environment {
 public:
  static void EnvironmentInit();

 private:
  /**
   * map regex rules from string to integer
   */
  static std::map<std::string, TokenType> regex_rules_;
};
}

#endif // CCOMPILER_ENVIRONMENT_H
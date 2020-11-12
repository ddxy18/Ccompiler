//
// Created by dxy on 2020/11/13.
//

#ifndef CCOMPILER_LIST_UTIL_H
#define CCOMPILER_LIST_UTIL_H

#include <list>

namespace CCompiler {
/**
 * It works like == operator. The only difference is that we compare elements
 * in the two lists by checking the two objects pointed by the pointer in the
 * corresponding location instead of checking the two pointer's addresses.
 * @tparam T
 * @param l_list
 * @param r_list
 * @return
 */
template<typename T>
bool Equal(const std::list<T *> &l_list, const std::list<T *> &r_list) {
  if (l_list.size() == r_list.size()) {
    auto l_it = l_list.begin(), r_it = r_list.begin();
    for (int i = 0; i < l_list.size(); ++i) {
      if (typeid(**l_it) != typeid(**r_it) || **l_it != **r_it) {
        return false;
      }
      l_it++;
      r_it++;
    }
    return true;
  }
  return false;
}
}

#endif // CCOMPILER_LIST_UTIL_H
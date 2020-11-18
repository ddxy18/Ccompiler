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
 * @tparam T If T is a class, the user must ensure it has overrode operator ==
 * and !=
 * @param l_list
 * @param r_list
 * @return
 */
// TODO(dxy): If T is a pointer type, it will not work correctly.
template<typename T>
bool Equal(const std::list<T *> &l_list, const std::list<T *> &r_list) {
  if (l_list.size() == r_list.size()) {
    auto l_it = l_list.begin(), r_it = r_list.begin();
    for (int i = 0; i < l_list.size(); ++i) {
      // check for nullptr
      if ((*l_it == nullptr) ^ (*r_it == nullptr)) {
        return false;
      }
      if (*l_it != nullptr) {
        if (typeid(**l_it) != typeid(**r_it) || **l_it != **r_it) {
          return false;
        }
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
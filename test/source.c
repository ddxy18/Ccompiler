//
// Created by dxy on 2020/8/27.
//
struct a{};
int main() {
  struct a{};
  int res = 0;
  int a[2];
  for (int i = 0; i < 100; ++i) {
    res += i;
  }
  return 0;
}
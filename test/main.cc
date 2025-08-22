#include <iostream>
int main() {
  const int y = 5;
  auto& d = y;  // d 是 const int& 类型（引用必须与原对象的 const 性一致）
  std::cout << "Value of d: " << d << std::endl;
  return 0;
}
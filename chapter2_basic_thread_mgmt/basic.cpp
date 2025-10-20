#include <iostream>
#include <thread>

int main() {
  // int v = 0;
  auto th = std::thread([&]() {
      int a = 0;
      std::cout << a++ << std::endl;
      if (a == 10)
        return;
  });
  th.detach();
  return 0;
}

#include <iostream>
#include <thread>

void thread_func() {
  std::cout << "Hello world from the thread" << std::endl;
}

int main() {
  std::cout << "Hello from the main thread" << std::endl;
  auto th = std::thread(thread_func);
  th.join();
  return 0;
}

#include <iostream>
#include <future>

int find_ans(std::string msg) {
  // Data race in cout exists
  // -fsanitize=thread will report data race due to cout buffer being mixed in threads
  // this will produce cout result mixed between threads which is not that bad for us here
  // so ignoring the data race
  std::cout << "Looking for an answer with message: " << msg << std::endl;
  return 42;
}

int main() {
  std::cout << "starting main" << std::endl;
  std::future<int> fut1 = std::async(std::launch::async, find_ans, "async launch");
  std::future<int> fut2 = std::async(std::launch::deferred, find_ans, "deferred launch");
  std::cout << "doing other things" << std::endl;
  std::cout << "Obtained answer from async: " << fut1.get() << std::endl;
  std::cout << "Obtained answer from deferred: " << fut2.get() << std::endl;
  std::cout << "Exiting" << std::endl;
  return 0;
}

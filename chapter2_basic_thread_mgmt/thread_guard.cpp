#include <iostream>
#include <thread>

class ThreadGuard {
  public:
    explicit ThreadGuard(std::thread& th): th(th) {}
    ~ThreadGuard() {
      std::cout << "Destrructing ThreadGuard" << std::endl;
      if (th.joinable()) {
        std::cout << "Let's wait for the running thread to finish first" << std::endl;
        th.join();
        std::cout << "Okay the thread has finished. Cleaning ThreadGuard" << std::endl;
      }
    }
    // don't want any implicit copy consttuctor and copy assignment that will be automatically definied by the compiler
    ThreadGuard(const ThreadGuard&) = delete;
    ThreadGuard& operator=(ThreadGuard const&) = delete;
  private:
    std::thread& th;
};


int main() {
  int v = 0;
  auto t = std::thread([&]() {
      while (v != 10) {
        std::cout << "from thread: " << v++ << std::endl;
      }
  });
  auto it = ThreadGuard {t};
  std::cout << "doing something independent in main thread" << std::endl;
  return 0;
}

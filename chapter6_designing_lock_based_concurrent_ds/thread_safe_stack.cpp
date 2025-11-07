#include <iostream>
#include <stack>
#include <thread>
#include <mutex>

using namespace std;

template<typename T>
class threadsafe_stack {
public:
  threadsafe_stack() {}

  threadsafe_stack(const threadsafe_stack& other) {
    lock_guard lk(other.mtx);
    data = other.data;
  }

  threadsafe_stack& operator=(const threadsafe_stack&) = delete;

  void push(T d) {
    lock_guard lk(mtx);
    data.push(move(d));
  }

  shared_ptr<T> pop() {
    lock_guard lk(mtx);
    if (data.empty()) throw "can't pop from empty stack";
    auto ret = make_shared<T>(move(data.top()));
    data.pop();
    return ret;
  }

  void pop(T& retval) {
    lock_guard lk(mtx);
    if (data.empty()) throw "can't pop from empty stack";
    retval = move(data.top());
    data.pop();
  }

private:
  stack<T> data;
  mutex mtx;

};


int main() {
  return 0;
}

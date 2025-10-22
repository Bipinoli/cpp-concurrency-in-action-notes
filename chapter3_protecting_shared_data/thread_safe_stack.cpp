#include <iostream>
#include <thread>
#include <mutex>
#include <stack>
#include <memory>

using namespace std;

template<typename T> 
class ThreadSafeStack {
public:
  // Rule of 5
  // Define no life cycle functions or all 
  // i.e destructor, copy constructor, move constructor, copy assignment operator, move assignment operator
  ThreadSafeStack() = default;
  ~ThreadSafeStack() = default;
  ThreadSafeStack(const ThreadSafeStack&) = delete;
  ThreadSafeStack(const ThreadSafeStack&&) = delete;
  ThreadSafeStack& operator=(const ThreadSafeStack&) = delete;
  ThreadSafeStack& operator=(const ThreadSafeStack&&) = delete;
  bool empty() {
    const lock_guard<mutex> lock(mtx);
    return stk.empty();
  }
  void push(T val) {
    const lock_guard<mutex> lock(mtx);
    stk.push(val);
  }
  shared_ptr<T> pop() {
    // Return value optimization (RVO) is not guaranteed in all cases
    // So the value is returned by calling the move constructor if possible 
    // otherwise with copy constructor
    // Such constructors may try to allocate memory in the heap for the returned object
    // The memory alloation in the heap can fail (not enough memory)
    // We lose data from the stack ff we have already popped the data by then 
    // Therefore we allocate the independent memory in heap for the returning object
    // before popping from the stack
    // If the allocation fails we don't mess the stack as the function throws without popping
    // We can send the shared_ptr to the returning object in the heap so that the calller can
    // assign the value the many variables otherwise unique_ptr could also be used
    const lock_guard<mutex> lock(mtx);
    // using move for efficiency incase the type supports move
    // otherwise even with std::move it will be a copy constructor
    shared_ptr<T> val = make_shared<T>(move(stk.top()));
    stk.pop();
    return val; // copy/mo
  } 
private:
  mutex mtx;
  stack<T> stk;
};


void producer(ThreadSafeStack<int>& stk) {
  for (int i=0; i<20; i++) {
    stk.push(i);
  }
}

void consumer(ThreadSafeStack<int>& stk) {
  for (int i=0; i<20; i++) {
    if (!stk.empty()) {
      // Note: prone to race condition
      // What if the stack is popped in another thread 
      // after chekcing empty() here?
      // In this particular example only one consumer exists so no problem.
      cout << "stack pop: " << *stk.pop() << endl;
    }
  }
}

int main() {
  ThreadSafeStack<int> stk;
  auto t1 = thread([&]() {
    producer(stk);
  });
  auto t2 = thread([&]() {
    consumer(stk);
  });
  t1.join();
  t2.join();
  return 0;
}

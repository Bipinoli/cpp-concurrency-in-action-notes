#include <iostream>
#include <thread>
#include <mutex>

using namespace std;

void init_func(bool shouldThrow) {
  if (shouldThrow) {
    cout << "Init func throwing exception in thread: " << this_thread::get_id() << endl;
    throw exception();
  } 
  cout << "Initialized successfully in thread: " << this_thread::get_id() << endl;
}

void thread_func(once_flag& flag, bool shouldThrow) {
  // Note:
  // call_once guarantees to call the function only once even when called by multiple threads
  // It maintains a flag (std::once_flag) to check if the function has been already called
  // It is very useful for init functions such as establihsing a shared db connection across thread etc.
  // The library call handles the thread safety and also avoids mutex lock when the function has already been called
  while (true) {
    try {
      call_once(flag, init_func, shouldThrow);
      break;
    } catch(...) {
      // loop until try succeds
    }
  }
  cout << "Running thread func in thread: " << this_thread::get_id() << endl; 
}


int main() {
  once_flag call_once_flag;
  auto t1 = thread([&](){
    thread_func(call_once_flag, true);
  });
  auto t2 = thread([&](){
    thread_func(call_once_flag, false);
  });
  auto t3 = thread([&](){
    thread_func(call_once_flag, false);
  });
  auto t4 = thread([&](){
    thread_func(call_once_flag, false);
  });

  t1.join();
  t2.join();
  t3.join();
  t4.join();

  return 0;
}

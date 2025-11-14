#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

using namespace std;

template<typename T>
class threadsafe_queue {
public:
  threadsafe_queue() {}
  threadsafe_queue(const threadsafe_queue&) = delete;
  threadsafe_queue& operator=(const threadsafe_queue&) = delete;

  void push(T d) {
    // allocation happens here outside of lock
    shared_ptr<T> v(make_shared<T>(move(d)));
    {
      lock_guard lk(mtx);
      data.push(v);
    }
    // better to notify after the lock has been released
    // otherwise the notification would reach but the lock is not ready yet
    // causing a little contention
    cd.notify_one();
  }

  shared_ptr<T> pop() {
    unique_lock lk(mtx);
    // this is roughly equivalent to 
    // while(!predicate) {
    //    cd.wait(lk);
    // }
    // so the predicate is checked before releasing the lock and waiting
    cd.wait(lk, [this](){ return !data.empty(); });
    auto ret = data.front();
    data.pop();
    return ret;
  }

private:
  queue<shared_ptr<T>> data;
  mutex mtx;
  condition_variable cd;
};


int main() {
  return 0;
}

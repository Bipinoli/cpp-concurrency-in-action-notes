#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <chrono>
#include <random>

using namespace std;

void random_sleep() {
  static thread_local mt19937 gen(random_device{}());
  uniform_int_distribution<int> dist(100, 1000);
  int sleep_ms = dist(gen);
  this_thread::sleep_for(chrono::milliseconds(sleep_ms));
}

void producer(queue<int>& msg, mutex& mtx, condition_variable& cond) {
  for (int i=0; i<10; i++) {
    {
      lock_guard lk(mtx);
      msg.push(i);
      cout << "Producer pushed message: " << i << endl;
    }
    cond.notify_one();
  }
  lock_guard lk(mtx);
  msg.push(-1); // kill message
  cout << "Producer pushed kill message: -1" << endl;
  cond.notify_all();
}

void consumer(int id, queue<int>& msg, mutex& mtx, condition_variable& cond) {
  while (true) {
    random_sleep();
    unique_lock lk(mtx);
    // Note:
    // Atomic release of lock and wait operation.
    // Atomicity is logically enforced by kernel.
    // We need unique_lock here as we do lock & unlock multiple times.
    // The thread could be awaken by spurious wakeups so the condition function is very important to distinguish that.
    // Spurious wakeups are due to kernel level optimizations and other things so the wakeup happens without notify_one or notify_all.
    // When multiple threads are waiting the mutex enforeces a serial wakeup as only one thread can acquire a lock.
    // And the condition function allows to know if the work has already been done by earlier thread.
    cond.wait(lk, [&]() { return !msg.empty(); });
    int m = msg.front();
    if (m == -1) {
      cout << "Killing consumer " << id << endl;
      break;
    }
    msg.pop();
    cout << "Consumer " << id << " consumed message: " << m << endl;
  }
}



int main() {
  queue<int> msg;
  mutex mtx;
  condition_variable cond;

  auto pd = thread([&]() {
    producer(msg, mtx, cond);
  });
  vector<thread> cons;
  for (int i=0; i<5; i++) {
    cons.push_back(thread([&, i]() {
      consumer(i, msg, mtx, cond); 
    }));
  }

  // With .join we block until the thread finishes.
  // So should only be called in the end.
  pd.join();
  for (thread& con: cons) {
    con.join();
  }

  return 0;
}

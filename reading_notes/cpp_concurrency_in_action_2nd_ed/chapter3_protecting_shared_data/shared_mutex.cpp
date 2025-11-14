#include <iostream>
#include <thread>
#include <shared_mutex>
#include <mutex>
#include <vector>
#include <random>
#include <chrono>
#include <unistd.h>

using namespace std;

struct Data {
  int val;
  thread::id modified_by;
};

void random_sleep() {
  static thread_local mt19937 gen(random_device{}());
  uniform_int_distribution<int> dist(100, 1000);
  int sleep_ms = dist(gen);
  this_thread::sleep_for(chrono::milliseconds(sleep_ms));
}

void reader(shared_mutex& mtx, const Data& data) {
  // Note: 
  // shared_mutex allows for multiple readers but only one writer.
  // Reading and writing is isolated i.e no reading & writing at the same time.
  // Reading lock can be obtaiend by using shared_lock RAII wrapper.
  // Writing lock can be obtained by using unique_lock RAII wrapper.
  // Note that the shared_mutex standard doesn't guarantee any fairness or starvation guarantee.
  // So a writer will likely keep waiting for a long time because there are so many readers coming in.
  random_sleep();
  shared_lock lk(mtx);
  cout << "Reader thread: " << this_thread::get_id() << " read value: " << data.val << " that was modified by the writer thread: " << data.modified_by << endl;
} 

void writer(shared_mutex& mtx, Data& data, int d) {
  random_sleep();
  unique_lock lk(mtx);
  data.val = d;
  data.modified_by = this_thread::get_id();
  cout << "Writer thread: " << this_thread::get_id() << " modified the value to: " << d << endl;
}


int main() {
  Data data {
    .val = 1,
    .modified_by = this_thread::get_id()
  };

  shared_mutex mtx;

  vector<thread> threads; 
  for (int i=0; i<10; i++) {
    threads.push_back(thread([&](){
      reader(mtx, data);
    }));
  }

  for (int i=0; i<5; i++) {
    // capturing i by value to avoid data-race of i in writer threads
    threads.push_back(thread([&mtx, &data, i]() {
      writer(mtx, data, i);
    }));
  }
  
  for (thread& t: threads) {
    t.join();
  }

  return 0;
}


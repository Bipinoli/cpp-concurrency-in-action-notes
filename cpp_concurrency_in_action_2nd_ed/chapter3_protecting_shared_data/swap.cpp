#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <random>

using namespace std;

struct PersonData {
  string name;
  int age;
  string message;
  mutable mutex m; // to allow modificatication even when the struct is const
};

int get_random_time() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dist(100, 1000); // 100-1000 ms
  return dist(gen);
}

void printer(const PersonData& p1, const PersonData& p2) {
  for (int i=0; i<10; i++) {
    int t = get_random_time();
    this_thread::sleep_for(std::chrono::milliseconds(t));
    // Note:
    // scoped_lock is a good alternative to std::lock() + lock_guard() with adopt_lock
    // For eg:
    // std::lock(m1, m2);
    // std::lock_guard g1(m1, std::adopt_lock);
    // std::lock_guard g2(m2, std::adopt_lock);
    //
    // Here we acquire multiple locks with std::lock 
    // std::lock uses deadlock prevention algorithm
    // for example: by using try_lock() and releasing acquired lock when one lock can't be acquired
    // and waiting again
    // but before waiting it advices the kernel to reschedule other threads to avoid big busy_wait condition
    // using this_thread::yield()
    // some other deadlock prevention algorithm do exponential backoff.
    // After acquiring all locks the lock_guard is built
    // The adopt_lock flag indicates that the thread also has the mutex.
    // When the guard goes out of scope the lock is realesed by destructor
    // the std::lock and std::lock_guard don't throw exception so this is a safe order
    // Any excpetion throwing code must be called after having a lock_guard
    //
    // Another modernalternative to lock_guard + adopt_lock + lock 
    // is using unique_lock with defer_lock flag + std::lock
    // For eg:
    // std::unique_lock l1 {mutex1, std::defer_lock};
    // std::unique_lock l2 {mutex2, std::defer_lock}
    // std::lock(l1, l2);
    //
    // This way we build a guard before calling the lock
    // std::defer_lock defers the acquire of lock for later i.e for std::lock
    auto lk = scoped_lock(p1.m, p2.m);
    cout << p1.name << " says '" << p1.message << "' to " << p2.name << endl;
  }
}

void swapper(PersonData& p1, PersonData& p2) {
  for (int i=0; i<10; i++) {
    int t = get_random_time();
    this_thread::sleep_for(std::chrono::milliseconds(t));
    auto lk = scoped_lock(p1.m, p2.m);
    auto temp = p1.message;
    p1.message = p2.message;
    p2.message = temp;
    cout << "swapped" << endl;
  } 
}



int main() {
  PersonData p1 {
    .name = "Lisa",
    .age = 33,
    .message = "I like tea",
  };

  PersonData p2 {
    .name = "Bipin",
    .age = 28,
    .message = "I like coffee",
  };

  auto t1 = thread([&]() {
    printer(p1, p2);
  });
  auto t2 = thread([&]() {
    swapper(p1, p2);
  });

  t1.join();
  t2.join();

  return 0;
}

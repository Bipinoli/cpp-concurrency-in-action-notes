#include <iostream>
#include <cassert>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <queue>
#include <random>
#include <functional>
#include "sequential.hpp"
#include "test.hpp"

using namespace std;


int main() {
  test::test_quicksort(sequential::quicksort_sequential);
  // vector<int> nums {1, 15, 4, 13, 2, 18, 3, 12, 20, 6, 14, 17, 8, 9, 10, 5, 7, 11, 19, 16};
  //
  // queue<Task> task_q;
  // vector<thread> workers;
  //
  // for (unsigned int i=0; i < thread::hardware_concurrency() - 1; i++) {
  //   workers.push_back(thread([&](){
  //
  //   }));
  // }
  //
  // for (thread& w: workers) {
  //   w.join();
  // }
  //

  return 0;
}

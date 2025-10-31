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
#include "concurrent.hpp"
#include "test.hpp"

using namespace std;


int main() {
  vector<vector<int>> nums_batch {
    {1, 15, 4, 13, 2, 18, 3, 12, 20, 6, 14, 17, 8, 9, 10, 5, 7, 11, 19, 16},
    {30, 25, 40, 35, 20, 45, 10, 50, 5, 55, 15, 60, 0, 70, 65, 80, 75, 90, 85, 100}
  };

  concurrent::QuicksortWorkers workers;
  workers.sort_batch(nums_batch); 

  workers.keep_alive();

  return 0;
}

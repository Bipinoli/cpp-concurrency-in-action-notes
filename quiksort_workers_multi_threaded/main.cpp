#include <iostream>
#include <vector>
#include <chrono>
#include "sequential.hpp"
#include "concurrent.hpp"
#include "test.hpp"

using namespace std;



int main() {
  test::test_sequential();
  test::test_concurrent();
  cout << "--------------------------------" << endl;

  cout  << "Performance comparison between sequential quicksort and concurrent quicksort:" << endl;

  concurrent::QuicksortWorkers workers;
  test::RandomGenerator rand_gen;

  for (int i=0; i<4; i++) {
    vector<vector<int>> nums_batch1;
    int nums_in_batch = 0;
    for (int i=0; i<100; i++) {
      const int sz = rand_gen.generate_random_number(1, 100000);
      nums_in_batch += sz;
      nums_batch1.push_back(rand_gen.generate_random_vector(sz, 1, 1000));
    }
    vector<vector<int>> nums_batch2(nums_batch1);
    cout << "Sorting batch #" << i << " with " << nums_in_batch << " numbers in total" << endl;

    auto conc_start = chrono::high_resolution_clock::now();
    workers.sort_batch(nums_batch1);
    auto conc_end = chrono::high_resolution_clock::now();
    auto conc_duration = chrono::duration_cast<chrono::milliseconds>(conc_end - conc_start);

    auto seq_start = chrono::high_resolution_clock::now();
    sequential::quicksort_sequential_batch(nums_batch2);
    auto seq_end = chrono::high_resolution_clock::now();
    auto seq_duration = chrono::duration_cast<chrono::milliseconds>(seq_end - seq_start);

    cout << "Sorting completed by sequential quicksort in " << seq_duration.count() << " ms" << endl;
    cout << "Sorting completed by concurrent quicksort with " << workers.number_of_workers() << " workers in " << conc_duration.count() << " ms" << endl;
  }

  workers.kill_workers();

  return 0;
}

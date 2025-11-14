#pragma once

#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <atomic>
#include <cassert>
#include <stdexcept>
#include <optional>

namespace concurrent {

using namespace std;


struct Task {
  int start_index;
  int end_index;
  vector<int>& nums;
  bool poison;
};

struct PivotResult {
  bool pivoted;
  int pivot_boundry_left;
  int pivot_boundry_right;
};


PivotResult arrange_around_pivot(const int start, const int end, vector<int>& nums);

class QuicksortWorkers {
public:

  QuicksortWorkers() {
    for (int i=0; i < thread::hardware_concurrency() - 1; i++) {
      workers.push_back(thread([this](){
        worker();
      }));
    }
  }

  int number_of_workers() {
    return workers.size();
  }

  void kill_workers() {
    {
      lock_guard lk(task_q_mtx);
      for (size_t i=0; i < workers.size(); i++) {
        task_q.push({
          .start_index = -1,
          .end_index = -1,
          .nums = *(new vector<int>()),
          .poison = true
        });
      }
    }
    // wait until all workers are killed
    for (auto& t: workers) {
      t.join();
    } 
  }

  void sort_batch(vector<vector<int>>& nums_batch) {
    {
      lock_guard lk(batch_mtx);
      if (in_progress_tasks > 0) {
        throw runtime_error("Exsiting batch hasn't finished");
      }
    }
    {
      lock_guard lk(task_q_mtx);
      assert(task_q.empty());
      in_progress_tasks = nums_batch.size();
      for (vector<int>& nums: nums_batch) {
        task_q.push({
          .start_index = 0,
          .end_index = static_cast<int>(nums.size() - 1),
          .nums = nums,
          .poison = false
        });
      }  
    }
    {
      // wait until batch finishes
      unique_lock lk(batch_mtx);
      batch_cv.wait(lk, [this](){
        return in_progress_tasks == 0;
      });
    }

  }


private:
  vector<thread> workers;
  queue<Task> task_q;
  mutex task_q_mtx;
  atomic<int> in_progress_tasks;
  mutex batch_mtx;
  condition_variable batch_cv;;

  void worker() {
    while (true) {
      optional<Task> task_opt;
      {
        lock_guard lk(task_q_mtx);
        if (!task_q.empty()) {
          // create in-place with emplace because Task can't be default constructed 
          // due to nums being a refernece
          task_opt.emplace(task_q.front());
          task_q.pop();
        }
      }
      if (!task_opt.has_value()) {
        this_thread::yield(); 
        continue;
      }
      Task task = task_opt.value();
      if (task.poison) {
        break;
      }
      auto pivot_rslt = concurrent::arrange_around_pivot(task.start_index, task.end_index, task.nums);
      if (pivot_rslt.pivoted) {
        scoped_lock lk(task_q_mtx, batch_mtx);
        task_q.push({
          .start_index = task.start_index,
          .end_index = max(pivot_rslt.pivot_boundry_left, task.start_index),
          .nums = task.nums,
          .poison = false
        });
        task_q.push({
          .start_index = min(pivot_rslt.pivot_boundry_right, task.end_index),
          .end_index = task.end_index,
          .nums = task.nums,
          .poison = false
        });
        in_progress_tasks += 1;
      } else {
        // the update of atomic can take some time until published to all threads
        // so when we notify the condition variable the update may not be available yet.
        // To avoid this we should update the atomic while holding the mutex related to the condition variable.
        {
          lock_guard lk(batch_mtx);
          in_progress_tasks -= 1;
        }
        batch_cv.notify_one();
      }
    }
  }

};


void swap(int a, int b, vector<int>& nums) {
  int temp = nums[a];
  nums[a] = nums[b];
  nums[b] = temp;
}


PivotResult arrange_around_pivot(const int start, const int end, vector<int>& nums) {
  assert(start <= end);
  if (start == end) {
    return {
      .pivoted = false,
      .pivot_boundry_left = -1,
      .pivot_boundry_right = -1
    };
  }
  if (start + 1 == end) {
    if (nums[end] < nums[start]) {
      swap(start, end, nums);
    }
    return {
      .pivoted = false,  
      .pivot_boundry_left = -1,
      .pivot_boundry_right = -1
    };
  }
  int pivot = nums[(start + end) / 2];

  // invariant:
  // left of l is strictly smaller than pivot
  // right or r is strictly greater than pivot
  int l = start;
  int r = end;

  int i = l;
  while (i <= r) {
    if (nums[i] < pivot) {
      swap(i, l, nums);
      l++; i++;
    } else if (nums[i] > pivot) {
      swap(i, r, nums);
      r--;
    } else {
      i++;
    }
  }

  return {
    .pivoted = true,
    .pivot_boundry_left = l - 1,
    .pivot_boundry_right = r + 1
  };
}


}

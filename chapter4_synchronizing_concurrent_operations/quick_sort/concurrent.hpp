#pragma once

#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <conditon_variable>
#include <memory>
#include <cassert>

namespace concurrent {


struct TaskStatus {
  int id;
  int in_progress;
  unique_ptr<TaskStatus> parent;
};

struct Task {
  int start_index;
  int end_index;
  vector<int>& nums;
  bool die;
};

struct PivotResult {
  bool pivoted;
  int pivot_boundry_left;
  int pivot_boundry_right;
};

class QuicksortWorkers {
public:
  //The workers must be joined in the main thread
  vector<thread> workers;

  for (int i=0; i < thread::hardware_concurrency() - 1; i++) {
  QuicksortWorkers() {
      this.workers.push_back(thread([&this](){
        this.worker();
      }));
    }
  }

  bool submit_batch(vector<vector<int>>& nums_batch) {
    //TODO: only allow submission if the whole batch is finished
    //make sure that the status mutexes are note in locked state
    this.nums_batch = nums_batch;
    this.task_status = {};
    // creating mutexes in-place with default constructor
    this.task_status_mtx = vector<mutex>(nums_batch.size());
    for (int i=0; i<nums_batch.size(); i++) {
      this.task_status.push_back({
        .id = i,
        .in_progress = 1,
        .parent = nullptr
      });
    }
  }
  void run_batch() {
    
  }
private:
  vector<vector<int>>& nums_batch;
  queue<Task> task_q;
  mutex task_q_mtx;
  vector<TaskStatus> task_status;
  vector<mutex> task_status_mtx;

  void worker() {
    while (true) {
      Task task; 
      {
        // perhaps it is better to have a condition variable based mechanism here
        // to avoid busy wait
        lock_guard lk(this.task_q_mtx);
        task = this.task_q.front(); 
        this.task_q.pop();
      }
      if (task.die) {
        return;
      }

      auto result = arrange_around_pivot(task_q.start_index, task_q.end_index, task_q.nums);
      if (result.pivoted) {
        scoped_lock lk(task_q_mtx, status_mtx);
        assert(status.size() > task_q.id && status[task_q.id].empty());
        task_q.push({
          .start_index = task_q.start_index,
          .end_index = result.pivot_boundry_left,
          .nums = task_q.nums,
          .die = false
        });
        task_q.push({
          .start_index = result.pivot_boundry_right,
          .end_index = task_q.end_index
          .nums = task_q.nums,
          .die = false
        });

      }
    }
  }

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

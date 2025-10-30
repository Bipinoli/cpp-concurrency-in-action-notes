#pragma once

#include <iostream>
#include <vector>
#include <queue>
#include <mutex>
#include <conditon_variable>
#include <memory>

namespace concurrent {

struct Task {
  int start_index;
  int end_index;
  vector<int>& nums;
  bool die;
};

struct TaskStatus {
  int in_progress;
  unique_ptr<TaskStatus> parent;
};

struct PivotResult {
  bool pivoted;
  int pivot_boundry_left;
  int pivot_boundry_right;
};


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

void notify_task_finish() {

}

void quicksort(queue<Task>& task_q, mutex& task_q_mtx) {
  while (true) {
    Task task; 
    {
      lock_guard lk(task_q_mtx);
      task = task_q.front(); 
      task_q.pop();
    }
    if (task.die) {
      return;
    }

    auto result = arrange_around_pivot(task_q.start_index, task_q.end_index, task_q.nums);
    if (result.pivoted) {
      
    }

  }
}

}

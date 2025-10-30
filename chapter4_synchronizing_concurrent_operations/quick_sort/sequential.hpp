#pragma once 

#include <iostream>
#include <cassert>
#include <vector>
#include <random>

namespace sequential {

using namespace std;

void swap(int a, int b, vector<int>& nums);


void quicksort_sequential(int start, int end, vector<int>& nums) {
  assert(start <= end);
  if (start == end) {
    return;
  }
  if (start + 1 == end) {
    if (nums[end] < nums[start]) {
      swap(start, end, nums);
    }
    return;
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
  quicksort_sequential(start, max(l-1, start), nums);
  quicksort_sequential(min(r+1, end), end, nums);
}


void swap(int a, int b, vector<int>& nums) {
  int temp = nums[a];
  nums[a] = nums[b];
  nums[b] = temp;
}

}

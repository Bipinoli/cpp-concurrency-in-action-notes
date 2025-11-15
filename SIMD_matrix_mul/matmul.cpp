#include <iostream>
#include <vector>
#include <cassert>
#include <random>
#include <chrono>
#include <functional>
#include <iomanip>
#include <sstream>
#include <immintrin.h>

using namespace std;


struct Matrix {
  int rows; 
  int cols;
  vector<int> data;
};


class RandomGen {
public:
  RandomGen(): rand_eng(random_device{}()), uniform_distrib(1,1000) {
  }
  vector<int> gen_vector(int n) {
    vector<int> ret(n);
    for (int i=0; i<n; i++) {
      ret[i] = uniform_distrib(rand_eng);
    }
    return ret;
  }
  int gen_int() {
    return uniform_distrib(rand_eng);
  }
private:
  default_random_engine rand_eng;
  uniform_int_distribution<int> uniform_distrib;
};



Matrix matmul(const Matrix& a, const Matrix& b) {
  assert(a.cols == b.rows);
  Matrix ans = {
    .rows = a.rows,
    .cols = b.cols,
    .data = vector<int>(a.rows * b.cols),
  };
  for (int r=0; r<a.rows; r++) {
    for (int c=0; c<b.cols; c++) {
      int s = 0;
      for (int i=0; i<a.cols; i++) {  
        s += a.data[r * a.cols + i] * b.data[i * b.cols + c];
      }
      ans.data[r * ans.cols + c] = s;
    }
  }
  return ans;
}


Matrix avx2_matmul(const Matrix& a, const Matrix& b) {
  // assuming AVX2 vectorization in intel CPUs
  // availabe AVX2 intrinsics:
  // https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#avxnewtechs=AVX2
  assert(a.cols == b.rows);
  Matrix ans = {
    .rows = a.rows,
    .cols = b.cols,
    .data = vector<int>(a.rows * b.cols)
  };
  for (int r=0; r<a.rows; r++) {
    for (int c=0; c<b.cols; c++) {
      int s = 0;
      int i = 0;
      while (i + 8 <= a.cols) {
        // for row: loading 8 contiguous int bytes from memory to 256 bit register in cpu
        __m256i rownums = _mm256_loadu_si256((__m256i *) &a.data[r * a.cols + i]);
        // for column: setting indices to gather the data from memory to the 256 bit register in cpu
        __m256i indices = _mm256_setr_epi32(0, b.cols, b.cols * 2, b.cols * 3, b.cols * 4, b.cols * 5, b.cols * 6, b.cols * 7);
        // gathering 8 ints from column in memory to the cpu register. Scaling by 4 as Ints take 4 bytes
        __m256i colnums = _mm256_i32gather_epi32((int*)&b.data[i * b.cols + c], indices, 4);
        // multiplying 8 ints at once. ignore overflow.
        __m256i mults = _mm256_mullo_epi32(rownums, colnums);

        // Pull the multipled result from CPU into memory
        // alignment ensures all the bits of the data will fit in a CPU cache, avoiding multiple pulls when the data crosses cache boundary
        alignas(32) int temp[8];
        _mm256_storeu_si256((__m256i *)temp, mults);

        // usually modern compilers can vectorize simple additions like this
        // so not manually calling the intrinsics
        s += temp[0] + temp[1] + temp[2] + temp[3] + temp[4] + temp[5] + temp[6] + temp[7];
        i += 8;
      }
      while (i < a.cols) {
        s += a.data[r * a.cols + i] * b.data[i * b.cols + c];
        i++;
      }
      ans.data[r * ans.cols + c] = s;
    }
  }
  return ans;
}


Matrix avx2_matmul2(const Matrix& a, const Matrix& b) {
  assert(a.cols == b.rows);
  // Main improvement: Transpose Matrix B
  // such that the column data would be contiguous
  // With it the column data better fits in the same cache line
  // Also we would be able to quickly pick the data into 256 bit register in CPU instead of having to gather from all over the memory
  Matrix bt = {
    .rows = b.cols,
    .cols = b.rows,
    .data = vector<int>(b.rows * b.cols)
  };
  for (int i=0; i<b.rows; i++) {
    for (int j=0; j<b.cols; j++) {
      bt.data[j * bt.cols + i] = b.data[i * b.cols + j];
    }
  }
  Matrix ans = {
    .rows = a.rows,
    .cols = b.cols,
    .data = vector<int>(a.rows * b.cols)
  };
  for (int r=0; r<a.rows; r++) {
    for (int c=0; c<b.cols; c++) {
      int s = 0;
      int i = 0;
      while (i + 8 <= a.cols) {
        __m256i rownums = _mm256_loadu_si256((__m256i *) &a.data[r * a.cols + i]);
        __m256i colnums = _mm256_loadu_si256((__m256i *) &bt.data[c * bt.cols + i]);
        __m256i mults = _mm256_mullo_epi32(rownums, colnums);

        // Pull the multipled result from CPU into memory
        // alignment ensures all the bits of the data will fit in a CPU cache, avoiding multiple pulls when the data crosses cache boundary
        alignas(32) int temp[8];
        _mm256_storeu_si256((__m256i *)temp, mults);

        // usually modern compilers can vectorize simple additions like this
        // so not manually calling the intrinsics
        s += temp[0] + temp[1] + temp[2] + temp[3] + temp[4] + temp[5] + temp[6] + temp[7];
        i += 8;
      }
      while (i < a.cols) {
        s += a.data[r * a.cols + i] * bt.data[c * bt.cols + i];
        i++;
      }
      ans.data[r * ans.cols + c] = s;
    }
  }
  return ans;
}

pair<long long, Matrix> multiply(const Matrix& a, const Matrix& b, function<Matrix(const Matrix&, const Matrix&)> mult_func) {
  auto start = chrono::high_resolution_clock::now();
  Matrix result = mult_func(a, b);
  auto end = chrono::high_resolution_clock::now();
  return {chrono::duration_cast<std::chrono::nanoseconds>(end - start).count(), result};
}


string format_duration(long long ns) {
  double millis = ns / 1'000'000'000.0;
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(2) << millis << " s";
  return oss.str();
}


int main() {
  cout << "Testing & benchmarking!!" << endl;
  RandomGen rand_gen;
  long long total_dur1 = 0;
  long long total_dur2 = 0;
  long long total_dur3 = 0;
  for (int i=0; i<20; i++) {
    int p = rand_gen.gen_int();
    int q = rand_gen.gen_int();
    int r = rand_gen.gen_int();
    Matrix ma = {
      .rows = p,
      .cols = q,
      .data = rand_gen.gen_vector(p * q),
    };
    Matrix mb = {
      .rows = q,
      .cols = r,
      .data = rand_gen.gen_vector(q * r),
    };
    auto [dur1, res1] = multiply(ma, mb, matmul);
    auto [dur2, res2] = multiply(ma, mb, avx2_matmul);
    auto [dur3, res3] = multiply(ma, mb, avx2_matmul2);

    for (int i=0; i<res1.data.size(); i++) {
      assert(res1.data[i] == res2.data[i] && res2.data[i] == res3.data[i]);
    }

    total_dur1 += dur1;
    total_dur2 += dur2;
    total_dur3 += dur3;

    std::cout << std::fixed << std::setprecision(2);
    std::cout 
        << "[" << std::setw(4) << ma.rows << ", " << std::setw(4) << ma.cols << "] X "
        << "[" << std::setw(4) << mb.rows << ", " << std::setw(4) << mb.cols << "] "
        << std::setw(14) << std::right << "Normal: " << std::setw(4) << format_duration(dur1)
        << std::setw(20) << "SIMD (gather): " << std::setw(4) << format_duration(dur2)
        << std::setw(3) <<  "(" << (double)dur1/dur2 << "x speedup)  "
        << std::setw(28) << "SIMD (with transpose): " << std::setw(4) << format_duration(dur3)
        << std::setw(3) << "(" << (double)dur1/dur3 << "x speedup)"
        << std::endl;
  };
  long long avg1 = total_dur1 / 20;
  long long avg2 = total_dur2 / 20;
  long long avg3 = total_dur3 / 20;
  std::cout << std::endl;
  std::cout << "Average" << std::endl;
  std::cout 
        << "Normal: " << std::setw(4) << format_duration(avg1)
        << std::setw(20) << "SIMD (gather): " << std::setw(4) << format_duration(avg2)
        << std::setw(3) <<  "(" << (double)avg1/avg2 << "x speedup)  "
        << std::setw(28) << "SIMD (with transpose): " << std::setw(4) << format_duration(avg3)
        << std::setw(3) << "(" << (double)avg1/avg3 << "x speedup)"
        << std::endl;

  return 0;
}

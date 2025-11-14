#include <iostream>
#include <vector>
#include <cassert>
#include <random>
#include <chrono>
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



Matrix matmul(Matrix& a, Matrix& b) {
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


Matrix avx2_matmul(Matrix& a, Matrix& b) {
  // assuming AVX2 vectorization in intel CPUs
  // availabe AVX2 intrinsics:
  // https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html#avxnewtechs=AVX2
  assert(a.cols == b.rows);
  Matrix ans = {
    .rows = a.rows,
    .cols = b.cols,
    .data = vector<int>(a.rows, b.cols)
  };
  for (int r=0; r<a.rows; r++) {
    for (int c=0; c<b.cols; c++) {
      int s = 0;
      int i = 0;
      while (i + 8 < a.cols) {
        // for row: loading 8 contiguous int bytes from memory to 256 bit register in cpu
        __m256i rownums = _mm256_load_si256((__m256i *) &a.data[r * a.cols + i]);
        // for column: setting indices to gather the data from memory to the 256 bit register in cpu
        __m256i indices = _mm256_setr_epi32(0, b.rows, b.rows * 2, b.rows * 3, b.rows * 4, b.rows * 5, b.rows * 6, b.rows * 7);
        // gathering data from memory to cpu register as pointed by indices. Scaling by 4 as Ints take 4 bytes
        __m256i colnums = _mm256_i32gather_epi32(&b.data[i * b.cols + c], indices, 4);
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
      }
      ans.data[r * ans.cols + c] = s;
    }
  }
  return matmul(a, b);
}




int main() {
  cout << "Testing & benchmarking!!" << endl;
  RandomGen rand_gen;
  for (int i=0; i<10; i++) {
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
    auto normal_start = chrono::high_resolution_clock::now();
    Matrix res1 = matmul(ma, mb);
    auto normal_end = chrono::high_resolution_clock::now();
    auto duration1 = chrono::duration_cast<std::chrono::milliseconds>(normal_end - normal_start).count();

    auto avx2_start = chrono::high_resolution_clock::now();
    Matrix res2 = avx2_matmul(ma, mb);
    auto avx2_end = chrono::high_resolution_clock::now();
    auto duration2 = chrono::duration_cast<std::chrono::milliseconds>(avx2_end - avx2_start).count();

    for (int i=0; i<res1.data.size(); i++) {
      assert(res1.data[i] == res2.data[i]);
    }
    cout << "Single core matrix mult (" << ma.rows * ma.cols << " * " << mb.rows * mb.cols << ") took " << duration1 << " ms in normal form & " << duration2 << " ms in SIMD form. SIMD version is " << (double) duration1 / (double)duration2 << " times faster." << endl;
  };

  return 0;
}

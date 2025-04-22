#include <cuda_runtime_api.h>

#include <cassert>

#include "gemm_gpu_tiling.h"

constexpr int TILE_SIZE = 32;  // 共享内存中分块(tiling), 减少全局内存访问次数

// gemm_gpu_tiling - GEMM on GPU, using tiling & shared memory to optimize
// global memory accesses
__global__ void gemm_gpu_tiling_kernel(
    int* __restrict__ C,        // [n, m], on gpu
    const int* __restrict__ A,  // [n, k], on gpu
    const int* __restrict__ B,  // [k, m], on gpu
    const int n, const int m, const int k) {
  // We copy the tile from a/b into shared memory, and then do the calculation
  __shared__ int a_tile[TILE_SIZE][TILE_SIZE];
  __shared__ int b_tile[TILE_SIZE][TILE_SIZE];
  int my_c_result = 0;
  for (int tile_index = 0; tile_index < k / TILE_SIZE; ++tile_index) {
    // Step 1. Load the tile from a/b into a/b_tile
    a_tile[threadIdx.y][threadIdx.x] =
        A[(blockIdx.x * TILE_SIZE + threadIdx.y) * k +
          (tile_index * TILE_SIZE + threadIdx.x)];
    b_tile[threadIdx.y][threadIdx.x] =
        B[(tile_index * TILE_SIZE + threadIdx.y) * m +
          (blockIdx.y * TILE_SIZE + threadIdx.x)];
    __syncthreads();  // 同步线程块内的所有线程，准备处理下一个分块
    // Step 2. Calculate the contribution to my_c_result
    for (int i = 0; i < TILE_SIZE; ++i) {
      my_c_result += a_tile[threadIdx.y][i] * b_tile[i][threadIdx.x];
    }
    __syncthreads();
  }
  // Step 3. Store my_c_result
  C[(blockIdx.x * TILE_SIZE + threadIdx.y) * m +
    (blockIdx.y * TILE_SIZE + threadIdx.x)] = my_c_result;
}

/// 我称之为Helper
void gemm_gpu_tiling(int* __restrict__ C,        // [n, m], on gpu
                     const int* __restrict__ A,  // [n, k], on gpu
                     const int* __restrict__ B,  // [k, m], on gpu
                     const int n, const int m, const int k) {
  assert(n % TILE_SIZE == 0);
  assert(m % TILE_SIZE == 0);
  assert(k % TILE_SIZE == 0);  // 这里断言，确定了分块的合法性
  dim3 grid_dim = dim3(n / TILE_SIZE,
                       m / TILE_SIZE);  // 到底有多少个block? 根据Tile_size决定
  dim3 block_dim =
      dim3(TILE_SIZE, TILE_SIZE);  // 一个block中有(TILE_SIZE, TILE_SIZE)个线程
  gemm_gpu_tiling_kernel<<<grid_dim, block_dim>>>(C, A, B, n, m, k);
}

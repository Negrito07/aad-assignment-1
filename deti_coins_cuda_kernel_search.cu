//
// Gabriel Negri, Marco Magalhães, December 2024
//
// Arquiteturas de Alto Desempenho 2024/2025
//
// DETI coins search CUDA kernel code
//
// deti_coins_cuda_kernel_search() --- each thread tries 1 potential DETI coins
//

//
// needed stuff
//

typedef unsigned int u32_t;
typedef unsigned int u08_t;

#include "md5.h"

//
// the nvcc compiler stores x[] and state[] in registers (constant indices!)
//
// global thread number: n = threadIdx.x + blockDim.x * blockIdx.x
// global warp number: n >> 5
// warp thread number: n & 31
//

extern "C" __global__ __launch_bounds__(128,1) void deti_coins_cuda_kernel_search(u32_t *deti_coins, u32_t v1, u32_t v2)
{
  u32_t n,a,b,c,d,coin[13],hash[4],state[4],x[16];

  //
  // get the global thread number
  //
  n = (u32_t)threadIdx.x + (u32_t)blockDim.x * (u32_t)blockIdx.x;
  //
  // create DETI coin for thread n
  //
  coin[0] = ('I' << 24) | ('T' << 16) | ('E' << 8) | 'D';
  coin[1] = ('i' << 24) | ('o' << 16) | ('c' << 8) | ' ';
  coin[2] = (' ' << 24) | (' ' << 16) | (' ' << 8) | 'n';
  coin[3] = (' ' << 24) | (' ' << 16) | (' ' << 8) | ' ';
  coin[4] = v1;
  coin[5] = v2;
  coin[6] = (' ' << 24) | (' ' << 16) | (' ' << 8) | ' ';
  coin[7] = (' ' << 24) | (' ' << 16) | (' ' << 8) | ' ';
  coin[8] = (' ' << 24) | (' ' << 16) | (' ' << 8) | ' ';
  coin[9] = (' ' << 24) | (' ' << 16) | (' ' << 8) | ' ';
  coin[10] = (' ' << 24) | (' ' << 16) | (' ' << 8) | ' ';
  coin[11] = (' ' << 24) | (' ' << 16) | (' ' << 8) | ' ';
  coin[12] = ('\n' << 24) | (' ' << 16) | (' ' << 8) | ' ';

  coin[3] += (n % 64) << 0;
  n /= 64;
  coin[3] += (n % 64) << 8;
  n /= 64;
  coin[3] += (n % 64) << 16;
  n /= 64;
  coin[3] += (n % 64) << 24;

  for (n = 0; n < 64; n++) {
    //
    // compute MD5 hash
    //
    # define C(c)         (c)
    # define ROTATE(x,n)  (((x) << (n)) | ((x) >> (32 - (n))))
    # define DATA(idx)    coin[idx]
    # define HASH(idx)    hash[idx]
    # define STATE(idx)   state[idx]
    # define X(idx)       x[idx]
      CUSTOM_MD5_CODE();
    # undef C
    # undef ROTATE
    # undef DATA
    # undef HASH
    # undef STATE
    # undef X

    //
    // validate potential DETI coin
    //
    if (hash[3u] == 0u) {
      a = atomicAdd(deti_coins, 13u);
      if (a <= 1024u - 13u) {
        deti_coins[a] = coin[0u];
        deti_coins[a+1u] = coin[1u];
        deti_coins[a+2u] = coin[2u];
        deti_coins[a+3u] = coin[3u];
        deti_coins[a+4u] = coin[4u];
        deti_coins[a+5u] = coin[5u];
        deti_coins[a+6u] = coin[6u];
        deti_coins[a+7u] = coin[7u];
        deti_coins[a+8u] = coin[8u];
        deti_coins[a+9u] = coin[9u];
        deti_coins[a+10u] = coin[10u];
        deti_coins[a+11u] = coin[11u];
        deti_coins[a+12u] = coin[12u];
      }  
    }
    coin[12u] += (1 << 16);
  }
}
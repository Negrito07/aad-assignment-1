// Arquiteturas de Alto Desempenho 2024/2025
//
// deti_coins_cuda_search() --- find DETI coins using deti_coins_cuda_kernel_search()
//
#if USE_CUDA > 0
#define MD5_CUDA

#ifndef DETI_COINS_CUDA_SEARCH
#define DETI_COINS_CUDA_SEARCH

static void deti_coins_cuda_search(u32_t n_random_words)
{
  u64_t n_attempts, n_coins;
  void *cu_params[3];
  u32_t v1, v2;
  u32_t idx, idx_max;

  v1 = v2 = 0x20202020u;
  initialize_cuda(0,"deti_coins_cuda_kernel_search.cubin","deti_coins_cuda_kernel_search",0u,1024u);
  idx_max = 1u;

  for(n_attempts = n_coins = 0ul;stop_request == 0;n_attempts+= (64ul << 20))
  {
    host_hash[0u] = 1u;
    CU_CALL( cuMemcpyHtoD , (device_hash,(void *)host_hash,(size_t)1024u * sizeof(u32_t)) );
    cu_params[0] = &device_hash;
    cu_params[1] = &v1;
    cu_params[2] = &v2;
    CU_CALL(cuLaunchKernel, (
      cu_kernel,
      (1u << 20) / 128u, // Blocos
      1u,1u,              // Dimensões Y e Z dos blocos
      128u,1u,1u,         // Threads por bloco (128 threads no eixo X)
      0u,                 // Tamanho da memória compartilhada
      (CUstream)0,
      &cu_params[0],      // Parâmetros
      NULL
    ));
    CU_CALL( cuMemcpyDtoH , ((void *)host_hash,device_hash,(size_t)1024 * sizeof(u32_t)) );
    
    if (host_hash[0u] > idx_max)
    {
      idx_max = host_hash[0u];
    }

    for (idx = 1u; idx < host_hash[0] && idx <= 1024 - 13u; idx += 13u)
    {
      if (idx <= 1024 - 13u) {
        save_deti_coin(&host_hash[idx]);
        n_coins++;
      } else {
        fprintf(stderr, "deti_coins_cuda_search: deti_coin not used\n");
      }
    }

    if (v1 != 0x7E7E7E7Eu) {
      next_value_to_try(v1);
    } else {
      v1 = 0x20202020u;
      next_value_to_try(v2);
    }
  }
  STORE_DETI_COINS();
  printf("deti_coins_cuda_search: %lu DETI coin%s found in %lu attempts (expected %.2f coins)\n",n_coins,(n_coins == 1ul) ? "" : "s",n_attempts,(double)n_attempts / (double)(1ul << 32));
  terminate_cuda();
}
#endif
#endif
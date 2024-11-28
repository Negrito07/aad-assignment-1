

#include <omp.h> // Adicionar suporte ao OpenMP
#include "md5_cpu_avx2.h"


static void deti_coins_simd_openmp_search(u32_t n_random_words)
{
    u32_t n, idx, coin[8][13], hash[8][4];
    u64_t n_attempts = 0, n_coins = 0;
    u08_t *bytes[8];
    u32_t interleaved_data[8 * 13] __attribute__((aligned(32)));
    u32_t interleaved_hash[8 * 4] __attribute__((aligned(32)));

    // Inicializa as moedas
    for (int lane = 0; lane < 8; lane++) {
        bytes[lane] = (u08_t *)&coin[lane][0];
        bytes[lane][0] = 'D'; bytes[lane][1] = 'E'; bytes[lane][2] = 'T'; bytes[lane][3] = 'I';
        bytes[lane][4] = ' '; bytes[lane][5] = 'c'; bytes[lane][6] = 'o'; bytes[lane][7] = 'i';
        bytes[lane][8] = 'n'; bytes[lane][9] = ' ';

        for (idx = 10u; idx < 52u - 1u; idx++) 
            bytes[lane][idx] = ' ';

        bytes[lane][51] = '\n';
    }

    // Loop paralelo com OpenMP
    #pragma omp parallel for private(n, idx, coin, hash, interleaved_data, interleaved_hash) reduction(+:n_coins, n_attempts)
    for (n = 0; n < (1ul << 32); n += 8) { // Ajuste o limite conforme necessário
        // Prepara os dados intercalados
        for (int lane = 0; lane < 8; lane++)
            for (idx = 0; idx < 13; idx++)
                interleaved_data[8 * idx + lane] = coin[lane][idx];

        // Computa hashes MD5 usando AVX2
        md5_cpu_avx2((v8si *)interleaved_data, (v8si *)interleaved_hash);

        // Verifica resultados e salva moedas
        for (int lane = 0; lane < 8; lane++) {
            for (idx = 0; idx < 4; idx++)
                hash[lane][idx] = interleaved_hash[8 * idx + lane];

            hash_byte_reverse(hash[lane]);
            u32_t power = deti_coin_power(hash[lane]);

            if (power >= 32u) {
                #pragma omp critical
                {
                    print_hash(hash[lane]);
                    save_deti_coin(coin[lane]);
                    n_coins++;
                }
            }

            // Próxima combinação
            for (idx = 10u; idx < 52u - 1u && bytes[lane][idx] == (u08_t)126; idx++)
                bytes[lane][idx] = ' ';
            if (idx < 52u - 1u)
                bytes[lane][idx]++;
        }

        n_attempts += 8;
    }

    STORE_DETI_COINS();
    printf("deti_coins_simd_openmp_search: %lu DETI coins found in %lu attempts\n", n_coins, n_attempts);
}


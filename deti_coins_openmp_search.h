#ifndef DETI_COINS_OPENMP_SEARCH
#define DETI_COINS_OPENMP_SEARCH

#include <omp.h> // Biblioteca para OpenMP
#include "md5_cpu_avx2.h"

static void deti_coins_openmp_search(u32_t n_random_words)
{
    const unsigned int n_threads = 4; // Número de threads (ajustável)
    u64_t n_attempts = 0, n_coins = 0;

    #pragma omp parallel num_threads(n_threads) reduction(+:n_coins, n_attempts)
    {
        int thread_id = omp_get_thread_num();
        u32_t n, idx, coin[8][13], hash[8][4];
        u08_t *bytes[8];
        u32_t interleaved_data[8 * 13] __attribute__((aligned(32)));
        u32_t interleaved_hash[8 * 4] __attribute__((aligned(32)));

        // Inicializa as moedas para cada thread
        for (int lane = 0; lane < 8; lane++) {
            bytes[lane] = (u08_t *)&coin[lane][0];
            bytes[lane][0] = 'D'; bytes[lane][1] = 'E'; bytes[lane][2] = 'T'; bytes[lane][3] = 'I';
            bytes[lane][4] = ' '; bytes[lane][5] = 'c'; bytes[lane][6] = 'o'; bytes[lane][7] = 'i';
            bytes[lane][8] = 'n'; bytes[lane][9] = ' ';

            // Aleatoriedade com base em n_random_words
            for (idx = 10u; idx < 52u - 1u; idx++) {
                if ((rand() % 13) < n_random_words) {
                    bytes[lane][idx] = (u08_t)(rand() % 94 + 32); // Caractere aleatório entre 32 e 126
                } else {
                    bytes[lane][idx] = ' '; // Espaços
                }
            }
            bytes[lane][51] = '\n'; // Terminação obrigatória
        }

        // Cada thread executa um loop independente
        for (n = thread_id; n < (1ul << 32); n += n_threads * 8) {
            // Prepara os dados intercalados
            for (int lane = 0; lane < 8; lane++)
                for (idx = 0; idx < 13; idx++)
                    interleaved_data[8 * idx + lane] = coin[lane][idx];

            // Computa hashes MD5 usando AVX2
            md5_cpu_avx2((v8si *)interleaved_data, (v8si *)interleaved_hash);

            // Verifica os resultados e salva moedas
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
    }

    STORE_DETI_COINS();
    printf("deti_coins_openmp_search: %lu DETI coins found in %lu attempts\n", n_coins, n_attempts);
}

#endif

#ifndef DETI_COINS_CPU_AVX2_SEARCH
#define DETI_COINS_CPU_AVX2_SEARCH

#include "md5_cpu_avx2.h"
        
static void deti_coins_cpu_avx2_search(u32_t n_random_words)
{
    u32_t n, idx, coin[8][13], hash[8][4];
    u64_t n_attempts, n_coins;
    u08_t *bytes[8];
    u32_t interleaved_data[8 * 13] __attribute__((aligned(32)));
    u32_t interleaved_hash[8 * 4] __attribute__((aligned(32)));

    // Inicializa as moedas
    for (int lane = 0; lane < 8; lane++) {
        bytes[lane] = (u08_t *)&coin[lane][0];
        bytes[lane][0] = 'D'; bytes[lane][1] = 'E'; bytes[lane][2] = 'T'; bytes[lane][3] = 'I';
        bytes[lane][4] = ' '; bytes[lane][5] = 'c'; bytes[lane][6] = 'o'; bytes[lane][7] = 'i';
        bytes[lane][8] = 'n'; bytes[lane][9] = ' ';

        // for (idx = 10u; idx < 52u - 1u; idx++) 
        //     bytes[lane][idx] = ' ';

        // Gerar um número aleatório de palavras para modificar
        int random_words_count = n_random_words;  // Número de palavras aleatórias baseado no argumento

        // Limite de aleatoriedade para as 13 palavras possíveis
        for (idx = 10u; idx < 52u - 1u; idx++) {
            // Aleatoriedade: Se for uma posição que deve ser aleatória
            if (rand() % 13 < random_words_count) {
                bytes[lane][idx] = (u08_t)(rand() % 94 + 32);  // Gerar caractere aleatório entre 32 (espço) e 126 (~)
            } else {
                bytes[lane][idx] = ' ';  // Se não, preenche com espaço
            }
        }

        bytes[lane][51] = '\n';
    }

    for (n_attempts = n_coins = 0ul; stop_request == 0; n_attempts += 8) {
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
            n = deti_coin_power(hash[lane]);

            if (n >= 32u) {
                //print_hash(hash[lane]);
               // printf("power: %d\n",n);
               // print_deti_coin(coin[lane]);
                save_deti_coin(coin[lane]);
                n_coins++;
            }

            for (idx = 10u; idx < 51u && bytes[lane][idx] == (u08_t)126; idx++)
                bytes[lane][idx] = ' ';
            if (idx < 52u - 1u)
                bytes[lane][idx]++;
        }
    }

    STORE_DETI_COINS();
    printf("deti_coins_cpu_avx2_search: %lu DETI coin%s found in %lu attempt%s (expected %.2f coins)\n",
           n_coins, (n_coins == 1ul) ? "" : "s", n_attempts, (n_attempts == 1ul) ? "" : "s",
           (double)n_attempts / (double)(1ul << 32));
}

#endif

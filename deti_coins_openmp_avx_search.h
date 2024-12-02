#ifndef DETI_COINS_OPENMP_AVX_SEARCH
#define DETI_COINS_OPENMP_AVX_SEARCH

#include <omp.h> // Biblioteca para OpenMP
#include "md5_cpu_avx.h"

static void deti_coins_openmp_avx_search(u32_t n_random_words){

    u64_t n_attempts = 0, n_coins = 0;

    #pragma omp parallel reduction(+:n_coins, n_attempts)
    {
        u32_t n, idx, coin[4][13], hash[4][4];
        u08_t *bytes[4];
        u32_t interleaved_data[4 * 13] __attribute__((aligned(16)));
        u32_t interleaved_hash[4 * 4] __attribute__((aligned(16)));


        int random_positions[9] = {0}; //Apenas 9 posições possíveis(Words)
        int random_count = 0;

        //Quais posições serão aleatórias tendo em conta n_random_words
        while (random_count < n_random_words) {
            int random_pos = rand() % 9; 
            if (!random_positions[random_pos]) {
                random_positions[random_pos] = 1; 
                random_count++;
            }
        }

        // Inicializa as moedas para os quatro caminhos de busca paralelos
        for (int lane = 0; lane < 4; lane++) {
            bytes[lane] = (u08_t *)&coin[lane][0];
            bytes[lane][0] = 'D'; bytes[lane][1] = 'E'; bytes[lane][2] = 'T'; bytes[lane][3] = 'I';
            bytes[lane][4] = ' '; bytes[lane][5] = 'c'; bytes[lane][6] = 'o'; bytes[lane][7] = 'i';
            bytes[lane][8] = 'n'; bytes[lane][9] = ' '; bytes[lane][10] = '0' + lane; bytes[lane][11] = '0' + omp_get_thread_num();

            for (int word_idx = 0; word_idx < 9; word_idx++) {
                int idx = 12 + word_idx * 4; // Calcula o índice inicial do byte correspondente
                if (random_positions[word_idx]) {
                    //Posição é aleatória
                    for (int byte = 0; byte < 4; byte++) {
                        bytes[lane][idx + byte] = (u08_t)(rand() % 94 + 32);
                    }
                } else {
                    //Posição não é aleatória
                    for (int byte = 0; byte < 4; byte++) {
                        bytes[lane][idx + byte] = ' '; 
                    }
                }
            }

            bytes[lane][48] = ' '; bytes[lane][49] = ' '; bytes[lane][50] = ' '; bytes[lane][51] = '\n';
        }

        while (stop_request == 0) {
            for (int lane = 0; lane < 4; lane++)
                for (idx = 0; idx < 13; idx++)
                    interleaved_data[4 * idx + lane] = coin[lane][idx];

            //Calcula os hashes MD5 usando AVX
            md5_cpu_avx((v4si *)interleaved_data, (v4si *)interleaved_hash);

            //Verifica os resultados e salva moedas válidas
            for (int lane = 0; lane < 4; lane++) {
                for (idx = 0; idx < 4; idx++)
                    hash[lane][idx] = interleaved_hash[4 * idx + lane];

                hash_byte_reverse(hash[lane]);
                
                n = deti_coin_power(hash[lane]);
                
                //Moeda válida
                if (n >= 32u) { 
                    // Região crítica
                    #pragma omp critical
                    {
                        //print_hash(hash[lane]);
                        //print_deti_coin(coin[lane]);                
                        save_deti_coin(coin[lane]);
                        n_coins++;
                    }
                }

                //Incrementa os bytes para próxima tentativa
                for (idx = 12u; idx < 52u - 1u && bytes[lane][idx] == (u08_t)126; idx++)
                    bytes[lane][idx] = ' ';
                if (idx < 52u - 1u)
                    bytes[lane][idx]++;            
            }

            n_attempts += 4;

        }
    }

    STORE_DETI_COINS();
    printf("deti_coins_openmp_avx_search: %lu DETI coin%s found in %lu attempt%s (expected %.2f coins)\n",
    n_coins, (n_coins == 1ul) ? "" : "s", n_attempts, (n_attempts == 1ul) ? "" : "s",
    (double)n_attempts / (double)(1ul << 32));

}


#endif

// Arquiteturas de Alto Desempenho 2024/2025
//
// deti_coins_cpu_avx_search() --- find DETI coins using md5_cpu_avx()
//

#ifndef DETI_COINS_CPU_AVX_SEARCH
#define DETI_COINS_CPU_AVX_SEARCH

#include "md5_cpu_avx.h"

void print_deti_coin(u32_t coin[13])
{
    u08_t *bytes = (u08_t *)coin;

    printf("DETI Coin Found: \"");
    for (int i = 0; i < 52; i++)
    {
        if (bytes[i] >= 32 && bytes[i] <= 126) // Imprime apenas caracteres legíveis
            printf("%c", bytes[i]);
        else
            printf("\\x%02X", bytes[i]); // Caracteres não legíveis como hexadecimal
    }
    printf("\"\n");
}


void print_hash(const u32_t hash[4])
{
    printf("MD5 Hash: ");
    for (int i = 0; i < 4; i++)
        printf("%08X", hash[i]); // Imprime cada parte do hash em hexadecimal
    printf("\n");
}


static void deti_coins_cpu_avx_search(u32_t n_random_words)
{
    u32_t n, idx, coin[4][13], hash[4][4];
    u64_t n_attempts, n_coins;
    u08_t *bytes[4];
    u32_t interleaved_data[4 * 13] __attribute__((aligned(16)));
    u32_t interleaved_hash[4 * 4] __attribute__((aligned(16)));

    // Inicializa as moedas para os quatro caminhos de busca paralelos
    for (int lane = 0; lane < 4; lane++) {
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

        bytes[lane][51] = '\n'; // Terminação obrigatória
    }

    // Inicia o loop de tentativa
    for (n_attempts = n_coins = 0ul; stop_request == 0; n_attempts += 4) {
        // Prepara os dados intercalados para entrada do AVX
        for (int lane = 0; lane < 4; lane++)
            for (idx = 0; idx < 13; idx++)
                interleaved_data[4 * idx + lane] = coin[lane][idx];

        // Calcula os hashes MD5 usando AVX
        md5_cpu_avx((v4si *)interleaved_data, (v4si *)interleaved_hash);

        // Verifica os resultados e salva moedas válidas
        for (int lane = 0; lane < 4; lane++) {
            // Transfere os dados de volta para o formato padrão
            for (idx = 0; idx < 4; idx++)
                hash[lane][idx] = interleaved_hash[4 * idx + lane];

            // Reverte os bytes do hash
            hash_byte_reverse(hash[lane]);

            // Determina o "poder" da moeda
            n = deti_coin_power(hash[lane]);

            // if (lane == 0){
            //   print_hash(hash[lane]);
            // }

            if (n >= 32u) { // Moeda válida
                print_hash(hash[lane]);
                //print_deti_coin(coin[lane]);

                
                save_deti_coin(coin[lane]);
                n_coins++;
            }

            // Incrementa os bytes para próxima tentativa
            for (idx = 10u; idx < 52u - 1u && bytes[lane][idx] == (u08_t)126; idx++)
                bytes[lane][idx] = ' ';
            if (idx < 52u - 1u)
                bytes[lane][idx]++;


            
        }
    }

    STORE_DETI_COINS();
    printf("deti_coins_cpu_avx_search: %lu DETI coin%s found in %lu attempt%s (expected %.2f coins)\n",
           n_coins, (n_coins == 1ul) ? "" : "s", n_attempts, (n_attempts == 1ul) ? "" : "s",
           (double)n_attempts / (double)(1ul << 32));
}



#endif

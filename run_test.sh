#!/bin/bash

# Nome do executável
EXECUTABLE="./deti_coins_intel"

# Parâmetros de execução
DURATION=3600 # 1 hora em segundos
RANDOM_WORDS=(1 9)
CASES=("1" "2" "5" "6")

# Diretório para salvar os resultados
RESULTS_DIR="results"
mkdir -p $RESULTS_DIR

# Loop pelos casos (-s1, -s2, -s5, -s6)
for case in "${CASES[@]}"; do
  # Loop pelos valores de n_random_words (1 e 9)
  for n_random in "${RANDOM_WORDS[@]}"; do
    echo "Running: -s$case with $n_random random words for $DURATION seconds"
    
    # Nome do arquivo de saída
    OUTPUT_FILE="${RESULTS_DIR}/output_s${case}_r${n_random}.txt"
    
    # Executa o comando e salva o resultado no arquivo
    $EXECUTABLE -s$case $DURATION $n_random > $OUTPUT_FILE 2>&1

    echo "Results saved to $OUTPUT_FILE"
  done
done

echo "All tests completed."

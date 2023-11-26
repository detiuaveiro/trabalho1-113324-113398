#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "instrumentation.h"
#include "image8bit.h"

int main(void) {
    printf("(ImageLocateSubImage)\n");

    // Carregar a imagem principal e a subimagem
    Image img1 = ImageLoad("pgm/medium/tools_2_765x460.pgm");
    Image img2 = ImageLoad("pgm/small/bird_256x256.pgm");

    // Variáveis para armazenar a posição da subimagem encontrada e o número de comparações
    int pos_x, pos_y;
    int num_comparisons = 0;

    // Medir o tempo de execução
    double start_time = cpu_time();
    int found = ImageLocateSubImage(img1, &pos_x, &pos_y, img2, &num_comparisons); // Adicionado o parâmetro para contar comparações
    double finish_time = cpu_time();
    double exec_time = finish_time - start_time;

    // Mostrar os resultados
    if (found) {
        printf("Subimagem encontrada na posição (%d, %d)\n", pos_x, pos_y);
    } else {
        printf("Subimagem não encontrada.\n");
    }
    printf("Número de comparações: %d\n", num_comparisons);
    printf("Tempo de execução: %f segundos\n", exec_time);

    // Limpar as imagens da memória
    ImageFree(img1);
    ImageFree(img2);

    return 0;
}

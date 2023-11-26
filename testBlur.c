#include "image8bit.h"
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "instrumentation.h"
#include <time.h>
#include <math.h>

int main(void)
{ 
    printf("(ImageBlur)\n");
    Image img;
    double start_time, finish_time, exec_time;

    img = ImageLoad("pgm/small/art3_222x217.pgm");
    start_time = cpu_time();
    ImageBlur(img, 7, 7); 
    finish_time = cpu_time();
    exec_time = finish_time - start_time;
    printf("Tempo de execução (art3_222x217.pgm): %f segundos\n", exec_time);

    img = ImageLoad("pgm/small/art4_300x300.pgm");
    start_time = cpu_time();
    ImageBlur(img, 7, 7); 
    finish_time = cpu_time();
    exec_time = finish_time - start_time;
    printf("Tempo de execução (art4_300x300.pgm): %f segundos\n", exec_time);

    img = ImageLoad("pgm/small/bird_256x256.pgm");
    start_time = cpu_time();
    ImageBlur(img, 7, 7); 
    finish_time = cpu_time();
    exec_time = finish_time - start_time;
    printf("Tempo de execução (bird_256x256.pgm): %f segundos\n", exec_time);

    img = ImageLoad("pgm/medium/airfield-05_640x480.pgm");
    start_time = cpu_time();
    ImageBlur(img, 7, 7); 
    finish_time = cpu_time();
    exec_time = finish_time - start_time;
    printf("Tempo de execução (airfield-05_640x480.pgm): %f segundos\n", exec_time);

    img = ImageLoad("pgm/medium/ireland-03_640x480.pgm");
    start_time = cpu_time();
    ImageBlur(img, 7, 7); 
    finish_time = cpu_time();
    exec_time = finish_time - start_time;
    printf("Tempo de execução (ireland-03_640x480.pgm): %f segundos\n", exec_time);

    img = ImageLoad("pgm/medium/mandrill_512x512.pgm");
    start_time = cpu_time();
    ImageBlur(img, 7, 7); 
    finish_time = cpu_time();
    exec_time = finish_time - start_time;
    printf("Tempo de execução (mandrill_512x512.pgm): %f segundos\n", exec_time);

    img = ImageLoad("pgm/medium/tac-pulmao_512x512.pgm");
    start_time = cpu_time();
    ImageBlur(img, 7, 7); 
    finish_time = cpu_time();
    exec_time = finish_time - start_time;
    printf("Tempo de execução (tac-pulmao_512x512.pgm): %f segundos\n", exec_time);

    img = ImageLoad("pgm/medium/tools_2_765x460.pgm");
    start_time = cpu_time();
    ImageBlur(img, 7, 7); 
    finish_time = cpu_time();
    exec_time = finish_time - start_time;
    printf("Tempo de execução (tools_2_765x460.pgm): %f segundos\n", exec_time);

    img = ImageLoad("pgm/large/airfield-05_1600x1200.pgm");
    start_time = cpu_time();
    ImageBlur(img, 7, 7); 
    finish_time = cpu_time();
    exec_time = finish_time - start_time;
    printf("Tempo de execução (airfield-05_1600x1200.pgm): %f segundos\n", exec_time);

    img = ImageLoad("pgm/large/einstein_940x940.pgm");
    start_time = cpu_time();
    ImageBlur(img, 7, 7); 
    finish_time = cpu_time();
    exec_time = finish_time - start_time;
    printf("Tempo de execução (einstein_940x940.pgm): %f segundos\n", exec_time);

    img = ImageLoad("pgm/large/ireland_03_1600x1200.pgm");
    start_time = cpu_time();
    ImageBlur(img, 7, 7); 
    finish_time = cpu_time();
    exec_time = finish_time - start_time;
    printf("Tempo de execução (ireland_03_1600x1200.pgm): %f segundos\n", exec_time);

    img = ImageLoad("pgm/large/ireland-06-1200x1600.pgm");
    start_time = cpu_time();
    ImageBlur(img, 7, 7); 
    finish_time = cpu_time();
    exec_time = finish_time - start_time;
    printf("Tempo de execução (ireland_06_1200x1600.pgm): %f segundos\n", exec_time);

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#define ONE_MB_SIZE 262144
int32_t out[ONE_MB_SIZE];

int main(int argc, char* argv[]) {
    if(argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: gen_ran <filename> [size(MB)]\n");
        exit(1);
    }
    int size;
    if(argc == 2) size = 100; // 如果只有两个参数 即只有一个文件名称 默认生成100MB的文件
    else size = atoi(argv[2]);
    FILE* outfile;
    if((outfile = fopen(argv[1], "wb")) == NULL) {
        fprintf(stderr, "open error\n");
        exit(1);
    }
    srand((unsigned int)time(0));
    int i, j;
    for(i = 1; i <= size; i++) {
        for(j = 0; j < ONE_MB_SIZE; j++) {
            out[j] = rand();
        }
        fwrite(out, sizeof(int32_t), ONE_MB_SIZE, outfile);
    }
    fprintf(stdout, "random input file %s was generated successfully\n", argv[1]);
    exit(0);
}

#include "apue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/times.h>
#define BUFFERSIZE 1073741824
char buf[BUFFERSIZE];

int main(int argc, char** argv) {
    if(argc <= 1 || argc > 3) {
        err_quit("Usage: ./timewrite <outfile> [sync]");
    }

    if(argc == 3 && strcmp(argv[2], "sync") != 0) { // 3 options but the third is not "sync"
        err_quit("Usage: ./timewrite <outfile> [sync]");
    }

    int fd;

    if(argc == 2) { // open file asynchronously
        if((fd = open(argv[1], O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR)) < 0) {
            fprintf(stderr, "file: %s open failed\n", argv[1]);
            exit(1);
        }
    } else if(argc == 3) { // open file synchronously
        if((fd = open(argv[1], O_WRONLY|O_CREAT|O_SYNC, S_IRUSR|S_IWUSR)) < 0) {
            fprintf(stderr, "file: %s open failed\n", argv[1]);
            exit(1);
        }
    }

    long long length; // the size of input file
    // read the input file into buffer
    if((length = read(STDIN_FILENO, buf, BUFFERSIZE)) < 0) {
        fprintf(stderr, "fille read error\n");
        exit(1);
    }
    int ticks = sysconf(_SC_CLK_TCK);

    clock_t clockStart, clockEnd;
    struct tms tmsStart, tmsEnd;

    int buffersize = 1024;

    // first time
    if(lseek(fd, 0, SEEK_SET) == -1) {
        err_sys("lseek failed\n");
        exit(1);
    }

    int i;
    int writesize;
    clock_t marktime = 0;
    fprintf(stdout, "BUFFSIZE USER_CPU(s) SYSTEM_CPU(s) CLOCK_TIME(s) LOOP_TIMES\n");
    for(buffersize = 256; buffersize <= 8,388,608; buffersize *= 2) {
        // locate to the head
        if(lseek(fd, 0, SEEK_SET) == -1) {
            err_sys("lseek error\n");
        }
        clockStart = times(&tmsStart);
        i = 0;
        clockStart = times(&tmsStart);
        int cnt = 0;
        for(i = 0; i < length; i += buffersize) {
            writesize = length - i;
            if(buffersize < writesize) writesize = buffersize;
            if(write(fd, buf + i, writesize) != writesize) {
                err_sys("write error\n");
                exit(1);
            }
            cnt++;
        }
        clockEnd = times(&tmsEnd);
        fprintf(stdout, "%8d     %.2lf          %.2lf         %.2lf %10d\n", buffersize, (double)(tmsEnd.tms_utime - tmsStart.tms_utime) / ticks, (double)(tmsEnd.tms_stime - tmsStart.tms_stime) / ticks, (double)(clockEnd - clockStart) / ticks, cnt);
    }
    exit(0);
}

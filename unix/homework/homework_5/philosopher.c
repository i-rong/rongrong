#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/wait.h>
#define N 5

/*
 *  * default file access permissions for new files.
 *   * */
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

static char *forks[N] = {"fork0", "fork1", "fork2", "fork3", "fork4"};

static void initlock() {
    int i;
    for(i = 0; i < N; i++) unlink(forks[i]);
}

static void lock(const char *lockfile) {
    int fd;
    while((fd = open(lockfile, O_RDONLY|O_CREAT|O_EXCL, FILE_MODE)) < 0) sleep(1); // 每隔1s尝试一次，直到打开为止
    close(fd);
}

static void unlock(const char *lockfile) {
    unlink(lockfile);
}


char *getTime() {
    char *time_now;
    time_now = (char*)malloc(sizeof(char) * 64);
    memset(time_now, 0, sizeof time_now);
    time_t tloc;
    struct tm *tm;
    time(&tloc);
    tm = localtime(&tloc);
    strftime(time_now, 63, "%H:%M:%S", tm);
    return time_now;
}

static void thinking(int i, int time) {
    fprintf(stdout, "Philosopher %d is thinking   %s\n", i, getTime());
    sleep(time);
}

static void eating(int i, int time) {
    fprintf(stdout, "Philosopher %d is eating     %s\n", i, getTime());
    sleep(time);
}

static void takefork(int i) {
    if(i == N - 1) { // if the last one, first take the left, then take the right
        lock(forks[N - 1]);
        lock(forks[0]);
    } else { // or not, first take the right, then take the left
        lock(forks[i + 1]);
        lock(forks[i]);
    }
}

static void putfork(int i) {
    if(i == N - 1) {
        unlock(forks[N - 1]);
        unlock(forks[0]);
    } else {
        unlock(forks[i + 1]);
        unlock(forks[i]);
    }
}

static void philosopher(int i, int time) {
    while(1) {
        thinking(i, time); // philosopher i think for second
        takefork(i); // philosopher i take the fork
        eating(i, time); // philosopher i eat for seconds
        putfork(i); // philosopher i put the fork
    }
}

int main(int argc, char* argv[]) {
    initlock();
    int seconds, i;
    pid_t pid;
    if(argc == 1) {
        seconds = 2;
    } else if(argc == 3 && (strcmp(argv[1], "-t") == 0)) {
        seconds = atoi(argv[2]);
    } else {
        fprintf(stderr, "Usage: ./philosopher [-t <time>]\n");
        exit(1);
    }

    for(i = 0; i < N; i++) {
        fflush(stdout); // 每次fork前都需要清空缓冲区
        pid = fork();
        if(pid < 0) {
            perror("fork error: ");
            exit(1);
        } else if(pid == 0) { // child
            philosopher(i, seconds);
            exit(0);
        }
    }

    for(i = 0; i < N; i++) wait(NULL);

    exit(0);
}

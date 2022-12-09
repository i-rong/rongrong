#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MAXLINE 1024
char *argv[MAXLINE];
int argc;
char *rf, *wf;
char prompt[MAXLINE] = "[Myshell ";

extern char **environ;

void print_prompt(char* string) {
    if(getcwd(string + 9, MAXLINE) == NULL) {
        perror("getcwd error: ");
        exit(1);
    }
    strncat(string, "]$", MAXLINE - 1);
    printf("%s", string);
}

void init() {
    memset(argv, 0, sizeof argv);
    argc = 0;
}

void read_from_buf(char *buf) {
    int type = 0;
    rf = NULL;
    wf = NULL;
    if(buf[strlen(buf) - 1] == '\n') buf[strlen(buf) - 1] = 0;
    char *token = strtok(buf, " ");
    while(token != NULL) {
        if(type == 1) { // rf
            rf = token;
            type = 0;
        } else if(type == 2) {
            wf = token;
            type = 0;
        } else if(!strcmp(token, "<")) type = 1;
        else if(!strcmp(token, ">")) type = 2;
        else argv[argc++] = token;
        token = strtok(NULL, " ");
    }
    argv[argc] = NULL;
}

void redirect(char *rf, char *wf) {
    int rfd, wfd;
    if(rf != NULL) {
        if((rfd = open(rf, O_RDONLY)) < 0) {
            perror("open error: ");
            exit(1);
        }
        if(dup2(rfd, STDIN_FILENO) < 0) { // 使用dup2将输入重定向到rf
            perror("dup2 error: ");
            exit(1);
        }
        close(rfd);
    }
    if(wf != NULL) {
        if((wfd = open(wf, O_WRONLY|O_CREAT|O_TRUNC, 644)) < 0) {
            perror("open error: ");
            exit(1);
        }
        if(dup2(wfd, STDOUT_FILENO) < 0) {
            perror("dup2 error: ");
            exit(1);
        }
        close(wfd);
    }
}

int main() {
    char buf[MAXLINE];
    pid_t pid;
    int status;

    while(1) {
        print_prompt(prompt);
        fgets(buf, MAXLINE - 1, stdin) != NULL;
        init();
        read_from_buf(buf);
        if(!strcmp(argv[0], "cd")) {
            if(argv[1] == NULL || !strcmp(argv[1], "~")) {
                argv[1] = getenv("HOME");
            }
            if(chdir(argv[1]) < 0) {
                perror("chdir error: ");
                exit(1);
            }
        }
        else if(!strcmp(argv[0], "pwd")) {
            char *curpath;
            curpath = getcwd(NULL, 0);
            if(curpath == NULL) {
                perror("getcwd error: ");
                exit(1);
            }
            fprintf(stdout, "%s\n", curpath);
        }
        else {
            if((pid = fork()) < 0) {
                perror("fork error: ");
                exit(1);
            } else if(pid == 0) {
                redirect(rf, wf);
                execve(argv[0], argv, environ);
                perror("execve error: ");
                exit(1);
            } else {
                if((pid = waitpid(pid, &status, 0)) < 0) {
                    perror("waitpid error: ");
                    exit(1);
                }
            }
        }
    }
    exit(0);
}

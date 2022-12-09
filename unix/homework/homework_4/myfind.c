#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <glob.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>

#define FIND_F 1 // file other than directory
#define FIND_D 2 // directory
#define FIND_DNR 3 // directory that can't be read
#define FIND_NS 4 // file that we can't stat
#define TYPE_P 1
#define TYPE_C 2
#define TYPE_N 3

#define PATHSIZE 255
#define BUFSIZE 1073741824
#define NAMESIZE 55
#define STRNUM 15 // 最多带有15个名称
static char fullpath[PATHSIZE];
static char c_filename[PATHSIZE];
static char c_filebuf[BUFSIZE];
static long long c_filelen;
static struct stat c_filestat;
static char* cur_path;
static char* tmp_path;
static char name_str[STRNUM][NAMESIZE];
static int str_num;

typedef int Myfunc(const char*, const struct stat*, int);
static long nreg, ndir, nblk, nchr, nfifo, nslink, nsock, ntot, nlrg; // 分别表示常规文件、目录文件...

static int path_noloop(const char* pathname) {
    char* pos;
    pos = strrchr(pathname, '/'); // 取到最右边的'/'的位置  保存在pos中
    if(pos == NULL) exit(1); // 如果没有'/'
    if(strcmp(pos + 1, ".") == 0 || strcmp(pos + 1, "..") == 0) return 0;
    else return 1;
}

static int myfunc1(const char* pathname, const struct stat* statptr, int type) {
    switch(type) {
        case FIND_F: // 文件
            switch(statptr->st_mode & S_IFMT) {
                case S_IFREG:   nreg++;   break;
                case S_IFBLK:   nblk++;   break;
                case S_IFCHR:   nchr++;   break;
                case S_IFIFO:   nfifo++;  break;
                case S_IFLNK:   nslink++; break;
                case S_IFSOCK:  nsock++;  break;
                case S_IFDIR:   fprintf(stderr, "for S_IFDIR for %s", pathname);
            }
            if(statptr->st_size > 4096) nlrg++;
            break;
        case FIND_D:
            ndir++; break;
        case FIND_DNR:
            fprintf(stderr, "%s: ", fullpath); perror(""); break;
        case FIND_NS:
            fprintf(stderr, "%s: ", fullpath); perror(""); break;
        default:
            fprintf(stderr, "unknown type %d for pathname %s\n", type, pathname);
    }
    return 0;
}

static int myfunc2(const char* pathname, const struct stat* statptr, int type) { // pathname是传入的绝对路径或相对路径
    if(type == FIND_F) {
        int fd;
        char* fullbuf = (char*)malloc(sizeof(char) * c_filelen); // 开辟一块和c_filelen一样大小的空间 用于存放从文件读入的数据
        char* tmppath = (char*)malloc(sizeof(char) * PATHSIZE);
        char* curpath = (char*)malloc(sizeof(char) * PATHSIZE);
        memset(fullbuf, 0, sizeof(fullbuf));
        if((fd = open(pathname, O_RDONLY)) < 0) { // 将文件打开 并将文件描述符放在fd中
            fprintf(stderr, "%s: ", pathname);
            perror("");
        }
        if(read(fd, fullbuf, c_filelen) < 0) {
            fprintf(stderr, "%s: ", pathname);
            perror("");
        }
        //printf("%s\n", fullbuf); fullbuf没问题
        //printf("c_filebuf: %s\n", c_filebuf); // c_filebuf没问题
        if(memcmp(fullbuf, c_filebuf, statptr->st_size) == 0) { // memcmp没问题
            const char* pos = strrchr(pathname, '/');
            if((tmppath = getcwd(NULL, 0)) == NULL) perror("getcwd()");
            strcpy(curpath, fullpath);
            char* pos2;
            pos2 = strrchr(curpath, '/');
            *pos2 = 0;
            if(chdir(curpath) < 0) perror("chdir()");
            if((curpath = getcwd(NULL, 0)) == NULL) perror("getcwd()");
            fprintf(stdout, "%s%s\n", curpath, pos);
            if(chdir(tmppath) < 0) perror("chdir()");
            free(fullbuf);
            free(tmppath);
            free(curpath);
            return 0;
        }
    } else if(type == FIND_DNR) {
        fprintf(stderr, "%s ", fullpath);
        perror("readdir()");
    } else if(type == FIND_NS) {
        fprintf(stderr, "%s ", fullpath);
        perror("stat()");
    }
    return 0;
}

static int myfunc3(const char* pathname, const struct stat* staptr, int type) {
    if(type == FIND_F) { // 是文件
        int fd, i;
        char* namebuf = (char*)malloc(sizeof(char) * NAMESIZE); // 开辟一块和NAMESIZE一样大小的空间 用于存放文件名称
        char* tmppath = (char*)malloc(sizeof(char) * PATHSIZE);
        char* curpath = (char*)malloc(sizeof(char) * PATHSIZE);
        const char* pos;
        pos = strrchr(pathname, '/');
        strcpy(namebuf, pos + 1);

        for(i = 0; i < str_num; i++) {
            if(strcmp(namebuf, name_str[i]) == 0) {
                char* pos2;
                strcpy(curpath, fullpath);
                pos2 = strrchr(curpath, '/');
                *pos2 = 0;
                if((tmppath = getcwd(NULL, 0)) == NULL) perror("getcwd()");
                if(chdir(curpath) < 0) perror("chdir()");
                if((curpath = getcwd(NULL, 0)) == NULL) perror("getcwd()");
                if(chdir(tmppath) < 0) perror("chdir()");
                fprintf(stdout, "%s%s\n", curpath, pos);
                break;
            }
        }
        free(namebuf);
        free(tmppath);
        free(curpath);
    } else if(type == FIND_DNR) {
        fprintf(stderr, "%s ", fullpath);
        perror("readdir()");
    } else if(type == FIND_NS) {
        fprintf(stderr, "%s ", fullpath);
        perror("stat()");
    }
    return 0;
}

static int dopath(Myfunc* func, int type) {
    struct stat statbuf;
    DIR* dp;
    struct dirent* dirp;
    int ret, n;
    if(type == TYPE_P) {
        struct stat statbuf;
        DIR* dp;
        struct dirent* dirp;
        int ret, n;
        if(lstat(fullpath, &statbuf) < 0) // stat error
            return (func(fullpath, &statbuf, FIND_NS));

        if(S_ISDIR(statbuf.st_mode) == 0) // 不是目录
            return (func(fullpath, &statbuf, FIND_F));
        // 下面是目录
        if((ret = func(fullpath, &statbuf, FIND_D)) != 0) {
            fprintf(stderr, "FIND_D error\n");
            return ret; // 如果不是0表示出错 这里直接返回
        }
        dp = opendir(fullpath);
        if(dp == NULL) {
            return func(fullpath, &statbuf, FIND_DNR);
        }
        n = strlen(fullpath);
        fullpath[n++] = '/';
        fullpath[n] = 0;
        while((dirp = readdir(dp)) != NULL) {
            strcpy(&fullpath[n], dirp->d_name);
            if(path_noloop(fullpath)) {
                if(ret = dopath(func, type) != 0) break; // 如果不存在回路
            }
        }
        fullpath[n - 1] = 0;
        if(closedir(dp) < 0) perror("closedir()");
        return ret;
    } else if(type == TYPE_C) {
        if(lstat(fullpath, &statbuf) < 0) { // stat error
            char* curpath = getcwd(NULL, 0);
            fprintf(stderr, "curpath : %s\n", curpath);
            return func(fullpath, &statbuf, FIND_NS);
        }
        if(!S_ISDIR(statbuf.st_mode)) {
            return func(fullpath, &statbuf, FIND_F);
        }
        if((dp = opendir(fullpath)) == NULL) {
            return func(fullpath, &statbuf, FIND_DNR);
        }
        n = strlen(fullpath);
        fullpath[n++] = '/';
        fullpath[n] = 0;
        while((dirp = readdir(dp)) != NULL) {
            strcpy(fullpath + n, dirp->d_name);
            if(path_noloop(fullpath)) {
                if(ret = dopath(func, type) != 0) break;
            }
        }
        fullpath[n - 1] = 0;
        if(closedir(dp) < 0) perror("closedir()");
        return ret;
    } else { // TYPE_N
        if(lstat(fullpath, &statbuf) < 0) { // stat error
            char* curpath = getcwd(NULL, 0);
            printf("curpath : %s\n", curpath);
            return func(fullpath, &statbuf, FIND_NS);
        }
        if(!S_ISDIR(statbuf.st_mode)) {
            return func(fullpath, &statbuf, FIND_F);
        }
        if((dp = opendir(fullpath)) == NULL) {
            return func(fullpath, &statbuf, FIND_DNR);
        }
        // 下面是目录
        n = strlen(fullpath);
        fullpath[n++] = '/';
        fullpath[n] = 0;
        while((dirp = readdir(dp)) != NULL) {
            strcpy(fullpath + n, dirp->d_name);
            if(path_noloop(fullpath)) {
                if(ret = dopath(func, type) != 0) break;
            }
        }
        fullpath[n - 1] = 0;
        if(closedir(dp) < 0) perror("close()");
        return ret;
    }
}

static int myfind(const char* pathname, Myfunc* func, int type) {
    strcpy(fullpath, pathname);
    struct stat statres;
    if(lstat(fullpath, &statres) < 0) {
        fprintf(stderr, "%s ", fullpath);
        perror("stat()");
        exit(1);
    }
    if(!S_ISDIR(statres.st_mode)) { // 如果是文件
        fprintf(stderr, "error: %s is a filename, but it should be a pathname\n", fullpath);
        exit(1);
    }
    return (dopath(func, type));
}

int main(int argc, char** argv) {
    int ret;
    if(argc < 2 || argc == 3) { // 参数数量不符 报错
        fprintf(stderr, "Usage: ./myfind <pathname> [-comp filename] [-name str]\n");
        exit(1);
    }
    if(argc == 2) {
        ret = myfind(argv[1], myfunc1, TYPE_P);
        ntot = nreg + ndir + nblk + nchr + nfifo + nslink + nsock;
        if(ntot == 0) ntot = 1; // 避免除数为0 所有文件所占的百分比都为0
        //fprintf(stdout, "regular files   = %5ld %9.2f %%\n", nreg, nreg * 100.0 / ntot);
        //fprintf(stdout, "directories     = %5ld %9.2f %%\n", ndir, ndir * 100.0 / ntot);
        //fprintf(stdout, "block special   = %5ld %9.2f %%\n", nblk, nblk * 100.0 / ntot);
        //fprintf(stdout, "char special    = %5ld %9.2f %%\n", nchr, nchr * 100.0 / ntot);
        //fprintf(stdout, "FIFOs           = %5ld %9.2f %%\n", nfifo, nfifo * 100.0 / ntot);
        //fprintf(stdout, "symbolic links  = %5ld %9.2f %%\n", nslink, nslink * 100.0 / ntot);
        //fprintf(stdout, "sockets         = %5ld %9.2f %%\n", nsock, nsock * 100.0 / ntot);
        fprintf(stdout, "There are %ld files with a length greater than 4096 bytes and the ratio is %5.2f%%.\n", nlrg, nlrg * 100.0 / ntot);
    } else {
        if(strcmp(argv[2], "-comp") == 0) {
            if(argc > 4) {
                fprintf(stderr, "Usage: ./myfind <pathname> [-comp filename]\n");
                exit(1);
            }
            struct stat statres;
            if(lstat(argv[1], &statres) < 0) {
                fprintf(stderr, "%s: ", argv[1]);
                perror("");
                exit(1);
            }
            strcpy(c_filename, argv[3]);
            //printf("203G c_filename: %s\n", c_filename);
            int fd = open(argv[3], O_RDONLY);
            if(lstat(c_filename, &c_filestat) < 0) perror("stat()"), exit(1);
            if(S_ISDIR(c_filestat.st_mode)) {
                fprintf(stderr, "%s is a directory, but it should be a filename\n", c_filename);
                exit(1);
            }
            if((c_filelen = read(fd, c_filebuf, BUFSIZE)) < 0) {
                perror("read()");
                exit(1);
            }
            ret = myfind(argv[1], myfunc2, TYPE_C);
        } else if(strcmp(argv[2], "-name") == 0) {
            str_num = argc - 3;
            // str都是文件名
            int i;
            for(i = 0; i < str_num; i++) {
                strcpy(name_str[i], argv[i + 3]);
            }
            ret = myfind(argv[1], myfunc3, TYPE_N);
        } else {
            fprintf(stderr, "Usage: ./myfind <pathname> [-comp filename] [-name str]\n");
            exit(1);
        }
    }
    exit(ret);
}

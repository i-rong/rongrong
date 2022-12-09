#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <glob.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>

#define FIND_F 1 // file other than directory
#define FIND_D 2 // directory
#define FIND_DNR 3 // directory that can't be read
#define FIND_NS 4 // file that we can't stat

#define PATHSIZE 1024
static char fullpath[PATHSIZE];
static char tmp[PATHSIZE];

typedef int Myfunc(const char*, const struct stat*, int);
static long nreg, ndir, nblk, nchr, nfifo, nslink, nsock, ntot, nlit; // 分别表示常规文件、目录文件...

static int path_noloop(const char* pathname) {
    char* pos;
    pos = strrchr(pathname, '/'); // 取到最右边的'/'的位置  保存在pos中
    if(pos == NULL) exit(1); // 如果没有'/'
    if(strcmp(pos + 1, ".") == 0 || strcmp(pos + 1, "..") == 0) return 0;
    else return 1;
}

static int myfunc(const char* pathname, const struct stat* statptr, int type) {
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
            if(statptr->st_size <= 4096) nlit++;
            break;
        case FIND_D:
            ndir++; break;
        case FIND_DNR:
            fprintf(stderr, "can't read directory %s", pathname); break;
        case FIND_NS:
            fprintf(stderr, "stat error for %s", pathname); break;
        default:
            fprintf(stderr, "unknown type %d for pathname %s", type, pathname);
    }
    return 0;
}

static int dopath(Myfunc* func) {
    struct stat statbuf;
    glob_t globres;
    int ret, n;
    int err;
    if(lstat(fullpath, &statbuf) < 0) // stat error
        return (func(fullpath, &statbuf, FIND_NS));

    if(S_ISDIR(statbuf.st_mode) == 0) // 不是目录
        return (func(fullpath, &statbuf, FIND_F));
    // 下面是目录
    if((ret = func(fullpath, &statbuf, FIND_D)) != 0) return ret; // 如果不是0表示出错 这里直接返回
    strcat(fullpath, "/*"); // 处理该目录下的所有非隐藏文件
    err = glob(fullpath, 0, NULL, &globres);
    if(err != 0) {
        fprintf(stderr, "glob error\n");
        exit(1);
    }
    strncpy(fullpath, tmp, PATHSIZE);
    strcat(fullpath, "/.*"); // 处理该目录下的所有隐藏文件
    err = glob(fullpath, GLOB_APPEND, NULL, &globres); // 追加写
    if(err != 0) {
        fprintf(stderr, "glob error\n");
        exit(1);
    }

    int i;
    for(i = 0; i < globres.gl_pathc; i++) {
        if(path_noloop(globres.gl_pathv[i])) {
            strcpy(fullpath, globres.gl_pathv[i]);
            dopath(func);
        }
    }
    globfree(&globres);
    return ret;
}


static int myfind(const char* filename, Myfunc* func) {
    strcpy(fullpath, filename);
    strcpy(tmp, filename);
    return (dopath(func));
}

int main(int argc, char** argv) {
    int ret;
    if(argc < 2) { // 如果只有一个参数 报错
        fprintf(stderr, "Usage: ./myfind <pathname>\n");
        exit(1);
    }
    ret = myfind(argv[1], myfunc);
    //ret = dopath(argv[1], myfunc);
    ntot = nreg + ndir + nblk + nchr + nfifo + nslink + nsock;
    if(ntot == 0) ntot = 1; // 避免除数为0 所有文件所占的百分比都为0
    fprintf(stdout, "regular files   = %5ld %9.2f %%\n", nreg, nreg * 100.0 / ntot);
    fprintf(stdout, "directories     = %5ld %9.2f %%\n", ndir, ndir * 100.0 / ntot);
    fprintf(stdout, "block special   = %5ld %9.2f %%\n", nblk, nblk * 100.0 / ntot);
    fprintf(stdout, "char special    = %5ld %9.2f %%\n", nchr, nchr * 100.0 / ntot);
    fprintf(stdout, "FIFOs           = %5ld %9.2f %%\n", nfifo, nfifo * 100.0 / ntot);
    fprintf(stdout, "symbolic links  = %5ld %9.2f %%\n", nslink, nslink * 100.0 / ntot);
    fprintf(stdout, "sockets         = %5ld %9.2f %%\n", nsock, nsock * 100.0 / ntot);
    fprintf(stdout, "<=4096Bytes     = %5ld %9.2f %%\n", nlit, nlit * 100.0 / ntot);

    exit(ret);
}

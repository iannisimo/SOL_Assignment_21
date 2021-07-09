#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <limits.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "utils.h"

static char debugPrint = 0;

int millisleep(int millis) {
    struct timespec nssleep;
    nssleep.tv_sec = ((long) millis) / 1000l;
    nssleep.tv_nsec = (millis % 1000) * 1000000;
    RET_ON((nanosleep(&nssleep, NULL)), -1, -1);
    errno = 0;
    return 0;
}

void setDebug(char dgb) {
    debugPrint = dgb;
}

char getNumber(char* str, int* val) {
    char* endptr = NULL;
    errno = 0;
    *val = (int) strtol(str, &endptr, 10);
    if(errno != 0 || endptr == NULL || *endptr != '\0') {
        return 0;
    }
    return 1;
}

int getSZ(char* str, size_t* val) {
    char* endptr = NULL;
    errno = 0;
    long tmp = strtol(str, &endptr, 10);
    if(errno != 0 || endptr == NULL || *endptr != '\0') {
        return -1;
    }
    *val = (size_t) tmp;
    return 0;
}

static int strncnt(char *str, char elem, int max) {
    RET_ON(str, NULL, -1);
    int count = 0;
    int i = 0;
    char c = str[i];
    while(c != '\0' && i <= max) {
        if(c == elem) count++;
        c = str[++i];
    }
    return count;
}

int tokenize(char *string, char* separator, char type, int *n, char ***args) {
    char **strings = NULL;
    RET_ON(string, NULL, -1);
    RET_ON(string[0], '-', ERR_INVALID);
    if(type == 'W' || type == 'r' || type == 'c') {
        int count = strncnt(string, separator[0], PATH_MAX) + 1;
        *n = count;
        RET_ON((strings = malloc(count * sizeof(char*))), NULL, ERR_NO_MEMORY);
        char *token = strtok(string, separator);
        for(int i = 0; i < count; i++) {
            if(token == NULL) break;
            int token_len = strnlen(token, PATH_MAX) + 1;
            RET_ON((strings[i] = malloc(token_len * sizeof(char))), NULL, ERR_NO_MEMORY);
            strncpy(strings[i], token, token_len);
            token = strtok(NULL, separator);
        }
    } else if(type == 'R') {
        if(strncmp(string, "n=", 2) == 0) {
            RET_ON((getNumber(string + 2, n)), 0, ERR_DOMAIN);
        } else {
            *n = 0;
        }
    } else if(type == 'w') {
        char *dir = strtok(string, separator);
        RET_ON(dir, NULL, -1);
        RET_ON((strings = malloc(sizeof(char*))), NULL, ERR_NO_MEMORY);
        int dirLen = strnlen(dir, PATH_MAX) + 1;
        RET_ON((strings[0] = malloc(dirLen * sizeof(char))), NULL, ERR_NO_MEMORY);
        strncpy(strings[0], dir, dirLen);
        char *nStr = strtok(NULL, separator);
        if(nStr != NULL) {
            if(strncmp(nStr, "n=", 2) == 0) {
                RET_ON((getNumber(nStr + 2, n)), 0, ERR_DOMAIN);
            } else {
                *n = 0;
            }
        }
    } else if(type == 't') {
        RET_ON((getNumber(string, n)), 0, ERR_DOMAIN);
    }
    *args = strings;
    return 0;
}

int debugf(const char *fmt, ...) {
    if(debugPrint == 0) return 0;
    va_list args;
    va_start(args, fmt);
    int r = vfprintf(stdout, fmt, args);
    va_end(args);
    return r;
}

int mkdir_p(char *path) {
    struct stat sb;
    for(char *p = path + 1; *p; p++) {
        if(*p == '/') {
            *p = '\0';
            if(stat(path, &sb) != 0) {
                // Path does not exist
                if(mkdir(path, S_IRWXU) == -1) {
                    *p = '/';
                    return -1;
                }
            } else if(!S_ISDIR(sb.st_mode)) {
                *p = '/';
                return -1;
            }
            *p = '/';
        }
    }
    return 0;
}

int writeToFolder(char *filename, char *dirname, void *data, size_t size) {
    size_t filelen = strnlen(filename, PATH_MAX);
    size_t pathlen = strnlen(dirname, PATH_MAX);
    if(pathlen + filelen >= PATH_MAX) {
        return ENAMETOOLONG;
    }
    char fullPath[PATH_MAX];
    strncpy(fullPath, dirname, pathlen);
    strncpy(fullPath + pathlen, filename, filelen+1);
    RET_ON(mkdir_p(fullPath), -1, -1);
    int file_fd;
    RET_ON((file_fd = open(fullPath, O_WRONLY | O_CREAT, 0644)), -1, -1);
    RET_ON(write(file_fd, data, size), -1, -1);
    RET_ON(close(file_fd), -1, -1);
    return 0;
}

int getFileBytes(char *pathname, char *absPath, void **buf, size_t *size) {
    int fd;
    RET_ON((fd = open(pathname, O_RDONLY)), -1, -1);
    struct stat st;
    RET_ON(fstat(fd, &st), -1, errno);
    *size = st.st_size;
    if(absPath != NULL)
        RET_ON(realpath(pathname, absPath), NULL, -1);
    RET_ON((*buf = malloc(*size + 1)), NULL, -1);
    RET_ON(read(fd, *buf, *size), -1, errno);
    return 0;
}

int getAbsPath(char *relPath, char *absPath) {
    char *tmp = realpath(relPath, absPath);
    if(tmp != NULL) return 0;
    if(tmp == NULL && errno != ENOENT) {
        return -1;
    }
    int rellen = strnlen(relPath, PATH_MAX);
    tmp = getcwd(absPath, PATH_MAX - rellen);
    if(tmp == NULL) return -1;
    int cwdlen = strnlen(absPath, PATH_MAX);
    absPath[cwdlen] = '/';
    strncpy(absPath + cwdlen + 1, relPath, rellen);
    return 0;
}
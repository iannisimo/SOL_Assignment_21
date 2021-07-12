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
#include "connectionAPI.h"

static char debugPrint = 0;

/**
 * @brief Print a void* as a string of length len
 */
void print_void(void* buf, size_t len) {
    for(int i = 0; i < len; i++) {
        debugf("%c", ((char *) buf)[i]);
    }
    debugf("\n");
}

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

/**
 * @brief Convert a string to an int value 
 * 
 * @return 0 on success, -1 on error
 */
char getNumber(char* str, int* val) {
    char* endptr = NULL;
    errno = 0;
    *val = (int) strtol(str, &endptr, 10);
    if(errno != 0 || endptr == NULL || *endptr != '\0') {
        if(errno == 0) errno = EDOM;
        return -1;
    }
    return 0;
}

/**
 * @brief Convert a string to a size_t value 
 * 
 * @return 0 on success, -1 on error
 */
int getSZ(char* str, size_t* val) {
    char* endptr = NULL;
    errno = 0;
    long tmp = strtol(str, &endptr, 10);
    if(errno != 0 || endptr == NULL || *endptr != '\0') {
        if(errno == 0) errno = EDOM;
        return -1;
    }
    *val = (size_t) tmp;
    return 0;
}

/**
 * @brief Count the occurrecnces of elem in str up to max
 * 
 * @return The number of occurrences found
 */
int strncnt(char *str, char elem, int max) {
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

/**
 * @brief Same as printf but prints iif (global) debugPrint == 1
 * 
 * @return Upon successful this function returns the number of characters printed (excluding the null byte used to end output to strings)
 */
int debugf(const char *fmt, ...) {
    if(debugPrint == 0) return 0;
    va_list args;
    va_start(args, fmt);
    int r = vfprintf(stdout, fmt, args);
    va_end(args);
    return r;
}

/**
 * @brief Equivalent to mkdir -p
 * 
 * @return 0 on success, -1 on error
 */
int mkdir_p(char *path) {
    struct stat sb;
    for(char *p = path + 1; *p; p++) {
        if(*p == '/') {
            *p = '\0';
            if(stat(path, &sb) != 0) {
                // Path does not exist
                if(mkdir(path, S_IRWXU) == -1) {
                    *p = '/';
                    errno = ENOENT;
                    return -1;
                }
            } else if(!S_ISDIR(sb.st_mode)) {
                *p = '/';
                errno = ENOTDIR;
                return -1;
            }
            *p = '/';
        }
    }
    return 0;
}

/**
 * @brief Create a file in /dirname/filename and populate it with data
 * 
 * @return 0 on success, -1 on error
 */
int writeToFolder(char *filename, char *dirname, void *data, size_t size) {
    size_t filelen = strnlen(filename, PATH_MAX);
    size_t pathlen = strnlen(dirname, PATH_MAX);
    if(pathlen + filelen >= PATH_MAX) {
        errno = ENAMETOOLONG;
        return -1;
    }
    char fullPath[PATH_MAX];
    strncpy(fullPath, dirname, pathlen);
    strncpy(fullPath + pathlen, filename, filelen+1);
    API_ERR(mkdir_p(fullPath), -1, -1);
    int file_fd;
    API_ERR((file_fd = open(fullPath, O_WRONLY | O_CREAT, 0644)), -1, -1);
    API_ERR(write(file_fd, data, size), -1, -1);
    API_ERR(close(file_fd), -1, -1);
    return 0;
}

/**
 * @brief Read the file in pathname and return it's data, size, and absolute path
 * 
 * @return 0 on success, -1 on error
 */
int getFileBytes(char *pathname, char *absPath, void **buf, size_t *size) {
    int fd;
    RET_ON((fd = open(pathname, O_RDONLY)), -1, errno);
    struct stat st;
    RET_ON(fstat(fd, &st), -1, errno);
    *size = st.st_size;
    if(absPath != NULL)
        RET_ON(realpath(pathname, absPath), NULL, -1);
    RET_ON((*buf = malloc(*size + 1)), NULL, -1);
    RET_ON(read(fd, *buf, *size), -1, errno);
    return 0;
}

/**
 * @brief Get the absolute path of relPath; if relPath doesn't exist [absPath = <cwd>/<relPath>]
 * 
 * @return 0 on success, -1 on error
 */
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
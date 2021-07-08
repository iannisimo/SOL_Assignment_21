#include "const.h"
#include "connectionAPI.h"
#include "utils.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <limits.h>


#include <stdio.h>

static int sfd;
static char currSockName[UNIX_PATH_MAX];

int readUntil(int fd, char* buf, int len, char delim) {
    char tmpBuf[1];
    for(int i = 0; i < len; i++) {
        RET_ON((read(fd, tmpBuf, 1)), -1, -1);
        if(tmpBuf[0] == delim) {
            buf[i] = '\0';
            return i + 1;
        }
        buf[i] = tmpBuf[0];
    }
    return 0;
}

int waitResponse(size_t *size, void **buf, char *pathname) {
    static char stBuf[128];
    int status;
    RET_ON(readUntil(sfd, stBuf, 127, '\0'), -1, -1);
    char *token;
    RET_ON((token = strtok((char *) stBuf, " ")), NULL, -1);
    RET_ON((getNumber(token, &status)), 0, -1);
    RET_ON((token = strtok(NULL, " ")), NULL, status);
    RET_ON(size, NULL, -1);
    RET_ON((getSZ(token, size)), -1, -1);
    token = strtok(NULL, " ");
    if(token != NULL) {
        RET_ON(pathname, NULL, -1);
        strncpy(pathname, token, PATH_MAX);
    }
    RET_ON((*buf = malloc(*size)), NULL, -1);
    RET_ON(read(sfd, *buf, *size), -1, -1);
    return status;
}

int openConnection(const char *sockname, int msec, const struct timespec abstime) {
    API_ERR((sfd = socket(AF_UNIX, SOCK_STREAM, 0)), -1, -1);
    struct sockaddr_un sa;
    strncpy(sa.sun_path, sockname, UNIX_PATH_MAX - 1);
    sa.sun_family = AF_UNIX; 
    int maxRetry = (int) (abstime.tv_nsec / (((long) msec) * 1000000));
    for(int i = 0; i < maxRetry; i++) {
        if(connect(sfd, (struct sockaddr*) &sa, sizeof(sa)) == -1) {
            if (errno == ENOENT) {
                API_ERR((millisleep(msec)), -1, -1);
            } else {
                return -1;
            }
        } else {
            break;
        }
    }
    strncpy(currSockName, sockname, UNIX_PATH_MAX - 1);
    errno = 0;
    return 0;
}

int closeConnection(const char *sockname) {
    if(strncmp(currSockName, sockname, UNIX_PATH_MAX) == 0) {
        currSockName[0] = '\0';
        API_ERR(close(sfd), -1, -1);
    } else {
        return -1;
    }
    errno = 0;
    return 0;
}

int openFile(const char *pathname, int flags) {
    if(IS_LOCK(flags)) {
        errno = EOPNOTSUPP;
        return -1;
    }
    char type = (IS_CREATE(flags)) ? 'c' : 'o';
    size_t path_len = strnlen(pathname, PATH_MAX + 1);
    if(path_len > PATH_MAX) {
        errno = EOVERFLOW;
        return -1;
    }
    char *buf;
    API_ERR((buf = malloc(path_len + 3)), NULL, -1);
    snprintf(buf, path_len + 2, "%c%s", type, pathname);
    API_ERR((write(sfd, buf, path_len + 2)), -1, -1);
    free(buf);
    int status;
    API_ERR((status = waitResponse(NULL, NULL, NULL)), -1, -1);
    return status;
}

int readFile(const char *pathname, void **buf, size_t* size) {
    size_t path_len = strnlen(pathname, PATH_MAX + 1);
    char *tmpBuf;
    API_ERR((tmpBuf = malloc(path_len + 3)), NULL, -1);
    int len = snprintf(tmpBuf, path_len + 2, "r%s", pathname);
    API_ERR((write(sfd, tmpBuf, len + 1)), -1, -1);
    free(tmpBuf);
    int status;
    API_ERR((status = waitResponse(size, buf, NULL)), -1, -1);
    return status;
}

int readNFiles(int N, const char *dirname) {
    char tmpBuf[128];
    int len = snprintf(tmpBuf, 127, "R%d", N);
    API_ERR((write(sfd, tmpBuf, len + 1)), -1, -1);
    int status = 0;
    size_t size;
    void *data;
    char pathname[PATH_MAX];
    char absRcv[PATH_MAX];
    if(dirname != NULL) {
        char relRcv[PATH_MAX];
        snprintf(relRcv, PATH_MAX, "%s", dirname);
        RET_ON(getAbsPath(relRcv, absRcv), -1, -1);
    }
    while((status = waitResponse(&size, &data, pathname)) == 0) {
        // SaveToDir(buf, size, dirname);
        if(dirname != NULL) {
            RET_ON(writeToFolder(pathname, absRcv, data, size), -1, -1);
        } else {
            printf("%s\n\t%ld\t%s\n", pathname, size, (char *) data);
        }
    }
    RET_ON(status, -1, -1);
    return status;
}


int writeFile(const char *pathname, const char *dirname) {
    errno = ENOTSUP;
    return -1;
}

int appendToFile(const char *pathname, void *buf, size_t size, const char *dirname) {
    if(dirname != NULL) {
        errno = ENOTSUP;
        return -1;
    }
    size_t path_len = strnlen(pathname, PATH_MAX + 1);
    char *tmpBuf;
    API_ERR((tmpBuf = malloc(path_len + 128)), NULL, -1);
    int len = snprintf(tmpBuf, path_len + 128, "a%ld %s", size, pathname);
    API_ERR((write(sfd, tmpBuf, len + 1)), -1, -1);
    API_ERR((write(sfd, buf, size)), -1, -1);
    free(tmpBuf);
    int status;
    API_ERR((status = waitResponse(NULL, NULL, NULL)), -1, -1);
    return status;
}

int lockFile(const char *pathname) {
    errno = ENOTSUP;
    return -1;
}

int unlockFile(const char *pathname) {
    errno = ENOTSUP;
    return -1;
}

int closeFile(const char *pathname) {
    size_t path_len = strnlen(pathname, PATH_MAX + 1);
    if(path_len > PATH_MAX) {
        errno = EOVERFLOW;
        return -1;
    }
    char *buf;
    API_ERR((buf = malloc(path_len + 3)), NULL, -1);
    int len = snprintf(buf, path_len + 2, "C%s", pathname);
    API_ERR((write(sfd, buf, len + 1)), -1, -1);
    free(buf);
    int status;
    API_ERR((status = waitResponse(NULL, NULL, NULL)), -1, -1);
    return status;
}

int removeFile(const char *pathname) {
    errno = ENOTSUP;
    return -1;
}
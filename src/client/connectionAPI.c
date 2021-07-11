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

// Check errno / return value

static int sfd;
static char currSockName[UNIX_PATH_MAX];

/**
 * @brief Read from fd until delim in encountered
 * 
 * @return 0 on success, -1 on error
 */
int readUntil(int fd, char* buf, int len, char delim) {
    char tmpBuf[1];
    for(int i = 0; i < len; i++) {
        API_ERR((read(fd, tmpBuf, 1)), -1, -1);
        if(tmpBuf[0] == delim) {
            buf[i] = '\0';
            return i + 1;
        }
        buf[i] = tmpBuf[0];
    }
    return 0;
}

/**
 * @brief Wait for the server to respond to a request, it could contain: status [size [pathname] buf] depending on the request
 * 
 * @return The status code from the server, -1 on error
 */
int waitResponse(size_t *size, void **buf, char *pathname) {
    static char stBuf[PATH_MAX + 128];
    int status;
    API_ERR(readUntil(sfd, stBuf, PATH_MAX+127, '\0'), -1, -1);
    char *token;
    API_ERR((token = strtok((char *) stBuf, " ")), NULL, -1);
    API_ERR((getNumber(token, &status)), -1, -1);
    RET_ON((token = strtok(NULL, " ")), NULL, status);
    API_ERR(size, NULL, EINVAL);
    API_ERR((getSZ(token, size)), -1, -1);
    token = strtok(NULL, " ");
    if(token != NULL) {
        API_ERR(pathname, NULL, EINVAL);
        strncpy(pathname, token, PATH_MAX);
    }
    API_ERR((*buf = malloc(*size)), NULL, -1);
    ssize_t tmpBufLen = 0;
    while((*size - tmpBufLen) > 0) {
        void* ptr = (void*)(((char*) *buf) + tmpBufLen);
        API_ERR ((tmpBufLen += read(sfd, ptr, *size - tmpBufLen)), -1, -1);
    }
    return status;
}

/**
 * @brief Opens the connection to sockname
 * 
 * @return 0 on success, -1 on error
 */
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

/**
 * @brief Closes the connection with the server if sockname corresponds to the opened one
 * 
 * @return 0 on success, -1 on error
 */
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

/**
 * @brief Sends a request to the server to open or create the file pathname
 * 
 * @return 0 on success, -1 on error
 */
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

/**
 * @brief Requests to the server the file pathname and saves the data to buf
 * 
 * @return 0 on success, -1 on error
 */
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

/**
 * @brief Read N files from a server and save them to dirname (if not NULL)
 * 
 * @return 0 on success, -1 on error
 */
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
        API_ERR(getAbsPath(relRcv, absRcv), -1, -1);
    }
    while((status = waitResponse(&size, &data, pathname)) == 0) {
        if(dirname != NULL) {
            API_ERR(writeToFolder(pathname, absRcv, data, size), -1, -1);
        } else {
            debugf("\nfilename:\n%s\ndata:\n%s\n", pathname, (char *) data);
        }
    }
    RET_ON(status, -1, -1);
    RET_ON(status, ENODATA, 0);
    return status;
}

int writeFile(const char *pathname, const char *dirname) {
    errno = ENOTSUP;
    return -1;
}

/**
 * @brief Append buf data to pathname
 * 
 * @return 0 on success, -1 on error
 */
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

/**
 * @brief Request to the server to close the file pathname
 * 
 * @return 0 on success, -1 on error
 */
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
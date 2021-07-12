#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include "utils.h"

/**
 * @brief Convert a string to int
 * 
 * @return 0 on success, EINVAL on error
 */
int getInt(char* str, int* val) {
    char* endptr = NULL;
    errno = 0;
    long tmp = strtol(str, &endptr, 10);
    if(errno != 0 || endptr == NULL || *endptr != '\0' || tmp > INT_MAX) {
        return EINVAL;
    }
    *val = (int) tmp;
    return 0;
}

/**
 * @brief Convert a string to long
 * 
 * @return 0 on success, EINVAL on error
 */
int getLong(char* str, long* val) {
    char* endptr = NULL;
    errno = 0;
    long tmp = strtol(str, &endptr, 10);
    if(errno != 0 || endptr == NULL || *endptr != '\0') {
        return EINVAL;
    }
    *val = tmp;
    return 0;
}

/**
 * @brief Convert a string to long long
 * 
 * @return 0 on success, EINVAL on error
 */
int getLLong(char* str, long long* val) {
    char* endptr = NULL;
    errno = 0;
    long long tmp = strtoull(str, &endptr, 10);
    if(errno != 0 || endptr == NULL || *endptr != '\0') {
        return EINVAL;
    }
    *val = tmp;
    return 0;
}

/**
 * @brief Convert a string to size_t
 * 
 * @return 0 on success, EINVAL on error
 */
int getSz(char *str, size_t *val) {
    char* endptr = NULL;
    errno = 0;
    unsigned long tmp = strtoul(str, &endptr, 10);
    if(errno != 0 || endptr == NULL || *endptr != '\0') {
        return EINVAL;
    }
    *val = (size_t) tmp;
    return 0;
}

/**
 * @brief Reads from fd until delim is found
 * 
 * @return On  success, the number of bytes read is returned, -1 on error
 */
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
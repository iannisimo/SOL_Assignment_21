#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include "utils.h"

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

int getSz(char *str, size_t *val) {
    char* endptr = NULL;
    errno = 0;
    unsigned long tmp = strtoul(str, &endptr, 10);
    if(errno != 0 || endptr == NULL || *endptr != '\0') {
        return -1;
    }
    *val = (size_t) tmp;
    return 0;
}

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

// void Pthread_mtx_lock(pthread_mutex_t* mtx) {
//     int err;
//     if((err = pthread_mutex_lock(mtx)) != 0) {
//         errno = err;
//         perror("lock");
//         pthread_exit((void*) &errno);
//     }
// }

// void Pthread_mtx_unlock(pthread_mutex_t* mtx) {
//     int err;rrno = err
//     if((err = pthread_mutex_unlock(mtx)) != 0) {
//         e;
//         perror("unlock");
//         pthread_exit((void*) &errno);
//     }
// }

// void Pthread_cond_wait(pthread_mutex_t* mtx, pthread_cond_t* cond) {
//     int err;
//     if((err = pthread_cond_wait(cond, mtx)) != 0) {
//         errno = err;
//         perror("wait");
//         pthread_exit((void*) &errno);
//     }
// }

// void Pthread_cond_signal(pthread_cond_t* cond) {
//     int err;
//     if((err = pthread_cond_signal(cond)) != 0) {
//         errno = err;
//         perror("signal");
//         pthread_exit((void*) &errno);
//     }
// }

// void Pthread_mtx_init(pthread_mutex_t* mtx) {
//     int err;
//     if((err = pthread_mutex_init(mtx, NULL)) != 0) {
//         errno = err;
//         perror("mtx init");
//         pthread_exit((void*) &errno);
//     }
// }

// void Pthread_cond_init(pthread_cond_t* cond) {
//     int err;
//     if((err = pthread_cond_init(cond, NULL)) != 0) {
//         errno = err;
//         perror("cond init");
//         pthread_exit((void*) &errno);
//     }
// }

// void Pthread_create(pthread_t* tid, const pthread_attr_t* attr, void*(*start_routine)(void*), void* arg) {
//     int err;
//     if((err = pthread_create(tid, attr, start_routine, arg)) != 0) {
//         errno = err;
//         perror("pthread create");
//         pthread_exit((void*) &errno);
//     }
// }
#include "utils.h"

// void Pthread_mtx_lock(pthread_mutex_t* mtx) {
//     int err;
//     if((err = pthread_mutex_lock(mtx)) != 0) {
//         errno = err;
//         perror("lock");
//         pthread_exit((void*) &errno);
//     }
// }

// void Pthread_mtx_unlock(pthread_mutex_t* mtx) {
//     int err;
//     if((err = pthread_mutex_unlock(mtx)) != 0) {
//         errno = err;
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
#include <pthread.h>
#include <sys/types.h>

#include "queue.h"

#ifndef __storage_h
#define __storage_h

#define EXEC_RET_ON(l, f, e, r) \
    if(f == e) { \
        { \
            l; \
        } \
        return r; \
    }

typedef struct _storage_summary {
    size_t max_size;
    size_t max_len;
    size_t deleted;
} StorageSummary_t;

typedef struct _storage {
    struct _storage_node *head;
    struct _storage_node *tail;
    size_t max_length;
    size_t max_size;
    size_t length;
    size_t size;
    pthread_mutex_t read_mtx;
    pthread_cond_t read_cond;
    pthread_mutex_t write_mtx;
    pthread_cond_t write_cond;
    size_t readers;
    size_t writers;
    struct _storage_summary summary;
    char writing;
} Storage_t;

typedef struct _storage_node {
    struct _storage_node *prev;
    struct _storage_node *next;
    char *filename;
    void *data;
    size_t size;
    Queue_t *clients;
    pthread_mutex_t mutex;
} StorageNode_t;

Storage_t *initStorage(size_t max_len, size_t max_size);
int StorageDestroy(Storage_t *storage);
void printSummary(Storage_t *storage);
int openFile(Storage_t *storage, int fd, char create, char* filename);
int closeFile(Storage_t *storage, int fd, char* filename);
Storage_t *pushFile(Storage_t *, char *, size_t, void *);
int readFile(Storage_t *storage, int fd, char *filename, size_t *size, void **data);
int readIthFile(Storage_t *storage, int index, size_t *size, void **data, char **pathname);
int appendToFile(Storage_t *storage, int fd, char *filename, size_t size, void *data);
int closeAllFiles(Storage_t *storage, int fd);

#endif
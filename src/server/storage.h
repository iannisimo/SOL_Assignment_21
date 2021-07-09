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

typedef struct _storage {
    struct _storage_node *head; // Pointer to the head of the list
    struct _storage_node *tail; // Pointer to the tail of the list
    size_t max_length;
    size_t max_size;
    size_t length;              // Number of files currently saved
    size_t size;                // Cumulative size of the files
    pthread_mutex_t read_mtx;      
    pthread_cond_t read_cond;
    pthread_mutex_t write_mtx;      
    pthread_cond_t write_cond;
    size_t readers;
    size_t writers;
    char writing;
} Storage_t;

typedef struct _storage_node {
    struct _storage_node *prev; // Previous node in the list
    struct _storage_node *next; // Next node in the list
    char *filename;             // Absolute path of the file
    void *data;                 // Pointer to the bytearray containing the file data
    size_t size;                // Size of `data`
    Queue_t *clients;           // File descriptors of the clients who opened the file
    pthread_mutex_t mutex;      // Mutex user do block certain operations for multithread concurrency
} StorageNode_t;

Storage_t *initStorage(size_t max_len, size_t max_size);
int StorageDestroy(Storage_t *storage);
int openFile(Storage_t *storage, int fd, char create, char* filename);
int closeFile(Storage_t *storage, int fd, char* filename);
Storage_t *pushFile(Storage_t *, char *, size_t, void *);
int readFile(Storage_t *storage, int fd, char *filename, size_t *size, void **data);
int readIthFile(Storage_t *storage, int index, size_t *size, void **data, char **pathname);
int appendToFile(Storage_t *storage, int fd, char *filename, size_t size, void *data);
int closeAllFiles(Storage_t *storage, int fd);

#endif
#include <pthread.h>
#include <sys/types.h>

#include "queue.h"

#ifndef __storage_h
#define __storage_h


typedef struct _storage {
    struct _storage_node *head; // Pointer to the head of the list
    struct _storage_node *tail; // Pointer to the tail of the list
    size_t length;              // Number of files currently saved
    size_t size;                // Cumulative size of the files
    pthread_mutex_t mutex;      // Mutex used to safely change lenght and size
    pthread_mutex_t searching_mtx;      
    pthread_cond_t searching_cond;
    pthread_mutex_t cleaning_mtx;      
    pthread_cond_t cleaning_cond;
    size_t searching;
    char cleaning;
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
int openFile(Storage_t *storage, int fd, char create, char* filename);
int closeFile(Storage_t *storage, int fd, char* filename);
Storage_t *pushFile(Storage_t *, char *, size_t, void *);
int readFile(Storage_t *storage, int fd, char *filename, size_t *size, void **data);
int appendFromSocket(Storage_t *storage, char *filename, size_t size, int fd);

#endif
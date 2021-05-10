#include <pthread.h>
#include <sys/types.h>

#ifndef __storage_h
#define __storage_h


typedef struct _storage {
    struct _storage_node *head; // Pointer to the head of the list
    struct _storage_node *tail; // Pointer to the tail of the list
    size_t lenght;              // Number of files currently saved
    size_t size;                // Cumulative size of the files
    pthread_mutex_t mutex;      // Mutex used to safely change lenght and size
} Storage_t;

typedef struct _storage_node {
    struct _storage_node *prev; // Previous node in the list
    struct _storage_node *next; // Next node in the list
    char *filename;             // Absolute path of the file
    unsigned char *data;        // Pointer to the bytearray containing the file data
    size_t size;                // Size of `data`
    pthread_mutex_t mutex;      // Mutex user do block certain operations for multithread concurrency
} StorageNode_t;

typedef struct _opened_file {
    struct _storage_node *node;
    struct _opened_file *next;
} OpenedFile_t;

typedef struct _opened_files {
    struct _opened_file *head;
} OpenedFiles_t;

Storage_t *initStorage();
Storage_t *pushFile(Storage_t *, char *, size_t, unsigned char *);

#endif
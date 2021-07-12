#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include "storage.h"
#include "utils.h"
#include "const.h"
#include "queue.h"

// Implementation of a multi-reader, single writer "lock" 

int init_read(Storage_t *storage) {
    RET_PT(pthread_mutex_lock(&storage->read_mtx), -1);
    while(storage->writers > 0) {
        RET_PT(pthread_cond_wait(&storage->read_cond, &storage->read_mtx), -1);
    }
    storage->readers += 1;
    RET_PT(pthread_mutex_unlock(&storage->read_mtx), -1);
    return 0;
}

int end_read(Storage_t *storage) {
    RET_PT(pthread_mutex_lock(&storage->read_mtx), -1);
    storage->readers -= 1;
    if(storage->readers < 0) storage->readers = 0;
    RET_PT(pthread_cond_signal(&storage->write_cond), -1);
    RET_PT(pthread_mutex_unlock(&storage->read_mtx), -1);
    return 0;
}

int init_write(Storage_t *storage) {
    RET_PT(pthread_mutex_lock(&storage->write_mtx), -1);
    storage->writers += 1;
    while((storage->readers > 0) || (storage->writing)) {
        RET_PT(pthread_cond_wait(&storage->write_cond, &storage->write_mtx), -1);
    }
    storage->writing = 1;
    RET_PT(pthread_mutex_unlock(&storage->write_mtx), -1);
    return 0;
}

int end_write(Storage_t *storage) {
    RET_PT(pthread_mutex_lock(&storage->write_mtx), -1);
    storage->writers -= 1;
    if(storage->writers < 0) storage->writers = 0;
    storage->writing = 0;
    if(storage->writers > 0) {
        RET_PT(pthread_cond_signal(&storage->write_cond), -1);
    } else {
        RET_PT(pthread_cond_broadcast(&storage->read_cond), -1);
    }
    RET_PT(pthread_mutex_unlock(&storage->write_mtx), -1);
    return 0;
}

// ************************************************************************************ //

/**
 * @brief Create a new StorageNode
 */
StorageNode_t *createNode(StorageNode_t *next, StorageNode_t *prev, char *filename, unsigned char *data, size_t size) {
    StorageNode_t *node;
    RET_MALLOC(node, StorageNode_t, NULL);
    node->next = next;
    node->prev = prev;
    node->filename = filename;
    node->data = data;
    node->size = size;
    node->clients = init_queue();
    RET_PT(pthread_mutex_init(&node->mutex, NULL), NULL);
    return node;
}

/**
 * @brief Initialize a Storage object to it's default state
 */
Storage_t *initStorage(size_t max_len, size_t max_size) {
    Storage_t *storage;
    RET_MALLOC(storage, Storage_t, NULL);
    RET_ON((storage->head = createNode(NULL, NULL, NULL, NULL, 0)), NULL, NULL);
    storage->tail = storage->head;
    storage->length = 0;
    storage->size = 0;
    RET_PT(pthread_mutex_init(&storage->read_mtx, NULL), NULL);
    RET_PT(pthread_mutex_init(&storage->write_mtx, NULL), NULL);
    RET_PT(pthread_cond_init(&storage->read_cond, NULL), NULL);
    RET_PT(pthread_cond_init(&storage->write_cond, NULL), NULL);
    storage->readers = 0;
    storage->writers = 0;
    storage->writing = 0;
    storage->max_length = max_len;
    storage->max_size = max_size;
    storage->summary.max_size = 0;
    storage->summary.max_len = 0;
    storage->summary.deleted = 0;
    return storage;
}

/**
 * @brief Search <filename> inside the <storage> entity 
 * 
 * @return The found StorageNode, NULL on fail or not found 
 */
StorageNode_t *findFile(Storage_t *storage, char* filename) {
    RET_ON(storage, NULL, NULL);

    RET_ON(init_read(storage), -1, NULL);

    StorageNode_t *node = storage->head;
    while(node != NULL) {
        if(node->filename != NULL) {
            if(strncmp(node->filename, filename, PATH_MAX) == 0) {
                RET_ON(end_read(storage), -1, NULL);
                return node;
            }
        }
        node = node->next;
    }

    RET_ON(end_read(storage), -1, NULL);

    return NULL;
}

/**
 * @brief Deletes enough files to not exceed the storage's capacity 
 * 
 * @return 0 on success, -1 on failure 
 */
int removeOverflow(Storage_t* storage) {
    if((storage->length <= storage->max_length) && (storage->size <= storage->max_size)) return 0;
    StorageNode_t *node = storage->head;
    RET_ON(node, NULL, -1);
    storage->summary.deleted += 1;
    storage->size -= node->size;
    storage->length -= 1;
    if(storage->size < 0) storage->size = 0;
    if(storage->length < 0) storage->length = 0;
    storage->head = node->next;
    free(node->filename);
    free(node->data);
    RET_ON(qFree(node->clients), -1, -1);
    RET_PT(pthread_mutex_destroy(&node->mutex), -1);
    free(node);
    return removeOverflow(storage);
}

/**
 * @brief Append new data on the file associated to <filename>
 * 
 * @return Operation status, -1 on failure
 */
int appendToFile(Storage_t *storage, int fd, char *filename, size_t size, void *data) {
    RET_ON(storage, NULL, -1);
    StorageNode_t *node;
    RET_ON((node = findFile(storage, filename)), NULL, ENOENT);   

    if(node->size + size > storage->max_size) return EFBIG;

    RET_PT(pthread_mutex_lock(&node->mutex), -1);
    int found;
    void *tmpPointer;
    EXEC_RET_ON(RET_PT(pthread_mutex_unlock(&node->mutex), -1), (found = qFind(node->clients, fd)), -1, -1);
    EXEC_RET_ON(RET_PT(pthread_mutex_unlock(&node->mutex), -1), found, 0, EACCES);
    EXEC_RET_ON(RET_PT(pthread_mutex_unlock(&node->mutex), -1), (tmpPointer = realloc(node->data, node->size + size)), NULL, -1);
    node->data = tmpPointer;
    void* ptr = (void*)(((char*) node->data) + node->size);
    memcpy(ptr, data, size);
    node->size += size;
    RET_PT(pthread_mutex_unlock(&node->mutex), -1);

    RET_ON(init_write(storage), -1, -1);

    storage->size += size;

    RET_ON(removeOverflow(storage), -1, -1);

    if(storage->size > storage->summary.max_size) storage->summary.max_size = storage->size; 

    RET_ON(end_write(storage), -1, -1);
    return 0;   
}

/**
 * @brief Open (or create) a file for a client (fd)
 * 
 * @return 0 on success
 */
int openFile(Storage_t *storage, int fd, char create, char* filename) {
    RET_ON(storage, NULL, -1);
    StorageNode_t *node = findFile(storage, filename);
    if(create == 1) {
        if(node != NULL) {
            return EEXIST; // Duplicate file;
        }

        RET_ON(init_write(storage),-1, -1);

        StorageNode_t *node = storage->tail;
        RET_ON((node->next = createNode(NULL, node, NULL, NULL, 0)), NULL, -1);
        storage->tail = node->next;
        RET_ON((node->filename = malloc(PATH_MAX)), NULL, -1);
        strncpy(node->filename, filename, PATH_MAX);
        node->size = 0;
        RET_ON((node->data = malloc(1)), NULL, -1);
        memcpy(node->data, "", 1);
        qPush(node->clients, fd);
        storage->length += 1;
    
        RET_ON(removeOverflow(storage), -1, -1);

        if(storage->length > storage->summary.max_len) storage->summary.max_len = storage->length;

        RET_ON(end_write(storage), -1, -1);
    } else {
        if(node == NULL) {
            return ENOENT;
        }
        int found;
        RET_ON((found = qFind(node->clients, fd)), -1, -1);
        // If already opened, do nothing and return success
        if(found == 0) {
            qPush(node->clients, fd);
            return 0;
        }
    }
    return 0;
}

/**
 * @brief Close a file for a client (fd)
 * 
 * @return 0 on success
 */
int closeFile(Storage_t *storage, int fd, char* filename) {
    RET_ON(storage, NULL, -1);
    StorageNode_t *node = findFile(storage, filename);
    RET_ON(node, NULL, ENOENT);
    RET_ON(qRemove(node->clients, fd), -1, -1);
    return 0;
}

/**
 * @brief Read filename from storage and return it's data
 * 
 * @return 0 on success
 */
int readFile(Storage_t *storage, int fd, char *filename, size_t *size, void **data) {
    RET_ON(storage, NULL, -1);
    StorageNode_t *node = findFile(storage, filename);
    RET_ON(node, NULL, ENOENT);
    int opened;
    RET_ON((opened = qFind(node->clients, fd)), -1, -1);
    RET_ON(opened, 0, EACCES);
    *size = node->size;
    *data = node->data;
    return 0;
}

/**
 * @brief Read the i-th file saved on the storage and return it's data and filename
 * 
 * @return 0 on success 
 */
int readIthFile(Storage_t *storage, int index, size_t *size, void **data, char **pathname) {
    RET_ON(storage, NULL, -1);
    if(index >= storage->length) return ENODATA;
    RET_ON(init_read(storage), -1, -1);
    StorageNode_t *node = storage->tail->prev;
    for(int i = 0; (i < index); i++) {
        node = node->prev;
    }
    RET_ON(end_read(storage), -1, -1);
    RET_ON(node, NULL, -1);
    *size = node->size;
    *data = node->data;
    *pathname = node->filename;
    return 0;
}

/**
 * @brief Close all files for a client (fd)
 * 
 * @return 0 on success 
 */
int closeAllFiles(Storage_t *storage, int fd) {
    RET_ON(storage, NULL, -1);
    StorageNode_t *node = storage->head;
    while(node != NULL) {
        RET_ON(qRemove(node->clients, fd), -1, -1);
        node = node->next;
    }
    return 0;
}

/**
 * @brief Destroy a storage node entity
 * 
 * @return 0 on success, -1 on error
 */
int StorageNodeDestroy(StorageNode_t *node) {
    RET_ON(qFree(node->clients), -1, -1);
    free(node->filename);
    free(node->data);
    RET_PT(pthread_mutex_destroy(&node->mutex), -1);
    free(node);
    return 0;
}

/**
 * @brief Destroy a storage entity
 * 
 * @return 0 on success, -1 on error
 */
int StorageDestroy(Storage_t *storage) {
    StorageNode_t *node = storage->head;
    StorageNode_t *next;
    while(node != NULL) {
        next = node->next;
        RET_ON(StorageNodeDestroy(node), -1, -1);
        node = next;
    }
    RET_PT(pthread_mutex_destroy(&storage->read_mtx), -1);
    RET_PT(pthread_mutex_destroy(&storage->write_mtx), -1);
    RET_PT(pthread_cond_destroy(&storage->read_cond), -1);
    RET_PT(pthread_cond_destroy(&storage->write_cond), -1);
    free(storage);
    return 0;
}

void printSummary(Storage_t *storage) {
    printf("The maximum number of files saved on the server was:\n\t%zu file%s\n", storage->summary.max_len, storage->summary.max_len == 1 ? "" : "s");
    printf("The maximum size occupied on the server was:\n\t%zu Byte%s\n", storage->summary.max_size, storage->summary.max_size == 1 ? "" : "s");
    printf("The cleanup algorithm has removed:\n\t%zu file%s\n", storage->summary.deleted, storage->summary.deleted == 1 ? "" : "s");
    printf("List of all files saved on the server at the end:\n");
    StorageNode_t *node = storage->head;
    while(node != NULL) {
        if(node->filename != NULL) {
            printf("\t%s\n", node->filename);
        }
        node = node->next;
    }
    if(node == storage->head->next) {
        printf("\t(empty)\n");
    }
}
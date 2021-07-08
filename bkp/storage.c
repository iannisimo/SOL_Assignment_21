#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include "storage.h"
#include "utils.h"
#include "const.h"
#include "queue.h"

static size_t MAX_LEN = 0;
static size_t MAX_SIZE = 0;

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

Storage_t *initStorage(size_t max_len, size_t max_size) {
    Storage_t *storage;
    RET_MALLOC(storage, Storage_t, NULL);
    RET_ON((storage->head = createNode(NULL, NULL, NULL, NULL, 0)), NULL, NULL);
    storage->tail = storage->head;
    storage->length = 0;
    storage->size = 0;
    RET_PT(pthread_mutex_init(&storage->mutex, NULL), NULL);
    RET_PT(pthread_mutex_init(&storage->searching_mtx, NULL), NULL);
    RET_PT(pthread_mutex_init(&storage->cleaning_mtx, NULL), NULL);
    RET_PT(pthread_cond_init(&storage->searching_cond, NULL), NULL);
    RET_PT(pthread_cond_init(&storage->cleaning_cond, NULL), NULL);
    storage->cleaning = 0;
    storage->searching = 0;
    MAX_LEN = max_len;
    MAX_SIZE = max_size;
    return storage;
}

StorageNode_t *findFile(Storage_t *storage, char* filename) {
    RET_ON(storage, NULL, NULL);

    RET_PT(pthread_mutex_lock(&storage->searching_mtx), NULL);
    while(storage->cleaning) {
        RET_PT(pthread_cond_wait(&storage->searching_cond, &storage->searching_mtx), NULL);
    }
    storage->searching += 1;
    RET_PT(pthread_mutex_unlock(&storage->searching_mtx), NULL);

    StorageNode_t *node = storage->head;
    while(node != NULL) {
        if(node->filename != NULL) {
            if(strncmp(node->filename, filename, S_PATH_MAX) == 0) {
                RET_PT(pthread_mutex_lock(&storage->searching_mtx), NULL);
                storage->searching -= 1;
                RET_PT(pthread_cond_signal(&storage->cleaning_cond), NULL);
                RET_PT(pthread_mutex_unlock(&storage->searching_mtx), NULL);
                return node;
            }
        }
        node = node->next;
    }

    RET_PT(pthread_mutex_lock(&storage->searching_mtx), NULL);
    storage->searching -= 1;
    RET_PT(pthread_cond_signal(&storage->cleaning_cond), NULL);
    RET_PT(pthread_mutex_unlock(&storage->searching_mtx), NULL);

    return NULL;
}

int removeOverflow(Storage_t* storage) {
    if((storage->length <= MAX_LEN) && (storage->size <= MAX_SIZE)) return 0;
    StorageNode_t *node = storage->head->next;
    RET_ON(node, NULL, -1);
    storage->size -= node->size;
    storage->length -= 1;
    storage->head->next = node->next;
    free(node->filename);
    free(node->data);
    RET_ON(qFree(node->clients), -1, -1);
    RET_PT(pthread_mutex_destroy(&node->mutex), -1);
    free(node);
    return removeOverflow(storage);
} 

int appendFromSocket(Storage_t *storage, char *filename, size_t size, int fd) {
    RET_ON(storage, NULL, -1);
    StorageNode_t *node;
    RET_ON((node = findFile(storage, filename)), NULL, ENOENT);
    RET_PT(pthread_mutex_lock(&node->mutex), -1);
    int found;
    RET_ON((found = qFind(node->clients, fd)), -1, -1);
    RET_ON(found, 0, EACCES);
    RET_ON((node->data = realloc(node->data, node->size + size)), NULL, -1);
    void* ptr = (void*)(((char*) node->data) + node->size);
    RET_ON(read(fd, ptr, size), -1, -1);
    node->size += size-1;
    RET_PT(pthread_mutex_unlock(&node->mutex), -1);

    RET_PT(pthread_mutex_lock(&storage->mutex), -1);
    storage->size += size;
    storage->length += 1;

    RET_PT(pthread_mutex_lock(&storage->cleaning_mtx), -1);
    while(storage->searching != 0) {
        RET_PT(pthread_cond_wait(&storage->cleaning_cond, &storage->cleaning_mtx), -1);
    }
    storage->cleaning = 1;
    RET_PT(pthread_mutex_unlock(&storage->cleaning_mtx), -1);


    RET_ON(removeOverflow(storage), -1, -1);


    RET_PT(pthread_mutex_lock(&storage->cleaning_mtx), -1);
    storage->cleaning = 0;
    RET_PT(pthread_cond_signal(&storage->searching_cond), -1);
    RET_PT(pthread_mutex_unlock(&storage->cleaning_mtx), -1);

    RET_PT(pthread_mutex_unlock(&storage->mutex), -1);
    return 0;
}

int openFile(Storage_t *storage, int fd, char create, char* filename) {
    RET_ON(storage, NULL, -1);
    StorageNode_t *node = findFile(storage, filename);
    if(create == 1) {
        if(node != NULL) {
            return EEXIST; // Duplicate file;
        }
        RET_PT(pthread_mutex_lock(&storage->tail->mutex), -1);
        StorageNode_t *node = storage->tail;
        RET_ON((node->next = createNode(NULL, node, NULL, NULL, 0)), NULL, -1);
        storage->tail = node->next;
        RET_ON((node->filename = malloc(S_PATH_MAX)), NULL, -1);
        strncpy(node->filename, filename, S_PATH_MAX);
        node->size = 0;
        RET_ON((node->data = malloc(1)), NULL, -1);
        memcpy(node->data, "", 1);
        qPush(node->clients, fd);
        RET_PT(pthread_mutex_unlock(&node->mutex), -1);
        RET_PT(pthread_mutex_lock(&storage->mutex), -1);
        storage->length += 1;
        RET_PT(pthread_mutex_lock(&storage->cleaning_mtx), -1);
        while(storage->searching != 0) {
            RET_PT(pthread_cond_wait(&storage->cleaning_cond, &storage->cleaning_mtx), -1);
        }
        storage->cleaning = 1;
        RET_PT(pthread_mutex_unlock(&storage->cleaning_mtx), -1);
    
    
        RET_ON(removeOverflow(storage), -1, -1);
    
    
        RET_PT(pthread_mutex_lock(&storage->cleaning_mtx), -1);
        storage->cleaning = 0;
        RET_PT(pthread_cond_signal(&storage->searching_cond), -1);
        RET_PT(pthread_mutex_unlock(&storage->cleaning_mtx), -1);
        RET_PT(pthread_mutex_unlock(&storage->mutex), -1);
    } else {
        if(node == NULL) {
            return ENOENT;
        }
        int found;
        RET_ON((found = qFind(node->clients, fd)), -1, -1);
        // If already opened, do nothing and return success
        if(found == 0) {
            qPush(node->clients, fd);
        }
    }
    return 0;
}

int closeFile(Storage_t *storage, int fd, char* filename) {
    RET_ON(storage, NULL, -1);
    StorageNode_t *node = findFile(storage, filename);
    RET_ON(node, NULL, ENOENT);
    RET_ON(qRemove(node->clients, fd), -1, -1);
    return 0;
}

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

// Storage_t *pushFile(Storage_t *storage, char *filename, size_t size, void *data) {
//     RET_ON(storage, NULL, NULL);
//     RET_PT(pthread_mutex_lock(&storage->mutex), NULL);
//     storage->length += 1;
//     storage->size   += size;
//     RET_PT(pthread_mutex_unlock(&storage->mutex), NULL);
//     RET_PT(pthread_mutex_lock(&storage->tail->mutex), NULL);
//     StorageNode_t *node = storage->tail;
//     RET_ON((node->next = createNode(NULL, node, NULL, NULL, 0)), NULL, NULL);
//     storage->tail = node->next;
//     node->filename = filename;
//     node->size = size;
//     node->data = data;
//     RET_PT(pthread_mutex_unlock(&storage->tail->mutex), NULL);
//     return storage;
// }

// static StorageNode_t *findFile(Storage_t *storage, char *filename) {
//     RET_ON(storage, NULL, NULL);
//     StorageNode_t *currNode = storage->head;
//     while(currNode != NULL) {
//         if(strncmp(currNode->filename, filename, MAX_FILENAME) == 0)
//             return currNode;
//         currNode = currNode->next;
//     }
//     return NULL;
// }

// static storage_node *createEmptyFile(size_t size, char* filename) {
//     storage_node* node;
//     RET_ON((node = malloc(sizeof(storage_node))), NULL);
//     RET_PT(pthread_mutex_lock(&(tail->mutex_block_write)), NULL);
//     node->filename = filename;
//     node->size = size;
//     if(tail != NULL) {
//         tail->next = node;
//         node->prev = tail;
//         node->next = NULL;
//         tail = node;
//     } else {
//         RET_PT(pthread_mutex_lock(&(head->mutex_block_write)), NULL);
//         head = node;
//         tail = node;
//         node->next = NULL;
//         node->prev = NULL;
//         RET_PT(pthread_mutex_unlock(&(head->mutex_block_write)), NULL);
//     }
//     RET_PT(pthread_mutex_unlock(&(tail->mutex_block_write)), NULL);
//     return node;
// }

// int createFile(size_t size, char* filename, unsigned char* data) {
//     storage_node *node = createEmptyFile(size, filename);
// }

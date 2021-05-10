#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include "storage.h"
#include "utils.h"
#include "const.h"

StorageNode_t *createNode(StorageNode_t *next, StorageNode_t *prev, char *filename, unsigned char *data, size_t size) {
    StorageNode_t *node;
    RET_MALLOC(node, StorageNode_t, NULL);
    node->next = next;
    node->prev = prev;
    node->filename = filename;
    node->data = data;
    node->size = size;
    RET_PT(pthread_mutex_init(&node->mutex, NULL), NULL);
    return node;
}

Storage_t *initStorage() {
    Storage_t *storage;
    RET_MALLOC(storage, Storage_t, NULL);
    RET_ON((storage->head = createNode(NULL, NULL, NULL, NULL, 0)), NULL, NULL);
    storage->tail = storage->head;
    storage->lenght = 0;
    storage->size = 0;
    RET_PT(pthread_mutex_init(&storage->mutex, NULL), NULL);
    return storage;
}

OpenedFiles_t *initOpened() {
    OpenedFiles_t *openedFiles;
    RET_MALLOC(openedFiles, OpenedFiles_t, NULL);
    RET_MALLOC(openedFiles->head, OpenedFile_t, NULL);
    openedFiles->head->next = NULL;
    return openedFiles;
}

int pushOpened(OpenedFiles_t *openedFiles, StorageNode_t *node) {
    OpenedFile_t *old_head = openedFiles->head;
    OpenedFile_t *new_head;
    RET_MALLOC(new_head, OpenedFile_t, ERR_NO_MEMORY);
    new_head->next = old_head;
    openedFiles->head = new_head;
}

// int openFile(Storage_t *storage, OpenedFiles_t* openedFiles, char* filename, int flags) {
//     StorageNode_t *node;
//     if(IS_CREATE(flags)) {

//     } else {
//         RET_ON((node = findFile(storage, filename)), NULL, ERR_FILE_NOT_FOUND);
        
//     }
//     RET_NO((pushOpened(openedFiles, node)), ERR_OK);
//     if(IS_LOCK(flags)) {
//         RET_PT((pthread_mutex_lock(&node->mutex)), ERR_LOCK);
//     }
// }

Storage_t *pushFile(Storage_t *storage, char *filename, size_t size, unsigned char *data) {
    RET_ON(storage, NULL);
    RET_PT(pthread_mutex_lock(&storage->mutex), NULL);
    storage->lenght += 1;
    storage->size   += size;
    RET_PT(pthread_mutex_unlock(&storage->mutex), NULL);
    RET_PT(pthread_mutex_lock(&storage->tail->mutex), NULL);
    StorageNode_t *node = storage->tail;
    RET_ON((node->next = createNode(NULL, node, NULL, NULL, 0)), NULL);
    storage->tail = node->next;
    node->filename = filename;
    node->size = size;
    node->data = data;
    RET_PT(pthread_mutex_unlock(&storage->tail->mutex), NULL);
    return storage;
}

static StorageNode_t *findFile(Storage_t *storage, char *filename) {
    RET_ON(storage, NULL, NULL);
    StorageNode_t *currNode = storage->head;
    while(currNode != NULL) {
        if(strncmp(currNode->filename, filename, MAX_FILENAME) == 0)
            return currNode;
        currNode = currNode->next;
    }
    return NULL;
}

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

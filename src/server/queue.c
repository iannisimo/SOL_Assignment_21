#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "queue.h"
#include "utils.h"

/**
 * This is an implementation of a thread-safe linked-list of integers.
 * It's used 
 * * as a queue for the incoming requests to be sent from the dispatcher to the workers.
 * * to store the information about which clients have opened a file
 */

/**
 * @brief Initialize the Queue
 */
Queue_t *init_queue() {
    Queue_t *q;
    RET_ON((q = malloc(sizeof(Queue_t))), NULL, NULL);
    RET_PT((pthread_mutex_init(&q->lock, NULL)), NULL);
    RET_PT((pthread_cond_init(&q->cond, NULL)), NULL);
    q->len = 0;
    RET_ON((q->head = malloc(sizeof(QNode_t))), NULL, NULL);
    q->head->next = NULL;
    q->head->val = -1;
    q->tail = q->head;
    return q;
}

/**
 * @brief Add a value to the tail of the list
 * 
 * @return 0 on success, -1 on error
 */
int qPush(Queue_t *q, int val) {
    RET_ON(q, NULL, -1);
    QNode_t *node;
    RET_MALLOC(node, QNode_t, -1);
    node->val = val;
    node->next = NULL;
    RET_PT((pthread_mutex_lock(&q->lock)), -1);
    q->tail->next = node;
    q->tail = node;
    q->len += 1;
    RET_PT((pthread_cond_signal(&q->cond)), -1);
    RET_PT((pthread_mutex_unlock(&q->lock)), -1);
    return 0;
}

/**
 * @brief Sends a wake signal to every thread who's waiting for an element. 
 * It's used to wake the workers in the event that they should close themselves
 * 
 * @return 0 on success, -1 on error
 */
int qWakeAll(Queue_t* q) {
    q->len = 1;
    RET_PT(pthread_cond_broadcast(&q->cond), -1);
    return 0;
}

/**
 * @brief Get the first element in the list, blocks if it's empty
 * 
 * @return 0 on success, -1 on error 
 */
int qPop(Queue_t *q) {
    RET_ON(q, NULL, -1);
    RET_PT((pthread_mutex_lock(&q->lock)), -1);
    while(q->len == 0) {
        RET_PT((pthread_cond_wait(&q->cond, &q->lock)), -1);
    }
    int val = 0;
    if(q->head != q->tail) {
        RET_ON(q->head->next, NULL, -1);
        QNode_t *n  = q->head;
        val = q->head->next->val;
        q->head = q->head->next;
        free(n);
        q->len -= 1;
    }
    RET_PT((pthread_mutex_unlock(&q->lock)), -1);
    return val;
}

/**
 * @brief Searches for fd in the list
 * 
 * @return 1 if found, 0 if not found, -1 on error
 */
int qFind(Queue_t *q, int fd) {
    RET_ON(q, NULL, -1);
    RET_ON(q->head, NULL, -1);
    QNode_t *node = q->head->next;
    while(node != NULL) {
        if(node->val == fd) {
            return 1;
        }
        node = node->next;
    }
    return 0;
}

/**
 * @brief Remove fd from the list
 * 
 * @return 1 on success, 0 if not found, -1 on error
 */
int qRemove(Queue_t *q, int fd) {
    RET_ON(q, NULL, -1);
    RET_ON(q->head, NULL, -1);
    QNode_t *prev = q->head;
    QNode_t *node = q->head->next;
    while(node != NULL) {
        if(node->val == fd) {
            RET_PT(pthread_mutex_lock(&q->lock), -1);
            prev->next = node->next;
            if(node == q->tail) {
                q->tail = q->head;
                if(q->tail->next != NULL) {
                    while(q->tail->next->next != NULL) {
                        q->tail = q->tail->next;
                    }
                }
            }
            free(node);
            RET_PT(pthread_mutex_unlock(&q->lock), -1);
            return 1;
        }
        prev = node;
        node = node->next;
    }
    return 0;
}

/**
 * @brief Destroy a Queue_t element
 * 
 * @return 0 on success, -1 on error 
 */
int qFree(Queue_t *q) {
    QNode_t *node = q->head;
    QNode_t *next = NULL;
    while(node != NULL) {
        next = node->next;
        free(node);
        node = next;
    }
    RET_PT(pthread_mutex_destroy(&q->lock), -1);
    RET_PT(pthread_cond_destroy(&q->cond), -1);
    free(q);
    return 0;
}
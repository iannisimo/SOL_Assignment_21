#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "queue.h"
#include "utils.h"

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

int qWakeAll(Queue_t* q) {
    q->len = 1;
    RET_PT(pthread_cond_broadcast(&q->cond), -1);
    return 0;
}

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
 * @returns
 *  -1: fatal error
 *   0: not found
 *   1: found 
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
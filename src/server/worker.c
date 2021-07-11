#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "worker.h"
#include "utils.h"
#include "const.h"
#include "queue.h"

int execute(int fd, Storage_t *storage) {
    static char buf[BUF_MAX];
    char op;
    size_t s;
    s = read(fd, &op, 1);
    RET_ON(s, -1, -1);
    RET_ON(s, 0, -2);
    switch (op) {
        case 'c': { // Create and open
            RET_ON(read(fd, buf, BUF_MAX - 1), -1, -1);
            int status;
            RET_ON((status = openFile(storage, fd, 1, buf)), -1, -1);
            size_t len = snprintf(buf, 4, "%d", status);
            RET_ON(write(fd, buf, len + 1), -1, -1);
            break;
        }
        case 'o': { // Open file
            RET_ON(read(fd, buf, BUF_MAX - 1), -1, -1);
            int status;
            RET_ON((status = openFile(storage, fd, 0, buf)), -1, -1);
            size_t len = snprintf(buf, 4, "%d", status);
            RET_ON(write(fd, buf, len + 1), -1, -1);
            break;
        }
        case 'C': { // Close file
            RET_ON(read(fd, buf, BUF_MAX - 1), -1, -1);
            int status;
            RET_ON((status = closeFile(storage, fd, buf)), -1, -1);
            size_t len = snprintf(buf, 4, "%d", status);
            RET_ON(write(fd, buf, len + 1), -1, -1);
            break;
        }
        case 'r': { // Read file
            RET_ON(read(fd, buf, BUF_MAX - 1), -1, -1);
            int status;
            size_t size = 0;
            void *data = NULL;
            RET_ON((status = readFile(storage, fd, buf, &size, &data)), -1, -1);
            size_t len = snprintf(buf, BUF_MAX, "%d %ld", status, size);
            RET_ON(write(fd, buf, len + 1), -1, -1);
            RET_ON(write(fd, data, size), -1, -1);
            break;
        }
        case 'R': { // Read files
            RET_ON(read(fd, buf, BUF_MAX - 1), -1, -1);
            size_t N;
            RET_ON(getSz(buf, &N), EINVAL, -1);
            int status;
            size_t size = 0;
            void *data;
            char *pathname;
            if (N == 0) N = storage->length;
            for(int i = 0; i < N; i++) {
                RET_ON((status = readIthFile(storage, i, &size, &data, &pathname)), -1, -1);
                if(status == ENODATA) break;
                size_t len = snprintf(buf, BUF_MAX, "%d %ld %s", status, size, pathname);
                RET_ON(write(fd, buf, len + 1), -1, -1);
                RET_ON(write(fd, data, size), -1, -1);
            }
            size_t len = snprintf(buf, BUF_MAX, "%d", ENODATA);
            RET_ON(write(fd, buf, len + 1), -1, -1);
            break;
        }
        case 'a': { // Append to file
            RET_ON(readUntil(fd, buf, BUF_MAX - 1, ' '), -1, -1);
            size_t size;
            if((getSz(buf, &size)) != 0) return -1;
            RET_ON(readUntil(fd, buf, BUF_MAX - 1, '\0'), -1, -1);
            int status = 0;
            if(size > 0) {
                void* tmpData;
                RET_ON((tmpData = malloc(size)), NULL, -1);
                ssize_t tmpDataSize = 0;
                while((size - tmpDataSize) != 0) {
                    void* ptr = (void*)(((char*) tmpData) + tmpDataSize);
                    RET_ON((tmpDataSize += read(fd, ptr, size - tmpDataSize)), -1, -1);
                }
                RET_ON((status = appendToFile(storage, fd, buf, size, tmpData)), -1, -1);
                free(tmpData);
            }
            size_t len = snprintf(buf, 4, "%d", status);
            RET_ON(write(fd, buf, len + 1), -1, -1);
            break;
        }
    }
    return 0;
}

void *runWorker(void *args) {
    
    printf("Started a worker thread\n");
    
    WArgs_t *wa = (WArgs_t*) args;
    while(1) {
        int fd;
        EXT_ON_PT((fd = qPop(wa->queue)), -1);
        if(wa->cc->closeAll) break;
        int status = execute(fd, wa->storage);
        if(status == -2) { // Client-side connection close
            close(fd);
            closeAllFiles(wa->storage, fd);
            int tmp = -1;
            RET_ON(write(wa->fd_pipe, &tmp, sizeof(int)), -1, NULL);
            continue;
        }
        if(status == -1) { // Error during execution
            // notify_aborted();
        }
        RET_ON(write(wa->fd_pipe, &fd, sizeof(int)), -1, NULL);
    }
    return NULL;
}
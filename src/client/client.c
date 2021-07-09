#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include "connectionAPI.h"
#include "argparse.h"
#include <dirent.h> 
#include "utils.h"

void printAll(char **argv) {
    RET_ON(argv, NULL, ;)
    printf("Arguments\n");
    int run = 0;
    while(run + 1) {
        if(argv[run] == NULL || argv[run][0] == '\0') return;
        printf("\t%s\n", argv[run]);
        run++;
    }
}

int readTask(char *filename, char *absRcv) {
    int status = 0;
    char absPath[PATH_MAX];
    void *data;
    size_t size;
    RET_ON(getAbsPath(filename, absPath), -1, -1);
    RET_NO(openFile(absPath, 0), 0);
    RET_ON((status = readFile(absPath, &data, &size)), -1, -1);
    RET_NO(closeFile(absPath), 0);
    if(absRcv != NULL && status == 0) {
        RET_ON(writeToFolder(absPath, absRcv, data, size), -1, -1);
    }
    free(data);
    return status;
}

int writeTask(char *filepath) {
    void *data;
    size_t size;
    int status;
    char absPath[PATH_MAX];
    RET_NO(getFileBytes(filepath, absPath, &data, &size), 0);
    RET_ON((status = openFile(absPath, O_CREATE)), -1, -1);
    if(status == EEXIST) {
        RET_ON((status = openFile(absPath, 0)), -1, -1);
    }
    RET_NO(status, 0);
    if(size > 0) RET_ON((status = appendToFile(absPath, data, size, NULL)), -1, -1);
    RET_NO(closeFile(absPath), 0);
    return status;
}

int treeWalkWrite(char *dirpath, int dirlen, int *N) {
    if(dirlen >= PATH_MAX-1) return -1;
    struct dirent *ent;
    DIR *d = opendir(dirpath);
    struct stat st;
    if(d == NULL) return -1;
    dirpath[dirlen++] = '/';
    while ((ent = readdir(d)) != NULL) {
        if(*N == 0) return 0;
        dirpath[dirlen] = '\0';
        int entlen = strlen(ent->d_name);
        if(ent->d_name[entlen-1] == '.') continue;
        memcpy(dirpath + dirlen, ent->d_name, entlen);
        dirpath[dirlen + entlen] = '\0';
        RET_ON(stat(dirpath, &st), -1, -1);
        if(S_ISDIR(st.st_mode)) {
            RET_ON(treeWalkWrite(dirpath, dirlen + entlen, N), -1, -1);
        } else {
            int status;
            RET_ON((status = writeTask(dirpath)), -1, -1);
            if(status != 0) {
                debugf("Error sending %s to the server\n", dirpath);
            }
            if(*N != -1) *N -= 1;
        }
    }
    closedir(d);
    dirpath[--dirlen] = '\0';
    return 0;
}

int execute(Task_t task) {
    int status;
    switch (task.type) {
        case 'r': {
            debugf("\nSending read requests for %d file%s\n", task.n, task.n == 1 ? "" : "s");
            int status = 0;
            char absRcv[PATH_MAX];
            if(task.receive_folder != NULL) {
                RET_ON(getAbsPath(task.receive_folder, absRcv), -1, -1);
            }
            for(int i = 0; i < task.n; i++) {
                debugf("Requesting file %s\n", task.send_args[i]);
                RET_ON((status = readTask(task.send_args[i], task.receive_folder == NULL ? NULL : absRcv)), -1, -1);
                if(status != 0) {
                    debugf("Failed with error: %s\n", strerror(status));
                }
            }
            break;
        }
        case 'R': {
            if(task.n != 0)
                debugf("\nRequesting %d random file%s from the server\n", task.n, task.n == 1 ? "" : "s");
            else
                debugf("\nRequesting all files from the server\n");
            RET_ON((status = readNFiles(task.n, task.receive_folder)), -1, -1);
            if(status != 0) {
                debugf("Failed with error: %s\n", strerror(status));
            }
            break;
        }
        case 'W': {
            debugf("\nSending write requests for %d file%s\n", task.n, task.n == 1 ? "" : "s");
            for(int i = 0; i < task.n; i++) {

                debugf("Writing file %s\n", task.send_args[i]);
                RET_ON((status = writeTask(task.send_args[i])), -1, -1);
                if(status != 0) {
                    debugf("Failed with error: %s\n", strerror(status));
                }
            }
            break;
        }
        case 't': {
            debugf("\nWaiting %d milliseconds\n", task.n);
            RET_ON(millisleep(task.n), -1, -1);
            break;
        }
        case 'w': {
            debugf("\nSending the contents of %s to the server\n", task.send_args[0]);
            char absPath[PATH_MAX];
            int N = task.n == 0 ? -1 : task.n;
            RET_ON(getAbsPath(task.send_args[0], absPath), -1, -1);
            int abslen = strnlen(absPath, PATH_MAX);
            RET_ON(treeWalkWrite(absPath, abslen, &N), -1, -1);
            break;
        }
    }
    if(status == 0) {
        debugf("Operation completed\n");
    }
    return status;
}

int main(int argc, char **argv) {
    SingleArgs_t sargs = parseSingleArgs(argc, argv);
    if(IS_HELP(sargs.flags)) {
        printf(HELP_MSG);
        exit(0);
    }
    setDebug(IS_PRINT(sargs.flags));
    if(sargs.sockname[0] == '\0') {
        printf("Please use the option -f to select a socket\n");
        return -1;
    }
    openConnection(sargs.sockname, 1500, (const struct timespec) {.tv_nsec = 5000000000});
    Task_t nextTask;
    while((nextTask = getNext(argc, argv)).type != 0) {
        // printf("NTask: %c\n", nextTask.type);
        int status = execute(nextTask);
        if(status == -1) debugf("Something went very wrong, unpredictable\n");
        if(status > 0) printf("Operation not successful: %s\n", strerror(status));
    }
    closeConnection(sargs.sockname);
    return 0;
}

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

/**
 * @brief Request filename fro the server
 * 
 * @param absRcv Where to save the file, NULL to print to stdout
 * @return The status code from the server, -1 on error
 */
int readTask(char *filename, char *absRcv) {
    int status = 0;
    char absPath[PATH_MAX];
    void *data;
    size_t size;
    RET_ON(getAbsPath(filename, absPath), -1, -1);
    RET_ERRNO(openFile(absPath, 0));
    status = readFile(absPath, &data, &size);
    if(closeFile(absPath) != 0) {
        free(data);
        int _err = errno;
        return _err;
    }
    if(status == 0) {
        if(absRcv != NULL) {
            if(writeToFolder(absPath, absRcv, data, size) == -1) {
                free(data);
                return -1;
            }
        } else {
            debugf("\nfilename:\n%s\ndata:\n", filename);
            print_void(data, size);
        }
    }
    free(data);
    return status;
}

/**
 * @brief Send the file in filepath to the server
 * 
 * @return The status code from the server, -1 on error
 */
int writeTask(char *filepath) {
    void *data;
    size_t size;
    int status;
    char absPath[PATH_MAX];
    RET_NO(getFileBytes(filepath, absPath, &data, &size), 0);
    errno = 0;
    if((status = openFile(absPath, O_CREATE)) == -1 && errno == EEXIST) {
        status = openFile(absPath, 0);
    }
    if(status != 0) {
        free(data);
        int _err = errno;
        return _err != 0 ? _err : -1;
    }
    int appendStatus = 0;
    if(size > 0) {
        status = appendToFile(absPath, data, size, NULL);
        appendStatus = errno;
    }
    free(data);
    RET_ERRNO(closeFile(absPath));
    if(status == -1) {
        return appendStatus;
    }
    return 0;
}

/**
 * @brief Recursive funcion that sends all the files in dirpath to the server and calls itself on the directories
 * 
 * @return The status code from the server, -1 on error
 */
int treeWalkWrite(char *dirpath, int dirlen, int *N) {
    int ret = 0;
    if(dirlen >= PATH_MAX-1) return -1;
    struct dirent *ent;
    DIR *d = opendir(dirpath);
    struct stat st;
    if(d == NULL) return -1;
    dirpath[dirlen++] = '/';
    while ((ent = readdir(d)) != NULL) {
        if(*N == 0) break;
        dirpath[dirlen] = '\0';
        int entlen = strlen(ent->d_name);
        if(ent->d_name[entlen-1] == '.') continue;
        memcpy(dirpath + dirlen, ent->d_name, entlen);
        dirpath[dirlen + entlen] = '\0';
        if(stat(dirpath, &st) == -1) {
            ret = -1;
            break;
        }
        if(S_ISDIR(st.st_mode)) {
            if(treeWalkWrite(dirpath, dirlen + entlen, N) == -1) {
                ret = -1;
                break;
            }
        } else {
            int status;
            if((status = writeTask(dirpath)) == -1) {
                ret = -1;
                break;
            }
            if(status == EFBIG) {
                debugf("Warning: the server has refused <%s> because is too big\n", dirpath);
            } else if(status != 0){
                debugf("Warning: could not write <%s> to the server because: %s", dirpath, strerror(status));
            }
            if(*N != -1) *N -= 1;
        }
    }
    closedir(d);
    dirpath[--dirlen] = '\0';
    return ret;
}

/**
 * @brief Execute a given task
 * 
 * @return The status code from the server, -1 on error
 */
int execute(Task_t task) {
    int status = 0;
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
                    debugf("Warning: could not complete request: %s\n", strerror(errno));
                }
            }
            debugf("Operation completed\n");
            status = -2;
            break;
        }
        case 'R': {
            if(task.n != 0)
                debugf("\nRequesting %d random file%s from the server\n", task.n, task.n == 1 ? "" : "s");
            else
                debugf("\nRequesting all files from the server\n");
            status = readNFiles(task.n, task.receive_folder);
            if(status == -1) {
                debugf("Warning: could not complete request: %s\n", strerror(errno));
            } else {
                debugf("Operation completed, got %d file%s", status, status == 1 ? "" : "s");
                return 0;
            }
            status = -2;
            break;
        }
        case 'W': {
            debugf("\nSending write requests for %d file%s\n", task.n, task.n == 1 ? "" : "s");
            for(int i = 0; i < task.n; i++) {

                RET_ON((status = writeTask(task.send_args[i])), -1, -1);
                if(status == EFBIG) {
                    debugf("Warning: the server has refused <%s> because is too big\n", task.send_args[i]);
                } else if(status != 0){
                    debugf("Warning: could not write <%s> to the server because: %s", task.send_args[i], strerror(status));
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
    // Read the configuration parameters
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
    // Connect and perform every task one by one
    openConnection(sargs.sockname, 1500, (const struct timespec) {.tv_nsec = 5000000000});
    Task_t nextTask;
    while((nextTask = getNext(argc, argv)).type != 0) {
        int status = execute(nextTask);
        freeTask(nextTask);
        if(status == -1) {
            debugf("Something went wrong, exiting...\n");
            break;
        }
    }
    closeConnection(sargs.sockname);
    return 0;
}

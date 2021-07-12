#ifndef __argparse_h
#define __argparse_h

#include "const.h"

/**
 * @brief This structure keeps the configuration parsed from the arguments  
 */
typedef struct _single_args {
    char sockname[UNIX_PATH_MAX];
    char  flags;
} SingleArgs_t;

/**
 * @brief This records all the necessary information about a task
 */
typedef struct _task {
    int n;
    char *receive_folder;
    char **send_args;
    char type;
} Task_t;

SingleArgs_t parseSingleArgs(int argc, char** argv);
Task_t getNext(int argc, char** argv);
void freeTask(Task_t t);

#endif
#ifndef __argparse_h
#define __argparse_h

#include "const.h"

typedef struct _single_args {
    char sockname[UNIX_PATH_MAX];
    char  flags;
} SingleArgs_t;

typedef struct _task {
    int n;
    char *receive_folder;
    char **send_args;
    char type;
} Task_t;

SingleArgs_t parseSingleArgs(int argc, char** argv);
Task_t getNext(int argc, char** argv);

#endif
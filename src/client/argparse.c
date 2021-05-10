#include <getopt.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "argparse.h"
#include "utils.h"

SingleArgs_t parseSingleArgs(int argc, char** argv) {
    SingleArgs_t args;
    args.sockname[0] = '\0';
    opterr = 0;
    int rstIndex = 1;
    int opt;
    while((opt = getopt(argc, argv, "f:hpr:R:w:W:t:c:d:")) != -1) {
        switch(opt) {
            case 'f': {
                if(optarg != NULL) {
                    strncpy(args.sockname, optarg, UNIX_PATH_MAX - 1);
                    rstIndex++;
                }
                break;
            }
            case 'h': {
                args.flags |= O_HELP;
                rstIndex++;
                break;
            }
            case 'p': {
                args.flags |= O_PRINT;
                rstIndex++;
                break;
            }
        }
    }
    optind = rstIndex;
    return args;
}

char *getReadDir(int argc, char** argv) {
    int idx = optind;
    RET_ON(argv[idx], NULL, NULL);
    char *dirArg;
    if(strncmp(argv[idx], "-d", 2) == 0) {
        dirArg = argv[idx + 1];
    } else if(strncmp(argv[idx - 1], "-d", 2) == 0) {
        dirArg = argv[idx];
    } else {
        return NULL;
    }
    RET_ON(dirArg, NULL, NULL);
    char *arg;
    int argLen = strnlen(dirArg, PATH_MAX) + 1;
    RET_ON((arg = malloc(argLen * sizeof(char))), NULL, NULL);
    strncpy(arg, dirArg, argLen);
    return arg;
}

Task_t getNext(int argc, char** argv) {
    Task_t task = {
        .receive_folder = NULL,
        .send_args = NULL,
        .n = 0
    };
    int opt;
    do {
        opt = getopt(argc, argv, "f:hpr:R:w:W:t:c:d:");
        printf("opt:\t%d\n", opt);
    } while(opt == 'f' || opt == 'h' || opt == 'p' || opt == 'd');
    task.type = (char) opt;
    RET_ON(task.type, -1, task);
    // char optcpy[PATH_MAX];
    // strncpy(optcpy, optarg, PATH_MAX - 1);
    task.send_args = tokenize(optarg, ",", task.type, &task.n);
    if(task.type == 'r' || task.type == 'R') {
        task.receive_folder = getReadDir(argc, argv);
    }
    return task;
}
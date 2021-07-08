#include <getopt.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "argparse.h"
#include "utils.h"

SingleArgs_t parseSingleArgs(int argc, char** argv) {
    SingleArgs_t args;
    args.flags = 0;
    args.sockname[0] = '\0';
    opterr = 0;
    int opt;
    while((opt = getopt(argc, argv, "f:hpr:R:w:W:t:c:d:")) != -1) {
        switch(opt) {
            case 'f': {
                if(optarg != NULL) {
                    strncpy(args.sockname, optarg, UNIX_PATH_MAX - 1);
                }
                break;
            }
            case 'h': {
                args.flags |= O_HELP;
                break;
            }
            case 'p': {
                args.flags |= O_PRINT;
                break;
            }
            default: {
                if(optarg != NULL) {
                    if(optarg[0] == '-') {
                        optind--;
                    }
                }
            }
        }
    }
    optind = 1;
    return args;
}

char *getRcvDir(int argc, char** argv) {
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
    RET_ON(dirArg[0], '-', NULL);
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
        .n = 0,
        .type = 0
    };
    int opt;
    do {
        opt = getopt(argc, argv, "-f:hpr:Rw:W:t:c:d:");
        RET_ON(opt, -1, task);
        if(opt == 'R' && optind < argc) optarg = argv[optind++];
        if(optarg != NULL) if(optarg[0] == '-') optind--;
    } while(opt == 'f' || opt == 'h' || opt == 'p' || opt == 'd');
    RET_ON(opt, -1, task);
    task.type = (char) opt;
    tokenize(optarg, ",", task.type, &task.n, &task.send_args);
    if(task.type == 'r' || task.type == 'R') {
        task.receive_folder = getRcvDir(argc, argv);
    }
    return task;
}
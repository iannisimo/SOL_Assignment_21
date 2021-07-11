#include <getopt.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "argparse.h"
#include "utils.h"

int tokenize(char *string, char* separator, char type, int *n, char ***args) {
    char **strings = NULL;
    RET_ON(string, NULL, -1);
    RET_ON(string[0], '-', EINVAL);
    if(type == 'W' || type == 'r' || type == 'c') {
        int count = strncnt(string, separator[0], PATH_MAX) + 1;
        *n = count;
        RET_ON((strings = malloc(count * sizeof(char*))), NULL, ENOMEM);
        char *token = strtok(string, separator);
        for(int i = 0; i < count; i++) {
            if(token == NULL) break;
            int token_len = strnlen(token, PATH_MAX) + 1;
            RET_ON((strings[i] = malloc(token_len * sizeof(char))), NULL, ENOMEM);
            strncpy(strings[i], token, token_len);
            token = strtok(NULL, separator);
        }
    } else if(type == 'R') {
        if(strncmp(string, "n=", 2) == 0) {
            RET_ON((getNumber(string + 2, n)), -1, EDOM);
        } else {
            *n = 0;
        }
    } else if(type == 'w') {
        char *dir = strtok(string, separator);
        RET_ON(dir, NULL, -1);
        RET_ON((strings = malloc(sizeof(char*))), NULL, ENOMEM);
        int dirLen = strnlen(dir, PATH_MAX) + 1;
        RET_ON((strings[0] = malloc(dirLen * sizeof(char))), NULL, ENOMEM);
        strncpy(strings[0], dir, dirLen);
        char *nStr = strtok(NULL, separator);
        if(nStr != NULL) {
            if(strncmp(nStr, "n=", 2) == 0) {
                RET_ON((getNumber(nStr + 2, n)), -1, EDOM);
            } else {
                *n = 0;
            }
        }
    } else if(type == 't') {
        RET_ON((getNumber(string, n)), 0, EDOM);
    }
    *args = strings;
    return 0;
}

/**
 * @brief Run through all the arguments searching for the ones that should appear only once 
 * 
 * @return SingleArgs_t 
 */
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

/**
 * @brief Checks if the task just parsed is followed by a -d indicating that the user wants to save the files in a folder
 * 
 * @return The desired pathname, NULL if not present 
 */
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

/**
 * @brief Get the Next task to send
 * 
 * @return Task_t 
 */
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
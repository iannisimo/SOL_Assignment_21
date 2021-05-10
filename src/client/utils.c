#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"


char getNumber(char* str, int* val) {
    char* endptr = NULL;
    errno = 0;
    *val = (int) strtol(str, &endptr, 10);
    if(errno != 0 || endptr == NULL || *endptr != '\0') {
        return 0;
    }
    return 1;
}

static int strncnt(char *str, char elem, int max) {
    RET_ON(str, NULL, -1);
    int count = 0;
    int i = 0;
    char c = str[i];
    while(c != '\0' && i <= max) {
        if(c == elem) count++;
        c = str[++i];
    }
    return count;
}

char **tokenize(char *string, char* separator, char type, int *n) {
    char **strings = NULL;
    RET_ON(string, NULL, NULL);
    RET_ON(string[0], '-', NULL);
    if(type == 'W' || type == 'r' || type == 'c') {
        int count = strncnt(string, separator[0], PATH_MAX) + 1;
        *n = count;
        RET_ON((strings = malloc(count * sizeof(char*))), NULL, NULL);
        char *token = strtok(string, separator);
        for(int i = 0; i < count; i++) {
            if(token == NULL) break;
            int token_len = strnlen(token, PATH_MAX) + 1;
            RET_ON((strings[i] = malloc(token_len * sizeof(char))), NULL, NULL);
            strncpy(strings[i], token, token_len);
            token = strtok(NULL, separator);
        }
    } else if(type == 'R') {
        if(strncmp(string, "n=", 2) == 0) {
            RET_ON((getNumber(string + 2, n)), 0, NULL);
        } else {
            *n = 0;
        }
    } else if(type == 'w') {
        char *dir = strtok(string, separator);
        RET_ON(dir, NULL, NULL);
        RET_ON((strings = malloc(sizeof(char*))), NULL, NULL);
        int dirLen = strnlen(dir, PATH_MAX) + 1;
        RET_ON((strings[0] = malloc(dirLen * sizeof(char))), NULL, NULL);
        strncpy(strings[0], dir, dirLen);
        char *nStr = strtok(NULL, separator);
        if(nStr == NULL)
            return strings;
        else {
            if(strncmp(nStr, "n=", 2) == 0) {
                RET_ON((getNumber(nStr + 2, n)), 0, NULL);
            } else {
                *n = 0;
            }
        }
    } else if(type == 't') {
        RET_ON((getNumber(string, n)), 0, NULL);
    }
    return strings;
}

// if(i == count && (strncmp(token, "n=", 2) == 0)) {
//                 RET_ON((getNumber(token + 2, n)), 0, NULL);
//                 RET_ON((realloc(strings, count - 1)), NULL, NULL);
//             }

int millisleep(int millis) {
    struct timespec nssleep;
    nssleep.tv_sec = ((long) millis) / 1000l;
    nssleep.tv_nsec = (millis % 1000) * 1000000;
    RET_ON((nanosleep(&nssleep, NULL)), -1, -1);
    return 0;
}
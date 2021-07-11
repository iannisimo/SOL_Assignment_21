#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include "config.h"
#include "utils.h"

#define MAX_LINE 128

static int readLine(FILE* stream, char *line) {
    RET_ON((fgets(line, MAX_LINE, stream)), NULL, EOF);
    char *end;
    RET_ON((end = strchr(line, '\0')), NULL, ERR_OVERFLOW);
    if(*(end - 1) == '\n') *(end - 1) = '\0';
    return ERR_OK;
}

static int skipWS(char **string) {
    RET_ON(string, NULL, EFAULT);
    RET_ON(*string, NULL, EFAULT);
    char *ptr = *string;
    int max = UNIX_PATH_MAX;
    while(isblank(*ptr) && (--max > 0)) ptr++;
    if(max <= 0) return EOVERFLOW;
    *string = ptr;
    return 0;
}

static int parseSize(char* val, long long *size) {
    char *end;
    RET_ON((end = strchr(val, '\0')), NULL, EOVERFLOW);
    char unit = *(end - 1);
    long long sizeMul = 1ll;
    switch(unit) {
        case 'B': {
            sizeMul = 1ll;
            *(end - 1) = '\0';
            break;
        }
        case 'K': {
            sizeMul = 1000ll;
            *(end - 1) = '\0';
            break;
        }
        case 'M': {
            sizeMul = 1000000ll;
            *(end - 1) = '\0';
            break;
        }
        case 'G': {
            sizeMul = 1000000000ll;
            *(end - 1) = '\0';
            break;
        }
    }
    RET_ON(getLLong(val, size), EINVAL, EINVAL);
    *size *= sizeMul;
    return 0;
}

static int checkConfig(Config_t config) {
    if(config.max_files <= 0) return EINVAL;
    if(config.max_size <= 0) return EINVAL;
    if(config.n_workers <= 0) return EINVAL;
    if(config.sockname[0] == '\0') return EINVAL;
    return 0;
}

int parseLine(char *line, Config_t *config) {
    char* colon = strchr(line, ':');
    RET_ON(colon, NULL, ERR_INVALID);
    colon[0] = '\0';
    char *val = colon + 1;
    RET_NO((skipWS(&val)), 0);
           if(strcmp(line, "N_WORKERS") == 0) {
        RET_ON((getInt(val, &config->n_workers)), EINVAL, EINVAL);
    } else if(strcmp(line, "MAX_SIZE") == 0) {
        RET_NO((parseSize(val, &config->max_size)), 0);
    } else if(strcmp(line, "MAX_FILES") == 0) {
        RET_ON((getInt(val, &config->max_files)), EINVAL, EINVAL);
    } else if(strcmp(line, "SOCKET_NAME") == 0) {
        strcpy(config->sockname, val);
    }
    return 0;
}

int parseConfig(char *path, Config_t *config) {
    FILE *cFile;
    RET_ON((cFile = fopen(path, "r")), NULL, errno);
    char currLine[MAX_LINE] = "";
    int status = 0;
    do {
        RET_ON((status = readLine(cFile, currLine)), ERR_OVERFLOW, ERR_OVERFLOW);
        if(status == EOF) break;
        CNT_NO((parseLine(currLine, config)), 0);
    } while(status != EOF);
    RET_ON((fclose(cFile)), EOF, EBADFD);
    RET_NO(checkConfig(*config), 0);
    return 0;
}
#include "const.h"

#ifndef __config_h
#define __config_h

typedef struct _config {
    int n_workers;
    int max_files;
    long long max_size;
    char sockname[UNIX_PATH_MAX];
} Config_t;

static const Config_t EMPTY_CONFIG = {
    .n_workers = -1,
    .max_files = -1,
    .max_size = -1,
    .sockname  = ""
};

int parseConfig(char *path, Config_t *config);

#endif
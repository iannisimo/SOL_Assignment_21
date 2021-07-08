#ifndef __const_h
#define __const_h

#include <limits.h>

#define MAX_FILENAME 1024
#define O_CREATE     0b01
#define O_LOCK       0b10

#define UNIX_PATH_MAX 108
#define BUF_MAX       (PATH_MAX + 128)

#define ERR_OK             0
#define ERR_FILE_NOT_FOUND 2
#define ERR_BAD_FD         9
#define ERR_NO_MEMORY      12
#define ERR_INVALID        22
#define ERR_LOCK           37
#define ERR_OVERFLOW       75

#endif
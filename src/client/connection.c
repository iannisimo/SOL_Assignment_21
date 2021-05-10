#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include "connection.h"
#include "utils.h"
#include "const.h"

#include <stdio.h>

static int sfd;
static char currSockName[UNIX_PATH_MAX];

int openConnection(const char *sockname, int msec, const struct timespec abstime) {
    RET_ON((sfd = socket(AF_UNIX, SOCK_STREAM, 0)), -1, -1);
    struct sockaddr_un sa;
    strncpy(sa.sun_path, sockname, UNIX_PATH_MAX - 1);
    sa.sun_family = AF_UNIX; 
    int maxRetry = (int) (abstime.tv_nsec / (((long) msec) * 1000000));
    for(int i = 0; i < maxRetry; i++) {
        if(connect(sfd, (struct sockaddr*) &sa, sizeof(sa)) == -1) {
            if (errno == ENOENT) {
                RET_ON((millisleep(msec)), -1, -1);
            } else {
                return -1;
            }
        } else {
            break;
        }
    }
    strncpy(currSockName, sockname, UNIX_PATH_MAX - 1);
    return 0;
}

int closeConnection(const char *sockname) {
    if(strncmp(currSockName, sockname, UNIX_PATH_MAX) == 0) {
        currSockName[0] = '\0';
        close(sfd);
    } else {
        return -1;
    }
    return 0;
}
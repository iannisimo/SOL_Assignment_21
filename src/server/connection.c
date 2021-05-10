#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include "connection.h"
#include "utils.h"
#include "const.h"

static int sfd;

int initSocket(char *sockname) {
    struct sockaddr_un sa;
    strncpy(sa.sun_path, sockname, UNIX_PATH_MAX - 1);
    sa.sun_family = AF_UNIX;
    RET_ON((sfd = socket(AF_UNIX, SOCK_STREAM, 0)), -1, -1);
    RET_ON((bind(sfd, (struct sockaddr *) &sa, sizeof(sa))), -1, -1);
    RET_ON((listen(sfd, SOMAXCONN)), -1, -1);
    return 0;
}

int waitConnection() {
    return accept(sfd, NULL, 0);
}
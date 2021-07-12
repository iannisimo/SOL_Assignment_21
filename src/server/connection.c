#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "connection.h"
#include "utils.h"
#include "const.h"

void *runMaster(void *args) {

    printf("Dispatcher thread started\n");

    MArgs_t *ma = (MArgs_t*) args;

    int sfd;
    fd_set set;
    fd_set rdset;
    int fd_num = 0;

    // Bind and listen to the socket

    struct sockaddr_un sa;
    strncpy(sa.sun_path, ma->sockname, UNIX_PATH_MAX-1);
    sa.sun_family = AF_UNIX;
    RET_ON((sfd = socket(AF_UNIX, SOCK_STREAM, 0)), -1, NULL);
    RET_ON((bind(sfd, (struct sockaddr *) &sa, sizeof(sa))), -1, NULL);
    RET_ON((listen(sfd, SOMAXCONN)), -1, NULL);

    // configuration of select()'s sets

    if(sfd > fd_num) fd_num = sfd;
    if(ma->fd_pipe > fd_num) fd_num = ma->fd_pipe;
    if(ma->exit_pipe > fd_num) fd_num = ma->exit_pipe;
    FD_ZERO(&set);
    FD_SET(sfd, &set);
    FD_SET(ma->fd_pipe, &set);
    FD_SET(ma->exit_pipe, &set);

    size_t connected_clients = 0;

    // Exits when it's asked to close everything immediately or if no client is connected and it cannot accept any more connections

    while(!ma->cc->closeAll && (ma->cc->acceptConns || connected_clients > 0)) {    
        rdset = set;
        if(select(fd_num+1, &rdset, NULL, NULL, NULL) == -1) {
            perror("Select error");
            continue;
        }
        for(int fd = 0; fd <= fd_num; fd++) {
            if(FD_ISSET(fd, &rdset)) {
                if(fd == ma->fd_pipe) {
                    int new_fd;
                    RET_ON((read(ma->fd_pipe, &new_fd, sizeof(int))), -1, NULL);
                    if(new_fd == -1) {
                        connected_clients -= 1;
                    } else {
                        FD_SET(new_fd, &set);
                        if(new_fd > fd_num) fd_num = new_fd;
                    }
                } else if(fd == ma->exit_pipe) {
                    FD_CLR(fd, &set);
                } else if(fd == sfd && ma->cc->acceptConns) {
                    int new_fd = accept(sfd, NULL, 0);
                    FD_SET(new_fd, &set);
                    connected_clients += 1;
                    if(new_fd > fd_num) fd_num = new_fd;
                } else {
                    FD_CLR(fd, &set);
                    RET_ON((qPush(ma->queue, fd)), -1, NULL); // Change this
                }
            }
        }
    }
    RET_ON(unlink(ma->sockname), -1, NULL);
    return NULL;
}


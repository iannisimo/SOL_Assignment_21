#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/un.h>
#include <string.h>

#include "storage.h"
#include "config.h"
#include "utils.h"
#include "worker.h"
#include "connection.h"
#include "queue.h"
#include "storage.h"

int sig_loop(sigset_t sig_mask, int exit_pipe, Queue_t *queue, closeConditions_t *cc) {
    int sig;
    while(1) {
        RET_PT(sigwait(&sig_mask, &sig), -1);
        printf("Received signal %s(%d)\n", strsignal(sig), sig);
        if(sig == SIGINT || sig == SIGQUIT) {
            cc->closeAll = 1;
            RET_ON(close(exit_pipe), -1, -1);
            RET_ON(qWakeAll(queue), -1, -1);
            return 0;
        } else if(sig == SIGHUP) {
            cc->acceptConns = 0;
            RET_ON(close(exit_pipe), -1, -1);
            return 0;
        }
    }
}

int main(int argc, char **argv) {
    if(argc < 2) {
        printf("Usage:\t%s config_file\n", argv[0]);
        return ENOENT;
    }
    Config_t config = EMPTY_CONFIG;
    RET_NO(parseConfig(argv[1], &config), 0);
    parseConfig(argv[1], &config);

    // EXT_ON(unlink(config.sockname), -1);

    closeConditions_t cc = {.acceptConns = 1, .closeAll = 0};

    printf("Starting setup sequence\n");

    sigset_t sig_mask;
    sigemptyset(&sig_mask);
    sigaddset(&sig_mask, SIGINT);
    sigaddset(&sig_mask, SIGQUIT);
    sigaddset(&sig_mask, SIGHUP);

    EXT_PT(pthread_sigmask(SIG_BLOCK, &sig_mask, NULL));

    struct sigaction s;
    memset(&s,0,sizeof(s));    
    s.sa_handler=SIG_IGN;
    RET_ON((sigaction(SIGPIPE,&s,NULL)), -1, -1);

    printf("Masked signals\n");

    Storage_t *storage;
    EXT_ON((storage = initStorage(config.max_files, config.max_size)), NULL);

    int fd_pipes[2];
    RET_ON((pipe(fd_pipes)), -1, -1);
    int exit_pipes[2];
    RET_ON((pipe(exit_pipes)), -1, -1);

    Queue_t *fdq;
    RET_ON((fdq = init_queue()), NULL, -1);
    WArgs_t wargs = {.fd_pipe = fd_pipes[1], .queue = fdq, .storage = storage, .exit_pipe = exit_pipes[1], .cc = &cc};
    MArgs_t margs = {.fd_pipe = fd_pipes[0], .queue = fdq, .sockname = config.sockname, .exit_pipe = exit_pipes[0], .cc = &cc};

    printf("Spawning threads\n");

    pthread_t *worker_tids;
    RET_ON((worker_tids = malloc(config.n_workers * sizeof(pthread_t))), NULL, ENOMEM); //Should somehow free this
    for(int i = 0; i < config.n_workers; i++) {
        EXT_PT((pthread_create(&(worker_tids[i]), NULL, &runWorker, &wargs)));
    }

    pthread_t master_tid;
    EXT_PT(pthread_create(&master_tid, NULL, &runMaster, &margs));

    sig_loop(sig_mask, exit_pipes[1], fdq, &cc);

    EXT_PT(pthread_join(master_tid, NULL));
    cc.closeAll = 1;
    RET_ON(qWakeAll(fdq), -1, -1);
    for(int i = 0; i < config.n_workers; i++) {
        EXT_PT(pthread_join((worker_tids[i]), NULL));
    }
    printSummary(storage);
    free(worker_tids);
    RET_ON(qFree(fdq), -1, -1);
    RET_ON(StorageDestroy(storage), -1, -1);
    return 0;

    // int sfd;
    // fd_set set;
    // fd_set rdset;
    // int fd_num = 0;
    // struct sockaddr_un sa;
    // strncpy(sa.sun_path, config.sockname, UNIX_PATH_MAX);
    // sa.sun_family = AF_UNIX;
    // RET_ON((sfd = socket(AF_UNIX, SOCK_STREAM, 0)), -1, -1);
    // RET_ON((bind(sfd, (struct sockaddr *) &sa, sizeof(sa))), -1, -1);
    // RET_ON((listen(sfd, SOMAXCONN)), -1, -1);
    // if(sfd > fd_num) fd_num = sfd;
    // if(pfd[0] > fd_num) fd_num = pfd[0];
    // FD_ZERO(&set);
    // FD_SET(sfd, &set);
    // FD_SET(pfd[0], &set);


    // while(!doExit) {    
    //     rdset = set;
    //     if(select(fd_num+1, &rdset, NULL, NULL, NULL) == -1) {
    //         perror("Select error");
    //         continue;
    //     }
    //     for(int fd = 0; fd <= fd_num; fd++) {
    //         // printf("FD\tRD\tEX\n%d\t%d\t%d\n", fd, FD_ISSET(fd, &rdset), FD_ISSET(fd, &exset));
    //         if(FD_ISSET(fd, &rdset)) {
    //             if(fd == pfd[0]) {
    //                 int new_fd;
    //                 RET_ON((read(pfd[0], &new_fd, sizeof(int))), -1, -1);
    //                 FD_SET(new_fd, &set);
    //                 if(new_fd > fd_num) fd_num = new_fd;
    //             }
    //             else if(fd == sfd) {
    //                 int new_fd = accept(sfd, NULL, 0);
    //                 FD_SET(new_fd, &set);
    //                 if(new_fd > fd_num) fd_num = new_fd;
    //             } else {
    //                 FD_CLR(fd, &set);
    //                 RET_ON((qPush(fdq, fd)), -1, 98439); // Change this
    //             }
    //         }
    //     }
    // }
    // return 0;
}

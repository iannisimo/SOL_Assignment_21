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

/**
 * @brief Waits for a masked signal to be called and executes the handler
 * 
 * @return 0 on success, -1 on error 
 */
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

    closeConditions_t cc = {.acceptConns = 1, .closeAll = 0};

    printf("Starting setup sequence\n");

    // Masking signals

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
    // If the masted (dispatcher) terminates, every worker gets shutdown
    cc.closeAll = 1;
    RET_ON(qWakeAll(fdq), -1, -1);
    for(int i = 0; i < config.n_workers; i++) {
        EXT_PT(pthread_join((worker_tids[i]), NULL));
    }
    printSummary(storage);

    // Cleanup

    free(worker_tids);
    RET_ON(qFree(fdq), -1, -1);
    RET_ON(StorageDestroy(storage), -1, -1);
    return 0;
}

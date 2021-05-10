#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "connection.h"
#include "argparse.h"

void printAll(char **argv) {
    RET_ON(argv, NULL, ;)
    printf("Arguments\n");
    int run = 0;
    while(run + 1) {
        if(argv[run] == NULL || argv[run][0] == '\0') return;
        printf("\t%s\n", argv[run]);
        run++;
    }
}

int main(int argc, char **argv) {
    SingleArgs_t sargs = parseSingleArgs(argc, argv);
    if(IS_HELP(sargs.flags)) {
        printf(HELP_MSG);
        exit(0);
    }
    if(sargs.sockname[0] == '\0') {
        printf("Please use the option -f to select a socket\n");
        return -1;
    }
    Task_t task = getNext(argc, argv);
    printf("type:\t%c\nn:\t%d\nr:\t%s\n", task.type, task.n, (task.receive_folder != NULL) ? task.receive_folder : "");
    printAll(task.send_args);
    // openConnection(argv[1], 1500, (const struct timespec) {.tv_nsec = 5000000000});
    return 0;
}

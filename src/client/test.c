#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "connectionAPI.h"
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
    void* buf;
    size_t sz;
    openConnection(sargs.sockname, 1500, (const struct timespec) {.tv_nsec = 5000000000});
    openFile("/tmp/pippo", O_CREATE);
    // sleep(2);
    openFile("/tmp/pa", O_CREATE);
    // sleep(2);
    appendToFile("/tmp/pa", (void*) "foo", 4, NULL);
    // sleep(2);
    appendToFile("/tmp/pippo", (void*) "bar", 4, NULL);
    // sleep(2);
    readFile("/tmp/pa", &buf, &sz);

    readFile("/tmp/pippo", &buf, &sz);
    // printf("%ld: %s\n", sz, buf);
    // sleep(2);
    // readFile("/tmp/pippo", &buf, &sz);
    // printf("%ld: %s\n", sz, (char*) buf);
    // sleep(2);
    closeFile("/tmp/pippo");
    // sleep(2);
    closeFile("/tmp/pa");
    
    readNFiles(10, NULL);

    // free(buf);
    // sleep(2);
    // openFile("/tmp/pippo", 0);
    // sleep(2);
    // closeFile("/tmp/pippo");
    // sleep(2);
    // openFile("/tmp", 0);
    // sleep(2);
    // closeFile("sd");
    // void *buf;
    // size_t sz;
    // readFile("/tmp/pippo", &buf, &sz);
    // printf("%ld -> %s\n", sz, (char*) buf);
    // free(buf);
    return 0;
}

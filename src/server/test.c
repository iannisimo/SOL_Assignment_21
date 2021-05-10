#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "storage.h"
#include "utils.h"
#include "connection.h"

int main(int argc, char **argv) {
    printf("Hello World!\n");
    /* Storage test */
    // 
    // EXT_ON((s = initStorage()), NULL);
    // Storage_t *s;
    // pushFile(s, "Ciao", 2, (unsigned char *) "a");
    // pushFile(s, "Mondo", 2, (unsigned char *) "b");
    // pushFile(s, "Come", 2, (unsigned char *) "c");
    // pushFile(s, "Va?", 2, (unsigned char *) "d");

    EXT_ON((initSocket(argv[1])), -1);
    int fd;
    EXT_ON((fd = waitConnection()), -1);
    printf("%d\n", fd);
    return 0;
}

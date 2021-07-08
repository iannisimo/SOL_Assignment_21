#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <sys/stat.h>
#include <limits.h>

#include "storage.h"
#include "config.h"
#include "utils.h"
#include "connection.h"

int main(int argc, char **argv) {
    if(argc < 2) {
        printf("Usage:\t%s config_file\n", argv[0]);
        return ENOENT;
    }
    Config_t config = EMPTY_CONFIG;
    RET_NO(parseConfig(argv[1], &config), 0);
    parseConfig(argv[1], &config);
    RET_NO(initSocket(config.sockname), 0);
    int socket = waitConnection();
    int exit = 0;
    char buf[128];
    while(!exit) {
        size_t s = read(socket, buf, 128);
        // buf[2] = '\0';
        printf("%ld -> %s\n", s, buf);
        if(buf[0] == 'r') {
            write(socket, "0 13", 5);
            write(socket, "Hello World!", 13);
        } else {
            write(socket, "0", 2);
        }
        sleep(1);
        
    }
    /* Storage test */
    // 
    // Storage_t *s;
    // EXT_ON((s = initStorage()), NULL);
    // pushFile(s, "Ciao", 2, (unsigned char *) "a");
    // pushFile(s, "Mondo", 2, (unsigned char *) "b");
    // pushFile(s, "Come", 2, (unsigned char *) "c");
    // pushFile(s, "Va?", 2, (unsigned char *) "d");

    // EXT_ON((initSocket(argv[1])), -1);
    // int fd;
    // EXT_ON((fd = waitConnection()), -1);
    // printf("%d\n", fd);
    // printf("%ld\n", sizeof(CONFIG_PARAMS));


    // EXT_NO((parseConfig(argv[1], &config)), ERR_OK);
    return 0;
}

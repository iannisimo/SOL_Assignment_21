#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "storage.h"
#include "utils.h"

int main(int argc, char **argv) {
    printf("Hello World!\n");
    Storage_t *s;
    EXT_ON((s = initStorage()), NULL);
    pushFile(s, "Ciao", 2, (unsigned char *) "a");
    printf("%s\n", s->head->data);
    return 0;
}

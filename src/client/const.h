#ifndef __const_h
#define __const_h

#define O_HELP 1
#define O_PRINT 2

#define UNIX_PATH_MAX 108
#define BUF_MAX       384

#define HELP_MSG \
    "Available commands:\n\n\
    -h\t\t\t\tShow this help text\n\
    -f filename\t\t\tSpecify the name of the socket\n\
    -p\t\t\t\tEnable output to console\n\
    -w dirname[,n=0]\t\tSend the contents of dirname to the server.\n\
    \t\t\t\t\tIf `n != 0` it sends at most `n` files\n\
    -W file1[,file2[...]]\tSend each file to the server\n\
    -r file1[,file2[...]]\tRead back each file from the server\n\
    -R [n=0]\t\t\tRead `n` files from the server\n\
    \t\t\t\t\tIf `n == 0` it requests every file\n\
    -d dirname\t\t\tWhen placed after -r or -R it tells the program where to save the received files\n\
    -t time\t\t\tIt tells the program to wait `time` milliseconds before processing the next request in line\n\n"

#endif
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/ioctl.h>
#include <minix/ioctl.h>
#include <sys/ioc_dfa.h>


int main(int argc, char** argv) {
    int fd;
    char a = '0';

    if ((fd = open("/dev/dfa", O_WRONLY)) < 0)
        exit(1);

    for(int i=0; i<256; i++) {
        write(fd, &a, 1);
    }
    return 0;
}
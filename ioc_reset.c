#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/ioctl.h>
#include <minix/ioctl.h>
#include <sys/ioc_dfa.h>


int main(int argc, char** argv) {
    int fd;
    char a;

    if ((fd = open("/dev/dfa", O_RDONLY)) < 0)
        exit(1);

    if (ioctl(fd, DFAIOCRESET, NULL) < 0)
        exit(1);
}
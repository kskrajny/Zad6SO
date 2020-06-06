#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/ioctl.h>
#include <minix/ioctl.h>
#include <sys/ioc_dfa.h>

int main(int argc, char** argv) {
    int fd;
    char a, tr[3];

    if ((fd = open("/dev/dfa", O_RDONLY)) < 0)
        exit(1);

    tr[1] = '0';

    for (a = SCHAR_MIN; a != SCHAR_MAX; a++) {
        tr[0] = a;
        tr[2] = a+1; 
        if (ioctl(fd, DFAIOCADD, tr) < 0)
            exit(1);
    }

    tr[0] = SCHAR_MAX;
    tr[2] = SCHAR_MIN;
    if (ioctl(fd, DFAIOCADD, tr) < 0)
        exit(1);

    tr[0] = 0;
    if (ioctl(fd, DFAIOCACCEPT, tr) < 0)
        exit(1);
}
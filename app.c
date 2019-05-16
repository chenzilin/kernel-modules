#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

int main()
{
    int fd;
    int ret;
    char buf[20];

    if((fd = open("/dev/simple_cdev", O_RDWR)) < 0) {
        perror("open");
        return -1;
    }

    write(fd, "a test string", 15);

    ret = lseek(fd, 0, SEEK_SET);

    read(fd, buf, 15);
    printf("<app>buf is [%s]\n", buf);

    close(fd);

    return 0;
}

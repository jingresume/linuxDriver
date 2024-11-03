#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

const char* dev_path = "/dev/my_mpu6050";

int main(int argc, char *argv[])
{
    short rev_data[6];
    int fd = open(dev_path, O_RDWR);

    if (fd < 0)
    {
        printf("open %s failed\n", dev_path);
        return -1;
    }

    int error = read(fd, rev_data, sizeof(rev_data));

    if (error < 0)
    {
        printf("read file error\n");
        close(fd);
        return -1;
    }

    printf("AX = %d AY = %d AZ = %d\n", rev_data[0], rev_data[1], rev_data[2]);
    printf("GX = %d GY = %d GZ = %d\n\n", rev_data[3], rev_data[4], rev_data[5]);

    close(fd);

    return 0;
}
#include <stdio.h>
#include <fcntl.h>
#include<unistd.h>

#define DEV_CNT 3
#define DEV_NAME "led_chrdev"
#define DEV_NAME_LEN 20

struct devInfo
{
    int fd;
    char device_name[DEV_NAME_LEN];
};

int main()
{
    struct devInfo testInfo[DEV_CNT];
    int i = 0;
    for (; i < DEV_CNT; i++)
    {
        snprintf(testInfo[i].device_name, DEV_NAME_LEN, "/dev/"DEV_NAME"%d", i);
        testInfo[i].fd = open(testInfo[i].device_name, O_RDWR);
        if (testInfo[i].fd != -1)
        {
            printf("open %s success.\n", testInfo[i].device_name);
        }
        else
        {
            printf("error: open %s failed\n", testInfo[i].device_name);
            return -1;
        }
    }

    int k = 5;
    while (k--)
    {
        for (i = 0; i < DEV_CNT; i++)
        {
            write(testInfo[i].fd, "0", 1);
            printf("enable %d led\n", i);
            sleep(1);
            write(testInfo[i].fd, "1", 1);
        }
    }

    for (i = 0; i < DEV_CNT; i++)
    {
        close(testInfo[i].fd);
        printf("close %d\n", i);
    }

    return 0;
}
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>

#include "test.h"

#define DEV_PATH "/dev/myoled"

int main()
{
    printf("oled test\n");
    int fd = open(DEV_PATH, O_RDWR);
    if (fd < 0)
    {
        printf("open %s failed\n", DEV_PATH);
        return -1;
    }
    oled_clear(fd);
    char ip_str[20];
    get_ip(ip_str);
    const char* str = "Hello world";
    oled_show_text(fd, 0, 0, str);
    oled_show_text(fd, 0, 1, ip_str);
    sleep(2);
    close(fd);
    return 0;
}

void oled_show_text(int fd, int x, int y, const char* str)
{
    int idx = 0;
    oled_display_data *pData;
    int len = strlen(str);
    
    unsigned char buffer[len * 6];
    u32 buffer_len = 0;
    while (idx < len)
    {
        int val = str[idx] - 32;
        for (int i = 0; i < 6; i++)
        {
            buffer[buffer_len++] = F6x8[val][i];
        }
        idx++;
    }
    size_t size = sizeof(oled_display_data) + buffer_len;
    pData = malloc(sizeof(oled_display_data) + buffer_len);
    pData->x = x;
    pData->y = y;
    pData->len = buffer_len;
    printf("x %d y %d size %d\n", x, y, buffer_len);
    memcpy(pData->display_buffer, buffer, buffer_len);

    write(fd, pData, size);

    free(pData);
}

void oled_fill(int fd, int data)
{
    u32 buffer_len = OLED_WIDTH * OLED_HEIGHT / 8;
    oled_display_data *pData;
    
    size_t size = sizeof(oled_display_data) + buffer_len;
    pData = malloc(size);

    pData->x = 0;
    pData->y = 0;
    pData->len = buffer_len;
    memset(pData->display_buffer, data, buffer_len);

    write(fd, pData, size);

    free(pData);
}

void oled_clear(int fd)
{
    oled_fill(fd, 0x00);
}

void get_ip(char* data)
{
    struct ifaddrs *ifaddr, *ifa;

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("Get ifaddr failed");
        return;
    }
    char* host = NULL;
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL) continue;

        if (ifa->ifa_addr->sa_family == AF_INET && !(ifa->ifa_flags & IFF_LOOPBACK))
        {
            struct sockaddr_in *addr = (struct sockaddr_in*)ifa->ifa_addr;
            host = inet_ntoa(addr->sin_addr);
            printf("IP: %s\n", host);
        }
    }
    strcpy(data, host);

    freeifaddrs(ifaddr);
}
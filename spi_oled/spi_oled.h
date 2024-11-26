#ifndef __SPI_OLED_H
#define __SPI_OLED_H

#include <linux/types.h>

typedef unsigned char u8;
typedef unsigned int  u32;

#define OLED_WIDTH 128
#define OLED_HEIGHT 64

typedef struct oled_display_data
{
    u8 x;
    u8 y;
    u32 len;
    u8 display_buffer[];
} oled_display_data;

#endif
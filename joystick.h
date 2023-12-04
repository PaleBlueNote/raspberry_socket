#ifndef __JOTSTICK__
#define __JOYSTICK__

#include "header.h"
#include "spi.h"

char state[1] = "S";

int initJoystick()
{
    int fd = open(DEVICE, O_RDWR);
    if (fd <= 0)
    {
        perror("Device open error");
        return -1;
    }

    if (prepare(fd) == -1)
    {
        perror("Device prepare error");
        return -1;
    }

    return fd;
}

// 채널값 ch[0] ch[1] ch[2] 가져오기
void readJoystick(int fd)
{
    int ch[3] = {
        0,
    };
    for (int i = 0; i < 3; i++)
    {
        int value = readadc(fd, i);
        ch[i] = value;
    }
    printf("ch0: %d ch1: %d ch2: %d\n", ch[0], ch[1], ch[2]);
    if (((ch[0] > 400 && ch[0] < 430) && ch[1] == 0) && (ch[2] > 400 && ch[2] < 550))
    {
        printf("STOP\n");
        state[0] = 'S';
    }
    if (ch[1] == 0 && (ch[2] > 900 && ch[2] < 1100))
    {
        printf("Right\n");
        state[0] = 'R';
    }
    if (((ch[0] > 400 && ch[0] < 430) && ch[1] == 0) && ch[2] == 0)
    {
        printf("Left\n");
        state[0] = 'L';
    }
    if ((ch[0] > 1010 && ch[0] < 1023) && ch[1] == 0)
    {
        printf("Forward\n");
        state[0] = 'F';
    }
    if ((ch[0] > 240 && ch[0] < 270) && (ch[1] > 240 && ch[1] < 270))
    {
        printf("Backward\n");
        state[0] = 'B';
    }
}

#endif
#ifndef __TOUCH_CENSOR__
#define __TOUCH_CENSOR__

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <memory.h>
#include <string.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define BUFFER_SIZE 20

#define PIR 4       // BCM_GPIO 23
int clnt_sock = -1;
void handle_signal(int signum);
int setup_touch_sensor(void);

int fd;
void delay_f(void);
void print_skill(void);

int touched = 0;
void handle_signal(int signum);
int setup_touch_sensor(void);

#endif
#ifndef __TOUCH_LCD_SERVER__
#define __TOUCH_LCD_SERVER__

#include "header.h"

#define BUFFER_SIZE 20

// prepare for socket communication
#define PIR 4       // BCM_GPIO 23
int clnt_sock = -1;
void handle_signal(int signum);
int setup_touch_sensor(void);

// prepare for lcd print
#define I2C_ADDR   0x27
#define LCD_CHR  1
#define LCD_CMD  0
#define LINE1  0x80
#define LINE2  0xC0
#define LCD_BACKLIGHT   0x08
#define ENABLE  0b00000100
#define BUTTON_PIN 23
#define IN 0
#define OUT 1

#define LOW 0
#define HIGH 1

#define PIN 20
#define POUT 21
#define VALUE_MAX1 40
#define DIRECTION_MAX1 40
// variable, function for lcd print
int fd;
void lcd_init(void);
void lcd_byte(int bits, int mode);
void lcd_toggle_enable(int bits);
void lcd_m(int line); // 몇번째 줄에 출력할지
void print_int(int i); // 정수 출력 코드
void cursor_to_home(void); // 커서를 홈 위치로 이동
void print_str(const char *s); // 문자열 출력 코드
void get_set(void); // 준비 함수
void time_limit(void); // 시간 제한
void delay_f(void); // 지연 함수

#endif
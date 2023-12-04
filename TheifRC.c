#include "header.h"
#include "lcd.h"
#include "touch_censor.h"
#include "joystick.h"
#include "GPIO.h"
#include "motor.h"
#include "PWM.h"


void* thread_input_to_rc_socket(void* arg) {
    int rc_clnt_sock = *(int*)arg;
    int centi_sec_counter = 0;
    struct timespec delay;
    delay.tv_sec = 0; // 초 단위
    delay.tv_nsec = 10000000; // 10,000,000 나노초 = 0.01 초

    while (1) {
        //0.01초 마다 실행해야 하는 작업---------------------------------------------------
        // if(터치센서 입력받음){
        //     write(rc_clnt_sock, "touched", strlen("touched"));
        // }
        //0.01초 마다 실행해야 하는 작업---------------------------------------------------
        //0.1초마다
        if((centi_sec_counter%10)==0){
        }
        //0.1초마다

        //1초 마다 실행해야 하는 작업------------------------------------------------------
        if((centi_sec_counter%100)==0){

        }
        //1초 마다 실행해야 하는 작업------------------------------------------------------

        centi_sec_counter++;
        nanosleep(&delay, NULL);; // 0.01초마다 버튼 상태 체크
    }
}

void* thread_rc_socket_to_output(void* arg) {
    int rc_clnt_sock = *(int*)arg;
    while (1) {
        char buffer[1024];
        int valread = read(rc_clnt_sock, buffer, 1024);
        if (valread > 0) {
            printf("%s", buffer);
        //rc카에서 읽어드린 값 
            if(strncmp(buffer,"S",strlen("S")) == 0){
                stopMotor();
            }
            else if(strncmp(buffer,"R",strlen("R")) == 0){
                turnRight(1000);
            }
            else if(strncmp(buffer,"L",strlen("L")) == 0){
                turnLeft(1000);
            }
            else if(strncmp(buffer,"F",strlen("F")) == 0){
                goForward(800);
            }
            else if(strncmp(buffer,"B",strlen("B")) == 0){
                goBackward(800);
            }
            else{
                printf("joystick value is not clear\n");
            }
            printf("%s\n",buffer);
        }
    }
}

int main(int argc, char *argv[]) {
    int rc_clnt_sock;
    struct sockaddr_in rc_serv_addr;
    initMotor();

    if (argc != 3) {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if ((rc_clnt_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        error_handling("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    memset(&rc_serv_addr, 0, sizeof(rc_serv_addr));
    rc_serv_addr.sin_family = AF_INET;
    rc_serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    rc_serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(rc_clnt_sock, (struct sockaddr *)&rc_serv_addr, sizeof(rc_serv_addr)) < 0) {
        error_handling("Connection failed");
        exit(EXIT_FAILURE);
    }

    pthread_t rc_input_thread, rc_output_thread;
    //rc카 라즈베리파이 입출력 thread
    pthread_create(&rc_input_thread, NULL, thread_input_to_rc_socket, (void*)&rc_clnt_sock);
    pthread_create(&rc_output_thread, NULL, thread_rc_socket_to_output, (void*)&rc_clnt_sock);

    pthread_join(rc_input_thread, NULL);
    pthread_join(rc_output_thread, NULL);

    close(rc_clnt_sock);

    return 0;
}
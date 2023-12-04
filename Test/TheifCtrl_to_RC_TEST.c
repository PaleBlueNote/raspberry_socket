#include <wiringPi.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <wiringPiSPI.h>
#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>
#include <netinet/in.h>
#include <wiringPiI2C.h>
#include <memory.h>
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>

bool touched = false;

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void* thread_input_to_rc_clnt_socket(void* arg) {
    int rc_clnt_sock = *(int*)arg;
    int centi_sec_counter = 0;
    int countdown = 3;
    struct timespec delay;
    delay.tv_sec = 0; // 초 단위
    delay.tv_nsec = 10000000; // 10,000,000 나노초 = 0.01 초
    
    while (1) {
        //0.01초 마다 실행해야 하는 작업---------------------------------------------------

        //0.01초 마다 실행해야 하는 작업-------------------------------------------------

        //0.1초마다 실행해야 하는 작업---------------------------------------------------
        if((centi_sec_counter%10)==0){
            write(rc_clnt_sock, "조이스틱 값", strlen("조이스틱 값"));
        }
        //0.1초마다 실행해야 하는 작업---------------------------------------------------

        //1초 마다 실행해야 하는 작업----------------------------------------------------
        if((centi_sec_counter%100)==0){
        }
        //1초 마다 실행해야 하는 작업----------------------------------------------------

        centi_sec_counter++;
        nanosleep(&delay, NULL); // 0.01초마다 버튼 상태 체크
    }
}

void* thread_rc_clnt_socket_to_output(void* arg) {
    int rc_clnt_sock = *(int*)arg;
    while (1) {
        char buffer[1024];
        int valread = read(rc_clnt_sock, buffer, 1024);
        if (valread > 0) {
            //rc카에서 읽어드린 값
            if (strncmp(buffer, "touched", strlen("touched")) == 0) {
                touched = true;
                printf("touched : %s\n", touched ? "true" : "false");
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int rc_serv_sock, rc_clnt_sock;
    struct sockaddr_in rc_serv_addr;
    struct sockaddr_in rc_clnt_addr;
    socklen_t rc_clnt_addr_size = sizeof(rc_clnt_addr);
    if ((rc_serv_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        error_handling("RC Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&rc_serv_addr, 0, sizeof(rc_serv_addr));
    rc_serv_addr.sin_family = AF_INET;
    rc_serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    rc_serv_addr.sin_port = htons(atoi(argv[1]));
    
    if (bind(rc_serv_sock, (struct sockaddr *)&rc_serv_addr, sizeof(rc_serv_addr)) < 0) {
        error_handling("RC Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(rc_serv_sock, 3) < 0) {
        error_handling("Listen failed");
        exit(EXIT_FAILURE);
    }

    if ((rc_clnt_sock = accept(rc_serv_sock, (struct sockaddr *)&rc_clnt_addr, &rc_clnt_addr_size)) < 0) {
        error_handling("Accept rc socket failed");
        exit(EXIT_FAILURE);
    }

    pthread_t rc_input_thread, rc_output_thread;
    //rc카 라즈베리파이 입출력 thread
    pthread_create(&rc_input_thread, NULL, thread_input_to_rc_clnt_socket, (void*)&rc_clnt_sock);
    pthread_create(&rc_output_thread, NULL, thread_rc_clnt_socket_to_output, (void*)&rc_clnt_sock);

    pthread_join(rc_input_thread, NULL);
    pthread_join(rc_output_thread, NULL);

    close(rc_clnt_sock);
    close(rc_serv_sock);

    return 0;
}
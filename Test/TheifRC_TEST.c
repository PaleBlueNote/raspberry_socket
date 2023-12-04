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

void error_handling(char *message)
{
fputs(message, stderr);
fputc('\n', stderr);
exit(1);
}

void* thread_input_to_rc_socket(void* arg) {
    int rc_clnt_sock = *(int*)arg;
    int centi_sec_counter = 0;
    struct timespec delay;
    delay.tv_sec = 0; // 초 단위
    delay.tv_nsec = 10000000; // 10,000,000 나노초 = 0.01 초

    while (1) {
        //0.01초 마다 실행해야 하는 작업---------------------------------------------------
        //0.01초 마다 실행해야 하는 작업---------------------------------------------------
        //0.1초마다
        if((centi_sec_counter%10)==0){
            write(rc_clnt_sock, "touched", strlen("touched"));
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
        //rc카에서 읽어드린 값 
            printf("%s\n",buffer);
        }
    }
}

int main(int argc, char *argv[]) {
    int rc_clnt_sock;
    struct sockaddr_in rc_serv_addr;

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
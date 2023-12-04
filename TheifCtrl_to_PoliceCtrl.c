#include "header.h"
#include "lcd.h"
#include "touch_censor.h"
#include "joystick.h"
#include "GPIO.h"
#include "spi.h"

#define STARTBUTTON 14
#define SKILLBUTTON 15

int joystick_fd; // added
bool server_ready_state = false;
bool client_ready_state = false;
bool stop_skill_active = false; //change
bool have_chaos_skill = true;

void *thread_input_to_rc_clnt_socket(void *arg)
{
    // int rc_clnt_sock = *(int *)arg;
    // int centi_sec_counter = 0;
    // int countdown = 3;
    // struct timespec delay;
    // delay.tv_sec = 0;             // 초 단위
    // delay.tv_nsec = 10000000;     // 10,000,000 나노초 = 0.01 초
    // joystick_fd = initJoystick(); // added

    // while (1)
    // {
    //     // 0.01초 마다 실행해야 하는 작업---------------------------------------------------
    //     // if( stop_skill 버튼이 눌림 ){
    //     //   write(rc_clnt_sock, "stop", strlen("stop"));
    //     //   have_stop_skill = false;
    //     // }
    //     // if( chaos_skill 버튼이 눌림 ){
    //     //   write(rc_clnt_sock, "chaos", strlen("chaos"));
    //     //   have_chaos_skill = false;
    //     // }
    //     // 0.01초 마다 실행해야 하는 작업-------------------------------------------------

    //     // 0.1초마다 실행해야 하는 작업---------------------------------------------------
    //     if ((centi_sec_counter % 10) == 0)
    //     {
    //         readJoystick(joystick_fd);
    //         write(rc_clnt_sock, state, sizeof(state)); // 불안
    //     }
    //     // 0.1초마다 실행해야 하는 작업---------------------------------------------------

    //     // 1초 마다 실행해야 하는 작업----------------------------------------------------
    //     if ((centi_sec_counter % 100) == 0)
    //     {
    //     }
    //     // 1초 마다 실행해야 하는 작업----------------------------------------------------

    //     centi_sec_counter++;
    //     nanosleep(&delay, NULL); // 0.01초마다 버튼 상태 체크
    // }
}

void *thread_rc_clnt_socket_to_output(void *arg)
{
    // int rc_clnt_sock = *(int *)arg;
    // while (1)
    // {
    //     char buffer[1024];
    //     int valread = read(rc_clnt_sock, buffer, 1024);
    //     if (valread > 0)
    //     {
    //         // rc카에서 읽어드린 값
    //         if (strncmp(buffer, "touched", strlen("touched")) == 0)
    //         {
    //             touched = 1;
    //             printf("touched : %s\n", touched ? "1" : "0");
    //         }
    //     }
    // }
}

void *thread_input_to_ctrl_clnt_socket(void *arg)
{
    int ctrl_clnt_sock = *(int*)arg;
    int centi_sec_counter = 0;
    int countdown = 3;
    struct timespec delay;
    delay.tv_sec = 0;             // 초 단위
    delay.tv_nsec = 10000000;     // 10,000,000 나노초 = 0.01 초
    joystick_fd = initJoystick(); // added

    while (1) {
        //0.01초 마다 실행해야 하는 작업---------------------------------------------------
        //0.01초 마다 실행해야 하는 작업---------------------------------------------------

        //0.1초마다
        if((centi_sec_counter%10)==0){
            if ((GPIORead(SKILLBUTTON) == 0) && have_chaos_skill) {
              printf("Theif chaos skill button pressed");
              write(ctrl_clnt_sock, "Theif chaos skill button pressed", strlen("Theif chaos skill button pressed"));
              have_chaos_skill = false; //한번 쓰면 끝!
            }
        }
        //0.1초마다

        //1초 마다 실행해야 하는 작업------------------------------------------------------
        if((centi_sec_counter%100)==0){
            if (GPIORead(STARTBUTTON) == 0) { //시작버튼 눌림
              server_ready_state = !server_ready_state;
              printf("Theif button state changed: %s\n", server_ready_state ? "true" : "false");
            }

            if(!(server_ready_state && client_ready_state)){
                write(ctrl_clnt_sock, "Countdown Canceled", strlen("Countdown Canceled"));
                countdown = 3;//카운트 다운 초기화
                printf("Theif Ready State: %s, Police Ready State: %s\n",
                    server_ready_state ? "true" : "false",
                    client_ready_state ? "true" : "false");
            }
            if(server_ready_state && client_ready_state){
                //count down 3초 보내기
                write(ctrl_clnt_sock, "Countdown Start", strlen("Countdown Start"));
                printf("Countdown: %d seconds\n", countdown);
                countdown--;
            }

            //countdown이 0이면 game start
            if (countdown==0) {
                printf("Game Start!\n");
                write(ctrl_clnt_sock, "Game Start!", strlen("Game Start!"));
            }
          //1초 마다 실행해야 하는 작업------------------------------------------------------
        }

        centi_sec_counter++;
        nanosleep(&delay, NULL); // 0.01초마다 버튼 상태 체크
    }
}

void *thread_ctrl_clnt_socket_to_output(void *arg)
{
    int ctrl_clnt_sock = *(int*)arg;
    while (1) {
        char buffer[1024];
        int valread = read(ctrl_clnt_sock, buffer, 1024);
        if (valread > 0) {
          if (strncmp(buffer, "Police start button pressed",strlen("Police start button pressed")) == 0) {
            client_ready_state = !client_ready_state;
            printf("Police button state changed: %s\n", client_ready_state ? "true" : "false");
          }
          if (strncmp(buffer, "Police stop skill button pressed", strlen("Police stop skill button pressed")) == 0) {
            printf("Police stop skill button pressed\n");
            stop_skill_active = true;
          }
          if (strncmp(buffer, "touched", strlen("touched")) == 0) {
            touched = true;
          }
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage : %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int ctrl_serv_sock, ctrl_clnt_sock;
    struct sockaddr_in ctrl_serv_addr;
    struct sockaddr_in ctrl_clnt_addr;
    socklen_t ctrl_clnt_addr_size = sizeof(ctrl_clnt_addr);

    if ((ctrl_serv_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        error_handling("Control Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&ctrl_serv_addr, 0, sizeof(ctrl_serv_addr));
    ctrl_serv_addr.sin_family = AF_INET;
    ctrl_serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    ctrl_serv_addr.sin_port = htons(atoi(arg[1]));

    if (bind(ctrl_serv_sock, (struct sockaddr *)&ctrl_serv_addr, sizeof(ctrl_serv_addr)) < 0) {
        error_handling("Control Bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(ctrl_serv_sock, 3) < 0) {
        error_handling("Listen failed");
        exit(EXIT_FAILURE);
    }

    if ((ctrl_clnt_sock = accept(ctrl_serv_sock, (struct sockaddr *)&ctrl_clnt_addr, &ctrl_clnt_addr_size)) < 0) {
        error_handling("Accept client socket failed");
        exit(EXIT_FAILURE);
    }

    // int rc_serv_sock, rc_clnt_sock;
    // struct sockaddr_in rc_serv_addr;
    // struct sockaddr_in rc_clnt_addr;
    // socklen_t rc_clnt_addr_size = sizeof(rc_clnt_addr);
    // if ((rc_serv_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    // {
    //     error_handling("RC Socket creation failed");
    //     exit(EXIT_FAILURE);
    // }

    // memset(&rc_serv_addr, 0, sizeof(rc_serv_addr));
    // rc_serv_addr.sin_family = AF_INET;
    // rc_serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // rc_serv_addr.sin_port = htons(atoi(argv[2])); // rc카랑할때 1, 조종기포함 2

    // if (bind(rc_serv_sock, (struct sockaddr *)&rc_serv_addr, sizeof(rc_serv_addr)) < 0)
    // {
    //     error_handling("RC Bind failed");
    //     exit(EXIT_FAILURE);
    // }

    // if (listen(rc_serv_sock, 3) < 0)
    // {
    //     error_handling("RC Listen failed");
    //     exit(EXIT_FAILURE);
    // }

    // if ((rc_clnt_sock = accept(rc_serv_sock, (struct sockaddr *)&rc_clnt_addr, &rc_clnt_addr_size)) < 0)
    // {
    //     error_handling("Accept RC socket failed");
    //     exit(EXIT_FAILURE);
    // }

    SetGPIO(STARTBUTTON);
    SetGPIO(SKILLBUTTON);

    // pthread_t rc_input_thread, rc_output_thread;
    pthread_t clnt_input_thread, clnt_output_thread;
    // rc카 라즈베리파이 입출력 thread
    // pthread_create(&rc_input_thread, NULL, thread_input_to_rc_clnt_socket, (void *)&rc_clnt_sock);
    // pthread_create(&rc_output_thread, NULL, thread_rc_clnt_socket_to_output, (void *)&rc_clnt_sock);

    경찰조종기쪽으로 라즈베리파이 입출력 thread
     pthread_create(&clnt_input_thread, NULL, thread_input_to_ctrl_clnt_socket, (void*)&ctrl_clnt_sock);
     pthread_create(&clnt_output_thread, NULL, thread_ctrl_clnt_socket_to_output, (void*)&ctrl_clnt_sock);

    // pthread_join(rc_input_thread, NULL);
    // pthread_join(rc_output_thread, NULL);
    pthread_join(clnt_input_thread, NULL);
    pthread_join(clnt_output_thread, NULL);

    // close(rc_clnt_sock);
    // close(rc_serv_sock);
    close(ctrl_clnt_sock);
    close(ctrl_serv_sock);

    if (GPIOUnexport(POUT) == -1 || GPIOUnexport(PIN) == -1)
    {
        return 4;
    }

    return 0;
}
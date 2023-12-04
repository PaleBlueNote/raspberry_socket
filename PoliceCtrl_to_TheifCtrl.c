#include "header.h"
#include "lcd.h"
#include "touch_censor.h"
#include "joystick.h"
#include "GPIO.h"
#include "spi.h"

#define STARTBUTTON 2
#define SKILLBUTTON 3

int joystick_fd; // added
// bool server_ready_state = false;
// bool client_ready_state = false;
bool chaos_skill_active = false; //change
bool have_stop_skill = true;
bool countdown_start = false;
bool game_start = false;

void* thread_input_to_ctrl_socket(void* arg) {
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
            if ((GPIORead(SKILLBUTTON) == 0) && have_stop_skill) {
                printf("client stop skill button pressed");
                write(ctrl_clnt_sock, "Police stop skill button pressed", strlen("Police stop skill button pressed"));
                have_stop_skill = false;
            }
        }
        //0.1초마다
    
        if((centi_sec_counter%100)==0){
        //1초 마다 실행해야 하는 작업------------------------------------------------------
            if (GPIORead(STARTBUTTON) == 0) {
                printf("client start button pressed");
                write(ctrl_clnt_sock, "client start button pressed", strlen("client start button pressed"));
            }
            if(!countdown_start){
                countdown = 3; //카운트 다운 초기화
            }
            
            if(countdown_start){
                // lcd_m(LINE2);
                // print_int(countdown);
                // count down 3초 보내기          
                printf("Countdown: %d seconds\n", countdown);
                countdown--;
            }
            if (countdown==0) {
                printf("Game Start!\n");
            }
            // if (game_start) {
            //     int time_limit = 120; // 2분
            //     int detected = 0;

            //     while (time_limit >= 0) {
            //         lcd_m(LINE2);
            //         print_int(time_limit / 60);
            //         print_str("m ");
            //         print_int(time_limit % 60);
            //         print_str("s ");

            //         // Check for touch sensor detection        
            //         if (!digitalRead(PIR)) {       
            //             --time_limit;
            //         }

            //         else
            //         {
            //             // 감지되었을 때
            //             printf("Detected\n");
                        
            //             // Notify the client to gracefully exit
            //             write(clnt_sock, "Police win!", sizeof("Police win!"));

            //             lcd_m(LINE1);
            //             print_str("Police win!");

            //             detected = 1;
            //             exit(0);
            //         }
            //     }
                
            //     // Time limit 끝나도 감지되지 않았을 때
            //     if (detected == 0) {
            //         write(clnt_sock, "Theif win!", sizeof("Theif win!"));

            //         lcd_m(LINE1);
            //         print_str("Theif win!");
            //     }
            // }
        //1초 마다 실행해야 하는 작업------------------------------------------------------
        }

        centi_sec_counter++;
        nanosleep(&delay, NULL); // 0.01초마다 버튼 상태 체크
    }
}

void* thread_ctrl_socket_to_output(void* arg) {
    int ctrl_clnt_socket = *(int*)arg;
    while (1) {
        char buffer[1024];
        int valread = read(ctrl_clnt_socket, buffer, 1024);
        if (valread > 0) {
            if (strncmp(buffer, "Countdown Start",strlen("Countdown Start")) == 0) {
                printf("Countdown Start\n");
                countdown_start= true;
            }
            if (strncmp(buffer, "Countdown Canceled",strlen("Countdown Canceled")) == 0) {
                printf("Countdown Canceled\n");
                countdown_start= false;
            }
            if (strncmp(buffer, "Game Start!") == 0) {
                printf("Game Start!\n");
                game_start= true;
            }
            if (strncmp(buffer, "Theif chaos skill button pressed", strlen("Theif chaos skill button pressed")) == 0) {
                printf("Theif chaos skill button pressed\n");
                chaos_skill_active = true;
            }
        }
    }
}

int main(int argc, char *argv[]) {

    if (argc != 3) {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    //------------------------------rcCar server socket------------------------------
    // int rc_serv_sock, rc_clnt_sock;
    // struct sockaddr_in rc_serv_addr;
    // struct sockaddr_in rc_clnt_addr;
    // socklen_t rc_clnt_addr_size = sizeof(rc_clnt_addr);

    // if ((rc_serv_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    //     error_handling("RC Socket creation failed");
    //     exit(EXIT_FAILURE);
    // }

    // memset(&rc_serv_addr, 0, sizeof(rc_serv_addr));
    // rc_serv_addr.sin_family = AF_INET;
    // rc_serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    // rc_serv_addr.sin_port = htons(atoi(argv[3]));

    // if (bind(rc_serv_sock, (struct sockaddr *)&rc_serv_addr, sizeof(rc_serv_addr)) < 0) {
    //     error_handling("RC Bind failed");
    //     exit(EXIT_FAILURE);
    // }

    // if (listen(rc_serv_sock, 3) < 0) {
    //     error_handling("Listen failed");
    //     exit(EXIT_FAILURE);
    // }

    // if ((rc_clnt_sock = accept(rc_serv_sock, (struct sockaddr *)&rc_clnt_addr, &rc_clnt_addr_size)) < 0) {
    //     error_handling("Accept rc socket failed");
    //     exit(EXIT_FAILURE);
    // }
    //---------------------------------------------------------------------------

    //------------------------------control client socket------------------------------
    int ctrl_clnt_sock;
    struct sockaddr_in ctrl_serv_addr;

    if ((ctrl_clnt_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        error_handling("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    memset(&ctrl_serv_addr, 0, sizeof(ctrl_serv_addr));
    ctrl_serv_addr.sin_family = AF_INET;
    ctrl_serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    ctrl_serv_addr.sin_port = htons(atoi(argv[2]));

    if (connect(ctrl_clnt_sock, (struct sockaddr *)&ctrl_serv_addr, sizeof(ctrl_serv_addr)) < 0) {
        error_handling("Connection failed");
        exit(EXIT_FAILURE);
    }
    //---------------------------------------------------------------------------

    SetGPIO(STARTBUTTON);
    SetGPIO(SKILLBUTTON);

    // pthread_t rc_input_thread, rc_output_thread;
    pthread_t ctrl_input_thread, ctrl_output_thread;
    //rc카 라즈베리파이 입출력 thread
    // pthread_create(&rc_input_thread, NULL, thread_input_to_rc_socket, (void*)&rc_clnt_sock);
    // pthread_create(&rc_output_thread, NULL, thread_rc_socket_to_output, (void*)&rc_clnt_sock);

    //경찰(서버)조종기쪽으로 라즈베리파이 입출력 thread
    pthread_create(&ctrl_input_thread, NULL, thread_input_to_ctrl_socket, (void*)&ctrl_clnt_sock);
    pthread_create(&ctrl_output_thread, NULL, thread_ctrl_socket_to_output, (void*)&ctrl_clnt_sock);

    // pthread_join(rc_input_thread, NULL);
    // pthread_join(rc_output_thread, NULL);
    pthread_join(ctrl_input_thread, NULL);
    pthread_join(ctrl_output_thread, NULL);

    // close(rc_serv_sock);
    // close(rc_clnt_sock);
    close(ctrl_clnt_sock);

    return 0;
}
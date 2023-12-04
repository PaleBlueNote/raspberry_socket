#include "header.h"
#include "touch_lcd_server.h"
#include "joystick.h"
#include "GPIO.h"

//start button
#define PIN 20
#define POUT 21

int countdown_start = false;
bool have_stop_skill = true;
bool have_chaos_skill = true;

bool game_start = false;
bool touched = false;


void* thread_input_to_rc_socket(void* arg) {
    int rc_clnt_sock = *(int*)arg;
    int centi_sec_counter = 0;
    int joyFD = initJoystick();
    
    while (1) {
        //0.01초 마다 실행해야 하는 작업---------------------------------------------------
        readJoystick(joyFD);
        write(rc_clnt_sock, state, sizeof(state));
        
        if( have_stop_skill 버튼이 눌림 ){
        write(rc_clnt_sock, "have_stop_skill", strlen("have_stop_skill"));
        have_stop_skill = false;
        }
        if( have_chaos_skill 버튼이 눌림 ){
        write(rc_clnt_sock, "have_chaos_skill", strlen("have_chaos_skill"));
        have_chaos_skill = false;
        }
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
        usleep(10000); // 0.01초마다 버튼 상태 체크
    }
}

void* thread_rc_socket_to_output(void* arg) {
    int rc_sock = *(int*)arg;
    while (1) {
        char buffer[1024];
        int valread = read(rc_sock, buffer, 1024);
        if (valread > 0) {
        //rc카에서 읽어드린 값 
        if(strcmp(buffer,"터치센서건드림")){
            //game over
            touched = true;

        }
        }
    }
}

void* thread_input_to_ctrl_socket(void* arg) {
    int ctrl_clnt_socket = *(int*)arg;
    int centi_sec_counter = 0;
    int countdown = 3;

    while (1) {
        //0.01초 마다 실행해야 하는 작업---------------------------------------------------
        if (감동) {
            write(ctrl_clnt_socket, "touched", strlen("touched"));
        }

        //0.01초 마다 실행해야 하는 작업---------------------------------------------------

        //0.1초마다
        if((centi_sec_counter%10)==0){
            if (GPIORead(시작버튼 핀) == 0) {
            printf("client start button pressed");
            write(ctrl_clnt_socket, "client start button pressed", strlen("client start button pressed"));
            }
            if (GPIORead(멈춤 스킬버튼핀) == 0) {
            printf("client stop skill button pressed");
            write(ctrl_clnt_socket, "client stop skill button pressed", strlen("client stop skill button pressed"));
            }
            if (GPIORead(카오스 스킬버튼핀) == 0) {
            printf("client chaos skill button pressed");
            write(ctrl_clnt_socket, "client chaos skill button pressed", strlen("client chaos skill button pressed"));
            }
        }
        //0.1초마다
    
        if((centi_sec_counter%100)==0){
        //1초 마다 실행해야 하는 작업------------------------------------------------------
        if(!countdown_start){
            countdown = 3; //카운트 다운 초기화
        }
        
        else{
            lcd_m(LINE2);
            print_int(countdown);
            // count down 3초 보내기          
            // printf("Countdown: %d seconds\n", countdown);
            countdown--;
        }
        
        //countdown이 0이면 game start
        if (!countdown) {
            game_start = true;
            printf("Game Start!\n");
        }

        if (game_start) {
            int time_limit = 120; // 2분
            int detected = 0;

            while (time_limit >= 0) {
                lcd_m(LINE2);
                print_int(time_limit / 60);
                print_str("m ");
                print_int(time_limit % 60);
                print_str("s ");

                // Check for touch sensor detection        
                if (!digitalRead(PIR)) {       
                    --time_limit;
                }

                else
                {
                    // 감지되었을 때
                    printf("Detected\n");
                    
                    // Notify the client to gracefully exit
                    write(clnt_sock, "Police win!", sizeof("Police win!"));

                    lcd_m(LINE1);
                    print_str("Police win!");

                    detected = 1;
                    exit(0);
                }
            }
            
            // Time limit 끝나도 감지되지 않았을 때
            if (detected == 0) {
                write(clnt_sock, "Theif win!", sizeof("Theif win!"));

                lcd_m(LINE1);
                print_str("Theif win!");
            }
        }
        //1초 마다 실행해야 하는 작업------------------------------------------------------
        }

        centi_sec_counter++;
        usleep(10000); // 0.01초마다 버튼 상태 체크
    }
}

void* thread_ctrl_socket_to_output(void* arg) {
    int ctrl_clnt_socket = *(int*)arg;
    while (1) {
        char buffer[1024];
        int valread = read(ctrl_clnt_socket, buffer, 1024);
        if (valread > 0) {
        if (strcmp(buffer, "Countdown Start") == 0) {
            countdown_start= true;
        }
        if (strcmp(buffer, "Game Start!") == 0) {
            printf("Game Start!\n");
        }
        if (strcmp(buffer, "server stop skill button pressed") == 0) {
            //모터멈춤
        }
        if (strcmp(buffer, "server chaos skill button pressed") == 0) {
            //모터 반대로
        }
        }
    }
}

int main(int argc, char *argv[]) {
    int rc_serv_sock, rc_clnt_sock, ctrl_clnt_sock;
    struct sockaddr_in rc_serv_addr;
    struct sockaddr_in rc_clnt_addr;
    struct sockaddr_in ctrl_serv_addr;
    socklen_t rc_clnt_addr_size = sizeof(rc_clnt_addr);

    if (argc != 3) {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    //------------------------------rcCar server socket------------------------------
    if ((rc_serv_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        error_handling("RC Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&rc_serv_addr, 0, sizeof(rc_serv_addr));
    rc_serv_addr.sin_family = AF_INET;
    rc_serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    rc_serv_addr.sin_port = htons(atoi(argv[3]));

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
    //---------------------------------------------------------------------------

    //------------------------------control client socket------------------------------
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

    SetGPIO(POUT,PIN);

    pthread_t rc_input_thread, rc_output_thread, ctrl_input_thread, ctrl_output_thread;
    //rc카 라즈베리파이 입출력 thread
    pthread_create(&rc_input_thread, NULL, thread_input_to_rc_socket, (void*)&rc_clnt_sock);
    pthread_create(&rc_output_thread, NULL, thread_rc_socket_to_output, (void*)&rc_clnt_sock);

    //경찰(서버)조종기쪽으로 라즈베리파이 입출력 thread
    pthread_create(&ctrl_input_thread, NULL, thread_input_to_ctrl_socket, (void*)&ctrl_clnt_sock);
    pthread_create(&ctrl_output_thread, NULL, thread_ctrl_socket_to_output, (void*)&ctrl_clnt_sock);

    pthread_join(rc_input_thread, NULL);
    pthread_join(rc_output_thread, NULL);
    pthread_join(ctrl_input_thread, NULL);
    pthread_join(ctrl_output_thread, NULL);

    close(rc_serv_sock);
    close(rc_clnt_sock);
    close(ctrl_clnt_sock);

    return 0;
}
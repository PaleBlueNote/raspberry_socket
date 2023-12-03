#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>

#define IN 0
#define OUT 1

#define LOW 0
#define HIGH 1

#define PIN 20
#define POUT 21
#define VALUE_MAX 40
#define DIRECTION_MAX 40

bool server_ready_state = false;
bool client_ready_state = false;

static int GPIOExport(int pin) {
#define BUFFER_MAX 3
  char buffer[BUFFER_MAX];
  ssize_t bytes_written;
  int fd;

  fd = open("/sys/class/gpio/export", O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open export for writing!\n");
    return (-1);
  }

  bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
  write(fd, buffer, bytes_written);
  close(fd);
  return (0);
}

static int GPIOUnexport(int pin) {
  char buffer[BUFFER_MAX];
  ssize_t bytes_written;
  int fd;

  fd = open("/sys/class/gpio/unexport", O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open unexport for writing!\n");
    return (-1);
  }

  bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
  write(fd, buffer, bytes_written);
  close(fd);
  return (0);
}

static int GPIODirection(int pin, int dir) {
  static const char s_directions_str[] = "in\0out";

  char path[DIRECTION_MAX];
  int fd;

  snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
  fd = open(path, O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open gpio direction for writing!\n");
    return (-1);
  }

  if (-1 ==
      write(fd, &s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2 : 3)) {
    fprintf(stderr, "Failed to set direction!\n");
    return (-1);
  }

  close(fd);
  return (0);
}

static int GPIORead(int pin) {
  char path[VALUE_MAX];
  char value_str[3];
  int fd;

  snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
  fd = open(path, O_RDONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open gpio value for reading!\n");
    return (-1);
  }

  if (-1 == read(fd, value_str, 3)) {
    fprintf(stderr, "Failed to read value!\n");
    return (-1);
  }

  close(fd);

  return (atoi(value_str));
}

static int GPIOWrite(int pin, int value) {
  static const char s_values_str[] = "01";

  char path[VALUE_MAX];
  int fd;

  snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
  fd = open(path, O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open gpio value for writing!\n");
    return (-1);
  }

  if (1 != write(fd, &s_values_str[LOW == value ? 0 : 1], 1)) {
    fprintf(stderr, "Failed to write value!\n");
    return (-1);
  }

  close(fd);
  return (0);
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

void* thread_input_to_rc_socket(void* arg) {
    int rc_sock = *(int*)arg;
    int centi_sec_counter = 0;
    int countdown = 3;

    while (1) {
        //0.01초 마다 실행해야 하는 작업---------------------------------------------------
        if (GPIORead(PIN) == 0 ) { //조이스틱 값이 변경되었을 때
            write(rc_sock, "조이스틱 값", strlen("조이스틱 값"));
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
          }
        }
    }
}

void* thread_input_to_clnt_socket(void* arg) {
    int client_socket = *(int*)arg;
    int centi_sec_counter = 0;
    int countdown = 3;

    while (1) {
        //0.01초 마다 실행해야 하는 작업---------------------------------------------------
        //0.01초 마다 실행해야 하는 작업---------------------------------------------------

        //0.1초마다
        if((centi_sec_counter%10)==0){
            if (GPIORead(PIN) == 0) {
              server_ready_state = !server_ready_state;
              printf("sever button state changed: %s\n", server_ready_state ? "true" : "false");
            }
            if (GPIORead(멈춤 스킬버튼핀) == 0) {
              printf("sever stop skill button pressed");
              write(client_socket, "sever stop skill button pressed", strlen("sever stop skill button pressed"));
            }
            if (GPIORead(카오스 스킬버튼핀) == 0) {
              printf("sever chaos skill button pressed");
              write(client_socket, "sever chaos skill button pressed", strlen("sever chaos skill button pressed"));
            }
        }
        //0.1초마다
      
        if((centi_sec_counter%100)==0){
          //1초 마다 실행해야 하는 작업------------------------------------------------------
          if(!(server_ready_state && client_ready_state)){
            countdown = 3;//카운트 다운 초기화
            printf("Server Ready State: %s, Client Ready State: %s\n",
                server_ready_state ? "true" : "false",
                client_ready_state ? "true" : "false");
          }
          else{
            //count down 3초 보내기          
            write(client_socket, "Countdown Start", strlen("Countdown Start"));
            printf("Countdown: %d seconds\n", countdown);
            countdown--;
          }
          //countdown이 0이면 game start
          if (countdown==0) {
            printf("Game Start!\n");
            write(client_socket, "Game Start!", strlen("Game Start!"));
          }
          //1초 마다 실행해야 하는 작업------------------------------------------------------
        }

        centi_sec_counter++;
        usleep(10000); // 0.01초마다 버튼 상태 체크
    }
}

void* thread_clnt_socket_to_output(void* arg) {
    int client_socket = *(int*)arg;
    while (1) {
        char buffer[1024];
        int valread = read(client_socket, buffer, 1024);
        if (valread > 0) {
          if (strcmp(buffer, "client start button pressed") == 0) {
                client_ready_state = !client_ready_state;
                printf("Client button state changed: %s\n", client_ready_state ? "true" : "false");
          }
          if (strcmp(buffer, "client stop skill button pressed") == 0) {
            //server모터멈춤
          }
          if (strcmp(buffer, "client chaos skill button pressed") == 0) {
            //server모터 반대로
          }
        }
    }
}

int main(int argc, char *argv[]) {
    int serv_sock, clnt_sock, rc_sock;
    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;
    struct sockaddr_in rc_addr;
    socklen_t clnt_addr_size = sizeof(clnt_addr);
    socklen_t rc_addr_size = sizeof(rc_addr);

    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if ((serv_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        error_handling("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        error_handling("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(serv_sock, 3) < 0) {
        error_handling("Listen failed");
        exit(EXIT_FAILURE);
    }

    if ((clnt_sock = accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size)) < 0) {
        error_handling("Accept client socket failed");
        exit(EXIT_FAILURE);
    }

    if ((rc_sock = accept(serv_sock, (struct sockaddr *)&rc_addr, &rc_addr_size)) < 0) {
        error_handling("Accept rc socket failed");
        exit(EXIT_FAILURE);
    }

    if (GPIOExport(POUT) == -1 || GPIOExport(PIN) == -1) {
        error_handling("GPIO Export failed");
        exit(EXIT_FAILURE);
    }

    if (GPIODirection(POUT, OUT) == -1 || GPIODirection(PIN, IN) == -1) {
        error_handling("GPIO Direction failed");
        exit(EXIT_FAILURE);
    }

    if (GPIOWrite(POUT, 1) == -1) {
        error_handling("GPIO Write failed");
        return 3;
    }

    pthread_t rc_input_thread, rc_output_thread, clnt_input_thread, clnt_output_thread;
    //rc카 라즈베리파이 입출력 thread
    pthread_create(&rc_input_thread, NULL, thread_input_to_rc_socket, (void*)&rc_sock);
    pthread_create(&rc_output_thread, NULL, thread_rc_socket_to_output, (void*)&rc_sock);

    //도둑조종기 라즈베리파이 입출력 thread
    pthread_create(&clnt_input_thread, NULL, thread_input_to_clnt_socket, (void*)&clnt_sock);
    pthread_create(&clnt_output_thread, NULL, thread_clnt_socket_to_output, (void*)&clnt_sock);

    pthread_join(rc_input_thread, NULL);
    pthread_join(rc_output_thread, NULL);
    pthread_join(clnt_input_thread, NULL);
    pthread_join(clnt_output_thread, NULL);

    close(rc_sock);
    close(clnt_sock);	
	  close(serv_sock);

    if (GPIOUnexport(POUT) == -1 || GPIOUnexport(PIN) == -1) {
        return 4;
    }

    return 0;
}

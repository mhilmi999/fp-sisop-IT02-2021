#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <math.h>

#define PORT 8000

int main(int argc, char const *argv[]) {
    char username[1024];
    bzero(username, sizeof(username));
    char password[1024];
    bzero(password, sizeof(password));
    char error[1024];
    bzero(error, sizeof(error));
    int valsend;
    int valread;

    if (argc == 1 && geteuid() == 0) {
        strcpy(username, "root");
        strcpy(password, "root");
    } else if (argc == 5) {
        strcpy(username, argv[2]);
        strcpy(password, argv[4]);
    } else {
        strcpy(error, "Usage: ./client -u username -p password");
        printf("%s", error);
        exit(EXIT_FAILURE);
    }


    struct sockaddr_in address;
    int sock = 0;
    struct sockaddr_in serv_addr;

    char input[1024];
    bzero(input, sizeof(input));
    char buffer[1024];
    bzero(buffer, sizeof(buffer));

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }
  
    memset(&serv_addr, '0', sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
      
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
  
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    bzero(buffer, sizeof(buffer));
    valsend = send(sock, username, strlen(username), 0);
    if(valsend < 0)
    {
        printf("send error");
    }
    sleep(1);
    valsend = send(sock, password, strlen(password), 0);
    if (valsend < 0)
    {
        printf("send error");
    }
    
    valread = read(sock, buffer, 1024);
    if (valread < 0)
    {
        printf("Read error");
    }
    
    printf("%s\n", buffer);
    if (strncmp(buffer, "login success", 13) != 0) {
        close(sock);
        return 0;
    }

    // logged in
    while (1) {
        bzero(buffer, sizeof(buffer));

        printf("lambang> ");
        scanf("%[^\n]%*c", input);

        if (strcmp(input, "exit") == 0 || strcmp(input, "quit") == 0 || strcmp(input, "q") == 0) {
            bzero(input, sizeof(input));
            sprintf(input, "exit");
            valsend = send(sock, input, strlen(input), 0);
            if(valsend < 0)
            {
                printf("send error");
            }
            close(sock);
            break;
        }

        valsend = send(sock, input, strlen(input), 0);
        if (valsend < 0)
        {
            printf("send error");
        }
        
        valread = read(sock, buffer, 1024);
        if (valread < 0)
        {
            printf("read error");
        }
        
        if (strlen(buffer) && strcmp(buffer, "yes") != 0) {
            printf("%s\n", buffer);
        }

        bzero(input, sizeof(input));
    }

    return 0;
}
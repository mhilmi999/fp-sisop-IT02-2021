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
    char database[1024];
    bzero(database, sizeof(database));

    int valsend, valread;
    if (argc == 2 && geteuid() == 0) {
        strcpy(username, "root");
        strcpy(password, "root");
        strcpy(database, argv[1]);
    } else if (argc == 6) {
        strcpy(username, argv[2]);
        strcpy(password, argv[4]);
        strcpy(database, argv[5]);
    } else {
        strcpy(error, "Usage: ./dumper -u username -p password database");
        printf("%s\n", error);
        exit(EXIT_FAILURE);
    }


    struct sockaddr_in address;
    int sock = 0;
    struct sockaddr_in serv_addr;

    // msg for server
    char input[1024];
    bzero(input, sizeof(input));
    // msg from server
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
  
    // jika gagal menyambungkan ke server manapun maka tampil "Connection Failed"
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }


    // code here
    memset(buffer, 0, sizeof(buffer));
    bzero(buffer, sizeof(buffer));
    send(sock, username, strlen(username), 0);
    sleep(1);
    send(sock, password, strlen(password), 0);

    // success or fail
    read(sock, buffer, 1024);
    if (strcmp(buffer, "login success") != 0) {
        close(sock);
        return 0;
    }

    sprintf(input, "dump %s", database);
    send(sock, input, strlen(input), 0);
    read(sock, buffer, 1024);
    printf("%s\n", buffer);
    
    close(sock);

    return 0;
}
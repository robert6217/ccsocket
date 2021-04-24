#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define PORT 8888
#define BUFFER 1024

int main() {
    char buffer[BUFFER] = {'\0'}, fn[BUFFER] = {'\0'}, ip[BUFFER] = {'\0'};
    int clientSocket;
    struct sockaddr_in serverAddr;

    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("[Client] Error in connection.\n");
        exit(1);
    }
    printf("[Client] Socket is created.\n");

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if ((connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr))) < 0) {
        printf("[Client] Error in connection.\n");
        exit(1);
    }
    printf("[Client] Connected to Server.\n");
    printf("\t-> [Client] Type \":exit\" to exit the connect.\n");
    printf("\t-> [Client] Type \":file\" to enter the file mode.\n");
    while (1) {
        printf("[Client] ");
        scanf("%s", buffer);

        send(clientSocket, buffer, sizeof(buffer), 0);

        if (strcmp(buffer, ":exit") == 0) {
            close(clientSocket);
            printf("[Client] Disconnected from server.\n");
            exit(1);
        } else if (strcmp(buffer, ":file") == 0) {
            while (1) {
                printf("\t-> [Client] Type \":exitfile\" to exit the file mode.\n");
                printf("\t-> [Client] Enter your file name: ");
                scanf("%s", fn);
                FILE *ipList;
                if (strcmp(fn, ":exitfile") == 0) {
                    char exitfile[] = ":exitfile";
                    send(clientSocket, exitfile, sizeof(exitfile), 0);
                    printf("\t-> [Client] %s\n", exitfile);
                    break;
                } else if ((ipList = fopen(fn, "r")) == NULL) {
                    perror("\t-> [Client] Open File\n");
                    exit(1);
                }
                fgets(ip, sizeof(ip), ipList);
                while (feof(ipList) == 0) {
                    ip[strlen(ip) - 1] = 0;
                    if (strcmp(&ip[strlen(ip) - 1], ",") == 0) {
                        ip[strlen(ip) - 1] = 0;
                    }
                    printf("\t-> [Client] ip: %s ", ip);
                    send(clientSocket, ip, strlen(ip) + 1, 0);
                    recv(clientSocket, buffer, sizeof(buffer), 0);
                    printf("\t-> [Server] %s\n", buffer);
                    fgets(ip, sizeof(ip), ipList);
                }
                fclose(ipList);
            }

        } else {  //echo
            memset(buffer, '\0', sizeof(buffer));
            if (recv(clientSocket, buffer, sizeof(buffer), 0) < 0) {
                printf("[Client] Error in receiving data.\n");
            } else {
                printf("[Server] %s\n", buffer);
                memset(buffer, '\0', sizeof(buffer));
            }
        }
    }
    return 0;
}
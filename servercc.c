#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <regex.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#define PORT 8888
#define RECVBUFFER 2048
#define LOGBUFFER 1024

bool ipreg(const char *ip, regex_t ipRegex) {
    regmatch_t matchptr[1];
    const size_t nmatch = 1;
    int status = regexec(&ipRegex, ip, nmatch, matchptr, 0);
    if (status == REG_NOMATCH) {
        return false;
    } else if (status == 0) {
        return true;
    } else {
        char msgbuf[256];
        regerror(status, &ipRegex, msgbuf, sizeof(msgbuf));
        printf("regmatch_t: %s\n", msgbuf);
        return false;
    }
}

int main() {
    int sockfd = 0, newSocket = 0;
    socklen_t addr_size;
    char buffer[RECVBUFFER];
    struct sockaddr_in serverAddr;
    struct sockaddr_in newAddr;
    char log[LOGBUFFER] = {'\0'};
    FILE *fplog = fopen("test.txt", "a");
    pid_t childpid;
    regex_t ipRegex;
    sem_t *sem;
    sem = sem_open("pSem", O_CREAT | O_EXCL, 0644, 1);
    const char *IPpattern = "^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)(\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)){3}$";
    int success = regcomp(&ipRegex, IPpattern, REG_EXTENDED | REG_ICASE);
    assert(success == 0);

    if (fplog == NULL) {
        perror("Error to fplog");
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("[Server] Error in connection.\n");
        exit(1);
    }
    char *tmplog = "[Server] Socket is created.\n";
    strncpy(log, tmplog, strlen(tmplog) + 1);
    fwrite(log, 1, strlen(log) + 1, fplog);
    printf("%s", log);

    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if ((bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr))) < 0) {
        perror("[Server] Error in binding.\n");
        exit(1);
    }
    memset(log, '\0', sizeof(log));
    snprintf(log, sizeof(log), "[Server] Bind to port %d\n", PORT);
    fwrite(log, 1, strlen(log) + 1, fplog);
    printf("%s", log);

    if (listen(sockfd, 10) == 0) {
        memset(log, '\0', sizeof(log));
        snprintf(log, sizeof(log), "[Server] Listen on port %s:%d\n", inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));
        fwrite(log, 1, strlen(log) + 1, fplog);
        printf("%s", log);
    } else {
        perror("[Server] Error in binding.\n");
    }

    while (1) {
        newSocket = accept(sockfd, (struct sockaddr *)&newAddr, &addr_size);
        if (newSocket < 0) {
            exit(1);
        }
        printf("[Server] Connection accepted from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));

        if ((childpid = fork()) == 0) {
            close(sockfd);

            while (1) {
                recv(newSocket, buffer, sizeof(buffer), 0);
                if (strcmp(buffer, ":exit") == 0) {
                    printf("[Server] Disconnected from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
                    break;
                } else if (strcmp(buffer, ":file") == 0) {
                    printf("[Server] Reciving the file form %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
                    memset(buffer, '\0', sizeof(buffer));
                    while (1) {
                        recv(newSocket, buffer, sizeof(buffer), 0);
                        if (strcmp(buffer, ":exitfile") == 0) {
                            printf("[Server] Exit file mode.\n");
                            memset(buffer, '\0', sizeof(buffer));
                            break;
                        } else {
                            bool ipresult = ipreg(buffer, ipRegex);
                            printf("\t-> [Client] %s %s\n", buffer, ipresult ? "True" : "False");
                            char match[10] = {'\0'};
                            snprintf(match, sizeof(match), "%s", ipresult ? "True" : "False");
                            send(newSocket, match, sizeof(match), 0);
                            memset(buffer, '\0', sizeof(buffer));
                        }
                    }
                    regfree(&ipRegex);
                    memset(buffer, '\0', sizeof(buffer));
                } else {
                    printf("[Client] %s\n", buffer);
                    send(newSocket, buffer, strlen(buffer) + 1, 0);
                    memset(buffer, '\0', sizeof(buffer));
                }
            }
        }
    }
    fclose(fplog);
    close(newSocket);

    return 0;
}
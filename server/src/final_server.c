#include "config.h"
#include "final_api.h"
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <threads.h>
#include <unistd.h>

volatile int server_running = 1;
#define MAX_CLIENTS 10
pthread_t thread_handles[MAX_CLIENTS];
int thread_count = 0;

void *handle_client(void *client_socket) {
    printf("Client connected, current thread count: %d\n", thread_count);
    int sock = *(int *)client_socket;
    char buffer[1024];

    memset(buffer, 0, 1024);
    read(sock, buffer, 1024);

    char response[1024];
    printf("request buffer %s\n", buffer);
    request_handler(buffer, response);
    printf("Response: %s\n", response);
    write(sock, response, strlen(response));

    sleep(1);
    close(sock);
    free(client_socket);
    printf("Closing client, current thread count: %d\n", thread_count);
    return NULL;
}

void *command_listener(void *param) {
    puts("Type 'stop' or 's' to stop the server\n");
    char command[10];
    while (server_running == 1) {
        scanf("%9s", command);
        if (strcmp(command, "stop") == 0 || strcmp(command, "s") == 0) {
            puts("Stopping server from manual command\n");
            server_running = 0;
            break;
        }
    }
    return NULL;
}

int start_server() {

    int sockfd;
    socklen_t clilen;
    char buffer[1024] = {0};
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("ERROR opening socket");
        exit(1);
    }
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(SERVER_PORT);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    pthread_t command_thread;
    int test = 0;
    if (pthread_create(&command_thread, NULL, command_listener, (void *)&test) != 0) {
        perror("Failed to create command thread");
        exit(1);
    }
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags < 0) {
        perror("Could not get flags for sockfd");
        exit(EXIT_FAILURE);
    }

    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("Could not set sockfd to non-blocking");
        exit(EXIT_FAILURE);
    }
    while (server_running == 1) {
        // printf("Waiting for client...\n");
        if (thread_count >= MAX_CLIENTS) {
            printf("To many clients, exiting looop and joining threads");
            break;
        }
        int *newsockfd = malloc(sizeof(int));

        *newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

        if (*newsockfd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(1000000);
                free(newsockfd);
                continue;
            } else {
                perror("ERROR on accept");
                free(newsockfd);
                break;
            }
        }

        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client, (void *)newsockfd) != 0) {
            perror("Thread creation failed");
            close(*newsockfd);
            free(newsockfd);
        }

        thread_handles[thread_count] = thread;
        thread_count++;
    }

    server_running = 0;
    pthread_join(command_thread, NULL);
    puts("Command Thread has been joined");
    for (int i = 0; i < thread_count; i++) {

        printf("Trying to join thread for client %d\n", i);
        pthread_join(thread_handles[i], NULL);
        printf("Joined thread for client %d\n", i);
    }
    close(sockfd);
    return 0;
}

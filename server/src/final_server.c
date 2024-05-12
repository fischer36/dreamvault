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

#define MAX_CLIENTS 10
volatile int server_running = 1;

struct Client {
    pthread_t thread;
    int socket;
    int is_active;
};

void *handle_client(void *client_ptr) {
    struct Client *client = (struct Client *)client_ptr;
    char buffer[1024];
    memset(buffer, 0, 1024);
    read(client->socket, buffer, 1024);
    char response[1024];
    printf("request buffer %s\n", buffer);
    request_handler(buffer, response);
    printf("Response: %s\n", response);
    write(client->socket, response, strlen(response));
    sleep(1);
    client->is_active = 0;
    close(client->socket);
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

    struct Client clients[MAX_CLIENTS] = {0};

    for (int i = 0; i < MAX_CLIENTS; i++) {
        struct Client client = {
            .thread = -1,
            .socket = -1,
            .is_active = 0,
        };
        clients[i] = client;
    }
    while (server_running == 1) {
        int newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(1000000);
                continue;
            } else {
                perror("ERROR on accept");
                break;
            }
        }

        int found_replacement = 0;
        while (found_replacement == 0) {
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].is_active == 0) {
                    printf("Found inactive client index: %d\n", i);
                    if (clients[i].thread != -1) {
                        puts("Joining inactive client thread\n");
                        pthread_join(clients[i].thread, NULL);
                    }
                    clients[i].is_active = 1;
                    clients[i].socket = newsockfd;
                    pthread_t thread;
                    if (pthread_create(&thread, NULL, handle_client, (void *)&clients[i]) != 0) {
                        perror("Thread creation failed");
                        exit(1);
                    }
                    clients[i].thread = thread;
                    found_replacement = 1;
                    break;
                }
            }
            sleep(1);
        }
    }
    server_running = 0;
    pthread_join(command_thread, NULL);
    puts("Command Thread has been joined");
    for (int i = 0; i < MAX_CLIENTS; i++) {
        struct Client client = clients[i];
        if (client.is_active == 1) {
            if (client.socket != -1) {
                close(client.socket);
            }
            if (client.thread != -1) {
                pthread_join(client.thread, NULL);
                printf("Joined thread for client %d\n", i);
            }
        }
    }

    close(sockfd);
    return 0;
}

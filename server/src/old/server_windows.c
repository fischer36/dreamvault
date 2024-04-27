
#include "config.h"
#include "http_parser.h"
#include <stdio.h>
#include <windows.h>
#include <winsock2.h>
#define MAX_CLIENTS 30

volatile int server_running = 1;

HANDLE threadHandles[MAX_CLIENTS];
int threadCount = 0;

DWORD WINAPI handle_client(LPVOID client_socket)
{
    puts("Thread started.");
    SOCKET clientSock = *(SOCKET *)client_socket;
    char client_message[1024];
    int recv_size;

    while ((recv_size = recv(clientSock, client_message, 1024, 0)) > 0)
    {
        client_message[recv_size] = '\0';
        HttpRequest request;
        char response[1024];
        if (api_request_handler(client_message, response) == 0)
        {

            printf("Successful Request");
        }

        if (response != NULL)
        {
            Sleep(1000);
            send(clientSock, response, strlen(response), 0);
        }
        else
        {
            send(clientSock, client_message, recv_size, 0);
        }

        break;
    }

    if (recv_size == 0)
    {
        puts("Client disconnected.");
    }
    else if (recv_size == -1)
    {
        printf("recv failed with error code: %d\n", WSAGetLastError());
    }

    printf("Closing client socket.\n");
    closesocket(clientSock);
    free(client_socket);
    return 0;
}

DWORD WINAPI command_listener(LPVOID param)
{
    char command[10];
    printf("Type 'stop' or 's' to shut down the server.\n");
    while (1)
    {
        scanf("%s", command);
        if (strcmp(command, "stop") == 0 || strcmp(command, "s") == 0)
        {
            printf("Shutting down server.\n");
            server_running = 0;
            break;
        }
    }
    return 0;
}
int start_server_win()
{
    WSADATA wsaData;
    SOCKET serverSocket, clientSocket;
    struct sockaddr_in server, client;
    int c;

    WSAStartup(MAKEWORD(2, 2), &wsaData);
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(SERVER_IP);
    server.sin_port = htons(SERVER_PORT);
    bind(serverSocket, (struct sockaddr *)&server, sizeof(server));
    listen(serverSocket, 3);

    u_long mode = 1;
    ioctlsocket(serverSocket, FIONBIO, &mode);

    int dotCount = 0;
    while (server_running)
    {
        c = sizeof(struct sockaddr_in);
        clientSocket = accept(serverSocket, (struct sockaddr *)&client, &c);

        if (clientSocket == INVALID_SOCKET)
        {
            if (WSAGetLastError() == WSAEWOULDBLOCK)
            {

                char dots[4] = "";
                for (int i = 0; i < dotCount + 1; i++)
                {
                    dots[i] = '.';
                }
                dots[dotCount + 1] = '\0';

                printf("\rWaiting for connection%s   ", dots);
                fflush(stdout);

                dotCount = (dotCount + 1) % 3;
                Sleep(1000);
            }
            else
            {
                Sleep(1000);
                printf("Accept failed with error code: %d\n",
                       WSAGetLastError());
            }
        }
        else
        {
            puts("Client connected");
            if (threadCount < MAX_CLIENTS)
            {
                SOCKET *threadSocket = malloc(sizeof(SOCKET));
                if (threadSocket == NULL)
                {
                    closesocket(clientSocket);
                }
                else
                {
                    printf("Creating Thread %d\n", threadCount + 1);
                    *threadSocket = clientSocket;
                    threadHandles[threadCount++] = CreateThread(
                        NULL, 0, handle_client, threadSocket, 0, NULL);
                }
            }
            else
            {
                printf("Max client limit reached.\n");
            }
        }
    }

    puts("Server Loop Broken. Closing threads and shutting down.");
    for (int i = 0; i < threadCount; i++)
    {
        WaitForSingleObject(threadHandles[i], INFINITE);
        CloseHandle(threadHandles[i]);
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}

#include "config.h"

int start_server()
{
    HANDLE commandThread =
        CreateThread(NULL, 0, command_listener, NULL, 0, NULL);
    if (commandThread == NULL)
    {
        printf("Failed to create command listener thread.\n");
        return 1;
    }

    start_server_win();

    WaitForSingleObject(commandThread, INFINITE);
    CloseHandle(commandThread);

    return 0;
}

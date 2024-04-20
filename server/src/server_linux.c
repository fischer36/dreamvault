// #include "api.h"
#include "api_xd.h"
#include "config.h"
#include "db_testing.h"
#include "sys.h"
#include "task_testing.h"
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

#define MAX_CLIENTS 3
pthread_t thread_handles[MAX_CLIENTS];
int thread_count = 0;

int ok(char buffer[1024]) {
  // int i = get_page_counter();
  // return 0;
  struct HTTP_RESPONSE responsexd2 = t_user_page_write(36, buffer);
  printf("%s", responsexd2.body);
  return 0;
  // struct HTTP_RESPONSE responsexd2 = t_register(buffer);
  printf("%s", responsexd2.body);
  return 0;
  char *uri_start = strstr(buffer, " ");
  uri_start += 1;
  char *uri_end = strstr(uri_start, " ");
  char uri_str[128];
  strncpy(uri_str, uri_start, uri_end - uri_start);
  uri_str[uri_end - uri_start] = '\0';
  printf("URI: %s\n", uri_str);

  one_slash(buffer, uri_str);
  return 0;
  /*
   * - register (post) ✔
   * - unregister (post) ✔
   * - login (post ) ✔
   * - logout (post) ✔
   * -
   * - users/id (get)
   */

  int slash_count = 0;
  for (int i = 0; i < strlen(uri_str); i++) {
    if (uri_str[i] == '/') {
      slash_count++;
    }
  }
  printf("SLASH COUNT %d\n", slash_count);

  if (strcmp(uri_str, "/register") == 0) {
  }

  if (strcmp(uri_str, "/unregister") == 0) {
  }

  if (strcmp(uri_str, "/login") == 0) {
  }

  if (strcmp(uri_str, "/logout") == 0) {
  }

  if (strncmp(uri_str, "/users/", 7) == 0) {
    if (strlen(uri_str) > 7) {
      char *user_id_start = uri_str + 7;
      char user_id_str[128];
      char *user_id_end = strstr(user_id_start, "/");

      if (user_id_end == NULL) {
        if (strlen(uri_str) <= user_id_start - uri_str) {
          return -1;
        }
        strcpy(user_id_str, user_id_start);
      } else {
        strncpy(user_id_str, user_id_start, user_id_end - user_id_start);
        user_id_str[user_id_end - user_id_start] = '\0';
      }

      printf("User ID: %s\n", user_id_str);
    } else {
      return -1;
    }
  }

  // printf("%s", buffer);
  return 0;
}
void *handle_client(void *client_socket) {
  printf("Client connected, current thread count: %d\n", thread_count);
  int sock = *(int *)client_socket;
  char buffer[1024];

  memset(buffer, 0, 1024);
  read(sock, buffer, 1024);
  printf("Client: %s\n", buffer);
  char response[1024];
  ok(buffer);
  // api_request_handler(buffer, response);
  write(sock, "Hello, client!", 14);

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

  pthread_t command_thread;
  int test = 0;
  if (pthread_create(&command_thread, NULL, command_listener, (void *)&test) !=
      0) {
    perror("Failed to create command thread");
    exit(1);
  }
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

int main() {
  // if (sys_file_exists("./hello/ok.txt")) {
  // } else {
  // }
  // int i = get_page_counter();
  start_server();
  //

  return 0;
}

#include "final_api.h"
// #include "final_http.h"
// #include "final_task.h"
// #include "final_db.h"
#include "final_db.h"
#include "final_server.c"
#include <stdio.h>
#include <string.h>
int main() {
    int user_id = 0;
    char *token = "d7118ed59ab3b8f263dbdd54596a7d83";
    char *sample_request = "PATCH /users/35/pages/41 HTTP/1.1\r\n"
                           "Host: localhost\r\n"
                           "Content-Type: application/json\r\n"
                           "Content-Length: 0\r\n"
                           "Authentication: d7118ed59ab3b8f263dbdd54596a7d83\r\n"
                           "\r\n"
                           "email: sample@gmail.com\r\n"
                           "password: samplePass123\r\n"
                           "page_title: MODIFIEDage\r\n"
                           "page_body: MODIFIEDontent\r\n";

    // printf("Request: %s\n", sample_request);
    // char result[512] = {0};
    // if (get_value(sample_request, "Authentication", result, TOKEN_LENGTH)) {
    //     printf("Error");
    // } else {
    //     printf("Result: %s %lu\n", result, strlen(result));
    // }
    // return 0;
    char response[1024] = {0};
    start_server();
    // request_handler(sample_request, response);
    printf("Response: %s\n", response);
    return 0;
}

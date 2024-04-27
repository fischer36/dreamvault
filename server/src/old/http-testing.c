#include "http_xd.h"
// #include "task_testing.h"
#include <stdio.h>

int main() {
  char *httpRequest = "GET /users/12/pages/12 HTTP/1.1\r\n"
                      "Host: www.example.com\r\n"
                      "Connection: close\r\n"
                      "\r\n"
                      "Body {\r\n"
                      "heöllo\r\n"
                      "}";
  // printf("%s\n", httpRequest);
  parse(httpRequest);
  return 0;
}

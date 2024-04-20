
#include "http_xd.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum UriType { USER, PAGE, LOGIN, LOGOUT, REGISTER, UNREGISTER, INVALID };

union Uri {
  struct {
    // /User/<user_id>
    int user_id;
  } User;

  struct {
    // /User/<user_id>/page/<page_id>
    int user_id;
    int page_id;
  } Page;
};

int extract_body(const char *buffer, char **body) {
  const char *start = buffer;
  const char *body_start = strstr(start, "\r\n\r\n");

  if (!body_start)
    return 1;

  body_start += 4;
  size_t body_length = strlen(body_start);
  *body = (char *)malloc(body_length + 1);
  if (!*body) {
    printf("Memory allocation for the body failed.\n");
    return -1;
  }

  strncpy(*body, body_start, body_length);
  (*body)[body_length] = '\0';

  return 0;
}


#include "http_xd.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#include "api_xd.h"
#include "http_xd.h"
#include "stdio.h"
#include "task_testing.h"
#include <stdlib.h>
#include <string.h>
static int search_value(const char *haystack, const char *search_key,
                        char result[215]) {

  char *key_index = strstr(haystack, search_key);
  if (!key_index) {
    return -1;
  }
  char *value_start = key_index + strlen(search_key);
  char *value_end = strchr(value_start, '"');
  if (value_end == NULL)
    return -1;
  if ((value_end - value_start) > 215)
    return -1;

  strncpy(result, value_start, value_end - value_start);
  result[value_end - value_start] = '\0';
  return 0;
}

int one_slash(char buffer[1024], char *uri_str) {
  struct HTTP_RESPONSE response;
  if (strcmp(uri_str, "/register") == 0) {
    char *body;
    if (extract_body(buffer, &body))
      return -1;

    const int MAX_EMAIL_LENGTH, MAX_PASSWORD_LENGTH = 15;
    const int MIN_EMAIL_LENGTH, MIN_PASSWORD_LENGTH = 4;
    char email[215];
    char password[215];
    if (search_value(body, "email", email) == 0 &&
        search_value(body, "password", password) == 0) {

      if (strlen(email) < MAX_EMAIL_LENGTH &&
          strlen(email) > MIN_EMAIL_LENGTH &&
          strlen(password) < MAX_EMAIL_LENGTH &&
          strlen(password) > MIN_PASSWORD_LENGTH &&
          strchr(email, '@') != NULL) {

        // Try to create user
        response = t_register(email);
      }
    }
    free(body);
  }

  if (strcmp(uri_str, "/unregister") == 0) {
  }

  if (strcmp(uri_str, "/login") == 0) {
  }

  if (strcmp(uri_str, "/logout") == 0) {
  }

  puts("[one_slash] invalid URI");
  return -1;
}

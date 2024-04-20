
#include "http_xd.h"
#include "http_parser.h"
// #include "task_testing.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static enum MethodType get_method(const char buffer[1024]) {
  const char *method_start = buffer;
  const char *method_end = strstr(buffer, " ");
  int method_length = method_end - method_start;

  if (method_length == 0 || method_length >= 10 || method_start == NULL) {
    return M_INVALID;
  }

  char method_str[10];
  strncpy(method_str, method_start, method_length);
  method_str[method_length] = '\0';
  printf("Method %s\n", method_str);

  if (strcmp(method_str, "GET") == 0) {
    return M_GET;
  } else if (strcmp(method_str, "POST") == 0) {
    return M_POST;
  } else if (strcmp(method_str, "PATCH") == 0) {
    return M_PATCH;
  } else if (strcmp(method_str, "DELETE") == 0) {
    return M_DELETE;
  }

  return M_INVALID;
}

static enum UriType get_uri(const char request[1024], union Uri *uri) {

  char *uri_start = strstr(request, " ");
  uri_start += 1;
  char *uri_end = strstr(uri_start, " ");
  char uri_str[512];
  strncpy(uri_str, uri_start, uri_end - uri_start);
  uri_str[uri_end - uri_start] = '\0';

  int slash_count = 0;
  for (int i = 0; i < uri_end - uri_start; i++) {
    if (uri_str[i] == '/') {
      slash_count++;
    }
  }

  if (slash_count == 1) {
    if (strcmp(uri_str, "/register") == 0) {
      strcpy(uri->Valid, "/register");
      uri->Valid[strlen("/register")] = '\0';
      return U_VALID;

    } else if (strcmp(uri_str, "/unregister") == 0) {
      strcpy(uri->Valid, "/unregister");
      uri->Valid[strlen("/unregister")] = '\0';
      return U_VALID;

    } else if (strcmp(uri_str, "/login") == 0) {
      strcpy(uri->Valid, "/login");
      uri->Valid[strlen("/login")] = '\0';
      return U_VALID;

    } else if (strcmp(uri_str, "/logout") == 0) {
      strcpy(uri->Valid, "/logout");
      uri->Valid[strlen("/logout")] = '\0';
      return U_VALID;
    }
  }

  if (slash_count == 2) {
    if (strncmp(uri_str, "/users/", 7) == 0 && strlen(uri_str) > 7) {
      const char *user_id = uri_str + 7;
      printf("user_id: %s\n", user_id);
      uri->User.user_id = atoi(user_id);
      return U_USER;
    }
  }

  if (slash_count == 4) {
    int user_id, page_id;
    if (sscanf(uri_str, "/users/%d/pages/%d", &user_id, &page_id) == 2) {
      printf("User ID: %d\n", user_id);
      printf("Page ID: %d\n", page_id);
      uri->Page.user_id = user_id;
      uri->Page.page_id = page_id;
      return U_USER_PAGE;
    }
  }

  strcpy(uri->Invalid, "Invalid URI\n");
  uri->Invalid[strlen("Invalid URI\n")] = '\0';
  return U_INVALID;
}

// enum UriType get_uri(const char buffer[1024], union Uri *uri) {
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

int parse(char request[1024]) {

  enum MethodType method = get_method(request);
  if (method == M_INVALID) {
    return -1;
  }

  union Uri uri;
  enum UriType uri_type = get_uri(request, &uri);

  if (uri_type == U_INVALID) {
    printf("Invalid %s\n", uri.Invalid);
    return -1;
  }

  switch (method) {
  case M_POST:
    printf("post\n");
    break;
  case M_GET:
    printf("get\n");
    break;
  case M_PATCH:
    printf("patch\n");
    break;
  case M_DELETE:
    printf("delete\n");
    break;
  case M_INVALID:
    printf("invalid method");
    return -1;
  }
  switch (uri_type) {
  case U_INVALID:
    printf("Invalid URI %s\n", uri.Invalid);
    return -1;
  case U_VALID:
    printf("Valid %s\n", uri.Valid);
    break;
  case U_USER:
    printf("User %d\n", uri.User.user_id);
    break;
  case U_USER_PAGE:
    printf("User %d Page %d\n", uri.Page.user_id, uri.Page.page_id);
    break;
  }
  return 0;
  // if (uri_type == U_INVALID) {
  //   printf("Invalid %s\n", uri.Invalid);
  //   return -1;
  // } else if (uri_type == U_USER) {
  //   printf("User %d\n", uri.User.user_id);
  // } else if (uri_type == U_USER_PAGE) {
  //   printf("User %d Page %d\n", uri.Page.user_id, uri.Page.page_id);
  //   switch (method) {
  //   case M_POST:
  //     t_page_create(request);
  //     break;
  //   case M_GET:
  //     t_page_read(uri.Page.page_id, request);
  //     break;
  //   case M_PATCH:
  //     t_page_write(uri.Page.page_id, request);
  //     break;
  //   case M_DELETE:
  //     t_page_delete(uri.Page.page_id, request);
  //     break;
  //   case M_INVALID:
  //     return -1;
  //   }
  // } else if (uri_type == U_VALID) {
  //   if (strcmp(uri.Valid, "/register") == 0 && method == M_POST) {
  //     t_register(request);
  //     return 0;
  //   } else if (strcmp(uri.Valid, "/unregister") == 0 && method == M_POST) {
  //     t_unregister(request);
  //     return 0;
  //   } else if (strcmp(uri.Valid, "/login") == 0 && method == M_POST) {
  //     t_login(request);
  //     return 0;
  //   } else if (strcmp(uri.Valid, "/logout") == 0 && method == M_POST) {
  //     t_logout(request);
  //     return 0;
  //   }
  // }
  return -1;
}

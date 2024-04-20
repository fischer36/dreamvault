#pragma once
#include <string.h>
/*
 * /register
 * /unregister
 * /login
 * /logout
 * /users/<user_id>
 * /users/<user_id>/page/<page_id>
 *
 *
 * */

enum MethodType { M_GET, M_POST, M_DELETE, M_PATCH, M_INVALID };

enum UriType { U_USER, U_USER_PAGE, U_VALID, U_INVALID };
union Uri {

  struct {
    int user_id;         // /user/<user_id>
    char following[512]; // users/<user_id>/<following>
  } User;

  struct {
    int user_id;         // /users/<user_id>
    int page_id;         // /users/<user_id>/page/<page_id>
    char following[512]; // users/<user_id>/page/<page_id>/<following>
  } Page;

  char Valid[512]; // register, unregister, login, logout.

  char Invalid[512]; // Everything that has no corresponding function, this
                     // contains error message
};

#include "stdio.h"
static enum UriType two_slash(const char uri_str[512], union Uri *uri);
static enum UriType one_slash(const char uri_str[512], union Uri *uri);

// enum MethodType get_method(const char buffer[1024]);
// enum UriType get_uri(const char buffer[1024], union Uri *uri);

int extract_body(const char *buffer, char **body);

int parse(char request[1024]);

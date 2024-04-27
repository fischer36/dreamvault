#include "api.h"
#include "db.h"
#include "worker.h"
#include <stdio.h>
#include <string.h>

// int ok(char buffer[1024]) {
//
//   struct HTTP_RESPONSE responsexd2 = t_login(buffer);
//   printf("%s", responsexd2.body);
//   char *resp_buff = NULL;
//   parse_http_response(responsexd2, &resp_buff);
//   if (resp_buff != NULL) {
//     printf("%s", resp_buff);
//     free(resp_buff);
//   } else {
//   }
//   return 0;
//   // struct HTTP_RESPONSE responsexd2 = t_register(buffer);
//   printf("%s", responsexd2.body);
//   return 0;
//   char *uri_start = strstr(buffer, " ");
//   uri_start += 1;
//   char *uri_end = strstr(uri_start, " ");
//   char uri_str[128];
//   strncpy(uri_str, uri_start, uri_end - uri_start);
//   uri_str[uri_end - uri_start] = '\0';
//   printf("URI: %s\n", uri_str);
//
//   one_slash(buffer, uri_str);
//   return 0;
//   /*
//    * - register (post) ✔
//    * - unregister (post) ✔
//    * - login (post ) ✔
//    * - logout (post) ✔
//    * -
//    * - users/id (get)
//    */
//
//   int slash_count = 0;
//   for (int i = 0; i < strlen(uri_str); i++) {
//     if (uri_str[i] == '/') {
//       slash_count++;
//     }
//   }
//   printf("SLASH COUNT %d\n", slash_count);
//
//   if (strcmp(uri_str, "/register") == 0) {
//   }
//
//   if (strcmp(uri_str, "/unregister") == 0) {
//   }
//
//   if (strcmp(uri_str, "/login") == 0) {
//   }
//
//   if (strcmp(uri_str, "/logout") == 0) {
//   }
//
//   if (strncmp(uri_str, "/users/", 7) == 0) {
//     if (strlen(uri_str) > 7) {
//       char *user_id_start = uri_str + 7;
//       char user_id_str[128];
//       char *user_id_end = strstr(user_id_start, "/");
//
//       if (user_id_end == NULL) {
//         if (strlen(uri_str) <= user_id_start - uri_str) {
//           return -1;
//         }
//         strcpy(user_id_str, user_id_start);
//       } else {
//         strncpy(user_id_str, user_id_start, user_id_end - user_id_start);
//         user_id_str[user_id_end - user_id_start] = '\0';
//       }
//
//       printf("User ID: %s\n", user_id_str);
//     } else {
//       return -1;
//     }
//   }
//
//   // printf("%s", buffer);
//   return 0;
// }
/*
Handles request with uri /register
*/
static int uri_register(HttpRequest request) {
  printf("uri_register - Deciding Task\n");
  HttpResponse response;
  HttpResponseCode code;
  switch (request.method) {
  case METHOD_POST: // CREATE ACC
    printf("uri_register - Executing Task: /Register POST\n");
    code = worker_task_register_post(request, &response);

    break;
  case METHOD_DELETE: // DELETE ACC
    printf("uri_register - Executing Task: /Register DELETE\n");
    code = worker_task_register_delete(request, &response);
    break;
  default:
    printf("uri_register - No Task Executed: Invalid Method\n");
    break;
  }
  if (code == HTTP_OK) {
    printf("CODE OK\n");
  } else {
    printf("CODE NOT OK\n");
  }
  return 0;
}

/*
Handles request with uri /user, /user/vault.
*/
static int uri_user(HttpRequest request, HttpResponse *response_new) {

  HttpResponse response;
  printf("uri_user - Deciding Task\n");

  if (strlen(request.uri_children) <= 1) {
    printf("uri_user no child uri found\n");
    switch (request.method) {
    case METHOD_POST: // LOGIN
      printf("uri_user - Executing Task: /User POST\n");

      if (worker_task_user_post(request, &response) == HTTP_OK) {
        response_new->body = response.body;
        response_new->body_size = response.body_size;
        response_new->code = HTTP_OK;
        return 0;
      }
      return 0;
      break;
    case METHOD_DELETE: // LOGOUT
      printf("uri_user - Executing Task: /User DELETE\n");
      worker_task_user_delete(request, &response);
      return 0;
      break;
    default:
      printf("uri_user - No Task Executed: Invalid Method\n");
      return -1;
      break;
    }
    return 0;
  }

  if (strncmp(request.uri_children, "/vault", strlen("/vault")) == 0) {

    if (strcmp(request.uri_children, "/vault") == 0) {
      printf("user/vault\n");
      char vault_name[128];

      if (request.method == METHOD_POST) {
        ;
        if (parse_search_body(request.body, "vault_name", vault_name, 127) !=
            0) {
          response.code = HTTP_BAD_REQUEST;
          return 1;
        } else {
          printf("Attempting to create vault %s\n", vault_name);
          MYSQL *db_connection = NULL;
          if (db_connection_get(&db_connection) != 0) {
            printf("Failed to connect to database\n");
            return 1;
          }

          int user_id = 0;
          if (worker_validate_token(request, &user_id) != 0) {
            printf("Failed to verify token\n");
            db_connection_close(db_connection);
            return 1;
          }
          if (db_vault_create(db_connection, user_id, vault_name) != 0) {
            printf("Failed to create vault\n");
            db_connection_close(db_connection);
            return 1;
          }

          db_connection_close(db_connection);
        }
        return 0;
      } else if (request.method == METHOD_GET) {
        printf("Attempting to get all vaults %s\n", vault_name);
        MYSQL *db_connection = NULL;
        if (db_connection_get(&db_connection) != 0) {
          printf("Failed to connect to database\n");
          return 1;
        }
        int user_id = 0;
        if (worker_validate_token(request, &user_id) != 0) {
          printf("Failed to verify token\n");
          db_connection_close(db_connection);
          return 1;
        }
        int *vault_ids;
        int vault_count = 0;
        printf("User/Vault/Child Post\n");
        if (db_user_get_vaults(db_connection, user_id, &vault_ids,
                               &vault_count) != 0) {
          printf("Failed to get vaults\n");
          db_connection_close(db_connection);
          return 1;
        } else {
          printf("Got %d vaults\n", vault_count);
          for (int i = 0; i < vault_count; i++) {
            printf("vault_id: %d\n", vault_ids[i]);
          }
          free(vault_ids);
          db_connection_close(db_connection);
          return 1;
        }
      }
    } else if (strncmp(request.uri_children, "/vault/", 7) == 0) {

      printf("User/Vault/Child\n");
      int vault_id = 0;
      if (sscanf(request.uri_children, "/vault/%d", &vault_id) == 1) {
        printf("Extracted vault_id: %d\n", vault_id);
        MYSQL *db_connection = NULL;
        if (db_connection_get(&db_connection) != 0) {
          printf("Failed to connect to database\n");
          return 1;
        }
        int user_id = 0;
        if (worker_validate_token(request, &user_id) != 0) {
          printf("Failed to verify token\n");
          db_connection_close(db_connection);
          return 1;
        }

        if (request.method == METHOD_GET) {

          printf("User/Vault/Child GET\n");
          Vault vault = {0};
          if (db_vault_get(db_connection, vault_id, &vault) != 0) {
            printf("Failed to get vault\n");
            db_connection_close(db_connection);
            return 1;
          }
          if (vault.owner_id != user_id) {
            printf("User does not own vault\n");
            db_connection_close(db_connection);
            return 1;
          }
          Page *pages_list = NULL;
          int pages_count = 0;
          if (db_pages_in_vault(db_connection, vault_id, &pages_list,
                                &pages_count) == 0) {
            if (pages_count > 0 && pages_list != NULL) {
              printf("Pages count: %d\n", pages_count);
              for (int i = 0; i < pages_count; i++) {
                printf("Page id: %d\n", pages_list[i].page_id);
              }
              free(pages_list);
              db_connection_close(db_connection);
              return 0;
            }

            db_connection_close(db_connection);
            return 1;
          }
          db_vault_delete(db_connection, vault_id);
        } else if (request.method == METHOD_DELETE) {

          printf("User/Vault/Child DELETE\n");
          Vault vault = {0};
          if (db_vault_get(db_connection, vault_id, &vault) != 0) {
            printf("Failed to get vault\n");
            db_connection_close(db_connection);
            return 1;
          }
          if (vault.owner_id != user_id) {
            printf("User does not own vault\n");
            db_connection_close(db_connection);
            return 1;
          }
          db_vault_delete(db_connection, vault_id);
        }
        db_connection_close(db_connection);
        return 0;
      }
    }
  }
  printf("User/ - No Task Executed: Invalid URI or metho9d\n");
  response.code = HTTP_BAD_REQUEST;

  return 1;
}

/*
Handles request with uri /page
*/
static int uri_page(HttpRequest request) {
  printf("uri_page - Deciding Task\n");
  HttpResponse response;
  char token[33];
  switch (request.method) {
  case METHOD_POST: // Create Page
    printf("uri_page - Executing Task: /Page POST\n");

    worker_task_page_post(request, &response);
    return 0;
    break;
  case METHOD_DELETE: // Delete Page
    worker_task_page_delete(request, &response);
    printf("uri_page - Executing Task: /Page DELETE\n");
    break;
  case METHOD_GET: // Read Page
    printf("uri_page - Executing Task: /Page GET\n");
    if (worker_task_page_get(request, &response) == HTTP_OK) {
      printf("PAGE %s\n", response.body);
      free(response.body);
    }
    break;
  case METHOD_PATCH: // Write Page
    printf("uri_page - Executing Task: /Page PATCH\n");
    printf("reuqest uri_children: %s\n", request.uri_children);
    int page_id = 0;
    if (sscanf(request.uri_children, "/%d", &page_id) != 1) {
      printf("unable to parse page_id: \n");
      return 1;
    }

    worker_task_page_patch(request, page_id, &response);
    break;
  default:
    printf("uri_page - No Task Executed: Invalid Method\n");
    break;
  }

  return 0;
}

/*
@request - http response struct. Predicates what task is in order.
@response - http response struct. Will define based on status.
*/
static int decide_task(const HttpRequest request, HttpResponse *response) {
  switch (request.uri) {
  case URI_REGISTER:
    printf("URI REGISTER\n");
    if (uri_register(request) != 0) {
      printf("URI REGISTER FAIL\n");
    } else {
      printf("URI REGISTER SUCESS\n");
    }
    break;
  case URI_USER:
    printf("URI USER\n");
    if (uri_user(request, response) != 0) {
      printf("URI USER FAIL\n");
    } else {
      printf("URI USER SUCESS\n");
    }
    break;
  case URI_PAGE:
    printf("URI PAGE\n");
    if (uri_page(request) != 0) {
      printf("URI PAGE FAIL\n");
    } else {
      printf("URI PAGE SUCESS\n");
    }
    break;
  default:
    printf("URI UNKNOWN\n");
    return -1;
  }
  return 0;
}

/*
@source - http request in bytes
@target - http response in bytes
*/
int api_request_handler(const char *source, char *target) {

  HttpRequest request;
  request.body = NULL;
  request.headers_array = NULL;
  request.header_count = 0;

  /*
  turn buffer into HttpRequest. if return value is non-zero no dynamic
  memory needs to be freed as nothing was allocated
  */
  if (parse_request(source, &request) != 0) {
    printf("api_request_handler [error]- Failed to parse request\n");
    return -1;
  }

  HttpResponse response;
  response.body = NULL;

  response.header_count = 0;
  response.body_size = 0;
  int response_bytes_size = 0;

  // Decide and handle task. Fill out the response struct
  if (decide_task(request, &response) != 0) {
    // printf("decide_task [error]- Failed to decide task\n");
  }

  // Parse the response struct into target
  parse_response(response, target, &response_bytes_size);

  // Delete request & response struct
  parse_delete_request(request);
  parse_delete_response(response);
  return 0;
}

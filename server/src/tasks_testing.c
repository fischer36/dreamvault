#include "db_testing.h"
#include "http_parser.h"
#include "http_xd.h"
#include "sys.h"
#include "task_testing.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static int search_token(const char *haystack, const char *search_key,
                        char result[32]) {

  char *key = strstr(haystack, search_key);
  if (!key) {
    return -1;
  }

  char *value_start = key + strlen(search_key);
  // char *value_end = strchr(value_start, '';
  //
  // while (!isspace((unsigned char)*value_end) && *value_end != '"' &&
  //        *value_end != '\0') {
  //   value_end++;
  // }
  //
  // int value_length = value_end - value_start;
  // if (value_length <= 0 || value_length > 215) {
  //   printf("Error: Not FOund! - Invalid value length\n");
  //   return -1;
  // }
  //
  strcpy(result, value_start);
  printf("KEY FOUND - %s\n", result);
  result[32] = '\0';
  return 0;
}

static int search_value(const char *haystack, const char *search_key,
                        char result[215]) {

  char *key = strstr(haystack, search_key);
  if (!key) {
    return -1;
  }
  char *value_start = key + strlen(search_key);
  while (!isalnum((unsigned char)*value_start) && *value_start != '\0') {
    value_start++;
  }
  char *value_end = value_start;

  while (!isspace((unsigned char)*value_end) && *value_end != '"' &&
         *value_end != '\0') {
    value_end++;
  }

  int value_length = value_end - value_start;
  if (value_length <= 0 || value_length > 215) {
    printf("Error: Not FOund! - Invalid value length\n");
    return -1;
  }

  strncpy(result, value_start, value_length);
  printf("KEY FOUND - %s\n", result);
  result[value_length] = '\0';
  return 0;
}

struct HTTP_RESPONSE t_page_read(int page_id, char buffer[1024]) {

  struct HTTP_RESPONSE response = {
      .code = "",
      .body = "",
      .headers = "",
  };

  strcpy(response.code, "500 Internal Server Error\r\n");

  char token[33] = {0};
  if (search_value(buffer, "Token:", token)) {
    strcpy(response.code, "401 Unauthorized\r\n");
    strcpy(response.body, "No token found");
    return response;
  }

  if (strlen(token) != 32) {
    strcpy(response.code, "401 Unauthorized\r\n");
    strcpy(response.body, "Invalid token ");
    return response;
  }

  int user_id = 0;
  if (validate_token(token, &user_id)) {
    strcpy(response.code, "401 Unauthorized\r\n");
    strcpy(response.body, "Invalid token");
    return response;
  }

  int owner_id = 0;
  int file_size = 0;
  char title[256];
  char file_path[256];

  if (get_page_info(page_id, &owner_id, title, file_path, &file_size)) {
    strcpy(response.code, "500 Internal Server Error\r\n");
    strcpy(response.body, "Failed to get page");
    return response;
  }

  if (user_id != owner_id) {
    strcpy(response.code, "401 Unauthorized\r\n");
    strcpy(response.body, "Unauthorized user for page");
    return response;
  }

  if (!sys_file_exists(file_path)) {
    strcpy(response.code, "500 Internal Server Error\r\n");
    strcpy(response.body, "Error finding page path");
    return response;
  }
  size_t data_size = 0;
  char *data = NULL;
  if (sys_file_read(file_path, &data, &data_size)) {
    strcpy(response.code, "500 Internal Server Error\r\n");
    strcpy(response.body, "Error page from database");
    return response;
  }
  printf("ok DUBB");
  printf("data : %s\n", data);
  if (strlen(data) > 512) {
    printf("Page data is too big, shortening");
    data[512] = '\0';
  }

  strcpy(response.code, "200 OK\r\n");
  strncpy(response.body, data, 512);
  if (data != NULL)
    free(data);
  return response;
}
struct HTTP_RESPONSE t_page_delete(int page_id, char buffer[1024]) {

  struct HTTP_RESPONSE response = {
      .code = "",
      .body = "",
      .headers = "",
  };

  strcpy(response.code, "500 Internal Server Error\r\n");

  char token[33] = {0};
  if (search_value(buffer, "Token:", token)) {
    strcpy(response.code, "401 Unauthorized\r\n");
    strcpy(response.body, "No token found");
    return response;
  }

  if (strlen(token) != 32) {
    strcpy(response.code, "401 Unauthorized\r\n");
    strcpy(response.body, "Invalid token ");
    return response;
  }

  int user_id = 0;
  if (validate_token(token, &user_id)) {
    strcpy(response.code, "401 Unauthorized\r\n");
    strcpy(response.body, "Invalid token");
    return response;
  }

  int owner_id = 0;
  int file_size = 0;
  char title[256];
  char file_path[256];

  if (get_page_info(page_id, &owner_id, title, file_path, &file_size)) {
    strcpy(response.code, "500 Internal Server Error\r\n");
    strcpy(response.body, "Failed to get page");
    return response;
  }

  if (user_id != owner_id) {
    strcpy(response.code, "401 Unauthorized\r\n");
    strcpy(response.body, "Unauthorized user for page");
    return response;
  }

  if (sys_file_exists(file_path)) {
    if (sys_file_delete(file_path)) {
      strcpy(response.code, "500 Internal Server Error\r\n");
      strcpy(response.body, "Error deleting page");
      return response;
    }
  }

  if (delete_page(page_id)) {
    strcpy(response.code, "500 Internal Server Error\r\n");
    strcpy(response.body, "Error page from database");
    return response;
  }

  printf("Ok it wokred");
  strcpy(response.code, "200 OK\r\n");
  strcpy(response.body, "Page deletion successful");
  return response;
}
struct HTTP_RESPONSE t_login(char buffer[1024]) {
  struct HTTP_RESPONSE response = {
      .code = "",
      .body = "",
      .headers = "",
  };
  strcpy(response.code, "500 Internal Server Error\r\n");

  char *body;
  if (extract_body(buffer, &body)) {
    strcpy(response.code, "404 Bad Request\r\n");
    strcpy(response.body, "No request body found");
    return response;
  }
  const int MAX_EMAIL_LENGTH = 25;
  const int MAX_PASSWORD_LENGTH = 25;
  const int MIN_EMAIL_LENGTH = 4;
  const int MIN_PASSWORD_LENGTH = 4;

  char email[215];
  char password[215];
  if (search_value(body, "email:", email) != 0 ||
      search_value(body, "password:", password) != 0) {
    strcpy(response.code, "404 Bad Request\r\n");
    strcpy(response.body, "No email or password found");
    free(body);
    return response;
  }

  free(body);

  if (strlen(email) > MAX_EMAIL_LENGTH || strlen(email) < MIN_EMAIL_LENGTH) {
    strcpy(response.code, "404 Bad Request\r\n");
    strcpy(response.body, "Invalid email");
    return response;
  }

  if (strlen(password) > MAX_PASSWORD_LENGTH ||
      strlen(password) < MIN_PASSWORD_LENGTH) {
    strcpy(response.code, "404 Bad Request\r\n");
    strcpy(response.body, "Invalid password");
    return response;
  }

  int user_id = 0;
  char hashed_password[65] = {0};

  if (get_user_id(email, &user_id, hashed_password) != 0) {
    strcpy(response.code, "404 Not Found\r\n");
    strcpy(response.body, "User not found");
    return response;
  }

  if (util_compare_hash(password, hashed_password) != 0) {
    strcpy(response.code, "401 Unauthorized\r\n");
    strcpy(response.body, "Invalid password");
    return response;
  }

  char token[33] = {0};
  if (util_generate_token(token) != 0) {
    strcpy(response.code, "500 Internal Server Error\r\n");
    strcpy(response.body, "Failed to generate token");
    return response;
  }

  if (insert_session(user_id, token) != 0) {
    strcpy(response.code, "500 Internal Server Error\r\n");
    strcpy(response.body, "Failed to create session");
    return response;
  }

  strcpy(response.code, "200 OK\r\n");
  snprintf(response.headers, sizeof(response.headers), "Token: %s\r\n", token);
  strcpy(response.body, "Login successful");
  return response;
}

struct HTTP_RESPONSE t_logout(char request[1024]) {

  struct HTTP_RESPONSE response = {
      .code = "",
      .body = "",
      .headers = "",
  };

  strcpy(response.code, "500 Internal Server Error\r\n");

  char token[33] = {0};
  if (search_value(request, "Token:", token)) {
    strcpy(response.code, "401 Unauthorized\r\n");
    strcpy(response.body, "No token found");
    return response;
  }

  if (strlen(token) != 32) {
    strcpy(response.code, "401 Unauthorized\r\n");
    strcpy(response.body, "Invalid token ");
    return response;
  }
  if (delete_session(token)) {
    strcpy(response.code, "400 Bad Request\r\n");
    strcpy(response.body, "Invalid token");
    return response;
  }

  strcpy(response.code, "200 OK\r\n");
  strcpy(response.body, "Logout successful");
  return response;
}
#include "http_xd.h"

#include <stdlib.h>
struct HTTP_RESPONSE t_register(const char request[1024]) {

  struct HTTP_RESPONSE response = {
      .code = "",
      .body = "",
      .headers = "",
  };

  strcpy(response.code, "500 Internal Server Error\r\n");

  char *body;
  if (extract_body(request, &body)) {
    strcpy(response.code, "404 Bad Request\r\n");
    strcpy(response.body, "No request body found");
    return response;
  }
  const int MAX_EMAIL_LENGTH = 25;
  const int MAX_PASSWORD_LENGTH = 25;
  const int MIN_EMAIL_LENGTH = 4;
  const int MIN_PASSWORD_LENGTH = 4;

  char email[215];
  char password[215];
  if (search_value(body, "email:", email) != 0 ||
      search_value(body, "password:", password) != 0) {
    strcpy(response.code, "404 Bad Request\r\n");
    strcpy(response.body, "No email or password found");
    free(body);
    return response;
  }

  free(body);

  if (strlen(email) > MAX_EMAIL_LENGTH || strlen(email) < MIN_EMAIL_LENGTH) {
    strcpy(response.code, "404 Bad Request\r\n");
    strcpy(response.body, "Invalid email");
    return response;
  }

  if (strlen(password) > MAX_PASSWORD_LENGTH ||
      strlen(password) < MIN_PASSWORD_LENGTH) {
    strcpy(response.code, "404 Bad Request\r\n");
    strcpy(response.body, "Invalid password");
    return response;
  }

  char hashed_password[65] = {0};
  if (util_hash(password, hashed_password)) {
    strcpy(response.code, "500 Internal Server Error\r\n");
    strcpy(response.body, "Failed to hash password");
    return response;
  }

  if (insert_user(email, hashed_password)) {
    strcpy(response.code, "404 Bad Request\r\n");
    strcpy(response.body, "User already exists");
    return response;
  }
  strcpy(response.code, "200 OK\r\n");
  strcpy(response.body, "Registration successful");
  return response;
}

struct HTTP_RESPONSE t_unregister(char request[1024]) {

  struct HTTP_RESPONSE response = {
      .code = "",
      .body = "",
      .headers = "",
  };

  strcpy(response.code, "500 Internal Server Error\r\n");

  char token[33] = {0};
  if (search_value(request, "Token:", token)) {
    strcpy(response.code, "401 Unauthorized\r\n");
    strcpy(response.body, "No token found");
    return response;
  }

  if (strlen(token) != 32) {
    strcpy(response.code, "401 Unauthorized\r\n");
    strcpy(response.body, "Invalid token ");
    return response;
  }
  int user_id = 0;
  if (validate_token(token, &user_id)) {
    strcpy(response.code, "401 Unauthorized\r\n");
    strcpy(response.body, "Invalid token");
    return response;
  }

  if (delete_user(user_id)) {
    strcpy(response.code, "500 Internal Server Error\r\n");
    strcpy(response.body, "Unable to find user");
    return response;
  }
  strcpy(response.code, "200 OK\r\n");
  strcpy(response.body, "Unregistration successful");
  return response;
}

struct HTTP_RESPONSE t_users_user_vaults_get(char *token) {
  struct HTTP_RESPONSE response = {
      .code = "",
      .body = "",
      .headers = "",
  };

  strcpy(response.code, "500 Internal Server Error\r\n");

  int user_id = 0;
  if (validate_token(token, &user_id)) {
    strcpy(response.code, "401 Unauthorized\r\n");
    strcpy(response.body, "Invalid token");
    return response;
  }

  int *vaults = NULL;
  int vault_count = 0;

  if (get_user_vaults(user_id, &vaults, &vault_count)) {
    strcpy(response.code, "500 Internal Server Error\r\n");
    strcpy(response.body, "Unable to fetch vaults");
    return response;
  }

  if (vault_count == 0) {
    strcpy(response.code, "200 OK\r\n");
    strcpy(response.body, "User has no vaults");
    return response;
  }

  strcpy(response.code, "200 OK\r\n");
  char line[50];
  strcpy(response.body, "Vault IDs: ");
  for (int i = 0; i < vault_count; i++) {
    snprintf(line, sizeof(line), "%d, ", vaults[i]);
    strcat(response.body, line);
  }

  free(vaults);
  int len = strlen(response.body);
  response.body[len - 2] = '\0';
  return response;
}

// struct HTTP_RESPONSE t_user_page_read(char request[1024]) {
//
//   int struct HTTP_RESPONSE response = {
//       .code = "",
//       .body = "",
//       .headers = "",
//   };
//
//   strcpy(response.code, "500 Internal Server Error\r\n");
//
//   if (validate_vault(user_id, vault_id)) {
//     strcpy(response.code, "401 Unauthorized\r\n");
//     strcpy(response.body, "Unauthorized user for page in vault");
//     return response;
//   }
//   char title[256], file_path[256];
//   int file_size;
//
//   if (get_page_info(page_id, title, file_path, &file_size)) {
//     strcpy(response.code, "500 Internal Server Error\r\n");
//     strcpy(response.body, "Unable to fetch page info");
//     return response;
//   }
//
//   strcpy(response.code, "200 OK\r\n");
//   snprintf(response.body, sizeof(response.body),
//            "page_title = \"%s\"\npage_path = \"%s\"\npage_size = %d",
//            title, file_path, file_size);
//   return response;
// }
struct HTTP_RESPONSE
t_users_user_vaults_vault_pages_page_patch(int user_id, int vault_id,
                                           int page_id, char *new_title,
                                           char *new_content) {

  struct HTTP_RESPONSE response = {
      .code = "",
      .body = "",
      .headers = "",
  };

  strcpy(response.code, "200 OK\r\n");
  strcpy(response.body, "Successfully patched page");
  return response;
}

struct HTTP_RESPONSE t_users_user_vaults_vault_pages_page_delete(int user_id,
                                                                 int page_id) {

  struct HTTP_RESPONSE response = {
      .code = "",
      .body = "",
      .headers = "",
  };

  strcpy(response.code, "500 Internal Server Error\r\n");

  if (validate_page(user_id, page_id)) {
    strcpy(response.code, "401 Unauthorized\r\n");
    strcpy(response.body, "Unauthorized user for page");
    return response;
  }

  if (delete_page(page_id)) {
    strcpy(response.code, "500 Internal Server Error\r\n");
    strcpy(response.body, "Unable to delete page");
    return response;
  }
  strcpy(response.code, "200 OK\r\n");
  strcpy(response.body, "Successfully deleted page");
  return response;
}

struct HTTP_RESPONSE t_page_write(int page_id, char request[1024]) {

  struct HTTP_RESPONSE response = {
      .code = "",
      .body = "",
      .headers = "",
  };

  strcpy(response.code, "500 Internal Server Error\r\n");

  char token[33] = {0};
  if (search_value(request, "Token:", token)) {
    strcpy(response.code, "401 Unauthorized\r\n");
    strcpy(response.body, "No token found");
    return response;
  }

  if (strlen(token) != 32) {
    strcpy(response.code, "401 Unauthorized\r\n");
    strcpy(response.body, "Invalid token ");
    return response;
  }
  int user_id = 0;
  if (validate_token(token, &user_id)) {
    strcpy(response.code, "401 Unauthorized\r\n");
    strcpy(response.body, "Invalid token");
    return response;
  }

  char title[128];
  if (search_value(request, "title:", title)) {
    strcpy(response.code, "404 Bad Request\r\n");
    strcpy(response.body, "No title found");
    return response;
  }

  char *content;
  if (parse_curly_bracket_contents(request, "content:", &content)) {
    strcpy(response.code, "404 Bad Request\r\n");
    strcpy(response.body, "No body found");
    return response;
  }

  char file_path[255] = {0};
  char page_title[255] = {0};
  int owner_id = 0;
  int page_size = 0;
  if (get_page_info(page_id, &user_id, page_title, file_path, &page_size)) {
    strcpy(response.code, "404 Bad Request\r\n");
    strcpy(response.body, "Page file not found in database");
    free(content);
    return response;
  }

  if (!sys_file_exists(file_path)) {
    strcpy(response.code, "404 Bad Request\r\n");
    strcpy(response.body, "Page file not found on system");
    free(content);
    return response;
  }

  if (sys_file_write(file_path, content)) {
    strcpy(response.code, "404 Bad Request\r\n");
    strcpy(response.body, "Error writing to file on system");
    free(content);
    return response;
  }
  free(content);
  strcpy(response.code, "200 OK\r\n");
  strcpy(response.body, "Successfully modified page");
  return response;
}

struct HTTP_RESPONSE t_page_create(char request[1024]) {
  struct HTTP_RESPONSE response = {
      .code = "",
      .body = "",
      .headers = "",
  };

  strcpy(response.code, "500 Internal Server Error\r\n");

  char token[33] = {0};
  if (search_value(request, "Token:", token)) {
    strcpy(response.code, "401 Unauthorized\r\n");
    strcpy(response.body, "No token found");
    return response;
  }

  if (strlen(token) != 32) {
    strcpy(response.code, "401 Unauthorized\r\n");
    strcpy(response.body, "Invalid token ");
    return response;
  }
  int user_id = 0;
  if (validate_token(token, &user_id)) {
    strcpy(response.code, "401 Unauthorized\r\n");
    strcpy(response.body, "Invalid token");
    return response;
  }

  char title[128];
  if (search_value(request, "title:", title)) {
    strcpy(response.code, "404 Bad Request\r\n");
    strcpy(response.body, "No title found");
    return response;
  }

  char *content;
  if (parse_curly_bracket_contents(request, "content:", &content)) {
    strcpy(response.code, "404 Bad Request\r\n");
    strcpy(response.body, "No body found");
    return response;
  }

  int page_increment = get_page_counter() + 1;
  if (page_increment == -1) {
    free(content);
    strcpy(response.code, "500 Internal Server Error\r\n");
    strcpy(response.body, "Unable to page count");
    return response;
  }

  if (sys_folder_exists("./pages") == 0) {
    if (sys_folder_create("./pages")) {
      free(content);
      strcpy(response.code, "500 Internal Server Error\r\n");
      strcpy(response.body, "Unable to create folder");
      return response;
    }
  }
  char file_path[255] = {0};

  snprintf(file_path, sizeof(file_path), "./pages/%d/%d.md",
           page_increment / 100, page_increment);
  // file_path[strlen()] = '\0';

  char *last_slash = strrchr(file_path, '/');
  last_slash[0] = '\0';
  printf("folder: %s\n", file_path);
  if (!sys_folder_exists(file_path)) {
    printf("Creatingf folder %s\n", file_path);
    if (sys_folder_create(file_path)) {
      free(content);
      strcpy(response.code, "500 Internal Server Error\r\n");
      strcpy(response.body, "Unable to create folder");
      return response;
    }
  }

  last_slash[0] = '/';

  if (sys_file_exists(file_path)) {
    free(content);
    strcpy(response.code, "500 Internal Server Error\r\n");
    strcpy(response.body, "Page already exists");
    return response;
  }

  if (sys_file_create(file_path)) {
    free(content);
    strcpy(response.code, "500 Internal Server Error\r\n");
    strcpy(response.body, "Unable to create file");
    return response;
  }

  if (sys_file_write(file_path, content)) {
    free(content);
    strcpy(response.code, "500 Internal Server Error\r\n");
    strcpy(response.body, "Unable to write to file");
    return response;
  }
  free(content);

  if (insert_page(user_id, 31, file_path, title)) {
    strcpy(response.code, "500 Internal Server Error\r\n");
    strcpy(response.body, "Unable to create vault");
    return response;
  }
  strcpy(response.code, "200 OK\r\n");
  snprintf(response.body, sizeof(response.body),
           "Successfully created page with page_id: %d", page_increment);
  return response;
};

struct HTTP_RESPONSE t_users_user_vaults_vault_get(int user_id, int vault_id) {

  struct HTTP_RESPONSE response = {
      .code = "",
      .body = "",
      .headers = "",
  };

  strcpy(response.code, "500 Internal Server Error\r\n");

  if (validate_vault(user_id, vault_id)) {
    strcpy(response.code, "401 Unauthorized\r\n");
    strcpy(response.body, "Unauthorized user for vault");
    return response;
  }
  char vault_name[255] = {0};
  int vault_page_count = 0;

  if (get_vault_info(vault_id, vault_name, &vault_page_count)) {
    strcpy(response.code, "500 Internal Server Error\r\n");
    strcpy(response.body, "Unable get vault info");
    return response;
  }
  strcpy(response.code, "200 OK\r\n");
  snprintf(response.body, sizeof(response.body),
           "vault_name = \"%s\"\nvault_page_count = %d", vault_name,
           vault_page_count);
  return response;
}
struct HTTP_RESPONSE t_users_user_vaults_vault_delete(int user_id,
                                                      int vault_id) {

  struct HTTP_RESPONSE response = {
      .code = "",
      .body = "",
      .headers = "",
  };

  strcpy(response.code, "500 Internal Server Error\r\n");

  if (validate_vault(user_id, vault_id)) {
    strcpy(response.code, "401 Unauthorized\r\n");
    strcpy(response.body, "Unauthorized user for vault");
    return response;
  }

  if (delete_vault(vault_id)) {
    strcpy(response.code, "500 Internal Server Error\r\n");
    strcpy(response.body, "Unable to delete vault");
    return response;
  }
  strcpy(response.code, "200 OK\r\n");
  strcpy(response.body, "Successfully deleted vault");
  return response;
}
struct HTTP_RESPONSE t_users_user_vaults_vault_pages_get(int user_id,
                                                         int vault_id) {

  struct HTTP_RESPONSE response = {
      .code = "",
      .body = "",
      .headers = "",
  };

  strcpy(response.code, "500 Internal Server Error\r\n");

  if (validate_vault(user_id, vault_id)) {
    strcpy(response.code, "401 Unauthorized\r\n");
    strcpy(response.body, "Unauthorized user for vault");
    return response;
  }

  int *pages = NULL;
  int page_count = 0;

  if (get_vault_pages(vault_id, &pages, &page_count)) {
    strcpy(response.code, "500 Internal Server Error\r\n");
    strcpy(response.body, "Unable to fetch vaults pages");
    return response;
  }

  if (page_count == 0) {
    strcpy(response.code, "200 OK\r\n");
    strcpy(response.body, "Vault has no pages");
    return response;
  }

  strcpy(response.code, "200 OK\r\n");
  char line[50];
  strcpy(response.body, "Page IDs: ");
  for (int i = 0; i < page_count; i++) {
    snprintf(line, sizeof(line), "%d, ", pages[i]);
    strcat(response.body, line);
  }

  free(pages);
  int len = strlen(response.body);
  response.body[len - 2] = '\0';
  return response;
}

struct HTTP_RESPONSE
t_users_user_vaults_vault_pages_post(int user_id, int vault_id, char *file_path,
                                     char *title, char *content) {

  struct HTTP_RESPONSE response = {
      .code = "",
      .body = "",
      .headers = "",
  };

  strcpy(response.code, "500 Internal Server Error\r\n");

  if (validate_vault(user_id, vault_id)) {
    strcpy(response.code, "401 Unauthorized\r\n");
    strcpy(response.body, "Unauthorized user for page in vault");
    return response;
  }

  if (insert_page(user_id, vault_id, file_path, title)) {
    strcpy(response.code, "500 Internal Server Error\r\n");
    strcpy(response.body, "Unable to create page in vault");
    return response;
  }
  strcpy(response.code, "200 OK\r\n");
  strcpy(response.body, "Successfully created page in vault");
  return response;
}

struct HTTP_RESPONSE t_users_user_vaults_vault_pages_page_get(int user_id,
                                                              int vault_id,
                                                              int page_id) {

  struct HTTP_RESPONSE response = {
      .code = "",
      .body = "",
      .headers = "",
  };

  strcpy(response.code, "500 Internal Server Error\r\n");

  if (validate_vault(user_id, vault_id)) {
    strcpy(response.code, "401 Unauthorized\r\n");
    strcpy(response.body, "Unauthorized user for page in vault");
    return response;
  }
  char title[256], file_path[256];
  int file_size;
  int owner_id = 0;
  if (get_page_info(page_id, &owner_id, title, file_path, &file_size)) {
    strcpy(response.code, "500 Internal Server Error\r\n");
    strcpy(response.body, "Unable to fetch page info");
    return response;
  }

  strcpy(response.code, "200 OK\r\n");
  snprintf(response.body, sizeof(response.body),
           "page_title = \"%s\"\npage_path = \"%s\"\npage_size = %d", title,
           file_path, file_size);
  return response;
}

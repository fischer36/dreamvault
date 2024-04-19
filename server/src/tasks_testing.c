#include "db_testing.h"
#include "task_testing.h"

struct HTTP_RESPONSE t_login(const char *email, const char *password) {
  struct HTTP_RESPONSE response = {
      .code = "",
      .body = "",
      .headers = "",
  };
  strcpy(response.code, "500 Internal Server Error\r\n");

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

struct HTTP_RESPONSE t_logout(char *token) {

  struct HTTP_RESPONSE response = {
      .code = "",
      .body = "",
      .headers = "",
  };

  strcpy(response.code, "500 Internal Server Error\r\n");
  if (delete_session(token)) {
    strcpy(response.code, "400 Bad Request\r\n");
    strcpy(response.body, "Invalid token");
    return response;
  }

  strcpy(response.code, "200 OK\r\n");
  strcpy(response.body, "Logout successful");
  return response;
}

struct HTTP_RESPONSE t_register(char *email, char *password) {

  struct HTTP_RESPONSE response = {
      .code = "",
      .body = "",
      .headers = "",
  };

  strcpy(response.code, "500 Internal Server Error\r\n");

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

struct HTTP_RESPONSE t_unregister(char *token) {

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

struct HTTP_RESPONSE t_users_user_vaults_post(char *token, char *title) {

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

  if (insert_vault(user_id, title)) {
    strcpy(response.code, "500 Internal Server Error\r\n");
    strcpy(response.body, "Unable to create vault");
    return response;
  }
  strcpy(response.code, "200 OK\r\n");
  strcpy(response.body, "Successfully created vault");
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

  if (get_page_info(page_id, title, file_path, &file_size)) {
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

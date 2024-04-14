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
    free(vaults);
    return response;
  }

  strcpy(response.code, "200 OK\r\n");
  char line[50];
  strcpy(response.body, "Vault IDs: ");
  for (int i = 0; i < vault_count; i++) {
    snprintf(line, sizeof(line), "%d, ", vaults[i]);
    strcat(response.body, line);
  }
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

struct HTTP_RESPONSE t_users_user_vaults_vault_get();
struct HTTP_RESPONSE t_users_user_vaults_vault_delete();

struct HTTP_RESPONSE t_users_user_vaults_vault_pages_get();
struct HTTP_RESPONSE t_users_user_vaults_vault_pages_post();

struct HTTP_RESPONSE t_users_user_vaults_vault_pages_page_get();
struct HTTP_RESPONSE t_users_user_vaults_vault_pages_page_patch();
struct HTTP_RESPONSE t_users_user_vaults_vault_pages_page_delete();

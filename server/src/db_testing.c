#include "db_testing.h"
#include "task_testing.h"

#define DB_HOST "localhost"
#define DB_PORT 0
#define DB_NAME "dreamvault"
#define DB_USER "root"
#define DB_PASSWORD "hahaXD"

static MYSQL *connect() {
  MYSQL *connection;
  connection = mysql_init(NULL);
  if (connection == NULL) {
    return NULL;
  }
  if (mysql_real_connect(connection, DB_HOST, DB_USER, DB_PASSWORD, DB_NAME, 0,
                         NULL, DB_PORT) == NULL) {
    mysql_close(connection);
    return NULL;
  }
  return connection;
}

static int disconnect(MYSQL *connection) {
  mysql_close(connection);
  return 0;
}

int validate_token(char token[33], int *owner_user_id) {
  MYSQL *connection = connect();
  if (connection == NULL) {
    return -1;
  }

  char query[256];
  snprintf(query, sizeof(query),
           "SELECT user_id FROM sessions WHERE token = '%s'", token);

  if (mysql_query(connection, query)) {
    disconnect(connection);
    return -1;
  }

  MYSQL_RES *result = mysql_store_result(connection);
  if (result == NULL) {
    disconnect(connection);
    return -1;
  }

  MYSQL_ROW row = mysql_fetch_row(result);
  if (!row) {
    mysql_free_result(result);
    disconnect(connection);
    return -1;
  }
  *owner_user_id = atoi(row[0]);
  mysql_free_result(result);
  disconnect(connection);
  return 0;
}

int get_vault_pages(int vault_id) {
  MYSQL *connection = connect();
  if (connection == NULL)
    return -1;

  char query[256];
  snprintf(query, sizeof(query), "SELECT * FROM pages WHERE vault_id = %d",
           vault_id);

  if (mysql_query(connection, query)) {
    disconnect(connection);
    return -1;
  }
  MYSQL_RES *result = mysql_store_result(connection);
  if (result == NULL) {
    disconnect(connection);
    return -1;
  }

  MYSQL_ROW row;
  while ((row = mysql_fetch_row(result))) {
    printf("page ID: %s\n", row[0]);
  }

  mysql_free_result(result);
  disconnect(connection);
  return 0;
}

int get_user_vaults(int user_id, int **vaults, int *vault_count) {
  MYSQL *connection = connect();
  if (connection == NULL) {
    return -1;
  }

  char query[256];
  snprintf(query, sizeof(query),
           "SELECT vault_id FROM vaults WHERE owner_id = %d", user_id);

  if (mysql_query(connection, query)) {
    disconnect(connection);
    return -1;
  }

  MYSQL_RES *result = mysql_store_result(connection);
  if (result == NULL) {
    disconnect(connection);
    return -1;
  }

  int num_fields = mysql_num_rows(result);
  if (num_fields > 0) {
    *vaults = malloc(num_fields * sizeof(int));
    if (*vaults == NULL) {
      mysql_free_result(result);
      disconnect(connection);
      return -1;
    }
  } else {
    *vaults = NULL;
  }

  *vault_count = 0;
  MYSQL_ROW row;
  while ((row = mysql_fetch_row(result))) {
    (*vaults)[*vault_count] = atoi(row[0]);
    (*vault_count)++;
  }
  mysql_free_result(result);
  disconnect(connection);
  return 0;
}

int get_user_pages(int user_id) {
  MYSQL *connection = connect();
  if (connection == NULL)
    return -1;

  char query[256];
  snprintf(query, sizeof(query), "SELECT * FROM pages WHERE owner_id = %d",
           user_id);

  if (mysql_query(connection, query)) {
    disconnect(connection);
    return -1;
  }
  MYSQL_RES *result = mysql_store_result(connection);
  if (result == NULL) {
    disconnect(connection);
    return -1;
  }

  MYSQL_ROW row;
  while ((row = mysql_fetch_row(result))) {
    printf("Page ID: %s\n", row[0]);
  }

  mysql_free_result(result);
  disconnect(connection);
  return 0;
}

int insert_user(char *email, char *password) {
  MYSQL *connection = connect();
  if (connection == NULL)
    return -1;

  char query[1024];

  snprintf(query, sizeof(query),
           "INSERT INTO users (email, password, storage_limit) VALUES ('%s', "
           "'%s', %d)",
           email, password, 2000);

  if (mysql_query(connection, query)) {
    printf("Error deleting vault: %s\n", mysql_error(connection));
    disconnect(connection);
    return -1;
  }
  disconnect(connection);
  return 0;
}

int delete_user(int user_id) {

  MYSQL *connection = connect();
  if (connection == NULL)
    return -1;

  char query[1024];

  snprintf(query, sizeof(query), "DELETE FROM users WHERE user_id= %d",
           user_id);

  if (mysql_query(connection, query)) {
    printf("Error deleting user: %s\n", mysql_error(connection));

    disconnect(connection);
    return -1;
  }
  int affected_rows = mysql_affected_rows(connection);
  if (affected_rows == 0) {
    printf("No user found with id : %d to delete.\n", user_id);
    disconnect(connection);
    return -1;
  }
  disconnect(connection);
  return 0;
}

int insert_vault(int user_id, char *title) {

  MYSQL *connection = connect();
  if (connection == NULL)
    return -1;

  char query[1024];
  snprintf(query, sizeof(query),
           "INSERT INTO vaults (owner_id, name) VALUES (%d, '%s')", user_id,
           title);

  if (mysql_query(connection, query)) {
    printf("Error creating vault: %s\n", mysql_error(connection));

    disconnect(connection);
    return -1;
  }

  disconnect(connection);
  return 0;
}

int delete_vault(int vault_id) {

  MYSQL *connection = connect();
  if (connection == NULL)
    return -1;
  char query[1024];
  snprintf(query, sizeof(query), "DELETE FROM vaults WHERE vault_id = %d",
           vault_id);

  if (mysql_query(connection, query)) {
    printf("Error deleting vault: %s\n", mysql_error(connection));

    disconnect(connection);
    return -1;
  }

  int affected_rows = mysql_affected_rows(connection);
  if (affected_rows == 0) {
    printf("No vault found with id %d to delete.\n", vault_id);
    disconnect(connection);
    return -1;
  }
  disconnect(connection);
  return 0;
}

int insert_page(int user_id, int vault_id, char *file_path, char *title) {

  MYSQL *connection = connect();
  if (connection == NULL)
    return -1;

  char query[1024];
  snprintf(query, sizeof(query),
           "INSERT INTO pages (owner_id, vault_id, file_path, title) VALUES "
           "(%d, %d, '%s', '%s')",
           user_id, vault_id, file_path, title);

  if (mysql_query(connection, query)) {
    printf("Error creating page: %s\n", mysql_error(connection));
    disconnect(connection);
    return -1;
  }

  disconnect(connection);
  return 0;
}

int delete_page(int page_id) {
  MYSQL *connection = connect();
  if (connection == NULL)
    return -1;
  char query[1024];
  snprintf(query, sizeof(query), "DELETE FROM pages WHERE page_id = %d",
           page_id);

  if (mysql_query(connection, query)) {
    printf("Error deleting page: %s\n", mysql_error(connection));

    disconnect(connection);
    return -1;
  }

  int affected_rows = mysql_affected_rows(connection);
  if (affected_rows == 0) {
    printf("No page found with id %d to delete.\n", page_id);
    disconnect(connection);
    return -1;
  }
  disconnect(connection);
  return 0;
}

int insert_session(int user_id, char *token) {

  MYSQL *connection = connect();
  if (connection == NULL)
    return -1;

  char query[1024];
  snprintf(query, sizeof(query),
           "INSERT INTO sessions(user_id, token) VALUES (%d, '%s') "
           "ON DUPLICATE KEY UPDATE token = VALUES(token)",
           user_id, token);

  if (mysql_query(connection, query)) {
    printf("Error creating session for user %d: %s\n", user_id,
           mysql_error(connection));
    disconnect(connection);
    return -1;
  }

  disconnect(connection);
  return 0;
}

int delete_session(char *token) {

  MYSQL *connection = connect();
  if (connection == NULL)
    return -1;

  char query[1024];

  snprintf(query, sizeof(query), "DELETE FROM sessions WHERE token = '%s'",
           token);

  if (mysql_query(connection, query)) {
    printf("Error deleting vault: %s\n", mysql_error(connection));

    disconnect(connection);
    return -1;
  }

  int affected_rows = mysql_affected_rows(connection);
  if (affected_rows == 0) {
    printf("No session found with token %s to delete.\n", token);
    disconnect(connection);
    return -1;
  }
  disconnect(connection);
  return 0;
}
int get_user_id(const char *email, int *user_id, char password[65]) {

  MYSQL *connection = connect();
  if (connection == NULL) {
    return -1;
  }

  char query[256];
  snprintf(query, sizeof(query),
           "SELECT user_id, password FROM users WHERE email = '%s'", email);

  if (mysql_query(connection, query)) {
    disconnect(connection);
    return -1;
  }

  MYSQL_RES *result = mysql_store_result(connection);
  if (result == NULL) {
    disconnect(connection);
    return -1;
  }

  MYSQL_ROW row = mysql_fetch_row(result);
  if (!row) {
    mysql_free_result(result);
    disconnect(connection);
    return -1;
  }
  *user_id = atoi(row[0]);
  strncpy(password, row[1], 65);
  password[65] = '\0';
  mysql_free_result(result);
  disconnect(connection);
  return 0;
}
#include "util.h"
int main() {
  // if (validate_token(char *token, int *owner_user_id))
  // char hashed[65];
  // if (util_hash("newpassxd", hashed))
  //
  //   printf("error hashing");
  //
  // printf("%s", hashed);
  // if (util_compare_hash(
  //         "newpassxd",
  //         "935844450a0376cb26723e1f56b0c109386cf89703f73c52ceab35c3effb1736"))
  //         {
  //   printf("ok fail");
  // } else {
  //
  //   printf("ok sucess");
  // }
  //       printf("ok sucess");
  // struct HTTP_RESPONSE response = t_logout(
  //     "935844450a0376cb26723e1f56b0c109386cf89703f73c52ceab35c3effb1736");
  // printf("%s", response.body);
  // char token[33];
  // if
  // (util_generate_token(token))
  //   printf("error creating token");
  // if (insert_session(22, token))
  //   printf(" error create pages");
  // int user_id = 0;
  // char password[255];

  // if (get_user_id("hello@gmail.com", &user_id, password)) {
  //
  //   printf("errorkjA");
  //   return -1;
  // }
  // char hashed[65];
  // if (util_compare_hash(
  //         "newpassxd2",
  //         "935844450a0376cb26723e1f56b0c109386cf89703f73c52ceab35c3effb1736"))
  //         {
  //   printf("error");
  // } else {
  //   printf("ok");
  // }
  // struct HTTP_RESPONSE response =
  //     t_unregister("6077304f7e373d374b9c4f9870f1f9ba");

  // int *vaults = NULL;
  // int vault_count = 0;
  // int user_id = 123; // Example user ID
  struct HTTP_RESPONSE response = t_users_user_vaults_post(
      "0484253aa7baaa61af4ade4ea7f11aab", "new vaultxd");
  printf("%s", response.body);
  //
  // if (get_user_vaults(23, &vaults, &vault_count) == 0) {
  //   for (int i = 0; i < vault_count; i++) {
  //     printf("Vault ID: %d\n", vaults[i]);
  //   }
  //   printf("ok? %d\n", vault_count);
  //   free(vaults); // Free the allocated memory
  // } else {
  //   printf("Failed to retrieve vaults.\n");
  // }
  // insert_user("oknew@gmail.com2231", hashed); // newpassxd
  // if (validate_token("2317304f7e373d374b9c4f9870f1f9ba", &user_id))
  //   printf(" error create pages");

  // printf(" %d, %s\n", user_id, password);

  // if (get_vault_pages(14))
  //   printf(" error pages");

  return 0;
}

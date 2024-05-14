#include "final_db.h"
#include "config.h"
#include "final_api.h"

static MYSQL *connect() {
    MYSQL *connection;
    connection = mysql_init(NULL);
    if (connection == NULL) {
        return NULL;
    }
    if (mysql_real_connect(connection, DB_HOST, DB_USER, DB_PASSWORD, DB_NAME, 0, NULL, DB_PORT) == NULL) {
        mysql_close(connection);
        return NULL;
    }
    return connection;
}

static int disconnect(MYSQL *connection) {
    mysql_close(connection);
    return 0;
}

int validate_page(int user_id, int page_id) {
    MYSQL *connection = connect();
    if (connection == NULL) {
        return -1;
    }

    char query[256];
    snprintf(query, sizeof(query), "SELECT owner_id FROM pages WHERE page_id = %d", page_id);

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

    int actual_owner_id = atoi(row[0]);
    mysql_free_result(result);
    disconnect(connection);

    if (actual_owner_id != user_id) {
        return -1;
    }
    return 0;
}
int validate_vault(int user_id, int vault_id) {
    MYSQL *connection = connect();
    if (connection == NULL) {
        return -1;
    }

    char query[256];
    snprintf(query, sizeof(query), "SELECT owner_id FROM vaults WHERE vault_id = %d", vault_id);

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

    int actual_owner_id = atoi(row[0]);
    mysql_free_result(result);
    disconnect(connection);

    if (actual_owner_id != user_id) {
        return -1;
    }
    return 0;
}

int validate_token(char token[33], int *owner_user_id) {
    MYSQL *connection = connect();
    if (connection == NULL) {
        return -1;
    }

    char query[256];
    snprintf(query, sizeof(query), "SELECT user_id FROM sessions WHERE token = '%s'", token);

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

int get_vault_pages(int vault_id, int **pages, int *page_count) {
    MYSQL *connection = connect();
    if (connection == NULL) {
        return -1;
    }

    char query[256];
    snprintf(query, sizeof(query), "SELECT page_id FROM pages WHERE vault_id = %d", vault_id);

    if (mysql_query(connection, query)) {
        disconnect(connection);
        return -1;
    }

    MYSQL_RES *result = mysql_store_result(connection);
    if (result == NULL) {
        disconnect(connection);
        return -1;
    }

    int num_rows = mysql_num_rows(result);
    if (num_rows > 0) {
        *pages = malloc(num_rows * sizeof(int));
        if (*pages == NULL) {
            mysql_free_result(result);
            disconnect(connection);
            return -1;
        }
    } else {
        *pages = NULL;
    }

    *page_count = 0;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        (*pages)[*page_count] = atoi(row[0]);
        (*page_count)++;
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
    snprintf(query, sizeof(query), "SELECT vault_id FROM vaults WHERE owner_id = %d", user_id);

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

int get_user_pages(int user_id, struct Page **pages, int *page_count) {
    MYSQL *connection = connect();
    if (connection == NULL) {
        return -1;
    }

    char query[256];
    snprintf(query, sizeof(query), "SELECT * FROM pages WHERE owner_id = %d", user_id);

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
        *pages = malloc(num_fields * sizeof(struct Page));
        if (*pages == NULL) {
            mysql_free_result(result);
            disconnect(connection);
            return -1;
        }
    } else {
        *pages = NULL;
    }

    *page_count = 0;
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        struct Page page = {
            .id = atoi(row[0]),
            .modified = atoi(row[6]),
        };
        (*pages)[*page_count] = page;
        (*page_count)++;
    }
    printf("%d\n", *page_count);
    mysql_free_result(result);
    disconnect(connection);
    return 0;
}

int insert_user(char *email, char *password) {
    MYSQL *connection = connect();
    if (connection == NULL) {

        printf("conection error \n");
        return -1;
    }

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

    snprintf(query, sizeof(query), "DELETE FROM users WHERE user_id= %d", user_id);

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
int get_page_counter() {

    MYSQL *connection = connect();
    if (connection == NULL)
        return -1;

    char query[1024];
    snprintf(query, sizeof(query), "SELECT MAX(page_id) FROM pages");

    if (mysql_query(connection, query)) {
        printf("Error creating max page_id: %s\n", mysql_error(connection));

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
    int counter = atoi(row[0]);
    mysql_free_result(result);
    return counter;
}
int insert_vault(int user_id, char *title) {

    MYSQL *connection = connect();
    if (connection == NULL)
        return -1;

    char query[1024];
    snprintf(query, sizeof(query), "INSERT INTO vaults (owner_id, name) VALUES (%d, '%s')", user_id, title);

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
    snprintf(query, sizeof(query), "DELETE FROM vaults WHERE vault_id = %d", vault_id);

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

int insert_page(int user_id, int vault_id, char *file_path, long modified, char *title) {

    MYSQL *connection = connect();
    if (connection == NULL)
        return -1;

    char query[1024];
    snprintf(query, sizeof(query),
             "INSERT INTO pages (owner_id, file_path, last_modified, title) VALUES "
             "(%d, '%s', %ld, '%s')",
             user_id, file_path, modified, title);

    if (mysql_query(connection, query)) {
        printf("Error creating page: %s\n", mysql_error(connection));
        disconnect(connection);
        return -1;
    }

    disconnect(connection);
    return 0;
}

int update_page(int page_id, char *title, char *content, long modified, int file_size) {

    MYSQL *connection = connect();
    if (connection == NULL)
        return -1;

    char query[1024];
    snprintf(query, sizeof(query), "UPDATE pages SET title = '%s', file_size = %d, last_modified = %ld WHERE page_id = %d", title,
             file_size, modified, page_id);

    if (mysql_query(connection, query)) {
        printf("Error updating page: %d %s\n", page_id, mysql_error(connection));
        disconnect(connection);
        return -1;
    }

    int affected_rows = mysql_affected_rows(connection);
    if (affected_rows == 0) {
        printf("No page found with id %d to modify\n", page_id);
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
    snprintf(query, sizeof(query), "DELETE FROM pages WHERE page_id = %d", page_id);

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
        printf("Error creating session for user %d: %s\n", user_id, mysql_error(connection));
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

    snprintf(query, sizeof(query), "DELETE FROM sessions WHERE token = '%s'", token);

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

int get_vault_info(int vault_id, char *name, int *page_count) {
    MYSQL *connection = connect();
    if (connection == NULL) {
        return -1;
    }

    char query[256];
    snprintf(query, sizeof(query), "SELECT name FROM vaults WHERE vault_id = %d", vault_id);

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

    strncpy(name, row[0], 255);
    name[255] = '\0';
    mysql_free_result(result);

    snprintf(query, sizeof(query), "SELECT COUNT(*) FROM pages WHERE vault_id = %d", vault_id);

    if (mysql_query(connection, query)) {
        disconnect(connection);
        return -1;
    }

    result = mysql_store_result(connection);
    if (result == NULL) {
        disconnect(connection);
        return -1;
    }

    row = mysql_fetch_row(result);
    if (!row) {
        mysql_free_result(result);
        disconnect(connection);
        return -1;
    }

    *page_count = atoi(row[0]);
    mysql_free_result(result);

    disconnect(connection);
    return 0;
}
int get_page_version(int page_id, int *version) {

    MYSQL *conn = connect();
    if (conn == NULL) {
        return -1;
    }
    printf("page_id: %d\n", page_id);

    char query[214];
    snprintf(query, sizeof(query), "SELECT version FROM pages WHERE page_id = %d", page_id);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Failed to query database: %s\n", mysql_error(conn));
        disconnect(conn);
        return -1;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (result == NULL) {
        fprintf(stderr, "Failed to store result: %s\n", mysql_error(conn));
        disconnect(conn);
        return -1;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    if (row == NULL) {
        fprintf(stderr, "No record found.\n");
        mysql_free_result(result);
        disconnect(conn);
        return -1;
    }
    *version = atoi(row[0]);
    mysql_free_result(result);
    disconnect(conn);
    return 0;
}
int get_page_info(int page_id, int *user_id, char title[256], char file_path[256], int *file_size) {
    MYSQL *conn = connect();
    if (conn == NULL) {
        return -1;
    }

    char query[1024];
    snprintf(query, sizeof(query),
             "SELECT owner_id, title, file_path, file_size FROM pages WHERE "
             "page_id = %d",
             page_id);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "Failed to query database: %s\n", mysql_error(conn));
        disconnect(conn);
        return -1;
    }

    MYSQL_RES *result = mysql_store_result(conn);
    if (result == NULL) {
        fprintf(stderr, "Failed to store result: %s\n", mysql_error(conn));
        disconnect(conn);
        return -1;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    if (row == NULL) {
        fprintf(stderr, "No record found.\n");
        mysql_free_result(result);
        disconnect(conn);
        return -1;
    }
    *user_id = atoi(row[0]);
    strncpy(title, row[1], 255);
    title[255] = '\0';
    strncpy(file_path, row[2], 255);
    file_path[255] = '\0';
    *file_size = atoi(row[3]);

    mysql_free_result(result);
    disconnect(conn);
    return 0;
}

int get_user_id(const char *email, int *user_id, char password[65]) {

    MYSQL *connection = connect();
    if (connection == NULL) {
        return -1;
    }

    char query[256];
    snprintf(query, sizeof(query), "SELECT user_id, password FROM users WHERE email = '%s'", email);

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
// int main() {
//   // if (validate_token(char *token, int *owner_user_id))
//   // char hashed[65];
//   // if (util_hash("newpassxd", hashed))
//   //
//   //   printf("error hashing");
//   //
//   // printf("%s", hashed);
//   // if (util_compare_hash(
//   //         "newpassxd",
//   // "935844450a0376cb26723e1f56b0c109386cf89703f73c52ceab35c3effb1736"))
//   //         {
//   //   printf("ok fail");
//   // } else {
//   //
//   //   printf("ok sucess");
//   // }
//   //       printf("ok sucess");
//   // struct HTTP_RESPONSE response = t_logout(
//   // "935844450a0376cb26723e1f56b0c109386cf89703f73c52ceab35c3effb1736");
//   // printf("%s", response.body);
//   // char token[33];
//   // if
//   // (util_generate_token(token))
//   //   printf("error creating token");
//   // if (insert_session(22, token))
//   //   printf(" error create pages");
//   // int user_id = 0;
//   // char password[255];
//
//   // if (get_user_id("hello@gmail.com", &user_id, password)) {
//   //
//   //   printf("errorkjA");
//   //   return -1;
//   // }
//   // char hashed[65];
//   // if (util_compare_hash(
//   //         "newpassxd2",
//   // "935844450a0376cb26723e1f56b0c109386cf89703f73c52ceab35c3effb1736"))
//   //         {
//   //   printf("error");
//   // } else {
//   //
//   //   printf("ok");
//   // }
//   // struct HTTP_RESPONSE response =
//   //     t_unregister("6077304f7e373d374b9c4f9870f1f9ba");
//
//   // int *vaults = NULL;
//   // int vault_count = 0;
//   // int user_id = 123; // Example user ID
//   struct HTTP_RESPONSE response =
//       t_users_user_vaults_vault_pages_page_delete(27, 28);
//
//   printf("%s", response.body);
//   //
//   // char title[256], file_path[256];
//   // int file_size;
//   // int page_id = 28; // Example page ID
//   //
//   // if (get_page_info(page_id, title, file_path, &file_size) == 0) {
//   //   printf("Title: %s\n", title);
//   //   printf("File Path: %s\n", file_path);
//   //   printf("File Size: %d\n", file_size);
//   // } else {
//   //   printf("Error retrieving page information.\n");
//   // }
//   // //
//   //
//   // if (validate_page(23, 17)) {
//   //   printf("Unauthorized");
//   // } else {
//   //
//   //   printf("authorized");
//   // }
//   // int vault_id = 17; // Example vault_id
//   // int *pages = NULL;
//   // int page_count = 0;
//   //
//   // int result = get_vault_pages(vault_id, &pages, &page_count);
//   // if (result == 0) {
//   //   if (page_count > 0) {
//   //
//   //     printf("Pages in Vault %d:\n", page_count);
//   //
//   //     for (int i = 0; i < page_count; i++) {
//   //       printf("%d\n", pages[i]);
//   //     }
//   //     free(pages);
//   //   } else {
//   //     printf("No pages found.\n");
//   //   }
//   // } else {
//   //   printf("Failed to retrieve pages.\n");
//   // }
//   // if (get_user_vaults(23, &vaults, &vault_count) == 0) {
//   //   for (int i = 0; i < vault_count; i++) {
//   //     printf("Vault ID: %d\n", vaults[i]);
//   //   }
//   //   printf("ok? %d\n", vault_count);
//   //   free(vaults); // Free the allocated memory
//   // } else {
//   //   printf("Failed to retrieve vaults.\n");
//   // }
//   // insert_user("oknew@gmail.com2231", hashed); // newpassxd
//   // if (validate_token("2317304f7e373d374b9c4f9870f1f9ba", &user_id))
//   //   printf(" error create pages");
//
//   // printf(" %d, %s\n", user_id, password);
//
//   // if (get_vault_pages(14))
//   //   printf(" error pages");
//
//   return 0;
// }
//
//
int get_user(char token[33], int *user_id) {

    MYSQL *connection = connect();
    if (connection == NULL) {
        return -1;
    }

    char query[256];
    snprintf(query, sizeof(query), "SELECT user_id FROM sessions WHERE token = '%s'", token);

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
    mysql_free_result(result);
    disconnect(connection);
    return 0;
}

#include "db.h"
#include "config.h"
#include "sys.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

int db_connection_get(MYSQL **connection) {
  *connection = mysql_init(NULL);
  if (*connection == NULL) {
    return 1;
  }
  if (mysql_real_connect(*connection, "localhost", "root", "hahaXD",
                         "dreamvault", 0, NULL, 0) == NULL) {
    mysql_close(*connection);
    return 1;
  }
  return 0;
}

int db_connection_close(MYSQL *connection) {
  mysql_close(connection);
  return 0;
}

int db_vault_delete(MYSQL *connection, const int vault_id) {
  char query[1024]; // Increased size for safety
                    //
  snprintf(query, sizeof(query), "DELETE FROM Vaults WHERE vault_id = %d",
           vault_id);

  if (mysql_query(connection, query)) {
    printf("db_vault_create error: %s\n", mysql_error(connection));
    return -1;
  }
  return 0;
}
int db_vault_create(MYSQL *connection, const int user_id,
                    const char *vault_name) {
  char query[1024]; // Increased size for safety
  snprintf(query, sizeof(query),
           "INSERT INTO Vaults(owner_id, name) VALUES (%d, '%s')", user_id,
           vault_name);

  if (mysql_query(connection, query)) {
    printf("db_vault_create error: %s\n", mysql_error(connection));
    return -1;
  }
  return 0;
}

int db_pages_in_vault(MYSQL *connection, int vault_id, Page **pages,
                      int *pages_count) {
  // Construct the SQL query to select all page_ids from pages where vault_id
  // matches
  char query[256];
  snprintf(query, sizeof(query),
           "SELECT page_id FROM pages WHERE vault_id = %d", vault_id);

  // Execute the SQL query
  if (mysql_query(connection, query)) {
    fprintf(stderr, "MySQL error: %s\n", mysql_error(connection));
    return -1;
  }

  // Store the result from the query
  MYSQL_RES *result = mysql_store_result(connection);
  if (result == NULL) {
    fprintf(stderr, "MySQL error: %s\n", mysql_error(connection));
    return -1;
  }

  // Get the number of rows in the result set
  *pages_count = mysql_num_rows(result);

  // Allocate memory for the pages array based on the count
  *pages = malloc(sizeof(Page) * (*pages_count));
  if (*pages == NULL) {
    fprintf(stderr, "Failed to allocate memory for pages\n");
    mysql_free_result(result);
    return -1;
  }

  // Fetch each row and populate the pages array
  MYSQL_ROW row;
  int index = 0;
  while ((row = mysql_fetch_row(result))) {
    (*pages)[index].page_id = atoi(row[0]);
    index++;
  }

  // Clean up
  mysql_free_result(result);

  return 0;
}
int db_vault_get(MYSQL *connection, const int vault_id, Vault *vault) {
  char query[1024];
  snprintf(
      query, sizeof(query),
      "SELECT vault_id, owner_id, name " // Fixed missing space and added name
      "FROM vaults WHERE vault_id = %d",
      vault_id);

  if (mysql_query(connection, query)) {
    printf("db_vault_get error: %s\n", mysql_error(connection));
    return -1;
  }

  MYSQL_RES *result = mysql_store_result(connection);
  if (result == NULL) {
    printf("Failed to store result\n");
    fprintf(stderr, "%s\n", mysql_error(connection));
    return -1;
  }

  MYSQL_ROW row = mysql_fetch_row(result);
  if (row) { // If a row was fetched
    vault->vault_id = atoi(row[0]);
    vault->owner_id = atoi(row[1]);
    // Ensure to copy the string safely, row[2] is the `name` column
    strncpy(vault->name, row[2], sizeof(vault->name) - 1);
    vault->name[sizeof(vault->name) - 1] = '\0'; // Null-terminate the string
  } else {
    printf("No record found with vault_id = %d\n", vault_id);
    mysql_free_result(result);
    return -1;
  }

  mysql_free_result(result);
  return 0;
}
int db_user_create(MYSQL *connection, const char *email, const char *password,
                   int storage_limit) {
  char query[256];

  snprintf(query, sizeof(query),
           "INSERT INTO Users (email, password, creation_date, "
           "storage_limit) VALUES ('%s', '%s', NOW(), %d)",
           email, password, 2000);

  if (mysql_query(connection, query)) {
    fprintf(stderr, "%s\n", mysql_error(connection));
    return -1;
  }

  return 0;
}

int db_user_get(MYSQL *connection, const int user_id, User *user) {
  printf("getting user: %d\n", user_id);
  char query[1024];
  snprintf(query, sizeof(query),
           "SELECT user_id, email, password, creation_date, storage_limit "
           "FROM Users WHERE user_id = '%d'",
           user_id);

  if (mysql_query(connection, query)) {
    fprintf(stderr, "%s\n", mysql_error(connection));
    printf("Failed to execute query\n");
    return -1;
  }

  MYSQL_RES *result = mysql_store_result(connection);
  if (result == NULL) {
    printf("Failed to store result\n");
    fprintf(stderr, "%s\n", mysql_error(connection));
    return -1;
  }

  MYSQL_ROW row;
  if ((row = mysql_fetch_row(result))) {
    printf("User found: %s\n", row[0]);
    // Assuming user_id and storage_limit are integers and others are
    // strings
    user->user_id = atoi(row[0]); // Convert string to int
    strncpy(user->email, row[1], sizeof(user->email) - 1); // Copy email
    user->email[sizeof(user->email) - 1] = '\0'; // Ensure null-termination
    strncpy(user->password, row[2],
            sizeof(user->password) - 1); // Copy hashed password
    user->password[sizeof(user->password) - 1] =
        '\0'; // Ensure null-termination
    strncpy(user->creation_date, row[3],
            sizeof(user->creation_date) - 1); // Copy creation creation_date
    user->creation_date[sizeof(user->creation_date) - 1] =
        '\0';                            // Ensure null-termination
    user->storage_limit = atoll(row[4]); // Convert string to long long int

    mysql_free_result(result);
    return 0;
  } else {
    mysql_free_result(result);
    return 1;
  }
}

int db_user_delete(MYSQL *connection, int user_id) {
  char query[256];

  snprintf(query, sizeof(query), "DELETE FROM Users WHERE user_id = %d",
           user_id);

  if (mysql_query(connection, query)) {
    fprintf(stderr, "%s\n", mysql_error(connection));
    return -1;
  }

  return 0;
}

bool db_does_user_exists(MYSQL *connection, const char *email) {
  char query[256];
  MYSQL_RES *result;
  int exists = 0;

  snprintf(query, sizeof(query),
           "SELECT EXISTS(SELECT 1 FROM Users WHERE email = '%s')", email);

  if (mysql_query(connection, query)) {
    fprintf(stderr, "%s\n", mysql_error(connection));
    return -1;
  }

  result = mysql_store_result(connection);
  if (result == NULL) {
    fprintf(stderr, "%s\n", mysql_error(connection));
    return -1;
  }

  MYSQL_ROW row = mysql_fetch_row(result);
  if (row && row[0]) {
    exists = atoi(row[0]);
  }

  mysql_free_result(result);

  return exists;
}
int db_page_delete(MYSQL *connection, const int page_id) {
  // Construct the SQL query to delete a page with the given page_id.
  char query[256];
  snprintf(query, sizeof(query), "DELETE FROM PAGES WHERE FileID = %d",
           page_id);

  // Execute the SQL query.
  if (mysql_query(connection, query)) {
    fprintf(stderr, "MySQL error: %s\n", mysql_error(connection));
    return -1; // Return -1 to indicate failure.
  }

  // Check if the deletion was successful by examining affected rows.
  if (mysql_affected_rows(connection) == 0) {
    // No rows were affected, indicating no such page_id exists.
    printf("No page with ID %d was found to delete.\n", page_id);
    return -1; // Optionally, return -1 or a specific code to indicate "not
               // found".
  } else {
    printf("Page with ID %d deleted successfully.\n", page_id);
    return 0; // Return 0 to indicate success.
  }
}

int db_page_create(MYSQL *connection, const int user_id, const int vault_id,
                   const char *title) {
  char folderName[20]; // For YYYY_MM format
  time_t currentTime = time(NULL);
  struct tm *timeInfo = localtime(&currentTime);
  sprintf(folderName, "%04d_%02d", timeInfo->tm_year + 1900,
          timeInfo->tm_mon + 1);

  const char *vault_path = "./vault";

  // Ensure the vault folder exists
  if (!sys_folder_exists(vault_path)) {
    if (sys_folder_create(vault_path) != 0) {
      fprintf(stderr, "Failed to create vault path: %s\n", vault_path);
      return 1;
    }
  }

  char folderPath[256];
  snprintf(folderPath, sizeof(folderPath), "%s/%s", vault_path, folderName);

  // Ensure the year_month folder exists
  if (!sys_folder_exists(folderPath)) {
    if (sys_folder_create(folderPath) != 0) {
      fprintf(stderr, "[error] Failed to create folder: %s\n", folderPath);
      return 1;
    }
  }

  char filePath[256];
  snprintf(filePath, sizeof(filePath), "%s/%d_%s.md", folderPath, user_id,
           title);

  if (sys_file_exists(filePath)) {
    fprintf(stderr, "[error] File already exists: %s\n", filePath);
    return 1;
  }

  if (sys_file_create(filePath) != 0) {
    fprintf(stderr, "[error] Failed to create file: %s\n", filePath);
    return 1;
  }

  int file_size = 69;

  char query[1024]; // Increased size for safety
  snprintf(query, sizeof(query),
           "INSERT INTO Pages (owner_id, vault_id, file_path, file_size, "
           "title) VALUES (%d, %d, '%s', %d, '%s')",
           user_id, vault_id, filePath, file_size, title);

  if (mysql_query(connection, query)) {
    sys_file_delete(filePath); // Cleanup if database insert fails
    fprintf(stderr, "MySQL error: %s\n", mysql_error(connection));
    return -1;
  }

  return 0;
}
int db_user_get_pages(MYSQL *connection, int user_id, Page **pages,
                      int *pages_count) {
  char query[512]; // Ensure sufficient size for your query
  snprintf(query, sizeof(query),
           "SELECT FileID, user_id, FilePath, FileSize, "
           "creation_date, LastModified, Title FROM pages WHERE user_id = %d",
           user_id);

  if (mysql_query(connection, query)) {
    fprintf(stderr, "Query failed: %s\n", mysql_error(connection));
    return -1;
  }

  MYSQL_RES *result = mysql_store_result(connection);
  if (!result) {
    fprintf(stderr, "Failed to retrieve result: %s\n", mysql_error(connection));
    return -1;
  }

  *pages_count = mysql_num_rows(result);
  if (*pages_count == 0) {
    mysql_free_result(result);
    return 0; // No pages found for user
  }

  *pages = malloc(sizeof(Page) * (*pages_count));
  if (!*pages) {
    fprintf(stderr, "Memory allocation failed\n");
    mysql_free_result(result);
    return -1;
  }

  MYSQL_ROW row;
  int i = 0;
  while ((row = mysql_fetch_row(result))) {
    (*pages)[i].page_id = atoi(row[0]);
    (*pages)[i].user_id = row[1] ? atoi(row[1]) : 0;
    strncpy((*pages)[i].path, row[2], sizeof((*pages)[i].path) - 1);
    (*pages)[i].size = atoll(row[3]);
    strncpy((*pages)[i].creation_date, row[4] ? row[4] : "",
            sizeof((*pages)[i].creation_date) - 1);
    strncpy((*pages)[i].modified_date, row[5] ? row[5] : "",
            sizeof((*pages)[i].modified_date) - 1);
    strncpy((*pages)[i].title, row[6], sizeof((*pages)[i].title) - 1);

    // Ensure strings are null-terminated
    (*pages)[i].path[sizeof((*pages)[i].path) - 1] = '\0';
    (*pages)[i].creation_date[sizeof((*pages)[i].creation_date) - 1] = '\0';
    (*pages)[i].modified_date[sizeof((*pages)[i].modified_date) - 1] = '\0';
    (*pages)[i].title[sizeof((*pages)[i].title) - 1] = '\0';

    i++;
  }

  mysql_free_result(result);
  return 0;
}

int db_page_update(MYSQL *connection, int page_id, const char *new_title,
                   const char *new_path, const long long page_size) {

  char query[512];
  snprintf(query, sizeof(query),
           "UPDATE PAGES SET title = '%s', file_size = %lld, file_path = '%s' "
           "WHERE page_id = %d",
           new_title, page_size, new_path, page_id);

  if (mysql_query(connection, query)) {
    fprintf(stderr, "Error executing query: %s\n", mysql_error(connection));
    return -1;
  }

  return 0;
}
int db_page_get(MYSQL *connection, int page_id, Page *page) {
  char query[256];
  MYSQL_RES *result;
  MYSQL_ROW row;

  snprintf(query, sizeof(query),
           "SELECT page_id, owner_id, vault_id, file_path, file_size, "
           "creation_date, last_modified, title FROM PAGES WHERE page_id = %d",
           page_id);

  if (mysql_query(connection, query)) {
    fprintf(stderr, "Error executing query: %s\n", mysql_error(connection));
    return -1;
  }

  result = mysql_store_result(connection);
  if (!result) {
    fprintf(stderr, "Error storing result: %s\n", mysql_error(connection));
    return -1;
  }

  // Assuming 'page' is a pointer to a Page struct and is already allocated
  if ((row = mysql_fetch_row(result))) {
    page->page_id = atoi(row[0]);
    page->user_id = atoi(row[1]); // Assuming 'user_id' in the struct maps
                                  // to 'owner_id' in the table
    page->vault_id = atoi(row[2]);

    // Ensure strings are safely copied and null-terminated
    strncpy(page->path, row[3], sizeof(page->path) - 1);
    page->path[sizeof(page->path) - 1] = '\0';

    page->size = atoll(row[4]);

    // 'creation_date' is a TIMESTAMP which will always have a value, so
    // direct copy is safe
    strncpy(page->creation_date, row[5], sizeof(page->creation_date) - 1);
    page->creation_date[sizeof(page->creation_date) - 1] = '\0';

    // 'last_modified' could be NULL if not set, though unlikely due to
    // DEFAULT_GENERATED on update
    if (row[6]) {
      strncpy(page->modified_date, row[6], sizeof(page->modified_date) - 1);
      page->modified_date[sizeof(page->modified_date) - 1] = '\0';
    } else {
      page->modified_date[0] =
          '\0'; // Handle potential NULL for 'last_modified'
    }

    strncpy(page->title, row[7], sizeof(page->title) - 1);
    page->title[sizeof(page->title) - 1] = '\0';
  } else {
    mysql_free_result(result); // Free result if no row is fetched or on error
    return -1; // Indicate failure (no row found or error occurred)
  }

  mysql_free_result(
      result); // Always clean up after you're done with the result set
  return 0;    // Indicate success
}

#include <mysql.h>
#include <stdio.h>

int db_session_get(MYSQL *connection, const char *token, Session *session) {
  // Construct the SQL query to select the session with the given token.
  printf("token: %s\n", token);
  printf("strlen: %lu\n", strlen(token));

  char query[516];
  snprintf(query, sizeof(query),
           "SELECT user_id, token, created_at FROM sessions WHERE token='%s'",
           token);

  // Execute the SQL query.
  if (mysql_query(connection, query)) {
    fprintf(stderr, "MySQL error: %s\n", mysql_error(connection));
    return -1;
  }

  // Store the result from the query.
  MYSQL_RES *result = mysql_store_result(connection);
  if (result == NULL) {
    fprintf(stderr, "MySQL error: %s\n", mysql_error(connection));
    return -1;
  }

  // Get the number of fields in the row.
  int num_fields = mysql_num_fields(result);

  // Fetch the row from the result set.
  MYSQL_ROW row = mysql_fetch_row(result);
  if (row) {
    // Make sure all the expected fields (Token, UserID, CreatedAt) are
    // present.
    if (num_fields >= 3) {
      // Convert the UserID to an integer and assign it to the session
      // struct.
      session->user_id = atoi(row[0]);

      // Copy the Token from the row to the session struct.
      strncpy(session->token, row[1], 33);

      // Copy the CreatedAt date from the row to the session struct.
      strncpy(session->created_date, row[2], sizeof(session->created_date) - 1);
      session->created_date[sizeof(session->created_date) - 1] =
          '\0'; // Ensure null-termination.
    } else {
      fprintf(stderr, "Error: Not all fields were retrieved.\n");
      mysql_free_result(result);
      return -1;
    }
  } else {
    printf("No session with the token %s was found.\n", token);
    mysql_free_result(result);
    return -1;
  }

  // Clean up.
  mysql_free_result(result);

  // If we've gotten this far, the session exists and the struct is filled
  // out.
  printf("Session found.\n");
  return 0;
}

int db_user_get_vaults(MYSQL *connection, int user_id, int **vault_ids,
                       int *vaults_total) {
  // Construct the query string
  char query[256];
  snprintf(query, sizeof(query),
           "SELECT vault_id FROM Vaults WHERE owner_id = %d", user_id);

  // Execute the query
  if (mysql_query(connection, query)) {
    fprintf(stderr, "Query execution error: %s\n", mysql_error(connection));
    return -1;
  }

  MYSQL_RES *result = mysql_store_result(connection);
  if (!result) {
    fprintf(stderr, "Failed to store result: %s\n", mysql_error(connection));
    return -1;
  }

  int num_vaults = mysql_num_rows(result);
  if (num_vaults > 0) {
    *vault_ids = malloc(num_vaults * sizeof(int));
    if (!*vault_ids) {
      fprintf(stderr, "Memory allocation failed\n");
      mysql_free_result(result);
      return -1;
    }

    MYSQL_ROW row;
    int i = 0;
    while ((row = mysql_fetch_row(result))) {
      (*vault_ids)[i++] = atoi(row[0]);
    }
  } else {
    *vault_ids = NULL;
  }

  mysql_free_result(result);
  *vaults_total = num_vaults;
  return 0;
}
int db_get_user_id(MYSQL *connection, const char *email, int *user_id) {

  printf("getting user: %s\n", email);
  char query[1024];
  snprintf(query, sizeof(query),
           "SELECT user_id, email, password, creation_date, storage_limit "
           "FROM Users WHERE email = '%s'",
           email);

  if (mysql_query(connection, query)) {
    fprintf(stderr, "%s\n", mysql_error(connection));
    printf("Failed to execute query\n");
    return -1;
  }

  MYSQL_RES *result = mysql_store_result(connection);
  if (result == NULL) {
    printf("Failed to store result\n");
    fprintf(stderr, "%s\n", mysql_error(connection));
    return -1;
  }

  MYSQL_ROW row;
  if ((row = mysql_fetch_row(result))) {
    *user_id = atoi(row[0]);
    return 0;
  } else {
    mysql_free_result(result);
    return 1;
  }
}

int db_session_create(MYSQL *connection, int user_id, const char *token) {

  char query[1024];
  snprintf(query, sizeof(query),
           "INSERT INTO Sessions (user_id, token) VALUES (%d, '%s') ON "
           "DUPLICATE KEY UPDATE token = VALUES(token)",
           user_id, token);

  if (mysql_query(connection, query)) {
    fprintf(stderr, "MySQL error: %s\n", mysql_error(connection));
    return -1;
  }

  printf("Session created or updated for user %d with token: %s\n", user_id,
         token);
  return 0;
}

int db_session_delete(MYSQL *connection, const char *token) {

  char query[256];
  snprintf(query, sizeof(query), "DELETE FROM sessions WHERE token='%s'",
           token);

  if (mysql_query(connection, query)) {
    fprintf(stderr, "MySQL error: %s\n", mysql_error(connection));
    return -1;
  }

  // Check if the deletion was successful.
  if (mysql_affected_rows(connection) == 0) {
    printf("No session with the token '%s' was found to delete.\n", token);
    return -1;
  } else {
    printf("Session with token '%s' deleted successfully.\n", token);
    return 0;
  }
}

int db_user_delete_sessions(MYSQL *connection, int user_id) {
  // Construct the SQL query to delete all pages for the given user_id.
  char query[256];
  snprintf(query, sizeof(query), "DELETE FROM SESSIONS WHERE user_id = %d",
           user_id);

  // Execute the SQL query.
  if (mysql_query(connection, query)) {
    fprintf(stderr, "MySQL error: %s\n", mysql_error(connection));
    return -1; // Return -1 to indicate failure.
  }
  return 0;
}

int db_user_delete_pages(MYSQL *connection, int user_id) {
  // Construct the SQL query to delete all pages for the given user_id.
  char query[256];
  snprintf(query, sizeof(query), "DELETE FROM PAGES WHERE user_id = %d",
           user_id);

  // Execute the SQL query.
  if (mysql_query(connection, query)) {
    fprintf(stderr, "MySQL error: %s\n", mysql_error(connection));
    return -1; // Return -1 to indicate failure.
  }

  // Check how many rows were affected.
  my_ulonglong affected_rows = mysql_affected_rows(connection);
  if (affected_rows == 0) {
    // No rows were affected, indicating no pages exist for the user_id or
    // deletion was not needed.
    printf("No pages found or deleted for user ID %d.\n", user_id);
  } else {
    // Rows were deleted.
    printf("%lu pages deleted for user ID %d.\n", affected_rows, user_id);
  }

  return 0; // Return 0 to indicate success.
}

// TESTING
//

int db_create_user(const char *email, const char *password, int storage_limit) {

  MYSQL *connection = NULL;
  if (db_connection_get(&connection) != 0)
    return -1;

  char query[256];

  snprintf(query, sizeof(query),
           "INSERT INTO Users (email, password, storage_limit"
           "VALUES ('%s', '%s', NOW(), %d)",
           email, password, 2000);

  if (mysql_query(connection, query)) {
    fprintf(stderr, "%s\n", mysql_error(connection));
    return -1;
  }

  db_connection_close(connection);

  return 0;
}

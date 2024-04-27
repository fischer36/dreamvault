#pragma once
#include "http_parser.h"

#include <mysql.h>
#include <stdbool.h>

typedef struct
{
    int user_id;
    char email[256];
    char password[256];
    char creation_date[256];
    long long int storage_limit;
} User;
typedef struct
{
    int page_id;
    int vault_id;
    int user_id;
    char path[256];
    long long int size;
    char creation_date[256];
    char modified_date[256];
    char title[256];
} Page;

typedef struct
{
    int user_id;
    char token[33];
    char created_date[20];
} Session;

typedef struct
{
    int vault_id;
    int owner_id;
    char name[256];
} Vault;

int db_page_get(MYSQL *connection, int page_id, Page *page);
int db_page_delete(MYSQL *connection, int page_id);
int db_page_create(MYSQL *connection, const int user_id, const int vault_id,
                   const char *title);
int db_page_update(MYSQL *connection, int page_id, const char *new_title,
                   const char *new_path, const long long data_size);

int db_pages_in_vault(MYSQL *connection, int vault_id, Page **pages,
                      int *pages_count);
int db_user_get_pages(MYSQL *connection, int user_id,
                      Page **pages, int *pages_count);

int db_user_get_vaults(MYSQL *connection, int user_id, int **vault_ids,
                       int *total_vaults);
int db_user_get(MYSQL *connection, const int user_id, User *user);
int db_user_delete(MYSQL *connection, int user_id);
int db_user_create(MYSQL *connection, const char email[256],
                   const char password[256], int storage_limit);
int db_user_delete_pages(MYSQL *connection, int user_id);
int db_user_delete_sessions(MYSQL *connection, int user_id);

int db_connection_get(MYSQL **db_connection);
int db_connection_close(MYSQL *connection);

int db_session_get(MYSQL *connection, const char *token, Session *session);
int db_session_delete(MYSQL *connection, const char *token);
int db_session_create(MYSQL *connection, int user_id, const char *token);

int db_vault_create(MYSQL *connection, int user_id,
                    const char vault_name[SIZE_VAULT_NAME]);
int db_vault_delete(MYSQL *connection, const int vault_id);
int db_vault_get(MYSQL *connection, const int vault_id, Vault *vault);
bool db_does_user_exists(MYSQL *connection, const char *email);
int db_get_user_id(MYSQL *connection, const char email[256], int *user_id);

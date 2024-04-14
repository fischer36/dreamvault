#pragma once
#include "mysql/mysql.h"
#include "util.h"
#include <stdio.h>
#include <string.h>

int insert_user(char *email, char *password);
int delete_user(int user_id);

int insert_session(int user_id, char *token);
int delete_session(char *token);

int insert_vault(int user_id, char *title);
int delete_vault(int vault_id);

int insert_page(int user_id, int vault_id, char *file_path, char *title);
int delete_page(int page_id);

int get_user_vaults(int user_id, int **vaults, int *vault_count);
int get_user_pages(int user_id);
int get_vault_pages(int vault_id);

int validate_token(char token[33], int *owner_user_id);
int get_user_id(const char *email, int *user_id, char *password);

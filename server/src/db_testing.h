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
int get_vault_pages(int vault_id, int **pages, int *page_count);

int validate_token(char token[33], int *owner_user_id);
int validate_vault(int user_id, int vault_id);
int validate_page(int user_id, int page_id);

int get_user_id(const char *email, int *user_id, char *password);

int get_vault_info(int vault_id, char *name, int *page_count);

int get_page_info(int page_id, char title[256], char file_path[256],
                  int *file_size);

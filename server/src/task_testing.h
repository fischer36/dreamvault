#pragma once

struct HTTP_RESPONSE {
  char code[65];
  char headers[513];
  char body[513];
};

struct HTTP_RESPONSE t_login(const char *email, const char *password);
struct HTTP_RESPONSE t_logout(char *token);

struct HTTP_RESPONSE t_register(char *email, char *password);
struct HTTP_RESPONSE t_unregister(char *token);

struct HTTP_RESPONSE t_users_user_vaults_get(char *token);
struct HTTP_RESPONSE t_users_user_vaults_post(char *token, char *title);

struct HTTP_RESPONSE t_users_user_vaults_vault_get(int user_id, int vault_id);
struct HTTP_RESPONSE t_users_user_vaults_vault_delete(int user_id,
                                                      int vault_id);

struct HTTP_RESPONSE t_users_user_vaults_vault_pages_get(int user_id,
                                                         int vault_id);
struct HTTP_RESPONSE
t_users_user_vaults_vault_pages_post(int user_id, int vault_id, char *file_path,
                                     char *title, char *content);

// This only gets page_info. Need to actually put the file contents in body
struct HTTP_RESPONSE t_users_user_vaults_vault_pages_page_get(int user_id,
                                                              int vault_id,
                                                              int page_id);

// TODO !
struct HTTP_RESPONSE t_users_user_vaults_vault_pages_page_patch(
    int user_id, int vault_id, int page_id, char *new_title, char *new_content);

// This only deletes it from the database, need to get the file_path and
// actually remove the file
struct HTTP_RESPONSE t_users_user_vaults_vault_pages_page_delete(int user_id,
                                                                 int page_id);

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

struct HTTP_RESPONSE t_users_user_vaults_vault_get();
struct HTTP_RESPONSE t_users_user_vaults_vault_delete();

struct HTTP_RESPONSE t_users_user_vaults_vault_pages_get();
struct HTTP_RESPONSE t_users_user_vaults_vault_pages_post();

struct HTTP_RESPONSE t_users_user_vaults_vault_pages_page_get();
struct HTTP_RESPONSE t_users_user_vaults_vault_pages_page_patch();
struct HTTP_RESPONSE t_users_user_vaults_vault_pages_page_delete();

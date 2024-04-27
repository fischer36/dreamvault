#pragma once
#include "http_xd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct HTTP_RESPONSE t_login(char buffer[1024]);
struct HTTP_RESPONSE t_logout(char *token);

struct HTTP_RESPONSE t_register(const char request[1024]);
struct HTTP_RESPONSE t_unregister(char *token);

struct HTTP_RESPONSE t_page_delete(int page_id, char buffer[1024]);
struct HTTP_RESPONSE t_page_write(int page_id, char request[1024]);
struct HTTP_RESPONSE t_page_create(char request[1024]);
struct HTTP_RESPONSE t_page_read(int page_id, char buffer[1024]);

// Fills out response body with a list of page ids on success.
struct HTTP_RESPONSE t_get_user_pages(int user_id);

// The following are old functions
struct HTTP_RESPONSE t_users_user_vaults_get(char *token);
struct HTTP_RESPONSE t_users_user_vaults_post(char *token, char *title);

struct HTTP_RESPONSE t_users_user_vaults_vault_get(int user_id, int vault_id);
struct HTTP_RESPONSE t_users_user_vaults_vault_delete(int user_id,
                                                      int vault_id);

struct HTTP_RESPONSE t_users_user_vaults_vault_pages_get(int user_id,
                                                         int vault_id);
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

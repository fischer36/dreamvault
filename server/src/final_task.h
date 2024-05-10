#pragma once
#include "final_http.h"

struct HTTP_RESPONSE t_invalid(char *error_msg);

struct HTTP_RESPONSE t_login(char email[EMAIL_LENGTH], char password[PASSWORD_LENGTH]);
struct HTTP_RESPONSE t_register(char email[EMAIL_LENGTH], char password[PASSWORD_LENGTH]);
struct HTTP_RESPONSE t_logout(char token[TOKEN_LENGTH]);
struct HTTP_RESPONSE t_unregister(char token[TOKEN_LENGTH]);

struct HTTP_RESPONSE t_get_user_pages(int user_id);
struct HTTP_RESPONSE t_page_delete(int user_id, int page_id);
struct HTTP_RESPONSE t_page_create(int user_id, char title[PAGE_TITLE_LENGTH], long modified, char content[PAGE_BODY_LENGTH]);

struct HTTP_RESPONSE t_page_write(int user_id, int page_id, const char title[PAGE_TITLE_LENGTH], char content[PAGE_BODY_LENGTH]);
struct HTTP_RESPONSE t_page_read(int user_id, int page_id);
struct HTTP_RESPONSE t_get_user(char token[TOKEN_LENGTH]);

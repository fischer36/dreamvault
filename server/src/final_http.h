#pragma once
#include "final_api.h"

struct HTTP_RESPONSE {
    char code[65];
    char headers[513];
    char body[513];
};

int get_value(const char *buffer, const char *key, char *value, int length);
enum Method { M_GET, M_POST, M_DELETE, M_PATCH, M_INVALID };

struct Uri {
    enum Type { U_REGISTER, U_UNREGISTER, U_LOGIN, U_LOGOUT, U_USERS, U_USER, U_USER_PAGES, U_USER_PAGE, U_INVALID } type;

    union Union {

        struct {
            char email[EMAIL_LENGTH];
            char password[EMAIL_LENGTH];
        } UserCombo; // /register and /login

        struct {
            char token[TOKEN_LENGTH];
        } Token; // /unregister and /logout

        struct {
            int user_id; // /user/<user_id>
            char token[TOKEN_LENGTH];
        } User;

        struct {
            int user_id; // /users/<user_id>
            int page_id; // /users/<user_id>/pages/<page_id>
            char token[TOKEN_LENGTH];
        } Page;

        char Invalid[512]; // Everything that has no corresponding function, this
                           // contains error message
    } Union;
};

enum Method http_method(const char buffer[1024]);
struct Uri http_uri(const char request[1024]);

int seralize_http_response(const struct HTTP_RESPONSE resp_struct, char resp[1024]);

int parse_page(const char buffer[1024], char title[PAGE_TITLE_LENGTH], char body[PAGE_BODY_LENGTH]);

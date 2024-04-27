#include "final_http.h"
#include "final_api.h"
#include "final_task.h"
#include <stdio.h>
#include <string.h>

int seralize_http_response(const struct HTTP_RESPONSE resp_struct, char resp[1024]) {
    long total_size = strlen(resp_struct.code) + strlen(resp_struct.headers) + strlen(resp_struct.body);

    if (total_size > 1024) {
        resp[0] = '\0';
        return -1;
    }

    sprintf(resp, "%s%s\r\n%s", resp_struct.code, resp_struct.headers, resp_struct.body);
    return 0;
}

int get_value(const char *buffer, const char *key, char *value, int length) {
    const char *value_start = strstr(buffer, key);
    // printf("Value start: %s\n", value_start);
    // printf("Key: %s\n", key);
    // printf("length %d", length);
    if (value_start == NULL) {

        return -1;
    }
    value_start += strlen(key);
    value_start += 1;

    if (value_start == NULL) {
        return -1;
    }

    char *value_end = strstr(value_start, "\r\n");
    if (value_end == NULL) {
        return -1;
    }

    long value_length = value_end - value_start;
    if (value_length < 3 || value_length >= length) {
        return -1;
    }

    strncpy(value, value_start, value_length);
    value[value_length] = '\0';
    return 0;
}

static int verify_token(const char *buffer, char token[TOKEN_LENGTH]) {

    if (get_value(buffer, "Authentication:", token, TOKEN_LENGTH) != 0) {

        return -1;
    }
    if (strlen(token) == 0) {
        return -1;
    }
    return 0;
}

static int verify_usercombo(const char *buffer, char email[EMAIL_LENGTH], char password[PASSWORD_LENGTH]) {

    if (get_value(buffer, "email:", email, EMAIL_LENGTH) || get_value(buffer, "password:", password, PASSWORD_LENGTH) != 0) {
        return -1;
    }

    if (strlen(email) == 0 || strlen(password) == 0) {
        return -1;
    }
    return 0;
}

enum Method http_method(const char buffer[1024]) {
    const char *method_start = buffer;
    const char *method_end = strstr(buffer, " ");

    if (method_end == NULL || method_start == NULL) {
        return M_INVALID;
    }

    int method_length = method_end - method_start;

    if (method_length == 0 || method_length >= 10) {
        return M_INVALID;
    }

    char method_str[10];
    strncpy(method_str, method_start, method_length);
    method_str[method_length] = '\0';

    if (strcmp(method_str, "GET") == 0) {
        return M_GET;
    } else if (strcmp(method_str, "POST") == 0) {
        return M_POST;
    } else if (strcmp(method_str, "PATCH") == 0) {
        return M_PATCH;
    } else if (strcmp(method_str, "DELETE") == 0) {
        return M_DELETE;
    } else {
        return M_INVALID;
    }
}

struct Uri http_uri(const char request[1024]) {
    char *uri_start = strstr(request, " ");
    uri_start += 1;
    char *uri_end = strstr(uri_start, " ");
    char uri_str[512];
    strncpy(uri_str, uri_start, uri_end - uri_start);

    uri_str[uri_end - uri_start] = '\0';
    printf("URI: %s\n", uri_str);
    int slash_count = 0;
    for (int i = 0; i < uri_end - uri_start; i++) {
        if (uri_str[i] == '/') {
            slash_count++;
        }
    }

    struct Uri uri = {0};

    if (slash_count == 1) {

        if (strcmp(uri_str, "/register") == 0) {
            uri.type =
                (verify_usercombo(request, uri.Union.UserCombo.email, uri.Union.UserCombo.password)) ? U_INVALID : U_REGISTER;
            return uri;
        }

        if (strcmp(uri_str, "/unregister") == 0) {
            uri.type = (verify_token(request, uri.Union.Token.token)) ? U_INVALID : U_UNREGISTER;
            return uri;
        }

        if (strcmp(uri_str, "/login") == 0) {
            uri.type = (verify_usercombo(request, uri.Union.UserCombo.email, uri.Union.UserCombo.password)) ? U_INVALID : U_LOGIN;
            return uri;
        }

        if (strcmp(uri_str, "/logout") == 0) {
            uri.type = (verify_token(request, uri.Union.Token.token)) ? U_INVALID : U_LOGOUT;
            return uri;
        }

        if (strcmp(uri_str, "/users") == 0) {
            uri.type = (verify_token(request, uri.Union.Token.token)) ? U_INVALID : U_USERS;
            return uri;
        }
    }
    if (slash_count == 2) {
        int user_id;
        if (sscanf(uri_str, "/users/%d", &user_id) == 1) {
            uri.Union.User.user_id = user_id;
            uri.type = (verify_token(request, uri.Union.User.token)) ? U_INVALID : U_USER;
            return uri;
        }
    }

    if (slash_count == 3) {
        printf("hello");
        int user_id;
        if (sscanf(uri_str, "/users/%d/pages", &user_id) == 1) {
            uri.Union.User.user_id = user_id;
            uri.type = (verify_token(request, uri.Union.User.token)) ? U_INVALID : U_USER_PAGES;
            printf("TOKEN %s, ", uri.Union.User.token);
            return uri;
        }
    }

    if (slash_count == 4) {
        int user_id, page_id;
        if (sscanf(uri_str, "/users/%d/pages/%d", &user_id, &page_id) == 2) {

            uri.Union.Page.user_id = user_id;
            uri.Union.Page.page_id = page_id;
            uri.type = (verify_token(request, uri.Union.Page.token)) ? U_INVALID : U_USER_PAGE;
            return uri;
        }
    }

    uri.type = U_INVALID;
    strcpy(uri.Union.Invalid, "Invalid URI\n");
    uri.Union.Invalid[strlen("Invalid URI\n")] = '\0';
    return uri;
}

int parse_page(const char buffer[1024], char title[PAGE_TITLE_LENGTH], char body[PAGE_BODY_LENGTH]) {

    if (get_value(buffer, "page_title:", title, PAGE_TITLE_LENGTH) ||
        get_value(buffer, "page_body:", body, PAGE_BODY_LENGTH) != 0) {
        puts("not ok");
        return -1;
    }
    puts("ok");
    return 0;
}

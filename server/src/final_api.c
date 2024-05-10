// #include "final_api.h"
#include "final_api.h"
#include "final_db.h"
#include "final_http.h"
#include "final_task.h"
#include <stdio.h>

static int verify_page_id(int user_id, int page_id) {
    if (validate_page(user_id, page_id)) {
        return -1;
    }
    return 0;
}
static int verify_user_id(const char token[TOKEN_LENGTH], int user_id) {
    int actual_token_owner_user_id = 0;
    if (validate_token(token, &actual_token_owner_user_id)) {
        return -1;
    }

    if (actual_token_owner_user_id != user_id) {
        return -1;
    }
    return 0;
}

void request_handler(const char request[1024], char response[1024]) {

    struct HTTP_RESPONSE response_struct = t_invalid("Bad Requesst");
    enum Method method = http_method(request);
    struct Uri uri = http_uri(request);
    printf("uri type %d ", uri.type);
    printf(" method %d", method);
    switch (uri.type) {
    case U_REGISTER:
        if (method == M_POST) {
            printf("Register - email: %s, password: %s\n", uri.Union.UserCombo.email, uri.Union.UserCombo.password);
            response_struct = t_register(uri.Union.UserCombo.email, uri.Union.UserCombo.password);
        }
        break;
    case U_UNREGISTER:
        if (method == M_POST) {
            printf("Unregister - token: %s\n", uri.Union.Token.token);
            response_struct = t_unregister(uri.Union.Token.token);
        }
        break;
    case U_LOGIN:
        if (method == M_POST) {
            printf("Login - email: %s, password: %s\n", uri.Union.UserCombo.email, uri.Union.UserCombo.password);
            response_struct = t_login(uri.Union.UserCombo.email, uri.Union.UserCombo.password);
        }
        break;
    case U_LOGOUT:
        if (method == M_POST) {
            printf("Logout - token: %s\n", uri.Union.Token.token);
            response_struct = t_logout(uri.Union.Token.token);
        }
        break;
    case U_USERS:
        if (method == M_GET) {
            printf("Users GET\n");
            response_struct = t_get_user(uri.Union.Token.token);
        }
        break;
    case U_USER_PAGES:
        if (verify_user_id(uri.Union.User.token, uri.Union.User.user_id) != 0) {
            response_struct = t_invalid("Invalid token");
            break;
        }
        if (method == M_POST) {
            printf("User pages POST - user id: %d, \n", uri.Union.Page.user_id);
            char title[128];
            char content[1024];
            long modified = 0;
            response_struct = (parse_page(request, title, &modified, content))
                                  ? t_invalid("Invalid request body")
                                  : t_page_create(uri.Union.Page.user_id, title, modified, content);
        }
        if (method == M_GET) {
            printf("User pages GET - user id: %d, \n", uri.Union.Page.user_id);
            response_struct = t_get_user_pages(uri.Union.Page.user_id);
        }
        break;
    case U_USER_PAGE:
        if (verify_user_id(uri.Union.Page.token, uri.Union.Page.user_id) != 0) {
            response_struct = t_invalid("Invalid token");
            break;
        }
        if (method == M_GET) {
            printf("Page GET - user id: %d, page id: %d\n", uri.Union.Page.user_id, uri.Union.Page.page_id);
            response_struct = t_page_read(uri.Union.Page.user_id, uri.Union.Page.page_id);
        } else if (method == M_PATCH) {
            printf("Page PATCH  - user id: %d, page id: %d\n", uri.Union.Page.user_id, uri.Union.Page.page_id);
            char title[128];
            char content[1024];
            long modified = 0;
            response_struct = (parse_page(request, title, &modified, content))
                                  ? t_invalid("Invalid request body")
                                  : t_page_write(uri.Union.Page.user_id, uri.Union.Page.page_id, title, content);

        } else if (method == M_DELETE) {
            printf("Page Delete - user id: %d, page id: %d\n", uri.Union.Page.user_id, uri.Union.Page.page_id);
            response_struct = t_page_delete(uri.Union.Page.user_id, uri.Union.Page.page_id);
        }
        break;
    case U_INVALID:
        response_struct = t_invalid(uri.Union.Invalid);
        break;
    }
    if (seralize_http_response(response_struct, response) != 0) {
        printf("Seralizing response failed\n");
    }
}

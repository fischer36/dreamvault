#include "final_api.h"
#include "final_db.h"
#include "final_http.h"
#include "final_sys.h"

#include <stdio.h>
#include <string.h>

struct HTTP_RESPONSE t_get_user(char token[TOKEN_LENGTH]) {

    printf("TOKEN T GET USER %s\n", token);
    struct HTTP_RESPONSE response = {
        .code = "",
        .body = "",
        .headers = "",
    };
    strcpy(response.code, "500 Internal Server Error\r\n");
    int user_id = 0;
    printf("Token: %s\n", token);
    if (get_user(token, &user_id)) {
        strcpy(response.code, "404 Not Found\r\n");
        strcpy(response.body, "Unable to get user id");
        return response;
    }

    strcpy(response.code, "200 OK\r\n");
    snprintf(response.body, sizeof(response.body), "User_id: %d\r\n", user_id);
    return response;
}
struct HTTP_RESPONSE t_get_user_pages(int user_id) {

    struct HTTP_RESPONSE response = {
        .code = "",
        .body = "",
        .headers = "",
    };
    struct Page *pages = NULL;
    int page_count = 0;
    strcpy(response.code, "500 Internal Server Error\r\n");
    if (get_user_pages(user_id, &pages, &page_count)) {
        strcpy(response.code, "404 Not Found\r\n");
        strcpy(response.body, "Unable to get user pages");
        return response;
    }
    // for (int i = 0; i < page_count; i++) {
    //     printf("%d\n", page_ids[i]);
    // }
    // printf("page count %d %lu\n", page_count, page_count * sizeof(struct Page));
    // if (page_count * sizeof(struct Page) > 512) {
    //
    //     // printf("To many pages to fit in response body \r\n");
    //     strcpy(response.code, "500 Internal Error\r\n");
    //     strcpy(response.body, "To many pages to fit in response body \r\n");
    //     free(pages);
    //     return response;
    // }
    strcpy(response.code, "200 OK\r\n");
    char linexd[70];
    sprintf(linexd, "Pages (%d): \r\n", page_count);
    strcpy(response.body, linexd);

    char line[70];
    for (int i = 0; i < page_count && i < 32; i++) {
        snprintf(line, sizeof(line), "id: %d, modified: %ld\n", pages[i].id, pages[i].modified);
        strcat(response.body, line);
    }
    free(pages);
    return response;
}

struct HTTP_RESPONSE t_login(char email[EMAIL_LENGTH], char password[PASSWORD_LENGTH]) {

    struct HTTP_RESPONSE response = {
        .code = "",
        .body = "",
        .headers = "",
    };

    strcpy(response.code, "500 Internal Server Error\r\n");
    int user_id = 0;
    char hashed_password[65] = {0};

    if (get_user_id(email, &user_id, hashed_password) != 0) {
        strcpy(response.code, "404 Not Found\r\n");
        strcpy(response.body, "User not found");
        return response;
    }

    if (util_compare_hash(password, hashed_password) != 0) {
        strcpy(response.code, "401 Unauthorized\r\n");
        strcpy(response.body, "Invalid password");
        return response;
    }

    char token[TOKEN_LENGTH] = {0};
    if (util_generate_token(token) != 0) {
        strcpy(response.code, "500 Internal Server Error\r\n");
        strcpy(response.body, "Failed to generate token");
        return response;
    }

    if (insert_session(user_id, token) != 0) {
        strcpy(response.code, "500 Internal Server Error\r\n");
        strcpy(response.body, "Failed to create session");
        return response;
    }

    strcpy(response.code, "200 OK\r\n");
    strcpy(response.body, "Login successful\r\n");
    snprintf(response.headers, sizeof(response.headers), "Token: %s\nUser_id:%d\r\n", token, user_id);

    return response;
}

struct HTTP_RESPONSE t_register(char email[EMAIL_LENGTH], char password[PASSWORD_LENGTH]) {

    struct HTTP_RESPONSE response = {
        .code = "",
        .body = "",
        .headers = "",
    };

    strcpy(response.code, "500 Internal Server Error\r\n");

    char hashed_password[65] = {0};
    if (util_hash(password, hashed_password)) {
        strcpy(response.code, "500 Internal Server Error\r\n");
        strcpy(response.body, "Failed to hash password");
        return response;
    }

    if (insert_user(email, hashed_password)) {
        strcpy(response.code, "404 Bad Request\r\n");
        strcpy(response.body, "User already exists");
        return response;
    }
    strcpy(response.code, "200 OK\r\n");
    strcpy(response.body, "Register successful");
    return response;
}

struct HTTP_RESPONSE t_logout(char token[TOKEN_LENGTH]) {

    struct HTTP_RESPONSE response = {
        .code = "",
        .body = "",
        .headers = "",
    };

    // if (delete_session(token)) {
    //     strcpy(response.code, "400 Bad Request\r\n");
    //     strcpy(response.body, "Invalid token");
    //     return response;
    // }

    strcpy(response.code, "200 OK\r\n");
    strcpy(response.body, "Logout successful");
    return response;
}

struct HTTP_RESPONSE t_unregister(char token[TOKEN_LENGTH]) {

    struct HTTP_RESPONSE response = {
        .code = "",
        .body = "",
        .headers = "",
    };

    strcpy(response.code, "500 Internal Server Error\r\n");

    int user_id = 0;
    if (validate_token(token, &user_id)) {
        strcpy(response.code, "401 Unauthorized\r\n");
        strcpy(response.body, "Invalid token");
        return response;
    }

    if (delete_user(user_id)) {
        strcpy(response.code, "500 Internal Server Error\r\n");
        strcpy(response.body, "Unable to find user");
        return response;
    }
    strcpy(response.code, "200 OK\r\n");
    strcpy(response.body, "Unregister successful");
    return response;
}
struct HTTP_RESPONSE t_page_create(int user_id, char *title, long modified, char content[PAGE_BODY_LENGTH]) {
    struct HTTP_RESPONSE response = {
        .code = "",
        .body = "",
        .headers = "",
    };

    strcpy(response.code, "500 Internal Server Error\r\n");

    int page_increment = get_page_counter() + 1;
    if (page_increment == -1) {
        strcpy(response.code, "500 Internal Server Error\r\n");
        strcpy(response.body, "Unable to page count");
        return response;
    }

    if (sys_folder_exists("./pages") == 0) {
        if (sys_folder_create("./pages")) {
            strcpy(response.code, "500 Internal Server Error\r\n");
            strcpy(response.body, "Unable to create folder");
            return response;
        }
    }
    char file_path[255] = {0};

    snprintf(file_path, sizeof(file_path), "./pages/%d/%d.md", page_increment / 100, page_increment);
    // file_path[strlen()] = '\0';

    char *last_slash = strrchr(file_path, '/');
    last_slash[0] = '\0';
    printf("folder: %s\n", file_path);
    if (!sys_folder_exists(file_path)) {
        printf("Creatingf folder %s\n", file_path);
        if (sys_folder_create(file_path)) {
            strcpy(response.code, "500 Internal Server Error\r\n");
            strcpy(response.body, "Unable to create folder");
            return response;
        }
    }

    last_slash[0] = '/';

    if (sys_file_exists(file_path)) {
        strcpy(response.code, "500 Internal Server Error\r\n");
        strcpy(response.body, "Page already exists");
        return response;
    }

    if (sys_file_create(file_path)) {
        strcpy(response.code, "500 Internal Server Error\r\n");
        strcpy(response.body, "Unable to create file");
        return response;
    }

    if (sys_file_write(file_path, content)) {
        strcpy(response.code, "500 Internal Server Error\r\n");
        strcpy(response.body, "Unable to write to file");
        return response;
    }

    if (insert_page(user_id, 31, file_path, modified, title)) {
        strcpy(response.code, "500 Internal Server Error\r\n");
        strcpy(response.body, "Unable to create vault");
        return response;
    }
    strcpy(response.code, "200 OK\r\n");
    snprintf(response.body, sizeof(response.body), "Successfully created page with page_id: %d", page_increment);
    return response;
};

struct HTTP_RESPONSE t_page_read(int user_id, int page_id) {

    struct HTTP_RESPONSE response = {
        .code = "",
        .body = "",
        .headers = "",
    };

    strcpy(response.code, "500 Internal Server Error\r\n");

    int owner_id = 0;
    int file_size = 0;
    char title[256];
    char file_path[256];

    if (get_page_info(page_id, &owner_id, title, file_path, &file_size)) {
        strcpy(response.code, "500 Internal Server Error\r\n");
        strcpy(response.body, "Failed to get page");
        return response;
    }
    if (user_id != owner_id) {
        strcpy(response.code, "401 Unauthorized\r\n");
        strcpy(response.body, "Unauthorized user for page");
        return response;
    }

    if (!sys_file_exists(file_path)) {
        strcpy(response.code, "500 Internal Server Error\r\n");
        strcpy(response.body, "Error finding page path");
        return response;
    }
    size_t data_size = 0;
    char *data = NULL;
    if (sys_file_read(file_path, &data, &data_size)) {
        strcpy(response.code, "500 Internal Server Error\r\n");
        strcpy(response.body, "Error page from database");
        return response;
    }
    if (strlen(data) > 512) {
        printf("Page data is too big, shortening");
        data[512] = '\0';
    }
    int page_version = 0;
    get_page_version(page_id, &page_version);
    strcpy(response.code, "200 OK\r\n");
    snprintf(response.body, sizeof(response.body), "Page_title: %s\r\nPage_size: %lu\r\nPage_version: %d\r\nPage_content: %s\r\n",
             file_path, data_size, page_version, data);
    // strncpy(response.body, data, 512);
    if (data != NULL)
        free(data);
    return response;
}

struct HTTP_RESPONSE t_page_delete(int user_id, int page_id) {

    struct HTTP_RESPONSE response = {
        .code = "",
        .body = "",
        .headers = "",
    };

    strcpy(response.code, "500 Internal Server Error\r\n");

    int owner_id = 0;
    int file_size = 0;
    char title[256];
    char file_path[256];

    if (get_page_info(page_id, &owner_id, title, file_path, &file_size)) {
        strcpy(response.code, "500 Internal Server Error\r\n");
        strcpy(response.body, "Failed to get page");
        return response;
    }

    if (user_id != owner_id) {
        strcpy(response.code, "401 Unauthorized\r\n");
        strcpy(response.body, "Unauthorized user for page");
        return response;
    }

    if (sys_file_exists(file_path)) {
        if (sys_file_delete(file_path)) {
            strcpy(response.code, "500 Internal Server Error\r\n");
            strcpy(response.body, "Error deleting page");
            return response;
        }
    }

    if (delete_page(page_id)) {
        strcpy(response.code, "500 Internal Server Error\r\n");
        strcpy(response.body, "Error page from database");
        return response;
    }
    strcpy(response.code, "200 OK\r\n");
    strcpy(response.body, "Page deletion successful");
    return response;
}

struct HTTP_RESPONSE t_page_write(int user_id, int page_id, const char title[PAGE_TITLE_LENGTH], char content[PAGE_BODY_LENGTH]) {

    struct HTTP_RESPONSE response = {
        .code = "",
        .body = "",
        .headers = "",
    };

    strcpy(response.code, "500 Internal Server Error\r\n");

    char file_path[255] = {0};
    char page_title[255] = {0};
    int owner_id = 0;
    int page_size = 0;
    int modified = 0;
    if (get_page_info(page_id, &owner_id, page_title, file_path, &page_size)) {
        strcpy(response.code, "404 Bad Request\r\n");
        strcpy(response.body, "Page file not found in database");
        return response;
    }

    printf("OKASDAS %d %d", user_id, owner_id);
    if (user_id != owner_id) {
        strcpy(response.code, "401 Unauthorized\r\n");
        strcpy(response.body, "Unauthorized user for page");
        return response;
    }

    if (!sys_file_exists(file_path)) {
        strcpy(response.code, "404 Bad Request\r\n");
        strcpy(response.body, "Page file not found on system");
        return response;
    }

    if (sys_file_write(file_path, content)) {
        strcpy(response.code, "404 Bad Request\r\n");
        strcpy(response.body, "Error writing to file on system");
        return response;
    }

    int file_size = strlen(content);
    if (update_page(page_id, page_title, content, modified, file_size)) {
        strcpy(response.code, "500 Internal Server Error\r\n");
        strcpy(response.body, "Error updating page in database");
        return response;
    }
    strcpy(response.code, "200 OK\r\n");
    strcpy(response.body, "Successfully modified page");
    return response;
}

struct HTTP_RESPONSE t_invalid_2(char *error_code, char *error_msg) {

    struct HTTP_RESPONSE response = {
        .code = "",
        .body = "",
        .headers = "",
    };

    strcpy(response.code, error_code);
    strcpy(response.body, error_msg);

    return response;
}
struct HTTP_RESPONSE t_invalid(char *error_msg) {

    struct HTTP_RESPONSE response = {
        .code = "",
        .body = "",
        .headers = "",
    };

    strcpy(response.code, "400 Bad Request\r\n");
    strcpy(response.body, error_msg);
    return response;
}

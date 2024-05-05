#pragma once

#define EMAIL_LENGTH 20
#define PASSWORD_LENGTH 20
#define TOKEN_LENGTH 33
#define PAGE_TITLE_LENGTH 20
#define PAGE_BODY_LENGTH 1024

struct Page {
    int id;
    int version;
};

void request_handler(const char request[1024], char response[1024]);

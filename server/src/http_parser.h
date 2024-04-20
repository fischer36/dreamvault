#pragma once
#include <stddef.h>
#include <stdio.h>
#define SIZE_EMAIL 255
#define SIZE_PASSWORD 255
#define SIZE_REQUEST_LINE 255
#define SIZE_HEADERS 1024
#define SIZE_BODY 1024
#define SIZE_KEY 65
#define SIZE_VALUE 65
#define SIZE_TOKEN 33
#define SIZE_RESPONSE 1024
#define SIZE_VAULT_NAME 255
typedef struct {

  char key[SIZE_KEY];
  char value[SIZE_VALUE];
} Pair;

typedef enum {

  HTTP_OK = 200,
  HTTP_BAD_REQUEST = 400,
  HTTP_UNAUTHORIZED = 401,
  HTTP_NOT_FOUND = 404,
  HTTP_INTERNAL_SERVER_ERROR = 500,

} HttpResponseCode;

typedef struct {
  HttpResponseCode code;
  Pair headers_array[10];
  int header_count;
  char *body;
  size_t body_size;
} HttpResponse;

typedef enum { URI_USER, URI_VAULT, URI_PAGE, URI_REGISTER, URI_INVALID } Uri;

typedef enum {
  METHOD_GET,
  METHOD_POST,
  METHOD_DELETE,
  METHOD_PATCH,
  METHOD_INVALID,

} Method;

typedef struct {
  Method method;
  Uri uri;
  char uri_children[128];
  Pair *headers_array;
  int header_count;
  char *body;
} HttpRequest;

int parse_response(const HttpResponse source, char target[1024],
                   int *target_size);

int parse_request(const char *source, HttpRequest *target);

int parse_search_headers(const HttpRequest haystack, const char *needle,
                         char *result);
int parse_search_body(const char *body, const char *search_key, char *value,
                      int value_size);
int parse_extract_uri_child(const char *source, char *target);

int parse_curly_bracket_contents(const char *source, char *key, char **target);

int parse_delete_request(HttpRequest request);
int parse_delete_response(HttpResponse response);

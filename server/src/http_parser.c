#include "http_parser.h"
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int parse_curly_bracket_contents(const char *source, char *key, char *target)
{
    printf("parse_curly_bracket_contents - searching for key: %s\n", key);

    const char *start = strstr(source, key);
    if (start == NULL)
    {
        printf("The key %s was not found.\n", key);
        return 1;
    }

    // Move the pointer to the character after "text".
    start += strlen(key);

    // Find the opening '{'.
    const char *open_bracket = strchr(start, '{');
    if (open_bracket == NULL)
    {
        printf("Opening curly bracket '{' not found after key %s.\n", key);
        return 1;
    }
    // Move past the '{' character to start search for closing '}'.
    open_bracket++;

    const char *close_bracket = strrchr(open_bracket, '}');
    if (close_bracket == NULL)
    {
        printf("Closing curly bracket '}' not found.\n");
        return 1;
    }

    ptrdiff_t length = close_bracket - open_bracket;

    // Copy the content between the curly brackets.
    strncpy(target, open_bracket, length);
    // Null-terminate the target string.
    target[length] = '\0';

    return 0;
}

static int extract_body(const char *buffer, char **body)
{
    const char *start = buffer;
    const char *body_start = strstr(start, "\r\n\r\n");

    if (!body_start)
    {
        printf("[Error][extract_body] No CRLF found, unexpected format.\n");
        return 1;
    }

    body_start += 4;
    size_t body_length = strlen(body_start);
    *body = (char *)malloc(body_length + 1);
    if (!*body)
    {
        printf("Memory allocation for the body failed.\n");
        return -1;
    }

    strncpy(*body, body_start, body_length);
    (*body)[body_length] = '\0';

    return 0;
}
static int extract_headers(const char *buffer, Pair **headers,
                           int *header_count)
{

    const char *headers_start = strstr(buffer, "\r\n");
    if (!headers_start)
    {
        printf("[Error][extract_headers] No CRLF found, unexpected format.\n");
        return 1;
    }

    headers_start += 2;
    const char *headers_end = strstr(headers_start, "\r\n\r\n");
    if (!headers_end)
    {
        printf("[Error][extract_headers] No CRLF found, unexpected format.\n");
        return 1;
    }
    char headers_str[513] = {0};
    size_t headers_length = headers_end - headers_start;
    if (headers_length >= 513)
    {

        printf("[Error][extract_headers] Length Is To Big :%lu\n",
               headers_length);
        return 1;
    }

    strncpy(headers_str, headers_start, headers_length += 2);
    headers_str[headers_length] = '\0';
    // printf("Headers Size: %lu\n", headers_length);

    char *line_start = headers_str;
    char *line_end;
    char line_str[256];
    Pair temp_pairs[10] = {0};

    while (*line_start)
    {
        line_end = strchr(line_start, '\n');
        if (!line_end || *header_count == 9)
        {
            break;
        }
        int line_length = (line_end - line_start);
        strncpy(line_str, line_start, line_length);
        line_str[line_length] = '\0';

        char *semi_colon = strchr(line_start, ':');
        if (!semi_colon)
            break;

        Pair pair;
        strncpy(pair.key, line_start, semi_colon - line_start);
        pair.key[semi_colon - line_start] = '\0';
        semi_colon += 2;
        strncpy(pair.value, semi_colon, line_end - semi_colon);
        pair.value[line_end - semi_colon] = '\0';
        temp_pairs[*header_count] = pair;
        line_start = line_end + 1;
        *header_count += 1;
    }
    int i;
    *headers = (Pair *)malloc(*header_count * sizeof(Pair));
    if (*headers == NULL)
    {
        return -1;
    }
    for (i = 0; i < *header_count; i++)
    {
        (*headers)[i] = temp_pairs[i];
    }
    return 0;
}
static Method extract_method(const char *buffer)
{

    char method_str[16];
    const char *method_end = strstr(buffer, " ");
    size_t method_length;

    if (!buffer)
    {
        printf("[Error] No CRLF found, unexpected format.\n");
        return METHOD_INVALID;
    }
    method_length = (method_end - buffer);
    if (method_length >= 16)
    {
        printf("[Error] Method Length Is To Big :%lu\n", method_length);
        return METHOD_INVALID;
    }
    strncpy(method_str, buffer, method_length);
    method_str[method_length] = '\0';

    if (strcmp(method_str, "GET") == 0)
    {
        // printf("GET\n");
        return METHOD_GET;
    }
    if (strcmp(method_str, "POST") == 0)
    {
        // printf("POST\n");
        return METHOD_POST;
    }
    if (strcmp(method_str, "DELETE") == 0)
    {
        // printf("DELETE\n");
        return METHOD_DELETE;
    }
    if (strcmp(method_str, "PATCH") == 0)
    {
        // printf("PUT\n");
        return METHOD_PATCH;
    }

    printf("[Error] Invalid Method: %s\n", method_str);
    return METHOD_INVALID;
}

int parse_extract_uri_child(const char *source, char *target)
{
    const int MAX_URI_CHILD_SIZE = 127;
    char *uri_start = strstr(source, " ");
    if (!uri_start)
    {
        printf("Error: Space not found before URI.\n");
        return 1;
    }
    uri_start += 2;

    char *uri_end = strstr(uri_start, " ");
    if (!uri_end)
    {
        printf("Error: Second Space not found before URI.\n");
        return 1;
    }
    char uri_str[256];

    strncpy(uri_str, uri_start, uri_end - uri_start);
    uri_str[uri_end - uri_start] = '\0';

    char *second_slash = strchr(uri_str, '/');
    if (!second_slash)
    {
        printf("parse_extract_uri_child: no child found in %s \n", uri_str);
        return 1;
    }

    if (strlen(second_slash) < 1 ||
        (strlen(second_slash) > MAX_URI_CHILD_SIZE))
    {
        printf("Error: Invalid URI child size.\n");
        return 1;
    }

    strcpy(target, second_slash);
    target[strlen(second_slash)] = '\0';

    return 0;
}

static Uri extract_uri(const char *buffer)
{

    char *uri_start = strstr(buffer, " ");
    uri_start += 1;

    char *uri_end = strstr(uri_start, " ");

    char uri_str[128];

    strncpy(uri_str, uri_start, uri_end - uri_start);

    uri_str[uri_end - uri_start] = '\0';

    printf("URI: %s\n", uri_str);

    if (strncmp(uri_str, "/user", strlen("/user")) == 0)
        return URI_USER;

    if (strncmp(uri_str, "/page", strlen("/page")) == 0)
        return URI_PAGE;

    if (strncmp(uri_str, "/register", strlen("/register")) == 0)
        return URI_REGISTER;

    printf("[Error] Parse Extract Uri - Invalid URI: %s\n", uri_str);
    return URI_INVALID;
}

int parse_request(const char *buffer, HttpRequest *request)
{

    request->method = extract_method(buffer);
    if (request->method == METHOD_INVALID)
    {
        printf("Error parsing method\n");
        return 1;
    }

    request->uri = extract_uri(buffer);
    if (request->uri == URI_INVALID)
    {
        printf("Error parsing uri\n");
        return 1;
    }
    if (parse_extract_uri_child(buffer, request->uri_children) != 0)
    {
        printf("No child");
        request->uri_children[0] = '\0';
    }
    if (extract_headers(buffer, &(request->headers_array),
                        &(request->header_count)) != 0)
    {
        printf("Error parsing headers\n");
        return 1;
    }

    if (extract_body(buffer, &(request->body)) != 0)
    {
        printf("Error parsing body\n");
        free(request->headers_array);
        return 1;
    }

    return 0;
}

int parse_response(const HttpResponse source, char target[1024],
                   int *target_size)
{
    char *code_str = "";
    switch (source.code)
    {
    case HTTP_OK:
        code_str = "HTTP/1.1 200 OK\r\n";
        break;
    case HTTP_BAD_REQUEST:
        code_str = "HTTP/1.1 400 Bad Request\r\n";
        break;
    case HTTP_NOT_FOUND:
        code_str = "HTTP/1.1 404 Not Found\r\n";
        break;
    default:
        return -1;
    }

    strcpy(target, code_str);
    *target_size = strlen(code_str);

    if (source.body != NULL && source.body_size > 0)
    {

        char *body_token = "\r\n\r\ntoken:";
        int body_token_length = strlen(body_token);

        int total_body_size = body_token_length + source.body_size;

        if ((*target_size + total_body_size) >= 1024)
        {
            return -3;
        }

        memcpy(target + *target_size, body_token, body_token_length);
        *target_size += body_token_length;

        memcpy(target + *target_size, source.body, source.body_size);
        *target_size += source.body_size;
    }
    if (*target_size < 1024)
    {
        target[*target_size] = '\0';
    }
    else
    {
        return -4;
    }
    printf("Response: %s\n", target);
    return 0;
}

int parse_search_headers(const HttpRequest request, const char *search_key,
                         char *target)
{

    for (int i = 0; i < request.header_count; i++)
    {
        if (strcmp(request.headers_array[i].key, search_key) == 0)
        {
            // Key found, set target to its value.
            strcpy(target, request.headers_array[i].value);
            return 0;
        }
    }
    return -1;
}

int parse_search_body(const char *body, const char *search_key, char *value,
                      int max_value_size)
{

    if (body == NULL)
        return 1;
    char *key = strstr(body, search_key);
    if (key)
    {
        char *value_start = key + strlen(search_key);
        while (!isalnum((unsigned char)*value_start) && *value_start != '\0')
        {
            value_start++;
        }
        // while (*value_start == ' ' && *value_start != '\0') {
        //     value_start++;
        // }
        char *value_end = value_start;

        while (!isspace((unsigned char)*value_end) && *value_end != '"' &&
               *value_end != '\0')
        {
            value_end++;
        }

        int value_length = value_end - value_start;
        if (value_length <= 0 || value_length > max_value_size)
        {
            printf("Error: Not FOund! - Invalid value length\n");
            return -1;
        }
        strncpy(value, value_start, value_length);
        value[value_length] = '\0';
        return 0;
    }
    else
    {
        return -1; // Key not found
    }
}

int parse_delete_request(HttpRequest request)
{
    if (request.body != NULL)
        free(request.body);

    if (request.headers_array != NULL)
        free(request.headers_array);

    request.body = NULL;
    request.headers_array = NULL;
    return 0;
}

int parse_delete_response(HttpResponse response)
{
    if (response.body != NULL)
    {
        free(response.body);

        response.body = NULL;
    }

    return 0;
}

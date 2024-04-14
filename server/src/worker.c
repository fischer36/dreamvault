#include "worker.h"
#include "api.h"
#include "db.h"
#include "sys.h"
// #include "util.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
/*
 * DONE:
 * user register, user unregister, user login, user logout.
 * page read, write, delete, create
 */
// User Log Out
HttpResponseCode worker_task_user_delete(HttpRequest request,
										 HttpResponse *response)
{

	char token[SIZE_TOKEN];

	if (parse_search_body(request.body, "token", token, SIZE_TOKEN) != 0)
	{
		printf("Failed to parse token\n");
		return HTTP_BAD_REQUEST;
	}

	printf("Token: %s\n", token);

	MYSQL *db_connection = 0;
	if (db_connection_get(&db_connection) != 0)
	{
		fprintf(stderr, "Failed to connect to database\n");
		return HTTP_INTERNAL_SERVER_ERROR;
	}
	if (db_session_delete(db_connection, token) != 0)
	{
		fprintf(stderr, "Failed to delete session\n");
		db_connection_close(db_connection);

		return HTTP_UNAUTHORIZED;
	}

	printf("Successfully logged out token %s\n", token);
	db_connection_close(db_connection);
	return HTTP_OK;
}

// User Log In
HttpResponseCode worker_task_user_post(HttpRequest request,
									   HttpResponse *response)
{
	char email[SIZE_EMAIL];
	char password[SIZE_PASSWORD];

	if (parse_search_body(request.body, "login_email", email, SIZE_EMAIL) !=
		0)
	{
		printf("Failed to parse email\n");
		return HTTP_BAD_REQUEST;
	}
	if (parse_search_body(request.body, "login_password", password,
						  SIZE_PASSWORD) != 0)
	{
		printf("Failed to parse password\n");
		return HTTP_BAD_REQUEST;
	}

	printf("[ Email: %s Password: %s\n", email, password);

	MYSQL *db_connection = 0;
	if (db_connection_get(&db_connection) != 0)
	{
		fprintf(stderr, "[worker_login] Failed to connect to database\n");
		return HTTP_INTERNAL_SERVER_ERROR;
	}

	int user_id = 0;

	if (db_get_user_id(db_connection, email, &user_id) != 0)
	{
		fprintf(stderr, "[worker_login] Failed to get user id\n");
		db_connection_close(db_connection);
		return HTTP_UNAUTHORIZED;
	}
	User user;
	if (db_user_get(db_connection, user_id, &user) != 0)
	{
		fprintf(stderr, "[worker_login] Failed to get user\n");
		db_connection_close(db_connection);
		return HTTP_UNAUTHORIZED;
	}

	if (util_compare_hash(password, user.password) != 0)
	{
		fprintf(stderr, "[worker_login] Failed to verify password\n");
		db_connection_close(db_connection);
		return HTTP_UNAUTHORIZED;
	}
	char token[SIZE_TOKEN];
	if (util_generate_token(token) != 0)
	{
		fprintf(stderr, "[worker_login] Failed to generate token\n");
		db_connection_close(db_connection);
		return HTTP_INTERNAL_SERVER_ERROR;
	}

	if (db_session_create(db_connection, user_id, token) != 0)
	{
		fprintf(stderr, "[worker_login] Failed to create session\n");
		db_connection_close(db_connection);
		return HTTP_INTERNAL_SERVER_ERROR;
	}
	response->body = malloc(SIZE_TOKEN); //
	if (response->body != NULL)
	{
		strncpy(response->body, token,
				SIZE_TOKEN - 1);			   // Copy the token into the allocated memory
		response->body[SIZE_TOKEN - 1] = '\0'; // Ensure null termination
		response->body_size = SIZE_TOKEN;
	}
	else
	{
		printf("[worker_login] Failed to allocate memory\n");
		return HTTP_INTERNAL_SERVER_ERROR;
		// Handle malloc failure; perhaps set an error code or print an error
		// message
	}
	//
	db_connection_close(db_connection);

	// strncpy(response->body, token, SIZE_TOKEN);
	return HTTP_OK;
}
// User Delete Account
HttpResponseCode worker_task_register_delete(HttpRequest request,
											 HttpResponse *response)
{

	char email[SIZE_EMAIL];
	char password[SIZE_PASSWORD];

	if (parse_search_body(request.body, "login_email", email, SIZE_EMAIL) !=
		0)
	{
		printf("Failed to parse email\n");
		return HTTP_BAD_REQUEST;
	}
	if (parse_search_body(request.body, "login_password", password,
						  SIZE_PASSWORD) != 0)
	{
		printf("Failed to parse password\n");
		return HTTP_BAD_REQUEST;
	}

	MYSQL *db_connection = NULL;
	if (db_connection_get(&db_connection) != 0)
	{
		fprintf(stderr, "Failed to connect to database\n");
		return HTTP_INTERNAL_SERVER_ERROR;
	}

	printf("email: %s password: %s\n", email, password);

	int user_id = 0;
	if (db_get_user_id(db_connection, email, &user_id) != 0)
	{
		fprintf(stderr, "Failed to find user\n");
		return HTTP_BAD_REQUEST;
	}

	User user;

	if (db_user_get(db_connection, user_id, &user) != 0)
	{
		fprintf(stderr, "Failed to get user\n");
		db_connection_close(db_connection);
		return HTTP_BAD_REQUEST;
	}

	// if (util_compare_hash(password, user.password) != 0) {
	//     fprintf(stderr, "Failed to verify password\n");
	//     db_connection_close(db_connection);
	//     return HTTP_UNAUTHORIZED;
	// }

	if (db_user_delete_sessions(db_connection, user_id) != 0)
	{
		fprintf(stderr, "Failed to delete user sessions\n");
		db_connection_close(db_connection);
		return HTTP_INTERNAL_SERVER_ERROR;
	}

	if (db_user_delete_pages(db_connection, user_id) != 0)
	{
		fprintf(stderr, "Failed to delete user pages\n");
		db_connection_close(db_connection);
		return HTTP_INTERNAL_SERVER_ERROR;
	}

	if (db_user_delete(db_connection, user_id) != 0)
	{
		fprintf(stderr, "Failed to delete user\n");
		db_connection_close(db_connection);
		return HTTP_INTERNAL_SERVER_ERROR;
	}
	printf("hello");
	printf("User_id %d deleted \n", user_id);
	db_connection_close(db_connection);
	return HTTP_OK;
}

// User Create Account
HttpResponseCode worker_task_register_post(HttpRequest request,
										   HttpResponse *response)
{

	char email[SIZE_EMAIL];
	char password[SIZE_PASSWORD];

	if (parse_search_body(request.body, "login_email", email, SIZE_EMAIL) !=
		0)
	{
		printf("Failed to parse email\n");
		return HTTP_BAD_REQUEST;
	}
	if (parse_search_body(request.body, "login_password", password,
						  SIZE_PASSWORD) != 0)
	{
		printf("Failed to parse password\n");
		return HTTP_BAD_REQUEST;
	}

	int storage_limit = 2000;

	MYSQL *db_connection = NULL;

	if (db_connection_get(&db_connection) != 0)
	{
		fprintf(stderr, "Failed to connect to database\n");
		return HTTP_INTERNAL_SERVER_ERROR;
	}

	if (db_does_user_exists(db_connection, email) == true)
	{

		fprintf(stderr, "Email already exists\n");
		db_connection_close(db_connection);
		return HTTP_BAD_REQUEST;
	}

	char hashed_password[256] = {0};

	if (util_hash(password, hashed_password) != 0)
	{
		fprintf(stderr, "Failed to hash password\n");
		db_connection_close(db_connection);
		return HTTP_INTERNAL_SERVER_ERROR;
	}

	if (db_user_create(db_connection, email, hashed_password, storage_limit) !=
		0)
	{
		fprintf(stderr, "Failed to create user\n");
		db_connection_close(db_connection);
		return HTTP_INTERNAL_SERVER_ERROR;
	}
	printf("Sucessfully created user %s\n", email);
	db_connection_close(db_connection);
	return HTTP_OK;
}

HttpResponseCode worker_task_page_patch(HttpRequest request, int page_id,
										HttpResponse *response)
{

	// Temp as page id will be in URI
	int user_id = 0;
	if (worker_validate_token(request, &user_id) != 0)
	{
		printf("Failed to verify token\n");
		return HTTP_UNAUTHORIZED;
	}

	MYSQL *connection = NULL;
	if (db_connection_get(&connection) != 0)
	{
		return -1;
	}
	Page page;
	if (db_page_get(connection, page_id, &page) != 0)
	{
		printf("Failed to get page\n");
		db_connection_close(connection);
		return HTTP_NOT_FOUND;
	}

	if (page.user_id != user_id)
	{
		printf("Page doesn't belong to user\n");
		db_connection_close(connection);
		return HTTP_UNAUTHORIZED;
	}

	char page_title[256];
	if (parse_search_body(request.body, "page_title", page_title, 256) != 0)
	{
		printf("Failed to parse page title\n");
		return HTTP_BAD_REQUEST;
	}

	char page_text[1024];
	if (parse_curly_bracket_contents(request.body, "page_text", page_text) !=
		0)
	{
		printf("Failed to parse page text\n");
		return HTTP_BAD_REQUEST;
	}

	if (sys_file_exists(page.path) == false)
	{
		fprintf(stderr, "File does not exist\n %s\n", page.path);
		db_connection_close(connection);
		return HTTP_INTERNAL_SERVER_ERROR;
	}

	size_t content_size = 0;

	if (sys_file_write(page.path, page_text, &content_size) != 0)
	{
		fprintf(stderr, "Failed to write page: %s\n", page.path);
		db_connection_close(connection);
		return HTTP_INTERNAL_SERVER_ERROR;
	}

	char new_path[256];
	if (strcmp(page.title, page_title) != 0)
	{

		char *last_slash = strrchr(page.path, '/');
		if (!last_slash)
		{
			fprintf(stderr, "Invalid path format in Page struct.\n");
			return -1;
		}
		size_t base_path_length =
			last_slash - page.path + 1; // +1 to include the last '/'
		snprintf(new_path, sizeof(new_path), "%.*s%d_%s.md",
				 (int)base_path_length, page.path, page.user_id, page_title);
		printf("Change in title:\nold path: %s\nnew path: %s\n", page.path,
			   new_path);

		if (sys_file_rename(page.path, new_path) != 0)
		{
			fprintf(stderr, "Failed to rename file: %s\n to %s\n", page.path,
					new_path);
			db_connection_close(connection);
			return HTTP_INTERNAL_SERVER_ERROR;
		}
	}
	else
	{

		snprintf(new_path, sizeof(new_path), "%s", page.path);
		printf("No change in title path: %s\n", new_path);
	}

	if (db_page_update(connection, page_id, page_title, new_path,
					   strlen(page_text)) != 0)
	{
		fprintf(stderr, "Failed to update page: %d\n", page_id);
		db_connection_close(connection);
		return HTTP_INTERNAL_SERVER_ERROR;
	}

	printf("[w_page_write] sucessfully updated page %d\n%s", page_id, new_path);
	db_connection_close(connection);
	return HTTP_OK;
}

// Page Delete
HttpResponseCode worker_task_page_delete(HttpRequest request,
										 HttpResponse *response)
{

	char page_id[256];
	int user_id = 0;
	if (worker_validate_token(request, &user_id) != 0)
		return 1;

	if (parse_search_body(request.body, "page_id", page_id, 256) != 0)
	{
		printf("Failed to parse page title\n");
		return HTTP_BAD_REQUEST;
	}

	MYSQL *connection = NULL;
	if (db_connection_get(&connection) != 0)
	{
		fprintf(stderr, "Failed to connect to database\n");
		return HTTP_INTERNAL_SERVER_ERROR;
	}

	Page page;

	if (db_page_get(connection, 15, &page) != 0)
	{
		fprintf(stderr, "Failed to get page\n");
		db_connection_close(connection);
		return HTTP_NOT_FOUND;
	}

	if (page.user_id != user_id)
	{
		fprintf(stderr, "Unauthorized user\n");
		db_connection_close(connection);
		return HTTP_NOT_FOUND;
	}

	if (sys_file_exists(page.path) == false)
	{
		fprintf(stderr, "File does not exist\n %s\n", page.path);
		db_connection_close(connection);
		return HTTP_INTERNAL_SERVER_ERROR;
	}

	if (sys_file_delete(page.path) != 0)
	{
		fprintf(stderr, "Failed to delete file\n");
		db_connection_close(connection);
		return HTTP_INTERNAL_SERVER_ERROR;
	}

	if (db_page_delete(connection, 15) != 0)
	{
		fprintf(stderr, "Failed to delete file from database\n");
		db_connection_close(connection);
		return HTTP_INTERNAL_SERVER_ERROR;
	}

	db_connection_close(connection);
	return HTTP_OK;
}

HttpResponseCode worker_task_page_get(HttpRequest request,
									  HttpResponse *response)
{

	char token[SIZE_TOKEN];
	char page_id_str[256];
	int page_id;
	int user_id = 0;
	strcpy(page_id_str, request.uri_children + 1);

	if (sscanf(page_id_str, "%d", &page_id) == 1)
	{
		printf("The extracted integer is: %d\n", page_id);
	}
	else
	{
		printf("Failed to extract an integer.\n");
		return -1;
	}

	// Search for authorization token in body string
	if (worker_validate_token(request, &user_id) != 0)
	{
		response->code = HTTP_UNAUTHORIZED;
		return 1;
	}

	MYSQL *connection = NULL;
	if (db_connection_get(&connection) != 0)
	{
		fprintf(stderr, "Failed to connect to database\n");
		return HTTP_INTERNAL_SERVER_ERROR;
	}

	Page page;
	// Get page from page_id, if requested page_id doesnt exist then it returns
	// a non-zero.
	if (db_page_get(connection, page_id, &page) != 0)
	{
		fprintf(stderr, "Failed to get page for id %d\n", page_id);
		db_connection_close(connection);
		return HTTP_NOT_FOUND;
	}

	// See if user id is the owner of requested page
	if (page.user_id != user_id)
	{
		fprintf(stderr, "Unauthorized user\n");
		db_connection_close(connection);
		return HTTP_NOT_FOUND;
	}

	// Check if the logical pages corresponding file exissts, path retrieved
	// from page.
	if (sys_file_exists(page.path) == false)
	{
		fprintf(stderr, "File does not exist\n %s\n", page.path);
		db_connection_close(connection);
		return HTTP_INTERNAL_SERVER_ERROR;
	}

	// Read file contents and allocate a buffer pointed to by request.body
	if (sys_file_read(page.path, &(response->body), &(response->body_size)) !=
		0)
	{
		fprintf(stderr, "Failed to read file\n");
		db_connection_close(connection);
		return HTTP_INTERNAL_SERVER_ERROR;
	}
	else
	{
		printf("BUFFER SIZE: %zu\n%s\n", response->body_size, response->body);
	}

	db_connection_close(connection);

	return HTTP_OK;
}

// Page Create
HttpResponseCode worker_task_page_post(HttpRequest request,
									   HttpResponse *response)
{

	int user_id = 0;
	if (worker_validate_token(request, &user_id) != 0)
	{
		return HTTP_UNAUTHORIZED;
	}
	char vault_id_str[256] = {0};

	if (parse_search_body(request.body, "vault_id", vault_id_str, 256) != 0)
	{
		printf("Failed to parse page id\n");
		return HTTP_BAD_REQUEST;
	}
	int vault_id = atoi(vault_id_str);

	char page_title[256] = {0};

	if (parse_search_body(request.body, "page_title", page_title, 256) != 0)
	{
		printf("Failed to parse page title\n");
		return HTTP_BAD_REQUEST;
	}

	MYSQL *connection = NULL;
	if (db_connection_get(&connection) != 0)
	{
		fprintf(stderr, "Failed to connect to database\n");
		return HTTP_INTERNAL_SERVER_ERROR;
	}

	if (db_page_create(connection, user_id, vault_id, page_title) != 0)
	{
		fprintf(stderr, "Failed to create page\n");
		db_connection_close(connection);
		return HTTP_BAD_REQUEST;
	}
	db_connection_close(connection);
	return HTTP_OK;
}

int worker_create_vault(int user_id, const char vault_name[SIZE_VAULT_NAME])
{

	return 0;
}
int worker_validate_token(const HttpRequest request, int *user_id)
{

	char token[33] = {0};
	for (int i = 0; i < request.header_count; i++)
	{
		if (strcmp(request.headers_array[i].key, "token") == 0)
		{
			strcpy(token, request.headers_array[i].value + 1);
			token[32] = '\0';
			printf("Token:%s\n", token);
			MYSQL *db_connection = NULL;
			if (db_connection_get(&db_connection) != 0)
			{
				return 1;
			}

			Session session;

			printf("Attempting to validate token: %s\n", token);
			if (db_session_get(db_connection, token, &session) != 0)
			{
				fprintf(stderr, "Failed to get session\n");
				db_connection_close(db_connection);
				return 1;
			}
			int year, month, day, hour, minute, second;
			sscanf(session.created_date, "%d-%d-%d %d:%d:%d", &year, &month, &day,
				   &hour, &minute, &second);

			time_t now;
			time(&now);
			struct tm *tm_now = localtime(&now);

			int years_diff = tm_now->tm_year + 1900 - year;
			int months_diff = tm_now->tm_mon + 1 - month;
			int days_diff = tm_now->tm_mday - day;

			if (years_diff > 0 || months_diff > 0 || days_diff > 1 ||
				(days_diff == 1 &&
				 (tm_now->tm_hour > hour ||
				  (tm_now->tm_hour == hour && tm_now->tm_min > minute) ||
				  (tm_now->tm_hour == hour && tm_now->tm_min == minute &&
				   tm_now->tm_sec >= second))))
			{
				printf("More than 24 hours have passed since the session was created. "
					   "Deleting session for token %s\n",
					   token);

				db_connection_close(db_connection);
				db_session_delete(db_connection, token);
				return 1;
			}
			else
			{
				printf("Less than 24 hours have passed since the session was "
					   "created.\n");
				*user_id = session.user_id;
				db_connection_close(db_connection);
				return 0;
			}
		}
	}
	return 1;
}

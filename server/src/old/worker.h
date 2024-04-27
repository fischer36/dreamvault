#pragma once

#include "db.h"
#include "http_parser.h"

// User Logout
HttpResponseCode worker_task_user_delete(HttpRequest request,
                                         HttpResponse *response);
// User Login
HttpResponseCode worker_task_user_post(HttpRequest request,
                                       HttpResponse *response);
// Create User
HttpResponseCode worker_task_register_delete(HttpRequest request,
                                             HttpResponse *response);
// Delete User
HttpResponseCode worker_task_register_post(HttpRequest request,
                                           HttpResponse *response);
// Page Delete
HttpResponseCode worker_task_page_delete(HttpRequest request,
                                         HttpResponse *response);
// Page Create
HttpResponseCode worker_task_page_post(HttpRequest request,
                                       HttpResponse *response);
// Page Write/Update
HttpResponseCode worker_task_page_patch(HttpRequest request, int page_id,
                                        HttpResponse *response);
// Page Read
HttpResponseCode worker_task_page_get(HttpRequest request,
                                      HttpResponse *response);

//  Parses token
int worker_wrapper_get_token(HttpRequest request, char *token);
// Validates Token And On Succes Fills Out Corresponding User ID
int worker_validate_token(const HttpRequest request, int *user_id);
#include "db.h"
#include "http_parser.h"
#include "util.h"

struct Response {
  char *code;
  char *s;
};

int register(char *email, char *password) {

  char password_hash[35] = {0};

  if (util_hash(password, password_hash) != 0)
    return -1;

  return 0;
}

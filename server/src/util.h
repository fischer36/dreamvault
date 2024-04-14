#pragma once

int util_compare_hash(const char *compare, const char *hashed);
int util_hash(const char *to_hash, char *hashed);
int util_generate_token(char token[33]);

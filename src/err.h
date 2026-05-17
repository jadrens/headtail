#pragma once
#include <string>
#include <cstdint>

void err_exit(const char* key);
void err_exit(const char* key, const char* a0);
void err_exit(const char* key, const char* a0, const char* a1);
uint64_t do_parse_num(const std::string& val, const std::string& arg_name);
void print_omitted_size(uint64_t bytes, bool force_byte);
#pragma once
#include <string>

struct LangEntry {
    const char* key;
    const char* value;
};

const LangEntry* get_strings(int* count);
const char* get_string(const char* key, const std::string& lang);
std::string detect_lang(const std::string& locale);
std::string get_current_lang();
void set_current_lang(const std::string& lang);
void register_lang_strings(const char* lang, const LangEntry* entries, int count);
std::string get_locale();
std::string detect_lang(const std::string& locale);
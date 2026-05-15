#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <unordered_map>
#include "i18n.h"

static std::string g_lang = "en";
static std::unordered_map<std::string, std::pair<const LangEntry*, int>> g_strings;

static bool starts_with(const char* s, const char* prefix) {
    size_t slen = strlen(s);
    size_t plen = strlen(prefix);
    if (slen < plen) return false;
    return strncmp(s, prefix, plen) == 0;
}

std::string get_locale() {
    const char* lc_all = std::getenv("LC_ALL");
    if (lc_all && lc_all[0]) return std::string(lc_all);
    const char* lang = std::getenv("LANG");
    if (lang && lang[0]) return std::string(lang);
    return "en";
}

std::string detect_lang(const std::string& locale) {
    if (starts_with(locale.c_str(), "zh")) return "zh";
    return "en";
}

void register_lang_strings(const char* lang, const LangEntry* entries, int count) {
    g_strings[lang] = {entries, count};
}

const char* get_string(const char* key, const std::string& lang) {
    auto it = g_strings.find(lang);
    if (it == g_strings.end()) it = g_strings.find("en");
    if (it == g_strings.end()) return key;

    const LangEntry* entries = it->second.first;
    int count = it->second.second;
    for (int i = 0; i < count; i++) {
        if (strcmp(entries[i].key, key) == 0) {
            return entries[i].value;
        }
    }
    return key;
}

std::string get_current_lang() {
    return g_lang;
}

void set_current_lang(const std::string& lang) {
    g_lang = lang;
}
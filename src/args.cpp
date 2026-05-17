#include "args.h"
#include "err.h"
#include "process.h"
#include "i18n.h"
#include <iostream>
#include <string>
#include <map>
#include <functional>
#include <fstream>
#include <filesystem>
#include <unordered_set>

extern std::string g_lang;
AppState g_state;

AppState& get_state() {
    return g_state;
}

void print_help() {
    std::cout << get_string("usage", g_lang) << "\n"
         << get_string("options", g_lang) << "\n"
         << get_string("opt_n", g_lang) << "\n"
         << get_string("opt_c", g_lang) << "\n"
         << get_string("opt_head", g_lang) << "\n"
         << get_string("opt_tail", g_lang) << "\n"
         << get_string("opt_input", g_lang) << "\n"
         << get_string("opt_show_size", g_lang) << "\n"
         << get_string("opt_byte", g_lang) << "\n"
         << get_string("opt_help", g_lang) << "\n";
}

static const std::unordered_set<std::string> flag_set = {
    "-h", "--head", "-t", "--tail", "-i", "--input",
    "-c", "-n", "-s", "--show-size", "--byte", "--help"
};

bool is_flag(const std::string& s) {
    return flag_set.contains(s);
}

static void require_arg(int& i, int argc, char* argv[], const char* flag) {
    if (++i >= argc || is_flag(argv[i])) err_exit("err_missing_arg", flag);
    if (g_state.mode == AppState::Mode::CHARS) err_exit("err_mode_duplicated");
}

uint64_t do_parse_num(const std::string& val, const std::string& arg_name);

void parse_args(int argc, char* argv[]) {
    using Handler = std::function<void(int&, int, char*[])>;
    static const std::map<std::string, Handler> handlers = {
        {"-n", [](int& i, int argc, char* argv[]) {
            g_state.mode = AppState::Mode::LINES;
            if (i + 1 < argc && !is_flag(argv[i + 1])) {
                g_state.head_count = g_state.tail_count = do_parse_num(argv[++i], "-n");
            }
        }},
        {"-c", [](int& i, int argc, char* argv[]) {
            g_state.mode = AppState::Mode::CHARS;
            if (i + 1 < argc && !is_flag(argv[i + 1])) {
                g_state.head_count = g_state.tail_count = do_parse_num(argv[++i], "-c");
            }
        }},
        {"-h", [](int& i, int argc, char* argv[]) {
            require_arg(i, argc, argv, "-h");
            g_state.head_count = do_parse_num(argv[i], "-h");
        }},
        {"--head", [](int& i, int argc, char* argv[]) {
            require_arg(i, argc, argv, "--head");
            g_state.head_count = do_parse_num(argv[i], "--head");
        }},
        {"-t", [](int& i, int argc, char* argv[]) {
            require_arg(i, argc, argv, "-t");
            g_state.tail_count = do_parse_num(argv[i], "-t");
        }},
        {"--tail", [](int& i, int argc, char* argv[]) {
            require_arg(i, argc, argv, "--tail");
            g_state.tail_count = do_parse_num(argv[i], "--tail");
        }},
        {"-i", [](int& i, int argc, char* argv[]) {
            require_arg(i, argc, argv, "-i");
            g_state.input_file = argv[i];
        }},
        {"--input", [](int& i, int argc, char* argv[]) {
            require_arg(i, argc, argv, "--input");
            g_state.input_file = argv[i];
        }},
        {"-s", [](int&, int, char*[]) { g_state.show_size = true; }},
        {"--show-size", [](int&, int, char*[]) { g_state.show_size = true; }},
        {"--byte", [](int&, int, char*[]) { g_state.force_byte = true; g_state.show_size = true; }},
        {"--help", [](int&, int, char*[]) { print_help(); std::exit(0); }},
    };

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        auto it = handlers.find(arg);
        if (it != handlers.end()) {
            it->second(i, argc, argv);
        } else if (arg[0] != '-') {
            g_state.input_file = arg;
        } else {
            err_exit("err_unknown_arg", arg.c_str());
        }
    }

    if (g_state.mode == AppState::Mode::NONE) {
        g_state.mode = AppState::Mode::LINES;
    }
}

std::istream* open_input(const std::string& file) {
    if (file.empty()) return &std::cin;

    std::error_code ec;
    if (std::filesystem::is_directory(file, ec)) {
        err_exit("err_is_folder", file.c_str());
    }
    std::ifstream* f = new std::ifstream(file, std::ios::binary);
    if (!*f) {
        delete f;
        err_exit("err_permission");
    }
    return f;
}
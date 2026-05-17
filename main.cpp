#include <fstream>
#include <iostream>
#include "src/args.h"
#include "src/process.h"
#include "src/i18n/i18n.h"

std::string g_lang;

int main(int argc, char* argv[]) {
    std::cout << std::unitbuf;
    g_lang = detect_lang(get_locale());

    parse_args(argc, argv);

    AppState& state = get_state();
    // cin or file_in
    std::istream* in = open_input(state.input_file);
    std::ifstream* file_in = dynamic_cast<std::ifstream*>(in);
    //  delete if file
    bool need_delete = (file_in != nullptr);

    if (state.mode == AppState::Mode::LINES) {
        process_lines(*in);
    } else {
        process_chars(*in);
    }

    if (need_delete) delete in;
    return 0;
}
#include "i18n.h"

extern void register_lang_strings(const char* lang, const LangEntry* entries, int count);

static const LangEntry local_strings[] = {
    {"usage", "Usage: headtail [OPTIONS]..."},
    {"options", "Options:"},
    {"opt_n", "  -n [NUM]               Line mode (default), display NUM lines at head/tail."},
    {"opt_c", "  -c [NUM]               Byte/Char mode, display NUM bytes at head/tail."},
    {"opt_head", "  -h, --head NUM         Specify the number of lines/bytes for the head."},
    {"opt_tail", "  -t, --tail NUM         Specify the number of lines/bytes for the tail."},
    {"opt_input", "  -i, --input FILE       Read from FILE instead of standard input."},
    {"opt_show_size", "  -s, --show-size        Show omitted size with auto-scaling unit (KiB, MiB, etc)."},
    {"opt_byte", "  --byte                 Force showing omitted size in raw bytes."},
    {"opt_help", "  --help                 Show this help message."},
    {"err_invalid_arg", "Error: Invalid argument \"{1}\" for {0}"},
    {"err_mode_duplicated", "Error: Mode duplicated."},
    {"err_missing_arg", "Error: Missing argument for {0}"},
    {"err_is_folder", "Error: \"{0}\" is a folder"},
    {"err_permission", "Error: permission denied."},
    {"err_unknown_arg", "Error: Unknown argument {0}"},
    {"err_memory", "Error: Memory limit exceeded. Tail size too large for current RAM."},
    {"omit_lines", "... ({0} lines omitted) ..."},
    {"omit_bytes", "... ({0} chars omitted) ..."},
    {"omit_size", "... ({1} bytes has been omitted) ..."},
    {"omit_size_unit", "... ({1} {2} has been omitted) ..."},
    {"units_bytes", "bytes"},
    {"units_kib", "KiB"},
    {"units_mib", "MiB"},
    {"units_gib", "GiB"},
    {"units_tib", "TiB"},
};

__attribute__((constructor))
static void init() {
    register_lang_strings("en", local_strings, sizeof(local_strings) / sizeof(local_strings[0]));
}
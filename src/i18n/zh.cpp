#include "i18n.h"

extern void register_lang_strings(const char* lang, const LangEntry* entries, int count);

static const LangEntry local_strings[] = {
    {"usage", "用法: headtail [选项]..."},
    {"options", "选项:"},
    {"opt_n", "  -n [NUM]               行模式（默认），显示头/尾各 NUM 行。"},
    {"opt_c", "  -c [NUM]               字节模式，显示头/尾各 NUM 字节。"},
    {"opt_head", "  -h, --head NUM         指定头部的行/字节数。"},
    {"opt_tail", "  -t, --tail NUM         指定尾部的行/字节数。"},
    {"opt_input", "  -i, --input 文件       从指定文件读取而非标准输入。"},
    {"opt_show_size", "  -s, --show-size        显示省略大小（自动单位 KiB, MiB 等）。"},
    {"opt_byte", "  --byte                 强制以原始字节显示省略大小。"},
    {"opt_keyword", "  -k, --keyword 关键词  关键词触发立即输出（逗号分隔）。"},
    {"opt_keyword_case", "  -K 关键词              大小写不敏感关键词匹配。"},
    {"opt_help", "  --help                 显示此帮助信息。"},
    {"err_invalid_arg", "错误: 参数 \"{1}\" 对 {0} 无效"},
    {"err_mode_duplicated", "错误: 模式重复指定。"},
    {"err_missing_arg", "错误: {0} 缺少参数"},
    {"err_is_folder", "错误: \"{0}\" 是一个文件夹"},
    {"err_permission", "错误: 权限不足。"},
    {"err_unknown_arg", "错误: 未知的参数 {0}"},
    {"err_memory", "错误: 内存限制超出。当前 RAM 不足以支持如此大的尾部大小。"},
    {"omit_lines", "... ({0} 行已省略) ..."},
    {"omit_bytes", "... ({0} 字符已省略) ..."},
    {"omit_size", "... ({1} 字节已省略) ..."},
    {"omit_size_unit", "... ({1} {2} 已省略) ..."},
    {"units_bytes", "字节"},
    {"units_kib", "KiB"},
    {"units_mib", "MiB"},
    {"units_gib", "GiB"},
    {"units_tib", "TiB"},
};

__attribute__((constructor))
static void init() {
    register_lang_strings("zh", local_strings, sizeof(local_strings) / sizeof(local_strings[0]));
}
#include "cfg1.is.qcr.h"
#include <stdio.h>
#include <locale.h>
#include <wchar.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// 安全的 UTF-8 输出函数（含平台宏）
static void safe_print_utf8(const char* str) {
    if (!str) return;

#ifdef _WIN32
    // 转换为宽字符输出
    int wlen = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
    if (wlen > 0) {
        wchar_t* wstr = (wchar_t*)malloc(wlen * sizeof(wchar_t));
        if (wstr) {
            MultiByteToWideChar(CP_UTF8, 0, str, -1, wstr, wlen);
            wprintf(L"%ls", wstr);
            free(wstr);
            return;
        }
    }
#endif
    // 回退方案
    printf("%s", str);
}

// 打印配置项（带索引）
static void print_config_item(size_t index, const char* key, const char* value) {
    printf("  [%zu] ", index);
    safe_print_utf8(key);
    printf(": ");
    safe_print_utf8(value);
    printf("\n");
}

int main() {
    // 设置控制台支持 UTF-8
#ifdef _WIN32
    // 设置控制台输出为 UTF-8
    SetConsoleOutputCP(CP_UTF8);
    // 设置控制台输入为 UTF-8
    SetConsoleCP(CP_UTF8);

    // 设置控制台字体支持中文
    CONSOLE_FONT_INFOEX font = { sizeof(font) };
    font.dwFontSize.Y = 16;
    wcscpy_s(font.FaceName, LF_FACESIZE, L"SimSun-ExtB"); // 使用支持中文的字体
    SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &font);
#else
    // 在 Linux/macOS 上设置 UTF-8 环境
    setenv("LC_ALL", "en_US.UTF-8", 1);
#endif

    // 设置本地化
    setlocale(LC_ALL, "zh_CN.UTF-8");

    wprintf(L"=== 配置文件键值获取示例 ===\n\n");

    const char* filename = "test.txt";

    // 1. 创建文件锁对象
    FileLock* lock = file_lock_create(filename);
    if (!lock) {
        wprintf(L"创建文件锁失败\n");
        return 1;
    }
    wprintf(L"已创建文件锁: %hs\n", filename);

    // 2. 锁定文件
    if (file_lock(lock) != 0) {
        wprintf(L"文件锁定失败\n");
        file_lock_destroy(lock);
        return 1;
    }
    wprintf(L"文件已锁定\n");

    // 3. 读取文件内容
    char* content = file_read_content(lock);
    if (!content) {
        wprintf(L"读取文件内容失败\n");
        file_unlock(lock);
        file_lock_destroy(lock);
        return 1;
    }

    wprintf(L"\n=== 文件原始内容 ===\n");
    safe_print_utf8(content);
    wprintf(L"\n=====================\n");

    // 4. 解析配置
    Config config;
    config_init(&config);

    if (!config_parse(&config, content)) {
        wprintf(L"解析配置文件失败\n");
        free(content);
        config_free(&config);
        file_unlock(lock);
        file_lock_destroy(lock);
        return 1;
    }
    free(content);

    // 5. 获取并打印所有键值对
    wprintf(L"\n=== 解析结果 (共 %zu 个键值对) ===\n", config.count);
    for (size_t i = 0; i < config.count; i++) {
        print_config_item(i + 1, config.pairs[i].key, config.pairs[i].value);
    }
    wprintf(L"===============================\n");

    // 6. 查找单个键值对（窄字符）
    wprintf(L"\n=== 查找单个键值对 (窄字符) ===\n");

    // 定义要查找的键列表
    const char* keys_to_find[] = {
        "应用名称", "版本", "语言", "路径", "说明",
        "多语言值", "启用功能", "最大连接数", "不存在的键"
    };

    for (size_t i = 0; i < sizeof(keys_to_find) / sizeof(keys_to_find[0]); i++) {
        const char* key = keys_to_find[i];
        const char* value = config_get_value(&config, key);

        if (value) {
            wprintf(L"  找到键 '");
            safe_print_utf8(key);
            wprintf(L"' 的值: '");
            safe_print_utf8(value);
            wprintf(L"'\n");
        }
        else {
            wprintf(L"  未找到键 '");
            safe_print_utf8(key);
            wprintf(L"'\n");
        }
    }

    // 7. 查找单个键值对（宽字符）
    wprintf(L"\n=== 查找单个键值对 (宽字符) ===\n");
    const wchar_t* wide_keys[] = {
        L"应用名称", L"版本", L"语言", L"路径", L"说明",
        L"多语言值", L"启用功能", L"最大连接数", L"不存在的键"
    };

    for (size_t i = 0; i < sizeof(wide_keys) / sizeof(wide_keys[0]); i++) {
        const wchar_t* key = wide_keys[i];
        const wchar_t* value = wchart_config_get_value(&config, key);

        if (value) {
            wprintf(L"  找到键 '%s' 的值: '%s'\n", key, value);
            free((void*)value); // 释放宽字符查询返回的内存
        }
        else {
            wprintf(L"  未找到键 '%s'\n", key);
        }
    }

    // 8. 查找并修改键值
    wprintf(L"\n=== 查找并修改键值 ===\n");
    const wchar_t* key_to_modify = L"最大连接数";
    const wchar_t* old_value = wchart_config_get_value(&config, key_to_modify);

    if (old_value) {
        wprintf(L"  键 '%s' 的当前值: %s\n", key_to_modify, old_value);

        // 修改值（实际应用中可能从用户输入获取）
        wchar_t new_value[50];
        wchar_t* end;
        long int_value = wcstol(old_value, &end, 10);

        if (*end == L'\0') { // 确保整个字符串都被转换
            swprintf(new_value, sizeof(new_value) / sizeof(wchar_t), L"%ld", int_value + 10);
            wprintf(L"  建议新值: %s\n", new_value);

            // 在实际应用中，这里会将修改写回配置
            // 例如：修改配置文件内容并写回
            //file_write_content(lock, new_content);
        }
        else {
            wprintf(L"  无法解析数值\n");
        }

        free((void*)old_value); // 释放宽字符查询返回的内存
    }
    else {
        wprintf(L"  找不到键 '%s', 无法修改\n", key_to_modify);
    }

    // 9. 清理资源
    config_free(&config);

    // 10. 解锁文件
    if (file_unlock(lock) != 0) {
        wprintf(L"文件解锁失败\n");
    }
    else {
        wprintf(L"\n文件已解锁\n");
    }

    file_lock_destroy(lock);

    wprintf(L"\n程序结束\n");
    return 0;
}
#pragma cfg1
#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <errno.h>
#include <wchar.h>
#include <locale.h>

// 跨平台文件锁定支持
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
typedef HANDLE FileHandle;
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/file.h>
typedef int FileHandle;
#endif

// 错误处理宏
#ifdef _WIN32
#define PRINT_ERROR(msg) do { \
    fprintf(stderr, "%s (Error %lu)\n", msg, GetLastError()); \
} while(0)
#else
#define PRINT_ERROR(msg) do { \
    fprintf(stderr, "%s: %s\n", msg, strerror(errno)); \
} while(0)
#endif

// 键值对结构
typedef struct {
    char* key;      // UTF-8编码键名
    char* value;    // UTF-8编码值
} KeyValuePair;

// 配置解析结果
typedef struct {
    KeyValuePair* pairs;    // 键值对数组
    size_t count;           // 键值对数量
    size_t capacity;        // 数组容量
} Config;

// 文件锁结构
typedef struct {
    FileHandle handle;      // 文件句柄
    char* file_path;        // 文件路径
    int is_locked;          // 锁定状态标志
#ifdef _WIN32
    OVERLAPPED lock_region; // Windows专用锁定区域
#endif
} FileLock;

// 初始容量
#define INITIAL_CAPACITY 16

// ================== 函数声明 ==================
void config_init(Config* config);
void config_free(Config* config);
int config_parse(Config* config, const char* content);
const char* config_get_value(const Config* config, const char* key);
char* config_serialize(const Config* config);

// 新增宽字符查询函数
const wchar_t* wchart_config_get_value(const Config* config, const wchar_t* key);

FileLock* file_lock_create(const char* file_path);
int file_lock(FileLock* lock);
int file_unlock(FileLock* lock);
void file_lock_destroy(FileLock* lock);
char* file_read_content(FileLock* lock);
int file_write_content(FileLock* lock, const char* content);

// ================== 跨平台辅助函数 ==================

// 安全的字符串复制函数 (替代strdup)
static char* safe_strdup(const char* str) {
    if (!str) return NULL;

    size_t len = strlen(str) + 1;
    char* copy = (char*)malloc(len);
    if (copy) {
        memcpy(copy, str, len);
    }
    return copy;
}

// 跨平台安全的字符串分割函数
static char* safe_strtok(char* str, const char* delimiters, char** saveptr) {
#ifdef _WIN32
    return strtok_s(str, delimiters, saveptr);
#else
    return strtok_r(str, delimiters, saveptr);
#endif
}

// ================== 宽字符辅助函数 ==================

// 安全的多字节转宽字符函数
static int safe_mbstowcs(wchar_t* wcstr, size_t sizeInWords, const char* mbstr, size_t count) {
#ifdef _WIN32
    // 使用Windows安全版本
    return mbstowcs_s(NULL, wcstr, sizeInWords, mbstr, count) == 0 ? 0 : -1;
#else
    // 非Windows平台使用标准函数
    size_t result = mbstowcs(wcstr, mbstr, sizeInWords);
    return (result == (size_t)-1) ? -1 : 0;
#endif
}

// 安全的宽字符转多字节函数
static int safe_wcstombs(char* mbstr, size_t sizeInBytes, const wchar_t* wcstr, size_t count) {
#ifdef _WIN32
    // 使用Windows安全版本
    return wcstombs_s(NULL, mbstr, sizeInBytes, wcstr, count) == 0 ? 0 : -1;
#else
    // 非Windows平台使用标准函数
    size_t result = wcstombs(mbstr, wcstr, sizeInBytes);
    return (result == (size_t)-1) ? -1 : 0;
#endif
}

// 窄字符转宽字符 (UTF-8 -> wchar_t)
static wchar_t* narrow_to_wide(const char* narrow) {
    if (!narrow) return NULL;

    // 保存当前locale
    char* old_locale = _strdup(setlocale(LC_CTYPE, NULL));
    if (!old_locale) return NULL;

    // 设置UTF-8 locale
    if (!setlocale(LC_CTYPE, "en_US.UTF-8")) {
        setlocale(LC_CTYPE, "");
    }

    // 计算所需长度
    size_t len = 0;
#ifdef _WIN32
    errno_t err = mbstowcs_s(&len, NULL, 0, narrow, 0);
    if (err != 0) {
        setlocale(LC_CTYPE, old_locale);
        free(old_locale);
        return NULL;
    }

    // 长度包括null终止符
    if (len == 0) {
        setlocale(LC_CTYPE, old_locale);
        free(old_locale);
        return NULL;
    }
#else
    len = mbstowcs(NULL, narrow, 0);
    if (len == (size_t)-1) {
        setlocale(LC_CTYPE, old_locale);
        free(old_locale);
        return NULL;
    }
    len++; // 为null终止符添加空间
#endif

    // 分配内存并转换
    wchar_t* wide = (wchar_t*)malloc(len * sizeof(wchar_t));
    if (!wide) {
        setlocale(LC_CTYPE, old_locale);
        free(old_locale);
        return NULL;
    }

#ifdef _WIN32
    err = mbstowcs_s(NULL, wide, len, narrow, _TRUNCATE);
    if (err != 0) {
        free(wide);
        setlocale(LC_CTYPE, old_locale);
        free(old_locale);
        return NULL;
    }
#else
    if (mbstowcs(wide, narrow, len) == (size_t)-1) {
        free(wide);
        setlocale(LC_CTYPE, old_locale);
        free(old_locale);
        return NULL;
    }
#endif

    // 恢复原始locale
    setlocale(LC_CTYPE, old_locale);
    free(old_locale);
    return wide;
}

// 宽字符转窄字符 (wchar_t -> UTF-8)
static char* wide_to_narrow(const wchar_t* wide) {
    if (!wide) return NULL;

    // 保存当前locale
    char* old_locale = _strdup(setlocale(LC_CTYPE, NULL));
    if (!old_locale) return NULL;

    // 设置UTF-8 locale
    if (!setlocale(LC_CTYPE, "en_US.UTF-8")) {
        setlocale(LC_CTYPE, "");
    }

    // 计算所需长度
    size_t len = 0;
#ifdef _WIN32
    errno_t err = wcstombs_s(&len, NULL, 0, wide, 0);
    if (err != 0 || len == 0) {
        setlocale(LC_CTYPE, old_locale);
        free(old_locale);
        return NULL;
    }
#else
    len = wcstombs(NULL, wide, 0);
    if (len == (size_t)-1) {
        setlocale(LC_CTYPE, old_locale);
        free(old_locale);
        return NULL;
    }
    len++; // 为null终止符添加空间
#endif

    // 分配内存并转换
    char* narrow = (char*)malloc(len);
    if (!narrow) {
        setlocale(LC_CTYPE, old_locale);
        free(old_locale);
        return NULL;
    }

#ifdef _WIN32
    err = wcstombs_s(NULL, narrow, len, wide, _TRUNCATE);
    if (err != 0) {
        free(narrow);
        setlocale(LC_CTYPE, old_locale);
        free(old_locale);
        return NULL;
    }
#else
    if (wcstombs(narrow, wide, len) == (size_t)-1) {
        free(narrow);
        setlocale(LC_CTYPE, old_locale);
        free(old_locale);
        return NULL;
    }
#endif

    // 恢复原始locale
    setlocale(LC_CTYPE, old_locale);
    free(old_locale);
    return narrow;
}

// ================== 函数实现 ==================

// 初始化配置对象
void config_init(Config* config) {
    config->pairs = NULL;
    config->count = 0;
    config->capacity = 0;
}

// 释放配置对象
void config_free(Config* config) {
    if (!config) return;

    for (size_t i = 0; i < config->count; i++) {
        free(config->pairs[i].key);
        free(config->pairs[i].value);
    }
    free(config->pairs);
    config->pairs = NULL;
    config->count = 0;
    config->capacity = 0;
}

// UTF-8字符长度计算
static int utf8_char_len(unsigned char c) {
    if (c < 0x80) return 1;
    if ((c & 0xE0) == 0xC0) return 2;
    if ((c & 0xF0) == 0xE0) return 3;
    if ((c & 0xF8) == 0xF0) return 4;
    return 1; // 无效序列作为单字节处理
}

// 精确转义处理
static char* parse_value(const char* input) {
    if (!input) return safe_strdup("");

    size_t len = strlen(input);
    // 分配足够空间（最坏情况：每个字符都需要转义）
    char* result = (char*)malloc(len * 2 + 1);
    if (!result) return NULL;

    size_t j = 0;
    const char* p = input;

    while (*p) {
        int char_len = utf8_char_len((unsigned char)*p);

        if (*p == '\\') {
            // 处理转义序列
            if (*(p + 1)) {
                // 有下一个字符
                int next_len = utf8_char_len((unsigned char)*(p + 1));
                if (*(p + 1) == ' ') {
                    // 反斜杠+空格：保留两者
                    memcpy(result + j, p, char_len);
                    j += char_len;
                    p += char_len;
                    memcpy(result + j, p, next_len);
                    j += next_len;
                    p += next_len;
                }
                else {
                    // 其他转义字符：只保留转义后的字符
                    p += char_len; // 跳过反斜杠
                    memcpy(result + j, p, next_len);
                    j += next_len;
                    p += next_len;
                }
            }
            else {
                // 行末单独反斜杠：保留
                memcpy(result + j, p, char_len);
                j += char_len;
                p += char_len;
            }
        }
        else if (*p == '#') {
            // 未转义的#，注释开始
            break;
        }
        else {
            // 普通字符（包括多字节字符）
            memcpy(result + j, p, char_len);
            j += char_len;
            p += char_len;
        }
    }

    result[j] = '\0';

    // 修剪首尾空白
    char* start = result;
    while (isspace((unsigned char)*start)) start++;

    char* end = result + j - 1;
    while (end > start && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';

    // 避免内存浪费
    char* trimmed = safe_strdup(start);
    free(result);
    return trimmed;
}

// 检查键是否已存在
static int key_exists(const Config* config, const char* key) {
    for (size_t i = 0; i < config->count; i++) {
        if (strcmp(config->pairs[i].key, key) == 0) {
            return 1;
        }
    }
    return 0;
}

// 添加键值对
static int config_add_pair(Config* config, const char* key, const char* raw_value) {
    // 检查键是否为空
    if (!key || !*key) return 0;

    // 检查键是否重复
    if (key_exists(config, key)) {
        fprintf(stderr, "Duplicate key error: %s\n", key);
        return 0;
    }

    // 扩展容量
    if (config->count >= config->capacity) {
        size_t new_capacity = config->capacity == 0 ? INITIAL_CAPACITY : config->capacity * 2;
        KeyValuePair* new_pairs = (KeyValuePair*)realloc(config->pairs, new_capacity * sizeof(KeyValuePair));
        if (!new_pairs) {
            perror("realloc failed");
            return 0;
        }
        config->pairs = new_pairs;
        config->capacity = new_capacity;
    }

    // 分配键
    config->pairs[config->count].key = safe_strdup(key);
    if (!config->pairs[config->count].key) {
        perror("dup_str key failed");
        return 0;
    }

    // 解析值
    config->pairs[config->count].value = parse_value(raw_value);
    if (!config->pairs[config->count].value) {
        perror("parse_value failed");
        free(config->pairs[config->count].key);
        return 0;
    }

    config->count++;
    return 1;
}

static int config_update_pair(Config* config, const char* key, const char* raw_value) {
    // 检查键是否为空
    if (!key || !*key) return 0;
    // 查找键
    for (size_t i = 0; i < config->count; i++) {
        if (strcmp(config->pairs[i].key, key) == 0) {
            // 更新值
            free(config->pairs[i].value);
            config->pairs[i].value = parse_value(raw_value);
            if (!config->pairs[i].value) {
                perror("parse_value failed");
                return 0;
            }
            return 1;
        }
    }
    // 键不存在，添加新键值对
    return config_add_pair(config, key, raw_value);
}

static int config_delete_pair(Config* config, const char* key) {
    // 检查键是否为空
    if (!key || !*key) return 0;
    // 查找键
    for (size_t i = 0; i < config->count; i++) {
        if (strcmp(config->pairs[i].key, key) == 0) {
            // 释放内存
            free(config->pairs[i].key);
            free(config->pairs[i].value);
            // 移动后面的元素
            for (size_t j = i + 1; j < config->count; j++) {
                config->pairs[j - 1] = config->pairs[j];
            }
            config->count--;
            return 1;
        }
    }
    // 键不存在
    return 0;
}

// 解析配置文件内容
int config_parse(Config* config, const char* content) {
    if (!content) return 0;

    char* line = safe_strdup(content);
    if (!line) {
        perror("dup_str failed");
        return 0;
    }

    char* saveptr = NULL;
    char* token = safe_strtok(line, "\n", &saveptr);
    int line_num = 0;

    while (token) {
        line_num++;

        // 跳过空行和注释行
        if (!*token || *token == '#') {
            token = safe_strtok(NULL, "\n", &saveptr);
            continue;
        }

        // 查找冒号分隔符
        char* colon = strchr(token, ':');
        if (!colon) {
            // 没有冒号，跳过
            token = safe_strtok(NULL, "\n", &saveptr);
            continue;
        }

        // 分割键和值
        *colon = '\0';
        char* key = token;
        char* value = colon + 1;

        // 修剪键的首尾空白
        char* key_end = key + strlen(key) - 1;
        while (key_end > key && isspace((unsigned char)*key_end)) {
            *key_end-- = '\0';
        }
        while (isspace((unsigned char)*key)) {
            key++;
        }

        // 添加键值对
        if (!config_add_pair(config, key, value)) {
            free(line);
            return 0;
        }

        token = safe_strtok(NULL, "\n", &saveptr);
    }

    free(line);
    return 1;
}

char* config_serialize(const Config* config) {
    if (!config || config->count == 0) {
        return safe_strdup(""); // 返回空字符串
    }
    
    // 1. 计算所需的总长度
    size_t total_len = 0;
    for (size_t i = 0; i < config->count; i++) {
        // 键长度 + ": " + 值长度 + "\n"
        total_len += strlen(config->pairs[i].key) + 2 + 
                     strlen(config->pairs[i].value) + 1;
    }
    
    // 2. 分配内存（额外空间用于 null 终止符）
    char* content = (char*)malloc(total_len + 1);
    if (!content) return NULL;
    
    // 3. 构建内容
    char* ptr = content;
    for (size_t i = 0; i < config->count; i++) {
        // 复制键
        size_t key_len = strlen(config->pairs[i].key);
        memcpy(ptr, config->pairs[i].key, key_len);
        ptr += key_len;
        
        // 添加分隔符
        *ptr++ = ':';
        *ptr++ = ' ';
        
        // 复制值
        size_t value_len = strlen(config->pairs[i].value);
        memcpy(ptr, config->pairs[i].value, value_len);
        ptr += value_len;
        
        // 添加换行符
        *ptr++ = '\n';
    }
    
    // 4. 添加 null 终止符
    *ptr = '\0';
    
    return content;
}

// 根据键查找值 (UTF-8)
const char* config_get_value(const Config* config, const char* key) {
    if (!config || !key) return NULL;

    for (size_t i = 0; i < config->count; i++) {
        if (strcmp(config->pairs[i].key, key) == 0) {
            return config->pairs[i].value;
        }
    }
    return NULL;
}

// 新增函数：宽字符键值查询
const wchar_t* wchart_config_get_value(const Config* config, const wchar_t* key) {
    if (!config || !key) return NULL;

    // 将宽字符键转换为UTF-8
    char* utf8_key = NULL;
    {
        // 设置locale用于转换
        char* old_locale = safe_strdup(setlocale(LC_CTYPE, NULL));
        char* new_locale = setlocale(LC_CTYPE, "en_US.UTF-8");
        if (!new_locale) {
            setlocale(LC_CTYPE, "");
        }

        // 计算所需长度
        size_t len;
#ifdef _WIN32
        if (wcstombs_s(&len, NULL, 0, key, 0) != 0) {
            setlocale(LC_CTYPE, old_locale);
            return NULL;
        }
#else
        len = wcstombs(NULL, key, 0);
        if (len == (size_t)-1) {
            setlocale(LC_CTYPE, old_locale);
            return NULL;
        }
#endif

        // 分配内存并转换
        utf8_key = (char*)malloc(len + 1);
        if (!utf8_key) {
            setlocale(LC_CTYPE, old_locale);
            return NULL;
        }

        // 使用安全转换函数
        if (safe_wcstombs(utf8_key, len + 1, key, len) != 0) {
            free(utf8_key);
            setlocale(LC_CTYPE, old_locale);
            return NULL;
        }

        // 恢复原始locale
        setlocale(LC_CTYPE, old_locale);
    }

    // 使用UTF-8键查询值
    const char* utf8_value = config_get_value(config, utf8_key);
    free(utf8_key);

    if (!utf8_value) return NULL;

    // 将UTF-8值转换为宽字符
    return narrow_to_wide(utf8_value);
}

// 创建文件锁对象
FileLock* file_lock_create(const char* file_path) {
    if (!file_path) return NULL;

    FileLock* lock = (FileLock*)calloc(1, sizeof(FileLock));
    if (!lock) return NULL;

    lock->file_path = safe_strdup(file_path);
    if (!lock->file_path) {
        free(lock);
        return NULL;
    }

#ifdef _WIN32
    // 将UTF-8路径转换为宽字符
    int wlen = MultiByteToWideChar(CP_UTF8, 0, file_path, -1, NULL, 0);
    if (wlen == 0) {
        PRINT_ERROR("MultiByteToWideChar failed");
        free(lock->file_path);
        free(lock);
        return NULL;
    }

    wchar_t* wfile_path = (wchar_t*)malloc(wlen * sizeof(wchar_t));
    if (!wfile_path) {
        PRINT_ERROR("malloc failed for wide string");
        free(lock->file_path);
        free(lock);
        return NULL;
    }

    if (MultiByteToWideChar(CP_UTF8, 0, file_path, -1, wfile_path, wlen) == 0) {
        PRINT_ERROR("MultiByteToWideChar conversion failed");
        free(wfile_path);
        free(lock->file_path);
        free(lock);
        return NULL;
    }

    lock->handle = CreateFileW(
        wfile_path,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    free(wfile_path);

    if (lock->handle == INVALID_HANDLE_VALUE) {
        PRINT_ERROR("CreateFileW failed");
        free(lock->file_path);
        free(lock);
        return NULL;
    }
#else
    lock->handle = open(
        file_path,
        O_RDWR | O_CREAT,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
    );

    if (lock->handle == -1) {
        PRINT_ERROR("open failed");
        free(lock->file_path);
        free(lock);
        return NULL;
    }
#endif

    lock->is_locked = 0;
    return lock;
}

// 锁定文件
int file_lock(FileLock* lock) {
    if (!lock || lock->is_locked) return -1;

#ifdef _WIN32
    lock->lock_region.Offset = 0;
    lock->lock_region.OffsetHigh = 0;

    if (!LockFileEx(
        lock->handle,
        LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY,
        0,
        MAXDWORD,
        MAXDWORD,
        &lock->lock_region
    )) {
        PRINT_ERROR("LockFileEx failed");
        return -1;
    }
#else
    struct flock fl = {
        .l_type = F_WRLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0
    };

    if (fcntl(lock->handle, F_SETLK, &fl) == -1) {
        PRINT_ERROR("fcntl lock failed");
        return -1;
    }
#endif

    lock->is_locked = 1;
    return 0;
}

// 解锁文件
int file_unlock(FileLock* lock) {
    if (!lock || !lock->is_locked) return -1;

#ifdef _WIN32
    if (!UnlockFileEx(
        lock->handle,
        0,
        MAXDWORD,
        MAXDWORD,
        &lock->lock_region
    )) {
        PRINT_ERROR("UnlockFileEx failed");
        return -1;
    }
#else
    struct flock fl = {
        .l_type = F_UNLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0
    };

    if (fcntl(lock->handle, F_SETLK, &fl) == -1) {
        PRINT_ERROR("fcntl unlock failed");
        return -1;
    }
#endif

    lock->is_locked = 0;
    return 0;
}

// 销毁文件锁对象
void file_lock_destroy(FileLock* lock) {
    if (!lock) return;

    if (lock->is_locked) {
        file_unlock(lock);
    }

#ifdef _WIN32
    if (lock->handle != INVALID_HANDLE_VALUE) {
        CloseHandle(lock->handle);
    }
#else
    if (lock->handle != -1) {
        close(lock->handle);
    }
#endif

    free(lock->file_path);
    free(lock);
}

// 读取文件内容
char* file_read_content(FileLock* lock) {
    if (!lock) return NULL;

#ifdef _WIN32
    DWORD size = GetFileSize(lock->handle, NULL);
    if (size == INVALID_FILE_SIZE) {
        PRINT_ERROR("GetFileSize failed");
        return NULL;
    }

    // 为可能存在的 BOM 留3字节空间
    char* content = (char*)malloc(size + 4);
    if (!content) return NULL;

    DWORD bytes_read;
    if (!ReadFile(lock->handle, content, size, &bytes_read, NULL)) {
        PRINT_ERROR("ReadFile failed");
        free(content);
        return NULL;
    }

    // 检查并移除UTF-8 BOM
    if (size >= 3 &&
        (unsigned char)content[0] == 0xEF &&
        (unsigned char)content[1] == 0xBB &&
        (unsigned char)content[2] == 0xBF)
    {
        // 移除BOM
        memmove(content, content + 3, size - 2);
        content[size - 3] = '\0';
    }
    else {
        content[size] = '\0';
    }

    return content;
#else
    struct stat st;
    if (fstat(lock->handle, &st) == -1) {
        PRINT_ERROR("fstat failed");
        return NULL;
    }

    char* content = (char*)malloc(st.st_size + 1);
    if (!content) return NULL;

    if (pread(lock->handle, content, st.st_size, 0) != st.st_size) {
        PRINT_ERROR("pread failed");
        free(content);
        return NULL;
    }

    // 检查并移除UTF-8 BOM
    if (st.st_size >= 3 &&
        (unsigned char)content[0] == 0xEF &&
        (unsigned char)content[1] == 0xBB &&
        (unsigned char)content[2] == 0xBF)
    {
        // 移除BOM
        memmove(content, content + 3, st.st_size - 2);
        content[st.st_size - 3] = '\0';
    }
    else {
        content[st.st_size] = '\0';
    }

    return content;
#endif
}

// 写入文件内容
int file_write_content(FileLock* lock, const char* content) {
    if (!lock || !content) return -1;

    size_t len = strlen(content);

#ifdef _WIN32
    // 截断文件
    SetFilePointer(lock->handle, 0, NULL, FILE_BEGIN);
    SetEndOfFile(lock->handle);

    DWORD bytes_written;
    if (!WriteFile(lock->handle, content, (DWORD)len, &bytes_written, NULL)) {
        PRINT_ERROR("WriteFile failed");
        return -1;
    }

    if (bytes_written != len) {
        fprintf(stderr, "Partial write: %lu/%zu bytes\n", bytes_written, len);
        return -1;
    }
#else
    // 截断文件
    ftruncate(lock->handle, 0);

    if (pwrite(lock->handle, content, len, 0) != (ssize_t)len) {
        PRINT_ERROR("pwrite failed");
        return -1;
    }
#endif

    return 0;
}

#endif // CONFIG_PARSER_H
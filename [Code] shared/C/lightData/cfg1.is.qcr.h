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

// ��ƽ̨�ļ�����֧��
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

// ��������
#ifdef _WIN32
#define PRINT_ERROR(msg) do { \
    fprintf(stderr, "%s (Error %lu)\n", msg, GetLastError()); \
} while(0)
#else
#define PRINT_ERROR(msg) do { \
    fprintf(stderr, "%s: %s\n", msg, strerror(errno)); \
} while(0)
#endif

// ��ֵ�Խṹ
typedef struct {
    char* key;      // UTF-8�������
    char* value;    // UTF-8����ֵ
} KeyValuePair;

// ���ý������
typedef struct {
    KeyValuePair* pairs;    // ��ֵ������
    size_t count;           // ��ֵ������
    size_t capacity;        // ��������
} Config;

// �ļ����ṹ
typedef struct {
    FileHandle handle;      // �ļ����
    char* file_path;        // �ļ�·��
    int is_locked;          // ����״̬��־
#ifdef _WIN32
    OVERLAPPED lock_region; // Windowsר����������
#endif
} FileLock;

// ��ʼ����
#define INITIAL_CAPACITY 16

// ================== �������� ==================
void config_init(Config* config);
void config_free(Config* config);
int config_parse(Config* config, const char* content);
const char* config_get_value(const Config* config, const char* key);

// �������ַ���ѯ����
const wchar_t* wchart_config_get_value(const Config* config, const wchar_t* key);

FileLock* file_lock_create(const char* file_path);
int file_lock(FileLock* lock);
int file_unlock(FileLock* lock);
void file_lock_destroy(FileLock* lock);
char* file_read_content(FileLock* lock);
int file_write_content(FileLock* lock, const char* content);

// ================== ��ƽ̨�������� ==================

// ��ȫ���ַ������ƺ��� (���strdup)
static char* safe_strdup(const char* str) {
    if (!str) return NULL;

    size_t len = strlen(str) + 1;
    char* copy = (char*)malloc(len);
    if (copy) {
        memcpy(copy, str, len);
    }
    return copy;
}

// ��ƽ̨��ȫ���ַ����ָ��
static char* safe_strtok(char* str, const char* delimiters, char** saveptr) {
#ifdef _WIN32
    return strtok_s(str, delimiters, saveptr);
#else
    return strtok_r(str, delimiters, saveptr);
#endif
}

// ================== ���ַ��������� ==================

// ��ȫ�Ķ��ֽ�ת���ַ�����
static int safe_mbstowcs(wchar_t* wcstr, size_t sizeInWords, const char* mbstr, size_t count) {
#ifdef _WIN32
    // ʹ��Windows��ȫ�汾
    return mbstowcs_s(NULL, wcstr, sizeInWords, mbstr, count) == 0 ? 0 : -1;
#else
    // ��Windowsƽ̨ʹ�ñ�׼����
    size_t result = mbstowcs(wcstr, mbstr, sizeInWords);
    return (result == (size_t)-1) ? -1 : 0;
#endif
}

// ��ȫ�Ŀ��ַ�ת���ֽں���
static int safe_wcstombs(char* mbstr, size_t sizeInBytes, const wchar_t* wcstr, size_t count) {
#ifdef _WIN32
    // ʹ��Windows��ȫ�汾
    return wcstombs_s(NULL, mbstr, sizeInBytes, wcstr, count) == 0 ? 0 : -1;
#else
    // ��Windowsƽ̨ʹ�ñ�׼����
    size_t result = wcstombs(mbstr, wcstr, sizeInBytes);
    return (result == (size_t)-1) ? -1 : 0;
#endif
}

// խ�ַ�ת���ַ� (UTF-8 -> wchar_t)
static wchar_t* narrow_to_wide(const char* narrow) {
    if (!narrow) return NULL;

    // ���浱ǰlocale
    char* old_locale = _strdup(setlocale(LC_CTYPE, NULL));
    if (!old_locale) return NULL;

    // ����UTF-8 locale
    if (!setlocale(LC_CTYPE, "en_US.UTF-8")) {
        setlocale(LC_CTYPE, "");
    }

    // �������賤��
    size_t len = 0;
#ifdef _WIN32
    errno_t err = mbstowcs_s(&len, NULL, 0, narrow, 0);
    if (err != 0) {
        setlocale(LC_CTYPE, old_locale);
        free(old_locale);
        return NULL;
    }

    // ���Ȱ���null��ֹ��
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
    len++; // Ϊnull��ֹ�����ӿռ�
#endif

    // �����ڴ沢ת��
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

    // �ָ�ԭʼlocale
    setlocale(LC_CTYPE, old_locale);
    free(old_locale);
    return wide;
}

// ================== ����ʵ�� ==================

// ��ʼ�����ö���
void config_init(Config* config) {
    config->pairs = NULL;
    config->count = 0;
    config->capacity = 0;
}

// �ͷ����ö���
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

// UTF-8�ַ����ȼ���
static int utf8_char_len(unsigned char c) {
    if (c < 0x80) return 1;
    if ((c & 0xE0) == 0xC0) return 2;
    if ((c & 0xF0) == 0xE0) return 3;
    if ((c & 0xF8) == 0xF0) return 4;
    return 1; // ��Ч������Ϊ���ֽڴ���
}

// ��ȷת�崦��
static char* parse_value(const char* input) {
    if (!input) return safe_strdup("");

    size_t len = strlen(input);
    // �����㹻�ռ䣨������ÿ���ַ�����Ҫת�壩
    char* result = (char*)malloc(len * 2 + 1);
    if (!result) return NULL;

    size_t j = 0;
    const char* p = input;

    while (*p) {
        int char_len = utf8_char_len((unsigned char)*p);

        if (*p == '\\') {
            // ����ת������
            if (*(p + 1)) {
                // ����һ���ַ�
                int next_len = utf8_char_len((unsigned char)*(p + 1));
                if (*(p + 1) == ' ') {
                    // ��б��+�ո񣺱�������
                    memcpy(result + j, p, char_len);
                    j += char_len;
                    p += char_len;
                    memcpy(result + j, p, next_len);
                    j += next_len;
                    p += next_len;
                }
                else {
                    // ����ת���ַ���ֻ����ת�����ַ�
                    p += char_len; // ������б��
                    memcpy(result + j, p, next_len);
                    j += next_len;
                    p += next_len;
                }
            }
            else {
                // ��ĩ������б�ܣ�����
                memcpy(result + j, p, char_len);
                j += char_len;
                p += char_len;
            }
        }
        else if (*p == '#') {
            // δת���#��ע�Ϳ�ʼ
            break;
        }
        else {
            // ��ͨ�ַ����������ֽ��ַ���
            memcpy(result + j, p, char_len);
            j += char_len;
            p += char_len;
        }
    }

    result[j] = '\0';

    // �޼���β�հ�
    char* start = result;
    while (isspace((unsigned char)*start)) start++;

    char* end = result + j - 1;
    while (end > start && isspace((unsigned char)*end)) end--;
    *(end + 1) = '\0';

    // �����ڴ��˷�
    char* trimmed = safe_strdup(start);
    free(result);
    return trimmed;
}

// �����Ƿ��Ѵ���
static int key_exists(const Config* config, const char* key) {
    for (size_t i = 0; i < config->count; i++) {
        if (strcmp(config->pairs[i].key, key) == 0) {
            return 1;
        }
    }
    return 0;
}

// ���Ӽ�ֵ��
static int config_add_pair(Config* config, const char* key, const char* raw_value) {
    // �����Ƿ�Ϊ��
    if (!key || !*key) return 0;

    // �����Ƿ��ظ�
    if (key_exists(config, key)) {
        fprintf(stderr, "Duplicate key error: %s\n", key);
        return 0;
    }

    // ��չ����
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

    // �����
    config->pairs[config->count].key = safe_strdup(key);
    if (!config->pairs[config->count].key) {
        perror("dup_str key failed");
        return 0;
    }

    // ����ֵ
    config->pairs[config->count].value = parse_value(raw_value);
    if (!config->pairs[config->count].value) {
        perror("parse_value failed");
        free(config->pairs[config->count].key);
        return 0;
    }

    config->count++;
    return 1;
}

// ���������ļ�����
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

        // �������к�ע����
        if (!*token || *token == '#') {
            token = safe_strtok(NULL, "\n", &saveptr);
            continue;
        }

        // ����ð�ŷָ���
        char* colon = strchr(token, ':');
        if (!colon) {
            // û��ð�ţ�����
            token = safe_strtok(NULL, "\n", &saveptr);
            continue;
        }

        // �ָ����ֵ
        *colon = '\0';
        char* key = token;
        char* value = colon + 1;

        // �޼�������β�հ�
        char* key_end = key + strlen(key) - 1;
        while (key_end > key && isspace((unsigned char)*key_end)) {
            *key_end-- = '\0';
        }
        while (isspace((unsigned char)*key)) {
            key++;
        }

        // ���Ӽ�ֵ��
        if (!config_add_pair(config, key, value)) {
            free(line);
            return 0;
        }

        token = safe_strtok(NULL, "\n", &saveptr);
    }

    free(line);
    return 1;
}

// ���ݼ�����ֵ (UTF-8)
const char* config_get_value(const Config* config, const char* key) {
    if (!config || !key) return NULL;

    for (size_t i = 0; i < config->count; i++) {
        if (strcmp(config->pairs[i].key, key) == 0) {
            return config->pairs[i].value;
        }
    }
    return NULL;
}

// �������������ַ���ֵ��ѯ
const wchar_t* wchart_config_get_value(const Config* config, const wchar_t* key) {
    if (!config || !key) return NULL;

    // �����ַ���ת��ΪUTF-8
    char* utf8_key = NULL;
    {
        // ����locale����ת��
        char* old_locale = safe_strdup(setlocale(LC_CTYPE, NULL));
        char* new_locale = setlocale(LC_CTYPE, "en_US.UTF-8");
        if (!new_locale) {
            setlocale(LC_CTYPE, "");
        }

        // �������賤��
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

        // �����ڴ沢ת��
        utf8_key = (char*)malloc(len + 1);
        if (!utf8_key) {
            setlocale(LC_CTYPE, old_locale);
            return NULL;
        }

        // ʹ�ð�ȫת������
        if (safe_wcstombs(utf8_key, len + 1, key, len) != 0) {
            free(utf8_key);
            setlocale(LC_CTYPE, old_locale);
            return NULL;
        }

        // �ָ�ԭʼlocale
        setlocale(LC_CTYPE, old_locale);
    }

    // ʹ��UTF-8����ѯֵ
    const char* utf8_value = config_get_value(config, utf8_key);
    free(utf8_key);

    if (!utf8_value) return NULL;

    // ��UTF-8ֵת��Ϊ���ַ�
    return narrow_to_wide(utf8_value);
}

// �����ļ�������
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
    // ��UTF-8·��ת��Ϊ���ַ�
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

// �����ļ�
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

// �����ļ�
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

// �����ļ�������
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

// ��ȡ�ļ�����
char* file_read_content(FileLock* lock) {
    if (!lock) return NULL;

#ifdef _WIN32
    DWORD size = GetFileSize(lock->handle, NULL);
    if (size == INVALID_FILE_SIZE) {
        PRINT_ERROR("GetFileSize failed");
        return NULL;
    }

    // Ϊ���ܴ��ڵ� BOM ��3�ֽڿռ�
    char* content = (char*)malloc(size + 4);
    if (!content) return NULL;

    DWORD bytes_read;
    if (!ReadFile(lock->handle, content, size, &bytes_read, NULL)) {
        PRINT_ERROR("ReadFile failed");
        free(content);
        return NULL;
    }

    // ��鲢�Ƴ�UTF-8 BOM
    if (size >= 3 &&
        (unsigned char)content[0] == 0xEF &&
        (unsigned char)content[1] == 0xBB &&
        (unsigned char)content[2] == 0xBF)
    {
        // �Ƴ�BOM
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

    // ��鲢�Ƴ�UTF-8 BOM
    if (st.st_size >= 3 &&
        (unsigned char)content[0] == 0xEF &&
        (unsigned char)content[1] == 0xBB &&
        (unsigned char)content[2] == 0xBF)
    {
        // �Ƴ�BOM
        memmove(content, content + 3, st.st_size - 2);
        content[st.st_size - 3] = '\0';
    }
    else {
        content[st.st_size] = '\0';
    }

    return content;
#endif
}

// д���ļ�����
int file_write_content(FileLock* lock, const char* content) {
    if (!lock || !content) return -1;

    size_t len = strlen(content);

#ifdef _WIN32
    // �ض��ļ�
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
    // �ض��ļ�
    ftruncate(lock->handle, 0);

    if (pwrite(lock->handle, content, len, 0) != (ssize_t)len) {
        PRINT_ERROR("pwrite failed");
        return -1;
    }
#endif

    return 0;
}

#endif // CONFIG_PARSER_H
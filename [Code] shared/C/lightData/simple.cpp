#include "cfg1.is.qcr.h"
#include <stdio.h>
#include <locale.h>
#include <wchar.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// ��ȫ�� UTF-8 �����������ƽ̨�꣩
static void safe_print_utf8(const char* str) {
    if (!str) return;

#ifdef _WIN32
    // ת��Ϊ���ַ����
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
    // ���˷���
    printf("%s", str);
}

// ��ӡ�������������
static void print_config_item(size_t index, const char* key, const char* value) {
    printf("  [%zu] ", index);
    safe_print_utf8(key);
    printf(": ");
    safe_print_utf8(value);
    printf("\n");
}

int simple() {
    // ���ÿ���̨֧�� UTF-8
#ifdef _WIN32
    // ���ÿ���̨���Ϊ UTF-8
    SetConsoleOutputCP(CP_UTF8);
    // ���ÿ���̨����Ϊ UTF-8
    SetConsoleCP(CP_UTF8);

    // ���ÿ���̨����֧������
    CONSOLE_FONT_INFOEX font = { sizeof(font) };
    font.dwFontSize.Y = 16;
    wcscpy_s(font.FaceName, LF_FACESIZE, L"SimSun-ExtB"); // ʹ��֧�����ĵ�����
    SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), FALSE, &font);
#else
    // �� Linux/macOS ������ UTF-8 ����
    setenv("LC_ALL", "en_US.UTF-8", 1);
#endif

    // ���ñ��ػ�
    setlocale(LC_ALL, "zh_CN.UTF-8");

    wprintf(L"=== �����ļ���ֵ��ȡʾ�� ===\n\n");

    const char* filename = "test.txt";

    // 1. �����ļ�������
    FileLock* lock = file_lock_create(filename);
    if (!lock) {
        wprintf(L"�����ļ���ʧ��\n");
        return 1;
    }
    wprintf(L"�Ѵ����ļ���: %hs\n", filename);

    // 2. �����ļ�
    if (file_lock(lock) != 0) {
        wprintf(L"�ļ�����ʧ��\n");
        file_lock_destroy(lock);
        return 1;
    }
    wprintf(L"�ļ�������\n");

    // 3. ��ȡ�ļ�����
    char* content = file_read_content(lock);
    if (!content) {
        wprintf(L"��ȡ�ļ�����ʧ��\n");
        file_unlock(lock);
        file_lock_destroy(lock);
        return 1;
    }

    wprintf(L"\n=== �ļ�ԭʼ���� ===\n");
    safe_print_utf8(content);
    wprintf(L"\n=====================\n");

    // 4. ��������
    Config config;
    config_init(&config);

    if (!config_parse(&config, content)) {
        wprintf(L"���������ļ�ʧ��\n");
        free(content);
        config_free(&config);
        file_unlock(lock);
        file_lock_destroy(lock);
        return 1;
    }
    free(content);

    // 5. ��ȡ����ӡ���м�ֵ��
    wprintf(L"\n=== ������� (�� %zu ����ֵ��) ===\n", config.count);
    for (size_t i = 0; i < config.count; i++) {
        print_config_item(i + 1, config.pairs[i].key, config.pairs[i].value);
    }
    wprintf(L"===============================\n");

    // 6. ���ҵ�����ֵ�ԣ�խ�ַ���
    wprintf(L"\n=== ���ҵ�����ֵ�� (խ�ַ�) ===\n");

    // ����Ҫ���ҵļ��б�
    const char* keys_to_find[] = {
        "Ӧ������", "�汾", "����", "·��", "˵��",
        "������ֵ", "���ù���", "���������", "�����ڵļ�"
    };

    for (size_t i = 0; i < sizeof(keys_to_find) / sizeof(keys_to_find[0]); i++) {
        const char* key = keys_to_find[i];
        const char* value = config_get_value(&config, key);

        if (value) {
            wprintf(L"  �ҵ��� '");
            safe_print_utf8(key);
            wprintf(L"' ��ֵ: '");
            safe_print_utf8(value);
            wprintf(L"'\n");
        }
        else {
            wprintf(L"  δ�ҵ��� '");
            safe_print_utf8(key);
            wprintf(L"'\n");
        }
    }

    // 7. ���ҵ�����ֵ�ԣ����ַ���
    wprintf(L"\n=== ���ҵ�����ֵ�� (���ַ�) ===\n");
    const wchar_t* wide_keys[] = {
        L"Ӧ������", L"�汾", L"����", L"·��", L"˵��",
        L"������ֵ", L"���ù���", L"���������", L"�����ڵļ�"
    };

    for (size_t i = 0; i < sizeof(wide_keys) / sizeof(wide_keys[0]); i++) {
        const wchar_t* key = wide_keys[i];
        const wchar_t* value = wchart_config_get_value(&config, key);

        if (value) {
            wprintf(L"  �ҵ��� '%s' ��ֵ: '%s'\n", key, value);
            free((void*)value); // �ͷſ��ַ���ѯ���ص��ڴ�
        }
        else {
            wprintf(L"  δ�ҵ��� '%s'\n", key);
        }
    }

    // 8. ���Ҳ��޸ļ�ֵ
    wprintf(L"\n=== ���Ҳ��޸ļ�ֵ ===\n");
    const wchar_t* key_to_modify = L"���������";
    const wchar_t* old_value = wchart_config_get_value(&config, key_to_modify);

    if (old_value) {
        wprintf(L"  �� '%s' �ĵ�ǰֵ: %s\n", key_to_modify, old_value);

        // �޸�ֵ��ʵ��Ӧ���п��ܴ��û������ȡ��
        wchar_t new_value[50];
        wchar_t* end;
        long int_value = wcstol(old_value, &end, 10);

        if (*end == L'\0') { // ȷ�������ַ�������ת��
            swprintf(new_value, sizeof(new_value) / sizeof(wchar_t), L"%ld", int_value + 10);
            wprintf(L"  ������ֵ: %s\n", new_value);

            // ��ʵ��Ӧ���У�����Ὣ�޸�д������
            // ���磺�޸������ļ����ݲ�д��
            //file_write_content(lock, new_content);
        }
        else {
            wprintf(L"  �޷�������ֵ\n");
        }

        free((void*)old_value); // �ͷſ��ַ���ѯ���ص��ڴ�
    }
    else {
        wprintf(L"  �Ҳ����� '%s', �޷��޸�\n", key_to_modify);
    }

    // 9. ������Դ
    config_free(&config);

    // 10. �����ļ�
    if (file_unlock(lock) != 0) {
        wprintf(L"�ļ�����ʧ��\n");
    }
    else {
        wprintf(L"\n�ļ��ѽ���\n");
    }

    file_lock_destroy(lock);

    wprintf(L"\n�������\n");
    return 0;
}
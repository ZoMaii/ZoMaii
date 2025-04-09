// gcc BT_BLE_LINUX_TEST.c -o ble -std=c99 -lgattlib
// gattlib 须在 github 中克隆仓库 https://github.com/labapart/gattlib/tree/0.7.2
// cmake , make install , /etc/ld.so.conf.d/*.conf , ldconfig
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <gattlib.h>

#define SERVICE_UUID        "180f"
#define CHARACTERISTIC_UUID "2a19"

// 全局变量，用于异步回调中保存连接
static gattlib_connection_t* g_connection = NULL;

// 连接完成的回调，匹配 gatt_connect_cb_t
static void on_connect(gattlib_adapter_t* adapter,
                       const char* dst,
                       gattlib_connection_t* connection,
                       int error,
                       void* user_data) {
    if (error != GATTLIB_SUCCESS) {
        fprintf(stderr, "连接失败，错误码: %d\n", error);
        g_connection = NULL;
    } else {
        g_connection = connection;
    }
}

// 打印用法
static void print_usage(const char *prog_name) {
    fprintf(stderr, "用法: %s -d <设备地址> [-f <文件位置> | -t <文本>]\n", prog_name);
}

// 连接到 BLE 设备（异步接口 + 同步等待）
static gattlib_connection_t* connect_to_device(const char *addr) {
    g_connection = NULL;
    unsigned long options = GATTLIB_CONNECTION_OPTIONS_LEGACY_BDADDR_LE_PUBLIC
                          | GATTLIB_CONNECTION_OPTIONS_LEGACY_BT_SEC_LOW;
    int ret = gattlib_connect(NULL, addr, options, on_connect, NULL);
    if (ret != GATTLIB_SUCCESS) {
        fprintf(stderr, "gattlib_connect 返回错误: %d\n", ret);
        return NULL;
    }
    // 最多等待 5 秒，让回调触发
    for (int i = 0; i < 5 && g_connection == NULL; i++) {
        sleep(1);
    }
    if (g_connection == NULL) {
        fprintf(stderr, "连接超时\n");
    } else {
        printf("Connected to %s\n", addr);
    }
    return g_connection;
}

// 读取 Characteristic
static int read_characteristic(gattlib_connection_t *connection, const uuid_t *uuid) {
    void*  buffer = NULL;
    size_t len    = 0;
    int    ret    = gattlib_read_char_by_uuid(connection, (uuid_t*)uuid, &buffer, &len);
    if (ret != GATTLIB_SUCCESS) {
        fprintf(stderr, "读取特征值失败: %d\n", ret);
        return -1;
    }
    printf("Characteristic value (%zu bytes): ", len);
    for (size_t i = 0; i < len; i++) {
        printf("%02x ", ((uint8_t*)buffer)[i]);
    }
    printf("\n");
    free(buffer);
    return 0;
}

// 写入 Characteristic
static int write_characteristic(gattlib_connection_t *connection, const uuid_t *uuid,
                                const uint8_t *data, size_t len) {
    int ret = gattlib_write_char_by_uuid(connection, (uuid_t*)uuid, data, len);
    if (ret != GATTLIB_SUCCESS) {
        fprintf(stderr, "写入特征值失败: %d\n", ret);
        return -1;
    }
    printf("Characteristic written successfully\n");
    return 0;
}

int main(int argc, char *argv[]) {
    const char *device_addr = NULL;
    const char *file_path   = NULL;
    const char *text        = NULL;

    int opt;
    while ((opt = getopt(argc, argv, "d:f:t:")) != -1) {
        switch (opt) {
            case 'd': device_addr = optarg; break;
            case 'f': file_path   = optarg; break;
            case 't': text        = optarg; break;
            default:
                print_usage(argv[0]);
                return EXIT_FAILURE;
        }
    }
    if (!device_addr) {
        fprintf(stderr, "错误: 必须指定设备地址 (-d)\n");
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    printf("设备地址: %s\n", device_addr);
    if (file_path) printf("文件: %s\n", file_path);
    if (text)      printf("文本: %s\n", text);

    // 1) 连接
    gattlib_connection_t *conn = connect_to_device(device_addr);
    if (!conn) return EXIT_FAILURE;

    // 2) 转换 UUID
    uuid_t svc_uuid, char_uuid;
    if (gattlib_string_to_uuid(SERVICE_UUID, strlen(SERVICE_UUID), &svc_uuid) < 0 ||
        gattlib_string_to_uuid(CHARACTERISTIC_UUID, strlen(CHARACTERISTIC_UUID), &char_uuid) < 0) {
        fprintf(stderr, "UUID 解析失败\n");
        gattlib_disconnect(conn, false);
        return EXIT_FAILURE;
    }

    // 3) 读特征值
    if (read_characteristic(conn, &char_uuid) < 0) {
        gattlib_disconnect(conn, false);
        return EXIT_FAILURE;
    }

    // 4) 写特征值：从文件或文本
    uint8_t *data = NULL;
    size_t   len  = 0;
    if (file_path) {
        FILE *f = fopen(file_path, "rb");
        if (!f) { perror("打开文件"); gattlib_disconnect(conn, false); return EXIT_FAILURE; }
        fseek(f, 0, SEEK_END); len = ftell(f); fseek(f, 0, SEEK_SET);
        data = malloc(len);
        if (!data) { perror("malloc"); fclose(f); gattlib_disconnect(conn, false); return EXIT_FAILURE; }
        fread(data, 1, len, f); fclose(f);
    } else if (text) {
        len  = strlen(text);
        data = malloc(len);
        if (!data) { perror("malloc"); gattlib_disconnect(conn, false); return EXIT_FAILURE; }
        memcpy(data, text, len);
    }
    if (data) {
        if (write_characteristic(conn, &char_uuid, data, len) < 0) {
            free(data);
            gattlib_disconnect(conn, false);
            return EXIT_FAILURE;
        }
        free(data);
    }

    // 5) 断开
    gattlib_disconnect(conn, false);
    printf("Disconnected\n");
    return EXIT_SUCCESS;
}



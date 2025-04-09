/**
Debian/Ubuntu	sudo apt-get install libbluetooth-dev
CentOS/RHEL	    sudo yum install bluez-libs-devel
Fedora	        sudo dnf install bluez-libs-devel
Arch/Manjaro	sudo pacman -S bluez-libs

stop firewalld.service on Linux
download "Serial Bluetooth Terminal" on Android (GooglePlay)

gcc bt.c -o bt -lbluetooth -std=c99
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

typedef struct {
    char dest_addr[18];   // 目标设备地址
    char file_path[256];  // 文件路径
    int is_text;          // 是否为文本模式
    int channel;          // RFCOMM通道号（-1表示自动检测）
} Config;

// 解析命令行参数
void parse_args(int argc, char *argv[], Config *config) {
    // 初始化默认值
    config->channel = -1;  // -1表示未指定通道号
    config->is_text = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 && i+1 < argc) {
            strncpy(config->dest_addr, argv[++i], 17);
        } else if (strcmp(argv[i], "-f") == 0 && i+1 < argc) {
            strncpy(config->file_path, argv[++i], 255);
            config->is_text = 0;
        } else if (strcmp(argv[i], "-t") == 0 && i+1 < argc) {
            strncpy(config->file_path, argv[++i], 255);
            config->is_text = 1;
        } else if (strcmp(argv[i], "-c") == 0 && i+1 < argc) {
            config->channel = atoi(argv[++i]);
        }
    }
}

// 通过 SDP 寻找 RFCOMM 服务
int auto_detect_channel(const char *dest_addr) {
    bdaddr_t target;
    str2ba(dest_addr, &target);

    sdp_session_t *session = sdp_connect(BDADDR_ANY, &target, SDP_RETRY_IF_BUSY);
    if (!session) {
        perror("SDP连接失败");
        return -1;
    }

    // 搜索RFCOMM服务
    uuid_t svc_uuid;
    sdp_uuid16_create(&svc_uuid, SERIAL_PORT_SVCLASS_ID);  // 使用 SERIAL_PORT 以获取更通用服务
    sdp_list_t *search_list = sdp_list_append(NULL, &svc_uuid);

    uint32_t range = 0x0000ffff;
    sdp_list_t *attrid_list = sdp_list_append(NULL, &range);

    sdp_list_t *response_list = NULL;
    int channel = -1;

    if (sdp_service_search_attr_req(session, search_list, SDP_ATTR_REQ_RANGE, attrid_list, &response_list) == 0) {
        for (sdp_list_t *r = response_list; r != NULL; r = r->next) {
            sdp_record_t *rec = (sdp_record_t *)r->data;
            sdp_list_t *proto_list = NULL;

            if (sdp_get_access_protos(rec, &proto_list) == 0) {
                for (sdp_list_t *p = proto_list; p != NULL; p = p->next) {
                    sdp_list_t *pds = (sdp_list_t *)p->data;
                    for (sdp_list_t *pd = pds; pd != NULL; pd = pd->next) {
                        sdp_data_t *d = (sdp_data_t *)pd->data;
                        int proto = 0;
                        for (; d != NULL; d = d->next) {
                            if (d->dtd == SDP_UUID16 || d->dtd == SDP_UUID32 || d->dtd == SDP_UUID128) {
                                proto = sdp_uuid_to_proto(&d->val.uuid);
                            }
                            if (proto == RFCOMM_UUID && d->dtd == SDP_UINT8) {
                                channel = d->val.uint8;
                                break;
                            }
                        }
                        if (channel != -1) break;
                    }
                    if (channel != -1) break;
                }
                sdp_list_free(proto_list, 0);  // 使用 0 只释放链表结构，不释放 data（由 BlueZ 管理）
            }
            sdp_record_free(rec);
            if (channel != -1) break;
        }
    } else {
        fprintf(stderr, "SDP服务搜索失败\n");
    }

    sdp_list_free(search_list, NULL);
    sdp_list_free(attrid_list, NULL);
    sdp_list_free(response_list, (sdp_free_func_t)sdp_record_free);
    sdp_close(session);

    return channel;
}



// 发送文件/文本数据
void send_data(Config *config) {
    // 确定通道号
    int channel = config->channel;
    if (channel == -1) {  // 自动检测
        printf("正在自动检测RFCOMM通道号...\n");
        if ((channel = auto_detect_channel(config->dest_addr)) == -1) {
            fprintf(stderr, "错误：未找到可用的RFCOMM服务\n");
            exit(EXIT_FAILURE);
        }
        printf("检测到通道号: %d\n", channel);
    }

    // 创建套接字
    int sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if (sock < 0) {
        perror("创建套接字失败");
        exit(EXIT_FAILURE);
    }

    // 配置目标地址
    struct sockaddr_rc addr = {0};
    addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = (uint8_t)channel;
    str2ba(config->dest_addr, &addr.rc_bdaddr);

    // 建立连接
    printf("正在连接 %s (通道 %d)...\n", config->dest_addr, channel);
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("连接失败");
        close(sock);
        exit(EXIT_FAILURE);
    }
    printf("连接成功!\n");

    // 打开文件/文本
    FILE *file = NULL;
    if (config->is_text) {
        file = fopen(config->file_path, "r");
    } else {
        file = fopen(config->file_path, "rb");
    }

    if (!file) {
        perror("打开文件失败");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // 传输数据
    char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (write(sock, buffer, bytes_read) != bytes_read) {
            perror("写入失败");
            break;
        }
    }

    // 清理资源
    fclose(file);
    close(sock);
    printf("传输完成\n");
}

int main(int argc, char *argv[]) {
    Config config = {0};

    parse_args(argc, argv, &config);

    if (strlen(config.dest_addr) == 0) {
        fprintf(stderr, "用法: %s -d <设备地址> [-c 通道号] [-f 文件] [-t 文本]\n", argv[0]);
        fprintf(stderr, "示例:\n");
        fprintf(stderr, "  %s -d 00:11:22:33:44:55 -c 1 -f data.bin\n", argv[0]);
        fprintf(stderr, "  %s -d 00:11:22:33:44:55 -t message.txt\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    send_data(&config);
    return EXIT_SUCCESS;
}
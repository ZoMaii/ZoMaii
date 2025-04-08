/**
Debian/Ubuntu	sudo apt-get install libbluetooth-dev
CentOS/RHEL	    sudo yum install bluez-libs-devel
Fedora	        sudo dnf install bluez-libs-devel
Arch/Manjaro	sudo pacman -S bluez-libs

stop firewalld.service on Linux
download "Serial Bluetooth Terminal" on Android (GooglePlay)
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

typedef struct {
    char dest_addr[18];
    char file_path[256];
    int is_text;
} Config;

void parse_args(int argc, char *argv[], Config *config) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 && i+1 < argc) {
            strncpy(config->dest_addr, argv[++i], 17);
        } else if (strcmp(argv[i], "-f") == 0 && i+1 < argc) {
            strncpy(config->file_path, argv[++i], 255);
            config->is_text = 0;
        } else if (strcmp(argv[i], "-t") == 0 && i+1 < argc) {
            strncpy(config->file_path, argv[++i], 255);
            config->is_text = 1;
        }
    }
}

void send_data(Config *config) {
    struct sockaddr_rc addr = {0};
    int sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    
    addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = 1;
    str2ba(config->dest_addr, &addr.rc_bdaddr);

    // 修复点1：补全 connect 函数的右括号
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {  // <- 关键修复
        perror("连接失败");
        exit(1);
    }

    FILE *file = fopen(config->file_path, config->is_text ? "r" : "rb");
    if (!file) {
        perror("文件打开失败");
        close(sock);
        exit(1);
    }

    char buffer[1024];
    size_t bytes_read;
    // 修复点2：补全 while 循环条件
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {  // <- 关键修复
        write(sock, buffer, bytes_read);
    }

    fclose(file);
    close(sock);
}

int main(int argc, char *argv[]) {
    Config config = {0};
    parse_args(argc, argv, &config);
    
    if (strlen(config.dest_addr) == 0) {
        fprintf(stderr, "用法: %s -d <设备地址> [-f <文件位置> | -t <文本>]\n", argv[0]);
        exit(1);
    }

    send_data(&config);
    return 0;
}



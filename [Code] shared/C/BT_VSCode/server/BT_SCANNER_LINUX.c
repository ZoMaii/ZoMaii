
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

int main() {
    // 获取本地蓝牙适配器 ID（默认第一个适配器）
    int dev_id = hci_get_route(NULL);
    if (dev_id < 0) {
        perror("找不到蓝牙适配器");
        exit(1);
    }

    // 打开蓝牙设备
    int sock = hci_open_dev(dev_id);
    if (sock < 0) {
        perror("无法打开蓝牙适配器");
        exit(1);
    }

    // 设置扫描参数
    int max_rsp = 255;      // 最大响应设备数
    int flags = IREQ_CACHE_FLUSH; // 刷新缓存
    inquiry_info *devs = (inquiry_info*)malloc(max_rsp * sizeof(inquiry_info));

    // 执行扫描（经典蓝牙设备）
    int num_rsp = hci_inquiry(dev_id, 8, max_rsp, NULL, &devs, flags);
    if (num_rsp < 0) {
        perror("扫描失败");
        exit(1);
    }

    printf("发现 %d 个蓝牙设备:\n", num_rsp);

    // 遍历所有发现的设备
    for (int i = 0; i < num_rsp; i++) {
        char addr[19] = {0};
        char name[248] = {0};

        // 获取设备地址
        ba2str(&(devs+i)->bdaddr, addr);

        // 尝试读取设备名称（可能需要设备响应）
        if (hci_read_remote_name(sock, &(devs+i)->bdaddr, sizeof(name), name, 0) < 0) {
            strcpy(name, "[未知]");
        }

        printf("地址: %s \t 名称: %s\n", addr, name);
    }

    // 清理资源
    free(devs);
    close(sock);
    return 0;
}


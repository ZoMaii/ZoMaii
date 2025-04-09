// 编译命令: gcc BT_SCANNER_BLE_LINUX.c -std=c99 -lbluetooth

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

// BLE 广播数据解析函数
static void parse_adv_data(const uint8_t *data, size_t len) {
    size_t offset = 0;
    while (offset < len) {
        uint8_t field_len = data[offset];
        if (field_len == 0 || offset + field_len > len) break;

        uint8_t type = data[offset + 1];
        const uint8_t *value = &data[offset + 2];
        uint8_t value_len = field_len - 1;

        // 解析广播数据类型 (参考: https://www.bluetooth.com/specifications/assigned-numbers/generic-access-profile/)
        switch (type) {
            case 0x01: // Flags
                printf("    Flags: 0x%02x\n", *value);
                break;
            case 0x09: // Complete Local Name
                printf("    Name: %.*s\n", value_len, value);
                break;
            case 0x03: // Complete 16-bit Service UUIDs
                printf("    Services: ");
                for (int i = 0; i < value_len/2; i++) {
                    printf("%04x ", value[i*2] | (value[i*2+1] << 8));
                }
                printf("\n");
                break;
            // 可扩展其他数据类型解析
        }
        offset += field_len + 1;
    }
}

int main() {
    // 获取蓝牙适配器 ID
    int dev_id = hci_get_route(NULL);
    if (dev_id < 0) {
        perror("无法找到蓝牙适配器");
        return 1;
    }

    // 打开 HCI Socket
    int sock = hci_open_dev(dev_id);
    if (sock < 0) {
        perror("无法打开蓝牙适配器");
        return 1;
    }

    // 设置 BLE 扫描参数
    struct hci_filter old_filter, new_filter;
    socklen_t olen = sizeof(old_filter);
    if (getsockopt(sock, SOL_HCI, HCI_FILTER, &old_filter, &olen) < 0) {
        perror("获取原始过滤器失败");
        close(sock);
        return 1;
    }

    // 设置新过滤器（仅接收 LE Meta 事件）
    hci_filter_clear(&new_filter);
    hci_filter_set_ptype(HCI_EVENT_PKT, &new_filter);
    hci_filter_set_event(EVT_LE_META_EVENT, &new_filter);
    if (setsockopt(sock, SOL_HCI, HCI_FILTER, &new_filter, sizeof(new_filter)) < 0) {
        perror("设置过滤器失败");
        close(sock);
        return 1;
    }

    // 启用 BLE 扫描
    le_set_scan_parameters_cp scan_params = {
        .type = 0x01,       // 主动扫描
        .interval = htobs(0x0010),
        .window = htobs(0x0010),
        .own_bdaddr_type = 0x00,
        .filter = 0x00
    };

    struct hci_request scan_req = {
        .ogf = OGF_LE_CTL,
        .ocf = OCF_LE_SET_SCAN_PARAMETERS,
        .event = EVT_CMD_COMPLETE,
        .cparam = &scan_params,
        .clen = LE_SET_SCAN_PARAMETERS_CP_SIZE
    };

    if (hci_send_req(sock, &scan_req, 1000) < 0) {
        perror("设置扫描参数失败");
        close(sock);
        return 1;
    }

    le_set_scan_enable_cp scan_enable = {
        .enable = 0x01,     // 启用扫描
        .filter_dup = 0x00  // 不过滤重复
    };

    struct hci_request enable_req = {
        .ogf = OGF_LE_CTL,
        .ocf = OCF_LE_SET_SCAN_ENABLE,
        .event = EVT_CMD_COMPLETE,
        .cparam = &scan_enable,
        .clen = LE_SET_SCAN_ENABLE_CP_SIZE
    };

    if (hci_send_req(sock, &enable_req, 1000) < 0) {
        perror("启用扫描失败");
        close(sock);
        return 1;
    }

    printf("正在扫描 BLE 设备... (按 Ctrl+C 停止)\n");

    // 循环接收事件
    uint8_t buf[HCI_MAX_EVENT_SIZE];
    while (1) {
        int len = read(sock, buf, sizeof(buf));
        if (len < 0) {
            perror("读取事件失败");
            break;
        }

        evt_le_meta_event *meta = (evt_le_meta_event*)(buf + (1 + HCI_EVENT_HDR_SIZE));
        if (meta->subevent != EVT_LE_ADVERTISING_REPORT) continue;

        le_advertising_info *info = (le_advertising_info*)(meta->data + 1);
        char addr[18];
        ba2str(&info->bdaddr, addr);

        // 打印设备基本信息
        printf("发现 BLE 设备:\n");
        printf("  Address: %s\n", addr);
        printf("  RSSI: %d dBm\n", info->data[info->length]);

        // 解析广播数据
        parse_adv_data(info->data, info->length);
        printf("------------------------\n");
    }

    // 恢复原始过滤器并关闭
    setsockopt(sock, SOL_HCI, HCI_FILTER, &old_filter, sizeof(old_filter));
    close(sock);
    return 0;
}


#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2bth.h>
#include <bluetoothapis.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Bthprops.lib")

// ǰ������
int GetRfcommPort(ULONGLONG btAddr, const GUID& uuid);

int wmain(int argc, wchar_t* argv[]) {
    if (argc != 3) {
        printf("�÷�: %S <���ε������豸��> <��Ϣ>\n", argv[0]);
        return 1;
    }

    // ��Ϣת��
    const int msgLen = WideCharToMultiByte(CP_UTF8, 0, argv[2], -1, NULL, 0, NULL, NULL);
    char* message = (char*)malloc(msgLen);
    WideCharToMultiByte(CP_UTF8, 0, argv[2], -1, message, msgLen, NULL, NULL);

    // �豸����
    BLUETOOTH_DEVICE_SEARCH_PARAMS searchParams = {
        sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS),
        1,  // ������������֤�豸
        TRUE, TRUE, FALSE, TRUE,  // ��������
        TRUE,  // ������������
    };

    BLUETOOTH_DEVICE_INFO deviceInfo = { sizeof(BLUETOOTH_DEVICE_INFO) };
    HBLUETOOTH_DEVICE_FIND hFind = BluetoothFindFirstDevice(&searchParams, &deviceInfo);
    ULONGLONG targetAddr = 0;

    if (hFind) {
        do {
            if (_wcsicmp(deviceInfo.szName, argv[1]) == 0) {
                targetAddr = deviceInfo.Address.ullLong;
                break;
            }
        } while (BluetoothFindNextDevice(hFind, &deviceInfo));
        BluetoothFindDeviceClose(hFind);
    }

    if (targetAddr == 0) {
        printf("�޷��ҵ������ε������豸\n");
        free(message);
        return 1;
    }

    // ��ȡ�˿�
    const int port = GetRfcommPort(targetAddr, SerialPortServiceClass_UUID);
    if (port <= 0) {
        printf("��Ӧ�豸�� SPP ����δ�ҵ�\n");
        free(message);
        return 1;
    }

    // Winsock��ʼ��
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // ����������Socket
    SOCKET btSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
    SOCKADDR_BTH addr = {
        AF_BTH,
        targetAddr,
        (ULONG)port,  // ��ʽ����ת��
        0
    };

    if (connect(btSocket, (SOCKADDR*)&addr, sizeof(addr)) != 0) {
        printf("����ʧ��: %d\n", WSAGetLastError());
        closesocket(btSocket);
        WSACleanup();
        free(message);
        return 1;
    }

    // ��������
    const int totalSent = send(btSocket, message, (int)strlen(message), 0);
    if (totalSent == SOCKET_ERROR) {
        printf("����ʧ��: %d\n", WSAGetLastError());
    }
    else {
        printf("���� %d �ֽ�(bytes)\n", totalSent);
    }

    // ������Դ
    closesocket(btSocket);
    WSACleanup();
    free(message);
    return 0;
}

// ������Ķ˿ڲ�ѯ����
int GetRfcommPort(ULONGLONG btAddr, const GUID& uuid) {
    WSAQUERYSETW querySet = { sizeof(WSAQUERYSETW) };
    querySet.dwNameSpace = NS_BTH;
    querySet.lpServiceClassId = const_cast<GUID*>(&uuid);

    // ��ȷ����������ַ
    BTH_ADDR bthAddr = btAddr;
    querySet.lpszContext = (LPWSTR)&bthAddr;

    HANDLE hLookup = NULL;
    if (WSALookupServiceBeginW(&querySet, LUP_FLUSHPREVIOUS, &hLookup) != 0) {
        return -1;
    }

    BYTE buffer[4096] = { 0 };
    DWORD bufferSize = sizeof(buffer);
    WSAQUERYSETW* pResults = (WSAQUERYSETW*)buffer;

    int port = -1;
    while (WSALookupServiceNextW(hLookup, LUP_RETURN_ADDR, &bufferSize, pResults) == 0) {
        if (pResults->lpcsaBuffer && pResults->lpcsaBuffer->RemoteAddr.lpSockaddr) {
            SOCKADDR_BTH* pSockAddr = (SOCKADDR_BTH*)pResults->lpcsaBuffer->RemoteAddr.lpSockaddr;
            port = (int)pSockAddr->port;
            break;
        }
    }

    WSALookupServiceEnd(hLookup);
    return port;
}
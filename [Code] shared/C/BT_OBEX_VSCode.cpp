#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <winsock2.h>
#include <ws2bth.h>
#include <bluetoothapis.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Bthprops.lib")

static const GUID OBEXObjectPushServiceClass_UUID = { 0x00001105, 0x0000, 0x1000, { 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } };

int GetObexPort(ULONGLONG btAddr, const GUID& uuid);
int SendObexFile(SOCKET btSocket, const wchar_t* filePath);

int wmain(int argc, wchar_t* argv[]) {
    if (argc != 3) {
        printf("�÷�: %S <���ε������豸��> <�ļ���ַ>\n", argv[0]);
        return 1;
    }

    const wchar_t* targetName = argv[1];
    const wchar_t* filePath = argv[2];

    BLUETOOTH_DEVICE_SEARCH_PARAMS searchParams = {
        sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS),
        TRUE, TRUE, TRUE, TRUE, TRUE, 8, NULL
    };

    BLUETOOTH_DEVICE_INFO deviceInfo = { sizeof(BLUETOOTH_DEVICE_INFO) };
    HBLUETOOTH_DEVICE_FIND hFind = BluetoothFindFirstDevice(&searchParams, &deviceInfo);
    ULONGLONG targetAddr = 0;

    if (hFind) {
        do {
            if (_wcsicmp(deviceInfo.szName, targetName) == 0) {
                targetAddr = deviceInfo.Address.ullLong;
                break;
            }
        } while (BluetoothFindNextDevice(hFind, &deviceInfo));
        BluetoothFindDeviceClose(hFind);
    }

    if (targetAddr == 0) {
        printf("�޷��ҵ������ε������豸\n");
        return 1;
    }

    int port = GetObexPort(targetAddr, OBEXObjectPushServiceClass_UUID);
    if (port <= 0) {
        printf("��Ӧ�豸�� OBEX ����δ�ҵ�\n");
        return 1;
    }

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET btSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
    SOCKADDR_BTH addr = { AF_BTH, targetAddr, (ULONG)port, 0 };

    if (connect(btSocket, (SOCKADDR*)&addr, sizeof(addr)) != 0) {
        printf("����ʧ��: %d\n", WSAGetLastError());
        closesocket(btSocket);
        WSACleanup();
        return 1;
    }

    if (SendObexFile(btSocket, filePath) != 0) {
        printf("�ļ�����ʧ��\n");
    }
    else {
        printf("�ļ�������ɣ�\n");
    }

    closesocket(btSocket);
    WSACleanup();
    return 0;
}

int GetObexPort(ULONGLONG btAddr, const GUID& uuid) {
    WSAQUERYSETW querySet = { sizeof(WSAQUERYSETW) };
    querySet.dwNameSpace = NS_BTH;
    querySet.lpServiceClassId = const_cast<GUID*>(&uuid);

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

int SendObexFile(SOCKET btSocket, const wchar_t* filePath) {
    FILE* file = _wfopen(filePath, L"rb");
    if (!file) {
        printf("�޷��Ķ��ļ�: %s\n", filePath);
        return -1;
    }

    // ��ȡ�ļ���С
    struct _stat fileStat;
    _wstat(filePath, &fileStat);
    DWORD fileSize = (DWORD)fileStat.st_size;

    // ���� CONNECT ��
    unsigned char connectPacket[] = {
        0x80,       // CONNECT
        0x00, 0x07, // �����ȣ�7�ֽ�
        0x10,       // OBEX �汾 1.0
        0x00,       // ��־
        0x00, 0x20, // ������С 8192
    };
    if (send(btSocket, (const char*)connectPacket, sizeof(connectPacket), 0) == SOCKET_ERROR) {
        printf("����ʧ��: %d\n", WSAGetLastError());
        fclose(file);
        return -1;
    }

    // ��ȡ��Ӧ
    unsigned char response[3];
    if (recv(btSocket, (char*)response, sizeof(response), 0) < 3 || response[0] != 0xA0) {
        printf("������Ӧ����\n");
        fclose(file);
        return -1;
    }

    // ���� PUT ����ͷ
    const wchar_t* fileName = wcsrchr(filePath, L'\\');
    fileName = fileName ? fileName + 1 : filePath;
    size_t fileNameLen = wcslen(fileName) * 2;

    DWORD packetSize = 3 + 5 + 3 + (DWORD)fileNameLen + 3 + fileSize;
    unsigned char putHeader[5];
    putHeader[0] = 0x02; // PUT ���հ�
    putHeader[1] = (packetSize >> 8) & 0xFF;
    putHeader[2] = packetSize & 0xFF;

    // ���� PUT ͷ
    if (send(btSocket, (const char*)putHeader, 3, 0) == SOCKET_ERROR) {
        printf("PUT header ����ʧ��\n");
        fclose(file);
        return -1;
    }

    // ���� Name ͷ
    unsigned char nameHeader[3];
    nameHeader[0] = 0x01; // Name
    nameHeader[1] = ((3 + fileNameLen) >> 8) & 0xFF;
    nameHeader[2] = (3 + fileNameLen) & 0xFF;
    send(btSocket, (const char*)nameHeader, 3, 0);
    send(btSocket, (const char*)fileName, (int)fileNameLen, 0);

    // ���� Length ͷ
    unsigned char lengthHeader[7];
    lengthHeader[0] = 0xC3; // Length
    lengthHeader[1] = 0x00;
    lengthHeader[2] = 0x04;
    lengthHeader[3] = (fileSize >> 24) & 0xFF;
    lengthHeader[4] = (fileSize >> 16) & 0xFF;
    lengthHeader[5] = (fileSize >> 8) & 0xFF;
    lengthHeader[6] = fileSize & 0xFF;
    send(btSocket, (const char*)lengthHeader, 5, 0);

    // ���� Body ͷ������
    unsigned char bodyHeader[3];
    bodyHeader[0] = 0x48; // Body
    bodyHeader[1] = 0x00;
    bodyHeader[2] = 0x00;

    char buffer[4096];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        bodyHeader[1] = (bytesRead + 3) >> 8;
        bodyHeader[2] = (bytesRead + 3) & 0xFF;
        send(btSocket, (const char*)bodyHeader, 3, 0);
        send(btSocket, buffer, (int)bytesRead, 0);
    }

    // ���ͽ�����
    unsigned char endPacket[] = { 0x82, 0x00, 0x03 };
    send(btSocket, (const char*)endPacket, 3, 0);

    fclose(file);
    return 0;
}
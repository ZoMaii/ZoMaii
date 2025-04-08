#include <windows.h>
#include <shlobj.h>
#include <iostream>
#pragma comment(lib, "Shell32.lib")

// 通过系统蓝牙工具 fsquirt.exe 触发传输
bool SendViaFsquirt(const wchar_t* filePath) {
    // 构造命令行参数
    std::wstring cmd = L"fsquirt.exe";

    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi;

    return CreateProcess(
        L"C:\\Windows\\System32\\fsquirt.exe",
        (LPWSTR)cmd.c_str(),
        NULL,
        NULL,
        FALSE,
        0,
        NULL,
        NULL,
        &si,
        &pi
    );
}

// 备用方案：通过控制面板指令
bool SendViaControlPanel(const wchar_t* filePath) {
    return (int)ShellExecute(
        NULL,
        L"open",
        L"control.exe",
        L"/name Microsoft.BluetoothDevices /page Advanced",
        NULL,
        SW_SHOW
    ) > 32;
}

int main() {
    const wchar_t* filePath = L" ";

    // 方法1：直接调用系统蓝牙工具
    if (SendViaFsquirt(filePath)) {
        std::wcout << L"已启动蓝牙传输向导" << std::endl;
        return 0;
    }

    // 方法2：备用控制面板方案
    if (SendViaControlPanel(filePath)) {
        std::wcout << L"请手动选择发送文件" << std::endl;
        return 0;
    }

    // 失败处理
    DWORD err = GetLastError();
    std::wcerr << L"操作失败 (错误代码: 0x" << std::hex << err << L")" << std::endl;

    // 显示具体错误信息
    if (err == ERROR_FILE_NOT_FOUND) {
        std::wcerr << L"系统缺少蓝牙支持组件 (fsquirt.exe)" << std::endl;
    }
    else if (err == ERROR_ACCESS_DENIED) {
        std::wcerr << L"请以管理员权限运行程序" << std::endl;
    }

    return 1;
}
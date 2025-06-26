#include "pch.h"
#include <windows.h>
#include "scs.h"

using namespace winrt;
using namespace Windows::Foundation;

// 打开指定COM口，成功返回句柄，失败返回INVALID_HANDLE_VALUE
HANDLE OpenSerialPort(const wchar_t* portName)
{
    HANDLE hComx = CreateFileW(
        portName,
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );
    return hComx;
}

// 向指定COM口写入数据，返回实际写入的字节数，失败返回-1
int WriteToSerialPort(HANDLE hComx, const void* buffer, DWORD length)
{
    DWORD bytesWritten = 0;
    if (!WriteFile(hComx, buffer, length, &bytesWritten, NULL)) {
        return -1;
    }
    return static_cast<int>(bytesWritten);
}

HANDLE hCom;

void scs_send(uint8_t* pkt, uint8_t pktsiz)
{

}

int main()
{
    init_apartment();
    Uri uri(L"http://aka.ms/cppwinrt");
    printf("Hello, %ls!\n", uri.AbsoluteUri().c_str());

    hCom = OpenSerialPort(L"COM3");

    return 0;
}

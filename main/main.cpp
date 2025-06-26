#include "pch.h"
#include <windows.h>
#include "scs.h"

using namespace winrt;
using namespace Windows::Foundation;

HANDLE hCom;
HANDLE hHeap;

// 打开指定COM口，成功返回句柄，失败返回INVALID_HANDLE_VALUE
HANDLE serial_open(const wchar_t* portName)
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

// 关闭指定COM口
void serial_close(HANDLE hComx)
{
    if (hComx != INVALID_HANDLE_VALUE) {
        CloseHandle(hComx);
    }
}

void* scs_malloc(size_t size)
{
    void* ptr = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, size);
    if (ptr == NULL) {
        printf("Memory allocation failed\n");
    }
    return ptr;
}

void scs_free(void* ptr)
{
    if (ptr != NULL) {
        HeapFree(hHeap, 0, ptr);
    }
}

void scs_send(uint8_t* pkt, uint32_t pktsiz)
{
    if (hCom == NULL || hCom == INVALID_HANDLE_VALUE) {
        printf("Serial port not open\n");
        return;
    }
    DWORD bytesWritten = 0;
    BOOL result = WriteFile(hCom, pkt, pktsiz, &bytesWritten, NULL);
    if (!result || bytesWritten != pktsiz) {
        printf("Write to serial port failed, written: %lu\n", bytesWritten);
    }
}

uint32_t scs_recv(uint8_t* buf, uint32_t bufsiz)
{
    if (hCom == NULL || hCom == INVALID_HANDLE_VALUE) {
        printf("Serial port not open\n");
        return 0;
    }
    DWORD bytesRead = 0;
    BOOL result = ReadFile(hCom, buf, bufsiz, &bytesRead, NULL);
    if (!result) {
        printf("Read from serial port failed\n");
        return 0;
    }
    return (uint32_t)bytesRead;
}

void scs_delay(uint32_t delay_ms)
{
    Sleep(delay_ms);
}

uint32_t scs_gettick()
{
    return (uint32_t)GetTickCount();
}

int main()
{
    init_apartment();
    Uri uri(L"http://aka.ms/cppwinrt");
    printf("Hello, %ls!\n", uri.AbsoluteUri().c_str());

	//注册回调函数
    scs_callback_register(
        scs_malloc,
        scs_free,
        scs_send,
        scs_recv,
        scs_delay,
        scs_gettick
	);

	//初始化COM口和获取进程堆
    hCom = serial_open(L"COM5");
	hHeap = GetProcessHeap();
    while (1) {
        scs_ping(42);
		Sleep(1000); // 每秒发送一次Ping
    }

    return 0;
}

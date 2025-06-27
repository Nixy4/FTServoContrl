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

int serial_config(HANDLE hComx, DWORD baudrate, BYTE parity, BYTE stopbits, BYTE bytesize)
{
    if (hComx == NULL || hComx == INVALID_HANDLE_VALUE) {
        printf("Invalid serial handle\n");
        return -1;
    }
    DCB dcb = {0};
    dcb.DCBlength = sizeof(DCB);
    if (!GetCommState(hComx, &dcb)) {
        printf("GetCommState failed\n");
        return -2;
    }
    dcb.BaudRate = baudrate;
    dcb.Parity = parity;
    dcb.StopBits = stopbits;
    dcb.ByteSize = bytesize;
    if (!SetCommState(hComx, &dcb)) {
        printf("SetCommState failed\n");
        return -3;
    }
    return 0;
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

void scs_send(uint8_t* pkt, scs_size pktsiz)
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

scs_size scs_recv(uint8_t* buf, scs_size bufsiz)
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
    return (scs_size)bytesRead;
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
	serial_config(hCom, 512000, NOPARITY, ONESTOPBIT, 8);
	hHeap = GetProcessHeap();

    scs_set_pos(1, 0);
    Sleep(1000);
    scs_set_pos(1, 1023);
    Sleep(1000);
    scs_set_pos(1, 0);

    return 0;
}

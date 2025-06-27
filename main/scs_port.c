#include "scs.h"

#include <windows.h>

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
    DCB dcb = { 0 };
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

void scs_port_init()
{
    // 初始化COM口
    hCom = serial_open(L"COM5");
    if (hCom == INVALID_HANDLE_VALUE) {
        printf("Failed to open COM port\n");
        return;
    }
    
    // 配置COM口参数
    if (serial_config(hCom, 512000, NOPARITY, ONESTOPBIT, 8) != 0) {
        printf("Failed to configure COM port\n");
        serial_close(hCom);
        return;
    }
    // 获取进程堆
    hHeap = GetProcessHeap();
    if (hHeap == NULL) {
        printf("Failed to get process heap\n");
        serial_close(hCom);
        return;
	}
}

void* scs_port_malloc(size_t size)
{
    void* ptr = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, size);
    if (ptr == NULL) {
        printf("Memory allocation failed\n");
    }
    return ptr;
}

void scs_port_free(void* ptr)
{
    if (ptr != NULL) {
        HeapFree(hHeap, 0, ptr);
    }
}

void scs_port_send(uint8_t* pkt, scs_size pktsiz)
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

scs_size scs_port_recv(uint8_t* buf, scs_size bufsiz)
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

void scs_port_delay(uint32_t delay_ms)
{
    Sleep(delay_ms);
}

uint32_t scs_port_gettick()
{
    return (uint32_t)GetTickCount();
}
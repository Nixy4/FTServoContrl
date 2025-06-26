#include "pch.h"
#include <windows.h>
#include "scs.h"

using namespace winrt;
using namespace Windows::Foundation;

HANDLE hCom;
HANDLE hHeap;

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

// 设置COM口参数，成功返回0，失败返回-1
int SetSerialPortParams(HANDLE hComx, DWORD baudRate, BYTE byteSize, BYTE parity, BYTE stopBits)
{
    DCB dcb = { 0 };
    dcb.DCBlength = sizeof(DCB);
    if (!GetCommState(hComx, &dcb)) {
        return -1; // 获取COM口状态失败
    }
    dcb.BaudRate = baudRate;
    dcb.ByteSize = byteSize;
    dcb.Parity = parity;
    dcb.StopBits = stopBits;
    if (!SetCommState(hComx, &dcb)) {
        return -1; // 设置COM口状态失败
    }
    return 0; // 成功
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

// 从指定COM口读取数据，返回实际读取的字节数，失败返回-1
int ReadFromSerialPort(HANDLE hComx, void* buffer, DWORD length)
{
    DWORD bytesRead = 0;
    if (!ReadFile(hComx, buffer, length, &bytesRead, NULL)) {
        return -1;
    }
    return static_cast<int>(bytesRead);
}

void scs_port_init(const wchar_t* portName, DWORD baudRate, BYTE byteSize, BYTE parity, BYTE stopBits)
{
    hCom = OpenSerialPort(portName);
    if (hCom == INVALID_HANDLE_VALUE) {
        printf("Failed to open serial port %ls\n", portName);
        return;
    }
    if (SetSerialPortParams(hCom, baudRate, byteSize, parity, stopBits) != 0) {
        printf("Failed to set serial port parameters\n");
        CloseHandle(hCom);
        hCom = INVALID_HANDLE_VALUE;
    }
}

void scs_port_deinit()
{
    if (hCom != INVALID_HANDLE_VALUE) {
        CloseHandle(hCom);
        hCom = INVALID_HANDLE_VALUE;
    }
}

void scs_send(uint8_t* buf, uint32_t size)
{
	WriteToSerialPort(hCom, buf, size);
}

uint32_t scs_recv(uint8_t* buf, uint32_t size)
{
    return (uint32_t)ReadFromSerialPort(hCom, buf, size);
}

void scs_delay(uint32_t delay_ms)
{
    Sleep(delay_ms);
}

uint32_t scs_gettick()
{
    return (uint32_t)GetTickCount();
}

void scs_memory_init()
{
    hHeap = GetProcessHeap();
    if (hHeap == NULL) {
        printf("Failed to get process heap\n");
    }
}

void* scs_malloc(size_t size)
{
    if (hHeap == NULL) {
        printf("Heap not initialized\n");
        return NULL;
    }
    void* ptr = HeapAlloc(hHeap, HEAP_ZERO_MEMORY, size);
    if (ptr == NULL) {
        printf("Memory allocation failed\n");
    }
    return ptr;
}

void scs_free(void* ptr)
{
    if (hHeap == NULL) {
        printf("Heap not initialized\n");
        return;
    }
    if (!HeapFree(hHeap, 0, ptr)) {
        printf("Memory deallocation failed\n");
    }
}

int main()
{
    init_apartment();
    Uri uri(L"http://aka.ms/cppwinrt");
    printf("Hello, %ls!\n", uri.AbsoluteUri().c_str());

    scs_callback_register(
        scs_send,
        scs_recv,
        scs_delay,
        scs_gettick
	);

	scs_memory_callback_register(
        scs_malloc,
        scs_free
    );

	scs_memory_init();

    scs_port_init(L"COM5", 500000, 8, NOPARITY, ONE5STOPBITS);
    if (hCom == INVALID_HANDLE_VALUE) {
        printf("Failed to initialize serial port\n");
        return -1;
	}

    scs_ping(42);
    return 0;
}

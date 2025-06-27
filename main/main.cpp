#include "pch.h"
#include <windows.h>
#include "scs.h"

using namespace winrt;
using namespace Windows::Foundation;

int main()
{
    init_apartment();

	scs_port_init();
    while (1) {
		scs_ping(1); // 发送Ping命令
		Sleep(1000); // 延时1秒
    }

    return 0;
}

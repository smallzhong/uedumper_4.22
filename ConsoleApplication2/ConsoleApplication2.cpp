// ConsoleApplication2.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "head.h"

const int g_base = 0x7FF63F4A0000;
const int g_gname_offset = 0x2E6E0C0;
const int g_GUObjectArray_offset = 0x2B8CA60;

DWORD g_pid = 0;
HANDLE g_hDevice = NULL;


void 初始化驱动()
{
	g_hDevice = CreateFileA("\\\\.\\C1ACDA339ED015753289D8DC63CF2A83", GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, 0);
	MY_ASSERT(g_hDevice != INVALID_HANDLE_VALUE);
}

bool Read(void* Addr, void* buf, int size)
{
	//return ReadProcessMemory(hProcess, Addr, buf, (SIZE_T)size, NULL);
	UserData Temp = { (DWORD)g_pid, (DWORD64)Addr, (DWORD)size, buf };
	DWORD RetSize = 0;
	BOOLEAN status = DeviceIoControl(g_hDevice, READ, &Temp, sizeof(UserData), NULL, 0, &RetSize, 0);
	MY_ASSERT(status != TRUE);
	return true;
}

void init()
{
	HWND hWnd = FindWindowA("UnrealWindow", NULL);
	if (!hWnd)
		EXIT_ERROR("窗口句柄获取失败,未找到UE窗口");
	GetWindowThreadProcessId(hWnd, &g_pid);
	if (!g_pid)
		EXIT_ERROR("获取Processid失败");
	初始化驱动();
}

int main()
{
	init();
}


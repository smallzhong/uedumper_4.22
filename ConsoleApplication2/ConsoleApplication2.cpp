// ConsoleApplication2.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "head.h"

const ULONG64 g_base = 0x7FF616AC0000;
const ULONG64 g_gname_offset = 0x2E6E0C0;
const ULONG64 g_GUObjectArray_offset = 0x2B8CA60;

// 4.22的一些define
const int ElementsPerChunk = 0x4000;
const int 从fname开头到字符串位置的偏移 = 0xc;
const int NAME_SIZE = 1024;

DWORD g_pid = 0;
HANDLE g_hDevice = NULL;


void 初始化驱动()
{
	g_hDevice = CreateFileA("\\\\.\\C1ACDA339ED015753289D8DC63CF2A83", GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, 0);
	MY_ASSERT(g_hDevice != INVALID_HANDLE_VALUE);
}

bool ReadMemory(ULONG64 Addr, void *buf, int size)
{
	//return ReadProcessMemory(hProcess, Addr, buf, (SIZE_T)size, NULL);
	UserData Temp = { (DWORD)g_pid, (DWORD64)Addr, (DWORD)size, buf };
	DWORD RetSize = 0;
	BOOLEAN status = DeviceIoControl(g_hDevice, READ, &Temp, sizeof(UserData), NULL, 0, &RetSize, 0);
	MY_ASSERT(status);
	return TRUE;
}

ULONG64 read8(ULONG64 addr)
{
	ULONG64 ret = 0;
	bool status = ReadMemory(addr, &ret, 8);
	MY_ASSERT(status);

	return ret;
}

ULONG32 read4(ULONG64 addr)
{
	ULONG32 ret = 0;
	bool status = ReadMemory(addr, &ret, 4);
	MY_ASSERT(status);

	return ret;
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

string get_name(uint32_t Index)
{
	ULONG64 t_gname = read8((ULONG64)g_base + g_gname_offset);
	
	ULONG32 ChunkIndex = Index / ElementsPerChunk;
	ULONG32 WithinChunkIndex = Index % ElementsPerChunk;

	ULONG64 chunk_ptr = read8(t_gname + ChunkIndex * 8);
    // 这里指向了FNameEntry
	ULONG64 fname_ptr = read8(chunk_ptr + WithinChunkIndex * 8);

	// TODO: 这里其实还有wide和ansi的判断，但是暂时没精力写。
	CHAR buf[NAME_SIZE];
	ReadMemory((ULONG64)fname_ptr + 从fname开头到字符串位置的偏移, buf, NAME_SIZE);

	return string(buf);
}



int main()
{
	init();
	for (int i = 0; i < 60000; i++)
	{
		cout << i << " " << get_name(i) << endl;
	}
}


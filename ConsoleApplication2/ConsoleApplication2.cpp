// ConsoleApplication2.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "head.h"
#include "file.h"

const ULONG64 g_base = 0x7FF616AC0000;
const ULONG64 g_gname_offset = 0x2E6E0C0;
const ULONG64 g_GUObjectArray_offset = 0x2B8CA60;
const int g_FUObjectArray_ObjObjects_offset = 0x10; // FUObjectArray结构体下面ObjObjects成员的offset
const int g_TUObjectArray_NumElements_offset = 0x14; // 以此类推。
const int g_TUObjectArray_NumChunks_offset = 0x1c; // 以此类推。
const int g_FUObjectItem_Object_offset = 0; // 以此类推。
const int g_UObject_Name_offset = 0x18; // 以此类推。
const int g_UObject_Class_offset = 0x10; // 以此类推。
const int g_UObject_Outer_offset = 0x20; // 以此类推。

const int g_UStruct_SuperStruct_offset = 0x40; // 以此类推。
const int g_UStruct_ChildProperties_offset = 0x48; // 以此类推。
const int g_UStruct_PropertiesSize_offset = 0x50; // 以此类推。



const int g_FUObjectItem_Size = 0x18; // FUObjectItem结构体的大小

// 4.22的一些define
const int ElementsPerChunk = 0x4000;
const int 从fname开头到字符串位置的偏移 = 0xc;
const int NAME_SIZE = 1024;

DWORD g_pid = 0;
HANDLE g_hDevice = NULL;

map<int, string> g_name_table;


void 初始化驱动()
{
	g_hDevice = CreateFileA("\\\\.\\C1ACDA339ED015753289D8DC63CF2A83", GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, 0);
	MY_ASSERT(g_hDevice != INVALID_HANDLE_VALUE);
}

bool ReadMemory(ULONG64 Addr, void* buf, int size)
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
	// 如果命中了就不用再手动查了。
	if (g_name_table.count(Index) != 0)
		return g_name_table[Index];

	ULONG64 t_gname = read8((ULONG64)g_base + g_gname_offset);

	ULONG32 ChunkIndex = Index / ElementsPerChunk;
	ULONG32 WithinChunkIndex = Index % ElementsPerChunk;

	ULONG64 chunk_ptr = read8(t_gname + ChunkIndex * 8);
	// 这里指向了FNameEntry
	ULONG64 fname_ptr = read8(chunk_ptr + WithinChunkIndex * 8);

	// TODO: 这里其实还有wide和ansi的判断，但是暂时没精力写。
	CHAR buf[NAME_SIZE];
	ReadMemory((ULONG64)fname_ptr + 从fname开头到字符串位置的偏移, buf, NAME_SIZE);

	string res = string(buf);
	g_name_table[Index] = res;

	return res;
}

// 获取gobjectarray里面有多少个element
int get_object_num_elememts()
{
	static int res = 0;
	if (res != 0)
	{
		return res;
	}
	res = read4(g_base + g_GUObjectArray_offset + g_FUObjectArray_ObjObjects_offset + g_TUObjectArray_NumElements_offset);
	return res;
}

// 根据object的id获取其地址。
ULONG64 get_uobject_addr_by_id(ULONG64 TUObjectArray_addr, ULONG64 id)
{
	int i = 0;
	for (; id > 65536; i++)
	{
		id -= 65536;
	}

	ULONG64 chunks = read8(TUObjectArray_addr + i * 8);
	ULONG64 item = read8(chunks) + g_FUObjectItem_Size * id;

	return read8(item + g_FUObjectItem_Object_offset);
}

ULONG64 get_struct_super(ULONG64 uobject_addr)
{
	return read8(uobject_addr + g_UStruct_SuperStruct_offset);
}

ULONG64 get_UObject_staticClass(ULONG64 TUObjectArray_addr)
{
	static ULONG64 res = 0;
	if (res != 0)
	{
		return res;
	}
	res = FindObject(TUObjectArray_addr, "Class\t/Script/CoreUObject.Object");
	return res;
}

ULONG64 get_AActor_staticClass(ULONG64 TUObjectArray_addr)
{
	static ULONG64 res = 0;
	if (res != 0)
	{
		return res;
	}
	res = FindObject(TUObjectArray_addr, "Class\t/Script/Engine.Actor");
	return res;
}

ULONG64 get_UEnum_staticClass(ULONG64 TUObjectArray_addr)
{
	static ULONG64 res = 0;
	if (res != 0)
	{
		return res;
	}
	res = FindObject(TUObjectArray_addr, "Class\t/Script/CoreUObject.Enum");
	return res;
}

ULONG64 get_UClass_staticClass(ULONG64 TUObjectArray_addr)
{
	static ULONG64 res = 0;
	if (res != 0)
	{
		return res;
	}
	res = FindObject(TUObjectArray_addr, "Class\t/Script/CoreUObject.Class");
	return res;
}

ULONG64 get_UFunction_staticClass(ULONG64 TUObjectArray_addr)
{
	static ULONG64 res = 0;
	if (res != 0)
	{
		return res;
	}
	res = FindObject(TUObjectArray_addr, "Class\t/Script/CoreUObject.Function");
	return res;
}

ULONG64 get_UScriptStruct_staticClass(ULONG64 TUObjectArray_addr)
{
	static ULONG64 res = 0;
	if (res != 0)
	{
		return res;
	}
	res = FindObject(TUObjectArray_addr, "Class\t/Script/CoreUObject.ScriptStruct");
	return res;
}



ULONG64 FindObject(ULONG64 TUObjectArray_addr, string fullName)
{
	for (int i = 0; i < get_object_num_elememts(); i++)
	{
		ULONG64 cur_object = get_uobject_addr_by_id(TUObjectArray_addr, i);
		string t_fullname = get_object_fullName(cur_object);
		if (cur_object && t_fullname == fullName)
		{
			return cur_object;
		}
	}

	return NULL;
}

ULONG64 get_object_outer(ULONG64 uobject_addr)
{
	return read8(uobject_addr + g_UObject_Outer_offset);
}

ULONG64 get_object_class(ULONG64 uobject_addr)
{
	return read8(uobject_addr + g_UObject_Class_offset);
}


string get_object_name(ULONG64 uobject_addr)
{
	ULONG32 NameId = read4(uobject_addr + g_UObject_Name_offset);
	return get_name(NameId);
}

string get_object_fullName(ULONG64 uobject_addr)
{
	string temp = get_object_name(uobject_addr);
	if (temp == "" || temp == "None")
		return temp;

	for (ULONG64 outer = get_object_outer(uobject_addr); outer; outer = get_object_outer(outer))
	{
		temp = get_object_name(outer) + "." + temp;
	}

	return get_object_name(get_object_class(uobject_addr)) + "\t" + temp;
}

bool IsA_UObject(ULONG64 TUObjectArray_addr, ULONG64 uobject_addr)
{
	ULONG64 cmp = get_UObject_staticClass(TUObjectArray_addr);
	MY_ASSERT(cmp);

	for (ULONG64 super = get_object_class(uobject_addr); super; super = get_struct_super(super))
	{
		if (super == cmp)
			return TRUE;
	}

	return FALSE;
}
bool IsA_AActor(ULONG64 TUObjectArray_addr, ULONG64 uobject_addr)
{
	ULONG64 cmp = get_AActor_staticClass(TUObjectArray_addr);
	MY_ASSERT(cmp);

	for (ULONG64 super = get_object_class(uobject_addr); super; super = get_struct_super(super))
	{
		if (super == cmp)
			return TRUE;
	}

	return FALSE;
}
bool IsA_UEnum(ULONG64 TUObjectArray_addr, ULONG64 uobject_addr)
{
	ULONG64 cmp = get_UEnum_staticClass(TUObjectArray_addr);
	MY_ASSERT(cmp);

	for (ULONG64 super = get_object_class(uobject_addr); super; super = get_struct_super(super))
	{
		if (super == cmp)
			return TRUE;
	}

	return FALSE;
}
bool IsA_UClass(ULONG64 TUObjectArray_addr, ULONG64 uobject_addr)
{
	ULONG64 cmp = get_UClass_staticClass(TUObjectArray_addr);
	MY_ASSERT(cmp);

	for (ULONG64 super = get_object_class(uobject_addr); super; super = get_struct_super(super))
	{
		if (super == cmp)
			return TRUE;
	}

	return FALSE;
}
bool IsA_UFunction(ULONG64 TUObjectArray_addr, ULONG64 uobject_addr)
{
	ULONG64 cmp = get_UFunction_staticClass(TUObjectArray_addr);
	MY_ASSERT(cmp);

	for (ULONG64 super = get_object_class(uobject_addr); super; super = get_struct_super(super))
	{
		if (super == cmp)
			return TRUE;
	}

	return FALSE;
}
bool IsA_UScriptStruct(ULONG64 TUObjectArray_addr, ULONG64 uobject_addr)
{
	ULONG64 cmp = get_UScriptStruct_staticClass(TUObjectArray_addr);
	MY_ASSERT(cmp);

	for (ULONG64 super = get_object_class(uobject_addr); super; super = get_struct_super(super))
	{
		if (super == cmp)
			return TRUE;
	}

	return FALSE;
}

void dump_objects()
{
	File objects("objects.txt");

	ULONG64 TUObjectArray_addr = g_base + g_GUObjectArray_offset + g_FUObjectArray_ObjObjects_offset;

	for (int i = 0; i < get_object_num_elememts(); i++)
	{
		ULONG64 cur_uobject_addr = get_uobject_addr_by_id(TUObjectArray_addr, i);
		if (!cur_uobject_addr)
			continue;

		if (IsA_UEnum(TUObjectArray_addr, cur_uobject_addr))
		{
			objects.fprintf("IsA_UEnum\n");
		}
		else if (IsA_UClass(TUObjectArray_addr, cur_uobject_addr))
		{
			objects.fprintf("IsA_UClass\n");

		}
		else if (IsA_UFunction(TUObjectArray_addr, cur_uobject_addr))
		{
			objects.fprintf("IsA_UFunction\n");

		}
		else if (IsA_UScriptStruct(TUObjectArray_addr, cur_uobject_addr))
		{
			objects.fprintf("IsA_UScriptStruct\n");
		}



		string name = get_object_fullName(cur_uobject_addr);



		objects.fprintf("[%017llx] %s\n", cur_uobject_addr, name.c_str());
	}
}

void 测试()
{
	init();
	dump_objects();
}

int main()
{
	clock_t start, end;
	start = clock();

	测试();

	end = clock();
	cout << "\n\n运行时间" << (double)(end - start) / CLOCKS_PER_SEC << endl;
}


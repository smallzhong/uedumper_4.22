// ConsoleApplication2.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "head.h"
#include "file.h"

const ULONG64 g_base = 0x7FF6142A0000;
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
const int g_UStruct_PropertiesSize_offset = 0x50; // 以此类推。
const int g_FNameEntry_AnsiName_or_WideName_offset = 0xc; // 以此类推。
const int g_UEnum_Names_offset = 0x40; // 以此类推
const int g_UFunction_Func_offset = 0xC0; // 以此类推

const int g_UStruct_Children_offset = 0x48; // 以此类推

const int g_UField_Next_offset = 0x28; // 以此类推


const int g_FUObjectItem_Size = 0x18; // FUObjectItem结构体的大小
const int g_UEnum_Names_SizeEveryEntry = 0x10; // UEnum里面names的每一个项目的大小（相当于sizeof(TPair)）

// 4.22的一些define
const int ElementsPerChunk = 0x4000;
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
	ReadMemory((ULONG64)fname_ptr + g_FNameEntry_AnsiName_or_WideName_offset, buf, NAME_SIZE);

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

string get_object_cppname(ULONG64 uobject_addr)
{
	string Name = get_object_name(uobject_addr);
	cout << Name << endl;
	if (Name == "" || Name == "None")
		return string();

	ULONG64 TUObjectArray_addr = g_base + g_GUObjectArray_offset + g_FUObjectArray_ObjObjects_offset;
	for (ULONG64  i = uobject_addr; i; i = get_struct_super(i))
	{
		if (i > g_base)
			break;
		if (IsA_AActor(TUObjectArray_addr, i))
			return "A" + Name;
		else if (IsA_UObject(TUObjectArray_addr, i))
			return "U" + Name;
	}

	return "F" + Name;
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

ULONG32 get_struct_PropertiesSize(ULONG64 uobject_addr)
{
	return read4(uobject_addr + g_UStruct_PropertiesSize_offset);
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

ULONG64 get_function_addr(ULONG64 uobject_addr)
{
	return read8(uobject_addr + g_UFunction_Func_offset);
}

File uenum_logger("enum.txt");
File ufunction_logger("function.txt");
File uclass_logger("class.txt");

ULONG64 get_struct_children(ULONG64 uobject_addr)
{
	return read8(uobject_addr + g_UStruct_Children_offset);
}

ULONG64 get_field_next(ULONG64 uobject_addr)
{
	return read8(uobject_addr + g_UField_Next_offset);
}

void dump_UFunction(ULONG64 uobject_addr)
{
	string FullName = "//" + get_object_fullName(uobject_addr) + "\n";
	ULONG64 func_addr = get_function_addr(uobject_addr);
	ULONG64 func_offset = func_addr - g_base;

	MY_ASSERT(func_offset); // TODO: 这里不一定，看别人的代码里面是return，但我觉得如果是0说明出错了

	// TODO: 名字中有__Delegate应返回，这里不返回。
	string FuncName = get_object_cppname(get_object_outer(uobject_addr)) + "::" + get_object_name(uobject_addr);

	string body = "";
	for (ULONG64 i = get_struct_children(uobject_addr); i; i = get_field_next(i))
	{
		string name = get_object_name(i);
		string klass = get_object_name(get_object_class(i));

		string retValue = "void ";

		if ("ReturnValue" == name)
		{
			retValue = klass;
		}

		body += klass + " " + name  + " ";

		ufunction_logger.fprintf("%s// Offset::0x%X;\n%s %s(%s)\n\n", FullName.c_str(), func_offset, retValue.c_str(), FuncName.c_str(), body.c_str());
	}
}

void dump_UEnum(ULONG64 uobject_addr)
{
	string FullName = "// " + get_object_fullName(uobject_addr) + "\n";
	string ClassName = "enum class " + get_object_name(uobject_addr) + " : ";
	string Type = "uint8_t";
	string Body = "";
	uint32_t Max = INT32_MIN;

	TArray<char*> Names = { 0 };
	ReadMemory(uobject_addr + g_UEnum_Names_offset, &Names, sizeof(Names));

	for (int i = 0; i < Names.ArrayNum; i++)
	{
		TPair<FName, ULONG64> cur_name;
		ReadMemory((ULONG64)Names.Allocator + i * g_UEnum_Names_SizeEveryEntry, &cur_name, sizeof(cur_name));

		string name = get_name(cur_name.first.ComparisonIndex);
		uint32_t value = cur_name.second;
		Max = max(value, Max);

		Body += "\t" + name + " " + to_string(value) + "\n";
	}

	if (Max > 256)
		Type = "uint32_t";

	uenum_logger.fprintf(FullName + ClassName + Type + "\n{\n" + Body + "};\n\n");
}

void dump_UClass(ULONG64 uobject_addr)
{
	string FullName = "// " + get_object_fullName(uobject_addr);
	uint32_t classSize = get_struct_PropertiesSize(uobject_addr);
	string ClassName = "class " + get_object_cppname(uobject_addr);
	
	uclass_logger.fprintf("%s\n// class size = 0x%04x\n%s\n{\n%s\n}\n\n", FullName.c_str(), classSize, ClassName.c_str(), "身体");
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
			dump_UEnum(cur_uobject_addr);
			//objects.fprintf("IsA_UEnum\n");
		}
		else if (IsA_UClass(TUObjectArray_addr, cur_uobject_addr))
		{
			dump_UClass(cur_uobject_addr);
			//objects.fprintf("IsA_UClass\n");
		}
		else if (IsA_UFunction(TUObjectArray_addr, cur_uobject_addr))
		{
			dump_UFunction(cur_uobject_addr);
			//objects.fprintf("IsA_UFunction\n");
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
	//cout << get_name(65) << endl;
}

int main()
{
	clock_t start, end;
	start = clock();

	测试();

	end = clock();
	cout << "\n\n运行时间" << (double)(end - start) / CLOCKS_PER_SEC << endl;
}


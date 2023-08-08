#pragma once
#include <assert.h>
#include <malloc.h>
#include <stdio.h>
#include <windows.h>

#include <iostream>

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <limits.h>
#include <map>
#include <string>
#include <time.h>

using namespace std;
#pragma warning(disable : 4996)
#pragma warning(disable : 6031) // 忽略警告"返回值被忽略: “getchar”"


#define EXIT_ERROR(x)                                 \
    do                                                \
    {                                                 \
        cout << "error in line " << __LINE__ << endl; \
        cout << x;                                    \
        getchar();                                    \
        exit(EXIT_FAILURE);                           \
    } while (0)

#define FAIL_LOG(x)                                          \
    do                                                       \
    {                                                        \
        cout << "test failure in line " << __LINE__ << endl; \
        cout << x;                                           \
        getchar();                                           \
        exit(EXIT_FAILURE);                                  \
    } while (0)

#define MY_ASSERT(x)                         \
    do                                       \
    {                                        \
        if (!(x))                            \
            EXIT_ERROR("ASSERTION failed!"); \
    } while (0)

typedef struct _UserData
{
    DWORD Pid;			//要读写的进程ID
    DWORD64 Address;	//要读写的地址
    DWORD Size;			//读写长度
    PVOID Data;			//数据存放位置
}UserData, * PUserData;

template<typename T>
class TArray
{
public:
    T* Allocator;
    int32_t ArrayNum;
    int32_t ArrayMax;
};

typedef struct _FName
{
    uint32_t ComparisonIndex;
    uint32_t Number;
} FName, * PFName;

template<typename A ,typename B>
struct TPair
{
    A first;
    B second;
};

#define READ	CTL_CODE(FILE_DEVICE_UNKNOWN,3333,METHOD_BUFFERED,FILE_ANY_ACCESS)


void 初始化驱动();
bool ReadMemory(ULONG64 Addr, void* buf, int size);
void init();
ULONG32 read4(PVOID addr);
ULONG64 read8(PVOID addr);
string get_name(uint32_t Index);
ULONG64 get_object_outer(ULONG64 uobject_addr);
ULONG64 get_object_class(ULONG64 uobject_addr);
string get_object_name(ULONG64 uobject_addr);
string get_object_fullName(ULONG64 uobject_addr);
ULONG64 FindObject(ULONG64 TUObjectArray_addr, string fullName);
ULONG64 get_UObject_staticClass(ULONG64 TUObjectArray_addr);
ULONG64 get_AActor_staticClass(ULONG64 TUObjectArray_addr);
ULONG64 get_UEnum_staticClass(ULONG64 TUObjectArray_addr);
ULONG64 get_UClass_staticClass(ULONG64 TUObjectArray_addr);
ULONG64 get_UFunction_staticClass(ULONG64 TUObjectArray_addr);
ULONG64 get_UScriptStruct_staticClass(ULONG64 TUObjectArray_addr);

bool IsA_UObject(ULONG64 TUObjectArray_addr, ULONG64 uobject_addr);
bool IsA_AActor(ULONG64 TUObjectArray_addr, ULONG64 uobject_addr);
bool IsA_UEnum(ULONG64 TUObjectArray_addr, ULONG64 uobject_addr);
bool IsA_UClass(ULONG64 TUObjectArray_addr, ULONG64 uobject_addr);
bool IsA_UFunction(ULONG64 TUObjectArray_addr, ULONG64 uobject_addr);
bool IsA_UScriptStruct(ULONG64 TUObjectArray_addr, ULONG64 uobject_addr);
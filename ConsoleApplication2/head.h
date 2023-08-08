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

using namespace std;
#pragma warning(disable : 4996)
#pragma warning(disable : 6031) // ���Ծ���"����ֵ������: ��getchar��"


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
    DWORD Pid;			//Ҫ��д�Ľ���ID
    DWORD64 Address;	//Ҫ��д�ĵ�ַ
    DWORD Size;			//��д����
    PVOID Data;			//���ݴ��λ��
}UserData, * PUserData;

#define READ	CTL_CODE(FILE_DEVICE_UNKNOWN,3333,METHOD_BUFFERED,FILE_ANY_ACCESS)

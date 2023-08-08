#pragma once

#include "head.h"

/*
* by ACC
* bÕ¾Id : °µ²ØC
* QÈº:417675203
*/

class File
{
public:
	File(const char* FileName);

	~File();

	int fprintf(string str);

	int fprintf(const char* pstr, ...);


	FILE* GetFile();
	int File::fflush();

private:
	FILE* file;
};


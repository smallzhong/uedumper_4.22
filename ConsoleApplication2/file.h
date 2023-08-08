#pragma once

#include "head.h"

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

